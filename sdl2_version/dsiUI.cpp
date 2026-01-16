#include "dsiUI.h"
#include "resourceManager.h"
#include "graphics/textRenderer.h"
#include "input.h"
#include "fileBrowser.h"
#include "ndsIconLoader.h"
#include "settings.h"
#include "gameGrid.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cmath>

SDL_Renderer* DSiUI::renderer = nullptr;
SDL_Texture* DSiUI::topBgTexture = nullptr;
SDL_Texture* DSiUI::bottomBgTexture = nullptr;
SDL_Texture* DSiUI::bottomBubbleTexture = nullptr;
SDL_Texture* DSiUI::batteryTextures[5] = {nullptr};
SDL_Texture* DSiUI::volumeTextures[5] = {nullptr};
SDL_Texture* DSiUI::folderTexture = nullptr;
SDL_Texture* DSiUI::ndsFileTexture = nullptr;
SDL_Texture* DSiUI::boxEmptyTexture = nullptr;
SDL_Texture* DSiUI::boxFullTexture = nullptr;

void DSiUI::init(SDL_Renderer* renderer) {
    DSiUI::renderer = renderer;
    ResourceManager::init(renderer);
    NDSIconLoader::init(renderer);
    loadTextures();
}

void DSiUI::cleanup() {
    freeTextures();
    NDSIconLoader::cleanup();
    ResourceManager::cleanup();
    renderer = nullptr;
}

void DSiUI::loadTextures() {
    // 默认加载Classic DS Menu主题资源（3ds/light）
    // 优先使用DS风格的背景文件
    topBgTexture = ResourceManager::loadImageFromTheme("background/top");
    bottomBgTexture = ResourceManager::loadImageFromTheme("background/bottom_ds");  // Classic DS Menu风格背景
    bottomBubbleTexture = ResourceManager::loadImageFromTheme("background/bottom_bubble_ds");  // Classic DS Menu风格bubble背景
    
    // 如果DS风格背景加载失败，尝试普通背景作为备用
    if (!bottomBgTexture) {
        bottomBgTexture = ResourceManager::loadImageFromTheme("background/bottom");
    }
    if (!bottomBubbleTexture) {
        bottomBubbleTexture = ResourceManager::loadImageFromTheme("background/bottom_bubble_macro");
    }
    
    // 加载电池图标
    for (int i = 0; i < 5; i++) {
        std::ostringstream oss;
        oss << "battery/battery" << i;
        batteryTextures[i] = ResourceManager::loadImageFromTheme(oss.str());
    }
    
    // 加载音量图标
    for (int i = 0; i < 5; i++) {
        std::ostringstream oss;
        oss << "volume/volume" << i;
        volumeTextures[i] = ResourceManager::loadImageFromTheme(oss.str());
    }
    
    // 加载UI元素（默认使用Classic DS Menu主题资源）
    // 文件夹不使用图片，使用通用绘制图标
    folderTexture = nullptr;  // 不再加载文件夹图片
    ndsFileTexture = ResourceManager::loadImageFromTheme("grf/nds_file");
    boxEmptyTexture = ResourceManager::loadImageFromTheme("grf/box_empty");
    boxFullTexture = ResourceManager::loadImageFromTheme("grf/box_full");
    
    // 如果找不到NDS文件图标，尝试使用small_cart作为备用（Classic DS Menu主题）
    if (!ndsFileTexture) {
        ndsFileTexture = ResourceManager::loadImageFromTheme("grf/small_cart");
    }
    
    std::cout << "Classic DS Menu UI纹理加载完成" << std::endl;
    std::cout << "  上屏背景: " << (topBgTexture ? "✓" : "✗") << std::endl;
    std::cout << "  下屏背景: " << (bottomBgTexture ? "✓" : "✗") << std::endl;
    std::cout << "  文件夹图标: 使用通用绘制图标" << std::endl;
    std::cout << "  NDS文件图标: " << (ndsFileTexture ? "✓" : "✗") << std::endl;
}

void DSiUI::freeTextures() {
    // 注意：纹理由ResourceManager管理，这里不需要手动释放
    topBgTexture = nullptr;
    bottomBgTexture = nullptr;
    bottomBubbleTexture = nullptr;
    for (int i = 0; i < 5; i++) {
        batteryTextures[i] = nullptr;
        volumeTextures[i] = nullptr;
    }
    folderTexture = nullptr;
    ndsFileTexture = nullptr;
    boxEmptyTexture = nullptr;
    boxFullTexture = nullptr;
}

