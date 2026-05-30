/**
 * @file    json_simple.h
 * @brief   极简JSON构建器 (无动态内存分配，适用于嵌入式)
 */

#ifndef __JSON_SIMPLE_H
#define __JSON_SIMPLE_H

#include <stdio.h>
#include <string.h>

/**
 * @brief  构建传感器数据 JSON 字符串
 * @param  buf      输出缓冲区
 * @param  buf_size 缓冲区大小
 * @param  temp     温度 ×10
 * @param  humi     湿度 ×10
 * @param  status   设备状态字符串
 * @param  ts       时间戳 (毫秒)
 * @return 写入的字节数
 */
static inline int json_build_data(char *buf, int buf_size,
                                   int16_t temp, int16_t humi,
                                   const char *status, uint32_t ts)
{
    return snprintf(buf, buf_size,
        "{\"type\":\"data\",\"temp\":%.1f,\"humi\":%.1f,\"status\":\"%s\",\"ts\":%lu}",
        temp / 10.0f, humi / 10.0f, status, (unsigned long)ts);
}

/**
 * @brief  构建心跳 JSON 字符串
 */
static inline int json_build_heartbeat(char *buf, int buf_size, const char *status)
{
    return snprintf(buf, buf_size,
        "{\"type\":\"heartbeat\",\"status\":\"%s\"}", status);
}

/**
 * @brief  构建告警 JSON 字符串
 */
static inline int json_build_alert(char *buf, int buf_size, const char *msg)
{
    return snprintf(buf, buf_size,
        "{\"type\":\"alert\",\"msg\":\"%s\"}", msg);
}

/**
 * @brief  构建应答 JSON 字符串
 */
static inline int json_build_ack(char *buf, int buf_size, int code)
{
    return snprintf(buf, buf_size,
        "{\"type\":\"ack\",\"code\":%d}", code);
}

/**
 * @brief  解析收到的 JSON 命令
 * @param  json_str 输入 JSON 字符串
 * @param  cmd_buf  输出命令字符串 (至少16字节)
 * @return 0=成功, 1=解析失败
 */
static inline int json_parse_cmd(const char *json_str, char *cmd_buf, int cmd_buf_size)
{
    /* 简单字符串匹配解析: {"cmd":"query"} → "query" */
    const char *cmd_start = strstr(json_str, "\"cmd\"");
    if (cmd_start == NULL) return 1;

    const char *val_start = strstr(cmd_start + 5, "\"");
    if (val_start == NULL) return 1;
    val_start++; /* 跳过引号 */

    const char *val_end = strstr(val_start, "\"");
    if (val_end == NULL) return 1;

    int len = (int)(val_end - val_start);
    if (len >= cmd_buf_size) len = cmd_buf_size - 1;
    memcpy(cmd_buf, val_start, len);
    cmd_buf[len] = '\0';

    return 0;
}

#endif /* __JSON_SIMPLE_H */
