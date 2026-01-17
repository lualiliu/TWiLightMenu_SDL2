# Makefile for TWiLight Menu++ SDL2 version

# 交叉编译工具链配置
ARCH ?= native
SYSROOT = $(HOME)/toolchain/aarch64-anbernic-linux-gnueabi/sysroot
CROSS_PREFIX = aarch64-linux-gnu-

# 根据架构设置编译器和工具
ifeq ($(ARCH),aarch64)
	CXX = $(CROSS_PREFIX)g++
	STRIP = $(CROSS_PREFIX)strip
	SYSROOT_FLAGS = --sysroot=$(SYSROOT)
	PKG_CONFIG_PATH = $(SYSROOT)/usr/lib/pkgconfig:$(SYSROOT)/usr/share/pkgconfig
	export PKG_CONFIG_PATH
	# 尝试查找sdl2-config
	SDL2_CONFIG := $(shell which $(CROSS_PREFIX)sdl2-config 2>/dev/null || find $(SYSROOT) -name sdl2-config 2>/dev/null | head -1)
	ifeq ($(SDL2_CONFIG),)
		SDL2_CONFIG = $(SYSROOT)/usr/bin/sdl2-config
	endif
	SDL2_CFLAGS = $(shell PKG_CONFIG_SYSROOT_DIR=$(SYSROOT) $(SDL2_CONFIG) --cflags 2>/dev/null || echo "-I$(SYSROOT)/usr/include/SDL2")
	SDL2_LIBS = $(shell PKG_CONFIG_SYSROOT_DIR=$(SYSROOT) $(SDL2_CONFIG) --libs 2>/dev/null || echo "-L$(SYSROOT)/usr/lib -lSDL2")
	LDFLAGS_EXTRA = -L$(SYSROOT)/usr/lib -Wl,-rpath-link,$(SYSROOT)/usr/lib
	PIC_FLAG = -fPIC
else
	CXX = g++
	STRIP = strip
	SYSROOT_FLAGS =
	SDL2_CFLAGS = $(shell sdl2-config --cflags)
	SDL2_LIBS = $(shell sdl2-config --libs)
	LDFLAGS_EXTRA =
	PIC_FLAG =
endif

CXXFLAGS = -std=c++17 -Wall -Wextra -O2 $(SYSROOT_FLAGS) $(PIC_FLAG)
LDFLAGS = $(SYSROOT_FLAGS) $(LDFLAGS_EXTRA)

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

# 显示当前配置
info:
	@echo "当前架构: $(ARCH)"
	@echo "编译器: $(CXX)"
	@echo "Sysroot: $(SYSROOT)"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"

# 帮助信息
help:
	@echo "使用方法:"
	@echo "  本地编译:  make"
	@echo "  aarch64交叉编译:  make ARCH=aarch64"
	@echo "  清理:      make clean"
	@echo "  查看配置:  make info"
	@echo ""
	@echo "交叉编译时需要确保工具链和sysroot路径正确:"
	@echo "  Sysroot: $(SYSROOT)"

.PHONY: all clean install info help

