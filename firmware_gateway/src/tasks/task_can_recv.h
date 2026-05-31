/**
 * @file    task_can_recv.h
 * @brief   网关 CAN 接收任务
 */

#ifndef __TASK_CAN_RECV_H
#define __TASK_CAN_RECV_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "can_driver.h"

/* 队列 */
extern QueueHandle_t xQueueCANRx;     /* CAN接收帧队列 */
extern QueueHandle_t xQueueUpload;    /* 上传数据队列 */
extern QueueHandle_t xQueueUARTCmd;   /* UART命令队列 */

/* 全局数据互斥锁 */
extern SemaphoreHandle_t xGatewayMutex;

/* 全局数据 (供 OLED 任务读取) */
typedef struct {
    float    temperature;
    float    humidity;
    uint8_t  soil_moisture;   /* 土壤湿度 0~100% */
    uint16_t light;           /* 光照强度 lux */
    uint8_t  sensor_online;   /* 1=在线, 0=离线 */
    uint32_t last_update_tick;
} GatewayData_t;

extern GatewayData_t g_GatewayData;

void Task_CAN_Recv(void *pvParameters);

#endif /* __TASK_CAN_RECV_H */
