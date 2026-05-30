/**
 * @file    task_esp_upload.c
 * @brief   ESP8266 TCP 上传任务实现
 *          - 初始化 ESP8266 WiFi AP + TCP Server
 *          - 从上传队列取数据 → JSON 序列化 → TCP 发送
 *          - 处理手机端 TCP 命令 (QUERY)
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

/* ---- JSON 输出缓冲区 ---- */
#define JSON_BUF_SIZE   256
static char s_json_buf[JSON_BUF_SIZE];

/* ---- 时间戳 (ms) ---- */
static uint32_t s_timestamp = 0;

/**
 * @brief  处理 TCP 命令
 */
static void process_tcp_cmd(const char *cmd_str)
{
    char cmd[16];
    if (json_parse_cmd(cmd_str, cmd, sizeof(cmd)) != 0) return;

    /* 构造 CAN 查询帧发往传感器 */
    CAN_Frame_t query_frame;
    uint8_t query_type = CMD_QUERY_ALL;

    if (strcmp(cmd, "query") == 0)
    {
        query_type = CMD_QUERY_ALL;
    }
    else if (strcmp(cmd, "temp") == 0)
    {
        query_type = CMD_QUERY_TEMP;
    }
    else if (strcmp(cmd, "humi") == 0)
    {
        query_type = CMD_QUERY_HUMI;
    }
    else
    {
        return; /* 未知命令 */
    }

    can_build_query_cmd(&query_frame, query_type);
    CAN_SendFrame(&query_frame);
}

/**
 * @brief  TCP 命令接收任务 (从 UART 队列到 JSON 解析)
 */
void Task_ESP_Upload(void *pvParameters)
{
    SensorData_t sensor_data;

    /* ---- 1. 初始化 ESP8266 (阻塞至成功) ---- */
    while (ESP8266TCP_Init() != 1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    /* ---- 2. 主循环 ---- */
    while (1)
    {
        /* 等待上传队列数据 或 UART 命令 (500ms 超时) */
        if (xQueueReceive(xQueueUpload, &sensor_data, pdMS_TO_TICKS(500)) == pdTRUE)
        {
            /* 构建 JSON */
            const char *status = (sensor_data.soil_moisture == 0xFF)
                               ? "alert" : "online";

            /* 增时间戳 */
            s_timestamp += 500;

            int len = json_build_data(s_json_buf, JSON_BUF_SIZE,
                                       sensor_data.temperature,
                                       sensor_data.humidity,
                                       status, s_timestamp);

            /* TCP 发送 (ID=0 的单连接模式) */
            ESP_SendData(0, s_json_buf, (uint16_t)len);
        }

        /* 检查 UART 命令 (来自手机 TCP) */
        if (xQueueReceive(xQueueUARTCmd, s_json_buf, 0) == pdTRUE)
        {
            process_tcp_cmd(s_json_buf);
        }
    }
}
