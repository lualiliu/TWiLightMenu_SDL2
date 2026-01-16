#include "gameGrid.h"
#include "input.h"
#include <SDL2/SDL.h>
#include <cmath>

int GameGrid::selectedIndex = 0;
int GameGrid::scrollOffset = -2;  // 初始偏移-2，让第一项显示在第三个位置
float GameGrid::animatedScrollOffset = -2.0f;  // 动画后的滚动偏移
int GameGrid::targetScrollOffset = -2;  // 目标滚动偏移
int GameGrid::maxVisible = 5;  // 修复：实际显示5个图标
int GameGrid::maxItemsCount = 0;

void GameGrid::setSelectedIndex(int index) {
    // 限制索引范围
    if (maxItemsCount > 0 && index >= maxItemsCount) {
        index = maxItemsCount - 1;
    }
    if (index < 0) index = 0;
    
    selectedIndex = index;
    
    // 调整滚动位置：选中项应该显示在第三个位置（索引2）
    const int targetPosition = 2;  // 第三个位置
    
    // 计算需要的scrollOffset，使选中项显示在第三个位置
    int desiredOffset = selectedIndex - targetPosition;
    
    // 确保滚动偏移不会超出范围
    if (maxItemsCount > 0) {
        // 最小偏移：第一项显示在第三个位置，scrollOffset = 0 - 2 = -2
        int minScroll = -2;
        
        // 最大偏移：最后一项显示在第三个位置，scrollOffset = (maxItemsCount - 1) - 2
        int maxScroll = maxItemsCount - 1 - targetPosition;
        
        // 如果项目数少于等于可见数量，调整最小偏移
        if (maxItemsCount <= maxVisible) {
            // 如果项目少，确保第一项能显示在第三个位置
            // 如果项目数 >= 3，可以使用-2偏移
            // 如果项目数 < 3，使用0偏移
            if (maxItemsCount >= 3) {
                minScroll = -2;
            } else {
                minScroll = 0;
            }
        }
        
        // 限制scrollOffset范围
        if (desiredOffset < minScroll) {
            targetScrollOffset = minScroll;
        } else if (desiredOffset > maxScroll) {
            targetScrollOffset = maxScroll;
        } else {
            targetScrollOffset = desiredOffset;
        }
    } else {
        // 如果没有项目数限制，至少确保scrollOffset >= -2
        if (desiredOffset < -2) {
            targetScrollOffset = -2;
        } else {
            targetScrollOffset = desiredOffset;
        }
    }
    
    // 更新实际scrollOffset（用于逻辑判断）
    scrollOffset = targetScrollOffset;
}

float GameGrid::getAnimatedScrollOffset() {
    return animatedScrollOffset;
}

void GameGrid::update() {
    // 平滑动画：使用线性插值从当前动画位置移动到目标位置
    const float animationSpeed = 0.3f;  // 动画速度（0-1之间，越大越快）
    float diff = targetScrollOffset - animatedScrollOffset;
    
    if (std::abs(diff) > 0.01f) {
        // 使用缓动函数实现平滑动画
        animatedScrollOffset += diff * animationSpeed;
    } else {
        // 如果差异很小，直接设置为目标值
        animatedScrollOffset = targetScrollOffset;
    }
    
    // 左右键：逐个移动
    if (InputManager::isKeyDown(KEY_LEFT)) {
        if (selectedIndex > 0) {
            setSelectedIndex(selectedIndex - 1);
        }
    }
    
    if (InputManager::isKeyDown(KEY_RIGHT)) {
        if (maxItemsCount == 0 || selectedIndex < maxItemsCount - 1) {
            setSelectedIndex(selectedIndex + 1);
        }
    }
    
    // L/R键（Q/W）：翻页
    if (InputManager::isKeyDown(KEY_L)) {
        // L键（Q）：向前翻一页（向左）
        int newIndex = selectedIndex - maxVisible;
        if (newIndex < 0) newIndex = 0;
        setSelectedIndex(newIndex);
    }
    
    if (InputManager::isKeyDown(KEY_R)) {
        // R键（W）：向后翻一页（向右）
        int newIndex = selectedIndex + maxVisible;
        if (maxItemsCount > 0 && newIndex >= maxItemsCount) {
            newIndex = maxItemsCount - 1;
        }
        setSelectedIndex(newIndex);
    }
}

