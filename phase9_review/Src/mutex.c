#include "mutex.h"
#include "scheduler.h"
#include "port.h"
#include <stddef.h>

mutex_status_t kernel_mutex_init(kernel_mutex_t *mutex)
{
    if (mutex == NULL) {
        return MUTEX_FAILED;
    }
    mutex->locked = 0;
    mutex->owner = NULL;
    mutex->original_priority = 0;
    mutex->wait_list.head = NULL;
    mutex->wait_list.tail = NULL;
    return MUTEX_PASSED;
}

mutex_status_t kernel_mutex_lock(kernel_mutex_t *mutex, uint32_t timeout_ticks)
{
    if (mutex == NULL) {
        return MUTEX_FAILED;
    }

    port_enter_critical();

    if (mutex->locked == 0) {
        mutex->locked = 1;
        mutex->owner = current_task;
        mutex->original_priority = current_task->priority;
        port_exit_critical();
        return MUTEX_PASSED;
    }

    /* Priority Inheritance Protocol */
    if (current_task->priority < mutex->owner->priority) {
        mutex->owner->priority = current_task->priority;
    }

    if (timeout_ticks == 0U) {
        port_exit_critical();
        return MUTEX_FAILED;
    }

    current_task->state = TASK_BLOCKED;
    current_task->current_wait_queue = &mutex->wait_list;

    if (timeout_ticks != KERNEL_MAX_DELAY) {
        current_task->delay_ticks = timeout_ticks;
    }

    task_queue_push_task(&mutex->wait_list, current_task);
    port_exit_critical();

    kernel_yield();

    if (current_task->state == TASK_BLOCKED) {
        return MUTEX_TIMED_OUT;
    }

    return MUTEX_PASSED;
}

mutex_status_t kernel_mutex_unlock(kernel_mutex_t *mutex)
{
    if (mutex == NULL) {
        return MUTEX_FAILED;
    }

    port_enter_critical();

    if (mutex->owner != current_task) {
        port_exit_critical();
        return MUTEX_FAILED;
    }

    /* Restore Priority */
    current_task->priority = mutex->original_priority;

    if (!task_queue_empty(&mutex->wait_list)) {
        TCB_t *next_owner = task_queue_pop_task(&mutex->wait_list);
        mutex->owner = next_owner;
        mutex->original_priority = next_owner->priority;
        next_owner->state = TASK_READY;
        port_exit_critical();
        kernel_yield();
        return MUTEX_PASSED;
    }

    mutex->locked = 0;
    mutex->owner = NULL;
    port_exit_critical();
    return MUTEX_PASSED;
}
