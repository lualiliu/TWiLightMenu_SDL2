#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <map>
#include <memory>

// 资源管理器 - 加载和管理Classic DS Menu主题资源
class ResourceManager {
public:
    static void init(SDL_Renderer* renderer);
    static void cleanup();
    
    // 加载图片资源
    static SDL_Texture* loadImage(const std::string& path);
    static SDL_Texture* loadImageFromTheme(const std::string& relativePath);
    
    // 获取主题路径
    static std::string getThemePath();
    static void setThemePath(const std::string& path);
    
    // 预加载常用资源
    static void preloadCommonResources();
    
    // 缓存管理
    static void clearCache();
    
private:
    static SDL_Renderer* renderer;
    static std::string themeBasePath;
    static std::map<std::string, SDL_Texture*> textureCache;
    static SDL_Texture* getCachedTexture(const std::string& key);
    static void cacheTexture(const std::string& key, SDL_Texture* texture);
};

