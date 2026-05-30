/**
 * @file    stm32f10x_it.c
 * @brief   传感器节点中断服务函数
 */

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS 内核使用的中断向量通过宏重映射:
 * SVC_Handler    → vPortSVCHandler
 * PendSV_Handler → xPortPendSVHandler
 * SysTick_Handler→ xPortSysTickHandler
 * 这三个在 FreeRTOSConfig.h 中定义，本文件不再重复实现
 */

void NMI_Handler(void) {}
void HardFault_Handler(void) { while (1); }
void MemManage_Handler(void) { while (1); }
void BusFault_Handler(void) { while (1); }
void UsageFault_Handler(void) { while (1); }
void DebugMon_Handler(void) {}
