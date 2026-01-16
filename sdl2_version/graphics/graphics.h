#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

// 前向声明
class SDL_Renderer;
class SDL_Texture;

// 颜色结构 (RGB15格式兼容)
struct RGB15 {
    uint16_t value;
    RGB15() : value(0) {}
    RGB15(uint16_t v) : value(v) {}
    RGB15(int r, int g, int b) {
        value = (r & 31) | ((g & 31) << 5) | ((b & 31) << 10);
    }
    int r() const { return value & 31; }
    int g() const { return (value >> 5) & 31; }
    int b() const { return (value >> 10) & 31; }
};

// RGB15辅助函数
inline RGB15 makeRGB15(int r, int g, int b) {
    return RGB15(r, g, b);
}

// RGB15宏定义 (兼容原代码，使用不同的名称避免冲突)
#define RGB15(r, g, b) makeRGB15((r), (g), (b))

// glImage兼容结构
struct glImage {
    uint16_t* imageData;
    uint16_t width;
    uint16_t height;
    uint16_t texWidth;
    uint16_t texHeight;
    uint16_t u_off;
    uint16_t v_off;
    SDL_Texture* texture; // SDL2纹理
};

// GL翻转标志
enum GL_FLIP {
    GL_FLIP_NONE = 0,
    GL_FLIP_H = 1,
    GL_FLIP_V = 2
};

// 全局渲染器
extern SDL_Renderer* g_renderer;

// 初始化图形系统
bool graphicsInit();

// 清理图形系统
void graphicsCleanup();

// 开始2D渲染
void glBegin2D();

// 结束2D渲染
void glEnd2D();

// 设置当前颜色
void glColor(RGB15 color);

// 绘制精灵
void glSprite(int x, int y, GL_FLIP flip, const glImage* img);

// 绘制填充矩形
void glBoxFilled(int x1, int y1, int x2, int y2, RGB15 color);

// 加载纹理
SDL_Texture* loadTexture(const std::string& path);

// 从RGB15数据创建纹理
SDL_Texture* createTextureFromRGB15(uint16_t* data, int width, int height);

// 更新帧
void updateFrame(bool updateFrame);

// 渲染帧
void renderFrame();

// 屏幕淡入淡出
bool screenFadedIn();
bool screenFadedOut();
void SetBrightness(int screen, int bright);

// 绘制当前日期
void drawCurrentDate();

// 绘制当前时间
void drawCurrentTime();

