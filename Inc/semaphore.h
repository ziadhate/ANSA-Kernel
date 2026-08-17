#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include <stdint.h>
#include "queue.h"

/* Kernel semaphore
  A semaphore contains a counter representing the number of available tokens 
  and a queue containing tasks waiting for a token
*/
typedef struct
{
    uint32_t count;           // Current number of available tokens
    uint32_t max_count;       // Maximum number of tokens allowed

    task_queue_t waiters; /* Queue of tasks currently blocked while waiting for a semaphore token */
} kernel_sem_t;

/*
  Initializes a semaphore.
  initial_count : Initial number of available tokens
  max_count : Maximum number of tokens the semaphore can contain
  
  The initial count must not exceed max_count.
*/
queue_status_t kernel_sem_init(kernel_sem_t *sem, uint32_t initial_count, uint32_t max_count);

/*
  Attempts to acquire one semaphore token
  If a token is available, the task takes it immediately.
  Otherwise, the task is blocked until a token becomes available or the specified timeout expires.

  timeout_ticks : Maximum number of ticks to wait
*/
queue_status_t kernel_sem_take(kernel_sem_t *sem, uint32_t timeout_ticks);

/*
  Releases one semaphore token
  If tasks are waiting for the semaphore, one waiting task may be unblocked instead of increasing the token count.
*/
queue_status_t kernel_sem_give(kernel_sem_t *sem);

/* Returns the current number of available semaphore tokens. Does not modify the semaphore state */
uint32_t kernel_sem_get_count(const kernel_sem_t *sem);


/*
  Changes the maximum number of tokens allowed by the semaphore.
  new_max_count : New maximum token count
  The new maximum must be compatible with the current semaphore state.
*/
queue_status_t kernel_sem_set_max_count(kernel_sem_t *sem, uint32_t new_max_count);

/*
  ISR-safe version of kernel_sem_give().
  Releases one semaphore token from interrupt context.
  px_higher_priority_task_woken is set to true if giving the
  semaphore unblocks a task with a higher priority than the 
  currently running task, allowing the ISR exit code to request a context switch.
*/
queue_status_t kernel_sem_give_from_isr(kernel_sem_t *sem, bool *px_higher_priority_task_woken);

/*
  ISR-safe version of kernel_sem_take().
  
  Attempts to acquire one semaphore token from interrupt context.
  Unlike the task-context version, this operation must not block the current execution context or wait for a timeout.
*/
queue_status_t kernel_sem_take_from_isr(kernel_sem_t *sem);

/* ISR-safe version of kernel_sem_get_count().
  Returns the current number of available tokens without modifying the semaphore.
*/
uint32_t kernel_sem_get_count_from_isr(const kernel_sem_t *sem);

#endif
