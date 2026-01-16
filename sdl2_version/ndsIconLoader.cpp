#include "ndsIconLoader.h"
#include <cstdio>
#include <cstring>
#include <iostream>

SDL_Renderer* NDSIconLoader::renderer = nullptr;
std::map<std::string, SDL_Texture*> NDSIconLoader::iconCache;

void NDSIconLoader::init(SDL_Renderer* renderer) {
    NDSIconLoader::renderer = renderer;
}

void NDSIconLoader::cleanup() {
    clearCache();
    renderer = nullptr;
}

void NDSIconLoader::clearCache() {
    for (auto& pair : iconCache) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    iconCache.clear();
}

bool NDSIconLoader::readNDSHeader(FILE* fp, NDSHeader& header) {
    if (!fp) return false;
    
    fseek(fp, 0, SEEK_SET);
    
    // 读取游戏标题和代码
    if (fread(header.gameTitle, 12, 1, fp) != 1) return false;
    if (fread(header.gameCode, 4, 1, fp) != 1) return false;
    
    // 跳过到bannerOffset位置（偏移0x68）
    fseek(fp, 0x68, SEEK_SET);
    if (fread(&header.bannerOffset, 4, 1, fp) != 1) return false;
    
    return true;
}

bool NDSIconLoader::readBanner(FILE* fp, u32 bannerOffset, NDSBanner& banner) {
    if (!fp || bannerOffset == 0) return false;
    
    fseek(fp, bannerOffset, SEEK_SET);
    
    // 读取版本和CRC
    if (fread(&banner.version, 2, 1, fp) != 1) return false;
    if (fread(banner.crc, 8, 1, fp) != 1) return false;
    if (fread(banner.reserved, 22, 1, fp) != 1) return false;
    
    // 读取图标数据（512字节）
    if (fread(banner.icon, 512, 1, fp) != 1) return false;
    
    // 读取调色板（16个u16，32字节）
    if (fread(banner.palette, 32, 1, fp) != 1) return false;
    
    // 读取标题（8种语言，每种128个u16，共2048字节）
    if (fread(banner.titles, sizeof(banner.titles), 1, fp) != 1) {
        // 如果读取失败，尝试只读取原始banner大小
        fseek(fp, bannerOffset + 0x20 + 512 + 32, SEEK_SET);
        // 只读取第一个标题（英语）
        if (fread(banner.titles[1], 128 * 2, 1, fp) != 1) {
            return false;
        }
    }
    
    return true;
}

// 将NDS tile格式的图标数据转换为线性格式
static void convertIconTilesToRaw(const u8* tilesSrc, u8* tilesNew) {
    const int PY = 32;  // 像素高度
    const int PX = 16;  // 字节宽度（32像素 / 2，因为4位深度）
    const int TILE_SIZE_Y = 8;
    const int TILE_SIZE_X = 4;
    int index = 0;
    
    // NDS图标是tile格式：4x8像素的tile排列
    // 按照原始代码的逻辑进行转换
    for (int tileY = 0; tileY < PY / TILE_SIZE_Y; ++tileY) {
        for (int tileX = 0; tileX < PX / TILE_SIZE_X; ++tileX) {
            for (int pY = 0; pY < TILE_SIZE_Y; ++pY) {
                for (int pX = 0; pX < TILE_SIZE_X; ++pX) {
                    // 计算目标位置（与原始代码保持一致）
                    // 这是字节索引：pX + tileX * TILE_SIZE_X + PX * (pY + tileY * TILE_SIZE_Y)
                    int destPos = pX + tileX * TILE_SIZE_X + PX * (pY + tileY * TILE_SIZE_Y);
                    
                    // 边界检查，防止数组越界
                    if (destPos >= 0 && destPos < 512 && index < 512) {
                        tilesNew[destPos] = tilesSrc[index++];
                    } else {
                        // 如果越界，跳过并增加索引
                        index++;
                    }
                }
            }
        }
    }
}

