/**
 * @file    task_esp_upload.h
 * @brief   ESP8266 TCP 上传任务
 */

#ifndef __TASK_ESP_UPLOAD_H
#define __TASK_ESP_UPLOAD_H

#include "FreeRTOS.h"
#include "task.h"

void Task_ESP_Upload(void *pvParameters);

#endif /* __TASK_ESP_UPLOAD_H */
