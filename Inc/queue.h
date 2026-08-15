#ifndef QUEUE_H
#define QUEUE_H
#include "task.h"
 
#include <stdint.h>
#include <stdbool.h>
 
struct task_queue_t{
    TCB_t *head;
    TCB_t *tail;
};

typedef struct {
    uint8_t *buffer;
    uint8_t capacity;
    uint16_t item_size;
    uint8_t head;
    uint8_t tail;
    uint8_t count;
    uint8_t overflow_flag;

    task_queue_t senders_waiting;
    task_queue_t receivers_waiting;
} kernel_queue_t;

typedef enum
{
    QUEUE_INIT_FAILED = 0,
    QUEUE_INIT_SUCCESS,
    QUEUE_PASSED,
    QUEUE_FAILED,
    QUEUE_TIMED_OUT
} queue_status_t;

queue_status_t kernel_queue_init(kernel_queue_t *q, void *buffer, uint8_t capacity, uint16_t item_size);
queue_status_t kernel_queue_push(kernel_queue_t *q, const void *data);
queue_status_t kernel_queue_pop(kernel_queue_t *q, void *data);
void task_queue_push_task(task_queue_t *q, TCB_t *task_tcb);
TCB_t *task_queue_pop_task(task_queue_t *q);
bool task_queue_empty(task_queue_t *q);
bool task_queue_remove(task_queue_t *q, TCB_t *task);
queue_status_t kernel_queue_send(kernel_queue_t *q, const void *item, uint32_t timeout_ticks) ;
queue_status_t kernel_queue_receive(kernel_queue_t *q, void *item, uint32_t timeout_ticks);
queue_status_t kernel_queue_send_from_isr(kernel_queue_t *q, const void *item);
queue_status_t kernel_queue_receive_from_isr(kernel_queue_t *q, void *item);

#endif
 
