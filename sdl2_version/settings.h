#pragma once

#include <string>

struct Settings {
    bool showFPS;
    int fontSize;
    std::string language;
    bool fullscreen;
    int scale;
    std::string topWallpaperPath;     // 上屏壁纸路径
    std::string bottomWallpaperPath;  // 下屏壁纸路径
    
    // 日期时间设置（相对于系统时间的偏移，单位：秒）
    int timeOffsetSeconds;  // 时间偏移（秒）
    
    Settings() : showFPS(true), fontSize(12), language("zh_CN"), fullscreen(false), scale(3), 
                 topWallpaperPath(""), bottomWallpaperPath(""), timeOffsetSeconds(0) {}
    
    void load();
    void save();
    
    // 获取调整后的时间
    time_t getAdjustedTime() const;
    // 设置时间偏移
    void setTimeOffset(int year, int month, int day, int hour, int minute);
};

extern Settings g_settings;

