/**
 * @file    usart.c
 * @brief   串口驱动实现 (仅 USART1, 网关使用)
 */

#include "usart.h"

char     USART1_RxBuffer[USART1_RX_MAX_LEN];
volatile uint16_t USART1_RxCount = 0;
volatile uint8_t  usart1_busy    = 0;

void USART1_ClearBuffer(void)
{
    memset(USART1_RxBuffer, 0, USART1_RX_MAX_LEN);
    USART1_RxCount = 0;
}

void USART1_Init(uint32_t baudRate)
{
    GPIO_InitTypeDef  gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef  nvic;

    /* 时钟 */
    RCC_APB2PeriphClockCmd(USART1_GPIO_CLK | USART1_CLK, ENABLE);

    /* GPIO: TX=AF_PP, RX=IN_FLOATING */
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Pin   = USART1_TX_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USART1_PORT, &gpio);

    gpio.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    gpio.GPIO_Pin   = USART1_RX_PIN;
    GPIO_Init(USART1_PORT, &gpio);

    /* USART */
    usart.USART_BaudRate            = baudRate;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    usart.USART_Parity              = USART_Parity_No;
    usart.USART_StopBits            = USART_StopBits_1;
    usart.USART_WordLength          = USART_WordLength_8b;
    USART_Init(USART1, &usart);

    /* NVIC */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    nvic.NVIC_IRQChannel                   = USART1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 7;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);

    USART_Cmd(USART1, ENABLE);
}

void USART_SendByte(USART_TypeDef *USARTx, uint8_t Data)
{
    USART_SendData(USARTx, Data);
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
}

void USART_SendArray(USART_TypeDef *USARTx, uint8_t *Array, uint16_t Length)
{
    for (uint16_t i = 0; i < Length; i++)
        USART_SendByte(USARTx, Array[i]);
}

void USART_SendString(USART_TypeDef *USARTx, char *String)
{
    while (*String)
        USART_SendByte(USARTx, (uint8_t)*String++);
}

void USART_Printf(USART_TypeDef *USARTx, char *format, ...)
{
    char buf[256];
    va_list arg;
    va_start(arg, format);
    vsnprintf(buf, sizeof(buf), format, arg);
    va_end(arg);
    USART_SendString(USARTx, buf);
}

int fputc(int ch, FILE *f)
{
    USART_SendByte(USART1, (uint8_t)ch);
    return ch;
}
