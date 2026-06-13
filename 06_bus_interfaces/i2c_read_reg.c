// SPDX-License-Identifier: GPL-2.0

#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int parse_u8(const char *text, unsigned int *value)
{
    char *end = NULL;
    unsigned long parsed = strtoul(text, &end, 0);

    if (*text == '\0' || *end != '\0' || parsed > 0xff)
        return -1;

    *value = (unsigned int)parsed;
    return 0;
}

int main(int argc, char **argv)
{
    const char *dev;
    unsigned int addr;
    unsigned int reg;
    unsigned char reg_byte;
    unsigned char value;
    int fd;

    if (argc != 4) {
        fprintf(stderr, "usage: %s <i2c-dev> <7bit-addr> <reg>\n", argv[0]);
        return 1;
    }

    dev = argv[1];
    if (parse_u8(argv[2], &addr) < 0 || addr > 0x7f) {
        fprintf(stderr, "invalid 7-bit i2c address: %s\n", argv[2]);
        return 1;
    }
    if (parse_u8(argv[3], &reg) < 0) {
        fprintf(stderr, "invalid register: %s\n", argv[3]);
        return 1;
    }

    fd = open(dev, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        perror("ioctl I2C_SLAVE");
        close(fd);
        return 1;
    }

    reg_byte = (unsigned char)reg;
    if (write(fd, &reg_byte, 1) != 1) {
        perror("write reg");
        close(fd);
        return 1;
    }

    if (read(fd, &value, 1) != 1) {
        perror("read value");
        close(fd);
        return 1;
    }

    printf("%s addr=0x%02x reg=0x%02x value=0x%02x\n", dev, addr, reg, value);
    close(fd);
    return 0;
}
