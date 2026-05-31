/**
 * @file    task_can_recv.c
 * @brief   CAN 接收任务: 解析传感器帧 → 更新全局数据 → 推入上传队列
 */

#include "task_can_recv.h"
#include "can_protocol.h"
#include "app_config.h"
#include <string.h>

QueueHandle_t      xQueueCANRx    = NULL;
QueueHandle_t      xQueueUpload   = NULL;
QueueHandle_t      xQueueUARTCmd  = NULL;
GatewayData_t      g_GatewayData  = {0.0f, 0.0f, 0, 0, 0, 0};
SemaphoreHandle_t  xGatewayMutex  = NULL;

void Task_CAN_Recv(void *pvParameters)
{
    CAN_Frame_t   rx_frame;
    SensorData_t  sensor_data;

    CAN1_Init(CAN_BAUDRATE, CAN_FILTER_ID, CAN_FILTER_MASK);
    CAN_BindRxQueue(xQueueCANRx);

    while (1)
    {
        if (xQueueReceive(xQueueCANRx, &rx_frame, portMAX_DELAY) == pdTRUE)
        {
            uint32_t msg_type = can_get_msg_type(rx_frame.ext_id);

            switch (msg_type)
            {
                case CAN_TYPE_DATA_RPT:
                case CAN_TYPE_QUERY_ACK:
                    can_parse_sensor_data(&rx_frame, &sensor_data);

                    xSemaphoreTake(xGatewayMutex, portMAX_DELAY);
                    g_GatewayData.temperature     = sensor_data.temperature / 10.0f;
                    g_GatewayData.humidity        = sensor_data.humidity / 10.0f;
                    g_GatewayData.soil_moisture   = sensor_data.soil_moisture;
                    g_GatewayData.light           = sensor_data.light;
                    g_GatewayData.sensor_online   = 1;
                    g_GatewayData.last_update_tick = xTaskGetTickCount();
                    xSemaphoreGive(xGatewayMutex);

                    xQueueOverwrite(xQueueUpload, &sensor_data);
                    break;

                case CAN_TYPE_HEARTBEAT:
                    xSemaphoreTake(xGatewayMutex, portMAX_DELAY);
                    g_GatewayData.sensor_online   = 1;
                    g_GatewayData.last_update_tick = xTaskGetTickCount();
                    xSemaphoreGive(xGatewayMutex);
                    break;

                case CAN_TYPE_ALERT:
                    sensor_data.temperature  = 0x7FFF;
                    sensor_data.humidity     = 0x7FFF;
                    sensor_data.soil_moisture = 0xFF;
                    sensor_data.light        = 0;
                    xQueueOverwrite(xQueueUpload, &sensor_data);
                    xSemaphoreTake(xGatewayMutex, portMAX_DELAY);
                    g_GatewayData.sensor_online   = 1;
                    g_GatewayData.last_update_tick = xTaskGetTickCount();
                    xSemaphoreGive(xGatewayMutex);
                    break;

                default:
                    break;
            }
        }
    }
}
