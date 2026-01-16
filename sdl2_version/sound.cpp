#include "sound.h"
#include <SDL2/SDL_mixer.h>
#include <iostream>

bool soundInit() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "音频初始化失败: " << Mix_GetError() << std::endl;
        return false;
    }
    return true;
}

void soundCleanup() {
    Mix_CloseAudio();
}

