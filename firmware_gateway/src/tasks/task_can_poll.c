/**
 * @file    task_can_poll.c
 * @brief   CAN 主动轮询任务实现
 *          - 定期查询传感器状态
 *          - 传感器离线时间过长时主动查询
 */

#include "task_can_poll.h"
#include "task_can_recv.h"
#include "can_driver.h"
#include "can_protocol.h"
#include "app_config.h"

void Task_CAN_Poll(void *pvParameters)
{
    CAN_Frame_t query_frame;

    /* 等待一段时间再开始轮询 (给系统初始化留时间) */
    vTaskDelay(pdMS_TO_TICKS(5000));

    while (1)
    {
        /* 如果传感器离线，主动查询 */
        uint8_t offline = 0;
        xSemaphoreTake(xGatewayMutex, portMAX_DELAY);
        offline = (g_GatewayData.sensor_online == 0);
        xSemaphoreGive(xGatewayMutex);

        if (offline)
        {
            can_build_query_cmd(&query_frame, CMD_QUERY_ALL);
            CAN_SendFrame(&query_frame);
        }

        /* 每 3 秒检查一次 */
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
