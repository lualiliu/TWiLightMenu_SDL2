cmake_minimum_required(VERSION 3.10)
project(TWiLightMenu_SDL2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

# 包含目录
include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

# 源文件
set(SOURCES
    main.cpp
    graphics/graphics.cpp
    graphics/fontHandler.cpp
    graphics/textRenderer.cpp
    input.cpp
    menu.cpp
    fpsCounter.cpp
    fileBrowse.cpp
    fileBrowser.cpp
    language.cpp
    sound.cpp
    settings.cpp
    resourceManager.cpp
    dsiUI.cpp
    gameGrid.cpp
)

# 可执行文件
add_executable(twilightmenu_sdl2 ${SOURCES})

# 链接库
target_link_libraries(twilightmenu_sdl2
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
)

# 编译选项
if(WIN32)
    target_link_libraries(twilightmenu_sdl2 mingw32)
endif()

