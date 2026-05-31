# 基于CAN总线的多节点农业监测系统

基于 STM32F103C8T6 + FreeRTOS + CAN 总线的分布式大棚环境监测系统。

## 系统架构

```
┌───────────────────────────┐      CAN Bus (500kbps)      ┌───────────────────────────┐
│     传感器采集节点          │ ◄═══════════════════════► │        网关节点            │
│     (Sensor Node)         │       双绞线 ≤30m           │     (Gateway Node)        │
│                           │                            │                           │
│  · STM32F103C8            │                            │  · STM32F103C8            │
│  · TJA1050 CAN 收发器     │                            │  · TJA1050 CAN 收发器     │
│  · DHT11 温湿度传感器     │                            │  · ESP8266 WiFi 模块      │
│  · 土壤湿度传感器 (ADC)    │                            │  · OLED SSD1306 显示      │
│  · BH1750 光照传感器(I2C) │                            │  · LED ×2 状态指示        │
│  · LED ×2 状态指示        │                            │                           │
└───────────────────────────┘                            └───────────┬───────────────┘
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
├── CLAUDE.md
├── LICENSE
├── .gitignore
├── docs/
│   ├── design/architecture.md      # 系统架构设计
│   ├── hardware-wiring.md          # 硬件接线图 (完整GPIO标注)
│   └── diagrams/                   # Mermaid架构图 + PNG渲染
├── hardware/                       # 硬件设计文件 (预留)
│   ├── schematic/
│   ├── pcb/
│   └── bom/
├── common/                         # 两个设备共享的代码
│   ├── cmsis/                      # ARM Cortex-M3 CMSIS
│   ├── std_periph_lib/             # STM32F10x 标准外设库
│   ├── freertos/                   # FreeRTOS v202212.01
│   ├── drivers/                    # 外设驱动
│   │   ├── can/                    # CAN总线 (PB8/PB9)
│   │   ├── usart/                  # USART1 (PA9/PA10)
│   │   ├── dht11/                  # DHT11 温湿度 (PB5)
│   │   ├── soil_moisture/          # 土壤湿度 ADC (PA0)
│   │   ├── bh1750/                 # 光照传感器 I2C (PB6/PB7)
│   │   ├── oled/                   # OLED SSD1306 (PB6/PB7)
│   │   ├── esp8266/                # ESP8266 WiFi
│   │   ├── led/                    # LED 控制 (PB3/PB4)
│   │   ├── key/                    # 按键扫描 (PB12/PB13)
│   │   └── delay/                  # 延时函数
│   └── utils/
│       ├── ring_buffer/            # 环形缓冲区
│       └── protocol/               # CAN协议 + JSON 工具
├── firmware_sensor/                # 传感器采集节点 Keil 工程
│   ├── project.uvprojx
│   ├── src/
│   │   ├── main.c
│   │   ├── stm32f10x_it.c
│   │   └── tasks/
│   │       ├── task_sensor_acq.c   # 传感器采集 (4路)
│   │       └── task_can_report.c   # CAN上报 + 指令处理
│   └── inc/
│       └── app_config.h
├── firmware_gateway/               # 网关节点 Keil 工程
│   ├── project.uvprojx
│   ├── src/
│   │   ├── main.c
│   │   ├── stm32f10x_it.c
│   │   └── tasks/
│   │       ├── task_can_recv.c     # CAN 接收
│   │       ├── task_esp_upload.c   # ESP8266 TCP 上传
│   │       ├── task_oled_disp.c    # OLED 显示
│   │       └── task_can_poll.c     # CAN 主动轮询
│   └── inc/
│       └── app_config.h
└── tools/                          # 辅助工具
    ├── can_monitor/
    ├── render_mermaid_pw.py        # Mermaid → PNG 渲染
    └── render_mermaid.py
```

## 快速开始

### 硬件要求

| 组件 | 传感器节点 | 网关节点 | 备注 |
|------|:---------:|:--------:|------|
| STM32F103C8T6 最小系统板 | ✓ | ✓ | Blue Pill |
| TJA1050 CAN 收发器模块 | ✓ | ✓ | 或 SN65HVD230 |
| DHT11 温湿度传感器 | ✓ | - | PB5, 4.7kΩ 上拉 |
| 土壤湿度传感器 (电阻式) | ✓ | - | PA0 ADC, LM393 模块 |
| BH1750FVI 光照传感器 | ✓ | - | PB6/PB7 I2C, ADDR=GND |
| ESP8266 WiFi 模块 | - | ✓ | ESP-01S, ≥300mA |
| OLED SSD1306 0.96" | - | ✓ | PB6/PB7, 地址 0x78 |
| LED ×2 | ✓ | ✓ | PB3/PB4, 串 220Ω |

> 完整接线图见 [docs/hardware-wiring.md](docs/hardware-wiring.md) 或 [docs/diagrams/index.html](docs/diagrams/index.html)

### 编译与烧录

1. 安装 Keil MDK-ARM v5 + STM32F1xx DFP 包
2. 打开 `firmware_sensor/project.uvprojx` → 编译 → 烧录到传感器节点
3. 打开 `firmware_gateway/project.uvprojx` → 编译 → 烧录到网关节点

### 手机连接

1. 手机连接 WiFi 热点: `SmartFarm` / 密码: `12345678`
2. 打开 TCP 工具连接 `192.168.4.1:8080`
3. 自动接收 JSON 数据或发送查询命令

## 通信协议

### CAN 协议 (STM32 ↔ STM32)

29-bit 扩展帧, 500kbps, 8字节数据载荷:

```
[0-1]=温度×10  [2-3]=湿度×10  [4]=土壤湿度%  [5-6]=光照lux  [7]=保留
```

ID 位域: `[优先级:3][消息类型:10][源节点:8][目标节点:8]`

### TCP JSON 协议 (网关 ↔ 手机)

**手机查询:**
```json
{"cmd": "query"}
```

**网关上发:**
```json
{"type":"data","temp":25.5,"humi":60.0,"soil":45,"light":3200,"status":"online","ts":500}
```

## FreeRTOS 任务架构

| 传感器节点 | 优先级 | 周期 | 职责 |
|-----------|:------:|------|------|
| Task_Sensor_Acq | 4 | 1s | 采集 DHT11 + 土壤 + 光照 |
| Task_CAN_Report | 3 | 事件 | CAN 数据上报 + 心跳 |
| Task_CAN_CmdHandler | 2 | 事件 | 处理网关查询指令 |

| 网关节点 | 优先级 | 周期 | 职责 |
|---------|:------:|------|------|
| Task_CAN_Recv | 4 | 事件 | CAN 帧接收 + 解析 |
| Task_ESP_Upload | 3 | 事件 | JSON 序列化 + TCP 上发 |
| Task_OLED_Display | 2 | 100ms | 4行温/湿/土/光显示 |
| Task_CAN_Poll | 2 | 3s | 离线主动查询 |

## 开源协议

MIT License
