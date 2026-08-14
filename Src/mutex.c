#include "../Inc/mutex.h"
#include "../Inc/scheduler.h"
#include "../Port/Cortex_M4/Inc/port.h"
#include <stddef.h>


/*
 * Initialize a mutex.
 */
mutex_status_t kernel_mutex_init(kernel_mutex_t *mutex)
{
    /* Check for invalid mutex pointer */
    if (mutex == NULL)
    {
        return MUTEX_ERROR;
    }

    /* Enter critical section */
    port_critical_enter();
    CRITICAL_SECTION_ENTER();

    /* Mutex is initially unlocked */
    mutex->locked = false;

    /* No task owns the mutex initially */
    mutex->owner = NULL;

    /* Initialize waiting task queue */
    mutex->waiting_tasks.head = NULL;
    mutex->waiting_tasks.tail = NULL;

    /* No linked mutex initially */
    mutex->next_owned = NULL;

    /* Exit critical section */
    CRITICAL_SECTION_EXIT();
    port_critical_exit();

    return MUTEX_SUCCESS;
}


/*
 * Lock a mutex.
 *
 * If the mutex is free:
 *      Current task becomes the owner.
 *
 * If the mutex is already locked:
 *      Current task is added to the waiting queue.
 *      Priority inheritance is applied.
 *      Current task is blocked.
 */
mutex_status_t kernel_mutex_lock(kernel_mutex_t *mutex)
{
    /* Check for invalid mutex pointer */
    if (mutex == NULL)
    {
        return MUTEX_ERROR;
    }

    /* A valid current task is required */
    if (current_task == NULL)
    {
        return MUTEX_ERROR;
    }

    /* Enter critical section */
    port_critical_enter();
    CRITICAL_SECTION_ENTER();

    /*
     * Case 1:
     * Mutex is free.
     */
    if (mutex->locked == false)
    {
        /* Lock the mutex */
        mutex->locked = true;

        /* Make current task the owner */
        mutex->owner = current_task;

        /*
         * Add mutex to owner's mutex list.
         */
        mutex->next_owned =
            current_task->owned_mutexes;

        current_task->owned_mutexes =
            mutex;

        /* Current task is not blocked */
        current_task->blocked_on = NULL;

        /* Exit critical section */
        CRITICAL_SECTION_EXIT();
        port_critical_exit();

        return MUTEX_SUCCESS;
    }

    /*
     * Case 2:
     * Mutex is already locked.
     */

    /*
     * Prevent recursive locking.
     */
    if (mutex->owner == current_task)
    {
        CRITICAL_SECTION_EXIT();
        port_critical_exit();

        return MUTEX_ERROR;
    }

    /*
     * Priority Inheritance
     *
     * Lower number = higher priority.
     *
     * If the waiting task has a higher priority
     * than the mutex owner, temporarily raise
     * the owner's priority.
     */
    if (current_task->priority < mutex->owner->priority)
    {
        mutex->owner->priority =
            current_task->priority;
    }

    /*
     * Remember which mutex is blocking
     * the current task.
     */
    current_task->blocked_on = mutex;

    /*
     * Add current task to waiting queue.
     */
    if (task_queue_push_task(
            &mutex->waiting_tasks,
            current_task) != 0)
    {
        /*
         * Queue insertion failed.
         */
        current_task->blocked_on = NULL;

        CRITICAL_SECTION_EXIT();
        port_critical_exit();

        return MUTEX_ERROR;
    }

    /*
     * Block the current task.
     */
    current_task->state = TASK_STATE_BLOCKED;

    /* Exit critical section */
    CRITICAL_SECTION_EXIT();
    port_critical_exit();

    /*
     * Give CPU to another task.
     */
    kernel_yield();

    return MUTEX_SUCCESS;
}


/*
 * Unlock a mutex.
 *
 * If no task is waiting:
 *      Mutex becomes completely unlocked.
 *
 * If tasks are waiting:
 *      Mutex ownership is transferred to the
 *      highest-priority waiting task.
 */
