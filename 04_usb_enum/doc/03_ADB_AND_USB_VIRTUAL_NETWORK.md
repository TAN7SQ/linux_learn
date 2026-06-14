# ADB、USB 虚拟网卡和 Linux Gadget 调试链路

## 1. 先把概念分开

这几个词经常被混在一起，但它们不是同一层东西。

```text
ADB
  Android Debug Bridge，是调试协议和工具链。
  重点是 adb client / adb server / adbd daemon。
  传输可以走 USB，也可以走 TCP/IP。

USB 虚拟网卡
  是 USB 设备向主机暴露一个“网卡接口”。
  主机侧会出现一个网络接口，设备侧也会出现一个网络接口。
  两端配置 IP 后，就能 ping、ssh、scp、socket、tcpdump。

虚拟一个 USB 网卡
  常见意思是在 Linux 设备端用 USB Gadget 框架创建 ECM/NCM/RNDIS 网卡功能。
  这不是自己从零写 USB 网卡驱动，而是配置内核已有 gadget function。
```

简单说：

```text
ADB 是调试通道。
USB 虚拟网卡是网络通道。
Linux Gadget 是设备端把 USB 口“变成某种 USB 设备”的内核机制。
```

## 2. ADB 是什么

ADB 的官方结构是三段：

```text
adb client
  你在 PC 上输入的 adb 命令。

adb server
  PC 上后台进程，默认监听 localhost:5037。
  负责管理多个设备和多个 adb client。

adbd
  Android 设备上的 daemon。
  真正执行 shell、push、pull、logcat、install 等动作。
```

典型命令：

```bash
adb devices -l
adb shell
adb push a.txt /data/local/tmp/
adb pull /data/local/tmp/log.txt .
adb logcat
adb forward tcp:9000 tcp:9000
adb tcpip 5555
adb connect <device-ip>:5555
```

ADB 通过 USB 工作时，PC 上的 `adb server` 会通过操作系统 USB 后端或 `libusb` 后端找到设备的 ADB 接口，然后和设备上的 `adbd` 通信。

这条链路大致是：

```text
PC shell
  -> adb client
  -> adb server, localhost:5037
  -> USB backend, native or libusb
  -> USB host controller
  -> USB cable
  -> Android USB device function
  -> adbd
  -> Android userspace / shell / file system / logcat
```

注意：ADB 不是网卡。`adb shell` 能进设备，不代表 PC 和设备之间已经有一个 IP 网络。

## 3. USB 虚拟网卡是什么

USB 虚拟网卡的目标是让一根 USB 线变成一条点对点网络链路。

设备端和主机端都会出现网络接口：

```text
Linux device side
  usb0 / usb1 / enx...
  IP: 192.168.7.2

Host PC side
  usb0 / enx... / RNDIS adapter / CDC NCM adapter
  IP: 192.168.7.1
```

配置好 IP 后：

```bash
ping 192.168.7.2
ssh root@192.168.7.2
scp app root@192.168.7.2:/tmp/
iperf3 -c 192.168.7.2
tcpdump -i usb0
```

它的底层不是 RJ45 以太网 PHY，而是把以太网帧或网络数据封装进 USB 传输。

常见协议或 function：

```text
CDC ECM
  标准 USB CDC Ethernet Control Model。
  Linux/macOS 支持较好。

CDC NCM
  Network Control Model。
  更适合高速 USB，能聚合多个网络包，吞吐通常更好。

RNDIS
  Microsoft Remote NDIS。
  Windows 兼容性常见，但协议更偏微软生态。

EEM
  Ethernet Emulation Model。
  也属于 Ethernet-over-USB 方案，但实际项目里不如 ECM/NCM/RNDIS 常见。
```

## 4. “USB 虚拟网卡”和“虚拟一个 USB 网卡”的区别

这里要分三种情况。

### 4.1 外接 USB 网卡

这是最直观的情况：

```text
USB Ethernet dongle
  -> 插到 Linux 主机 USB Host 口
  -> 主机识别 Realtek / ASIX / Microchip 等芯片
  -> Linux 加载对应 USB 网卡驱动
  -> 主机出现 eth1 / enx...
```

这是一个真实 USB 外设，不是你虚拟出来的。

### 4.2 Linux 设备虚拟成 USB 网卡

这是嵌入式调试里最重要的情况。

