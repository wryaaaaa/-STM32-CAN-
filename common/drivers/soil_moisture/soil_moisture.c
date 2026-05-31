/**
 * @file    soil_moisture.c
 * @brief   土壤湿度传感器驱动实现
 *          使用 ADC1 通道0 (PA0), 12位精度
 */

#include "soil_moisture.h"

/* 采样次数 (用于均值滤波) */
#define SM_SAMPLE_COUNT  8

void SoilMoisture_Init(void)
{
    GPIO_InitTypeDef  gpio;

    /* 1. 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);   /* ADC 时钟 = 72/6 = 12MHz */

    /* 2. GPIO: PA0 = 模拟输入 */
    gpio.GPIO_Pin  = GPIO_Pin_0;
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &gpio);

    /* 3. ADC1 配置 */
    ADC_DeInit(ADC1);
    ADC_InitTypeDef adc_init = {
        .ADC_Mode               = ADC_Mode_Independent,
        .ADC_ScanConvMode       = DISABLE,
        .ADC_ContinuousConvMode = DISABLE,
        .ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None,
        .ADC_DataAlign          = ADC_DataAlign_Right,
        .ADC_NbrOfChannel       = 1
    };
    ADC_Init(ADC1, &adc_init);

    /* 4. 规则通道配置: CH0, 采样时间 55.5 cycle (高阻抗信号需要长采样) */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

    /* 5. 校准 */
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

uint8_t SoilMoisture_Read(void)
{
    uint32_t sum = 0;

    for (uint8_t i = 0; i < SM_SAMPLE_COUNT; i++)
    {
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
        sum += ADC_GetConversionValue(ADC1);
    }

    /* 均值: 12位 ADC 范围 0~4095, 取反并映射到 0~100%
     * 传感器通常: 水多 → 电压低 (ADC值小), 干燥 → 电压高 (ADC值大)
     * 取反: humidity% = 100 - (adc_val / 4095 * 100) */
    uint16_t avg = (uint16_t)(sum / SM_SAMPLE_COUNT);
    uint8_t  pct = (uint8_t)(100 - (avg * 100 / 4095));

    return (pct > 100) ? 0 : pct;
}
