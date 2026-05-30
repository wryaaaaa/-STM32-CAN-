/**
 * @file    task_sensor_acq.h
 * @brief   传感器采集任务 (DHT11 温湿度)
 */

#ifndef __TASK_SENSOR_ACQ_H
#define __TASK_SENSOR_ACQ_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "app_config.h"

extern QueueHandle_t xQueueSensorData;

void Task_Sensor_Acq(void *pvParameters);

#endif /* __TASK_SENSOR_ACQ_H */
