// SPDX-License-Identifier: GPL-2.0

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static speed_t baud_to_speed(int baud)
{
    switch (baud) {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    default:
        return 0;
    }
}

int main(int argc, char **argv)
{
    const char *dev;
    int baud;
    speed_t speed;
    int fd;
    struct termios tty;
    const char tx[] = "uart ping\n";
    char rx[128];
    ssize_t n;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <tty-dev> <baud>\n", argv[0]);
        return 1;
    }

    dev = argv[1];
    baud = atoi(argv[2]);
    speed = baud_to_speed(baud);
    if (speed == 0) {
        fprintf(stderr, "unsupported baud: %s\n", argv[2]);
        return 1;
    }

    fd = open(dev, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return 1;
    }

    cfmakeraw(&tty);
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 20;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return 1;
    }

    if (write(fd, tx, strlen(tx)) < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    memset(rx, 0, sizeof(rx));
    n = read(fd, rx, sizeof(rx) - 1);
    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    printf("rx %zd bytes: %s\n", n, rx);
    close(fd);
    return 0;
}
