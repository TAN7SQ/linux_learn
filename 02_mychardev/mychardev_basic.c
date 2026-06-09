// SPDX-License-Identifier: GPL-2.0
/*
 * mychardev_basic.c
 *
 * 这是字符设备的第一阶段学习代码：只实现 open/read/write/release。
 *
 * 用户态访问路径：
 *   cat /dev/mychardev
 *   echo "hello" > /dev/mychardev
 *
 * 内核态分发路径：
 *   VFS -> struct file_operations -> 本文件中的驱动函数
 */

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>   // DEFINE_MUTEX, mutex_lock_interruptible
#include <linux/uaccess.h> // copy_to_user, copy_from_user

#define DEVICE_NAME "mychardev"
#define CLASS_NAME "mychardev_class"
#define BUFFER_SIZE 128

// 内部简单缓冲区，read会把它复制到用户态，write会把用户态数据复制进来
static char mychardev_buf[BUFFER_SIZE] = "mychardev_basic: hello from kernel\n";
static size_t mychardev_len = sizeof("mychardev_basic: hello from kernel\n") - 1;
static DEFINE_MUTEX(mychardev_lock);

/*
 * dev_t 保存设备号。
 *
 * 设备号由两部分组成：
 *   major：主设备号，表示哪一个驱动
 *   minor：次设备号，表示这个驱动下的哪一个设备实例
 */

/*
 * struct cdev 表示一个字符设备对象。
 *
 * cdev 会和 file_operations 绑定，VFS 找到这个 cdev 后，
 * 就知道 open/read/write 应该分发到哪些驱动函数。
 */

/*
 * file_operations 是字符设备最核心的分发表。
 */

/**
 * @brief open 函数
 * @param inode 设备节点
 * @param file 文件对象
 * @return int 0 成功
 * @note 当用户态 open("/dev/mychardev", ...) 时，VFS 会调用这里。
 * */
static int mychardev_open(struct inode *inode,
                          struct file *file)
{
    pr_info("mychardev_basic: open major=%u minor=%u\n", imajor(inode), iminor(inode));
    return 0;
}

/**
 * @brief read 函数
 * @param file 文件对象
 * @param user_buf 用户态缓冲区地址，它带有 __user 标记，不能直接解引用。
 * @param count 用户态希望最多读取多少字节
 * @param ppos 文件偏移，cat 会反复 read，直到驱动返回 0。
 * @return ssize_t 读取的字节数
 * @note 当用户态 read(fd, user_buf, count) 或 cat /dev/mychardev 时调用这里。
 * */
static ssize_t mychardev_read(struct file *file,
                              char __user *user_buf,
                              size_t count, loff_t *ppos)
{
    size_t available;
    size_t to_copy;
    ssize_t ret;

    if (mutex_lock_interruptible(&mychardev_lock))
        return -ERESTARTSYS;
    // 读取时被中断，返回 -ERESTARTSYS，表示需要重新读取

    // 如果偏移已经到达缓冲区末尾，返回 0。
    if (*ppos < 0) {
        ret = -EINVAL;
        goto out_unlock;
    }

    if ((size_t)*ppos >= mychardev_len) {
        ret = 0;
        goto out_unlock;
    }

    available = mychardev_len - *ppos;
    to_copy = min(count, available);

    // 不能使用memcpy，因为用户态缓冲区和内核态缓冲区的地址空间是不同的
    if (copy_to_user(user_buf, mychardev_buf + *ppos, to_copy)) {
        ret = -EFAULT;
        goto out_unlock;
    }

    *ppos += to_copy;
    ret = to_copy;

    pr_info("mychardev_basic: read %zd bytes, ppos = %lld\n", ret, *ppos);

out_unlock:
    mutex_unlock(&mychardev_lock);
    return ret;
}

/**
 * @brief write 函数
 * @param file 文件对象
 * @param user_buf 用户态缓冲区地址，它带有 __user 标记，不能直接解引用。
 * @param count 用户态希望最多写入多少字节
 * @param ppos 文件偏移
 * @return ssize_t 写入的字节数
 * @note 当用户态 write(fd, user_buf, count) 或 echo "hello" > /dev/mychardev 时调用这里。
 * */
