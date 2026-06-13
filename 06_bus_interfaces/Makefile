CC ?= gcc
BUILD_DIR := build
CFLAGS += -Wall -Wextra -O2

TARGETS := uart_echo i2c_read_reg spi_transfer

all: $(addprefix $(BUILD_DIR)/,$(TARGETS))

$(BUILD_DIR)/%: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
