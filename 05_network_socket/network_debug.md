# Network Debug Record

Fill this file on the board and PC when running the network stage.

## Link and IP

```bash
ip addr
ip route
ethtool eth0
ping <peer-ip>
```

## Throughput

Board:

```bash
iperf3 -s
```

PC:

```bash
iperf3 -c <board-ip>
```

## Packet capture

```bash
sudo tcpdump -i eth0 -nn port 9000
sudo tcpdump -i eth0 -nn udp port 9001
```

## Questions to answer

- Is the failure at link layer, IP configuration, routing, firewall, or application protocol?
- Does TCP show a three-way handshake?
- Does UDP transmit without a reply path?
- What happens after unplugging and reconnecting the cable?