void DSiUI::drawTopBackground() {
    if (!renderer) return;
    
    // 如果设置了自定义上屏壁纸，优先使用
    if (!g_settings.topWallpaperPath.empty()) {
        SDL_Texture* customWallpaper = ResourceManager::loadImage(g_settings.topWallpaperPath);
        if (customWallpaper) {
            SDL_Rect destRect = {0, 0, 256, 192};
            SDL_RenderCopy(renderer, customWallpaper, nullptr, &destRect);
            return;
        }
    }
    
    if (topBgTexture) {
        SDL_Rect destRect = {0, 0, 256, 192};
        SDL_RenderCopy(renderer, topBgTexture, nullptr, &destRect);
    } else {
        // 备用：绘制纯色背景
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_Rect rect = {0, 0, 256, 192};
        SDL_RenderFillRect(renderer, &rect);
    }
}

void DSiUI::drawBottomBackground(bool showBubble) {
    if (!renderer) return;
    
    // 如果设置了自定义下屏壁纸，优先使用
    if (!g_settings.bottomWallpaperPath.empty()) {
        SDL_Texture* customWallpaper = ResourceManager::loadImage(g_settings.bottomWallpaperPath);
        if (customWallpaper) {
            SDL_Rect destRect = {0, 192, 256, 192};
            SDL_RenderCopy(renderer, customWallpaper, nullptr, &destRect);
            return;
        }
    }
    
    SDL_Texture* bgToUse = showBubble ? bottomBubbleTexture : bottomBgTexture;
    
    if (bgToUse) {
        SDL_Rect destRect = {0, 192, 256, 192};
        SDL_RenderCopy(renderer, bgToUse, nullptr, &destRect);
    } else {
        // 备用：绘制DSi风格的渐变背景
        for (int y = 0; y < 192; y++) {
            Uint8 color = 180 - (y * 30 / 192);
            SDL_SetRenderDrawColor(renderer, color, color, color, 255);
            SDL_RenderDrawLine(renderer, 0, 192 + y, 256, 192 + y);
        }
    }
}

void DSiUI::setTopWallpaper(const std::string& path) {
    g_settings.topWallpaperPath = path;
    // 纹理会在drawTopBackground中动态加载，不需要在这里预加载
}

void DSiUI::setBottomWallpaper(const std::string& path) {
    g_settings.bottomWallpaperPath = path;
    // 纹理会在drawBottomBackground中动态加载，不需要在这里预加载
}

void DSiUI::drawBatteryIcon(int level, bool charging) {
    if (!renderer) return;
    
    // 限制level在0-4之间
    if (level < 0) level = 0;
    if (level > 4) level = 4;
    
    // 上屏位置（UI元素在上屏）
    SDL_Texture* batteryTex = batteryTextures[level];
    if (batteryTex) {
        SDL_Rect destRect = {230, 8, 20, 12};
        SDL_RenderCopy(renderer, batteryTex, nullptr, &destRect);
    } else {
        // 备用：简单绘制电池图标
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect batteryRect = {230, 8, 18, 10};
        SDL_RenderDrawRect(renderer, &batteryRect);
        SDL_Rect batteryTip = {248, 10, 2, 6};
        SDL_RenderFillRect(renderer, &batteryTip);
        
        // 绘制电量
        if (level > 0) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect chargeRect = {232, 10, level * 3, 6};
            SDL_RenderFillRect(renderer, &chargeRect);
        }
    }
}

void DSiUI::drawVolumeIcon(int level) {
    if (!renderer) return;
    
    // 限制level在0-4之间
    if (level < 0) level = 0;
    if (level > 4) level = 4;
    
    // 上屏位置（UI元素在上屏）
    SDL_Texture* volumeTex = volumeTextures[level];
    if (volumeTex) {
        SDL_Rect destRect = {5, 8, 20, 12};
        SDL_RenderCopy(renderer, volumeTex, nullptr, &destRect);
    } else {
        // 备用：简单绘制音量图标
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // 绘制音量波形
        for (int i = 0; i < level; i++) {
            SDL_Rect bar = {232 + i * 4, 25 + (3 - i), 2, i + 1};
            SDL_RenderFillRect(renderer, &bar);
        }
    }
}

