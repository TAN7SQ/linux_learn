ARCH := arm64
TOOLCHAIN_DIR := /home/tans/workspace/orangepi-build/toolchains/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu
CROSS_COMPILE := $(TOOLCHAIN_DIR)/bin/aarch64-none-linux-gnu-
LOCALVERSION := -rockchip-rk356x

KERNEL_SRC := /home/tans/workspace/orangepi-build/kernel/orange-pi-5.10-rk35xx
KERNEL_BUILD := /home/tans/workspace/kernel_build/rk3566_cm4

MODULE_NAME := hello_kernel
BUILD_DIR := $(CURDIR)/build

all:
	mkdir -p $(BUILD_DIR)
	ln -sf ../$(MODULE_NAME).c $(BUILD_DIR)/$(MODULE_NAME).c # 软连接
	printf 'obj-m := $(MODULE_NAME).o\n' > $(BUILD_DIR)/Makefile
	make -C $(KERNEL_SRC) \
		O=$(KERNEL_BUILD) \
		ARCH=$(ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		LOCALVERSION=$(LOCALVERSION) \
		M=$(BUILD_DIR) \
		modules

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
