/**
 * @file    task_oled_disp.h
 * @brief   OLED 显示任务
 */

#ifndef __TASK_OLED_DISP_H
#define __TASK_OLED_DISP_H

#include "FreeRTOS.h"
#include "task.h"

void Task_OLED_Display(void *pvParameters);

#endif /* __TASK_OLED_DISP_H */
