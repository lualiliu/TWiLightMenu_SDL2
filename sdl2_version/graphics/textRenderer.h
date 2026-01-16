#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

// 文本渲染器（使用SDL2_ttf）
class TextRenderer {
public:
    static void init(SDL_Renderer* renderer);
    static void cleanup();
    static void drawText(int x, int y, const std::string& text, SDL_Color color, int fontSize = 12);
    static void drawTextCentered(int x, int y, int width, const std::string& text, SDL_Color color, int fontSize = 12);
    static int getTextWidth(const std::string& text, int fontSize = 12);
    static int getTextHeight(int fontSize = 12);
    
private:
    static SDL_Renderer* renderer;
    static TTF_Font* getFont(int size);
    static TTF_Font* smallFont;
    static TTF_Font* mediumFont;
    static TTF_Font* largeFont;
    static bool fontsLoaded;
    static void loadFonts();
    static void freeFonts();
};

