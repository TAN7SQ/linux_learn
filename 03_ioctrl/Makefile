ARCH := arm64
TOOLCHAIN_DIR := /home/tans/workspace/orangepi-build/toolchains/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu
CROSS_COMPILE := $(TOOLCHAIN_DIR)/bin/aarch64-none-linux-gnu-
LOCALVERSION := -rockchip-rk356x
USER_CC := aarch64-linux-gnu-gcc
USER_APP := ioctrl_test

KERNEL_SRC := /home/tans/workspace/orangepi-build/kernel/orange-pi-5.10-rk35xx
KERNEL_BUILD := /home/tans/workspace/kernel_build/rk3566_cm4

MODULE := ioctrl_basic
BUILD_DIR := $(CURDIR)/build

obj-m := $(MODULE).o

all:  modules user

modules:
	$(MAKE) -C $(KERNEL_SRC) \
		O=$(KERNEL_BUILD) \
		ARCH=$(ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		LOCALVERSION=$(LOCALVERSION) \
		M=$(CURDIR) \
		modules
	@mkdir -p $(BUILD_DIR)
	@for f in $(MODULE).ko $(MODULE).o $(MODULE).mod $(MODULE).mod.c $(MODULE).mod.o Module.symvers modules.order .$(MODULE).* .Module.symvers.cmd .modules.order.cmd; do \
		if [ -e "$$f" ]; then mv "$$f" $(BUILD_DIR)/; fi; \
	done

user:
	@mkdir -p $(BUILD_DIR)
	$(USER_CC) -Wall -Wextra -O2 -o $(BUILD_DIR)/$(USER_APP) $(USER_APP).c

check:
	modinfo $(BUILD_DIR)/$(MODULE).ko | grep -E '^(filename|name|vermagic):'
	file $(BUILD_DIR)/$(USER_APP)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all modules user check clean
