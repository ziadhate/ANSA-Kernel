#include "../Inc/semaphore.h"
#include "../Inc/scheduler.h"

queue_status kernel_sem_init(kernel_sem_t *sem, uint32_t initial_count, uint32_t max_count)
{
    if ((sem == NULL) || (max_count == 0U) || (initial_count > max_count))
    {
        return init_failed;
    }
 
    CRITICAL_SECTION_ENTER();

    //Rejects invalid situations
    sem->count     = initial_count;
    sem->max_count = max_count;
 
    //initialize waiting queue
    sem->waiters.head = NULL; 
    sem->waiters.tail = NULL;
 
    CRITICAL_SECTION_EXIT();
    return init_success;
}

queue_status kernel_sem_take(kernel_sem_t *sem, uint32_t timeout_ticks)
{
    queue_status status;
 
    if (sem == NULL)
    {
        return failed;
    }
 
    CRITICAL_SECTION_ENTER();
 
    if (sem->count > 0U)
    {
        // Fast path: a token is already available. 
        sem->count--;
        CRITICAL_SECTION_EXIT();
        return passed;
    }
 
    if (timeout_ticks == 0U)
    {
        // No token and caller doesn't want to wait.
        CRITICAL_SECTION_EXIT();
        return failed;
    }
 
    // No token available return time out
    current_task->timed_out = 0U;
    current_task->tick_counter = kernel_get_tick_count() + timeout_ticks;
    current_task->current_wait_queue = &sem->waiters;
    current_task->state = TASK_STATE_BLOCKED;
    task_queue_push_task(&sem->waiters, current_task);
 
    CRITICAL_SECTION_EXIT();
 
    kernel_yield();
 
    CRITICAL_SECTION_ENTER();
 
    if (current_task->timed_out)
    {
        CRITICAL_SECTION_EXIT();
        return timed_out;
    }
 
    if (sem->count > 0U)
    {
        sem->count--;
        status = passed;
    }
    else
    {
        //Should not happen under normal operation but safety precautions
        status = failed;
    }
 
    CRITICAL_SECTION_EXIT();
    return status;
}

queue_status kernel_sem_give(kernel_sem_t *sem)
{
    if (sem == NULL)
    {
        return failed;
    }
 
    CRITICAL_SECTION_ENTER();
 
    if (sem->count >= sem->max_count)
    {
        
        CRITICAL_SECTION_EXIT();
        return failed;
    }
 
    sem->count++;
 
    if (!task_queue_empty(&sem->waiters))
    {
        TCB_t *woken = task_queue_pop_task(&sem->waiters);
        woken->current_wait_queue = NULL;
        woken->state = TASK_STATE_READY;
    }
 
    CRITICAL_SECTION_EXIT();
    return passed;
}

