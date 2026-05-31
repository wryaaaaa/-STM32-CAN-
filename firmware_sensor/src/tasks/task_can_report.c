/**
 * @file    task_can_report.c
 * @brief   CAN 数据上报 + 指令处理任务实现
 */

#include "task_can_report.h"
#include "can_driver.h"
#include "can_protocol.h"
#include "dht11.h"
#include "bh1750.h"
#include "soil_moisture.h"
#include "app_config.h"
#include "led.h"

/* 队列定义 */
QueueHandle_t xQueueSensorData = NULL;
QueueHandle_t xQueueCANCmd     = NULL;

void Task_CAN_Report(void *pvParameters)
{
    SensorData_t sensor_data;
    CAN_Frame_t  tx_frame;
    TickType_t   xLastHeartbeat = xTaskGetTickCount();

    /* 初始化 CAN */
    CAN1_Init(CAN_BAUDRATE, CAN_FILTER_ID, CAN_FILTER_MASK);
    CAN_BindRxQueue(xQueueCANCmd);

    while (1)
    {
        /* 阻塞等待传感器数据 */
        if (xQueueReceive(xQueueSensorData, &sensor_data, portMAX_DELAY) == pdTRUE)
        {
            if (sensor_data.soil_moisture == 0xFF)
            {
                /* 告警帧 */
                can_build_alert(&tx_frame, DEV_STATUS_DHT11_ERR);
                CAN_SendFrame(&tx_frame);
                LED2_Turn();
            }
            else
            {
                /* 正常数据帧 */
                can_build_data_report(&tx_frame, &sensor_data);
                CAN_SendFrame(&tx_frame);
            }
        }

        /* 心跳 (5秒) */
        if ((xTaskGetTickCount() - xLastHeartbeat) >= pdMS_TO_TICKS(HEARTBEAT_PERIOD_MS))
        {
            xLastHeartbeat = xTaskGetTickCount();
            can_build_heartbeat(&tx_frame, DEV_STATUS_OK);
            CAN_SendFrame(&tx_frame);
        }
    }
}

void Task_CAN_CmdHandler(void *pvParameters)
{
    CAN_Frame_t  rx_frame;
    SensorData_t sensor_data;

    while (1)
    {
        if (xQueueReceive(xQueueCANCmd, &rx_frame, portMAX_DELAY) == pdTRUE)
        {
            if (can_get_msg_type(rx_frame.ext_id) == CAN_TYPE_QUERY_CMD)
            {
                /* 即时采集 */
                memset(&sensor_data, 0, sizeof(sensor_data));
                DHT11_Read_Data(&sensor_data.temperature, &sensor_data.humidity);
                sensor_data.soil_moisture = SoilMoisture_Read();
                sensor_data.light         = BH1750_ReadLight(BH1750_ADDR_GND);

                CAN_Frame_t tx_frame;
                can_build_query_ack(&tx_frame, &sensor_data);
                CAN_SendFrame(&tx_frame);
            }
        }
    }
}
