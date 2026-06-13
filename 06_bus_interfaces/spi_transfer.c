// SPDX-License-Identifier: GPL-2.0

#include <errno.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MAX_BYTES 64

static int hex_nibble(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static int parse_hex_bytes(const char *text, uint8_t *out, size_t *out_len)
{
    size_t len = strlen(text);
    size_t i;

    if (len == 0 || (len % 2) != 0 || len / 2 > MAX_BYTES)
        return -1;

    for (i = 0; i < len / 2; i++) {
        int hi = hex_nibble(text[i * 2]);
        int lo = hex_nibble(text[i * 2 + 1]);

        if (hi < 0 || lo < 0)
            return -1;
        out[i] = (uint8_t)((hi << 4) | lo);
    }

    *out_len = len / 2;
    return 0;
}

int main(int argc, char **argv)
{
    const char *dev;
    uint32_t speed;
    uint8_t tx[MAX_BYTES];
    uint8_t rx[MAX_BYTES];
    size_t len;
    int fd;
    struct spi_ioc_transfer tr;
    size_t i;

    if (argc != 4) {
        fprintf(stderr, "usage: %s <spidev> <speed-hz> <hex-bytes>\n", argv[0]);
        return 1;
    }

    dev = argv[1];
    speed = (uint32_t)strtoul(argv[2], NULL, 0);
    if (speed == 0) {
        fprintf(stderr, "invalid speed: %s\n", argv[2]);
        return 1;
    }

    memset(tx, 0, sizeof(tx));
    memset(rx, 0, sizeof(rx));
    if (parse_hex_bytes(argv[3], tx, &len) < 0) {
        fprintf(stderr, "hex-bytes must be an even-length hex string up to %d bytes\n", MAX_BYTES);
        return 1;
    }

    fd = open(dev, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = (unsigned long)tx;
    tr.rx_buf = (unsigned long)rx;
    tr.len = (uint32_t)len;
    tr.speed_hz = speed;
    tr.bits_per_word = 8;

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("ioctl SPI_IOC_MESSAGE");
        close(fd);
        return 1;
    }

    printf("rx:");
    for (i = 0; i < len; i++)
        printf(" %02x", rx[i]);
    printf("\n");

    close(fd);
    return 0;
}
