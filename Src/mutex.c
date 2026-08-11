#include "../Inc/mutex.h"
#include "../Inc/scheduler.h"
#include "../Port/Cortex_M4/Inc/port.h"
#include <stddef.h>

mutex_status_t kernel_mutex_init(kernel_mutex_t *mutex)
{
    
    if (mutex == NULL)
    {
        return MUTEX_ERROR;
    }

    port_critical_enter();
   
    //Mutex is initially unlocked.
    mutex->locked = false;
    
    //No task owns the mutex initially.
    mutex->owner = NULL;

    //Initialize the waiting task queue. NULL means no task is waiting.
    mutex->waiting_tasks.head = NULL;
    mutex->waiting_tasks.tail = NULL;
    
    port_critical_exit();

    return MUTEX_SUCCESS;
}


mutex_status_t kernel_mutex_lock(kernel_mutex_t *mutex)
{
    //Check for invalid mutex pointer.
    if (mutex == NULL)
    {
        return MUTEX_ERROR;
    }

    /*We need a valid current task because the mutex owner must be a task.*/
    if (current_task == NULL)
    {
        return MUTEX_ERROR;
    }

    /* Enter critical section because we are going to modify kernel synchronization data.*/
    port_critical_enter();

    //Check whether mutex is currently free.
    if (mutex->locked == false)
    {
        //Mutex is free. Give ownership to current task.
        mutex->locked = true;
        mutex->owner = current_task;
        
        //Critical section is no longer required.
        port_critical_exit();
        return MUTEX_SUCCESS;
    }
    
    /*
    Change current task state to BLOCKED. The scheduler will therefore not select this task while it is waiting. */
    current_task->current_wait_queue = &mutex->waiting_tasks;
    current_task->state = TASK_STATE_BLOCKED;
    task_queue_push_task(&mutex->waiting_tasks, current_task);

    port_critical_exit();

    kernel_yield();

    return MUTEX_SUCCESS;
}


mutex_status_t kernel_mutex_unlock(kernel_mutex_t *mutex)
{
    /* Check mutex pointer. */
    if (mutex == NULL)
    {
        return MUTEX_ERROR;
    }
   
    //There must be a current task.
    if (current_task == NULL)
    {
        return MUTEX_ERROR;
    }
   
    /* Enter critical section because mutex ownership and waiting queue will be modified.*/
    port_critical_enter();

    bool yield_requested = false;
    // Check whether the current task owns the mutex.
    if (mutex->owner != current_task)
    {
        //A task other than the owner is trying to unlock the mutex.
        port_critical_exit();
        return MUTEX_ERROR;
    }


    /* Check whether any task is waiting for the mutex. */
    if (task_queue_empty(&mutex->waiting_tasks))
    {
        mutex->locked = false;
        mutex->owner = NULL;
    }
    else
    {
        
        /* At least one task is waiting. Remove the first waiting task.*/
        TCB_t *next_owner =task_queue_pop_task(&mutex->waiting_tasks);

        //Make sure a valid task was obtained.
        if (next_owner == NULL)
        {
            //Something went wrong with the queue.
            port_critical_exit();

            return MUTEX_ERROR;
        }

        mutex->locked = true;
        mutex->owner = next_owner;
        
        next_owner->state = TASK_STATE_READY; //Wake the waiting task.
        next_owner->current_wait_queue = NULL; //Clear the task's waiting queue information.

        yield_requested=true;
    }

    port_critical_exit();

    /* First check if a yield should occur, then request scheduler yield to check whether the newly READY task should execute.*/
    if (yield_requested){
        kernel_yield();
    }

    return MUTEX_SUCCESS;
}

mutex_status_t kernel_mutex_try_lock(kernel_mutex_t *mutex)
{
    if (mutex == NULL)
    {
        return MUTEX_ERROR;
    }

    /*  A task must exist to own the mutex.*/
    if (current_task == NULL)
    {
        return MUTEX_ERROR;
    }

    /* Protect mutex access. */
    port_critical_enter();

    //Check whether mutex is already locked.  
    if (mutex->locked == true)
    {
        /* Mutex is owned. Do not block the current task. */
        port_critical_exit();

        return MUTEX_BUSY;
    }

    /*Mutex is free. Give ownership to current task.*/
    mutex->locked = true;
    mutex->owner = current_task;

    port_critical_exit();

    return MUTEX_SUCCESS;
}
