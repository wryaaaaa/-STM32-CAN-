# 基于CAN总线的多节点农业监测系统

基于 STM32F103C8T6 + FreeRTOS + CAN 总线的分布式大棚环境监测系统。

## 系统架构

```
┌─────────────────────┐        CAN Bus (500kbps)        ┌─────────────────────┐
│   传感器采集节点      │ ◄═══════════════════════════► │     网关节点         │
│   (Sensor Node)     │         双绞线 ≤30m             │   (Gateway Node)    │
│                     │                                 │                     │
│  · STM32F103C8      │                                 │  · STM32F103C8      │
│  · TJA1050 CAN收发器│                                 │  · TJA1050 CAN收发器 │
│  · DHT11 温湿度传感器│                                 │  · ESP8266 WiFi模块  │
│  · LED 状态指示     │                                 │  · OLED SSD1306 显示 │
└─────────────────────┘                                 │  · LED 状态指示     │
                                                        └──────────┬──────────┘
                                                                   │ TCP (WiFi)
                                                                   ▼
                                                            ┌──────────────┐
                                                            │   手机 APP    │
                                                            │ (TCP Client) │
                                                            └──────────────┘
```

## 目录结构

```
├── README.md
├── docs/                       # 设计文档
│   ├── architecture.md         # 系统架构
│   ├── can-protocol.md         # CAN协议定义
│   └── development-guide.md    # 开发环境搭建
├── hardware/                   # 硬件设计
│   ├── schematic/
│   ├── pcb/
│   └── bom/
├── common/                     # 共享代码
│   ├── cmsis/                  # ARM Cortex-M3 支持
│   ├── std_periph_lib/         # STM32F10x 标准外设库
│   ├── freertos/               # FreeRTOS v202212.01
│   ├── drivers/                # 外设驱动
│   │   ├── can/                # CAN总线驱动
│   │   ├── usart/              # 串口驱动
│   │   ├── dht11/              # DHT11传感器
│   │   ├── oled/               # OLED SSD1306
│   │   ├── esp8266/            # ESP8266 WiFi
│   │   ├── led/                # LED控制
│   │   ├── key/                # 按键扫描
│   │   └── delay/              # 延时函数
│   └── utils/                  # 工具库
│       ├── ring_buffer/        # 环形缓冲区
│       └── protocol/           # CAN协议 & JSON工具
├── firmware_sensor/            # 传感器采集节点工程
│   ├── project.uvprojx         # Keil MDK 工程
│   ├── src/
│   │   ├── main.c
│   │   ├── stm32f10x_it.c
│   │   └── tasks/
│   └── inc/
│       └── app_config.h
├── firmware_gateway/           # 网关节点工程
│   ├── project.uvprojx         # Keil MDK 工程
│   ├── src/
│   │   ├── main.c
│   │   ├── stm32f10x_it.c
│   │   └── tasks/
│   └── inc/
│       └── app_config.h
└── tools/                      # 调试工具
    └── can_monitor/
```

## 快速开始

### 硬件要求

| 组件 | 传感器节点 | 网关节点 |
|------|:---------:|:--------:|
| STM32F103C8T6 最小系统板 | ✓ | ✓ |
| TJA1050 CAN 收发器模块 | ✓ | ✓ |
| DHT11 温湿度传感器 | ✓ | - |
| ESP8266 WiFi 模块 | - | ✓ |
| OLED SSD1306 (I2C) | - | ✓ |
| LED ×2 | ✓ | ✓ |

### 编译与烧录

1. 安装 Keil MDK-ARM v5 + STM32F1xx DFP 包
2. 打开 `firmware_sensor/project.uvprojx` → 编译 → 烧录到传感器节点
3. 打开 `firmware_gateway/project.uvprojx` → 编译 → 烧录到网关节点

### CAN 总线连接

```
传感器节点 TJA1050          网关节点 TJA1050
     CAN_H ◄══════════════════► CAN_H
     CAN_L ◄══════════════════► CAN_L
             两端各接 120Ω 终端电阻
```

### 手机连接

1. 手机连接 WiFi 热点: `SmartFarm` / 密码: `12345678`
2. 打开 TCP 工具连接 `192.168.4.1:8080`
3. 自动接收 JSON 数据或发送查询命令

## 通信协议

### CAN 协议 (STM32 ↔ STM32)

29-bit 扩展帧, 500kbps, ID 位域: `[优先级:3][消息类型:10][源节点:8][目标:8]`

### TCP 协议 (网关 ↔ 手机)

**手机发送命令：**
```json
{"cmd": "query"}
```

**网关上传数据：**
```json
{"type":"data","temp":25.5,"humi":60.0,"status":"online","ts":500}
```

详见 [docs/can-protocol.md](docs/can-protocol.md)

## 开源协议

MIT License
