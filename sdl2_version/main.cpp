#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <limits.h>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "graphics/graphics.h"
#include "graphics/fontHandler.h"
#include "graphics/textRenderer.h"
#include "fileBrowse.h"
#include "fileBrowser.h"
#include "language.h"
#include "sound.h"
#include "input.h"
#include "menu.h"
#include "fpsCounter.h"
#include "settings.h"
#include "dsiUI.h"
#include "resourceManager.h"
#include "gameGrid.h"

// 声明清理函数
extern void graphicsCleanup();
extern void fontCleanup();
extern void fileBrowseCleanup();
extern void languageCleanup();
extern void soundCleanup();

// 前向声明
extern SDL_Renderer* g_renderer;

// 全局菜单和文件浏览器
Menu* mainMenu = nullptr;
extern FileBrowser* g_fileBrowser;

// SDL2窗口和渲染器
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* screenTexture = nullptr;

// 屏幕尺寸 (DSi分辨率)
const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 384; // 上下双屏
const int TOP_SCREEN_HEIGHT = 192;
const int BOTTOM_SCREEN_HEIGHT = 192;

// 缩放因子
const int SCALE = 3;

bool running = true;

// 初始化SDL2
bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "SDL初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 创建窗口
    window = SDL_CreateWindow(
        "TWiLight Menu SDL2",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH * SCALE,
        SCREEN_HEIGHT * SCALE,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::cerr << "窗口创建失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 创建渲染器
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "渲染器创建失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 设置渲染缩放
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    // 初始化SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image初始化失败: " << IMG_GetError() << std::endl;
        return false;
    }

    // 设置纹理过滤
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    return true;
}

