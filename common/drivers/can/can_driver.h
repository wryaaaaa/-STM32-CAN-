/**
 * @file    can_driver.h
 * @brief   CAN总线通用驱动接口
 * @note    基于STM32F103 + TJA1050，29-bit扩展帧
 */

#ifndef __CAN_DRIVER_H
#define __CAN_DRIVER_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/* ============================================================
 *                        协议常量定义
 * ============================================================ */

/* CAN ID 位域布局 (29-bit 扩展帧)
 * Bit28-26: 优先级  Bit25-16: 消息类型  Bit15-8: 源节点  Bit7-0: 目标节点
 */
#define CAN_PRI_MASK        0x1C000000U   /* Bit[28:26] */
#define CAN_TYPE_MASK       0x03FF0000U   /* Bit[25:16] */
#define CAN_SRC_MASK        0x0000FF00U   /* Bit[15:8]  */
#define CAN_DST_MASK        0x000000FFU   /* Bit[7:0]   */

#define CAN_PRI_SHIFT       26
#define CAN_TYPE_SHIFT      16
#define CAN_SRC_SHIFT       8

/* 优先级 (值越小优先级越高) */
#define CAN_PRI_EMERGENCY   0x0U  /* 最高: 告警帧 */
#define CAN_PRI_COMMAND     0x1U  /* 指令/应答 */
#define CAN_PRI_DATA        0x2U  /* 传感器数据 */
#define CAN_PRI_HEARTBEAT   0x3U  /* 最低: 心跳 */

/* 消息类型 */
#define CAN_TYPE_DATA_RPT   0x001U  /* 传感器数据上报 */
#define CAN_TYPE_QUERY_CMD  0x002U  /* 查询指令 */
#define CAN_TYPE_QUERY_ACK  0x003U  /* 查询应答 */
#define CAN_TYPE_HEARTBEAT  0x004U  /* 心跳帧 */
#define CAN_TYPE_ALERT      0x005U  /* 告警帧 */

/* 节点地址 */
#define CAN_ADDR_GATEWAY    0x00U
#define CAN_ADDR_SENSOR     0x01U
#define CAN_ADDR_BROADCAST  0xFFU

/* 查询类型 */
#define CMD_QUERY_ALL       0x01U
#define CMD_QUERY_TEMP      0x02U
#define CMD_QUERY_HUMI      0x03U

/* 设备状态 */
#define DEV_STATUS_OK       0x00U
#define DEV_STATUS_DHT11_ERR 0x01U

/* ============================================================
 *                        数据类型定义
 * ============================================================ */

/* CAN 帧结构 */
typedef struct {
    uint32_t ext_id;       /* 29-bit 扩展ID */
    uint8_t  data[8];      /* 数据载荷 (最大8字节) */
    uint8_t  dlc;          /* 数据长度 (0~8) */
} CAN_Frame_t;

/* 传感器数据结构 */
typedef struct {
    int16_t  temperature;   /* 温度 ×10 (如 255 = 25.5°C) */
    int16_t  humidity;      /* 湿度 ×10 (如 600 = 60.0%) */
    uint8_t  soil_moisture; /* 土壤湿度 0~100% */
    uint16_t light;         /* 光照强度 (lux) */
} SensorData_t;

/* ============================================================
 *                        API 函数声明
 * ============================================================ */

/**
 * @brief  初始化 CAN1 控制器
 * @param  baudrate   波特率 (如 500000 = 500kbps)
 * @param  filter_id  接收过滤器ID
 * @param  filter_mask 接收过滤器掩码 (0=不关心, 1=必须匹配)
 */
void CAN1_Init(uint32_t baudrate, uint32_t filter_id, uint32_t filter_mask);

/**
 * @brief  发送 CAN 帧 (阻塞式，带超时)
 * @param  frame      指向要发送的帧
 * @return 0=成功, 1=失败/超时
 */
uint8_t CAN_SendFrame(CAN_Frame_t *frame);

/**
 * @brief  绑定接收消息队列
 * @param  queue      用于接收 CAN 帧的 FreeRTOS 队列句柄
 * @note   CAN RX 中断将帧推入此队列
 */
void CAN_BindRxQueue(QueueHandle_t queue);

/**
 * @brief  构建 CAN ID (便捷宏)
 */
#define CAN_BUILD_ID(pri, type, src, dst) \
    (((uint32_t)(pri)  << CAN_PRI_SHIFT)  | \
     ((uint32_t)(type) << CAN_TYPE_SHIFT) | \
     ((uint32_t)(src)  << CAN_SRC_SHIFT)  | \
     ((uint32_t)(dst)))

#endif /* __CAN_DRIVER_H */
