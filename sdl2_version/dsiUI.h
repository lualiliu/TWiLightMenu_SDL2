#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>

// 前向声明
struct FileEntry;

// Classic DS Menu风格UI管理器（使用3DS Light主题的DS风格资源）
class DSiUI {
public:
    static void init(SDL_Renderer* renderer);
    static void cleanup();
    
    // 绘制Classic DS Menu风格背景（使用DS风格的背景文件）
    static void drawTopBackground();
    static void drawBottomBackground(bool showBubble = false);
    
    // 设置壁纸
    static void setTopWallpaper(const std::string& path);
    static void setBottomWallpaper(const std::string& path);
    
    // 绘制UI元素
    static void drawBatteryIcon(int level, bool charging = false);
    static void drawVolumeIcon(int level);
    static void drawShoulderButtons(bool leftActive, bool rightActive);
    
    // 绘制游戏图标网格（支持文件列表）
    static void drawGameGrid(int selectedIndex, int scrollOffset, const std::vector<struct FileEntry>* files = nullptr);
    
    // 绘制日期时间
    static void drawDSiDateTime();
    
    // 绘制START边框
    static void drawStartBorder(bool visible);
    
    // 检查文件是否为NDS文件
    static bool isNDSFile(const std::string& filename);
    
private:
    static SDL_Renderer* renderer;
    static SDL_Texture* topBgTexture;
    static SDL_Texture* bottomBgTexture;
    static SDL_Texture* bottomBubbleTexture;
    static SDL_Texture* batteryTextures[5];
    static SDL_Texture* volumeTextures[5];
    static SDL_Texture* folderTexture;
    static SDL_Texture* ndsFileTexture;
    static SDL_Texture* boxEmptyTexture;
    static SDL_Texture* boxFullTexture;
    
    static void loadTextures();
    static void freeTextures();
};

