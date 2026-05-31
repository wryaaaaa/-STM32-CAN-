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
 */

void NMI_Handler(void) {}
void HardFault_Handler(void) { while (1); }
void MemManage_Handler(void) { while (1); }
void BusFault_Handler(void) { while (1); }
void UsageFault_Handler(void) { while (1); }
void DebugMon_Handler(void) {}

/**
 * @brief  栈溢出钩子 (configCHECK_FOR_STACK_OVERFLOW=2 时调用)
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* 栈溢出 — 死循环等待调试器介入 */
    (void)xTask;
    (void)pcTaskName;
    while (1);
}
