CXXFLAGS := -Wall -Werror -std=c++17 -g
LDFLAGS := $(shell pkg-config --libs glfw3 glew)

# always write into a build dir
BIN_DIR ?= build

# path to the single file image loader
STB_IMAGE_PATH := "https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image.h"

all: $(BIN_DIR)/snake

clean:
	@rm $(BIN_DIR)/snake
	@rmdir $(BIN_DIR)

$(BIN_DIR)/snake: snake.cc $(BIN_DIR) thirdparty/stb_image.h
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

thirdparty/stb_image.h:
	$(info Download the used single file image loader)
	@mkdir -p thirdparty
	@curl -o $@ -G $(STB_IMAGE_PATH)

.PHONY: all build clean
