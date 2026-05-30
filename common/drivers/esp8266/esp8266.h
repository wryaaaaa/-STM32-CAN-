/**
 * @file    esp8266.h
 * @brief   ESP8266 WiFi 模块驱动 (AT指令, AP模式, TCP Server)
 */

#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f10x.h"
#include "usart.h"

/* ---- API ---- */
uint8_t ESP8266TCP_Init(void);
uint8_t ESP8266_WaitConnect(void);
uint8_t ESP_SendCmd(char *cmd, char *ack, uint32_t timeout_ms);
uint8_t ESP_SendData(uint8_t client_id, char *data, uint16_t data_len);

#endif /* __ESP8266_H */
