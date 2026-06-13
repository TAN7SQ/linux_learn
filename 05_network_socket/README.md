# 05_network_socket

Ethernet and socket learning stage for Orange Pi CM4.

## Goal

Build confidence in Linux network debugging and user-space TCP/UDP programming.

```text
PC command tool
  -> Ethernet/Wi-Fi network
  -> Orange Pi TCP/UDP process
  -> status response
  -> tcpdump/iperf3 evidence
```

## Commands

```bash
ip addr
ip route
ethtool eth0
ping <peer-ip>
iperf3 -s
iperf3 -c <peer-ip>
sudo tcpdump -i eth0 -nn port 9000
```

## Build

```bash
make
```

## TCP demo

Board:

```bash
./build/tcp_server 0.0.0.0 9000
```

PC or another terminal:

```bash
./build/tcp_client <board-ip> 9000 "status?"
```

## UDP demo

Receiver:

```bash
./build/udp_receiver 0.0.0.0 9001
```

Sender:

```bash
./build/udp_sender <board-ip> 9001 "hello udp"
```

## Output checklist

- IP address, route, and link speed.
- Successful TCP request and response.
- Successful UDP datagram send and receive.
- `tcpdump` capture for TCP handshake or UDP packet.
- `iperf3` throughput record.

## Interview answer target

Be able to explain when to use TCP or UDP for device control, state reporting, and image streaming.
