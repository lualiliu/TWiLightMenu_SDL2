#pragma once

#include <string>
#include <vector>
#include <dirent.h>

struct FileEntry {
    std::string name;
    std::string path;
    std::string title;  // NDS文件内部标题
    bool isDirectory;
    bool isParent;  // ".." 目录
    size_t size;
    
    FileEntry() : isDirectory(false), isParent(false), size(0) {}
};

class FileBrowser {
public:
    FileBrowser();
    ~FileBrowser();
    
    bool init(const std::string& startPath = ".");
    void cleanup();
    
    // 导航
    bool changeDirectory(const std::string& path);
    bool enterDirectory(const std::string& name);
    bool goUp();
    
    // 获取文件列表
    const std::vector<FileEntry>& getFiles() const { return files; }
    std::string getCurrentPath() const { return currentPath; }
    
    // 选择
    int getSelectedIndex() const { return selectedIndex; }
    void setSelectedIndex(int index);
    FileEntry* getSelectedEntry();
    
    // 更新和渲染
    void update();
    void render();
    
    bool isActive() const { return active; }
    void setActive(bool a) { active = a; }
    
    // 设置过滤模式
    enum FilterMode {
        FILTER_NONE,      // 显示所有文件
        FILTER_NDS_ONLY,  // 只显示NDS文件
        FILTER_PNG_ONLY   // 只显示PNG文件
    };
    void setFilterMode(FilterMode mode) { filterMode = mode; refreshFileList(); }
    FilterMode getFilterMode() const { return filterMode; }
    
    // 获取选中的文件路径（用于壁纸选择）
    std::string getSelectedFilePath() const;
    
private:
    std::vector<FileEntry> files;
    std::string currentPath;
    int selectedIndex;
    bool active;
    int scrollOffset;
    int maxVisibleItems;
    FilterMode filterMode;
    
    void refreshFileList();
    void sortFiles();
    static bool compareEntries(const FileEntry& a, const FileEntry& b);
    static bool isPNGFile(const std::string& filename);
};

