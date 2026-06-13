# USB 枚举链路

## 观察命令

插入设备前：

```bash
lsusb
dmesg | tail -n 30
```

插入设备时开两个终端：

```bash
dmesg -w
```

```bash
udevadm monitor --kernel --property
```

插入设备后：

```bash
lsusb
lsusb -v -d <vid:pid>
```

## 关键字段

```text
idVendor / idProduct
  厂商和产品 ID，用来识别具体设备。

bDeviceClass
  设备级类别。有些设备在设备级声明类别，有些在接口级声明。

bNumConfigurations
  设备支持多少个配置。

bInterfaceClass
  接口类别，例如 Video、CDC、HID、Mass Storage。

Endpoint Descriptor
  看端点方向 IN/OUT，传输类型 Control/Bulk/Interrupt/Isochronous。
```

## 设备节点对应关系

```text
UVC Camera
  -> /dev/video*
  -> uvcvideo

USB Serial
  -> /dev/ttyUSB* 或 /dev/ttyACM*
  -> usbserial / ch341 / cp210x / cdc_acm

HID
  -> /dev/input/event*
  -> usbhid

USB Storage
  -> /dev/sd*
  -> usb-storage / uas
```

## 排错顺序

1. `lsusb` 看不到：先查供电、线缆、接口、Host 控制器。
2. `lsusb` 看得到但没有节点：查设备类驱动是否加载。
3. 有节点但应用打不开：查权限、占用、设备格式、应用参数。
4. 设备反复断开：查供电、电缆、带宽、内核日志中的 reset。
