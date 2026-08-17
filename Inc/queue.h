#ifndef QUEUE_H
#define QUEUE_H
#include "task.h"
 
#include <stdint.h>
#include <stdbool.h>
 
/* Task waiting queue. 
    Stores tasks that are currently blocked while waiting for a resource, event, or queue operation
*/
struct task_queue_t
{
    TCB_t *head;        // First task in the waiting queue
    TCB_t *tail;        // Last task in the waiting queue
};


/* Kernel message queue.
    Stores fixed-size items in a circular buffer and maintains separate waiting queues for tasks trying to send or receive
*/
typedef struct
{
    uint8_t *buffer;            // Memory buffer used to store queue items
    uint8_t capacity;           // Maximum number of items the queue can hold
    uint16_t item_size;         // Size of each item in bytes

    /*
        Circular buffer indexes.
        head : position from which the next item is read
        tail : position at which the next item is written
    */
    uint8_t head;
    uint8_t tail;

    uint8_t count;      // Current number of items in the queue

    /*
        Indicates that an overflow condition occurred.
        Set when an item cannot be inserted because the queue is full.
    */
    uint8_t overflow_flag;

    /* Tasks blocked because the queue is full and they are waiting to send an item */
    task_queue_t senders_waiting;

    /* Tasks blocked because the queue is empty and they are waiting to receive an item */
    task_queue_t receivers_waiting;
} kernel_queue_t;


/* Result codes returned by queue operations */
typedef enum
{
    QUEUE_INIT_FAILED = 0,      // Queue initialization failed
    QUEUE_INIT_SUCCESS,         // Queue successfully initialized
    QUEUE_PASSED,               // Operation completed successfully
    QUEUE_FAILED,               // Operation failed
    QUEUE_TIMED_OUT             // Operation could not complete before timeout
} queue_status_t;


/* Initializes a kernel message queue.

    q : Queue to initialize
    buffer : Memory area used to store queue items
    capacity : Maximum number of items that can be stored
    item_size: Size of each item in bytes
*/
queue_status_t kernel_queue_init(kernel_queue_t *q, void *buffer, uint8_t capacity, uint16_t item_size);

/*  Inserts one item into the queue.
    This is the low-level, non-blocking operation. The function fails if the queue is full
*/
queue_status_t kernel_queue_push(kernel_queue_t *q, const void *data);

/*
    Removes one item from the queue.
    This is the low-level, non-blocking operation. The function fails if the queue is empty
*/
queue_status_t kernel_queue_pop(kernel_queue_t *q, void *data);

/* Adds a task inside a task waiting queue where tasks are sorted by their priority */
void task_queue_push_task(task_queue_t *q, TCB_t *task_tcb);

/*
    Removes and returns the first task from a waiting queue.
    Returns NULL if the queue is empty.
 */
TCB_t *task_queue_pop_task(task_queue_t *q);

/* Checks whether a task waiting queue is empty.
    Returns true if no task is waiting.
*/
bool task_queue_empty(task_queue_t *q);

/*  Removes a specific task from a waiting queue.
    Returns true if the task was found and removed, otherwise returns false.
*/
bool task_queue_remove(task_queue_t *q, TCB_t *task);

/*  Sends an item to the queue.
    If the queue is full, the calling task may be blocked until space becomes available or the timeout expires
    timeout_ticks : Maximum number of ticks to wait
*/
queue_status_t kernel_queue_send(kernel_queue_t *q, const void *item, uint32_t timeout_ticks);

/*  Receives an item from the queue.
    If the queue is empty, the calling task may be blocked until an item becomes available or the timeout expires
    timeout_ticks : Maximum number of ticks to wait
*/
queue_status_t kernel_queue_receive(kernel_queue_t *q, void *item, uint32_t timeout_ticks);

/*  ISR-safe version of kernel_queue_send().
    Attempts to send an item without blocking. The operation returns immediately if the queue is full
*/
queue_status_t kernel_queue_send_from_isr(kernel_queue_t *q, const void *item);

/*  ISR-safe version of kernel_queue_receive().
    Attempts to receive an item without blocking. The operation returns immediately if the queue is empty
*/
queue_status_t kernel_queue_receive_from_isr(kernel_queue_t *q, void *item);

#endif
 
