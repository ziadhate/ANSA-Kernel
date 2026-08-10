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

typedef enum{
	init_failed=0,
	init_success,
	passed,
	failed,
	timed_out,
} queue_status;

queue_status kernel_queue_init(kernel_queue_t *q, void *buffer, uint8_t capacity, uint16_t item_size);
queue_status kernel_queue_push(kernel_queue_t *q, const void *data);
queue_status kernel_queue_pop(kernel_queue_t *q, void *data);
int task_queue_push_task(task_queue_t *q, TCB_t *task_tcb);
TCB_t *task_queue_pop_task(task_queue_t *q);
bool task_queue_empty(task_queue_t *q);
bool task_queue_remove(task_queue_t *q, TCB_t *task);
queue_status kernel_queue_send(kernel_queue_t *q, const void *item, uint32_t timeout_ticks) ;
queue_status kernel_queue_receive(kernel_queue_t *q, void *item, uint32_t timeout_ticks);
void kernel_tick_check_timeouts(void);

#endif
 
