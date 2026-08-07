#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include <stdint.h>

typedef struct{
  uint32_t count;
  uint32_t max_count;    //max number of tokens
  uint32_t waiters;      //blocked state tasks or tasks waiting
} kernel_sem_t;

queue_status kernel_sem_init(kernel_sem_t &sem, uint32_t inital_count,uint32_t max_count);
queue_status kernel_sem_take(kernel_sem_t *sem, uint32_t timeout_ticks);
queue_status kernel_sem_give(kernel_sem_t *sem);
uint32_t kernel_sem_get_count(const kernel_sem_t *sem);

#endif
