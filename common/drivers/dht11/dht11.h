/**
 * @file    dht11.h
 * @brief   DHT11 温湿度传感器驱动
 */

#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"

/**
 * @brief  初始化 DHT11 (PB5)
 */
void DHT11_Init(void);

/**
 * @brief  读取温湿度
 * @param  temperature  输出: 温度 ×10 (如 255 = 25.5°C)
 * @param  humidity     输出: 湿度 ×10 (如 600 = 60.0%)
 * @return 0=成功, 1=失败
 */
uint8_t DHT11_Read_Data(int16_t *temperature, int16_t *humidity);

#endif /* __DHT11_H */
