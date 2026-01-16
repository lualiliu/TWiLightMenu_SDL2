#include "menu.h"
#include "graphics/textRenderer.h"
#include "graphics/graphics.h"
#include <algorithm>

Menu::Menu() : selectedIndex(0), active(false), menuX(10), menuY(50), itemHeight(20), menuWidth(200) {
}

Menu::~Menu() {
}

void Menu::addItem(const std::string& text, int id, bool enabled) {
    items.push_back(MenuItem(text, id, enabled));
}

void Menu::clear() {
    items.clear();
    selectedIndex = 0;
}

void Menu::update() {
    if (!active) return;
    
    handleInput();
}

void Menu::handleInput() {
    if (InputManager::isKeyDown(KEY_UP)) {
        selectedIndex--;
        if (selectedIndex < 0) {
            selectedIndex = items.size() - 1;
        }
        // 跳过禁用的项目
        while (!items[selectedIndex].enabled && items.size() > 1) {
            selectedIndex--;
            if (selectedIndex < 0) {
                selectedIndex = items.size() - 1;
            }
        }
    }
    
    if (InputManager::isKeyDown(KEY_DOWN)) {
        selectedIndex++;
        if (selectedIndex >= (int)items.size()) {
            selectedIndex = 0;
        }
        // 跳过禁用的项目
        while (!items[selectedIndex].enabled && items.size() > 1) {
            selectedIndex++;
            if (selectedIndex >= (int)items.size()) {
                selectedIndex = 0;
            }
        }
    }
}

void Menu::render() {
    if (!active || items.empty()) return;
    
    extern SDL_Renderer* g_renderer;
    if (!g_renderer) return;
    
    // 计算菜单宽度（根据最长文本）
    int maxWidth = 180;
    for (const auto& item : items) {
        int width = TextRenderer::getTextWidth(item.text, 12) + 30;
        if (width > maxWidth) {
            maxWidth = width;
        }
    }
    menuWidth = maxWidth;  // 保存宽度供其他函数使用
    
    // 绘制菜单背景
    SDL_SetRenderDrawColor(g_renderer, 40, 40, 40, 240);
    SDL_Rect bgRect = {menuX - 5, menuY - 5, maxWidth + 10, (int)(items.size() * itemHeight + 30)};
    SDL_RenderFillRect(g_renderer, &bgRect);
    
    // 绘制边框
    SDL_SetRenderDrawColor(g_renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(g_renderer, &bgRect);
    
    // 绘制标题栏
    SDL_SetRenderDrawColor(g_renderer, 50, 50, 50, 255);
    SDL_Rect titleRect = {menuX - 5, menuY - 5, maxWidth + 10, 20};
    SDL_RenderFillRect(g_renderer, &titleRect);
    
    SDL_Color titleColor = {255, 255, 255, 255};
    TextRenderer::drawTextCentered(menuX - 5, menuY - 2, maxWidth + 10, "Menu", titleColor, 12);
    
    // 绘制菜单项（从标题栏下方开始）
    int menuItemStartY = menuY + 20;
    for (size_t i = 0; i < items.size(); i++) {
        int y = menuItemStartY + i * itemHeight;
        
        // 高亮选中的项目
        if ((int)i == selectedIndex) {
            SDL_SetRenderDrawColor(g_renderer, 60, 60, 120, 255);
            SDL_Rect highlightRect = {menuX - 3, y - 2, maxWidth + 6, itemHeight - 2};
            SDL_RenderFillRect(g_renderer, &highlightRect);
        }
        
        // 绘制文本
        SDL_Color textColor;
        if (!items[i].enabled) {
            textColor = {100, 100, 100, 255};
        } else if ((int)i == selectedIndex) {
            textColor = {255, 255, 255, 255};
        } else {
            textColor = {200, 200, 200, 255};
        }
        
        std::string displayText = items[i].text;
        if ((int)i == selectedIndex) {
            displayText = "  " + displayText;  // 使用Unicode箭头符号
        } else {
            displayText = "  " + displayText;
        }
        
        TextRenderer::drawText(menuX, y, displayText, textColor, 12);
    }
}

int Menu::getSelectedId() const {
    if (selectedIndex >= 0 && selectedIndex < (int)items.size()) {
        return items[selectedIndex].id;
    }
    return -1;
}

int Menu::getSelectedIndex() const {
    return selectedIndex;
}

void Menu::setSelectedIndex(int index) {
    if (index >= 0 && index < (int)items.size()) {
        selectedIndex = index;
    }
}

