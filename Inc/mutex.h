#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h> 
#include <stdbool.h>

#include <task.h>
#include <queue.h>

/*  Kernel mutex.
    A mutex provides exclusive access to a shared resource. Only one task can own the mutex at a time.
*/
typedef struct kernel_mutex_t
{
    bool locked;         // Indicates whether the mutex is currently locked

    /* Task currently owning the mutex. NULL when the mutex is not owned */
    TCB_t *owner;

    /* Queue of tasks blocked while waiting to acquire the mutex */
    task_queue_t waiting_tasks;

    /* Links this mutex to the next mutex owned by the same task.
        Used to maintain the list of mutexes currently owned by the task stored in 'owner'.
    */
    struct kernel_mutex_t *next_owned;

} kernel_mutex_t;


/* Result codes returned by mutex operations */
typedef enum
{
    MUTEX_SUCCESS = 0,       // Operation completed successfully
    MUTEX_ERROR,             // Invalid argument or internal error
    MUTEX_BUSY,              // Mutex is already owned
    MUTEX_TIMEOUT            // Mutex could not be acquired before timeout

} mutex_status_t;


/*
    Initializes a mutex.
    The mutex starts in the unlocked state with no owner and an empty waiting queue.
*/
mutex_status_t kernel_mutex_init(kernel_mutex_t *mutex);

/*  Acquires the mutex.
    If the mutex is available, the current task becomes its owner.
    If the mutex is already owned, the current task may be blocked until the mutex becomes available.
*/
mutex_status_t kernel_mutex_lock(kernel_mutex_t *mutex);

/*
    Releases the mutex.
    The current task must be the owner of the mutex.
    If other tasks are waiting, one of them may be unblocked and become the new owner.
*/
mutex_status_t kernel_mutex_unlock(kernel_mutex_t *mutex);

/*
    Attempts to acquire the mutex without blocking.
    Returns MUTEX_SUCCESS if the mutex was acquired.
    Returns MUTEX_BUSY immediately if another task already owns it.
*/
mutex_status_t kernel_mutex_try_lock(kernel_mutex_t *mutex);

#endif
