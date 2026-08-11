#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include <stdint.h>
#include "queue.h"

typedef struct{
  uint32_t count;
  uint32_t max_count;    //max number of tokens
  task_queue_t waiters;      //blocked state tasks or tasks waiting
} kernel_sem_t;

queue_status_t kernel_sem_init(kernel_sem_t *sem, uint32_t initial_count,uint32_t max_count);
queue_status_t kernel_sem_take(kernel_sem_t *sem, uint32_t timeout_ticks);
queue_status_t kernel_sem_give(kernel_sem_t *sem);
uint32_t kernel_sem_get_count(const kernel_sem_t *sem);
queue_status kernel_sem_set_max_count(kernel_sem_t *sem, uint32_t new_max_count);

#endif
