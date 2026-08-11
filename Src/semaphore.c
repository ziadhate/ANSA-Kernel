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

    //Rejects invalid situations
    sem->count     = initial_count;
    sem->max_count = max_count;
 
    //initialize waiting queue
    sem->waiters.head = NULL; 
    sem->waiters.tail = NULL;
 
    port_critical_exit();
    return QUEUE_INIT_SUCCESS;
}

queue_status kernel_sem_set_max_count(kernel_sem_t *sem, uint32_t new_max_count)
{
    if (sem == NULL || new_max_count == 0U)
    {
        return failed;
    }

    CRITICAL_SECTION_ENTER();

    // Prevent reducing max_count below current token count (or adjust logic as per your RTOS requirements)
    if (new_max_count < sem->count)
    {
        CRITICAL_SECTION_EXIT();
        return failed;
    }

    sem->max_count = new_max_count;

    CRITICAL_SECTION_EXIT();
    return passed;
}

queue_status_t kernel_sem_take(kernel_sem_t *sem, uint32_t timeout_ticks)
{
    queue_status_t status;
 
    if (sem == NULL)
    {
        return QUEUE_FAILED;
    }
 
    port_critical_enter();
 
    if (sem->count > 0U)
    {
        // Fast path: a token is already available. 
        sem->count--;
        port_critical_exit();
        return QUEUE_PASSED;
    }
 
    if (timeout_ticks == 0U)
    {
        // No token and caller doesn't want to wait.
        port_critical_exit();
        return QUEUE_FAILED;
    }
 
    // No token available return time out
    current_task->timed_out = 0U;
    current_task->tick_counter = kernel_get_tick_count() + timeout_ticks; //increament timer
    current_task->current_wait_queue = &sem->waiters;
    current_task->state = TASK_STATE_BLOCKED; //BLOCK the task
    task_queue_push_task(&sem->waiters, current_task); // push task for getting ready
 
    port_critical_exit();
 
    kernel_yield(); //context switch
 
    port_critical_enter();
 
    if (current_task->timed_out)
    {
        port_critical_exit();
        return QUEUE_TIMED_OUT;
    }
 
    if (sem->count > 0U)
    {
        sem->count--;
        status = QUEUE_PASSED;
    }
    else
    {
        //Should not happen under normal operation but safety precautions
        status = QUEUE_FAILED;
    }
 
    port_critical_exit();
    return status;
}

//release tokens
queue_status_t kernel_sem_give(kernel_sem_t *sem)
{
    if (sem == NULL)
    {
        return QUEUE_FAILED;
    }
 
    port_critical_enter();
 
    if (sem->count >= sem->max_count) //check number of tokens is full
    {
        
        port_critical_exit();
        return QUEUE_FAILED;
    }
 
    sem->count++; // if not full add tokens
 
    if (!task_queue_empty(&sem->waiters))
    {
        TCB_t *woken = task_queue_pop_task(&sem->waiters); //pop highest priority task
        woken->current_wait_queue = NULL;
        woken->state = TASK_STATE_READY;
    }
 
    port_critical_exit();
    return QUEUE_PASSED;
}
uint32_t kernel_sem_get_count(const kernel_sem_t *sem)
{
    uint32_t count;
 
    if (sem == NULL)
    {
        return SEM_INVALID_COUNT;
    }
 
    port_critical_enter();
    count = sem->count; // we need to return after the critical section enter so we need to keep as it is
    port_critical_exit();
 
    return count;
}
