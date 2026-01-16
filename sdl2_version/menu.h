#pragma once

#include <string>
#include <vector>
#include "input.h"

struct MenuItem {
    std::string text;
    int id;
    bool enabled;
    
    MenuItem(const std::string& t, int i, bool e = true) 
        : text(t), id(i), enabled(e) {}
};

class Menu {
public:
    Menu();
    ~Menu();
    
    void addItem(const std::string& text, int id, bool enabled = true);
    void clear();
    
    void update();
    void render();
    
    int getSelectedId() const;
    int getSelectedIndex() const;
    void setSelectedIndex(int index);
    
    bool isActive() const { return active; }
    void setActive(bool a) { active = a; }
    
private:
    std::vector<MenuItem> items;
    int selectedIndex;
    bool active;
    int menuX, menuY;
    int itemHeight;
    int menuWidth;
    
    void handleInput();
};

