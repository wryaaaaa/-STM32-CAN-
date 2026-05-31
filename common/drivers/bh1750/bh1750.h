/**
 * @file    bh1750.h
 * @brief   BH1750FVI 数字光照传感器驱动 (I2C1, PB6=SCL/PB7=SDA)
 */

#ifndef __BH1750_H
#define __BH1750_H

#include "stm32f10x.h"

/* BH1750 I2C 地址 (ADDR引脚接地=0x23, 接VCC=0x5C) */
#define BH1750_ADDR_GND  0x46   /* 7-bit: 0x23, 左移1位 = 0x46 */
#define BH1750_ADDR_VCC  0xB8   /* 7-bit: 0x5C, 左移1位 = 0xB8 */

/**
 * @brief  初始化 BH1750 (I2C1, 连续高分辨率模式)
 * @param  addr  I2C 地址 (BH1750_ADDR_GND 或 BH1750_ADDR_VCC)
 */
void BH1750_Init(uint8_t addr);

/**
 * @brief  读取光照强度
 * @param  addr  I2C 地址
 * @return 光照强度 (lux), 0=读取失败
 */
uint16_t BH1750_ReadLight(uint8_t addr);

#endif /* __BH1750_H */
