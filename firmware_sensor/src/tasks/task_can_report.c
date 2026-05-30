/**
 * @file    task_can_report.c
 * @brief   CAN 数据上报 + 指令处理任务实现
 */

#include "task_can_report.h"
#include "can_driver.h"
#include "can_protocol.h"
#include "app_config.h"
#include "led.h"

/* 队列定义 */
QueueHandle_t xQueueSensorData = NULL;
QueueHandle_t xQueueCANCmd     = NULL;

/**
 * @brief  任务: 定时上报传感器数据 + 心跳
 *         从传感器数据队列取数 → 打包CAN帧 → 发送
 */
void Task_CAN_Report(void *pvParameters)
{
    SensorData_t sensor_data;
    CAN_Frame_t  tx_frame;
    TickType_t   xLastWakeTime = xTaskGetTickCount();
    TickType_t   xLastHeartbeat = xTaskGetTickCount();

    /* 初始化 CAN */
    CAN1_Init(CAN_BAUDRATE, CAN_FILTER_ID, CAN_FILTER_MASK);

    /* 绑定接收队列 (给 Task_CAN_CmdHandler 用) */
    CAN_BindRxQueue(xQueueCANCmd);

    while (1)
    {
        /* 100ms 周期检查 */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));

        /* ---- 数据上报 ---- */
        if (xQueueReceive(xQueueSensorData, &sensor_data, 0) == pdTRUE)
        {
            /* 判断是否为告警数据 */
            if (sensor_data.soil_moisture == 0xFF)
            {
                /* 发送告警帧 (最高优先级) */
                can_build_alert(&tx_frame, DEV_STATUS_DHT11_ERR);
                CAN_SendFrame(&tx_frame);
                LED2_Turn();    /* LED2 告警指示 */
            }
            else
            {
                /* 正常数据帧 */
                can_build_data_report(&tx_frame, &sensor_data);
                CAN_SendFrame(&tx_frame);
            }
        }

        /* ---- 心跳 (5秒周期) ---- */
        if ((xTaskGetTickCount() - xLastHeartbeat) >= pdMS_TO_TICKS(HEARTBEAT_PERIOD_MS))
        {
            xLastHeartbeat = xTaskGetTickCount();
            can_build_heartbeat(&tx_frame, DEV_STATUS_OK);
            CAN_SendFrame(&tx_frame);
        }
    }
}

/**
 * @brief  任务: 处理网关发来的 CAN 查询指令
 */
void Task_CAN_CmdHandler(void *pvParameters)
{
    CAN_Frame_t  rx_frame;
    SensorData_t sensor_data;

    while (1)
    {
        /* 阻塞等待 CAN 命令帧 */
        if (xQueueReceive(xQueueCANCmd, &rx_frame, portMAX_DELAY) == pdTRUE)
        {
            uint32_t msg_type = can_get_msg_type(rx_frame.ext_id);

            if (msg_type == CAN_TYPE_QUERY_CMD)
            {
                /* 收到查询指令 → 立刻读一次传感器 */
                uint8_t query_type = rx_frame.data[0];

                if (DHT11_Read_Data(&sensor_data.temperature, &sensor_data.humidity) == 0)
                {
                    sensor_data.soil_moisture = 0;
                    sensor_data.light         = 0;

                    CAN_Frame_t tx_frame;

                    /* 根据查询类型决定回复内容 */
                    if (query_type == CMD_QUERY_TEMP)
                    {
                        sensor_data.humidity = 0; /* 仅温度 */
                    }
                    else if (query_type == CMD_QUERY_HUMI)
                    {
                        sensor_data.temperature = 0; /* 仅湿度 */
                    }

                    /* 发送应答帧 */
                    can_build_query_ack(&tx_frame, &sensor_data);
                    CAN_SendFrame(&tx_frame);
                }
            }
        }
    }
}
