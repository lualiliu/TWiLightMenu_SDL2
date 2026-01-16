#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <map>
#include <cstdint>

// 类型定义
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// NDS文件头结构（简化版）
struct NDSHeader {
    char gameTitle[12];
    char gameCode[4];
    u8 reserved[0x9C];
    u32 bannerOffset;  // Banner数据偏移
};

// NDS Banner结构（包含图标和标题）
struct NDSBanner {
    u16 version;
    u16 crc[4];
    u8 reserved[22];
    u8 icon[512];      // 32x32图标，4位每像素
    u16 palette[16];   // 16色调色板
    u16 titles[8][128]; // 8种语言的标题
};

// NDS图标加载器
class NDSIconLoader {
public:
    static void init(SDL_Renderer* renderer);
    static void cleanup();
    
    // 从NDS文件加载图标
    static SDL_Texture* loadIconFromNDS(const std::string& filePath);
    
    // 从NDS文件读取标题（UTF-16转UTF-8）
    static std::string loadTitleFromNDS(const std::string& filePath, int langIndex = 1);
    
    // 清除缓存
    static void clearCache();
    
private:
    static SDL_Renderer* renderer;
    static std::map<std::string, SDL_Texture*> iconCache;
    
    // 将4位图标数据转换为RGBA纹理
    static SDL_Texture* convertIconToTexture(const u8* iconData, const u16* palette);
    
    // 读取NDS文件头
    static bool readNDSHeader(FILE* fp, NDSHeader& header);
    
    // 读取Banner数据
    static bool readBanner(FILE* fp, u32 bannerOffset, NDSBanner& banner);
};

