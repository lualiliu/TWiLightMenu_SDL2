#pragma once

class GameGrid {
public:
    static int getSelectedIndex() { return selectedIndex; }
    static int getScrollOffset() { return scrollOffset; }
    static float getAnimatedScrollOffset();  // 获取动画后的滚动偏移
    static void setSelectedIndex(int index);
    static void setMaxItems(int maxItems) { maxItemsCount = maxItems; }
    static void update();
    
private:
    static int selectedIndex;
    static int scrollOffset;
    static float animatedScrollOffset;  // 动画后的滚动偏移
    static int targetScrollOffset;  // 目标滚动偏移
    static int maxVisible;
    static int maxItemsCount;  // 总项目数
};

