# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

基于STM32与LoRa的智能农业大棚监测系统 — STM32F103C8 网关节点。通过 LoRa 无线接收远程传感器节点（DHT11温湿度）的数据，在 OLED 上显示，并通过 ESP8266 WiFi 模块上传至云端。

- **MCU:** STM32F103C8 (Cortex-M3, 64KB Flash @ 0x08000000, 20KB RAM @ 0x20000000, 72MHz)
- **RTOS:** FreeRTOS v202212.01 (preemptive, 5 priority levels, 17KB heap, 1ms tick)
- **Peripheral Library:** STM32F10x Standard Peripheral Library v3.5.0
- **Toolchain:** Keil MDK-ARM v5 (uVision), ARMCC v5.06.5, C99 mode
- **Build output:** `Objects/project.hex` (`CreateHexFile=1`)

## Build

```
# Open in Keil uVision and rebuild:
project.uvprojx   # Menu: Project → Rebuild all target files

# Or use UV4.exe command line:
UV4.exe -b project.uvprojx -t "Target 1" -j0 -o build.log

# Clean build artifacts:
.\keilkill.bat
```

Preprocessor define: `USE_STDPERIPH_DRIVER`. Include paths are in the project file at the `Cads → VariousControls → IncludePath` element. IntelliSense config in `.vscode/c_cpp_properties.json` points to a MinGW GCC (not the actual Keil compiler — expect false positives from VS Code static analysis).

## High-Level Architecture

### Task Model (FreeRTOS)

The application runs 3 persistent tasks under a mutex-protected shared data model:

| Task | Priority | Stack | Role |
|------|----------|-------|------|
| `Task_LoRa_Rx` | 4 (highest) | 256 words | Polls `Lora_Data_Ready` flag → parses `{T:xx.x,H:yy.y}` from USART2 buffer → updates `g_SystemData` under mutex |
| `Task_ESP_Upload` | 3 | 512 words | Initializes ESP8266 as WiFi AP+TCP server → polls `data_updated` flag → formats `"T:%.1f,H:%.1f"` → sends via `AT+CIPSEND` |
| `Task_OLED_Display` | 2 (lowest) | 256 words | Reads `g_SystemData` every 100ms under mutex → refreshes 3-line OLED display |

A `Start_Task` (priority 1) creates all three tasks inside a critical section, then deletes itself.

### Shared Data (Critical Section)

```c
typedef struct {
    float temperature;
    float humidity;
    uint8_t data_updated;   // Set by LoRa task, consumed/cleared by ESP task
    uint8_t net_connected;  // Set by ESP task when TCP init succeeds, read by OLED task
} System_Data_t;
```

Access to `g_SystemData` is guarded by `xSystemMutex` (mutex semaphore). The `data_updated` flag serves as a producer-consumer handshake between LoRa reception and ESP upload.

### Interrupt-Driven Data Reception

USART2 (LoRa, PA2-TX/PA3-RX, 9600 baud, APB1 bus) uses an RX interrupt (`USART2_IRQHandler` in [External_function/USART.c](External_function/USART.c)):
1. Bytes accumulate in `USART2_RxBuffer[]` until `'}'` is received
2. When `'}'` arrives, sets `Lora_Data_Ready = 1` and stops accepting new bytes
3. `Task_LoRa_Rx` polls this flag, parses the frame, then calls `Lora_ClearRxBuffer()` which resets the flag and re-enables RX interrupts
4. If buffer overflows before `'}'` appears, the buffer is forcefully cleared

This flag-based handshake prevents the ISR from overwriting data that hasn't been processed yet.

USART1 (ESP8266/Debug, PA9-TX/PA10-RX, 115200 baud, APB2 bus) uses a simpler ISR that just accumulates bytes with overflow protection — the ESP task polls the buffer via `strstr()` for AT command responses and connection state strings (`"CLOSED"`, `"CONNECT"`, `"ERROR"`, `"SEND OK"`).

### Directory Map

| Directory | Purpose |
|-----------|---------|
| `user/` | Application entry (`main.c`), FreeRTOS task definitions, ISR stubs, peripheral config header |
| `External_function/` | Device drivers: LoRa (E32 module via USART2), ESP8266 (AT commands via USART1), OLED (I2C SSD1306), DHT11, USART (generic tx/rx/printf for any USARTx), LED, KEY, Delay |
| `library_function/` | STM32F10x Standard Peripheral Library (GPIO, USART, SPI, I2C, TIM, ADC, DMA, RCC, NVIC, CAN, etc.) — read-only vendor code |
| `FreeRTOS/` | FreeRTOS kernel sources: `inc/` (headers), `src/` (tasks, queue, timers, event_groups, croutine, list), `port/` (ARM Cortex-M3 port + heap_4.c), `FreeRTOSConfig.h` at root |
| `start/` | Startup asm (`startup_stm32f10x_md.s` for medium-density), `system_stm32f10x.c` (SystemInit/clock tree), CMSIS core (`core_cm3.c/.h`), master header `stm32f10x.h` |
| `system/` | MCU peripheral init wrappers (ADC, BKP, DMA, Flash, NVIC, RTC, Tim, WDG) — thin initialization helpers |
| `Objects/` | Build output: `.o`, `.d`, `.axf`, `.hex` |
| `Listings/` | Linker map, disassembly listing |

### Interrupt Priority Configuration

NVIC Priority Group 4 (all 4 bits are preemption priority, no sub-priority):
- FreeRTOS management range: priorities 5–15 (via `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5`)
- System exceptions (PendSV, SysTick, SVC): handled by FreeRTOS kernel
- USART1_IRQn: preemption priority 7
- USART1_IRQn: preemption priority 7
- CAN1_RX0 (USB_LP_CAN1_RX0_IRQn): preemption priority 6
- **Important:** STM32F103C8 medium-density 启动文件使用的符号名是 `USB_LP_CAN1_RX0_IRQHandler`（CAN1 RX0 与 USB 共享中断线），不是 `CAN1_RX0_IRQHandler`
- Fault handlers (HardFault, MemManage, BusFault, UsageFault) all trap in infinite loops
- Stack overflow detection: `configCHECK_FOR_STACK_OVERFLOW = 2` (enabled)

### Key Design Patterns

- **Producer-consumer flag with mutex:** LoRa ISR → flag → task parses → mutex-guarded struct update → ESP task consumes under same mutex. This avoids queue overhead for simple sensor data.
- **AT command polling with timeout:** ESP8266 driver sends AT command, then busy-waits with `vTaskDelay(10ms)` increments checking for acknowledgment strings in the shared USART1 receive buffer.
- **Critical section at task creation:** `Start_Task` wraps `xTaskCreate` calls in `taskENTER_CRITICAL()/taskEXIT_CRITICAL()` to prevent any task from running before all tasks are created (prevents initialization races).
