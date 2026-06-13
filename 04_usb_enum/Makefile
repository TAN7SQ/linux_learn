CC ?= gcc
BUILD_DIR := build
TARGET := usb_list_devices

CFLAGS += -Wall -Wextra -O2
LDLIBS += $(shell pkg-config --libs libusb-1.0)
CFLAGS += $(shell pkg-config --cflags libusb-1.0)

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): usb_list_devices.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
