# 04_usb_enum

USB learning stage for Orange Pi CM4.

## Goal

Understand the path from plugging in a USB device to seeing a user-space device node.

```text
USB device plug-in
  -> host controller detects attach
  -> kernel enumerates descriptors
  -> device class driver matches
  -> user-space sees /dev/video*, /dev/ttyUSB*, /dev/input/event*, or block device nodes
```

## Devices to test

Record at least three device classes:

```text
UVC camera
USB serial adapter
USB storage or HID keyboard/mouse
```

## Commands

```bash
lsusb
lsusb -v -d <vid:pid>
dmesg -w
udevadm monitor --kernel --property
find /dev -maxdepth 1 -name 'video*' -o -name 'ttyUSB*' -o -name 'input*'
```

Optional trace:

```bash
sudo modprobe usbmon
sudo cat /sys/kernel/debug/usb/usbmon/0u
```

## Build and run

Install libusb development headers on the build host or board:

```bash
sudo apt install libusb-1.0-0-dev pkg-config
make
./build/usb_list_devices
```

## Output checklist

- `lsusb` output before and after plugging in the device.
- `lsusb -v` fields: `idVendor`, `idProduct`, `bDeviceClass`, `bNumConfigurations`, endpoint direction and type.
- `dmesg` lines for enumeration and driver binding.
- Device node path and the kernel driver that created it.

## Interview answer target

Be able to explain why a USB device is visible in `lsusb` but missing from the expected `/dev` node.
