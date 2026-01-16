#include "graphics.h"
#include "textRenderer.h"
#include "../dsiUI.h"
#include "../gameGrid.h"
#include "../fileBrowser.h"
#include "../input.h"
extern FileBrowser* g_fileBrowser;
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <sstream>
#include <iomanip>

// 全局渲染器
SDL_Renderer* g_renderer = nullptr;

// 当前颜色
static RGB15 currentColor(31, 31, 31);

// 屏幕亮度
static int screenBrightness[2] = {0, 0};
static bool fadeType = false; // false = out, true = in
static bool invertedColors = false;

// 帧缓冲区 (用于模拟DS双屏)
static SDL_Texture* topScreenTexture = nullptr;
static SDL_Texture* bottomScreenTexture = nullptr;

bool graphicsInit() {
    if (!g_renderer) {
        std::cerr << "渲染器未初始化" << std::endl;
        return false;
    }

    // 初始化文本渲染器
    TextRenderer::init(g_renderer);
    
    // 初始化DSi UI
    DSiUI::init(g_renderer);

    // 创建屏幕纹理
    topScreenTexture = SDL_CreateTexture(
        g_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        256,
        192
    );

    bottomScreenTexture = SDL_CreateTexture(
        g_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        256,
        192
    );

    if (!topScreenTexture || !bottomScreenTexture) {
        std::cerr << "屏幕纹理创建失败: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void graphicsCleanup() {
    DSiUI::cleanup();
    TextRenderer::cleanup();
    
    if (topScreenTexture) {
        SDL_DestroyTexture(topScreenTexture);
        topScreenTexture = nullptr;
    }
    if (bottomScreenTexture) {
        SDL_DestroyTexture(bottomScreenTexture);
        bottomScreenTexture = nullptr;
    }
}

void glBegin2D() {
    // SDL2不需要特殊的开始标记
    // 但我们可以设置渲染目标
}

void glEnd2D() {
    // SDL2不需要特殊的结束标记
}

void glColor(RGB15 color) {
    currentColor = color;
}

void glSprite(int x, int y, GL_FLIP flip, const glImage* img) {
    if (!img || !img->texture || !g_renderer) {
        return;
    }

    SDL_Rect srcRect = {
        img->u_off,
        img->v_off,
        img->width,
        img->height
    };

    SDL_Rect dstRect = {
        x,
        y,
        img->width,
        img->height
    };

    SDL_RendererFlip sdlFlip = SDL_FLIP_NONE;
    if (flip & GL_FLIP_H) {
        sdlFlip = (SDL_RendererFlip)(sdlFlip | SDL_FLIP_HORIZONTAL);
    }
    if (flip & GL_FLIP_V) {
        sdlFlip = (SDL_RendererFlip)(sdlFlip | SDL_FLIP_VERTICAL);
    }

    // 设置颜色调制 (用于亮度调整)
    int brightness = screenBrightness[0] / 8;
    if (brightness < 31) {
        Uint8 r = (currentColor.r() * brightness) / 31;
        Uint8 g = (currentColor.g() * brightness) / 31;
        Uint8 b = (currentColor.b() * brightness) / 31;
        SDL_SetTextureColorMod(img->texture, r * 8, g * 8, b * 8);
    } else {
        SDL_SetTextureColorMod(img->texture, 255, 255, 255);
    }

    SDL_RenderCopyEx(g_renderer, img->texture, &srcRect, &dstRect, 0.0, nullptr, sdlFlip);
}

void glBoxFilled(int x1, int y1, int x2, int y2, RGB15 color) {
    if (!g_renderer) {
        return;
    }

    // 确保坐标正确
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);

    SDL_Rect rect = {x1, y1, x2 - x1, y2 - y1};

    // 应用亮度
    int brightness = screenBrightness[0] / 8;
    Uint8 r = (color.r() * brightness) / 31;
    Uint8 g = (color.g() * brightness) / 31;
    Uint8 b = (color.b() * brightness) / 31;

    SDL_SetRenderDrawColor(g_renderer, r * 8, g * 8, b * 8, 255);
    SDL_RenderFillRect(g_renderer, &rect);
}

SDL_Texture* loadTexture(const std::string& path) {
    if (!g_renderer) {
        return nullptr;
    }

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "无法加载图像: " << path << " - " << IMG_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        std::cerr << "无法创建纹理: " << path << " - " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return texture;
}

SDL_Texture* createTextureFromRGB15(uint16_t* data, int width, int height) {
    if (!g_renderer || !data) {
        return nullptr;
    }

    // 创建RGBA表面
    SDL_Surface* surface = SDL_CreateRGBSurface(
        0,
        width,
        height,
        32,
        0x000000FF,
        0x0000FF00,
        0x00FF0000,
        0xFF000000
    );

    if (!surface) {
        return nullptr;
    }

    // 转换RGB15到RGBA8888
    Uint32* pixels = (Uint32*)surface->pixels;
    for (int i = 0; i < width * height; i++) {
        uint16_t rgb15 = data[i];
        int r = (rgb15 & 31) << 3;
        int g = ((rgb15 >> 5) & 31) << 3;
        int b = ((rgb15 >> 10) & 31) << 3;
        pixels[i] = (0xFF << 24) | (b << 16) | (g << 8) | r;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, surface);
    SDL_FreeSurface(surface);

    return texture;
}

