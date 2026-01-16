#pragma once

#include <SDL2/SDL.h>

class FPSCounter {
public:
    static void init();
    static void update();
    static void render();
    static float getFPS();
    static float getAverageFPS();
    
private:
    static Uint32 lastTime;
    static Uint32 frameCount;
    static float currentFPS;
    static float averageFPS;
    static Uint32 fpsUpdateTime;
    static const int FPS_HISTORY_SIZE = 60;
    static float fpsHistory[FPS_HISTORY_SIZE];
    static int fpsHistoryIndex;
};

