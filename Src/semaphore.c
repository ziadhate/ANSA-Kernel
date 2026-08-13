#include "../Inc/semaphore.h"
#include "../Inc/scheduler.h"
#include "../Port/Cortex_M4/Inc/port.h"

#define SEM_INVALID_COUNT 0xFFFFFFFFU

queue_status_t kernel_sem_init(kernel_sem_t *sem, uint32_t initial_count, uint32_t max_count)
{
    if ((sem == NULL) || (max_count == 0U) || (initial_count > max_count))
    {
        return QUEUE_INIT_FAILED;
    }

    port_critical_enter();

    sem->count     = initial_count;
    sem->max_count = max_count;
    sem->waiters.head = NULL;
    sem->waiters.tail = NULL;

    port_critical_exit();
    return QUEUE_INIT_SUCCESS;
}

queue_status_t kernel_sem_set_max_count(kernel_sem_t *sem, uint32_t new_max_count)
{
    if ((sem == NULL) || (new_max_count == 0U))
    {
        return QUEUE_FAILED;
    }

    port_critical_enter();

    if (new_max_count < sem->count)
    {
        port_critical_exit();
        return QUEUE_FAILED;
    }

    sem->max_count = new_max_count;

    port_critical_exit();
    return QUEUE_PASSED;
}

queue_status_t kernel_sem_take(kernel_sem_t *sem, uint32_t timeout_ticks)
{
    if (sem == NULL)
    {
        return QUEUE_FAILED;
    }

    port_critical_enter();

    // Fast path: Token available immediately
    if (sem->count > 0U)
    {
        sem->count--;
        port_critical_exit();
        return QUEUE_PASSED;
    }

    // Non-blocking caller and no token available
    if (timeout_ticks == 0U)
    {
        port_critical_exit();
        return QUEUE_FAILED;
    }

    if (current_task == NULL)
    {
        port_critical_exit();
        return QUEUE_FAILED;
    }

    // Block current task
    current_task->timed_out = 0U;
    current_task->tick_counter = kernel_get_tick_count() + timeout_ticks;
    current_task->current_wait_queue = &sem->waiters;
    current_task->state = TASK_STATE_BLOCKED;
    
    task_queue_push_task(&sem->waiters, current_task);

    port_critical_exit();

    // Context switch to next ready task
    kernel_yield();

    // Resumed execution after yield
    port_critical_enter();

    // Always ensure the task is removed from the wait queue to prevent race conditions
    task_queue_remove(current_task->current_wait_queue, current_task);
    current_task->current_wait_queue = NULL;

    if (current_task->timed_out)
    {
        port_critical_exit();
        return QUEUE_TIMED_OUT;
    }

    // Token was successfully handed off directly via kernel_sem_give()
    port_critical_exit();
    return QUEUE_PASSED;
}

queue_status_t kernel_sem_give(kernel_sem_t *sem)
{
    if (sem == NULL)
    {
        return QUEUE_FAILED;
    }

    port_critical_enter();

    if (!task_queue_empty(&sem->waiters))
    {
        // Direct handoff: Transfer token directly to woken task without incrementing sem->count
        TCB_t *woken = task_queue_pop_task(&sem->waiters);
        woken->current_wait_queue = NULL;
        woken->state = TASK_STATE_READY;
        
        port_trigger_context_switch();
    }
    else
    {
        if (sem->count >= sem->max_count)
        {
            port_critical_exit();
            return QUEUE_FAILED;
        }
        sem->count++;
    }

    port_critical_exit();
    return QUEUE_PASSED;
}

uint32_t kernel_sem_get_count(const kernel_sem_t *sem)
{
    if (sem == NULL)
    {
        return SEM_INVALID_COUNT;
    }

    port_critical_enter();
    uint32_t count = sem->count;
    port_critical_exit();

    return count;
}