static ssize_t mychardev_write(struct file *file,
                               const char __user *user_buf,
                               size_t count, loff_t *ppos)
{
    size_t to_copy;
    ssize_t ret;

    if (mutex_lock_interruptible(&mychardev_lock))
        return -ERESTARTSYS;

    /*
     * 缓冲区最后一个字节留给 '\0'，方便按字符串打印。
     * 这不是字符设备的硬性要求，只是本学习代码为了看日志更直观。
     */
    to_copy = min(count, (size_t)BUFFER_SIZE - 1);

    /*
     * memset清空缓冲区不是字符设备必须要求。
     * 这里主要是为了避免旧内容残留，方便后面按字符串打印日志。
     * 真正有效数据长度由 mychardev_len 记录。
     */
    memset(mychardev_buf, 0, sizeof(mychardev_buf));

    if (copy_from_user(mychardev_buf, user_buf, to_copy)) {
        ret = -EFAULT;
        goto out_unlock;
    }

    mychardev_buf[to_copy] = '\0';
    mychardev_len = to_copy;
    ret = to_copy;

    pr_info("mychardev_basic: write %zu bytes: %s", to_copy, mychardev_buf);

out_unlock:
    mutex_unlock(&mychardev_lock);
    return ret;
}

/**
 * @brief release 函数
 * @param inode 设备节点
 * @param file 文件对象
 * @return int 0 成功
 * @note 当用户态 close(fd) 时，VFS 会调用这里。
 * */
static int mychardev_release(struct inode *inode,
                             struct file *file)
{
    pr_info("mychardev_basic: release\n");
    return 0;
}

//保存设备号,有主设备号(表示驱动)和次设备号(表示设备实例)两部分组成
static dev_t mychardev_devno;
// 保存字符设备对象，用于绑定 file_operations
static struct cdev mychardev_cdev;
static const struct file_operations mychardev_fops = {
    .owner = THIS_MODULE,
    .open = mychardev_open,
    .read = mychardev_read,
    .write = mychardev_write,
    .release = mychardev_release,
};
// class 和 device 用来配合系统创建设备节点。
//  * cdev_add 之后，内核知道有这个字符设备；
//  * device_create 之后，用户态通常能看到 /dev/mychardev。
static struct class *mychardev_class;
static struct device *mychardev_device;
static int __init mychardev_init(void)
{
    int ret;

    // 1. 自动申请一个设备号
    ret = alloc_chrdev_region(&mychardev_devno, 0, 1, DEVICE_NAME);
    if (ret) {
        pr_err("mychardev_init: alloc_chrdev_region failed, ret = %d\n", ret);
        return ret;
    }
    pr_info("mychardev_basic: allocated major=%u minor=%u\n",
            MAJOR(mychardev_devno),
            MINOR(mychardev_devno));

    // 2. 初始化 cdev，并绑定 file_operations。
    cdev_init(&mychardev_cdev, &mychardev_fops);
    mychardev_cdev.owner = THIS_MODULE; // 设置 owner 为当前模块，防止其他模块使用

    // 3. 添加 cdev 到内核
    ret = cdev_add(&mychardev_cdev, mychardev_devno, 1);
    if (ret) {
        pr_err("mychardev_init: cdev_add failed, ret = %d\n", ret);
        goto err_unregister_devno;
    }

    // 4. 注册设备节点
    mychardev_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mychardev_class)) {
        ret = PTR_ERR(mychardev_class);
        pr_err("mychardev_basic: class_create failed: %d\n", ret);
        goto err_del_cdev;
    }

    // 5. 创建设备对象，通常会触发/dev/mychardev的创建
    mychardev_device = device_create(mychardev_class, NULL, mychardev_devno, NULL, DEVICE_NAME);
    if (IS_ERR(mychardev_device)) {
        ret = PTR_ERR(mychardev_device);
        pr_err("mychardev_basic: device_create failed, ret = %d\n", ret);
        goto err_destroy_class;
    }

    // 6. 创建成功打印加载信息
    pr_info("mychardev_basic: loaded, device=/dev/%s\n", DEVICE_NAME);
    return 0;

err_destroy_class:
    class_destroy(mychardev_class);
err_del_cdev:
    cdev_del(&mychardev_cdev);
err_unregister_devno:
    unregister_chrdev_region(mychardev_devno, 1);
    return ret;
}

static void __exit mychardev_exit(void)
{
    /*
     * 卸载顺序必须和加载顺序相反。
     */
    device_destroy(mychardev_class, mychardev_devno);
    class_destroy(mychardev_class);
    cdev_del(&mychardev_cdev);
    unregister_chrdev_region(mychardev_devno, 1);

    pr_info("mychardev_basic: unloaded\n");
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tans");
MODULE_DESCRIPTION("---->A basic character device driver<----");