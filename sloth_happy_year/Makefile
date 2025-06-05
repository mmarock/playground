CFLAGS := -Wall -Werror -g
LDFLAGS := -lm

BIN_DIR ?= build

all: $(BIN_DIR)/happy_year

clean:
	@rm $(BIN_DIR)/happy_year
	@rmdir $(BIN_DIR)

$(BIN_DIR)/%: %.c
	@mkdir -p $@
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

.PHONY: all build clean
