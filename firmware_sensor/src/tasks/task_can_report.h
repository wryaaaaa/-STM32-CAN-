/**
 * @file    task_can_report.h
 * @brief   CAN数据上报任务 + CAN命令处理任务
 */

#ifndef __TASK_CAN_REPORT_H
#define __TASK_CAN_REPORT_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

extern QueueHandle_t xQueueSensorData;
extern QueueHandle_t xQueueCANCmd;

void Task_CAN_Report(void *pvParameters);
void Task_CAN_CmdHandler(void *pvParameters);

#endif /* __TASK_CAN_REPORT_H */
