CXXFLAGS := -Wall -Werror -std=c++17 -g
LDFLAGS := $(shell pkg-config --libs glfw3 glew)

BIN_DIR ?= build

all: $(BIN_DIR)/snake

clean:
	@rm $(BIN_DIR)/snake
	@rmdir $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $@

$(BIN_DIR)/snake: snake.cc $(BIN_DIR) #thirdparty/stb_image.h
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

# TODO don't add to git if downloaded elsewhere
#thirdparty/stb_image.h:
#	@mkdir -p thirdparty
#	@wget

.PHONY: all build clean