void updateFrame(bool updateFrame) {
    // 更新游戏逻辑
    // 这里可以添加动画、输入处理等
    
    // 更新日期和时间（每秒更新一次）
    static Uint32 lastTimeUpdate = 0;
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastTimeUpdate > 1000) {
        lastTimeUpdate = currentTime;
        // 日期和时间会在renderFrame中重新绘制
    }
}

void renderFrame() {
    if (!g_renderer) {
        return;
    }

    // 清除屏幕
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);

    // 交换上下屏：上屏显示UI元素，下屏显示游戏网格
    // 绘制上屏背景（现在用于UI元素）
    DSiUI::drawTopBackground();
    
    // 绘制下屏背景（现在用于游戏网格）
    DSiUI::drawBottomBackground(true);
    
    // 绘制分隔线
    SDL_SetRenderDrawColor(g_renderer, 100, 100, 100, 255);
    SDL_RenderDrawLine(g_renderer, 0, 192, 256, 192);

    // 绘制DSi风格的UI元素（现在在上屏）
    DSiUI::drawBatteryIcon(4, false);  // 电池图标（满电）
    DSiUI::drawVolumeIcon(3);          // 音量图标
    DSiUI::drawDSiDateTime();          // 日期时间
    // L/R按钮将在后面根据实际按键状态绘制
    
    // 获取文件列表（如果文件浏览器已初始化）
    const std::vector<FileEntry>* fileList = nullptr;
    int totalItems = 0;
    if (g_fileBrowser) {
        fileList = &g_fileBrowser->getFiles();
        totalItems = fileList->size();
    } else {
        totalItems = 10;  // 默认占位符数量
    }
    
    // 更新GameGrid的最大项目数
    GameGrid::setMaxItems(totalItems);
    
    // 绘制游戏网格（现在在下屏）
    int selectedGame = GameGrid::getSelectedIndex();
    int scrollOffset = GameGrid::getScrollOffset();
    
    // 检查L/R键是否被按下
    bool leftActive = InputManager::isKeyHeld(KEY_L);
    bool rightActive = InputManager::isKeyHeld(KEY_R);
    
    DSiUI::drawGameGrid(selectedGame, scrollOffset, fileList);
    //DSiUI::drawStartBorder(true);
    
    // 更新L/R按钮显示状态
    DSiUI::drawShoulderButtons(leftActive, rightActive);
    
    // 绘制提示文本（下屏底部：y坐标+192）
    SDL_Color hintColor = {0, 0, 0, 255};
    TextRenderer::drawTextCentered(0, 250 + 192, 256, "Press START to open menu", hintColor, 12);
    TextRenderer::drawTextCentered(0, 265 + 192, 256, "Press ESC to exit", hintColor, 12);
}

bool screenFadedIn() {
    return (screenBrightness[0] == 0 && screenBrightness[1] == 0);
}

bool screenFadedOut() {
    return (screenBrightness[0] > 24 || screenBrightness[1] > 24);
}

void SetBrightness(int screen, int bright) {
    if (screen < 0 || screen > 1) {
        return;
    }

    if (bright < -31) bright = -31;
    if (bright > 31) bright = 31;

    screenBrightness[screen] = abs(bright);
    fadeType = (bright >= 0);
}

void drawCurrentDate() {
    if (!g_renderer) {
        return;
    }

    // 获取当前日期
    time_t rawTime;
    struct tm* timeInfo;
    char dateStr[32];
    
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    
    // 格式化日期: YYYY/MM/DD
    strftime(dateStr, sizeof(dateStr), "%Y/%m/%d", timeInfo);
    
    // 绘制日期背景
    SDL_SetRenderDrawColor(g_renderer, 30, 30, 30, 255);
    SDL_Rect dateRect = {180, 200, 70, 12};
    SDL_RenderFillRect(g_renderer, &dateRect);
    
    // 绘制日期文本
    SDL_Color textColor = {200, 200, 200, 255};
    TextRenderer::drawText(182, 202, dateStr, textColor);
}

void drawCurrentTime() {
    if (!g_renderer) {
        return;
    }

    // 获取当前时间
    time_t rawTime;
    struct tm* timeInfo;
    char timeStr[32];
    
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    
    // 格式化时间: HH:MM
    strftime(timeStr, sizeof(timeStr), "%H:%M", timeInfo);
    
    // 绘制时间背景
    SDL_SetRenderDrawColor(g_renderer, 30, 30, 30, 255);
    SDL_Rect timeRect = {180, 215, 70, 12};
    SDL_RenderFillRect(g_renderer, &timeRect);
    
    // 绘制时间文本
    SDL_Color textColor = {200, 200, 200, 255};
    TextRenderer::drawText(182, 217, timeStr, textColor);
}

