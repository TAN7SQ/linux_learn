# 03_ioctrl 阶段收口

## 本阶段目标

把字符设备学习收口到 `read/write/ioctl + 用户态测试`，不再继续扩展复杂字符设备玩具模块。

本阶段要能讲清楚：

```text
open("/dev/mychardev")
  -> VFS
  -> inode / struct file
  -> cdev
  -> file_operations
  -> read/write/unlocked_ioctl
```

## 本阶段代码

```text
03_ioctrl/ioctrl_basic.c
03_ioctrl/ioctrl_basic.h
03_ioctrl/ioctrl_test.c
03_ioctrl/Makefile
```

## 构建

在 Ubuntu 构建主机上：

```bash
cd /home/tans/workspace/linux_drivers/03_ioctrl
make
make check
```

只编译用户态测试程序：

```bash
make USER_CC=aarch64-linux-gnu-gcc user
```

## 板卡验证

在 Orange Pi CM4 上：

```bash
cd /home/tans/workspace/linux_drivers/03_ioctrl
sudo insmod build/ioctrl_basic.ko
ls -l /dev/mychardev
sudo ./build/ioctrl_test
dmesg | tail -n 40
sudo rmmod ioctrl_basic
```

## 必须观察到的现象

- `/dev/mychardev` 被创建。
- `ioctrl_test` 能打印 `initial`、`after read/write`、`after clear` 三组状态。
- `after clear` 后 `buffer_len`、`write_count`、`read_count` 应回到清空后的状态。
- `dmesg` 中能看到 `open`、`read`、`write`、`ioctl`、`release` 日志。

## 面试追问

1. 为什么 `ioctl` 里用户态指针不能直接解引用？
2. 为什么共享给用户态的结构体尽量用 `__u32`，不要用 `long`、指针、`size_t`？
3. `read/write` 里为什么要用 `copy_to_user` 和 `copy_from_user`？
4. `device_create` 和 `cdev_add` 分别解决什么问题？
5. `mutex_lock_interruptible` 返回 `-ERESTARTSYS` 代表什么？

## 完成门槛

能不用看稿说清楚：

```text
/dev -> fd -> struct file -> cdev -> file_operations -> ioctl handler
```
