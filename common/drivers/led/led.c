#include "stm32f10x.h"                  // Device header

void LED_Init(void)
{
    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);       // 使能GPIOB时钟

    // 配置GPIO
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;      // 假设LED连接在PB3和PB4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;            // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;           // 速度50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                      // 初始化GPIOB

    // 初始化时关闭LED
    GPIO_ResetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_4);
}

void LED1_Turn(void)
{
    // 切换LED1状态 (假设LED1连接在PB3)
    if(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_3))
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_3); // 关闭LED1
    }
    else
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_3);   // 打开LED1
    }
}

void LED2_Turn(void)
{
    // 切换LED2状态 (假设LED2连接在PB4)
    if(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_4))
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_4); // 关闭LED2
    }
    else
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_4);   // 打开LED2
    }
}
