#include "sound.h"
#include <SDL2/SDL_mixer.h>
#include <iostream>

static Mix_Music* backgroundMusic = nullptr;

bool soundInit() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "音频初始化失败: " << Mix_GetError() << std::endl;
        return false;
    }
    
    // 初始化MP3支持
    int flags = MIX_INIT_MP3;
    int initialized = Mix_Init(flags);
    if ((initialized & flags) != flags) {
        std::cerr << "MP3初始化失败: " << Mix_GetError() << std::endl;
        // 继续运行，可能不支持MP3但可以播放其他格式
    }
    
    return true;
}

void soundCleanup() {
    if (backgroundMusic) {
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }
    Mix_CloseAudio();
    Mix_Quit();
}

bool playBackgroundMusic(const char* filePath) {
    // 如果已有背景音乐在播放，先停止
    if (backgroundMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }
    
    // 加载背景音乐
    backgroundMusic = Mix_LoadMUS(filePath);
    if (!backgroundMusic) {
        std::cerr << "无法加载背景音乐: " << filePath << " - " << Mix_GetError() << std::endl;
        return false;
    }
    
    // 循环播放背景音乐
    if (Mix_PlayMusic(backgroundMusic, -1) == -1) {
        std::cerr << "无法播放背景音乐: " << Mix_GetError() << std::endl;
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
        return false;
    }
    
    std::cout << "背景音乐已开始播放: " << filePath << std::endl;
    return true;
}

void stopBackgroundMusic() {
    if (backgroundMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }
}

