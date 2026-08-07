#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h> 
#include <stdbool.h>

#include <task.h>
#include <queue.h>

typedef struct
{
    bool locked;               //Mutex State
    TCB_t *owner;              //Current Task Which is Owner of Mutex
    task_queue_t Waiting_Tasks; //Task Waiting for Mutex 


}kernel_mutex_t;

typedef enum
{
    MUTEX_SUCCESS = 0,
    MUTEX_ERROR,
    MUTEX_BUSY,
    MUTEX_TIMEOUT

} mutex_status_t;

mutex_status_t kernel_mutex_init(kernel_mutex_t *mutex);

mutex_status_t kernel_mutex_lock(kernel_mutex_t *mutex);

mutex_status_t kernel_mutex_unlock(kernel_mutex_t *mutex);

mutex_status_t kernel_mutex_try_lock(kernel_mutex_t *mutex);


#endif