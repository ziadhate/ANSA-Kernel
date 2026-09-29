#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>
#include <stdbool.h>
#include "task.h"

#define KERNEL_MAX_DELAY 0xFFFFFFFFU

typedef enum {
    MUTEX_PASSED = 0,
    MUTEX_FAILED,
    MUTEX_TIMED_OUT,
    MUTEX_TIMEOUT = MUTEX_TIMED_OUT
} mutex_status_t;

typedef struct kernel_mutex_t {
    uint8_t locked;
    TCB_t *owner;
    uint8_t original_priority;
    task_queue_t wait_list;
} kernel_mutex_t;

mutex_status_t kernel_mutex_init(kernel_mutex_t *mutex);
mutex_status_t kernel_mutex_lock(kernel_mutex_t *mutex, uint32_t timeout_ticks);
mutex_status_t kernel_mutex_unlock(kernel_mutex_t *mutex);

#endif /* MUTEX_H */
