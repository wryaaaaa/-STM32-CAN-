/**
 * @file    app_config.h
 * @brief   网关节点配置
 */

#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

/* ---- 系统时钟 ---- */
#define SYS_CLOCK_HZ            72000000UL

/* ---- CAN 配置 ---- */
#define CAN_BAUDRATE            500000UL
#define CAN_FILTER_ID           0x00000000UL
#define CAN_FILTER_MASK         0x00000000UL

/* ---- ESP8266 WiFi ---- */
#define WIFI_SSID               "SmartFarm"
#define WIFI_PASSWORD           "12345678"
#define WIFI_TCP_PORT           8080

/* ---- OLED 刷新周期 ---- */
#define OLED_REFRESH_PERIOD_MS  100UL

/* ---- 传感器离线检测 ---- */
#define SENSOR_OFFLINE_TIMEOUT  10000UL         /* 10秒无数据视为离线 */

/* ---- 任务优先级 ---- */
#define PRIO_CAN_RECV           4               /* 最高: CAN接收 */
#define PRIO_ESP_UPLOAD         3               /* WiFi上传 */
#define PRIO_OLED_DISP          2               /* OLED显示 */
#define PRIO_CAN_POLL           2               /* CAN轮询 */

/* ---- 任务栈大小 (字) ---- */
#define STK_CAN_RECV            256
#define STK_ESP_UPLOAD          512             /* 协议栈需要较大栈 */
#define STK_OLED_DISP           256
#define STK_CAN_POLL            256

/* ---- 队列深度 ---- */
#define QUEUE_CAN_RX_LEN        8               /* CAN接收队列 */
#define QUEUE_UPLOAD_LEN        4               /* 上传队列 */
#define QUEUE_UART_CMD_LEN      4               /* UART命令队列 */

/* ---- USART1 (ESP8266) 缓冲区 ---- */
#define USART1_RX_BUF_SIZE      512

#endif /* __APP_CONFIG_H */
