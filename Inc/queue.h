#ifndef QUEUE_H
#define QUEUE_H
 
#include <stdint.h>
 
typedef struct {
	uint8_t *buffer;
	uint16_t capacity;
	uint16_t head;
	uint16_t tail;
	uint16_t count;
	uint8_t  overflow_flag;
} RingQueue_t;
 
void    kernel_queue_init(RingQueue_t *q, uint8_t *buf, uint16_t cap);
uint8_t kernel_queue_push(RingQueue_t *q, uint8_t data);
uint8_t kernel_queue_pop(RingQueue_t *q, uint8_t *data);
 
#endif
 
