// SPDX-License-Identifier: GPL-2.0
/*
 * mychardev_ioctl.c
 *
 * 这是字符设备第二阶段学习代码：在 read/write 基础上加入 ioctl。
 */

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#include "ioctrl_basic.h" // 包含 ioctl 命令定义

#define DEVICE_NAME "mychardev"
#define CLASS_NAME "mychardev_class"
#define BUFFER_SIZE 128

static char mychardev_buf[BUFFER_SIZE] = "mychardev_ioctl: hello from kernel\n";
static size_t mychardev_len = sizeof("mychardev_ioctl: hello from kernel\n") - 1;
static unsigned int write_count;
static unsigned int read_count;

static DEFINE_MUTEX(mychardev_lock);

/***************************************************** */
static int mychardev_open(struct inode *inode, struct file *file)
{
    pr_info("mychardev_open: inode=%p, file=%p\n", inode, file);
    return 0;
}

static ssize_t mychardev_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    size_t available;
    size_t to_copy;
    ssize_t ret;

    if (mutex_lock_interruptible(&mychardev_lock))
        return -ERESTARTSYS;

    if (*ppos >= mychardev_len) {
        ret = 0;
        goto out_unlock;
    }

    available = mychardev_len - *ppos;
    to_copy = min(count, available);

    if (copy_to_user(user_buf, mychardev_buf + *ppos, to_copy)) {
        ret = -EFAULT;
        goto out_unlock;
    }

    *ppos += to_copy;
    read_count++;
    ret = to_copy;

    pr_info("mychardev_ioctl: read %zu bytes, read_count=%u\n",
            to_copy,
            read_count);
    return ret;

out_unlock:
    mutex_unlock(&mychardev_lock);
    return ret;
}

static ssize_t mychardev_write(struct file *file,
                               const char __user *user_buf,
                               size_t count, loff_t *ppos)
{

    size_t to_copy;
    ssize_t ret;

    if (mutex_lock_interruptible(&mychardev_lock))
        return -ERESTARTSYS;

    to_copy = min(count, (size_t)BUFFER_SIZE - 1);
    memset(mychardev_buf, 0, sizeof(mychardev_buf));

    if (copy_from_user(mychardev_buf, user_buf, to_copy)) {
        ret = -EFAULT;
        goto out_unlock;
    }

    mychardev_buf[to_copy] = '\0';
    mychardev_len = to_copy;
    write_count++;
    ret = to_copy;

    pr_info("mychardev_write: count=%zu, ppos=%lld\n", count, *ppos);
    return ret;

out_unlock:
    mutex_unlock(&mychardev_lock);
    return ret;
}

/**
 * ioctl:
 * 用户态 ioctl(fd, cmd, arg) 时，VFS 会调用这里。
 * cmd:
 *   命令编号，例如 MYCHARDEV_IOCTL_GET_STATUS。
 * arg:
 *   用户态传入的参数。它本质上是一个整数，常被当成用户态指针使用。
 *   只要它表示用户态地址，就必须用 copy_to_user/copy_from_user。
 */
static long mychardev_ioctl(struct file *file,
                            unsigned int cmd,
                            unsigned long arg)
{
    struct mychardev_status status;
    long ret;

    switch (cmd) {
    case MYCHARDEV_IOCTL_GET_STATUS:
        if (mutex_lock_interruptible(&mychardev_lock))
            return -ERESTARTSYS;

        status.buffer_len = mychardev_len;
        status.write_count = write_count;
        status.read_count = read_count;

        mutex_unlock(&mychardev_lock);
        if (copy_to_user((void __user *)arg, &status, sizeof(status)))
            return -EFAULT;
        pr_info("mychardev_ioctl: ioctl GET_STATUS, status=%u, %u, %u\n",
                status.buffer_len,
                status.write_count,
                status.read_count);
        break;
    case MYCHARDEV_IOCTL_CLEAR_BUFFER:
        if (mutex_lock_interruptible(&mychardev_lock))
            return -ERESTARTSYS;
        memset(mychardev_buf, 0, sizeof(mychardev_buf));
        mychardev_len = sizeof("mychardev_ioctl: hello from kernel\n") - 1;
        write_count = 0;
        read_count = 0;
        mutex_unlock(&mychardev_lock);
        pr_info("mychardev_ioctl: ioctl CLEAR_BUFFER\n");
        break;

    default:
        // -ENOTTY 表示 ioctl 命令编号无效。
        ret = -ENOTTY;
        break;
    }

    pr_info("mychardev_ioctl: cmd=%d, arg=%ld\n", cmd, arg);
    return ret;
}

