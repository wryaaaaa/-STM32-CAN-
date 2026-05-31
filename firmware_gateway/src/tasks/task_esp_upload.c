/**
 * @file    task_esp_upload.c
 * @brief   ESP8266 TCP 上传任务实现
 */

#include "task_esp_upload.h"
#include "task_can_recv.h"
#include "can_driver.h"
#include "can_protocol.h"
#include "app_config.h"
#include "esp8266.h"
#include "usart.h"
#include "json_simple.h"
#include <stdio.h>
#include <string.h>

#define JSON_BUF_SIZE   256
static char s_json_buf[JSON_BUF_SIZE];
static uint32_t s_timestamp = 0;

static void process_tcp_cmd(const char *cmd_str)
{
    char cmd[16];
    if (json_parse_cmd(cmd_str, cmd, sizeof(cmd)) != 0) return;

    CAN_Frame_t query_frame;
    uint8_t query_type = CMD_QUERY_ALL;

    if      (strcmp(cmd, "query") == 0) query_type = CMD_QUERY_ALL;
    else if (strcmp(cmd, "temp")  == 0) query_type = CMD_QUERY_TEMP;
    else if (strcmp(cmd, "humi")  == 0) query_type = CMD_QUERY_HUMI;
    else return;

    can_build_query_cmd(&query_frame, query_type);
    CAN_SendFrame(&query_frame);
}

void Task_ESP_Upload(void *pvParameters)
{
    SensorData_t sensor_data;

    while (ESP8266TCP_Init() != 1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    while (1)
    {
        if (xQueueReceive(xQueueUpload, &sensor_data, pdMS_TO_TICKS(500)) == pdTRUE)
        {
            const char *status = (sensor_data.soil_moisture == 0xFF)
                               ? "alert" : "online";
            s_timestamp += 500;

            int len = json_build_data(s_json_buf, JSON_BUF_SIZE,
                                       sensor_data.temperature,
                                       sensor_data.humidity,
                                       sensor_data.soil_moisture,
                                       sensor_data.light,
                                       status, s_timestamp);
            ESP_SendData(0, s_json_buf, (uint16_t)len);
        }

        if (xQueueReceive(xQueueUARTCmd, s_json_buf, 0) == pdTRUE)
        {
            process_tcp_cmd(s_json_buf);
        }
    }
}
