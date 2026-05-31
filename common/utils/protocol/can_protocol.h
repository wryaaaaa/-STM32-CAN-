/**
 * @file    can_protocol.h
 * @brief   CAN协议编解码工具
 * @note    CAN帧 8字节布局:
 *         [0-1]=温度×10  [2-3]=湿度×10  [4]=土壤湿度%  [5-6]=光照lux  [7]=保留
 */

#ifndef __CAN_PROTOCOL_H
#define __CAN_PROTOCOL_H

#include "can_driver.h"

/* ---- 编码: 构建各类 CAN 帧 ---- */

/* 构建传感器数据上报帧 */
static inline void can_build_data_report(CAN_Frame_t *frame, SensorData_t *data)
{
    frame->ext_id = CAN_BUILD_ID(CAN_PRI_DATA, CAN_TYPE_DATA_RPT,
                                  CAN_ADDR_SENSOR, CAN_ADDR_GATEWAY);
    frame->dlc = 8;
    frame->data[0] = (uint8_t)(data->temperature >> 8);
    frame->data[1] = (uint8_t)(data->temperature);
    frame->data[2] = (uint8_t)(data->humidity >> 8);
    frame->data[3] = (uint8_t)(data->humidity);
    frame->data[4] = data->soil_moisture;
    frame->data[5] = (uint8_t)(data->light >> 8);      /* 光照高字节 */
    frame->data[6] = (uint8_t)(data->light);            /* 光照低字节 */
    frame->data[7] = 0;                                 /* 保留 */
}

/* 构建查询指令帧 */
static inline void can_build_query_cmd(CAN_Frame_t *frame, uint8_t query_type)
{
    frame->ext_id = CAN_BUILD_ID(CAN_PRI_COMMAND, CAN_TYPE_QUERY_CMD,
                                  CAN_ADDR_GATEWAY, CAN_ADDR_SENSOR);
    frame->dlc = 1;
    frame->data[0] = query_type;
}

/* 构建查询应答帧 */
static inline void can_build_query_ack(CAN_Frame_t *frame, SensorData_t *data)
{
    can_build_data_report(frame, data);
    frame->ext_id = CAN_BUILD_ID(CAN_PRI_COMMAND, CAN_TYPE_QUERY_ACK,
                                  CAN_ADDR_SENSOR, CAN_ADDR_GATEWAY);
}

/* 构建心跳帧 */
static inline void can_build_heartbeat(CAN_Frame_t *frame, uint8_t status)
{
    frame->ext_id = CAN_BUILD_ID(CAN_PRI_HEARTBEAT, CAN_TYPE_HEARTBEAT,
                                  CAN_ADDR_SENSOR, CAN_ADDR_GATEWAY);
    frame->dlc = 1;
    frame->data[0] = status;
}

/* 构建告警帧 */
static inline void can_build_alert(CAN_Frame_t *frame, uint8_t alert_code)
{
    frame->ext_id = CAN_BUILD_ID(CAN_PRI_EMERGENCY, CAN_TYPE_ALERT,
                                  CAN_ADDR_SENSOR, CAN_ADDR_GATEWAY);
    frame->dlc = 1;
    frame->data[0] = alert_code;
}

/* ---- 解码: 从 CAN 帧提取数据 ---- */

/* 解析传感器数据帧 */
static inline void can_parse_sensor_data(CAN_Frame_t *frame, SensorData_t *data)
{
    data->temperature   = (int16_t)((frame->data[0] << 8) | frame->data[1]);
    data->humidity      = (int16_t)((frame->data[2] << 8) | frame->data[3]);
    data->soil_moisture = frame->data[4];
    data->light         = (uint16_t)((frame->data[5] << 8) | frame->data[6]);
}

/* 提取消息类型字段 */
static inline uint32_t can_get_msg_type(uint32_t ext_id)
{
    return (ext_id & CAN_TYPE_MASK) >> CAN_TYPE_SHIFT;
}

/* 提取源节点 */
static inline uint32_t can_get_src(uint32_t ext_id)
{
    return (ext_id & CAN_SRC_MASK) >> CAN_SRC_SHIFT;
}

#endif /* __CAN_PROTOCOL_H */