void DSiUI::drawShoulderButtons(bool leftActive, bool rightActive) {
    if (!renderer) return;
    
    // 绘制L/R肩键提示（上屏位置，UI元素在上屏）
    SDL_Color colorL = leftActive ? SDL_Color{255, 255, 255, 255} : SDL_Color{150, 150, 150, 255};
    SDL_Color colorR = rightActive ? SDL_Color{255, 255, 255, 255} : SDL_Color{150, 150, 150, 255};
    
    TextRenderer::drawText(10, 170, "L  Previous", colorL, 12);
    TextRenderer::drawText(205, 170, "Next  R", colorR, 12);

    TextRenderer::drawText(35, 8, "RG DS", SDL_Color{150, 150, 150, 255}, 12); //user name
}

bool DSiUI::isNDSFile(const std::string& filename) {
    // 转换为小写进行比较
    std::string lowerName = filename;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    // NDS文件扩展名列表
    const char* ndsExtensions[] = {".nds", ".dsi", ".ids", ".srl", ".app", ".argv", nullptr};
    
    for (int i = 0; ndsExtensions[i] != nullptr; i++) {
        if (lowerName.length() >= strlen(ndsExtensions[i])) {
            std::string ext = lowerName.substr(lowerName.length() - strlen(ndsExtensions[i]));
            if (ext == ndsExtensions[i]) {
                return true;
            }
        }
    }
    return false;
}

