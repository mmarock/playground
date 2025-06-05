BIN_DIR := build
PROJECTS := sloth_happy_year snake

all: $(foreach var,$(PROJECTS),build_$(var))

clean: $(foreach var,$(PROJECTS),clean_$(var))

build_%: % %/Makefile
	$(MAKE) BIN_DIR=$(BIN_DIR) -C $< all

clean_%: % %/Makefile $(BIN_DIR)
	$(MAKE) BIN_DIR=$(BIN_DIR) -C $< clean

.PHONY: all build clean
