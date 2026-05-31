/**
 * @file    task_sensor_acq.c
 * @brief   传感器采集任务实现
 *          - 周期性读取 DHT11 (温湿度) + 土壤湿度(ADC) + BH1750(光照)
 *          - 失败超阈值时发告警标记
 *          - 将 SensorData_t 推入队列
 */

#include "task_sensor_acq.h"
#include "task_can_report.h"
#include "can_driver.h"
#include "dht11.h"
#include "soil_moisture.h"
#include "bh1750.h"
#include "led.h"

extern QueueHandle_t xQueueSensorData;

/* 内部状态 */
static uint8_t s_fail_count = 0;

void Task_Sensor_Acq(void *pvParameters)
{
    SensorData_t sensor_data;
    TickType_t   xLastWakeTime = xTaskGetTickCount();

    /* 初始化传感器 */
    DHT11_Init();
    SoilMoisture_Init();
    BH1750_Init(BH1750_ADDR_GND);
    LED_Init();

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SENSOR_ACQ_PERIOD_MS));

        /* 清零数据 */
        memset(&sensor_data, 0, sizeof(sensor_data));
        uint8_t all_ok = 1;

        /* ---- DHT11 温湿度 ---- */
        if (DHT11_Read_Data(&sensor_data.temperature, &sensor_data.humidity) != 0)
        {
            sensor_data.temperature = 0x7FFF;
            sensor_data.humidity    = 0x7FFF;
            all_ok = 0;
        }

        /* ---- 土壤湿度 (ADC) ---- */
        sensor_data.soil_moisture = SoilMoisture_Read();

        /* ---- BH1750 光照 ---- */
        sensor_data.light = BH1750_ReadLight(BH1750_ADDR_GND);

        /* 故障处理 */
        if (!all_ok)
        {
            s_fail_count++;
        }
        else
        {
            s_fail_count = 0;
            LED1_Turn();            /* LED1 闪表示采集成功 */
        }

        if (s_fail_count >= DHT11_MAX_FAIL_COUNT)
        {
            sensor_data.soil_moisture = 0xFF;  /* 告警标记 */
            s_fail_count = 0;
        }

        /* 推入队列 (覆盖旧值, 非阻塞) */
        xQueueOverwrite(xQueueSensorData, &sensor_data);
    }
}
