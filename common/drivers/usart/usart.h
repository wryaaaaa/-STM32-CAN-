/**
 * @file    usart.h
 * @brief   通用串口驱动 (USART1 用于 ESP8266 / 调试)
 */

#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* USART1 (ESP8266) 缓冲区 */
#define USART1_RX_MAX_LEN   512

extern char     USART1_RxBuffer[USART1_RX_MAX_LEN];
extern volatile uint16_t USART1_RxCount;

/* ESP8266 驱动正在使用缓冲区标志 (ISR 检查，防止冲突) */
extern volatile uint8_t usart1_busy;

/* ---- 引脚定义 (PA9=TX, PA10=RX) ---- */
#define USART1_GPIO_CLK     RCC_APB2Periph_GPIOA
#define USART1_CLK          RCC_APB2Periph_USART1
#define USART1_PORT         GPIOA
#define USART1_TX_PIN       GPIO_Pin_9
#define USART1_RX_PIN       GPIO_Pin_10

/* ---- API ---- */
void USART1_Init(uint32_t baudRate);
void USART1_ClearBuffer(void);

void USART_SendByte(USART_TypeDef *USARTx, uint8_t Data);
void USART_SendArray(USART_TypeDef *USARTx, uint8_t *Array, uint16_t Length);
void USART_SendString(USART_TypeDef *USARTx, char *String);
void USART_Printf(USART_TypeDef *USARTx, char *format, ...);

#endif /* __USART_H */
