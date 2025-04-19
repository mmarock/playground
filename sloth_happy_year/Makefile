CFLAGS := -Wall -Werror -g
LDFLAGS := -lm

BIN_DIR ?= build

all: build

build: $(BIN_DIR)/happy_year

clean:
	@rm $(BIN_DIR)/happy_year
	@rmdir $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $@


$(BIN_DIR)/%: %.c $(BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

.PHONY: all build clean
