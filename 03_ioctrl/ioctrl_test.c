// SPDX-License-Identifier: GPL-2.0
/*
 * User-space smoke test for ioctrl_basic.ko.
 *
 * Run on the Orange Pi CM4 after loading the module:
 *   sudo insmod build/ioctrl_basic.ko
 *   sudo ./build/ioctrl_test
 *   dmesg | tail -n 30
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ioctrl_basic.h"

#define DEVICE_PATH "/dev/mychardev"

static int print_status(int fd, const char *tag)
{
    struct mychardev_status status;

    if (ioctl(fd, MYCHARDEV_IOCTL_GET_STATUS, &status) < 0) {
        perror("ioctl GET_STATUS");
        return -1;
    }

    printf("%s: buffer_len=%u write_count=%u read_count=%u\n",
           tag,
           status.buffer_len,
           status.write_count,
           status.read_count);
    return 0;
}

int main(void)
{
    const char message[] = "hello from user ioctl test\n";
    char buffer[128];
    int fd;
    ssize_t ret;

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open " DEVICE_PATH);
        return 1;
    }

    if (print_status(fd, "initial") < 0)
        goto fail;

    ret = write(fd, message, strlen(message));
    if (ret < 0) {
        perror("write");
        goto fail;
    }
    printf("write: %zd bytes\n", ret);

    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek");
        goto fail;
    }

    memset(buffer, 0, sizeof(buffer));
    ret = read(fd, buffer, sizeof(buffer) - 1);
    if (ret < 0) {
        perror("read");
        goto fail;
    }
    printf("read: %zd bytes: %s", ret, buffer);

    if (print_status(fd, "after read/write") < 0)
        goto fail;

    if (ioctl(fd, MYCHARDEV_IOCTL_CLEAR_BUFFER) < 0) {
        perror("ioctl CLEAR_BUFFER");
        goto fail;
    }

    if (print_status(fd, "after clear") < 0)
        goto fail;

    close(fd);
    return 0;

fail:
    close(fd);
    return 1;
}
