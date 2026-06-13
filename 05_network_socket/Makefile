CC ?= gcc
BUILD_DIR := build
CFLAGS += -Wall -Wextra -O2

TARGETS := tcp_server tcp_client udp_receiver udp_sender

all: $(addprefix $(BUILD_DIR)/,$(TARGETS))

$(BUILD_DIR)/%: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
