/**
 * @file    ring_buffer.h
 * @brief   轻量级环形缓冲区 (用于UART接收缓冲)
 */

#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include <stdint.h>
#include <string.h>

typedef struct {
    uint8_t *buffer;
    uint16_t size;
    volatile uint16_t head;
    volatile uint16_t tail;
} RingBuffer_t;

void     RingBuffer_Init(RingBuffer_t *rb, uint8_t *buf, uint16_t size);
uint16_t RingBuffer_Write(RingBuffer_t *rb, uint8_t data);
uint16_t RingBuffer_Read(RingBuffer_t *rb, uint8_t *data);
uint16_t RingBuffer_Available(RingBuffer_t *rb);
void     RingBuffer_Clear(RingBuffer_t *rb);

#endif /* __RING_BUFFER_H */
