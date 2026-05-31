/**
 * @file    task_oled_disp.c
 * @brief   OLED 显示任务 — 4行轮播: 温度/湿度/土壤/光照
 */

#include "task_oled_disp.h"
#include "task_can_recv.h"
#include "app_config.h"
#include "oled.h"

void Task_OLED_Display(void *pvParameters)
{
    float    show_temp = 0.0f;
    float    show_humi = 0.0f;
    uint8_t  show_soil = 0;
    uint16_t show_light = 0;
    uint8_t  is_online  = 0;

    OLED_Init();
    OLED_Clear();
    OLED_ShowString(1, 1, "CAN Gateway v2");
    OLED_ShowString(2, 1, "Init sensors...");

    while (1)
    {
        xSemaphoreTake(xGatewayMutex, portMAX_DELAY);
        show_temp  = g_GatewayData.temperature;
        show_humi  = g_GatewayData.humidity;
        show_soil  = g_GatewayData.soil_moisture;
        show_light = g_GatewayData.light;
        is_online  = g_GatewayData.sensor_online;

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
            /* 第1行: 温度 */
            OLED_ShowString(1, 1, "T:");
            OLED_ShowFloatNum(1, 4, show_temp, 2, 1);
            OLED_ShowString(1, 10, "C");

            /* 第2行: 湿度 */
            OLED_ShowString(2, 1, "H:");
            OLED_ShowFloatNum(2, 4, show_humi, 2, 1);
            OLED_ShowString(2, 10, "%");

            /* 第3行: 土壤 */
            OLED_ShowString(3, 1, "Soil:");
            OLED_ShowNum(3, 7, show_soil, 3);
            OLED_ShowString(3, 10, "%");

            /* 第4行: 光照 */
            OLED_ShowString(4, 1, "Lux:");
            OLED_ShowNum(4, 6, show_light, 5);
        }

        vTaskDelay(pdMS_TO_TICKS(OLED_REFRESH_PERIOD_MS));
    }
}
