# linux_learn

Orange Pi RK3566 embedded Linux learning code.

The current path is not to keep adding toy kernel drivers. Character-device
code is used as the kernel entry point, then the main focus moves to USB,
Ethernet, I2C/SPI/UART, camera/display, and user-space system debugging.

## Current modules

```text
01_hello_kernel/
  Minimal Linux kernel module.

02_mychardev/
  Basic character device driver.

03_ioctrl/
  Character device closeout: read/write/ioctl plus user-space smoke test.

04_usb_enum/
  USB enumeration, descriptor reading, and device-node debugging.

05_network_socket/
  TCP/UDP user-space demos and Ethernet debug checklist.

06_bus_interfaces/
  UART/I2C/SPI user-space bus tools and debug checklist.

tools/
  Local helper scripts for development.
```

## Target platform

```text
Board: Rockchip RK3566 Orange Pi CM4 Board
Board kernel: 5.10.160-rockchip-rk356x
Build host: Ubuntu 22.04
```

## Build notes

The module Makefiles expect the kernel source, kernel build output, and cross
compiler to exist on the build host.

User-space examples use the native board compiler by default. For cross
compilation, override `CC` or the directory-specific `USER_CC`:

```bash
make CC=aarch64-linux-gnu-gcc
make USER_CC=aarch64-linux-gnu-gcc
```

Generated build output is intentionally ignored:

```text
learn_doc/
build/
**/build/
.cache/
*.ko
*.o
*.mod.c
```

The learning notes are kept locally and are not included in this GitHub repo.

## Stage checks

Each stage should leave runnable evidence:

```text
commands
logs
source code
README notes
```
