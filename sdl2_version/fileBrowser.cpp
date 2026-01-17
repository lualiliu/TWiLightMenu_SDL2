#include "fileBrowser.h"
#include "input.h"
#include "graphics/textRenderer.h"
#include "graphics/graphics.h"
#include "dsiUI.h"
#include "ndsIconLoader.h"
#include <algorithm>
#include <sys/stat.h>
#include <cstring>
#include <sstream>
#include <iomanip>

extern SDL_Renderer* g_renderer;

FileBrowser::FileBrowser() 
    : selectedIndex(0), active(false), scrollOffset(0), maxVisibleItems(12), filterMode(FILTER_NDS_ONLY) {
}

FileBrowser::~FileBrowser() {
    cleanup();
}

bool FileBrowser::init(const std::string& startPath) {
    currentPath = startPath;
    selectedIndex = 0;
    scrollOffset = 0;
    refreshFileList();
    return true;
}

void FileBrowser::cleanup() {
    files.clear();
}

void FileBrowser::refreshFileList() {
    files.clear();
    
    DIR* dir = opendir(currentPath.c_str());
    if (!dir) {
        // 如果无法打开目录，添加错误信息
        FileEntry error;
        error.name = "[无法打开目录]";
        error.isDirectory = false;
        files.push_back(error);
        return;
    }
    
    // 添加 ".." 目录（如果不是根目录）
    if (currentPath != "." && currentPath != "/") {
        FileEntry parent;
        parent.name = "..";
        parent.path = "..";
        parent.isDirectory = true;
        parent.isParent = true;
        parent.size = 0;
        files.push_back(parent);
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过 "." 目录
        if (strcmp(entry->d_name, ".") == 0) {
            continue;
        }
        
        FileEntry file;
        file.name = entry->d_name;
        file.path = currentPath + "/" + entry->d_name;
        file.isDirectory = (entry->d_type == DT_DIR);
        file.isParent = false;
        
        // 根据过滤模式处理文件
        if (file.isDirectory) {
            // 文件夹总是显示（允许导航）
            file.size = 0;
            files.push_back(file);
        } else {
            // 文件：根据过滤模式决定是否显示
            bool shouldShow = false;
            
            if (filterMode == FILTER_NONE) {
                shouldShow = true;  // 显示所有文件
            } else if (filterMode == FILTER_NDS_ONLY) {
                shouldShow = DSiUI::isNDSFile(file.name);  // 只显示NDS文件
            } else if (filterMode == FILTER_PNG_ONLY) {
                shouldShow = isPNGFile(file.name);  // 只显示PNG文件
            }
            
            if (!shouldShow) {
                continue;  // 跳过不符合过滤条件的文件
            }
            
            // 获取文件大小
            struct stat st;
            if (stat(file.path.c_str(), &st) == 0) {
                file.size = st.st_size;
            }
            
            // 如果是NDS文件，读取标题
            if (DSiUI::isNDSFile(file.name)) {
                file.title = NDSIconLoader::loadTitleFromNDS(file.path, 1); // 使用英语标题
                if (file.title.empty()) {
                    file.title = file.name;  // 如果无法读取标题，使用文件名
                }
            } else {
                file.title = file.name;  // 非NDS文件使用文件名作为标题
            }
            
            files.push_back(file);
        }
    }
    
    closedir(dir);
    
    sortFiles();
    
    // 确保选中索引有效
    if (selectedIndex >= (int)files.size()) {
        selectedIndex = files.size() - 1;
    }
    if (selectedIndex < 0) {
        selectedIndex = 0;
    }
}

void FileBrowser::sortFiles() {
    std::sort(files.begin(), files.end(), compareEntries);
}

bool FileBrowser::compareEntries(const FileEntry& a, const FileEntry& b) {
    // ".." 总是在最前面
    if (a.isParent) return true;
    if (b.isParent) return false;
    
    // 目录优先
    if (a.isDirectory && !b.isDirectory) return true;
    if (!a.isDirectory && b.isDirectory) return false;
    
    // 按名称排序
    return a.name < b.name;
}

bool FileBrowser::changeDirectory(const std::string& path) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return false;
    }
    closedir(dir);
    
    currentPath = path;
    selectedIndex = 0;
    scrollOffset = 0;
    refreshFileList();
    return true;
}

bool FileBrowser::enterDirectory(const std::string& name) {
    std::string newPath = currentPath;
    if (newPath != ".") {
        newPath += "/" + name;
    } else {
        newPath = name;
    }
    return changeDirectory(newPath);
}

bool FileBrowser::goUp() {
    size_t lastSlash = currentPath.find_last_of('/');
    if (lastSlash != std::string::npos && lastSlash > 0) {
        std::string newPath = currentPath.substr(0, lastSlash);
        return changeDirectory(newPath);
    } else if (currentPath != ".") {
        return changeDirectory(".");
    }
    return false;
}

void FileBrowser::setSelectedIndex(int index) {
    if (index >= 0 && index < (int)files.size()) {
        selectedIndex = index;
        
        // 调整滚动位置
        if (selectedIndex < scrollOffset) {
            scrollOffset = selectedIndex;
        } else if (selectedIndex >= scrollOffset + maxVisibleItems) {
            scrollOffset = selectedIndex - maxVisibleItems + 1;
        }
    }
}

