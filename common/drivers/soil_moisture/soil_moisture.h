/**
 * @file    soil_moisture.h
 * @brief   土壤湿度传感器驱动 (模拟量, ADC1-CH0/PA0)
 */

#ifndef __SOIL_MOISTURE_H
#define __SOIL_MOISTURE_H

#include "stm32f10x.h"

/**
 * @brief  初始化 ADC1-CH0 (PA0) 用于土壤湿度采集
 */
void SoilMoisture_Init(void);

/**
 * @brief  读取土壤湿度
 * @return 0~100 (百分比), 0=完全干燥, 100=完全浸湿
 */
uint8_t SoilMoisture_Read(void);

#endif /* __SOIL_MOISTURE_H */
