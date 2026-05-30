/**
 * @file    dht11.c
 * @brief   DHT11 驱动实现 (PB5, 使用Delay微秒级时序)
 */

#include "dht11.h"
#include "delay.h"

/* DHT11 引脚: PB5 */
#define DHT11_PORT      GPIOB
#define DHT11_PIN       GPIO_Pin_5
#define DHT11_CLK       RCC_APB2Periph_GPIOB

/* 宏: 切换引脚方向 */
#define DHT11_OUT()     { GPIO_InitTypeDef g = {DHT11_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP}; \
                          GPIO_Init(DHT11_PORT, &g); }
#define DHT11_IN()      { GPIO_InitTypeDef g = {DHT11_PIN, GPIO_Speed_50MHz, GPIO_Mode_IPU};  \
                          GPIO_Init(DHT11_PORT, &g); }

#define DHT11_DQ_H()    GPIO_SetBits(DHT11_PORT, DHT11_PIN)
#define DHT11_DQ_L()    GPIO_ResetBits(DHT11_PORT, DHT11_PIN)
#define DHT11_READ()    GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN)

void DHT11_Init(void)
{
    RCC_APB2PeriphClockCmd(DHT11_CLK, ENABLE);
    DHT11_OUT();
    DHT11_DQ_H();
}

/**
 * @brief  从 DHT11 读取一个字节
 */
static uint8_t DHT11_ReadByte(void)
{
    uint8_t i, data = 0;
    uint16_t timeout;
    for (i = 0; i < 8; i++)
    {
        /* 等待低电平结束 (50us) */
        timeout = 500;
        while (DHT11_READ() == 0 && --timeout > 0);
        if (timeout == 0) return 0;
        Delay_us(40);

        data <<= 1;
        if (DHT11_READ() == 1)
        {
            data |= 0x01;
        }

        /* 等待高电平结束 */
        timeout = 500;
        while (DHT11_READ() == 1 && --timeout > 0);
        if (timeout == 0) return 0;
    }
    return data;
}

/**
 * @brief  读取温湿度 (带校验)
 */
uint8_t DHT11_Read_Data(int16_t *temperature, int16_t *humidity)
{
    uint8_t buf[5] = {0};
    uint8_t i;

    /* ---- 1. 发送起始信号 ---- */
    DHT11_OUT();
    DHT11_DQ_L();
    Delay_ms(20);           /* > 18ms */
    DHT11_DQ_H();
    Delay_us(30);           /* 20~40us */

    /* ---- 2. 切换到输入，等待应答 ---- */
    DHT11_IN();

    /* DHT11 应答: 拉低 80us → 拉高 80us */
    if (DHT11_READ() != 0) return 1;           /* 等待拉低 */
    Delay_us(80);
    if (DHT11_READ() != 1) return 1;           /* 等待拉高 */
    Delay_us(80);

    /* ---- 3. 读取 5 字节 ---- */
    for (i = 0; i < 5; i++)
    {
        buf[i] = DHT11_ReadByte();
    }

    /* DHT11 拉低结束 */
    DHT11_OUT();
    DHT11_DQ_H();

    /* ---- 4. 校验 ---- */
    if (buf[4] != (uint8_t)(buf[0] + buf[1] + buf[2] + buf[3]))
    {
        return 1;   /* 校验失败 */
    }

    /* ---- 5. 输出 ×10 格式 ---- */
    *humidity    = (int16_t)buf[0] * 10;  /* 湿度整数部分 ×10 */
    *temperature = (int16_t)buf[2] * 10;  /* 温度整数部分 ×10 */

    return 0;
}
