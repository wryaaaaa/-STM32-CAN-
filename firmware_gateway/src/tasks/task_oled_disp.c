/**
 * @file    task_oled_disp.c
 * @brief   OLED 显示任务实现
 *          - 100ms 刷新周期
 *          - 显示温湿度、传感器在线状态
 */

#include "task_oled_disp.h"
#include "task_can_recv.h"
#include "app_config.h"
#include "oled.h"

void Task_OLED_Display(void *pvParameters)
{
    float   show_temp = 0.0f;
    float   show_humi = 0.0f;
    uint8_t is_online = 0;

    OLED_Init();
    OLED_Clear();
    OLED_ShowString(1, 1, "CAN Gateway");
    OLED_ShowString(2, 1, "Init...");

    while (1)
    {
        /* 读取全局数据 (互斥锁保护) */
        xSemaphoreTake(xGatewayMutex, portMAX_DELAY);
        show_temp  = g_GatewayData.temperature;
        show_humi  = g_GatewayData.humidity;
        is_online  = g_GatewayData.sensor_online;

        /* 超时检测: 10秒无更新 → 传感器离线 */
        if ((xTaskGetTickCount() - g_GatewayData.last_update_tick)
             > pdMS_TO_TICKS(SENSOR_OFFLINE_TIMEOUT))
        {
            is_online = 0;
            g_GatewayData.sensor_online = 0;
        }
        xSemaphoreGive(xGatewayMutex);

        OLED_Clear();

        if (!is_online)
        {
            OLED_ShowString(1, 1, "Sensor Offline");
            OLED_ShowString(3, 1, "Waiting...");
        }
        else
        {
            OLED_ShowString(1, 1, "Temp: ");
            OLED_ShowFloatNum(1, 7, show_temp, 2, 1);
            OLED_ShowString(1, 13, "C");

            OLED_ShowString(2, 1, "Humi: ");
            OLED_ShowFloatNum(2, 7, show_humi, 2, 1);
            OLED_ShowString(2, 13, "%");

            OLED_ShowString(3, 1, "CAN: Online");
        }

        vTaskDelay(pdMS_TO_TICKS(OLED_REFRESH_PERIOD_MS));
    }
}