```text
开发板 USB OTG / Device 口
  -> Linux USB Gadget
  -> 配置 ECM/NCM/RNDIS function
  -> PC 认为插入了一张 USB 网卡
  -> 开发板和 PC 通过 USB 建立 IP 网络
```

这才是通常说的“虚拟一个 USB 网卡”。

关键前提：

```text
开发板 USB 控制器必须支持 device/peripheral/OTG 模式。
内核必须启用 USB Gadget、libcomposite、configfs 和对应 function。
USB 线必须接到支持 device 模式的口。
```

检查命令：

```bash
ls /sys/class/udc
```

如果这里是空的，说明当前系统没有可用 UDC，不能直接创建 USB Gadget 网卡。原因可能是硬件口不支持 device mode、设备树 `dr_mode` 不对、内核配置不全，或者线插错口。

### 4.3 Linux 里创建普通虚拟网卡

例如：

```bash
sudo ip link add dummy0 type dummy
sudo ip link add veth0 type veth peer name veth1
sudo ip tuntap add dev tap0 mode tap
```

这类是 Linux 网络栈里的虚拟接口，不经过 USB 总线。它能用于网络协议实验，但不能验证 USB 枚举、USB 描述符、Gadget function、主机驱动匹配。

所以：

```text
dummy/veth/tap
  虚拟网络接口，不涉及 USB。

USB Gadget ECM/NCM/RNDIS
  通过 USB 总线向主机暴露一个网卡接口。
```

## 5. 为什么 Linux 上也可以用 USB 虚拟网卡调试

因为 Linux 同时支持两边：

```text
Linux 作为 USB Device
  使用 USB Gadget 子系统。
  通过 configfs 创建 ECM/NCM/RNDIS 等 function。

Linux 作为 USB Host
  使用 cdc_ether / cdc_ncm / rndis_host / usbnet 等驱动。
  插入设备后生成网络接口。
```

开发板常见调试模式：

```text
PC Linux
  USB Host
  cdc_ether / cdc_ncm / rndis_host
  host usb network interface: 192.168.7.1

Orange Pi / Android / embedded Linux board
  USB Device / Gadget
  ECM/NCM/RNDIS function
  device usb network interface: 192.168.7.2
```

然后就能用普通网络工具调试：

```bash
ping
ssh
scp
rsync
nfs
iperf3
tcpdump
gdbserver
```

这对没有以太网口、Wi-Fi 不稳定、板卡还没完成网络配置的阶段非常有用。

## 6. 从底层到应用层的数据流

### 6.1 USB 虚拟网卡数据流

设备端应用发送 TCP 包：

```text
userspace application
  -> socket()
  -> Linux TCP/IP stack
  -> net_device: usb0
  -> gadget network function: ECM/NCM/RNDIS
  -> USB Device Controller, UDC
  -> USB cable
  -> PC USB Host Controller
  -> host driver: cdc_ether / cdc_ncm / rndis_host
  -> host net_device
  -> PC TCP/IP stack
  -> PC application
```

反过来，PC 发给设备也是同一条路径反向走。

### 6.2 ADB over USB 数据流

```text
PC terminal
  -> adb client
  -> adb server, localhost:5037
  -> USB backend
  -> USB bulk endpoints
  -> adbd on Android device
  -> shell / file transfer / logcat / package manager
```

ADB over USB 不经过 Linux IP 网络栈，所以你不能直接用 `ping` 验证 ADB USB 通道。

### 6.3 ADB over TCP 数据流

```text
PC terminal
  -> adb client
  -> adb server
  -> TCP/IP network
  -> adbd listening on tcp:5555
```

这时 ADB 才走网络。网络可以是 Wi-Fi、以太网，也可以是 USB 虚拟网卡。

例如：

```bash
adb tcpip 5555
adb connect 192.168.7.2:5555
```

其中 `192.168.7.2` 可以是 USB 虚拟网卡分配给设备端的 IP。

## 7. Linux Gadget 创建 USB 虚拟网卡的典型过程

以下命令是理解链路用的通用模板。实际板卡要先确认 `/sys/class/udc` 非空。

### 7.1 检查 UDC

```bash
ls /sys/class/udc
```

假设输出：

```text
fcc00000.dwc3
```

### 7.2 挂载 configfs

```bash
sudo modprobe libcomposite
sudo mount -t configfs none /sys/kernel/config
```

