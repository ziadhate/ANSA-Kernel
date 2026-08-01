#include "../Inc/queue.h"
#include "../Inc/scheduler.h"
#include <string.h>

/*
	Initializes a kernel queue by configuring its storage buffer,
	resetting the circular buffer state, and preparing the sender and receiver waiting queues for operation.
*/
queue_status kernel_queue_init(kernel_queue_t *q, void *buffer, uint8_t capacity, uint16_t item_size)
{
    if ((q == NULL) || (buffer == NULL) || (capacity == 0U) || (item_size == 0U))
    {
        return init_failed;
    }
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
    return init_success;
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
        return failed;
    }
    memcpy(&q->buffer[q->tail*q->item_size], data, q->item_size);
    q->tail = (q->tail+1) % q->capacity;
    q->count++;
    return passed;
}

/*
	Removes the oldest item from the circular buffer, updates the queue
	state, and returns the appropriate operation status.
*/
queue_status kernel_queue_pop(kernel_queue_t *q, void *data)
{
    if (q->count==0)
    {
        return failed;
    }
    memcpy(data, &q->buffer[q->head*q->item_size], q->item_size);
    q->head = (q->head+1) % q->capacity;
    q->count--;
    return failed;
}

// Inserts a blocked task at the end of a waiting queue, preserving FIFO ordering for subsequent wake-up operations.
void task_queue_push_task(task_queue_t *q, TCB_t *task_tcb){
    // Code
	if(q == NULL || task_tcb == NULL){
		return;
	}
	task_tcb -> next == NULL;
    if (q->tail == NULL)
    {
        q->head = task_tcb;
        q->tail = task_tcb;
    }
    else
    {
        q->tail->next = task_tcb;
        q->tail = task_tcb;
    }
}

// Removes and returns the first task from a waiting queue while maintaining queue consistency.
TCB_t *task_queue_pop_task(task_queue_t *q){
    if ((q == NULL) || (q->head == NULL))
    {
        return NULL;
    }

    TCB_t *task = q->head;
    q->head = task->next;

    if (q->head == NULL)
    {
        q->tail = NULL;
    }

    task->next = NULL;
    return task;
}

// Checks whether the specified waiting queue contains any blocked tasks.
bool task_queue_empty(task_queue_t *q){
    return (q == NULL) || (q->head == NULL);
}

// Removes a specific task from a waiting queue regardless of its position and updates the queue links accordingly.
// Useful when we remove tasks based on a specific parameter (ex: priority)
bool task_queue_remove(task_queue_t *q, TCB_t *task){
    if ((q == NULL) || (task == NULL) || (q->head == NULL))
    {
        return false;
    }

    TCB_t *prev = NULL;
    TCB_t *curr = q->head;

    while (curr != NULL)
    {
        if (curr == task)
        {
            if (prev == NULL)
            {
                q->head = curr->next;
            }
            else
            {
                prev->next = curr->next;
            }

            if (curr == q->tail)
            {
                q->tail = prev;
            }

            curr->next = NULL;
            return true;
        }

        prev = curr;
        curr = curr->next;
    }

    return false;
}

/*
	Sends an item to a kernel queue. If the queue is full, the calling task may block until space becomes available
	or the specified timeout expires. 
	Waiting receiver tasks are awakened when new data becomes available.
*/
/*
	Sends an item to a kernel queue. If the queue is full, the calling task may block until space becomes available
	// or the specified timeout expires. 
	Waiting receiver tasks are awakened when new data becomes available.
 
	Blocking model (matches TCB_t / scheduler.h):
	  - current_task is the running task (extern in scheduler.h).
	  - Blocking is done by setting current_task->state = TASK_STATE_BLOCKED,
	    recording current_wait_queue + tick_counter (timeout), linking the
	    task into the queue's wait list, then calling kernel_yield() to
	    force a context switch. kernel_yield() is assumed to block until
	    this task is scheduled again (same pattern as task_delay()).
	  - Whichever side wakes a waiting task (either the opposite send/receive
	    call, or kernel_tick_check_timeouts() on expiry) is responsible for
	    unlinking it from the wait list and setting state back to READY --
	    so a task never needs to remove itself from the list it was on.
	  - task->timed_out distinguishes why we woke up.
*/
queue_status kernel_queue_send(kernel_queue_t *q, const void *item, uint32_t timeout_ticks){
    queue_status status;
 
    if ((q == NULL) || (item == NULL))
    {
        return failed;
    }
 
    CRITICAL_SECTION_ENTER();
 
    if (q->count < q->capacity)
    {
        /* Fast path: room available, no blocking needed. */
        (void)kernel_queue_push(q, item);
 
        if (!task_queue_empty(&q->receivers_waiting))
        {
            TCB_t *woken = task_queue_pop_task(&q->receivers_waiting);
            woken->current_wait_queue = NULL;
            woken->state = TASK_STATE_READY;
        }
 
        CRITICAL_SECTION_EXIT();
        return passed;
    }
 
    if (timeout_ticks == 0U)
    {
        /* Queue full and caller doesn't want to wait. */
        CRITICAL_SECTION_EXIT();
        return failed;
    }
 
    /* Queue is full: block the calling task until space frees up or timeout expires. */
    current_task->timed_out = 0U;
    current_task->tick_counter = timeout_ticks;
    current_task->current_wait_queue = &q->senders_waiting;
    current_task->state = TASK_STATE_BLOCKED;
    task_queue_push_task(&q->senders_waiting, current_task);
 
    CRITICAL_SECTION_EXIT();
 
    /* Give up the CPU; we resume here once woken (by a receiver) or timed out. */
    kernel_yield();
 
    CRITICAL_SECTION_ENTER();
 
    if (current_task->timed_out)
    {
        CRITICAL_SECTION_EXIT();
        return timed_out;
    }
 
    /* Woken because a receiver freed up space for us; we were already
       unlinked from senders_waiting by whoever woke us. */
    status = kernel_queue_push(q, item);
 
    if (!task_queue_empty(&q->receivers_waiting))
    {
        TCB_t *woken = task_queue_pop_task(&q->receivers_waiting);
        woken->current_wait_queue = NULL;
        woken->state = TASK_STATE_READY;
    }
 
    CRITICAL_SECTION_EXIT();
    return status;
}
 
