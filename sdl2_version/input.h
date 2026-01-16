#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <map>

// NDS按键定义
enum NDSKey {
    KEY_A = 1 << 0,
    KEY_B = 1 << 1,
    KEY_SELECT = 1 << 2,
    KEY_START = 1 << 3,
    KEY_RIGHT = 1 << 4,
    KEY_LEFT = 1 << 5,
    KEY_UP = 1 << 6,
    KEY_DOWN = 1 << 7,
    KEY_R = 1 << 8,
    KEY_L = 1 << 9,
    KEY_X = 1 << 10,
    KEY_Y = 1 << 11,
    KEY_TOUCH = 1 << 12
};

// 输入状态
struct InputState {
    uint16_t keysHeld;      // 当前按下的键
    uint16_t keysDown;      // 本帧按下的键
    uint16_t keysUp;        // 本帧释放的键
    int touchX;             // 触摸X坐标
    int touchY;             // 触摸Y坐标
    bool touchDown;         // 触摸是否按下
    bool touchPressed;      // 触摸是否刚按下
    bool touchReleased;     // 触摸是否刚释放
};

class InputManager {
public:
    static void init();
    static void update();
    static void cleanup();
    
    // 获取输入状态
    static InputState getState();
    
    // 检查按键状态
    static bool isKeyHeld(NDSKey key);
    static bool isKeyDown(NDSKey key);
    static bool isKeyUp(NDSKey key);
    
    // 触摸相关
    static bool isTouching();
    static void getTouchPos(int& x, int& y);
    
    // 键盘映射
    static void setKeyMapping(SDL_Keycode key, NDSKey ndsKey);
    
private:
    static InputState currentState;
    static InputState previousState;
    static std::map<SDL_Keycode, NDSKey> keyMap; // 键盘映射表
    static void initKeyMap();
};

