BIN_DIR := $(PWD)/build
PROJECTS := sloth_happy_year

all: $(foreach var,$(PROJECTS),build_$(var))

clean: $(foreach var,$(PROJECTS),clean_$(var))
	@rmdir $(BIN_DIR)

build_%: % %/Makefile
	@mkdir -p $(BIN_DIR)
	$(MAKE) BIN_DIR=$(BIN_DIR)/$< -C $< all

clean_%: % %/Makefile $(BIN_DIR)
	$(MAKE) BIN_DIR=$(BIN_DIR)/$< -C $< clean

.PHONY: all build clean
