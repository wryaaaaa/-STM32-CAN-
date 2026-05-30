/**
 * @file    can_driver.c
 * @brief   CAN总线驱动实现 (STM32F103 + TJA1050)
 */

#include "can_driver.h"

/* CAN 引脚定义: PB8=CAN_RX, PB9=CAN_TX (默认复用) */
#define CAN_GPIO_PORT   GPIOB
#define CAN_RX_PIN      GPIO_Pin_8
#define CAN_TX_PIN      GPIO_Pin_9
#define CAN_GPIO_CLK    RCC_APB2Periph_GPIOB
#define CAN_PERIPH_CLK  RCC_APB1Periph_CAN1
#define CAN_IRQ_CH      USB_LP_CAN1_RX0_IRQn

/* 内部变量: 接收队列句柄 (由应用层绑定) */
static QueueHandle_t s_CAN_RxQueue = NULL;

/* 内部变量: 超时计数 (防止发送死锁) */
#define CAN_TX_TIMEOUT  0xFFFFFUL

/* ============================================================
 *                      中断服务函数
 * ============================================================ */

/**
 * @brief  CAN1 RX0 中断处理 (FIFO0 接收到新帧)
 */
void CAN1_RX0_IRQHandler(void)
{
    CAN_Frame_t frame;
    CanRxMsg rx_msg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET)
    {
        CAN_Receive(CAN1, CAN_FIFO0, &rx_msg);

        /* 仅处理扩展帧 */
        if (rx_msg.IDE == CAN_Id_Extended)
        {
            frame.ext_id = rx_msg.ExtId;
            frame.dlc    = rx_msg.DLC;
            for (uint8_t i = 0; i < rx_msg.DLC && i < 8; i++)
            {
                frame.data[i] = rx_msg.Data[i];
            }

            /* 将帧推入应用层队列 (非阻塞 ISR 版本) */
            if (s_CAN_RxQueue != NULL)
            {
                xQueueSendToBackFromISR(s_CAN_RxQueue, &frame, &xHigherPriorityTaskWoken);
            }
        }

        CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* ============================================================
 *                      初始化
 * ============================================================ */

void CAN1_Init(uint32_t baudrate, uint32_t filter_id, uint32_t filter_mask)
{
    GPIO_InitTypeDef   gpio;
    CAN_InitTypeDef    can;
    CAN_FilterInitTypeDef can_filter;
    NVIC_InitTypeDef   nvic;

    /* ---- 1. 时钟 ---- */
    RCC_APB2PeriphClockCmd(CAN_GPIO_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(CAN_PERIPH_CLK, ENABLE);

    /* ---- 2. GPIO (PB8=RX, PB9=TX, 复用推挽) ---- */
    gpio.GPIO_Pin   = CAN_RX_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_IPU;           /* RX: 输入上拉 */
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(CAN_GPIO_PORT, &gpio);

    gpio.GPIO_Pin   = CAN_TX_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;         /* TX: 复用推挽 */
    GPIO_Init(CAN_GPIO_PORT, &gpio);

    /* ---- 3. CAN 控制器 ---- */
    CAN_DeInit(CAN1);
    CAN_StructInit(&can);
    can.CAN_TTCM   = DISABLE;                  /* 关闭时间触发 */
    can.CAN_ABOM   = ENABLE;                   /* 自动离线恢复 */
    can.CAN_AWUM   = ENABLE;                   /* 自动唤醒 */
    can.CAN_NART   = DISABLE;                  /* 自动重传 (使能以提高可靠性) */
    can.CAN_RFLM   = DISABLE;                  /* 锁存模式关闭 */
    can.CAN_TXFP   = DISABLE;                  /* 优先级由ID决定 */
    can.CAN_Mode   = CAN_Mode_Normal;
    can.CAN_SJW    = CAN_SJW_1tq;
    can.CAN_BS1    = CAN_BS1_3tq;
    can.CAN_BS2    = CAN_BS2_2tq;
    can.CAN_Prescaler = 12;                    /* 72MHz/(1+3+2)/12=500kbps */
    CAN_Init(CAN1, &can);

    /* ---- 4. 过滤器 (接收所有匹配 filter_id & filter_mask 的扩展帧) ---- */
    can_filter.CAN_FilterNumber      = 0;
    can_filter.CAN_FilterMode        = CAN_FilterMode_IdMask;
    can_filter.CAN_FilterScale       = CAN_FilterScale_32bit;
    can_filter.CAN_FilterIdHigh      = (uint16_t)(filter_id >> 13);
    can_filter.CAN_FilterIdLow       = (uint16_t)((filter_id << 3) | CAN_ID_EXT);
    can_filter.CAN_FilterMaskIdHigh  = (uint16_t)(filter_mask >> 13);
    can_filter.CAN_FilterMaskIdLow   = (uint16_t)((filter_mask << 3) | CAN_ID_EXT);
    can_filter.CAN_FilterFIFOAssignment = CAN_FIFO0;
    can_filter.CAN_FilterActivation  = ENABLE;
    CAN_FilterInit(&can_filter);

    /* ---- 5. NVIC 中断 (优先级在 FreeRTOS 管理范围内) ---- */
    nvic.NVIC_IRQChannel                   = CAN_IRQ_CH;
    nvic.NVIC_IRQChannelPreemptionPriority = 6;     /* 较高优先级，确保不丢帧 */
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);

    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);        /* FIFO0 非空中断 */
}

/* ============================================================
 *                      发送
 * ============================================================ */

uint8_t CAN_SendFrame(CAN_Frame_t *frame)
{
    CanTxMsg tx_msg;
    uint8_t  mailbox;
    uint32_t timeout = CAN_TX_TIMEOUT;

    /* 检查是否有空闲邮箱 */
    while (CAN_TransmitStatus(CAN1, 0) != CAN_TxStatus_Ok
        && CAN_TransmitStatus(CAN1, 1) != CAN_TxStatus_Ok
        && CAN_TransmitStatus(CAN1, 2) != CAN_TxStatus_Ok)
    {
        if (--timeout == 0) return 1;
    }

    /* 填充发送消息 */
    tx_msg.StdId = 0;
    tx_msg.ExtId = frame->ext_id;
    tx_msg.IDE   = CAN_Id_Extended;
    tx_msg.RTR   = CAN_RTR_Data;
    tx_msg.DLC   = frame->dlc;
    for (uint8_t i = 0; i < frame->dlc && i < 8; i++)
    {
        tx_msg.Data[i] = frame->data[i];
    }

    /* 将帧写入空闲邮箱并请求发送 */
    mailbox = CAN_Transmit(CAN1, &tx_msg);
    if (mailbox == CAN_TxStatus_NoMailBox)
    {
        return 1;                           /* 无空闲邮箱 */
    }

    return 0;                               /* 发送成功 */
}

/* ============================================================
 *                      队列绑定
 * ============================================================ */

void CAN_BindRxQueue(QueueHandle_t queue)
{
    s_CAN_RxQueue = queue;
}
