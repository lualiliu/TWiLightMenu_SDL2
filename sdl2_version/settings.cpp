#include "settings.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <cmath>

Settings g_settings;

void Settings::load() {
    std::ifstream file("twilightmenu_sdl2.ini");
    if (!file.is_open()) {
        // 使用默认设置
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        if (key == "showFPS") {
            showFPS = (value == "1" || value == "true");
        } else if (key == "fontSize") {
            fontSize = std::stoi(value);
        } else if (key == "language") {
            language = value;
        } else if (key == "fullscreen") {
            fullscreen = (value == "1" || value == "true");
        } else if (key == "scale") {
            scale = std::stoi(value);
        } else if (key == "topWallpaperPath") {
            topWallpaperPath = value;
        } else if (key == "bottomWallpaperPath") {
            bottomWallpaperPath = value;
        } else if (key == "timeOffsetSeconds") {
            timeOffsetSeconds = std::stoi(value);
        }
    }
    
    file.close();
}

void Settings::save() {
    std::ofstream file("twilightmenu_sdl2.ini");
    if (!file.is_open()) {
        std::cerr << "Failed to save settings file" << std::endl;
        return;
    }
    
    file << "showFPS=" << (showFPS ? "1" : "0") << std::endl;
    file << "fontSize=" << fontSize << std::endl;
    file << "language=" << language << std::endl;
    file << "fullscreen=" << (fullscreen ? "1" : "0") << std::endl;
    file << "scale=" << scale << std::endl;
    file << "topWallpaperPath=" << topWallpaperPath << std::endl;
    file << "bottomWallpaperPath=" << bottomWallpaperPath << std::endl;
    file << "timeOffsetSeconds=" << timeOffsetSeconds << std::endl;
    
    file.close();
}

time_t Settings::getAdjustedTime() const {
    time_t currentTime = time(nullptr);
    return currentTime + timeOffsetSeconds;
}

void Settings::setTimeOffset(int year, int month, int day, int hour, int minute) {
    // 获取当前系统时间
    time_t currentTime = time(nullptr);
    struct tm* currentTm = localtime(&currentTime);
    
    // 创建目标时间结构
    struct tm targetTm = *currentTm;
    targetTm.tm_year = year - 1900;  // tm_year是从1900年开始的年数
    targetTm.tm_mon = month - 1;    // tm_mon是0-11
    targetTm.tm_mday = day;
    targetTm.tm_hour = hour;
    targetTm.tm_min = minute;
    targetTm.tm_sec = 0;
    
    // 转换为时间戳
    time_t targetTime = mktime(&targetTm);
    
    // 计算偏移量（秒）
    timeOffsetSeconds = (int)(targetTime - currentTime);
}

