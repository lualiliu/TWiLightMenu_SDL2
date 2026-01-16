#include "textRenderer.h"
#include <iostream>
#include <cstring>

SDL_Renderer* TextRenderer::renderer = nullptr;
TTF_Font* TextRenderer::smallFont = nullptr;
TTF_Font* TextRenderer::mediumFont = nullptr;
TTF_Font* TextRenderer::largeFont = nullptr;
bool TextRenderer::fontsLoaded = false;

void TextRenderer::init(SDL_Renderer* renderer) {
    TextRenderer::renderer = renderer;
    
    // 初始化SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init失败: " << TTF_GetError() << std::endl;
        // 如果TTF初始化失败，继续使用简单渲染
        return;
    }
    
    loadFonts();
}

void TextRenderer::loadFonts() {
    if (fontsLoaded) return;
    
    // 尝试加载支持中文的系统字体
    const char* fontPaths[] = {
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",  // Noto CJK字体（支持中文）
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",        // 文泉驿微米黑
        "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",          // 文泉驿正黑
        "/usr/share/fonts/truetype/arphic/uming.ttc",             // AR PL UMing
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",        // DejaVu（不支持中文但作为备用）
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/System/Library/Fonts/PingFang.ttc",                    // macOS中文字体
        "/System/Library/Fonts/STHeiti Light.ttc",               // macOS中文字体
        "/System/Library/Fonts/Helvetica.ttc",
        "C:/Windows/Fonts/msyh.ttc",                             // Windows微软雅黑
        "C:/Windows/Fonts/simsun.ttc",                           // Windows宋体
        "C:/Windows/Fonts/arial.ttf",
        nullptr
    };
    
    const char* fontPath = nullptr;
    for (int i = 0; fontPaths[i] != nullptr; i++) {
        FILE* f = fopen(fontPaths[i], "r");
        if (f) {
            fclose(f);
            fontPath = fontPaths[i];
            break;
        }
    }
    
    if (!fontPath) {
        std::cerr << "警告: 未找到系统字体，使用内置简单字体" << std::endl;
        // 如果找不到字体，使用简单渲染
        return;
    }
    
    // 加载不同大小的字体（增大字体以提高清晰度）
    smallFont = TTF_OpenFont(fontPath, 12);
    mediumFont = TTF_OpenFont(fontPath, 16);
    largeFont = TTF_OpenFont(fontPath, 20);
    
    if (!smallFont) smallFont = mediumFont;
    if (!mediumFont) mediumFont = largeFont;
    if (!largeFont) {
        std::cerr << "警告: 无法加载字体，使用简单渲染" << std::endl;
        return;
    }
    
    fontsLoaded = true;
}

void TextRenderer::freeFonts() {
    if (smallFont && smallFont != mediumFont && smallFont != largeFont) {
        TTF_CloseFont(smallFont);
    }
    if (mediumFont && mediumFont != largeFont) {
        TTF_CloseFont(mediumFont);
    }
    if (largeFont) {
        TTF_CloseFont(largeFont);
    }
    smallFont = nullptr;
    mediumFont = nullptr;
    largeFont = nullptr;
    fontsLoaded = false;
}

void TextRenderer::cleanup() {
    freeFonts();
    TTF_Quit();
    TextRenderer::renderer = nullptr;
}

TTF_Font* TextRenderer::getFont(int size) {
    if (!fontsLoaded) return nullptr;
    
    if (size <= 12) {
        return smallFont ? smallFont : mediumFont;
    } else if (size <= 16) {
        return mediumFont ? mediumFont : largeFont;
    } else {
        return largeFont;
    }
}

void TextRenderer::drawText(int x, int y, const std::string& text, SDL_Color color, int fontSize) {
    if (!renderer) return;
    
    // 如果字体未加载，使用简单渲染
    TTF_Font* font = getFont(fontSize);
    if (!font) {
        // 简单的ASCII字符渲染（备用方案）
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        int currentX = x;
        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i];
            if (c >= '0' && c <= '9') {
                // 简单的数字显示
                SDL_Rect rect = {currentX, y, 6, 8};
                SDL_RenderFillRect(renderer, &rect);
            } else if (c >= 'A' && c <= 'Z') {
                SDL_Rect rect = {currentX, y, 6, 8};
                SDL_RenderDrawRect(renderer, &rect);
            } else if (c >= 'a' && c <= 'z') {
                SDL_Rect rect = {currentX, y, 6, 8};
                SDL_RenderDrawRect(renderer, &rect);
            } else if (c == ' ') {
                // 空格
            } else if (c == ':') {
                SDL_Rect r1 = {currentX + 2, y + 2, 2, 2};
                SDL_Rect r2 = {currentX + 2, y + 5, 2, 2};
                SDL_RenderFillRect(renderer, &r1);
                SDL_RenderFillRect(renderer, &r2);
            }
            currentX += 8;
        }
        return;
    }
    
    // 使用TTF渲染文本（UTF-8支持，使用Blended模式以获得更好的清晰度）
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!textSurface) {
        // 如果UTF-8渲染失败，尝试使用Latin1
        textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
        if (!textSurface) {
            // 如果Blended失败，回退到Solid模式
            textSurface = TTF_RenderUTF8_Solid(font, text.c_str(), color);
            if (!textSurface) {
                textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
                if (!textSurface) {
                    std::cerr << "无法渲染文本: " << TTF_GetError() << std::endl;
                    return;
                }
            }
        }
    }
    
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        SDL_FreeSurface(textSurface);
        return;
    }
    
    SDL_Rect destRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, nullptr, &destRect);
    
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

void TextRenderer::drawTextCentered(int x, int y, int width, const std::string& text, SDL_Color color, int fontSize) {
    int textWidth = getTextWidth(text, fontSize);
    int startX = x + (width - textWidth) / 2;
    drawText(startX, y, text, color, fontSize);
}

int TextRenderer::getTextWidth(const std::string& text, int fontSize) {
    TTF_Font* font = getFont(fontSize);
    if (!font) {
        // 简单估算（UTF-8字符可能占用多个字节）
        int charCount = 0;
        for (size_t i = 0; i < text.length(); ) {
            unsigned char c = text[i];
            if ((c & 0x80) == 0) {
                i++;
                charCount++;
            } else if ((c & 0xE0) == 0xC0) {
                i += 2;
                charCount++;
            } else if ((c & 0xF0) == 0xE0) {
                i += 3;
                charCount++;
            } else if ((c & 0xF8) == 0xF0) {
                i += 4;
                charCount++;
            } else {
                i++;
                charCount++;
            }
        }
        return charCount * 8;
    }
    
    int w, h;
    // 尝试UTF-8，如果失败则使用Latin1
    if (TTF_SizeUTF8(font, text.c_str(), &w, &h) != 0) {
        if (TTF_SizeText(font, text.c_str(), &w, &h) != 0) {
            return text.length() * 8;
        }
    }
    return w;
}

int TextRenderer::getTextHeight(int fontSize) {
    TTF_Font* font = getFont(fontSize);
    if (!font) {
        return 8;
    }
    return TTF_FontHeight(font);
}