/* 
	Receives an item from a kernel queue. If the queue is empty, the calling task may block until data becomes available
	or the specified timeout expires. 
	Waiting sender tasks are awakened when space becomes available.
 
	Mirrors kernel_queue_send() -- see the comment above it for the blocking model.
 */
queue_status kernel_queue_receive(kernel_queue_t *q, void *item, uint32_t timeout_ticks){
    queue_status status;
 
    if ((q == NULL) || (item == NULL))
    {
        return failed;
    }
 
    CRITICAL_SECTION_ENTER();
 
    if (q->count > 0U)
    {
        /* Fast path: data available, no blocking needed. */
        (void)kernel_queue_pop(q, item);
 
        if (!task_queue_empty(&q->senders_waiting))
        {
            TCB_t *woken = task_queue_pop_task(&q->senders_waiting);
            woken->current_wait_queue = NULL;
            woken->state = TASK_STATE_READY;
        }
 
        CRITICAL_SECTION_EXIT();
        return passed;
    }
 
    if (timeout_ticks == 0U)
    {
        /* Queue empty and caller doesn't want to wait. */
        CRITICAL_SECTION_EXIT();
        return failed;
    }
 
    /* Queue is empty: block the calling task until data arrives or timeout expires. */
    current_task->timed_out = 0U;
    current_task->tick_counter = timeout_ticks;
    current_task->current_wait_queue = &q->receivers_waiting;
    current_task->state = TASK_STATE_BLOCKED;
    task_queue_push_task(&q->receivers_waiting, current_task);
 
    CRITICAL_SECTION_EXIT();
 
    /* Give up the CPU; we resume here once woken (by a sender) or timed out. */
    kernel_yield();
 
    CRITICAL_SECTION_ENTER();
 
    if (current_task->timed_out)
    {
        CRITICAL_SECTION_EXIT();
        return timed_out;
    }
 
    /* Woken because a sender put something in for us; we were already
       unlinked from receivers_waiting by whoever woke us. */
    status = kernel_queue_pop(q, item);
 
    if (!task_queue_empty(&q->senders_waiting))
    {
        TCB_t *woken = task_queue_pop_task(&q->senders_waiting);
        woken->current_wait_queue = NULL;
        woken->state = TASK_STATE_READY;
    }
 
    CRITICAL_SECTION_EXIT();
    return status;
}
 
/* 
	Scans blocked tasks waiting on queues, detects expired timeouts, removes timed-out tasks from their waiting queues, and
	makes them ready for execution.
 
	No separate queue registry is needed: task_table[]/total_tasks (from
	task.h) already lists every task, and a task blocked on a queue is
	identifiable purely by (state == TASK_STATE_BLOCKED &&
	current_wait_queue != NULL) -- tasks blocked via task_delay() instead
	have current_wait_queue == NULL, so they're naturally skipped here.
 
	Intended to be called once per tick from kernel_update_ticks().
*/
void kernel_tick_check_timeouts(void){
    uint8_t i;
 
    CRITICAL_SECTION_ENTER();
 
    for (i = 0U; i < total_tasks; i++)
    {
        TCB_t *task = &task_table[i];
 
        if ((task->state == TASK_STATE_BLOCKED) && (task->current_wait_queue != NULL))
        {
            if (task->tick_counter > 0U)
            {
                task->tick_counter--;
 
                if (task->tick_counter == 0U)
                {
                    (void)task_queue_remove(task->current_wait_queue, task);
                    task->current_wait_queue = NULL;
                    task->timed_out = 1U;
                    task->state = TASK_STATE_READY;
                }
            }
        }
    }
 
    CRITICAL_SECTION_EXIT();
}