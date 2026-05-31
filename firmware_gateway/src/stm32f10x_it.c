/**
 * @file    stm32f10x_it.c
 * @brief   网关节点中断服务函数
 */

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "usart.h"
#include "task_can_recv.h"
#include <string.h>

/* ============================================================
 *               系统异常处理
 * ============================================================ */

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
    (void)xTask;
    (void)pcTaskName;
    while (1);
}

/* ============================================================
 *               USART1 中断 (ESP8266 / 手机命令)
 * ============================================================ */

extern char USART1_RxBuffer[];
extern volatile uint16_t USART1_RxCount;
extern volatile uint8_t  usart1_busy;    /* ESP8266驱动正在使用缓冲区 */

void USART1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        uint8_t ch = USART_ReceiveData(USART1);

        /* 累积到缓冲区 */
        if (USART1_RxCount < (USART1_RX_MAX_LEN - 1))
        {
            USART1_RxBuffer[USART1_RxCount++] = (char)ch;
            USART1_RxBuffer[USART1_RxCount] = '\0';

            /* 仅当ESP8266驱动不在使用缓冲区时，才推送命令并清空 */
            if ((ch == '\n' || ch == '}') && !usart1_busy)
            {
                if (xQueueUARTCmd != NULL)
                {
                    xQueueSendToBackFromISR(xQueueUARTCmd,
                        USART1_RxBuffer, &xHigherPriorityTaskWoken);

                    memset(USART1_RxBuffer, 0, USART1_RX_MAX_LEN);
                    USART1_RxCount = 0;
                }
            }
        }
        else
        {
            /* 缓冲区溢出 → 清空 */
            memset(USART1_RxBuffer, 0, USART1_RX_MAX_LEN);
            USART1_RxCount = 0;
        }

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
