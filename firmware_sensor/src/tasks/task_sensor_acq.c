/**
 * @file    task_sensor_acq.c
 * @brief   传感器采集任务实现
 *          - 周期性读取 DHT11 温湿度
 *          - 失败超阈值时发告警
 *          - 将 SensorData_t 推入队列
 */

#include "task_sensor_acq.h"
#include "task_can_report.h"
#include "dht11.h"
#include "led.h"

extern QueueHandle_t xQueueSensorData;

/* 内部状态: 连续失败计数 */
static uint8_t s_fail_count = 0;

void Task_Sensor_Acq(void *pvParameters)
{
    SensorData_t sensor_data;
    TickType_t   xLastWakeTime = xTaskGetTickCount();

    /* 初始化 DHT11 */
    DHT11_Init();

    /* 初始化 LED */
    LED_Init();

    while (1)
    {
        /* 精确周期延时 */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SENSOR_ACQ_PERIOD_MS));

        /* 读取 DHT11 */
        if (DHT11_Read_Data(&sensor_data.temperature, &sensor_data.humidity) == 0)
        {
            /* 读取成功 */
            sensor_data.soil_moisture = 0;  /* 预留 */
            sensor_data.light         = 0;  /* 预留 */
            s_fail_count = 0;

            /* LED1 闪一下表示采集成功 */
            LED1_Turn();

            /* 推入队列 (非阻塞，满则丢弃旧帧) */
            xQueueOverwrite(xQueueSensorData, &sensor_data);
        }
        else
        {
            /* 读取失败，累加计数 */
            s_fail_count++;
            if (s_fail_count >= DHT11_MAX_FAIL_COUNT)
            {
                /* 触发告警: 推入一个故障标记 */;
                sensor_data.temperature  = 0x7FFF; /* 无效值 */
                sensor_data.humidity     = 0x7FFF;
                sensor_data.soil_moisture = 0xFF;  /* 故障标记 */
                sensor_data.light        = 0;
                xQueueOverwrite(xQueueSensorData, &sensor_data);
            }
        }
    }
}
