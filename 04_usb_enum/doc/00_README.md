# 04_usb_enum 学习入口

## 学习目标

这一阶段不要求手写 USB Host 驱动。目标是掌握 USB 外设从插入到用户态可见的全过程。

```text
USB 外设插入
  -> Host Controller 检测连接
  -> 内核读取设备描述符
  -> 选择配置、接口、端点
  -> 匹配 UVC / CDC ACM / HID / Mass Storage 等类驱动
  -> 用户态出现 /dev/video* / /dev/ttyUSB* / /dev/input/event* / block device
```

## 你要学习什么

- USB 设备描述符、配置描述符、接口描述符、端点描述符。
- `idVendor`、`idProduct`、`bDeviceClass`、`bInterfaceClass`。
- 常见设备类：UVC、CDC ACM、HID、Mass Storage。
- `lsusb`、`lsusb -v`、`dmesg`、`udevadm monitor`。
- `libusb` 如何从用户态读取设备描述符。

## 本阶段代码

```text
04_usb_enum/usb_list_devices.c
04_usb_enum/Makefile
```

## 构建

```bash
cd /home/tans/workspace/linux_drivers/04_usb_enum
sudo apt install libusb-1.0-0-dev pkg-config
make
./build/usb_list_devices
```

## 最小产出

- 至少记录三类 USB 设备：UVC 摄像头、USB 串口、U 盘或 HID。
- 每类设备都要保存 `lsusb`、`lsusb -v`、`dmesg` 关键信息。
- 能解释为什么 `lsusb` 能看到设备，但用户态节点可能没有出现。
