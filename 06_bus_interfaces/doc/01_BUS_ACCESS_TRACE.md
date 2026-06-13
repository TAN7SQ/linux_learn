# I2C/SPI/UART 访问链路

## I2C

```text
i2c_read_reg
  -> open("/dev/i2c-X")
  -> ioctl(fd, I2C_SLAVE, addr)
  -> write register address
  -> read register value
  -> I2C controller driver
  -> SDA/SCL
  -> external chip
```

关键排查：

- `i2cdetect -l` 没有总线：设备树或控制器未启用。
- `i2cdetect` 没有地址：接线、电源、地址、电平、上拉。
- 地址出现但读失败：寄存器协议、读写时序、驱动占用。

## SPI

```text
spi_transfer
  -> open("/dev/spidevB.C")
  -> ioctl(fd, SPI_IOC_MESSAGE)
  -> SPI controller driver
  -> SCLK / MOSI / MISO / CS
  -> external chip
```

关键排查：

- 没有 `/dev/spidev*`：设备树没有打开 spidev 或控制器。
- 有节点但传输异常：mode、speed、bits_per_word、CS、电平。
- MISO 全 0 或全 FF：接线、片选、外设供电。

## UART

```text
uart_echo
  -> open("/dev/ttyS*")
  -> termios 配置波特率和 raw 模式
  -> write/read
  -> UART controller driver
  -> TX/RX
  -> external module or loopback
```

关键排查：

- 没有节点：设备树或串口复用未启用。
- 乱码：波特率、数据位、停止位、校验不一致。
- 只发不收：TX/RX 接反、地线没共地、模块电平不匹配。
