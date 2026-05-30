/**
 * @file    main.c
 * @brief   网关节点入口
 * @note    功能: CAN接收传感器数据 → OLED显示 → ESP8266 TCP上传
 * @hardware STM32F103C8 + TJA1050 + ESP8266 + OLED(SSD1306)
 */

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "app_config.h"
#include "usart.h"
#include "task_can_recv.h"
#include "task_esp_upload.h"
#include "task_oled_disp.h"
#include "task_can_poll.h"

/* 启动任务: 创建所有任务后自删除 */
static void Start_Task(void *pvParameters)
{
    /* 创建互斥锁 */
    xGatewayMutex = xSemaphoreCreateMutex();

    /* 创建队列 */
    xQueueCANRx   = xQueueCreate(QUEUE_CAN_RX_LEN,  sizeof(CAN_Frame_t));
    xQueueUpload  = xQueueCreate(QUEUE_UPLOAD_LEN,  sizeof(SensorData_t));
    xQueueUARTCmd = xQueueCreate(QUEUE_UART_CMD_LEN, 64);

    if (xGatewayMutex != NULL
        && xQueueCANRx != NULL && xQueueUpload != NULL && xQueueUARTCmd != NULL)
    {
        taskENTER_CRITICAL();

        /* 创建 CAN 接收任务 (最高优先级) */
        xTaskCreate((TaskFunction_t)Task_CAN_Recv,
                    "CAN_Rcv", STK_CAN_RECV, NULL,
                    PRIO_CAN_RECV, NULL);

        /* 创建 ESP8266 上传任务 */
        xTaskCreate((TaskFunction_t)Task_ESP_Upload,
                    "ESP_Up", STK_ESP_UPLOAD, NULL,
                    PRIO_ESP_UPLOAD, NULL);

        /* 创建 OLED 显示任务 */
        xTaskCreate((TaskFunction_t)Task_OLED_Display,
                    "OLED", STK_OLED_DISP, NULL,
                    PRIO_OLED_DISP, NULL);

        /* 创建 CAN 轮询任务 */
        xTaskCreate((TaskFunction_t)Task_CAN_Poll,
                    "CAN_Poll", STK_CAN_POLL, NULL,
                    PRIO_CAN_POLL, NULL);

        taskEXIT_CRITICAL();
    }

    vTaskDelete(NULL);
}

/**
 * @brief  main 入口
 */
int main(void)
{
    /* NVIC 优先级分组: 4位抢占优先级 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* 初始化 USART1 (ESP8266, 115200) */
    USART1_Init(115200);

    /* 创建启动任务 */
    xTaskCreate((TaskFunction_t)Start_Task,
                "Start", 128, NULL, 1, NULL);

    /* 启动 FreeRTOS 调度器 */
    vTaskStartScheduler();

    while (1);
}
