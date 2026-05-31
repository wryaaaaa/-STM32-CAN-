# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

基于CAN总线+FreeRTOS的多节点农业大棚监测系统。两个STM32F103C8节点通过CAN总线通信：传感器采集节点（DHT11+土壤湿度+BH1750光照）采集环境数据，网关节点接收后通过OLED显示、ESP8266 WiFi上传至手机。

- **MCU:** STM32F103C8 (Cortex-M3, 64KB Flash @ 0x08000000, 20KB RAM @ 0x20000000, 72MHz)
- **RTOS:** FreeRTOS v202212.01 (preemptive, 5 priority levels, 17KB heap, 1ms tick)
- **Peripheral Library:** STM32F10x Standard Peripheral Library v3.5.0
- **Toolchain:** Keil MDK-ARM v5 (uVision), ARMCC v5.06.5, C99 mode
- **Build output:** `Objects/project.hex` (`CreateHexFile=1`)

## Build

```
# Open in Keil uVision and rebuild:
firmware_sensor/project.uvprojx    # Menu: Project → Rebuild all target files
firmware_gateway/project.uvprojx   # Menu: Project → Rebuild all target files

# Or use UV4.exe command line:
UV4.exe -b firmware_sensor/project.uvprojx -t "Target 1" -j0 -o build.log
UV4.exe -b firmware_gateway/project.uvprojx -t "Target 1" -j0 -o build.log

# Clean build artifacts:
.\keilkill.bat
```

Preprocessor define: `USE_STDPERIPH_DRIVER`. Include paths are in each project file at `Cads → VariousControls → IncludePath`.

## High-Level Architecture

### Pin Allocation

| Pin | Function | Sensor Node | Gateway Node |
|-----|----------|:-----------:|:------------:|
| PA0 | ADC1_CH0 | Soil Moisture | — |
| PA9 | USART1_TX | — | ESP8266 RXD |
| PA10 | USART1_RX | — | ESP8266 TXD |
| PB3 | GPIO | LED1 | LED1 |
| PB4 | GPIO | LED2 | LED2 |
| PB5 | GPIO | DHT11 DATA | — |
| PB6 | I2C1_SCL | BH1750 SCL (HW I2C) | OLED SCL (bit-bang) |
| PB7 | I2C1_SDA | BH1750 SDA (HW I2C) | OLED SDA (bit-bang) |
| PB8 | **CAN1_RX** (default) | TJA1050 RXD | TJA1050 RXD |
| PB9 | **CAN1_TX** (default) | TJA1050 TXD | TJA1050 TXD |
| PB12 | GPIO | KEY0 (reserved) | KEY0 (reserved) |
| PB13 | GPIO | KEY1 (reserved) | KEY1 (reserved) |

### Task Model (FreeRTOS)

**Sensor Node — 3 tasks:**

| Task | Priority | Stack | Cycle | Role |
|------|:--------:|-------|-------|------|
| `Task_Sensor_Acq` | 4 | 256 | 1s | Read DHT11 + Soil ADC + BH1750 → push to queue |
| `Task_CAN_Report` | 3 | 256 | event/1s | xQueueReceive → build CAN frame → send + heartbeat every 5s |
| `Task_CAN_CmdHandler` | 2 | 256 | event | Handle query frames from gateway → immediate read + reply |

**Gateway Node — 4 tasks:**

| Task | Priority | Stack | Cycle | Role |
|------|:--------:|-------|-------|------|
| `Task_CAN_Recv` | 4 | 256 | event | CAN ISR → xQueueCANRx → parse → update GatewayData + push upload queue |
| `Task_ESP_Upload` | 3 | 512 | event | Init ESP8266 AP+TCP server → JSON serialize → AT+CIPSEND |
| `Task_OLED_Display` | 2 | 256 | 100ms | Read GatewayData under mutex → 4-line OLED (T/H/Soil/Lux) |
| `Task_CAN_Poll` | 2 | 256 | 3s | Query sensor if offline |

### CAN Protocol

- **Physical:** PB8=CAN_RX, PB9=CAN_TX, TJA1050, 500kbps, 120Ω terminators
- **Frame:** 29-bit extended ID, 8-byte DLC
- **Data layout:** `[0-1]=temp×10 [2-3]=humi×10 [4]=soil% [5-6]=lux [7]=reserved`
- **ID:** `[priority:3][msg_type:10][src:8][dst:8]`
- **Priority levels:** 0=alert, 1=command, 2=data, 3=heartbeat
- **ISR name:** MUST use `USB_LP_CAN1_RX0_IRQHandler` (STM32F103C8 medium-density vector name, shared with USB LP line)

### Interrupt Priority Configuration

NVIC Priority Group 4 (4-bit preemption, no sub-priority):
- FreeRTOS management range: 5–15 (`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5`)
- CAN1_RX0: preemption priority 6 (`USB_LP_CAN1_RX0_IRQn`)
- USART1: preemption priority 7
- Stack overflow detection: `configCHECK_FOR_STACK_OVERFLOW = 2` (enabled, hook in stm32f10x_it.c)

### Directory Map

| Directory | Purpose |
|-----------|---------|
| `firmware_sensor/` | Sensor node Keil project + app code |
| `firmware_gateway/` | Gateway Keil project + app code |
| `common/cmsis/` | ARM Cortex-M3 CMSIS + startup (`startup_stm32f10x_md.s`) |
| `common/std_periph_lib/` | STM32F10x Standard Peripheral Library v3.5.0 — read-only |
| `common/freertos/` | FreeRTOS kernel: `inc/`, `src/`, `port/` (heap_4.c) |
| `common/drivers/can/` | CAN driver (PB8/PB9, TJA1050, 500kbps) |
| `common/drivers/usart/` | USART1 driver (PA9/PA10, 115200) |
| `common/drivers/dht11/` | DHT11 one-wire driver (PB5) |
| `common/drivers/soil_moisture/` | Soil moisture ADC driver (PA0, ADC1_CH0) |
| `common/drivers/bh1750/` | BH1750 I2C driver (PB6/PB7, HW I2C1) |
| `common/drivers/oled/` | OLED SSD1306 bit-bang I2C (PB6/PB7) |
| `common/drivers/esp8266/` | ESP8266 AT command driver |
| `common/drivers/led/key/delay/` | Utility drivers |
| `common/utils/protocol/` | CAN protocol encoder/decoder + JSON builder |
| `docs/` | Architecture docs, hardware wiring, Mermaid diagrams |

### Key Design Patterns

- **Message queue for task decoupling:** CAN ISR → `xQueueSendToBackFromISR` → task receives via `xQueueReceive`. No flag-based handshake.
- **Mutex-guarded shared data:** `GatewayData_t` accessed by 3 tasks under `xGatewayMutex`.
- **Queue overwrite for latest data:** `xQueueOverwrite` for sensor data (only latest value matters).
- **AT command with busy flag:** `usart1_busy` prevents USART1 ISR from clearing buffer while `ESP_SendCmd` is polling.
- **Critical section at task creation:** `Start_Task` wraps `xTaskCreate` calls in `taskENTER_CRITICAL()` to prevent init races.
