/**
 * @file    ring_buffer.c
 * @brief   轻量级环形缓冲区实现
 */

#include "ring_buffer.h"

void RingBuffer_Init(RingBuffer_t *rb, uint8_t *buf, uint16_t size)
{
    rb->buffer = buf;
    rb->size   = size;
    rb->head   = 0;
    rb->tail   = 0;
}

uint16_t RingBuffer_Write(RingBuffer_t *rb, uint8_t data)
{
    uint16_t next = (rb->head + 1) % rb->size;
    if (next == rb->tail) return 0;     /* 满 */
    rb->buffer[rb->head] = data;
    rb->head = next;
    return 1;
}

uint16_t RingBuffer_Read(RingBuffer_t *rb, uint8_t *data)
{
    if (rb->head == rb->tail) return 0; /* 空 */
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;
    return 1;
}

uint16_t RingBuffer_Available(RingBuffer_t *rb)
{
    return (rb->head - rb->tail + rb->size) % rb->size;
}

void RingBuffer_Clear(RingBuffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}