FileEntry* FileBrowser::getSelectedEntry() {
    if (selectedIndex >= 0 && selectedIndex < (int)files.size()) {
        return &files[selectedIndex];
    }
    return nullptr;
}

void FileBrowser::update() {
    if (!active) return;
    
    // 处理输入
    if (InputManager::isKeyDown(KEY_UP)) {
        setSelectedIndex(selectedIndex - 1);
    }
    
    if (InputManager::isKeyDown(KEY_DOWN)) {
        setSelectedIndex(selectedIndex + 1);
    }
    
    if (InputManager::isKeyDown(KEY_A)) {
        FileEntry* entry = getSelectedEntry();
        if (entry) {
            if (entry->isParent) {
                goUp();
            } else if (entry->isDirectory) {
                enterDirectory(entry->name);
            } else {
                // 文件选择
                // TODO: 处理文件选择
            }
        }
    }
    
    if (InputManager::isKeyDown(KEY_B)) {
        active = false;
    }
}

void FileBrowser::render() {
    if (!active || !g_renderer) return;
    
    // 绘制文件浏览器背景
    SDL_SetRenderDrawColor(g_renderer, 30, 30, 30, 240);
    SDL_Rect bgRect = {5, 5, 246, 374};
    SDL_RenderFillRect(g_renderer, &bgRect);
    
    // 绘制边框
    SDL_SetRenderDrawColor(g_renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(g_renderer, &bgRect);
    
    // 绘制标题
    SDL_Color titleColor = {255, 255, 255, 255};
    TextRenderer::drawTextCentered(5, 10, 246, "文件浏览器", titleColor, 14);
    
    // 绘制当前路径
    SDL_Color pathColor = {180, 180, 180, 255};
    std::string displayPath = currentPath;
    if (displayPath.length() > 30) {
        displayPath = "..." + displayPath.substr(displayPath.length() - 27);
    }
    TextRenderer::drawText(10, 25, displayPath, pathColor, 10);
    
    // 绘制分隔线
    SDL_SetRenderDrawColor(g_renderer, 80, 80, 80, 255);
    SDL_RenderDrawLine(g_renderer, 10, 38, 246, 38);
    
    // 绘制文件列表
    int startY = 45;
    int itemHeight = 18;
    int visibleStart = scrollOffset;
    int visibleEnd = std::min(visibleStart + maxVisibleItems, (int)files.size());
    
    for (int i = visibleStart; i < visibleEnd; i++) {
        int y = startY + (i - visibleStart) * itemHeight;
        
        // 高亮选中的项目
        if (i == selectedIndex) {
            SDL_SetRenderDrawColor(g_renderer, 60, 60, 120, 255);
            SDL_Rect highlightRect = {10, y - 2, 226, itemHeight - 2};
            SDL_RenderFillRect(g_renderer, &highlightRect);
        }
        
        // 绘制文件/目录图标和名称
        SDL_Color textColor;
        if (i == selectedIndex) {
            textColor = {255, 255, 255, 255};
        } else if (files[i].isDirectory) {
            textColor = {150, 200, 255, 255};
        } else {
            textColor = {200, 200, 200, 255};
        }
        
        std::string displayName = files[i].name;
        if (files[i].isDirectory) {
            displayName = "[DIR] " + displayName;
        } else {
            // 显示文件大小
            std::ostringstream oss;
            if (files[i].size < 1024) {
                oss << "[" << files[i].size << "B] ";
            } else if (files[i].size < 1024 * 1024) {
                oss << "[" << std::fixed << std::setprecision(1) 
                    << (files[i].size / 1024.0) << "KB] ";
            } else {
                oss << "[" << std::fixed << std::setprecision(1) 
                    << (files[i].size / (1024.0 * 1024.0)) << "MB] ";
            }
            displayName = oss.str() + displayName;
        }
        
        // 截断过长的名称
        if (displayName.length() > 25) {
            displayName = displayName.substr(0, 22) + "...";
        }
        
        TextRenderer::drawText(15, y, displayName, textColor, 10);
    }
    
    // 绘制滚动条（如果有）
    if ((int)files.size() > maxVisibleItems) {
        int totalHeight = maxVisibleItems * itemHeight;
        int scrollBarHeight = (totalHeight * maxVisibleItems) / files.size();
        if (scrollBarHeight < 10) scrollBarHeight = 10;
        int scrollBarY = startY + (scrollOffset * totalHeight) / files.size();
        SDL_SetRenderDrawColor(g_renderer, 100, 100, 100, 255);
        SDL_Rect scrollBar = {235, scrollBarY, 5, scrollBarHeight};
        SDL_RenderFillRect(g_renderer, &scrollBar);
    }
    
    // 绘制提示
    SDL_Color hintColor = {150, 150, 150, 255};
    TextRenderer::drawText(10, 360, "A: Open/Select  B: Back  ESC: Exit", hintColor, 9);
}

bool FileBrowser::isPNGFile(const std::string& filename) {
    // 转换为小写进行比较
    std::string lowerName = filename;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    // 检查PNG扩展名
    if (lowerName.length() >= 4) {
        std::string ext = lowerName.substr(lowerName.length() - 4);
        return ext == ".png";
    }
    return false;
}

std::string FileBrowser::getSelectedFilePath() const {
    if (selectedIndex >= 0 && selectedIndex < (int)files.size()) {
        return files[selectedIndex].path;
    }
    return "";
}

