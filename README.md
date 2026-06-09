# linux_learn

Orange Pi RK3566 embedded Linux driver learning code.

## Current modules

```text
01_hello_kernel/
  Minimal Linux kernel module.

02_mychardev/
  Basic character device driver.

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
