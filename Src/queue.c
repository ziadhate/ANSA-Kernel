#include "../Inc/queue.h"
#include "../Inc/scheduler.h"

/*
	Initializes a kernel queue by configuring its storage buffer,
	resetting the circular buffer state, and preparing the sender and receiver waiting queues for operation.
*/
queue_status kernel_queue_init(kernel_queue_t *q, void *buffer, uint8_t capacity, uint16_t item_size)
{
	CRITICAL_SECTION_ENTER();
    q->buffer=(uint8_t *)buffer;
    q->capacity=capacity;
    q->item_size=item_size;
    q->head=0;
    q->tail=0;
    q->count=0;
    q->overflow_flag=0;
    
    q->senders_waiting.head=NULL;
    q->senders_waiting.tail=NULL;
    
    q->receivers_waiting.head=NULL;
    q->receivers_waiting.tail=NULL;
    
    CRITICAL_SECTION_EXIT();
}

/* 
	Inserts a new item into the circular buffer if space is available.
	Updates the queue state and returns the appropriate status.
*/
queue_status kernel_queue_push(kernel_queue_t *q, const void *data)
{
    if (q->count==q->capacity)
    {
        q->overflow_flag = 1;
        return init_failed;
    }
    memcpy(&q->buffer[q->tail*q->item_size], data, q->item_size);
    q->tail = (q->tail+1) % q->capacity;
    q->count++;
    return init_success;
}

/*
	Removes the oldest item from the circular buffer, updates the queue
	state, and returns the appropriate operation status.
*/
uint8_t kernel_queue_pop(kernel_queue_t *q, void *data)
{
    if (q->count==0)
    {
        return init_failed;
    }
    memcpy(data, &q->buffer[q->head*q->item_size], q->item_size);
    q->head = (q->head+1) % q->capacity;
    q->count--;
    return init_failed;
}

// Inserts a blocked task at the end of a waiting queue, preserving FIFO ordering for subsequent wake-up operations.
void task_queue_push_task(task_queue_t *q, TCB_t *task_tcb){
    // Code
}

// Removes and returns the first task from a waiting queue while maintaining queue consistency.
TCB_t *task_queue_pop_task(task_queue_t *q){
    // Code
}

// Checks whether the specified waiting queue contains any blocked tasks.
bool task_queue_empty(task_queue_t *q){
    // Code
}

// Removes a specific task from a waiting queue regardless of its position and updates the queue links accordingly.
// Useful when we remove tasks based on a specific parameter (ex: priority)
TCB_t task_queue_remove(task_queue_t *q, TCB_t *task){
    // Code
}

/*
	Sends an item to a kernel queue. If the queue is full, the calling task may block until space becomes available
	or the specified timeout expires. 
	Waiting receiver tasks are awakened when new data becomes available.
*/
queue_status kernel_queue_send(kernel_queue_t *q, const void *item, uint32_t timeout_ticks){
    // Code
}

/* 
	Receives an item from a kernel queue. If the queue is empty, the calling task may block until data becomes available
	or the specified timeout expires. 
	Waiting sender tasks are awakened when space becomes available.
 */
queue_status kernel_queue_receive(kernel_queue_t *q, void *item, uint32_t timeout_ticks){
    // Code
}

/* 
	Scans blocked tasks waiting on queues, detects expired timeouts, removes timed-out tasks from their waiting queues, and
	makes them ready for execution.
*/
void kernel_tick_check_timeouts(void){
    // Code
}