如果已经挂载，会提示 busy，可以用：

```bash
mount | grep configfs
```

### 7.3 创建 gadget

```bash
cd /sys/kernel/config/usb_gadget
sudo mkdir g1
cd g1

echo 0x1d6b | sudo tee idVendor
echo 0x0104 | sudo tee idProduct

sudo mkdir -p strings/0x409
echo "20260614" | sudo tee strings/0x409/serialnumber
echo "TAN7SQ" | sudo tee strings/0x409/manufacturer
echo "RK3566 USB Gadget Ethernet" | sudo tee strings/0x409/product
```

### 7.4 创建配置

```bash
sudo mkdir -p configs/c.1/strings/0x409
echo "ECM network config" | sudo tee configs/c.1/strings/0x409/configuration
echo 250 | sudo tee configs/c.1/MaxPower
```

### 7.5 创建 ECM 网卡 function

Linux 主机优先尝试 ECM 或 NCM。这里先用 ECM：

```bash
sudo mkdir functions/ecm.usb0
sudo ln -s functions/ecm.usb0 configs/c.1/
```

如果目标主要是 Windows，通常会考虑 RNDIS：

```bash
sudo mkdir functions/rndis.usb0
sudo ln -s functions/rndis.usb0 configs/c.1/
```

如果目标是较新的 Linux 高速传输，可以考虑 NCM：

```bash
sudo mkdir functions/ncm.usb0
sudo ln -s functions/ncm.usb0 configs/c.1/
```

不要一开始把 ECM、NCM、RNDIS 全堆上去。先让一个 function 跑通，再做 composite。

### 7.6 绑定 UDC

```bash
UDC=$(ls /sys/class/udc | head -n 1)
echo "$UDC" | sudo tee UDC
```

绑定后，PC 端应该看到新 USB 设备，并生成网络接口。

### 7.7 配置设备端 IP

在开发板：

```bash
sudo ip addr add 192.168.7.2/24 dev usb0
sudo ip link set usb0 up
ip addr show usb0
```

### 7.8 配置主机端 IP

在 PC Linux：

```bash
ip link
sudo ip addr add 192.168.7.1/24 dev <host-usb-net-if>
sudo ip link set <host-usb-net-if> up
ping 192.168.7.2
```

如果主机用了 NetworkManager，接口名可能是 `enx...`，也可能被自动配置。

### 7.9 验证

开发板：

```bash
ip addr show usb0
sudo tcpdump -i usb0 -nn
```

主机：

```bash
lsusb
lsusb -t
dmesg | tail -n 50
ip addr
ping 192.168.7.2
ssh tans@192.168.7.2
```

## 8. Android 上 ADB 和 USB 网卡的关系

Android 设备常见 USB 功能是 composite device。也就是说，一根 USB 线里可以同时暴露多个接口：

```text
ADB
MTP/PTP
RNDIS / USB tethering
Accessory
Audio
```

常见场景：

```text
只开 USB debugging
  PC 能 adb shell。
  不一定有 USB 网卡。

开启 USB tethering
  PC 出现 USB 网卡。
  手机可能做 NAT，让 PC 通过手机上网。

同时开 USB debugging 和 tethering
  PC 可能同时看到 ADB 接口和 USB 网卡接口。
```

所以：

```text
adb devices 能看到设备
  只能说明 ADB 通道通了。

ip addr 看到 USB 网卡
  才说明 USB 网络通道存在。
```

## 9. 调试时怎么判断问题在哪一层

### 9.1 USB 层

```bash
lsusb
lsusb -v -d <vid:pid>
dmesg -w
lsusb -t
```

判断：

- PC 完全看不到设备：线缆、供电、OTG 口、UDC、设备树、USB role。
- 能看到设备但没有网卡：function 配置、主机驱动、协议选择。
- 反复断连：供电、线材、速率协商、UDC 错误。

### 9.2 网卡层

```bash
ip link
ip addr
ethtool -i <ifname>
```

判断：

- 有网卡但没 IP：手动配置 IP 或 DHCP。
- 有 IP 但 ping 不通：掩码、路由、防火墙、对端没 up。
- 速度异常：ECM/NCM/RNDIS 选择、USB 速率、线材。

### 9.3 网络协议层

