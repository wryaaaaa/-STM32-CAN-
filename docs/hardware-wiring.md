# 硬件接线图

## STM32F103C8T6 默认复用引脚使用一览

| 引脚 | 默认复用功能 | 本项目用途 | 所属设备 |
|------|:----------:|-----------|:--------:|
| PA9 | USART1_TX | ESP8266 RXD | 网关 |
| PA10 | USART1_RX | ESP8266 TXD | 网关 |
| PB3 | GPIO | LED1 | 两者 |
| PB4 | GPIO | LED2 | 两者 |
| PB5 | GPIO | DHT11 DATA | 传感器 |
| PB6 | I2C1_SCL | OLED SCL | 网关 |
| PB7 | I2C1_SDA | OLED SDA | 网关 |
| PB8 | **CAN1_RX** | TJA1050 RXD | 两者 |
| PB9 | **CAN1_TX** | TJA1050 TXD | 两者 |
| PB12 | GPIO | KEY0 (预留) | 两者 |
| PB13 | GPIO | KEY1 (预留) | 两者 |

> CAN 使用硬件默认引脚 PB8(CAN_RX) + PB9(CAN_TX)，**无需重映射**。
> OLED 使用 PB6(I2C1_SCL) + PB7(I2C1_SDA) 软件模拟 I2C。

---

## 一、传感器采集节点

```
                    STM32F103C8T6
                  ┌─────────────────┐
                  │                 │
    LED1 ◄── PB3  │                 │  PB8 ───► TJA1050 RXD ──► CAN_H / CAN_L
    LED2 ◄── PB4  │                 │  PB9 ◄─── TJA1050 TXD
                  │                 │
   DHT11 ◄── PB5  │                 │
    VCC ─── 3.3V  │                 │
    GND ─── GND   └─────────────────┘
  (DATA 接 4.7kΩ 上拉到 3.3V)

            TJA1050
         ┌────────────┐
  PB8 ──┤1 TXD    CANH├──┬── 双绞线 ──► 网关
  GND ──┤2 GND    CANL├──┘
  5V ───┤3 VCC      RS├── GND (斜率控制)
  PB9 ──┤4 RXD    Vref├── 悬空
         └────────────┘
          总线两端各接 120Ω
```

### 传感器节点接线速查表

| STM32 引脚 | 外设 | 线色建议 | 备注 |
|:---------:|------|:------:|------|
| PB3 | LED1 阳极 | — | 串 220Ω → GND |
| PB4 | LED2 阳极 | — | 串 220Ω → GND |
| PB5 | DHT11 DATA | 黄 | 外接 4.7kΩ 上拉至 3.3V |
| PB8 | TJA1050 RXD (pin1) | 绿 | CAN 接收 |
| PB9 | TJA1050 TXD (pin4) | 蓝 | CAN 发送 |
| 3.3V / 5V | DHT11 VCC, TJA1050 VCC | 红 | |
| GND | 所有模块 GND | 黑 | **两个节点 GND 需共地** |

---

## 二、网关节点

```
                    STM32F103C8T6
                  ┌─────────────────┐
                  │                 │
    LED1 ◄── PB3  │                 │  PA9 ───► ESP8266 RXD
    LED2 ◄── PB4  │                 │  PA10 ◄── ESP8266 TXD
                  │                 │
  OLED SCL ◄─ PB6 │                 │  PB8 ───► TJA1050 RXD ──► CAN_H / CAN_L
  OLED SDA ◄─ PB7 │                 │  PB9 ◄─── TJA1050 TXD
                  └─────────────────┘

         ESP8266-01S                  OLED SSD1306
      ┌──────────────┐            ┌──────────────┐
 VCC ─┤VCC        TXD├── PA10   ─┤VCC        SCL├── PB6
 GND ─┤GND        RXD├── PA9    ─┤GND        SDA├── PB7
3.3V──┤CH_PD     RST├── 3.3V     └──────────────┘
3.3V──┤RST (10kΩ上拉)  │           (I2C 地址 0x78)
      └──────────────┘
      ESP8266 峰值电流 ≥300mA，需独立 3.3V 供电
```

### 网关节点接线速查表

| STM32 引脚 | 默认复用 | 外设 | 备注 |
|:---------:|:------:|------|------|
| PB3 | GPIO | LED1 | 串 220Ω → GND |
| PB4 | GPIO | LED2 | 串 220Ω → GND |
| PA9 | USART1_TX | ESP8266 RXD | 115200bps |
| PA10 | USART1_RX | ESP8266 TXD | |
| PB6 | I2C1_SCL | OLED SCL | 软件 I2C |
| PB7 | I2C1_SDA | OLED SDA | 软件 I2C |
| PB8 | **CAN1_RX** | TJA1050 RXD | CAN 默认引脚 |
| PB9 | **CAN1_TX** | TJA1050 TXD | CAN 默认引脚 |

---

## 三、CAN 总线连接

```
  传感器节点                             网关节点
  ┌─────────┐                          ┌─────────┐
  │ TJA1050 │                          │ TJA1050 │
  │  CANH ├──┬──── 双绞线 ────────┬──┤ CANH    │
  │  CANL ├──┘                   └──┤ CANL    │
  └─────────┘                        └─────────┘
       │                                  │
       ├── 120Ω ── GND                    ├── 120Ω ── GND
       │                                  │

  参数: 500kbps | 扩展帧 | 终端电阻 120Ω ×2 | ≤30m | 共地
```

---

## 四、完整物料清单 (BOM)

| 序号 | 组件 | 传感器节点 | 网关节点 | 型号/规格 |
|:----:|------|:---------:|:--------:|------|
| 1 | STM32 最小系统板 | 1 | 1 | STM32F103C8T6 (Blue Pill) |
| 2 | CAN 收发器模块 | 1 | 1 | TJA1050 或 SN65HVD230 |
| 3 | DHT11 温湿度模块 | 1 | 0 | 3.3V-5V |
| 4 | ESP8266 WiFi 模块 | 0 | 1 | ESP-01S |
| 5 | OLED 显示屏 | 0 | 1 | 0.96" SSD1306 I2C (4pin) |
| 6 | LED (颜色任意) | 2 | 2 | 3mm 或 5mm |
| 7 | 220Ω 电阻 | 2 | 2 | LED 限流 |
| 8 | 120Ω 电阻 | 1 | 1 | CAN 终端 (1/4W) |
| 9 | 4.7kΩ 电阻 | 1 | 0 | DHT11 DATA 上拉 |
| 10 | 10kΩ 电阻 | 0 | 2 | ESP8266 RST/CH_PD |
| 11 | 双绞线 (≤30m) | 1 | 1 | CAN 总线 |
| 12 | USB-TTL 模块 | 1 | 1 | 烧录用 |
| 13 | 杜邦线 + 面包板 | 若干 | 若干 | |

---

## 五、上电检查

| 步骤 | 操作 | 预期现象 |
|:----:|------|------|
| 1 | 传感器节点上电 | LED1 每秒闪一次 (DHT11 采集正常) |
| 2 | 网关节点上电 | OLED 显示 "CAN Gateway" → "Init..." |
| 3 | 连接 CAN 总线 (两节点) | 网关 OLED 显示温度和湿度 |
| 4 | 拔掉 DHT11 | 传感器 LED2 闪烁 (告警) |
| 5 | 手机连 WiFi `SmartFarm/12345678` | TCP `192.168.4.1:8080` 收到 JSON |
