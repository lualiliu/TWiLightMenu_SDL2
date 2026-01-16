#include "fpsCounter.h"
#include "graphics/textRenderer.h"
#include "graphics/graphics.h"
#include <SDL2/SDL.h>
#include <sstream>
#include <iomanip>
#include <cmath>

// 前向声明
extern SDL_Renderer* g_renderer;

Uint32 FPSCounter::lastTime = 0;
Uint32 FPSCounter::frameCount = 0;
float FPSCounter::currentFPS = 0.0f;
float FPSCounter::averageFPS = 0.0f;
Uint32 FPSCounter::fpsUpdateTime = 0;
float FPSCounter::fpsHistory[FPS_HISTORY_SIZE] = {0};
int FPSCounter::fpsHistoryIndex = 0;

void FPSCounter::init() {
    lastTime = SDL_GetTicks();
    frameCount = 0;
    currentFPS = 0.0f;
    averageFPS = 0.0f;
    fpsUpdateTime = SDL_GetTicks();
    for (int i = 0; i < FPS_HISTORY_SIZE; i++) {
        fpsHistory[i] = 0.0f;
    }
    fpsHistoryIndex = 0;
}

void FPSCounter::update() {
    frameCount++;
    Uint32 currentTime = SDL_GetTicks();
    
    // 每秒更新一次FPS
    if (currentTime - fpsUpdateTime >= 1000) {
        currentFPS = frameCount * 1000.0f / (currentTime - fpsUpdateTime);
        
        // 更新FPS历史
        fpsHistory[fpsHistoryIndex] = currentFPS;
        fpsHistoryIndex = (fpsHistoryIndex + 1) % FPS_HISTORY_SIZE;
        
        // 计算平均FPS
        float sum = 0.0f;
        int count = 0;
        for (int i = 0; i < FPS_HISTORY_SIZE; i++) {
            if (fpsHistory[i] > 0) {
                sum += fpsHistory[i];
                count++;
            }
        }
        if (count > 0) {
            averageFPS = sum / count;
        }
        
        frameCount = 0;
        fpsUpdateTime = currentTime;
    }
}

void FPSCounter::render() {
    if (!g_renderer) return;
    
    // 绘制FPS信息
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << "FPS: " << currentFPS;
    if (averageFPS > 0) {
        oss << " (Avg: " << averageFPS << ")";
    }
    
    SDL_Color fpsColor;
    if (currentFPS >= 55) {
        fpsColor = {0, 255, 0, 255}; // 绿色
    } else if (currentFPS >= 30) {
        fpsColor = {255, 255, 0, 255}; // 黄色
    } else {
        fpsColor = {255, 0, 0, 255}; // 红色
    }
    
    TextRenderer::drawText(10, 10, oss.str(), fpsColor);
}

float FPSCounter::getFPS() {
    return currentFPS;
}

float FPSCounter::getAverageFPS() {
    return averageFPS;
}

