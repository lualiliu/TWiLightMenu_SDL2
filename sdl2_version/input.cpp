#include "input.h"
#include <cstring>
#include <vector>
#include <iostream>
#include <SDL2/SDL_gamecontroller.h>

InputState InputManager::currentState = {0};
InputState InputManager::previousState = {0};
std::map<SDL_Keycode, NDSKey> InputManager::keyMap;

// 手柄支持
static std::vector<SDL_GameController*> gameControllers;

void InputManager::init() {
    keyMap.clear();
    initKeyMap();
    memset(&currentState, 0, sizeof(currentState));
    memset(&previousState, 0, sizeof(previousState));
    
    // 初始化手柄支持
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "手柄子系统初始化失败: " << SDL_GetError() << std::endl;
    } else {
        // 打开所有已连接的手柄
        for (int i = 0; i < SDL_NumJoysticks(); i++) {
            if (SDL_IsGameController(i)) {
                SDL_GameController* controller = SDL_GameControllerOpen(i);
                if (controller) {
                    gameControllers.push_back(controller);
                    std::cout << "检测到手柄: " << SDL_GameControllerName(controller) << std::endl;
                }
            }
        }
    }
}

void InputManager::initKeyMap() {
    // 默认键盘映射
    keyMap[SDLK_x] = KEY_A;
    keyMap[SDLK_z] = KEY_B;
    keyMap[SDLK_RETURN] = KEY_START;
    keyMap[SDLK_RSHIFT] = KEY_SELECT;
    keyMap[SDLK_RIGHT] = KEY_RIGHT;
    keyMap[SDLK_LEFT] = KEY_LEFT;
    keyMap[SDLK_UP] = KEY_UP;
    keyMap[SDLK_DOWN] = KEY_DOWN;
    keyMap[SDLK_q] = KEY_L;
    keyMap[SDLK_w] = KEY_R;
    keyMap[SDLK_a] = KEY_X;
    keyMap[SDLK_s] = KEY_Y;
}

void InputManager::update() {
    // 保存上一帧状态
    previousState = currentState;
    
    // 重置本帧状态
    currentState.keysDown = 0;
    currentState.keysUp = 0;
    currentState.touchPressed = false;
    currentState.touchReleased = false;
    
    // 处理键盘输入
    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
    uint16_t newKeysHeld = 0;
    
    // 检查所有映射的按键
    for (const auto& pair : keyMap) {
        SDL_Keycode keycode = pair.first;
        NDSKey ndsKey = pair.second;
        SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode);
        if (scancode != SDL_SCANCODE_UNKNOWN && keyboardState[scancode]) {
            newKeysHeld |= ndsKey;
        }
    }
    
    // 处理手柄输入
    for (SDL_GameController* controller : gameControllers) {
        if (!controller) continue;
        
        // A键（通常是底部按钮，Xbox: A, PlayStation: Cross）
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A)) {
            newKeysHeld |= KEY_A;
        }
        // B键（通常是右侧按钮，Xbox: B, PlayStation: Circle）
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B)) {
            newKeysHeld |= KEY_B;
        }
        // X键（通常是左侧按钮）
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X)) {
            newKeysHeld |= KEY_X;
        }
        // Y键（通常是顶部按钮）
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y)) {
            newKeysHeld |= KEY_Y;
        }
        // Start键
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START)) {
            newKeysHeld |= KEY_START;
        }
        // Select键
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK)) {
            newKeysHeld |= KEY_SELECT;
        }
        // 方向键
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP)) {
            newKeysHeld |= KEY_UP;
        }
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
            newKeysHeld |= KEY_DOWN;
        }
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
            newKeysHeld |= KEY_LEFT;
        }
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
            newKeysHeld |= KEY_RIGHT;
        }
        // 肩键
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) {
            newKeysHeld |= KEY_L;
        }
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) {
            newKeysHeld |= KEY_R;
        }
    }
    
    // 计算按键变化
    currentState.keysDown = newKeysHeld & ~previousState.keysHeld;
    currentState.keysUp = previousState.keysHeld & ~newKeysHeld;
    currentState.keysHeld = newKeysHeld;
    
    // 处理鼠标/触摸输入
    int mouseX, mouseY;
    Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
    
    // 转换为逻辑坐标（考虑缩放）
    mouseX = mouseX / 3; // SCALE = 3
    mouseY = mouseY / 3;
    
    bool wasTouching = currentState.touchDown;
    currentState.touchDown = (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    
    if (currentState.touchDown) {
        currentState.touchX = mouseX;
        currentState.touchY = mouseY;
        currentState.touchPressed = !wasTouching;
    } else {
        currentState.touchReleased = wasTouching;
    }
    
    // 处理触摸事件
    SDL_Event e;
    while (SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONUP) > 0) {
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            currentState.touchPressed = true;
            currentState.touchDown = true;
            currentState.touchX = e.button.x / 3;
            currentState.touchY = e.button.y / 3;
        } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
            currentState.touchReleased = true;
            currentState.touchDown = false;
        }
    }
}

void InputManager::cleanup() {
    // 关闭所有手柄
    for (SDL_GameController* controller : gameControllers) {
        if (controller) {
            SDL_GameControllerClose(controller);
        }
    }
    gameControllers.clear();
    
    memset(&currentState, 0, sizeof(currentState));
    memset(&previousState, 0, sizeof(previousState));
}

InputState InputManager::getState() {
    return currentState;
}

bool InputManager::isKeyHeld(NDSKey key) {
    return (currentState.keysHeld & key) != 0;
}

bool InputManager::isKeyDown(NDSKey key) {
    return (currentState.keysDown & key) != 0;
}

bool InputManager::isKeyUp(NDSKey key) {
    return (currentState.keysUp & key) != 0;
}

bool InputManager::isTouching() {
    return currentState.touchDown;
}

void InputManager::getTouchPos(int& x, int& y) {
    x = currentState.touchX;
    y = currentState.touchY;
}

void InputManager::setKeyMapping(SDL_Keycode key, NDSKey ndsKey) {
    if (ndsKey != 0) {
        keyMap[key] = ndsKey;
    } else {
        keyMap.erase(key);
    }
}