// 清理SDL2资源
void cleanupSDL() {
    if (screenTexture) {
        SDL_DestroyTexture(screenTexture);
        screenTexture = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    IMG_Quit();
    SDL_Quit();
}

// 处理输入事件
void handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        if (mainMenu && mainMenu->isActive()) {
                            mainMenu->setActive(false);
                        } else {
                            running = false;
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    
    // 更新输入管理器
    InputManager::update();
    
    // 处理菜单输入
    if (mainMenu) {
        if (InputManager::isKeyDown(KEY_START) && !mainMenu->isActive()) {
            mainMenu->setActive(true);
        } else if (InputManager::isKeyDown(KEY_B) && mainMenu->isActive()) {
            mainMenu->setActive(false);
        }
    }
}

int main(int argc, char* argv[]) {
    // 初始化SDL2
    if (!initSDL()) {
        return 1;
    }

    // 设置全局渲染器 (在graphics.cpp中定义)
    g_renderer = renderer;

    // 初始化图形系统
    if (!graphicsInit()) {
        std::cerr << "图形系统初始化失败" << std::endl;
        cleanupSDL();
        return 1;
    }

    // 初始化字体系统
    if (!fontInit()) {
        std::cerr << "字体系统初始化失败" << std::endl;
        cleanupSDL();
        return 1;
    }

    // 初始化语言系统
    if (!languageInit()) {
        std::cerr << "语言系统初始化失败" << std::endl;
        cleanupSDL();
        return 1;
    }

    // 初始化文件浏览系统
    if (!fileBrowseInit()) {
        std::cerr << "文件浏览系统初始化失败" << std::endl;
        cleanupSDL();
        return 1;
    }

    // 初始化音频系统
    if (!soundInit()) {
        std::cerr << "音频系统初始化失败（继续运行）" << std::endl;
    }

    // 初始化输入管理器
    InputManager::init();
    
    // 初始化FPS计数器
    FPSCounter::init();
    
    // 加载设置
    g_settings.load();
    
    // 创建主菜单
    mainMenu = new Menu();
    mainMenu->addItem("File Browser", 1);
    mainMenu->addItem("Settings", 2);
    mainMenu->addItem("Author: lualiliu", 3);
    mainMenu->addItem("Exit", 4);
    mainMenu->setActive(false);

    std::cout << "TWiLight Menu SDL2 版本已启动" << std::endl;
    std::cout << "控制说明:" << std::endl;
    std::cout << "  X键 - A键" << std::endl;
    std::cout << "  Z键 - B键" << std::endl;
    std::cout << "  方向键 - 方向键" << std::endl;
    std::cout << "  Enter - Start键" << std::endl;
    std::cout << "  Right Shift - Select键" << std::endl;
    std::cout << "  Q/W - L/R键" << std::endl;
    std::cout << "  A/S - X/Y键" << std::endl;
    std::cout << "  Start键 - 打开菜单" << std::endl;
    std::cout << "  ESC键 - 退出" << std::endl;

    // 主循环
    Uint32 lastTime = SDL_GetTicks();
    const Uint32 targetFPS = 60;
    const Uint32 frameTime = 1000 / targetFPS;

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 deltaTime = currentTime - lastTime;

        // 处理事件
        handleEvents();

        // 更新FPS计数器
        FPSCounter::update();
        
        // 更新菜单
        if (mainMenu) {
            mainMenu->update();
            
            // 处理菜单选择
            if (mainMenu->isActive() && InputManager::isKeyDown(KEY_A)) {
                int selectedId = mainMenu->getSelectedId();
                switch (selectedId) {
                    case 1: // File Browser
                        if (g_fileBrowser) {
                            g_fileBrowser->setActive(true);
                            mainMenu->setActive(false);
                        }
                        break;
                    case 2: // Settings
                        {
                            // 创建设置菜单
                            Menu* settingsMenu = new Menu();
                            std::string topWallpaperText = std::string("Top Wallpaper: ") + (g_settings.topWallpaperPath.empty() ? "Default" : "Custom");
                            std::string bottomWallpaperText = std::string("Bottom Wallpaper: ") + (g_settings.bottomWallpaperPath.empty() ? "Default" : "Custom");
                            settingsMenu->addItem(topWallpaperText, 14);
                            settingsMenu->addItem(bottomWallpaperText, 15);
                            settingsMenu->addItem("Date & Time", 16);
                            settingsMenu->addItem("Save Settings", 12);
                            settingsMenu->addItem("Back", 13);
                            settingsMenu->setActive(true);
                            
                            while (settingsMenu->isActive() && running) {
                                handleEvents();
                                settingsMenu->update();
                                
                                if (settingsMenu->isActive() && InputManager::isKeyDown(KEY_A)) {
                                    int id = settingsMenu->getSelectedId();
                                    switch (id) {
                                        case 14: // Top wallpaper settings
                                            {
                                                // 打开文件管理器选择PNG文件
                                                if (!g_fileBrowser) {
                                                    fileBrowseInit();
                                                }
                                                if (g_fileBrowser) {
                                                    g_fileBrowser->setFilterMode(FileBrowser::FILTER_PNG_ONLY);
                                                    g_fileBrowser->setActive(true);
                                                    settingsMenu->setActive(false);
                                                    
                                                    // 等待文件选择
                                                    while (g_fileBrowser->isActive() && running) {
                                                        handleEvents();
                                                        g_fileBrowser->update();
                                                        
                                                        if (InputManager::isKeyDown(KEY_A)) {
                                                            FileEntry* entry = g_fileBrowser->getSelectedEntry();
                                                            if (entry && !entry->isDirectory) {
                                                                // 选择了PNG文件
                                                                std::string selectedPath = g_fileBrowser->getSelectedFilePath();
                                                                if (!selectedPath.empty()) {
                                                                    g_settings.topWallpaperPath = selectedPath;
                                                                    DSiUI::setTopWallpaper(selectedPath);
                                                                }
                                                                g_fileBrowser->setActive(false);
                                                                break;
                                                            }
                                                        }
                                                        
                                                        if (InputManager::isKeyDown(KEY_B)) {
                                                            g_fileBrowser->setActive(false);
                                                            break;
                                                        }
                                                        
                                                        updateFrame(true);
                                                        SDL_RenderClear(renderer);
                                                        renderFrame();
                                                        g_fileBrowser->render();
                                                        SDL_RenderPresent(renderer);
                                                        SDL_Delay(16);
                                                    }
                                                    
                                                    // 恢复NDS文件过滤
                                                    g_fileBrowser->setFilterMode(FileBrowser::FILTER_NDS_ONLY);
                                                    
                                                    // 更新菜单显示
                                                    settingsMenu->clear();
                                                    topWallpaperText = std::string("Top Wallpaper: ") + (g_settings.topWallpaperPath.empty() ? "Default" : "Custom");
                                                    bottomWallpaperText = std::string("Bottom Wallpaper: ") + (g_settings.bottomWallpaperPath.empty() ? "Default" : "Custom");
                                                    settingsMenu->addItem(topWallpaperText, 14);
                                                    settingsMenu->addItem(bottomWallpaperText, 15);
                                                    settingsMenu->addItem("Date & Time", 16);
                                                    settingsMenu->addItem("Save Settings", 12);
                                                    settingsMenu->addItem("Back", 13);
                                                    settingsMenu->setActive(true);
                                                }
                                            }
                                            break;
                                        case 15: // Bottom wallpaper settings
                                            {
                                                // 打开文件管理器选择PNG文件
                                                if (!g_fileBrowser) {
                                                    fileBrowseInit();
                                                }
                                                if (g_fileBrowser) {
                                                    g_fileBrowser->setFilterMode(FileBrowser::FILTER_PNG_ONLY);
                                                    g_fileBrowser->setActive(true);
                                                    settingsMenu->setActive(false);
                                                    
                                                    // 等待文件选择
                                                    while (g_fileBrowser->isActive() && running) {
                                                        handleEvents();
                                                        g_fileBrowser->update();
                                                        
                                                        if (InputManager::isKeyDown(KEY_A)) {
                                                            FileEntry* entry = g_fileBrowser->getSelectedEntry();
                                                            if (entry && !entry->isDirectory) {
                                                                // 选择了PNG文件
                                                                std::string selectedPath = g_fileBrowser->getSelectedFilePath();
                                                                if (!selectedPath.empty()) {
                                                                    g_settings.bottomWallpaperPath = selectedPath;
                                                                    DSiUI::setBottomWallpaper(selectedPath);
                                                                }
                                                                g_fileBrowser->setActive(false);
                                                                break;
                                                            }
                                                        }
                                                        
                                                        if (InputManager::isKeyDown(KEY_B)) {
                                                            g_fileBrowser->setActive(false);
                                                            break;
                                                        }
                                                        
                                                        updateFrame(true);
                                                        SDL_RenderClear(renderer);
                                                        renderFrame();
                                                        g_fileBrowser->render();
                                                        SDL_RenderPresent(renderer);
                                                        SDL_Delay(16);
                                                    }
                                                    
                                                    // 恢复NDS文件过滤
                                                    g_fileBrowser->setFilterMode(FileBrowser::FILTER_NDS_ONLY);
                                                    
                                                    // 更新菜单显示
                                                    settingsMenu->clear();
                                                    topWallpaperText = std::string("Top Wallpaper: ") + (g_settings.topWallpaperPath.empty() ? "Default" : "Custom");
                                                    bottomWallpaperText = std::string("Bottom Wallpaper: ") + (g_settings.bottomWallpaperPath.empty() ? "Default" : "Custom");
                                                    settingsMenu->addItem(topWallpaperText, 14);
                                                    settingsMenu->addItem(bottomWallpaperText, 15);
                                                    settingsMenu->addItem("Date & Time", 16);
                                                    settingsMenu->addItem("Save Settings", 12);
                                                    settingsMenu->addItem("Back", 13);
                                                    settingsMenu->setActive(true);
                                                }
                                            }
                                            break;
                                        case 16: // Date & Time settings
                                            {
                                                // 获取当前调整后的时间
                                                time_t adjustedTime = g_settings.getAdjustedTime();
                                                struct tm* timeInfo = localtime(&adjustedTime);
                                                
                                                int year = timeInfo->tm_year + 1900;
                                                int month = timeInfo->tm_mon + 1;
                                                int day = timeInfo->tm_mday;
                                                int hour = timeInfo->tm_hour;
                                                int minute = timeInfo->tm_min;
                                                
                                                int editingField = 0;  // 0=year, 1=month, 2=day, 3=hour, 4=minute
                                                bool editing = true;
                                                
                                                while (editing && running) {
                                                    handleEvents();
                                                    
                                                    // 处理输入
                                                    if (InputManager::isKeyDown(KEY_UP)) {
                                                        switch (editingField) {
                                                            case 0: year++; break;
                                                            case 1: month++; if (month > 12) month = 1; break;
                                                            case 2: day++; if (day > 31) day = 1; break;
                                                            case 3: hour++; if (hour > 23) hour = 0; break;
                                                            case 4: minute++; if (minute > 59) minute = 0; break;
                                                        }
                                                    }
                                                    
                                                    if (InputManager::isKeyDown(KEY_DOWN)) {
                                                        switch (editingField) {
                                                            case 0: year--; if (year < 2000) year = 2099; break;
                                                            case 1: month--; if (month < 1) month = 12; break;
                                                            case 2: day--; if (day < 1) day = 31; break;
                                                            case 3: hour--; if (hour < 0) hour = 23; break;
                                                            case 4: minute--; if (minute < 0) minute = 59; break;
                                                        }
                                                    }
                                                    
                                                    if (InputManager::isKeyDown(KEY_LEFT)) {
                                                        editingField--;
                                                        if (editingField < 0) editingField = 4;
                                                    }
                                                    
                                                    if (InputManager::isKeyDown(KEY_RIGHT)) {
                                                        editingField++;
                                                        if (editingField > 4) editingField = 0;
                                                    }
                                                    
                                                    if (InputManager::isKeyDown(KEY_A) || InputManager::isKeyDown(KEY_START)) {
                                                        // 应用设置
                                                        g_settings.setTimeOffset(year, month, day, hour, minute);
                                                        std::cout << "Date & Time set to: " << year << "/" << month << "/" << day << " " << hour << ":" << minute << std::endl;
                                                        editing = false;
                                                    }
                                                    
                                                    if (InputManager::isKeyDown(KEY_B)) {
                                                        editing = false;
                                                    }
                                                    
                                                    // 渲染
                                                    updateFrame(true);
                                                    SDL_RenderClear(renderer);
                                                    renderFrame();
                                                    
                                                    // 绘制日期时间设置界面（与Menu对齐）
                                                    if (renderer) {
                                                        // 使用与Menu相同的坐标
                                                        const int menuX = 10;
                                                        const int menuY = 50;
                                                        const int menuWidth = 196;
                                                        const int menuHeight = 120;
                                                        
                                                        // 背景（与Menu对齐）
                                                        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 240);
                                                        SDL_Rect bgRect = {menuX - 5, menuY - 5, menuWidth + 10, menuHeight + 10};
                                                        SDL_RenderFillRect(renderer, &bgRect);
                                                        
                                                        // 边框
                                                        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                                                        SDL_RenderDrawRect(renderer, &bgRect);
                                                        
                                                        // 标题栏（与Menu对齐）
                                                        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
                                                        SDL_Rect titleRect = {menuX - 5, menuY - 5, menuWidth + 10, 20};
                                                        SDL_RenderFillRect(renderer, &titleRect);
                                                        
                                                        // 标题（与Menu对齐）
                                                        SDL_Color titleColor = {255, 255, 255, 255};
                                                        TextRenderer::drawTextCentered(menuX - 5, menuY - 2, menuWidth + 10, "Date & Time", titleColor, 12);
                                                        
                                                        // 格式化显示
                                                        std::ostringstream dateTimeStr;
                                                        dateTimeStr << std::setfill('0') << std::setw(4) << year << "/"
                                                                   << std::setw(2) << month << "/"
                                                                   << std::setw(2) << day << " "
                                                                   << std::setw(2) << hour << ":"
                                                                   << std::setw(2) << minute;
                                                        
                                                        // 高亮当前编辑的字段
                                                        SDL_Color normalColor = {200, 200, 200, 255};
                                                        SDL_Color highlightColor = {255, 255, 0, 255};
                                                        
                                                        // 内容区域从标题栏下方开始（与Menu对齐）
                                                        int xPos = menuX + 40;
                                                        int yPos = menuY + 20 + 30 + 10;  // menuY + 20是标题栏结束，+10是间距
                                                        
                                                        // 绘制年份
                                                        SDL_Color yearColor = (editingField == 0) ? highlightColor : normalColor;
                                                        std::string yearStr = std::to_string(year);
                                                        TextRenderer::drawText(xPos, yPos, yearStr, yearColor, 12);
                                                        xPos += TextRenderer::getTextWidth(yearStr, 12) + 2;
                                                        TextRenderer::drawText(xPos, yPos, "/", normalColor, 12);
                                                        xPos += 8;
                                                        
                                                        // 绘制月份
                                                        SDL_Color monthColor = (editingField == 1) ? highlightColor : normalColor;
                                                        std::string monthStr = (month < 10 ? "0" : "") + std::to_string(month);
                                                        TextRenderer::drawText(xPos, yPos, monthStr, monthColor, 12);
                                                        xPos += TextRenderer::getTextWidth(monthStr, 12) + 2;
                                                        TextRenderer::drawText(xPos, yPos, "/", normalColor, 12);
                                                        xPos += 8;
                                                        
                                                        // 绘制日期
                                                        SDL_Color dayColor = (editingField == 2) ? highlightColor : normalColor;
                                                        std::string dayStr = (day < 10 ? "0" : "") + std::to_string(day);
                                                        TextRenderer::drawText(xPos, yPos, dayStr, dayColor, 12);
                                                        xPos += TextRenderer::getTextWidth(dayStr, 12) + 8;
                                                        
                                                        // 绘制小时
                                                        SDL_Color hourColor = (editingField == 3) ? highlightColor : normalColor;
                                                        std::string hourStr = (hour < 10 ? "0" : "") + std::to_string(hour);
                                                        TextRenderer::drawText(xPos, yPos, hourStr, hourColor, 12);
                                                        xPos += TextRenderer::getTextWidth(hourStr, 12) + 2;
                                                        TextRenderer::drawText(xPos, yPos, ":", normalColor, 12);
                                                        xPos += 8;
                                                        
                                                        // 绘制分钟
                                                        SDL_Color minuteColor = (editingField == 4) ? highlightColor : normalColor;
                                                        std::string minuteStr = (minute < 10 ? "0" : "") + std::to_string(minute);
                                                        TextRenderer::drawText(xPos, yPos, minuteStr, minuteColor, 12);
                                                        
                                                        // 提示（在底部）
                                                        SDL_Color hintColor = {150, 150, 150, 255};
                                                        //TextRenderer::drawTextCentered(menuX - 5, menuY + menuHeight - 25, menuWidth + 10, "Up/Down: Change  Left/Right: Select", hintColor, 10);
                                                        TextRenderer::drawTextCentered(menuX - 5, menuY + menuHeight - 10, menuWidth + 10, "A/Start: Save  B: Cancel", hintColor, 10);
                                                    }
                                                    
                                                    SDL_RenderPresent(renderer);
                                                    SDL_Delay(16);
                                                }
                                                
                                                // 更新菜单显示
                                                settingsMenu->clear();
                                                topWallpaperText = std::string("Top Wallpaper: ") + (g_settings.topWallpaperPath.empty() ? "Default" : "Custom");
                                                bottomWallpaperText = std::string("Bottom Wallpaper: ") + (g_settings.bottomWallpaperPath.empty() ? "Default" : "Custom");
                                                settingsMenu->addItem(topWallpaperText, 14);
                                                settingsMenu->addItem(bottomWallpaperText, 15);
                                                settingsMenu->addItem("Date & Time", 16);
                                                settingsMenu->addItem("Save Settings", 12);
                                                settingsMenu->addItem("Back", 13);
                                                settingsMenu->setActive(true);
                                            }
                                            break;
                                        case 12:
                                            g_settings.save();
                                            std::cout << "Settings saved" << std::endl;
                                            break;
                                        case 13:
                                            settingsMenu->setActive(false);
                                            break;
                                    }
                                }
                                
                                if (settingsMenu->isActive() && InputManager::isKeyDown(KEY_B)) {
                                    settingsMenu->setActive(false);
                                }
                                
                                updateFrame(true);
                                SDL_RenderClear(renderer);
                                renderFrame();
                                settingsMenu->render();
                                SDL_RenderPresent(renderer);
                                
                                SDL_Delay(16);
                            }
                            
                            delete settingsMenu;
                            mainMenu->setActive(false);
                        }
                        break;
                    case 3: // About
                        std::cout << "TWiLight Menu SDL2 Version 0.1.0" << std::endl;
                        mainMenu->setActive(false);
                        break;
                    case 4: // Exit
                        running = false;
                        break;
                }
            }
            
        // 更新文件浏览器
        if (g_fileBrowser && g_fileBrowser->isActive()) {
            g_fileBrowser->update();
            if (InputManager::isKeyDown(KEY_START)) {
                g_fileBrowser->setActive(false);
            }
        }
        
        // 处理游戏网格导航（当菜单和文件浏览器都不活动时）
        if ((!mainMenu || !mainMenu->isActive()) && (!g_fileBrowser || !g_fileBrowser->isActive())) {
            GameGrid::update();
            
            // 处理A键或Start键按下：进入文件夹或启动NDS文件
            if (InputManager::isKeyDown(KEY_A) || InputManager::isKeyDown(KEY_START)) {
                if (g_fileBrowser) {
                    int selectedIndex = GameGrid::getSelectedIndex();
                    const std::vector<FileEntry>& fileList = g_fileBrowser->getFiles();
                    
                    if (selectedIndex >= 0 && selectedIndex < (int)fileList.size()) {
                        const FileEntry& entry = fileList[selectedIndex];
                        
                        if (entry.isDirectory) {
                            // 进入文件夹
                            std::string dirPath = entry.path;
                            if (entry.isParent) {
                                // 返回上一级目录
                                g_fileBrowser->goUp();
                            } else {
                                // 进入子目录
                                g_fileBrowser->enterDirectory(entry.name);
                            }
                            // 重置选中索引
                            GameGrid::setSelectedIndex(0);
                            std::cout << "进入文件夹: " << g_fileBrowser->getCurrentPath() << std::endl;
                        } else if (DSiUI::isNDSFile(entry.name)) {
                            // 启动NDS文件
                            std::string ndsPath = entry.path;
                            // 获取绝对路径
                            char* absPath = realpath(ndsPath.c_str(), nullptr);
                            if (absPath) {
                                std::string absolutePath = absPath;
                                free(absPath);
                                
                                // 调用start_drastic.sh脚本
                                std::string command = "start_drastic.sh \"" + absolutePath + "\"";
                                std::cout << "启动NDS文件: " << absolutePath << std::endl;
                                std::cout << "执行命令: " << command << std::endl;
                                
                                // 在后台执行脚本
                                int result = system((command + " > /tmp/drastic.log 2>&1 &").c_str());
                                if (result == 0) {
                                    std::cout << "NDS文件启动成功，日志输出到 /tmp/drastic.log" << std::endl;
                                } else {
                                    std::cerr << "启动NDS文件失败，返回码: " << result << std::endl;
                                }
                            } else {
                                std::cerr << "无法获取绝对路径: " << ndsPath << std::endl;
                            }
                        }
                    }
                }
            }
        }
        }

        // 更新游戏逻辑
        updateFrame(true);

        // 渲染
        SDL_RenderClear(renderer);
        renderFrame();
        
        // 渲染FPS（如果启用）
        if (g_settings.showFPS) {
            FPSCounter::render();
        }
        
        // 渲染文件浏览器
        if (g_fileBrowser && g_fileBrowser->isActive()) {
            g_fileBrowser->render();
        }
        
        // 渲染菜单
        if (mainMenu && mainMenu->isActive()) {
            mainMenu->render();
        }
        
        SDL_RenderPresent(renderer);

        // 帧率控制
        Uint32 frameTimeElapsed = SDL_GetTicks() - currentTime;
        if (frameTimeElapsed < frameTime) {
            SDL_Delay(frameTime - frameTimeElapsed);
        }

        lastTime = currentTime;
    }

    // 清理资源
    if (mainMenu) {
        delete mainMenu;
        mainMenu = nullptr;
    }
    InputManager::cleanup();
    graphicsCleanup();
    fontCleanup();
    fileBrowseCleanup();
    languageCleanup();
    soundCleanup();
    cleanupSDL();
    
    std::cout << "程序已退出" << std::endl;
    return 0;
}

