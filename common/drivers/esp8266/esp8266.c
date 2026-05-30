#include "esp8266.h"
#include "usart.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
  * @brief  发送AT指令并等待回复
  */
extern volatile uint8_t usart1_busy;

uint8_t ESP_SendCmd(char* cmd, char* ack, uint32_t timeout_ms)
{
    usart1_busy = 1;
    USART1_ClearBuffer();
    USART_SendString(USART1, cmd);

    uint32_t waited = 0;
    while (waited < timeout_ms)
    {
        if (strstr(USART1_RxBuffer, ack) != NULL)
        {
            usart1_busy = 0;
            return 1;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
        waited += 10;
    }
    usart1_busy = 0;
    return 0;
}

/**
  * @brief  发送TCP数据 (智能版 - 防止误删断开信号)
  * @return 1:发送成功, 0:发送失败
  */
uint8_t ESP_SendData(uint8_t client_id, char* data, uint16_t data_len)
{
    // ---------------------------------------------------------
    // 关键修复1：在发数据清空缓冲区前，先检查是否已经断开！
    // ---------------------------------------------------------
    if (strstr(USART1_RxBuffer, "CLOSED") != NULL)
    {
        // 发现断开信号，千万别清空缓冲区！
        // 直接返回失败，让 Task_ESP8266_Conn 去处理断开逻辑
        return 0; 
    }

    char cmd_buffer[64]; 
    sprintf(cmd_buffer, "AT+CIPSEND=%d,%d\r\n", client_id, data_len);
    
    // 发送 AT+CIPSEND
    USART1_ClearBuffer(); // 此时确认没有CLOSED，可以清空
    USART_SendString(USART1, cmd_buffer);
    
    // 等待 ">"
    uint32_t waited = 0;
    uint8_t ready_to_send = 0;
    while(waited < 1000)
    {
        if(strstr(USART1_RxBuffer, ">") != NULL) {
            ready_to_send = 1;
            break;
        }
        // 关键修复2：如果收到 ERROR 或 CLOSED，立即停止
        if(strstr(USART1_RxBuffer, "ERROR") != NULL || strstr(USART1_RxBuffer, "CLOSED") != NULL) {
            return 0;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        waited += 10;
    }
    
    if(!ready_to_send) return 0;
    
    // 发送实际数据
    USART_SendArray(USART1, (uint8_t*)data, data_len);
    USART_SendString(USART1, "\r\n");
    
    // 等待 "SEND OK"
    waited = 0;
    while (waited < 2000)
    {
        if (strstr(USART1_RxBuffer, "SEND OK") != NULL)
        {
            USART1_ClearBuffer(); 
            return 1;
        }
        // 关键修复3：发送过程中发现断开或错误，立即退出，不要死等
        if (strstr(USART1_RxBuffer, "CLOSED") != NULL || strstr(USART1_RxBuffer, "ERROR") != NULL)
        {
            return 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(10)); 
        waited += 10;
    }
    
    USART1_ClearBuffer();
    return 0;
}

/**
  * @brief  等待连接
  */
uint8_t ESP8266_WaitConnect(void)
{
    USART1_ClearBuffer();
    while (1)
    {
        if (strstr(USART1_RxBuffer, "CONNECT") != NULL)
        {
            char* connect_pos = strstr(USART1_RxBuffer, "CONNECT");
            if (connect_pos != NULL && connect_pos > USART1_RxBuffer)
            {
                char* comma_pos = connect_pos - 1;
                while (comma_pos > USART1_RxBuffer && *comma_pos != ',')
                    comma_pos--;
                
                if (comma_pos > USART1_RxBuffer)
                    return atoi(comma_pos + 1);
            }
            // 如果解析失败但确实连接了，返回默认ID 0
            return 0;
        }
        
        if (USART1_RxCount >= USART1_RX_MAX_LEN - 5)
            USART1_ClearBuffer();
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
  * @brief  初始化 ESP8266
  */
uint8_t ESP8266TCP_Init(void)
{
    if (ESP_SendCmd("AT\r\n", "OK", 1000) != 1) return 0;
    USART_SendString(USART1, "AT+RST\r\n");
    vTaskDelay(pdMS_TO_TICKS(1000)); 
    USART1_ClearBuffer();
    if (ESP_SendCmd("AT+CWMODE=2\r\n", "OK", 1000) != 1) return 0;
    if (ESP_SendCmd("AT+CWSAP=\"SmartFarm\",\"12345678\",1,4\r\n", "OK", 2000) != 1) return 0;
    if (ESP_SendCmd("AT+CIPMUX=1\r\n", "OK", 1000) != 1) return 0;
    if (ESP_SendCmd("AT+CIPSERVER=1,8080\r\n", "OK", 1000) != 1) return 0;
    return 1;
}
