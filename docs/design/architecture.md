# 系统架构设计

## 概述

多节点 CAN 总线农业监测系统，由一个传感器采集节点和一个网关节点通过 CAN 总线组成分布式网络。

## 硬件架构

### 传感器采集节点
- MCU: STM32F103C8T6 (Cortex-M3, 72MHz, 64KB Flash, 20KB RAM)
- CAN收发器: TJA1050 (PB8=CAN_RX, PB9=CAN_TX)
- 传感器: DHT11 (PB5)
- 指示: LED1 (PB3), LED2 (PB4)

### 网关节点
- MCU: STM32F103C8T6
- CAN收发器: TJA1050 (PB8=CAN_RX, PB9=CAN_TX)
- WiFi: ESP8266 (USART1: PA9=TX, PA10=RX, 115200bps)
- 显示: OLED SSD1306 I2C (PB8=SCL, PB9=SDA, 软件I2C)
- 指示: LED1 (PB3), LED2 (PB4)

## 软件架构 (FreeRTOS)

### 传感器节点任务

```
┌──────────────────┐    xQueueSensorData    ┌──────────────────┐
│ Task_Sensor_Acq  │ ──────────────────────>│  Task_CAN_Report │
│ (Prio:3, 1s周期) │    (SensorData_t)       │ (Prio:4, 事件)   │
│                  │                         │                  │
│ · DHT11读取      │                         │ · CAN帧打包发送   │
│ · 故障检测       │                         │ · 定时心跳       │
└──────────────────┘                         └──────────────────┘

┌──────────────────┐    xQueueCANCmd         ┌──────────────────┐
│   CAN1_RX0_ISR   │ ──────────────────────>│Task_CAN_CmdHandler│
│                  │    (CAN_Frame_t)        │ (Prio:2, 事件)   │
│                  │                         │                  │
│                  │                         │ · 查询指令解析   │
│                  │                         │ · 即时采集+应答  │
└──────────────────┘                         └──────────────────┘
```

### 网关节点任务

```
CAN1_RX0_ISR → xQueueCANRx → Task_CAN_Recv (Prio:4)
                                   │
                    ┌──────────────┼──────────────┐
                    ▼              ▼              ▼
            xQueueUpload    g_GatewayData    (更新在线状态)
                    │              │
                    ▼              ▼
          Task_ESP_Upload    Task_OLED_Display
          (Prio:3, 事件)     (Prio:2, 100ms)

USART1_IRQ → xQueueUARTCmd → Task_ESP_Upload (处理TCP命令)

Task_CAN_Poll (Prio:2, 3s周期) → 传感器离线时主动查询
```

## 数据流

### 定时上报 (正常模式)
```
DHT11 → Task_Sensor_Acq → xQueueSensorData → Task_CAN_Report
→ CAN TX → CAN RX → Task_CAN_Recv → xQueueUpload
→ Task_ESP_Upload → JSON → ESP8266 TCP → 手机
```

### 查询模式 (手机触发)
```
手机 → TCP → USART1_ISR → xQueueUARTCmd → Task_ESP_Upload
→ CAN查询帧 → 传感器 Task_CAN_CmdHandler → DHT11即时读取
→ CAN应答帧 → 网关 Task_CAN_Recv → JSON → 手机
```
