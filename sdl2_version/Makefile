# Makefile for TWiLight Menu++ SDL2 version

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = 

# SDL2库
SDL2_CFLAGS = $(shell sdl2-config --cflags)
SDL2_LIBS = $(shell sdl2-config --libs)

# SDL2_image、SDL2_mixer和SDL2_ttf
SDL2_IMAGE_LIBS = -lSDL2_image
SDL2_MIXER_LIBS = -lSDL2_mixer
SDL2_TTF_LIBS = -lSDL2_ttf

# 源文件
SOURCES = main.cpp \
          graphics/graphics.cpp \
          graphics/fontHandler.cpp \
          graphics/textRenderer.cpp \
          input.cpp \
          menu.cpp \
          fpsCounter.cpp \
          fileBrowse.cpp \
          fileBrowser.cpp \
          language.cpp \
          sound.cpp \
          settings.cpp \
          resourceManager.cpp \
          dsiUI.cpp \
          gameGrid.cpp \
          ndsIconLoader.cpp

# 对象文件
OBJECTS = $(SOURCES:.cpp=.o)

# 可执行文件
TARGET = twilightmenu_sdl2

# 默认目标
all: $(TARGET)

# 链接
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(SDL2_LIBS) $(SDL2_IMAGE_LIBS) $(SDL2_MIXER_LIBS) $(SDL2_TTF_LIBS) $(LDFLAGS)

# 编译规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(SDL2_CFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(OBJECTS) $(TARGET)

# 安装 (可选)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

.PHONY: all clean install

