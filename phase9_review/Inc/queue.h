#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include "task.h"

#ifndef KERNEL_MAX_DELAY
#define KERNEL_MAX_DELAY 0xFFFFFFFFU
#endif

typedef enum {
    QUEUE_PASSED = 0,
    QUEUE_FAILED,
    QUEUE_BUSY,
    QUEUE_TIMED_OUT
} queue_status_t;

typedef struct {
    uint8_t *buffer;
    uint32_t item_size;
    uint32_t max_items;
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    task_queue_t wait_send;
    task_queue_t wait_receive;
} kernel_queue_t;

queue_status_t kernel_queue_init(kernel_queue_t *q, uint8_t *buffer, uint32_t item_size, uint32_t max_items);
queue_status_t kernel_queue_send(kernel_queue_t *q, const void *item, uint32_t timeout_ticks);
queue_status_t kernel_queue_receive(kernel_queue_t *q, void *item, uint32_t timeout_ticks);
queue_status_t kernel_queue_send_from_isr(kernel_queue_t *q, const void *item, bool *pxHigherPriorityTaskWoken);
queue_status_t kernel_queue_receive_from_isr(kernel_queue_t *q, void *item, bool *pxHigherPriorityTaskWoken);
bool task_queue_empty(const task_queue_t *q);

#endif /* QUEUE_H */