void DSiUI::drawGameGrid(int selectedIndex, int scrollOffset, const std::vector<FileEntry>* files) {
    if (!renderer) return;
    
    // DSi风格的游戏图标网格（水平滚动，现在在下屏）
    const int iconSize = 48;
    const int iconSpacing = 58;
    const int startX = -10;  // 第一项起始坐标为-20
    const int startY = 90 + 192;  // 下屏位置：y坐标+192，向下移动30像素
    const int visibleIcons = 5;
    
    // 确定要显示的文件数量
    int totalItems = files ? files->size() : 10;  // 如果没有文件列表，显示10个占位符
    
    // 绘制选中文件的标题（在菜单上方居中显示）
    if (files && selectedIndex >= 0 && selectedIndex < (int)files->size()) {
        const FileEntry& selectedEntry = (*files)[selectedIndex];
        std::string displayName;
        // 优先使用NDS内部标题，如果没有则使用文件名
        if (!selectedEntry.title.empty()) {
            displayName = selectedEntry.title;
        } else if (!selectedEntry.name.empty()) {
            displayName = selectedEntry.name;
        }
        
        if (!displayName.empty()) {
            // 如果标题太长，截断
            if (displayName.length() > 30) {
                displayName = displayName.substr(0, 27) + "...";
            }
            SDL_Color textColor = {0, 0, 0, 255};
            // 在菜单上方居中显示（startY - 20位置）
            TextRenderer::drawTextCentered(0, startY - 20 - 30, 256, displayName, textColor, 12);
        }
    }
    
    // 获取动画后的滚动偏移（用于平滑移动效果）
    float animatedOffset = GameGrid::getAnimatedScrollOffset();
    
    // 绘制游戏框（DSi风格：水平排列）
    for (int i = 0; i < visibleIcons; i++) {
        // 使用动画偏移计算位置
        float posFloat = animatedOffset + i;
        int pos = (int)std::round(posFloat);
        
        // 跳过无效的项目索引（负数或超出范围）
        if (pos < 0 || pos >= totalItems) {
            // 即使pos无效，也继续循环以保持位置一致
            continue;
        }
        
        // 修复滚动计算：图标位置 = 起始位置 + 显示索引 * 间距
        // 使用浮点数偏移实现平滑动画
        // 计算目标位置（基于动画偏移）
        float targetX = startX + (animatedOffset + i - scrollOffset) * iconSpacing;
        // 平滑插值到目标位置
        float xFloat = startX + i * iconSpacing + (animatedOffset - scrollOffset) * iconSpacing;
        int x = (int)std::round(xFloat);
        int y = startY;
        
        if (x < -iconSize || x > 256) continue;
        
        // 确定文件类型
        bool isDirectory = false;
        bool isNDS = false;
        std::string fileName = "";
        
        if (files && pos < (int)files->size()) {
            const FileEntry& entry = (*files)[pos];
            isDirectory = entry.isDirectory;
            isNDS = !isDirectory && isNDSFile(entry.name);
            fileName = entry.name;
        }
        
        bool isSelected = (pos == selectedIndex);
        
        // 绘制选中效果的背景光晕
        if (isSelected) {
            // 绘制多层光晕效果
            for (int glow = 0; glow < 3; glow++) {
                int alpha = 80 - glow * 20;
                int offset = glow * 2;
                SDL_SetRenderDrawColor(renderer, 100, 150, 255, alpha);
                SDL_Rect glowRect = {x - offset, y - offset, iconSize + offset * 2, iconSize + offset * 2};
                SDL_RenderFillRect(renderer, &glowRect);
            }
        }
        
        SDL_Texture* boxTex = isSelected ? boxFullTexture : boxEmptyTexture;
        if (boxTex) {
            SDL_Rect destRect = {x, y, iconSize, iconSize};
            SDL_RenderCopy(renderer, boxTex, nullptr, &destRect);
            
            // 如果选中，绘制美化的高亮边框
            if (isSelected) {
                // 外层白色边框
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_Rect outerRect = {x - 3, y - 3, iconSize + 6, iconSize + 6};
                SDL_RenderDrawRect(renderer, &outerRect);
                
                // 内层蓝色边框
                SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
                SDL_Rect innerRect = {x - 1, y - 1, iconSize + 2, iconSize + 2};
                SDL_RenderDrawRect(renderer, &innerRect);
                
                // 顶部高光效果
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
                SDL_Rect highlightRect = {x + 2, y + 2, iconSize - 4, 8};
                SDL_RenderFillRect(renderer, &highlightRect);
            }
        } else {
            // 备用：绘制美化的框
            if (isSelected) {
                // 选中状态：渐变背景
                for (int i = 0; i < iconSize; i++) {
                    Uint8 color = 200 + (i * 55 / iconSize);
                    SDL_SetRenderDrawColor(renderer, color, color, 255, 255);
                    SDL_RenderDrawLine(renderer, x, y + i, x + iconSize, y + i);
                }
                // 边框
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_Rect boxRect = {x, y, iconSize, iconSize};
                SDL_RenderDrawRect(renderer, &boxRect);
            } else {
                // 未选中状态：简单边框
                SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
                SDL_Rect boxRect = {x, y, iconSize, iconSize};
                SDL_RenderDrawRect(renderer, &boxRect);
            }
        }
        
        // 绘制图标
        SDL_Texture* iconTex = nullptr;
        if (isDirectory) {
            // 经典文件夹图标：简洁、清晰
            int iconX = x + 8;
            int iconY = y + 8;
            int iconW = 32;
            int iconH = 32;
            
            // 文件夹主体（简单的黄色/米色填充）
            SDL_SetRenderDrawColor(renderer, 240, 220, 160, 255);
            SDL_Rect folderBody = {iconX, iconY + 6, iconW, iconH - 6};
            SDL_RenderFillRect(renderer, &folderBody);
            
            // 文件夹标签（稍深的颜色）
            SDL_SetRenderDrawColor(renderer, 220, 200, 140, 255);
            SDL_Rect folderTab = {iconX + 2, iconY, 20, 8};
            SDL_RenderFillRect(renderer, &folderTab);
            
            // 标签折角（简单的斜角）
            SDL_SetRenderDrawColor(renderer, 200, 180, 120, 255);
            SDL_Rect tabCorner = {iconX + 20, iconY + 2, 6, 6};
            SDL_RenderFillRect(renderer, &tabCorner);
            
            // 简单的边框（深色轮廓）
            SDL_SetRenderDrawColor(renderer, 180, 160, 100, 255);
            // 主体边框
            SDL_Rect borderRect = {iconX, iconY + 6, iconW, iconH - 6};
            SDL_RenderDrawRect(renderer, &borderRect);
            // 标签边框
            SDL_Rect tabBorder = {iconX + 2, iconY, 20, 8};
            SDL_RenderDrawRect(renderer, &tabBorder);
            
            // 简单的内部装饰线（可选，增加细节）
            SDL_SetRenderDrawColor(renderer, 200, 180, 120, 255);
            SDL_RenderDrawLine(renderer, iconX + 4, iconY + 16, iconX + iconW - 6, iconY + 16);
            SDL_RenderDrawLine(renderer, iconX + 4, iconY + 22, iconX + iconW - 6, iconY + 22);

        } else if (isNDS) {
            // 尝试从NDS文件加载实际图标
            if (files && pos < (int)files->size()) {
                const FileEntry& entry = (*files)[pos];
                SDL_Texture* ndsIcon = NDSIconLoader::loadIconFromNDS(entry.path);
                if (ndsIcon) {
                    iconTex = ndsIcon;
                } else if (ndsFileTexture) {
                    // 如果加载失败，使用默认NDS图标
                    iconTex = ndsFileTexture;
                }
            } else if (ndsFileTexture) {
                iconTex = ndsFileTexture;
            }
            
            // 如果还是没有图标，绘制备用图标
            if (!iconTex) {
                SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
                SDL_Rect ndsRect = {x + 8, y + 8, 32, 32};
                SDL_RenderFillRect(renderer, &ndsRect);
            }
        }
        
        if (iconTex) {
            // 为图标添加阴影效果（仅在选中时）
            if (isSelected) {
                // 绘制图标阴影
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
                SDL_Rect shadowRect = {x + 10, y + 10, 32, 32};
                SDL_RenderFillRect(renderer, &shadowRect);
            }
            
            SDL_Rect iconRect = {x + 8, y + 8, 32, 32};
            SDL_RenderCopy(renderer, iconTex, nullptr, &iconRect);
            
            // 为选中的图标添加高光效果
            if (isSelected) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 80);
                SDL_Rect iconHighlight = {x + 8, y + 8, 32, 12};
                SDL_RenderFillRect(renderer, &iconHighlight);
            }
        }
        
        // 绘制文件名（在图标下方，所有项目都显示）
        if (!fileName.empty()) {
            // 截断过长的文件名
            std::string displayName = fileName;
            if (displayName.length() > 8) {
                displayName = displayName.substr(0, 5) + "...";
            }
            // 选中项目使用BLACK，未选中使用灰色
            SDL_Color textColor = (pos == selectedIndex) ? SDL_Color{0, 0, 0, 255} : SDL_Color{180, 180, 180, 255};
            //TextRenderer::drawTextCentered(x, y + iconSize + 2, iconSize, displayName, textColor, 11);
        }
    }
    
    // 绘制滚动指示器（如果有多个项目）
    if (scrollOffset > 0) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect leftArrow = {5, startY + iconSize / 2 - 4, 8, 8};
        //SDL_RenderFillRect(renderer, &leftArrow);
    }
    if (selectedIndex < totalItems - 1 && scrollOffset + visibleIcons < totalItems) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect rightArrow = {243, startY + iconSize / 2 - 4, 8, 8};
        //SDL_RenderFillRect(renderer, &rightArrow);
    }
}

