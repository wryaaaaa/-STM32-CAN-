#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"

void KEY_Init(void)
{
    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);      // 使能GPIOB时钟

    // 配置GPIO
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;      // KEY0=PB12, KEY1=PB13
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;               // 下拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;           // 速度50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                      // 初始化GPIOB    
}

uint8_t KEY0_Scan(void)
{
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 1)
    {
        vTaskDelay(10);
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 1);
        return 1;
    }
    return 0;
}

uint8_t KEY1_Scan(void)
{
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 1)
    {
        vTaskDelay(10);
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 1);
        return 1;
    }
    return 0;
}
