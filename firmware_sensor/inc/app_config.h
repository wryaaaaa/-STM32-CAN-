/**
 * @file    app_config.h
 * @brief   传感器采集节点配置
 */

#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

/* ---- 系统时钟 ---- */
#define SYS_CLOCK_HZ            72000000UL

/* ---- CAN 配置 ---- */
#define CAN_BAUDRATE            500000UL
#define CAN_FILTER_ID           0x00000000UL   /* 接收网关发来的所有帧 */
#define CAN_FILTER_MASK         0x00000000UL   /* 不关心任何位 → 全收 */

/* ---- 传感器采集周期 ---- */
#define SENSOR_ACQ_PERIOD_MS   1000UL          /* 1秒采集一次 */

/* ---- CAN 上报周期 ---- */
#define CAN_REPORT_PERIOD_MS   1000UL          /* 1秒上报一次 */

/* ---- 心跳周期 ---- */
#define HEARTBEAT_PERIOD_MS    5000UL          /* 5秒发一次心跳 */

/* ---- DHT11 故障阈值 ---- */
#define DHT11_MAX_FAIL_COUNT   3               /* 连续失败3次触发告警 */

/* ---- 任务优先级 (0最低) ---- */
#define PRIO_SENSOR_ACQ        4               /* 最高: 传感器采集 (DHT11时序敏感) */
#define PRIO_CAN_REPORT        3               /* CAN上报 */
#define PRIO_CAN_CMD           2               /* CAN指令处理 */

/* ---- 任务栈大小 (字) ---- */
#define STK_CAN_REPORT         256
#define STK_SENSOR_ACQ         256
#define STK_CAN_CMD            256

/* ---- 队列深度 ---- */
#define QUEUE_SENSOR_DATA_LEN  4               /* 传感器数据队列 */
#define QUEUE_CAN_CMD_LEN      4               /* CAN命令队列 */

#endif /* __APP_CONFIG_H */
