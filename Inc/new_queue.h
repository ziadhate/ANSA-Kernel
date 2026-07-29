#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

typedef struct {
    void     **buffer;       /* array of pointers, provided by caller */
    uint16_t capacity;        /* max number of pointers it can hold */
    uint16_t head;            /* index to pop from next */
    uint16_t tail;            /* index to push into next */
    uint16_t count;           /* current number of stored pointers */
    uint8_t  overflow_flag;   /* set to 1 if a push was rejected due to full queue */
} PtrQueue_t;

void    kernel_queue_init(PtrQueue_t *q, void **buf, uint16_t capacity);
uint8_t kernel_queue_push(PtrQueue_t *q, void *item);
uint8_t kernel_queue_pop(PtrQueue_t *q, void **item);
uint8_t kernel_queue_is_empty(PtrQueue_t *q);
uint8_t kernel_queue_is_full(PtrQueue_t *q);

#endif