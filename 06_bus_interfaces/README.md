# 06_bus_interfaces

I2C, SPI, and UART learning stage for Orange Pi CM4.

## Goal

Understand how a Linux user-space program reaches a real peripheral through a
bus device node.

```text
user-space tool
  -> /dev/i2c-* /dev/spidev* /dev/ttyS*
  -> kernel bus controller driver
  -> board pinmux and device tree
  -> external peripheral
```

## Build

```bash
make
```

## I2C

Install tools:

```bash
sudo apt install i2c-tools
```

Discover buses and devices:

```bash
i2cdetect -l
sudo i2cdetect -y <bus>
sudo ./build/i2c_read_reg /dev/i2c-1 0x50 0x00
```

## UART

Loopback test or connect a USB serial adapter:

```bash
stty -F /dev/ttyS1 115200 raw -echo
./build/uart_echo /dev/ttyS1 115200
```

## SPI

Use `spidev` only after the device node exists:

```bash
ls -l /dev/spidev*
sudo ./build/spi_transfer /dev/spidev0.0 1000000 aa55
```

## Output checklist

- Device node path.
- Device tree or overlay that enables the bus.
- Command output from the user-space tool.
- Failure classification: wiring, voltage, pinmux/device tree, kernel driver, or user-space configuration.

## Interview answer target

Be able to draw the path from user-space read/write/ioctl to a bus controller and external chip.
