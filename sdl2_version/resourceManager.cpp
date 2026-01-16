#include "resourceManager.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>

SDL_Renderer* ResourceManager::renderer = nullptr;
std::string ResourceManager::themeBasePath = "../romsel_dsimenutheme/nitrofiles/themes/3ds/light";
std::map<std::string, SDL_Texture*> ResourceManager::textureCache;

void ResourceManager::init(SDL_Renderer* renderer) {
    ResourceManager::renderer = renderer;
    
    // 默认使用Classic DS Menu主题（3ds/light）
    // 尝试多个可能的主题路径，优先使用nitrofiles目录
    const char* possiblePaths[] = {
        "../theme",  // 优先：theme目录
        nullptr
    };
    
    bool found = false;
    for (int i = 0; possiblePaths[i] != nullptr; i++) {
        std::string testPath = std::string(possiblePaths[i]) + "/theme.ini";
        std::ifstream test(testPath);
        if (test.good()) {
            themeBasePath = possiblePaths[i];
            test.close();
            std::cout << "找到Classic DS Menu主题路径: " << themeBasePath << std::endl;
            found = true;
            break;
        }
        // 也检查是否有background目录
        testPath = std::string(possiblePaths[i]) + "/background";
        std::ifstream test2(testPath);
        if (test2.good()) {
            themeBasePath = possiblePaths[i];
            test2.close();
            std::cout << "找到Classic DS Menu主题路径（通过background目录）: " << themeBasePath << std::endl;
            found = true;
            break;
        }
    }
    
    if (!found) {
        std::cout << "警告：未找到Classic DS Menu主题，使用默认路径" << std::endl;
    }
}

void ResourceManager::cleanup() {
    clearCache();
    renderer = nullptr;
}

SDL_Texture* ResourceManager::loadImage(const std::string& path) {
    if (!renderer) {
        std::cerr << "渲染器未初始化" << std::endl;
        return nullptr;
    }
    
    // 检查缓存
    SDL_Texture* cached = getCachedTexture(path);
    if (cached) {
        return cached;
    }
    
    // 加载图片
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "无法加载图片: " << path << " - " << IMG_GetError() << std::endl;
        return nullptr;
    }
    
    // 处理BMP格式的透明通道（#FF00FD，RGB(255, 0, 253)）
    std::string lowerPath = path;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
    if (lowerPath.length() >= 4 && lowerPath.substr(lowerPath.length() - 4) == ".bmp") {
        // 检查表面格式，如果不是RGBA格式，需要转换
        if (surface->format->format != SDL_PIXELFORMAT_RGBA8888) {
            SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
            if (converted) {
                SDL_FreeSurface(surface);
                surface = converted;
            }
        }
        
        // 锁定表面以访问像素数据
        if (SDL_LockSurface(surface) == 0) {
            Uint32* pixels = (Uint32*)surface->pixels;
            int pitch = surface->pitch / 4;  // 每行的像素数（32位RGBA）
            
            // 遍历所有像素，将#FF00FD颜色设为透明
            // #FF00FD = RGB(255, 0, 253)
            for (int y = 0; y < surface->h; y++) {
                for (int x = 0; x < surface->w; x++) {
                    Uint32 pixel = pixels[y * pitch + x];
                    // 提取RGB值
                    Uint8 r, g, b, a;
                    SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
                    
                    // 检测#FF00FD（RGB(255, 0, 253)）透明色
                    // 精确匹配：R=255, G=0, B=253，允许小的误差
                    if (r >= 250 && r <= 255 && 
                        g >= 0 && g <= 5 && 
                        b >= 250 && b <= 255) {
                        // 设置为完全透明
                        pixels[y * pitch + x] = SDL_MapRGBA(surface->format, r, g, b, 0);
                    }
                }
            }
            
            SDL_UnlockSurface(surface);
        }
    }
    
    // 转换为纹理
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        std::cerr << "无法创建纹理: " << path << " - " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    // 缓存纹理
    cacheTexture(path, texture);
    
    return texture;
}

SDL_Texture* ResourceManager::loadImageFromTheme(const std::string& relativePath) {
    // 优先使用Classic DS Menu主题资源（3ds/light）
    // 尝试多个扩展名
    const char* extensions[] = {".png", ".bmp", ".grf", ""};
    
    // 首先尝试从当前主题路径加载（默认是3ds/light）
    for (int i = 0; extensions[i][0] != '\0'; i++) {
        std::string fullPath = themeBasePath + "/" + relativePath + extensions[i];
        SDL_Texture* tex = loadImage(fullPath);
        if (tex) {
            return tex;
        }
    }
    
    // 如果主题路径失败，尝试resources目录的Classic DS Menu主题（3ds/light）
    std::string resourcesPath = "../romsel_dsimenutheme/resources/dsimenu_theme_examples/3ds/light/" + relativePath;
    for (int i = 0; extensions[i][0] != '\0'; i++) {
        std::string fullPath = resourcesPath + extensions[i];
        SDL_Texture* tex = loadImage(fullPath);
        if (tex) {
            return tex;
        }
    }
    
    // 最后尝试grit目录（PNG源文件）
    std::string gritPath = "../romsel_dsimenutheme/resources/dsimenu_theme_examples/3ds/light/grit/" + relativePath;
    for (int i = 0; extensions[i][0] != '\0'; i++) {
        std::string fullPath = gritPath + extensions[i];
        SDL_Texture* tex = loadImage(fullPath);
        if (tex) {
            return tex;
        }
    }
    
    return nullptr;
}

std::string ResourceManager::getThemePath() {
    return themeBasePath;
}

void ResourceManager::setThemePath(const std::string& path) {
    themeBasePath = path;
    clearCache(); // 清除缓存，因为主题改变了
}

void ResourceManager::preloadCommonResources() {
    // 预加载常用资源（尝试多种格式）- Classic DS Menu主题
    loadImageFromTheme("background/top");
    loadImageFromTheme("background/bottom_bubble");
    loadImageFromTheme("background/bottom");
    loadImageFromTheme("grf/folder");
    loadImageFromTheme("grf/box_empty");
    loadImageFromTheme("grf/box_full");
}

void ResourceManager::clearCache() {
    for (auto& pair : textureCache) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    textureCache.clear();
}

SDL_Texture* ResourceManager::getCachedTexture(const std::string& key) {
    auto it = textureCache.find(key);
    if (it != textureCache.end()) {
        return it->second;
    }
    return nullptr;
}

void ResourceManager::cacheTexture(const std::string& key, SDL_Texture* texture) {
    textureCache[key] = texture;
}

