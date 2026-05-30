/**
 * @file    task_can_poll.h
 * @brief   CAN 主动轮询任务 (网关→传感器的周期性查询)
 */

#ifndef __TASK_CAN_POLL_H
#define __TASK_CAN_POLL_H

#include "FreeRTOS.h"
#include "task.h"

void Task_CAN_Poll(void *pvParameters);

#endif /* __TASK_CAN_POLL_H */