void DSiUI::drawDSiDateTime() {
    if (!renderer) return;
    
    // 获取调整后的日期时间（考虑用户设置的时间偏移）
    time_t rawTime = g_settings.getAdjustedTime();
    struct tm* timeInfo = localtime(&rawTime);
    char dateStr[32];
    char timeStr[32];
    
    // 格式化日期: YYYY/MM/DD
    strftime(dateStr, sizeof(dateStr), "%Y/%m/%d", timeInfo);
    
    // 格式化时间: HH:MM
    strftime(timeStr, sizeof(timeStr), "%H:%M", timeInfo);
    
    // DSi风格的日期时间显示（上屏右上角，UI元素在上屏）
    SDL_Color dateColor = {150, 150, 150, 255};
    TextRenderer::drawText(120, 8, dateStr, dateColor, 12);
    TextRenderer::drawText(190, 8, timeStr, dateColor, 12);
}

void DSiUI::drawStartBorder(bool visible) {
    if (!renderer || !visible) return;
    
    // 绘制START边框（在下屏选中游戏下方）
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_Rect borderRect = {80, 140 + 192+8, 96, 20};
    SDL_RenderDrawRect(renderer, &borderRect);
    
    SDL_Color textColor = {255, 255, 255, 255};
    TextRenderer::drawTextCentered(80, 143 + 192+10, 96, "START", SDL_Color{150, 150, 150, 255}, 12);
}