static int mychardev_release(struct inode *inode, struct file *file)
{
    pr_info("mychardev_release: inode=%p, file=%p\n", inode, file);
    return 0;
}

/***************************************************** */

static dev_t mychardev_devno;
static struct cdev mychardev_cdev;
static struct class *mychardev_class;

static const struct file_operations mychardev_fops = {
    .owner = THIS_MODULE,
    .open = mychardev_open,
    .read = mychardev_read,
    .write = mychardev_write,
    .unlocked_ioctl = mychardev_ioctl,
    .release = mychardev_release,
};

static int __init mychardev_init(void)
{
    int ret;

    /* TODO 14: 使用 alloc_chrdev_region 申请设备号。 */
    ret = alloc_chrdev_region(&mychardev_devno, 0, 1, DEVICE_NAME);
    if (ret) {
        pr_err("mychardev_ioctl_todo: alloc_chrdev_region failed, ret=%d\n", ret);
        return ret;
    }

    /* TODO 15: cdev_init，并设置 owner。 */
    cdev_init(&mychardev_cdev, &mychardev_fops);
    mychardev_cdev.owner = THIS_MODULE;

    /* TODO 16: cdev_add，把设备号和 cdev 注册进内核。 */
    ret = cdev_add(&mychardev_cdev, mychardev_devno, 1);
    if (ret) {
        pr_err("mychardev_ioctl_todo: cdev_add failed, ret=%d\n", ret);
        goto ret_chrdev_region;
    }

    /* TODO 17: class_create。 */
    mychardev_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mychardev_class)) {
        ret = PTR_ERR(mychardev_class);
        pr_err("mychardev_ioctl_todo: class_create failed, ret=%d\n", ret);
        goto ret_destroy_cdev;
    }

    /* TODO 18: device_create，生成 /dev/mychardev_todo。 */
    if (IS_ERR(device_create(mychardev_class, NULL, mychardev_devno, NULL, DEVICE_NAME))) {
        ret = PTR_ERR(device_create(mychardev_class, NULL, mychardev_devno, NULL, DEVICE_NAME));
        pr_err("mychardev_ioctl_todo: device_create failed, ret=%d\n", ret);
        ret = -ENOMEM;
        goto ret_destroy_class;
    }

    pr_info("mychardev_ioctl_todo: loaded, device=/dev/%s major=%u minor=%u\n",
            DEVICE_NAME,
            MAJOR(mychardev_devno),
            MINOR(mychardev_devno));
    return 0;

    /*
     * TODO 19:
     * 按失败位置补完整错误处理标签。
     * 顺序参考：
     *   device_create 失败 -> class_destroy -> cdev_del -> unregister_chrdev_region
     *   class_create 失败  -> cdev_del -> unregister_chrdev_region
     *   cdev_add 失败      -> unregister_chrdev_region
     */
ret_destroy_class:
    class_destroy(mychardev_class);
ret_destroy_cdev:
    cdev_del(&mychardev_cdev);
ret_chrdev_region:
    unregister_chrdev_region(mychardev_devno, 1);
    return ret;
}

static void __exit mychardev_exit(void)
{
    /* TODO 20: 按 init 的反向顺序释放 device、class、cdev、设备号。 */
    device_destroy(mychardev_class, mychardev_devno);
    class_destroy(mychardev_class);
    cdev_del(&mychardev_cdev);
    unregister_chrdev_region(mychardev_devno, 1);

    pr_info("mychardev_ioctl_todo: unloaded\n");
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_AUTHOR("Tans");
MODULE_DESCRIPTION("TODO character device driver: read/write/ioctl");
MODULE_LICENSE("GPL");
