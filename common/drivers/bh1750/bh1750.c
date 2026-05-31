/**
 * @file    bh1750.c
 * @brief   BH1750FVI 驱动实现 (硬件 I2C1: PB6=SCL, PB7=SDA)
 * @note    连续高分辨率模式, 测量时间约 120ms
 */

#include "bh1750.h"
#include "FreeRTOS.h"
#include "task.h"

/* I2C 超时 */
#define I2C_TIMEOUT  0xFFFFFUL

/* BH1750 指令 */
#define BH1750_POWER_ON         0x01
#define BH1750_RESET            0x07
#define BH1750_CONT_HRES_MODE   0x10   /* 连续高分辨率: 1 lux, 120ms */

/* ---- 低层 I2C 操作 ---- */

static uint8_t I2C_WaitEvent(I2C_TypeDef *I2Cx, uint32_t event)
{
    uint32_t timeout = I2C_TIMEOUT;
    while (!I2C_CheckEvent(I2Cx, event))
    {
        if (--timeout == 0) return 1;
        taskYIELD();   /* 避免忙等锁死低优先级任务 */
    }
    return 0;
}

static uint8_t I2C_WriteByte(I2C_TypeDef *I2Cx, uint8_t addr, uint8_t data)
{
    I2C_GenerateSTART(I2Cx, ENABLE);
    if (I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) return 1;

    I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Transmitter);
    if (I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) return 1;

    I2C_SendData(I2Cx, data);
    if (I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return 1;

    I2C_GenerateSTOP(I2Cx, ENABLE);
    return 0;
}

static uint8_t I2C_ReadBytes(I2C_TypeDef *I2Cx, uint8_t addr, uint8_t *buf, uint8_t len)
{
    I2C_GenerateSTART(I2Cx, ENABLE);
    if (I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) return 1;

    I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Receiver);
    if (I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) return 1;

    for (uint8_t i = 0; i < len; i++)
    {
        /* 倒数第二个字节发送 NACK */
        if (i == len - 1)
            I2C_AcknowledgeConfig(I2Cx, DISABLE);

        if (I2C_WaitEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED)) return 1;
        buf[i] = I2C_ReceiveData(I2Cx);
    }

    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    I2C_GenerateSTOP(I2Cx, ENABLE);
    return 0;
}

/* ---- 公开 API ---- */

void BH1750_Init(uint8_t addr)
{
    GPIO_InitTypeDef  gpio;
    I2C_InitTypeDef   i2c;

    /* 1. 时钟: GPIOB + I2C1 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    /* 2. GPIO: PB6=SCL, PB7=SDA (复用开漏输出) */
    gpio.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    gpio.GPIO_Mode  = GPIO_Mode_AF_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);

    /* 3. I2C1: 标准模式 100kHz (APB1=36MHz) */
    I2C_DeInit(I2C1);
    i2c.I2C_Mode              = I2C_Mode_I2C;
    i2c.I2C_DutyCycle         = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1       = 0x00;
    i2c.I2C_Ack               = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    i2c.I2C_ClockSpeed        = 100000;
    I2C_Init(I2C1, &i2c);
    I2C_Cmd(I2C1, ENABLE);

    /* 4. BH1750 上电 */
    I2C_WriteByte(I2C1, addr, BH1750_POWER_ON);

    /* 5. 设置连续高分辨率模式 (测量时间 120ms) */
    I2C_WriteByte(I2C1, addr, BH1750_CONT_HRES_MODE);

    /* 6. 等待第一次测量完成 (180ms, 留余量确保数据有效) */
    vTaskDelay(pdMS_TO_TICKS(180));
}

uint16_t BH1750_ReadLight(uint8_t addr)
{
    uint8_t buf[2] = {0};

    if (I2C_ReadBytes(I2C1, addr, buf, 2) != 0)
        return 0;

    /* BH1750 返回 16-bit 原始值, lux = raw / 1.2 = raw * 10 / 12 (整数运算) */
    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    return (uint16_t)((raw * 10UL) / 12UL);
}
