/**
 * @file    main.c
 * @brief   传感器采集节点入口
 * @note    功能: DHT11采集 → CAN打包上报 → 响应网关查询指令
 * @hardware STM32F103C8 + TJA1050 + DHT11
 */

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "can_driver.h"
#include "app_config.h"
#include "task_can_report.h"
#include "task_sensor_acq.h"

/* 启动任务: 创建其他任务后自删除 */
static void Start_Task(void *pvParameters)
{
    /* 创建队列 */
    xQueueSensorData = xQueueCreate(QUEUE_SENSOR_DATA_LEN, sizeof(SensorData_t));
    xQueueCANCmd     = xQueueCreate(QUEUE_CAN_CMD_LEN,     sizeof(CAN_Frame_t));

    if (xQueueSensorData != NULL && xQueueCANCmd != NULL)
    {
        taskENTER_CRITICAL();

        /* 创建 CAN 上报任务 (最高优先级，确保及时发送) */
        xTaskCreate((TaskFunction_t)Task_CAN_Report,
                    "CAN_Rpt", STK_CAN_REPORT, NULL,
                    PRIO_CAN_REPORT, NULL);

        /* 创建传感器采集任务 */
        xTaskCreate((TaskFunction_t)Task_Sensor_Acq,
                    "Sensor", STK_SENSOR_ACQ, NULL,
                    PRIO_SENSOR_ACQ, NULL);

        /* 创建 CAN 指令处理任务 */
        xTaskCreate((TaskFunction_t)Task_CAN_CmdHandler,
                    "CAN_Cmd", STK_CAN_CMD, NULL,
                    PRIO_CAN_CMD, NULL);

        taskEXIT_CRITICAL();
    }

    /* 启动任务自删除 */
    vTaskDelete(NULL);
}

/**
 * @brief  main 入口
 */
int main(void)
{
    /* NVIC 优先级分组: 4位抢占优先级 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* 创建启动任务 */
    xTaskCreate((TaskFunction_t)Start_Task,
                "Start", 128, NULL, 1, NULL);

    /* 启动 FreeRTOS 调度器 */
    vTaskStartScheduler();

    /* 不应到达这里 */
    while (1);
}