SDL_Texture* NDSIconLoader::convertIconToTexture(const u8* iconData, const u16* palette) {
    if (!renderer || !iconData || !palette) return nullptr;
    
    // 首先将tile格式转换为线性格式
    u8 linearIconData[512];
    convertIconTilesToRaw(iconData, linearIconData);
    
    // 创建32x32的RGBA表面
    SDL_Surface* surface = SDL_CreateRGBSurface(
        0, 32, 32, 32,
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
    );
    
    if (!surface) {
        std::cerr << "无法创建表面: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    // 锁定表面以写入像素
    SDL_LockSurface(surface);
    Uint32* pixels = (Uint32*)surface->pixels;
    
    // 转换4位图标数据为RGBA
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            int index = y * 32 + x;
            int byteIndex = index / 2;
            int nibbleIndex = index % 2;
            
            // 提取4位像素索引（从线性数据）
            u8 pixelIndex;
            if (nibbleIndex == 0) {
                pixelIndex = linearIconData[byteIndex] & 0x0F;  // 低4位
            } else {
                pixelIndex = (linearIconData[byteIndex] >> 4) & 0x0F;  // 高4位
            }
            
            // 从调色板获取颜色（RGB15格式，小端序）
            u16 rgb15 = palette[pixelIndex];
            
            // 转换RGB15到RGBA8888
            // RGB15格式：位0-4=红色，位5-9=绿色，位10-14=蓝色，位15未使用
            int r = ((rgb15 >> 0) & 31) << 3;
            int g = ((rgb15 >> 5) & 31) << 3;
            int b = ((rgb15 >> 10) & 31) << 3;
            
            // 确保值在0-255范围内
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            
            // 如果颜色是透明色（索引0且RGB为0），设置为透明
            Uint8 a = 255;
            if (pixelIndex == 0 && rgb15 == 0) {
                a = 0;
            }
            
            // 设置像素（RGBA格式）
            pixels[y * 32 + x] = (a << 24) | (b << 16) | (g << 8) | r;
        }
    }
    
    SDL_UnlockSurface(surface);
    
    // 创建纹理
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    return texture;
}

SDL_Texture* NDSIconLoader::loadIconFromNDS(const std::string& filePath) {
    if (!renderer) {
        std::cerr << "渲染器未初始化" << std::endl;
        return nullptr;
    }
    
    // 检查缓存
    auto it = iconCache.find(filePath);
    if (it != iconCache.end()) {
        return it->second;
    }
    
    // 打开NDS文件
    FILE* fp = fopen(filePath.c_str(), "rb");
    if (!fp) {
        std::cerr << "无法打开NDS文件: " << filePath << std::endl;
        return nullptr;
    }
    
    // 读取文件头
    NDSHeader header;
    if (!readNDSHeader(fp, header)) {
        fclose(fp);
        return nullptr;
    }
    
    // 检查banner偏移
    if (header.bannerOffset == 0) {
        fclose(fp);
        return nullptr;
    }
    
    // 读取Banner数据
    NDSBanner banner;
    if (!readBanner(fp, header.bannerOffset, banner)) {
        fclose(fp);
        return nullptr;
    }
    
    fclose(fp);
    
    // 转换图标为纹理
    SDL_Texture* texture = convertIconToTexture(banner.icon, banner.palette);
    
    if (texture) {
        // 缓存纹理
        iconCache[filePath] = texture;
    }
    
    return texture;
}

// UTF-16转UTF-8辅助函数
static std::string utf16ToUtf8(const u16* utf16, size_t maxLen) {
    std::string result;
    for (size_t i = 0; i < maxLen && utf16[i] != 0; i++) {
        u16 code = utf16[i];
        if (code < 0x80) {
            result += (char)code;
        } else if (code < 0x800) {
            result += (char)(0xC0 | (code >> 6));
            result += (char)(0x80 | (code & 0x3F));
        } else {
            result += (char)(0xE0 | (code >> 12));
            result += (char)(0x80 | ((code >> 6) & 0x3F));
            result += (char)(0x80 | (code & 0x3F));
        }
    }
    return result;
}

std::string NDSIconLoader::loadTitleFromNDS(const std::string& filePath, int langIndex) {
    // 限制语言索引范围
    if (langIndex < 0) langIndex = 0;
    if (langIndex > 7) langIndex = 7;
    
    // 打开NDS文件
    FILE* fp = fopen(filePath.c_str(), "rb");
    if (!fp) {
        return "";
    }
    
    // 读取文件头
    NDSHeader header;
    if (!readNDSHeader(fp, header)) {
        fclose(fp);
        return "";
    }
    
    // 检查banner偏移
    if (header.bannerOffset == 0) {
        fclose(fp);
        return "";
    }
    
    // 读取Banner数据
    NDSBanner banner;
    if (!readBanner(fp, header.bannerOffset, banner)) {
        fclose(fp);
        return "";
    }
    
    fclose(fp);
    
    // 选择标题语言（优先使用指定语言，如果为空则尝试其他语言）
    int currentLang = langIndex;
    while (currentLang >= 0 && 
           (banner.titles[currentLang][0] == 0 || 
            (banner.titles[currentLang][0] == 0x20 && banner.titles[currentLang][1] == 0))) {
        currentLang--;
        if (currentLang < 0) break;
    }
    
    if (currentLang < 0) {
        // 如果所有语言都为空，返回空字符串
        return "";
    }
    
    // 转换UTF-16到UTF-8
    return utf16ToUtf8(banner.titles[currentLang], 128);
}

