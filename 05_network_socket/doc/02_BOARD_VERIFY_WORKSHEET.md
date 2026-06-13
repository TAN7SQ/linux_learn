# 网络板卡验证记录表

## 基础网络

```text
板卡 IP：
PC IP：
网口名：
链路速率：
是否能 ping 通：
默认路由：
DNS：
```

命令：

```bash
ip addr
ip route
ethtool eth0
ping <peer-ip>
```

## TCP 验证

```text
server 命令：
client 命令：
server 输出：
client 输出：
tcpdump 关键行：
问题：
结论：
```

## UDP 验证

```text
receiver 命令：
sender 命令：
receiver 输出：
sender 输出：
tcpdump 关键行：
问题：
结论：
```

## 吞吐验证

```bash
iperf3 -s
iperf3 -c <board-ip>
```

记录：

```text
方向：
吞吐：
丢包或重传：
CPU 占用：
结论：
```
