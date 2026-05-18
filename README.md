# Remote UART Bridge

基于 ESP32 的远程串口桥接器，通过 ESP-NOW 将 PC 端 USB 串口透传到远端设备的 UART，实现无线串口调试与固件下载。

## 系统架构

```
┌─────────┐   USB CDC-ACM    ┌──────────────┐   ESP-NOW    ┌──────────────┐   UART   ┌──────────┐
│   PC    │ ◄──────────────► │ Server (S3)  │ ◄──────────► │ Client (C3)  │ ◄──────► │ 目标设备  │
└─────────┘                  └──────────────┘              └──────────────┘          └──────────┘
                                     │
                              Wi-Fi AP (Web 管理界面)
```

系统由两种角色组成：

- **Server**（运行于 ESP32-S3）：通过 USB CDC-ACM 连接 PC，将 USB 串口数据、波特率变更、DTR/RTS 信号通过 ESP-NOW 转发到远端 Client
- **Client**（运行于 ESP32-C3）：通过 UART 连接目标设备，接收 Server 透传的数据并转发到 UART，同时支持自动下载模式（DTR/RTS 控制 GPIO）

## 数据流

### USB → 远端 UART

```
PC USB → usb_bridge(TinyUSB RX回调)
       → message_manager(打包为 lwpkt 帧, 命令: USB_TO_UART_DATA)
       → espnow_manager(发送到所有已启用 Peer)
       → 远端 Client message_manager(解析 lwpkt 帧)
       → uart_bridge_tx(写入 UART)
```

### 远端 UART → USB

```
目标设备 UART → uart_bridge(RX 回调)
             → message_manager(打包为 lwpkt 帧, 命令: UART_TO_USB_DATA)
             → espnow_manager(发送到 Server)
             → Server message_manager(解析 lwpkt 帧)
             → usb_bridge_tx(写入 USB TX 环形缓冲区 → PC)
```

### 串口参数与信号透传

- **波特率变更**：PC 端修改串口参数时，Server 捕获 `line coding` 变更，通过 `USB_LINE_CODING_CHANGED` 命令发送到 Client，Client 调用 `uart_bridge_reset_baud_rate` 动态修改 UART 波特率
- **DTR/RTS 信号**：PC 端切换 DTR/RTS 时，Server 通过 `USB_LINE_STATE_CHANGED` 命令发送到 Client，Client 调用 `auto_download_set_gpio_level` 驱动 GPIO，实现目标设备的一键下载

## ESP-NOW 通信协议

使用 lwpkt 轻量级封包协议进行帧封装，定义了以下命令：

| 命令值 | 名称 | 方向 | 说明 |
|--------|------|------|------|
| 0 | SCAN | Server → 广播 | 设备扫描请求 |
| 1 | SCAN_RESPONSE | Client → Server | 扫描响应（含设备标签、RSSI） |
| 2 | ADD_PEER_DEVICE | 双向 | 添加对端为持久 Peer |
| 3 | USB_TO_UART_DATA | Server → Client | USB 串口数据透传 |
| 4 | UART_TO_USB_DATA | Client → Server | UART 数据透传 |
| 5 | USB_LINE_CODING_CHANGED | Server → Client | 波特率/校验/停止位变更 |
| 6 | USB_LINE_STATE_CHANGED | Server → Client | DTR/RTS 信号变更 |

lwpkt 配置：启用 CMD 字段和 CRC-8 校验，禁用地址和标志字段，最大载荷 1024 字节。

## Web 管理界面

设备启动后创建 Wi-Fi AP（SSID 默认为设备 MAC 地址），提供 Web 管理界面（端口 80）。

### 页面

| 路径 | 说明 |
|------|------|
| `/` 或 `/espnow` | ESP-NOW 设备管理页面 |
| `/monitor` | 系统监控页面 |

### API 接口

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/api/system/status` | 系统状态（内存、运行时间、芯片温度、SDK 版本、MAC、标签） |
| POST | `/api/system/label` | 设置设备标签 |
| POST | `/api/ota/upload` | OTA 固件升级 |
| GET | `/api/espnow/peers` | 获取 Peer 列表（MAC、标签、启用状态） |
| POST | `/api/espnow/scan` | 扫描 ESP-NOW 设备（等待 500ms 收集响应） |
| POST | `/api/espnow/peer/add` | 添加 Peer（JSON: `{"mac":"XX:XX:...","label":"..."}`) |
| POST | `/api/espnow/peer/delete` | 删除 Peer |
| POST | `/api/espnow/peer/enable` | 启用 Peer |
| POST | `/api/espnow/peer/disable` | 禁用 Peer |

## 组件说明

| 组件 | 说明 |
|------|------|
| **usb-bridge** | USB CDC-ACM 桥接（TinyUSB），处理数据收发、line coding 和 line state 变更 |
| **uart-bridge** | UART 桥接，管理串口收发和波特率动态切换 |
| **espnow-manager** | ESP-NOW 通信管理，Peer 列表持久化（NVS），设备发现与消息收发 |
| **message-manager** | 消息路由与协议层，定义命令格式，在 USB/UART/ESP-NOW 之间路由消息 |
| **auto-download** | 自动下载模式 GPIO 控制，根据 DTR/RTS 信号驱动目标设备进入下载模式 |
| **wifi-manager** | Wi-Fi AP 初始化，为 Web 管理界面提供网络接入 |
| **web** | HTTP 服务器，提供 Web 管理页面和 REST API |
| **nvs-manager** | NVS 持久化封装，提供 blob 读写接口 |
| **espstate-monitor** | 芯片温度监控 |
| **lwpkt** | 轻量级封包协议库（帧封装/解析，含 CRC-8 校验） |
| **lwrb** | 轻量级无锁环形缓冲区库 |
| **memory-display** | RAM 使用率调试输出 |

## 硬件配置

### Server（ESP32-S3）

- USB CDC-ACM 接口（TinyUSB，VID: Espressif，PID: 0x4002）
- 8MB Flash
- 双 OTA 分区（各 3MB）

### Client（ESP32-C3）

- UART：TX GPIO4, RX GPIO5（默认 115200bps）
- 自动下载：DTR → GPIO0, RTS → GPIO1
- 4MB Flash
- 双 OTA 分区（各 1.875MB）

## 编译与烧录

依赖 ESP-IDF >= 6.0.0。

### 编译 Server（ESP32-S3）

```bash
idf.py set-target esp32s3
idf.py build
idf.py flash
```

### 编译 Client（ESP32-C3）

```bash
idf.py set-target esp32c3
idf.py build
idf.py flash
```

角色选择通过 `menuconfig` 中 `REMOTE UART BRIDGE` 菜单配置，ESP32-S3 默认为 Server，ESP32-C3 默认为 Client。

## 使用流程

1. 烧录 Server 固件到 ESP32-S3，通过 USB 连接 PC
2. 烧录 Client 固件到 ESP32-C3，通过 UART 连接目标设备
3. PC 连接 Server 的 Wi-Fi AP，打开 Web 管理页面
4. 点击扫描发现 Client 设备，添加为 Peer
5. PC 端打开串口终端（如 minicom、PuTTY），连接 Server 的 USB 串口
6. 即可通过无线方式与远端目标设备的串口交互
