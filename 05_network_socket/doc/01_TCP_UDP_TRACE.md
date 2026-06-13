# TCP/UDP 链路理解

## TCP 控制链路

```text
socket(AF_INET, SOCK_STREAM)
  -> bind
  -> listen
  -> accept
  -> read/write
  -> close
```

适合：

- 控制命令。
- 配置下发。
- 必须确认收到的数据。
- 状态查询。

代价：

- 有连接建立过程。
- 断线重连需要额外处理。
- 延迟和阻塞行为要处理好。

## UDP 状态或数据链路

```text
socket(AF_INET, SOCK_DGRAM)
  -> bind 或 sendto
  -> recvfrom
```

适合：

- 高频状态广播。
- 可以丢少量包的数据。
- 局域网内简单发现协议。

代价：

- 不保证到达。
- 不保证顺序。
- 需要自己设计序号、时间戳、重传或丢包容忍。

## 抓包观察点

TCP：

```text
SYN
SYN, ACK
ACK
PSH, ACK
FIN / RST
```

UDP：

```text
源 IP / 目的 IP
源端口 / 目的端口
payload 长度
是否有反向应答
```

## 嵌入式面试表达

> 如果是设备控制命令，我优先用 TCP，因为要确认配置是否到达。如果是高频状态或图像预览里的轻量数据，可以考虑 UDP，但要自己处理序号、时间戳和丢包容忍。