mutex_status_t kernel_mutex_unlock(kernel_mutex_t *mutex)
{
    /* Check for invalid mutex pointer */
    if (mutex == NULL)
    {
        return MUTEX_ERROR;
    }

    /* A valid current task is required */
    if (current_task == NULL)
    {
        return MUTEX_ERROR;
    }

    /* Enter critical section */
    port_critical_enter();
    CRITICAL_SECTION_ENTER();

    /*
     * Check whether current task owns the mutex.
     */
    if (mutex->owner != current_task)
    {
        /*
         * Only the owner can unlock the mutex.
         */
        CRITICAL_SECTION_EXIT();
        port_critical_exit();

        return MUTEX_ERROR;
    }

    /*
     * Case 1:
     * No task is waiting for the mutex.
     */
    if (task_queue_empty(&mutex->waiting_tasks))
    {
        /*
         * Remove mutex from owner's mutex list.
         */
        if (current_task->owned_mutexes == mutex)
        {
            current_task->owned_mutexes =
                mutex->next_owned;
        }

        mutex->next_owned = NULL;

        /*
         * Restore original priority.
         */
        current_task->priority =
            current_task->base_priority;

        /*
         * Completely unlock the mutex.
         */
        mutex->locked = false;
        mutex->owner = NULL;
    }

    /*
     * Case 2:
     * At least one task is waiting.
     */
    else
    {
        /*
         * Get the next waiting task.
         *
         * task_queue_pop_task() selects the
         * appropriate waiting task according
         * to your queue implementation.
         */
        TCB_t *next_owner =
            task_queue_pop_task(&mutex->waiting_tasks);

        /*
         * Make sure a valid task was obtained.
         */
        if (next_owner == NULL)
        {
            CRITICAL_SECTION_EXIT();
            port_critical_exit();

            return MUTEX_ERROR;
        }

        /*
         * Remove mutex from old owner's list.
         */
        if (current_task->owned_mutexes == mutex)
        {
            current_task->owned_mutexes =
                mutex->next_owned;
        }

        mutex->next_owned = NULL;

        /*
         * Restore old owner's original priority.
         */
        current_task->priority =
            current_task->base_priority;

        /*
         * Transfer mutex ownership.
         */
        mutex->locked = true;
        mutex->owner = next_owner;

        /*
         * Add mutex to new owner's list.
         */
        mutex->next_owned =
            next_owner->owned_mutexes;

        next_owner->owned_mutexes =
            mutex;

        /*
         * Wake the new owner.
         */
        next_owner->state =
            TASK_STATE_READY;

        next_owner->blocked_on = NULL;

        next_owner->current_wait_queue = NULL;

        next_owner->next = NULL;
    }

    /* Exit critical section */
    CRITICAL_SECTION_EXIT();
    port_critical_exit();

    /*
     * Ask scheduler whether another task
     * should execute.
     */
    kernel_yield();

    return MUTEX_SUCCESS;
}


/*
 * Try to lock a mutex.
 *
 * Unlike kernel_mutex_lock(), this function
 * never blocks the current task.
 *
 * Returns:
 *      MUTEX_SUCCESS -> mutex acquired
 *      MUTEX_BUSY    -> mutex already locked
 *      MUTEX_ERROR   -> invalid argument
 */
mutex_status_t kernel_mutex_try_lock(kernel_mutex_t *mutex)
{
    /* Check for invalid mutex pointer */
    if (mutex == NULL)
    {
        return MUTEX_ERROR;
    }

    /* A valid current task is required */
    if (current_task == NULL)
    {
        return MUTEX_ERROR;
    }

    /* Enter critical section */
    port_critical_enter();
    CRITICAL_SECTION_ENTER();

    /*
     * Check whether mutex is already locked.
     */
 if (mutex->locked == true)
{
    if (mutex->owner == current_task)
    {
        CRITICAL_SECTION_EXIT();
        port_critical_exit();

        return MUTEX_ERROR;
    }

    CRITICAL_SECTION_EXIT();
    port_critical_exit();

    return MUTEX_BUSY;
}

    /*
     * Mutex is free.
     * Give ownership to current task.
     */
    mutex->locked = true;
    mutex->owner = current_task;

    /*
     * Add mutex to owner's mutex list.
     */
    mutex->next_owned = current_task->owned_mutexes;

    current_task->owned_mutexes = mutex;

    current_task->blocked_on = NULL;

    /* Exit critical section */
    CRITICAL_SECTION_EXIT();
    port_critical_exit();

    return MUTEX_SUCCESS;
}