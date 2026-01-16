#include "fileBrowse.h"
#include "fileBrowser.h"
#include <iostream>

FileBrowser* g_fileBrowser = nullptr;

bool fileBrowseInit() {
    g_fileBrowser = new FileBrowser();
    if (!g_fileBrowser->init(".")) {
        delete g_fileBrowser;
        g_fileBrowser = nullptr;
        return false;
    }
    // 设置默认过滤模式为只显示NDS文件（但允许文件夹导航）
    g_fileBrowser->setFilterMode(FileBrowser::FILTER_NDS_ONLY);
    return true;
}

void fileBrowseCleanup() {
    if (g_fileBrowser) {
        delete g_fileBrowser;
        g_fileBrowser = nullptr;
    }
}

