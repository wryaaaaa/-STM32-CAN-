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
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_5;      // 假设按键连接在PB6和PB5
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;               // 下拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;           // 速度50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                      // 初始化GPIOB    
}

uint8_t KEY0_Scan(void)
{
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 1)           // 按键按下
    {
        vTaskDelay(10);
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 1);   // 等待按键释放
        return 1;                                               // 返回按键按下标志
    }
    return 0;                                                   // 无按键按下
}

uint8_t KEY1_Scan(void)
{
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 1)           // 按键按下
    {
        vTaskDelay(10);
        while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 1);   // 等待按键释放
        return 1;                                               // 返回按键按下标志
    }
    return 0;                                                   // 无按键按下
}