```bash
tcpdump -i <ifname> -nn
ping <peer-ip>
iperf3 -s
iperf3 -c <peer-ip>
```

判断：

- ARP 有没有发出和回应。
- ICMP echo 有没有到对端。
- TCP 三次握手有没有完成。
- UDP 有没有单向到达。

### 9.4 ADB 层

```bash
adb kill-server
adb start-server
adb devices -l
ADB_LIBUSB=1 adb devices -l
adb -s <serial> shell
```

判断：

- `unauthorized`：设备端未授权 RSA 指纹。
- `offline`：adbd 或 USB 通道状态异常。
- 设备不出现：USB 接口、udev 权限、ADB 后端、线缆。
- TCP 模式不通：先检查 IP 网络，再检查 `adbd` 是否监听。

## 10. ADB、USB 虚拟网卡、普通虚拟网卡对比

| 项目 | ADB over USB | USB 虚拟网卡 ECM/NCM/RNDIS | Linux dummy/veth/tap |
|---|---|---|---|
| 本质 | 调试协议 | USB 上的网络接口 | Linux 内部虚拟网络接口 |
| 是否需要 IP | USB 模式不需要 | 需要 | 需要 |
| 主机看到什么 | ADB 设备接口 | 网卡接口 | 本机虚拟接口 |
| 设备端核心 | `adbd` | USB Gadget + net_device | Linux network stack |
| 能否 `ping` | 不能直接 ping ADB USB | 可以 | 可以，但不经过 USB |
| 常用功能 | shell、push、pull、logcat、install | ssh、scp、nfs、iperf3、tcpdump | 网络协议实验 |
| 调试重点 | adb server、adbd、授权、USB 后端 | USB 枚举、function、IP、host driver | 路由、namespace、桥接 |
| 嵌入式价值 | Android 调试很强 | 通用 Linux 板卡调试很强 | 适合主机侧网络实验 |

## 11. 对 Orange Pi / RK3566 学习的建议

当前阶段建议按这个顺序学：

```text
1. USB Host 观察
   插 UVC、USB 串口、U 盘，练 lsusb、dmesg、设备节点。

2. USB Gadget 条件确认
   在板卡上执行 ls /sys/class/udc。
   如果非空，再尝试 ECM/NCM gadget。

3. USB 虚拟网卡
   先跑 ECM 或 NCM，配置 192.168.7.1/24 和 192.168.7.2/24。
   用 ping、ssh、iperf3、tcpdump 验证。

4. ADB 只作为理解对象
   如果没有 Android 系统或 adbd，不要把 ADB 作为 Orange Pi Linux 主线。
   重点理解它和 USB 网卡不是一层东西。
```

## 12. 面试回答模板

如果被问“ADB 和 USB 虚拟网卡有什么区别”，可以这样答：

> ADB 是 Android 的调试协议和工具链，由 PC 端 adb client、adb server 和设备端 adbd 组成。它可以通过 USB bulk endpoint 或 TCP/IP 传输，但本身不是网卡。USB 虚拟网卡是 Linux USB Gadget 或 Android tethering 暴露出来的网络接口，主机和设备两端都会出现 net_device，配置 IP 后可以 ping、ssh、iperf3、tcpdump。两者都能通过一根 USB 线调试设备，但 ADB 走调试协议，USB 网卡走 Linux 网络栈。

如果被问“Linux 为什么能用 USB 虚拟网卡调试”，可以这样答：

> 因为 Linux 设备端有 USB Gadget 框架，可以用 configfs 配置 ECM/NCM/RNDIS function；Linux 主机端也有 cdc_ether、cdc_ncm、rndis_host、usbnet 这类驱动。USB 枚举成功后，两端都会出现网络接口，后面就是普通 IP 网络调试。

## 13. 参考资料

- Android Developers: Android Debug Bridge
  - <https://developer.android.com/tools/adb>
- ADB man page in AOSP
  - <https://android.googlesource.com/platform/packages/modules/adb/+/refs/heads/master/docs/user/adb.1.md>
- Linux Kernel: USB gadget configured through configfs
  - <https://docs.kernel.org/usb/gadget_configfs.html>
- Linux Kernel: Multifunction Composite Gadget
  - <https://www.kernel.org/doc/html/v6.0/usb/gadget_multi.html>
- Linux USB usbnet driver framework
  - <https://www.linux-usb.org/usbnet/>
