# 05_network_socket 学习入口

## 学习目标

这一阶段目标是掌握嵌入式 Linux 设备的网络应用和网络调试，而不是手写网卡驱动。

你要能完成：

```text
PC
  -> TCP/UDP 命令
  -> Orange Pi 用户态进程
  -> 状态响应
  -> tcpdump / iperf3 证据
```

## 你要学习什么

- `ip addr`、`ip route`、`ethtool`。
- TCP server/client。
- UDP sender/receiver。
- `tcpdump` 抓包。
- `iperf3` 测吞吐。
- TCP/UDP 在设备控制、状态上报、图像传输中的取舍。

## 本阶段代码

```text
05_network_socket/tcp_server.c
05_network_socket/tcp_client.c
05_network_socket/udp_receiver.c
05_network_socket/udp_sender.c
05_network_socket/network_debug.md
```

## 构建

```bash
cd /home/tans/workspace/linux_drivers/05_network_socket
make
```

## 最小验证

Orange Pi：

```bash
./build/tcp_server 0.0.0.0 9000
```

PC 或另一个终端：

```bash
./build/tcp_client <board-ip> 9000 "status?"
```

抓包：

```bash
sudo tcpdump -i eth0 -nn port 9000
```
