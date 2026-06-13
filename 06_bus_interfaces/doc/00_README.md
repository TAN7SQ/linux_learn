# 06_bus_interfaces 学习入口

## 学习目标

这一阶段目标是掌握 I2C、SPI、UART 外设从用户态访问到硬件总线的链路。

```text
用户态程序
  -> /dev/i2c-* /dev/spidev* /dev/ttyS*
  -> 内核 bus controller driver
  -> pinmux / device tree
  -> 外部芯片或模块
```

## 你要学习什么

- I2C 地址、寄存器读写、ACK/NACK。
- SPI 模式、片选、时钟、全双工传输。
- UART 波特率、数据位、停止位、校验、raw 模式。
- 设备树如何打开控制器和 pinmux。
- 如何区分接线、电平、设备树、驱动、用户态配置问题。

## 本阶段代码

```text
06_bus_interfaces/i2c_read_reg.c
06_bus_interfaces/spi_transfer.c
06_bus_interfaces/uart_echo.c
```

## 构建

```bash
cd /home/tans/workspace/linux_drivers/06_bus_interfaces
make
```

## 最小验证

```bash
i2cdetect -l
sudo i2cdetect -y <bus>
sudo ./build/i2c_read_reg /dev/i2c-1 0x50 0x00

./build/uart_echo /dev/ttyS1 115200

ls -l /dev/spidev*
sudo ./build/spi_transfer /dev/spidev0.0 1000000 aa55
```
