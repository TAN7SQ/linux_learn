/* SPDX-License-Identifier: GPL-2.0 */
#ifndef MYCHARDEV_IOCTL_H
#define MYCHARDEV_IOCTL_H

#include <linux/ioctl.h>
#include <linux/types.h>

/*
 * 这个结构体由内核态填充，用户态读取。
 *
 * 注意：
 *   共享给用户态的结构体尽量使用固定宽度类型，例如 __u32。
 *   不要在这种接口结构体里使用 long、指针、size_t 这类容易受 ABI 影响的类型。
 */
struct mychardev_status
{
    __u32 buffer_len;
    __u32 write_count;
    __u32 read_count;
};

// ioctl magic 用来给一组 ioctl 命令分类。
// 这里用 'm' 表示，你可以根据需要修改。
#define MYCHARDEV_IOCTL_MAGIC 'm'

// _IOR, I 表示ioctl，R 表示站在用户态角度，从内核态读取。
#define MYCHARDEV_IOCTL_GET_STATUS \
    _IOR(MYCHARDEV_IOCTL_MAGIC, 0x01, struct mychardev_status)

// _IO: 表示 ioctl 命令没有参数，用户态和内核态都直接交互。
#define MYCHARDEV_IOCTL_CLEAR_BUFFER \
    _IO(MYCHARDEV_IOCTL_MAGIC, 0x02)

#endif