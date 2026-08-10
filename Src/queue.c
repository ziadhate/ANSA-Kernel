#include "../Inc/queue.h"
#include "../Inc/scheduler.h"
#include <string.h>
 
/*
	Initializes a kernel queue by configuring its storage buffer,
	resetting the circular buffer state, and preparing the sender and receiver waiting queues for operation.
*/
queue_status_t kernel_queue_init(kernel_queue_t *q, void *buffer, uint8_t capacity, uint16_t item_size)
{
    if ((q == NULL) || (buffer == NULL) || (capacity == 0U) || (item_size == 0U))
    {
        return QUEUE_INIT_FAILED;
    }
	port_critical_enter();
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
    
    port_critical_exit();
    return QUEUE_INIT_SUCCESS;
}
 
/* 
	Inserts a new item into the circular buffer if space is available.
	Updates the queue state and returns the appropriate status.
*/
queue_status_t kernel_queue_push(kernel_queue_t *q, const void *data)
{
    if (q->count==q->capacity)
    {
        q->overflow_flag = 1;
        return QUEUE_FAILED;
    }
    memcpy(&q->buffer[q->tail*q->item_size], data, q->item_size);
    q->tail = (q->tail+1) % q->capacity;
    q->count++;
    return QUEUE_PASSED;
}
 
/*
	Removes the oldest item from the circular buffer, updates the queue
	state, and returns the appropriate operation status.
*/
queue_status_t kernel_queue_pop(kernel_queue_t *q, void *data)
{
    if (q->count==0)
    {
        return QUEUE_FAILED;
    }
    memcpy(data, &q->buffer[q->head*q->item_size], q->item_size);
    q->head = (q->head+1) % q->capacity;
    q->count--;
<<<<<<< HEAD
    return passed;

    return QUEUE_PASSED;
>>>>>>> 6b52e105fb5629fbf31170d9af30d42b07535610
}
 
/*
	Inserts a blocked task at the end of a waiting queue, preserving FIFO
	arrival order. Priority ordering is handled by task_queue_pop_task()
	at removal time instead of at insertion time (see comment there).
*/

int task_queue_push_task(task_queue_t *q, TCB_t *task_tcb){
    if ((q == NULL) || (task_tcb == NULL))
    {
        return -1;
    }

    task_tcb->next = NULL;

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

    return 0;
}
 
/*
	Removes and returns the highest-priority task from a waiting queue,
	scanning the whole (FIFO-ordered) list rather than just popping the
	head. Ties are broken in favor of whichever equal-priority task has
	been waiting longest (i.e. appears earliest), by using a strict "<"
	comparison so the first min-value (highest-priority) node found
	during the left-to-right scan wins.
 
	NOTE: TCB_t->priority is a uint8_t with no fixed direction defined
	elsewhere, so this assumes the convention "SMALLER value == HIGHER
	priority" (e.g. priority 0 is the most urgent, larger numbers are
	lower priority -- idle task would be created at the highest number).
	If your scheduler actually treats larger values as higher priority
	instead, flip the comparison below ("<" -> ">") -- nothing else needs
	to change.
*/
TCB_t *task_queue_pop_task(task_queue_t *q){
    TCB_t *curr;
    TCB_t *curr_prev;
    TCB_t *best;
    TCB_t *best_prev;
 
    if ((q == NULL) || (q->head == NULL))
    {
        return NULL;
    }
 
    best = q->head;
    best_prev = NULL;
 
    curr_prev = q->head;
    curr = q->head->next;
 
    while (curr != NULL)
    {
        if (curr->priority < best->priority)
        {
            best = curr;
            best_prev = curr_prev;
        }
 
        curr_prev = curr;
        curr = curr->next;
    }
 
    if (best_prev == NULL)
    {
        q->head = best->next;
    }
    else
    {
        best_prev->next = best->next;
    }
 
    if (best == q->tail)
    {
        q->tail = best_prev;
    }
 
    best->next = NULL;
    return best;
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
	// or the specified timeout expires. 
	Waiting receiver tasks are awakened when new data becomes available.
 
	Blocking model (matches TCB_t / scheduler.h):
	  - current_task is the running task (extern in scheduler.h).
	  - Blocking is done by setting current_task->state = TASK_STATE_BLOCKED,
	    recording current_wait_queue + an absolute deadline tick in
	    current_task->tick_counter (kernel_get_tick_count() + timeout_ticks),
	    linking the task into the queue's wait list (plain FIFO append --
	    see task_queue_push_task()), then calling kernel_yield() to force a
	    context switch. kernel_yield() is assumed to block until this task
	    is scheduled again (same pattern as task_delay()).
	  - Whichever side wakes a waiting task (either the opposite send/receive
	    call via task_queue_pop_task(), or kernel_tick_check_timeouts() via
	    task_queue_remove() on expiry) is responsible for unlinking it from
	    the wait list and setting state back to READY -- so a task never
	    needs to remove itself from the list it was on.
	  - task_queue_pop_task() scans the wait list and removes the
	    HIGHEST-PRIORITY task (not simply the first one that blocked) --
	    see the comment on task_queue_pop_task() for the priority
	    convention assumed.
	  - task->timed_out distinguishes why we woke up.
	  - tick_counter is now an ABSOLUTE deadline tick rather than a
	    per-tick decremented countdown: kernel_tick_check_timeouts()
	    simply compares kernel_get_tick_count() against it instead of
	    counting down every tick.
*/
queue_status_t kernel_queue_send(kernel_queue_t *q, const void *item, uint32_t timeout_ticks){
    queue_status_t status;
    if ((q == NULL) || (item == NULL))
    {
        return QUEUE_FAILED;
    }
 
    port_critical_enter();
 
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
 
        port_critical_exit();
        return QUEUE_PASSED;
    }
 
    if (timeout_ticks == 0U)
    {
        /* Queue full and caller doesn't want to wait. */
        port_critical_exit();
        return QUEUE_FAILED;
    }
 
    /* Queue is full: block the calling task until space frees up or timeout expires.
       tick_counter now holds an ABSOLUTE deadline tick, not a countdown. */
    current_task->timed_out = 0U;
    current_task->tick_counter = kernel_get_tick_count() + timeout_ticks;
    current_task->current_wait_queue = &q->senders_waiting;
    current_task->state = TASK_STATE_BLOCKED;
    task_queue_push_task(&q->senders_waiting, current_task);
 
    port_critical_exit();
 
    /* Give up the CPU; we resume here once woken (by a receiver) or timed out. */
    kernel_yield();
 
    port_critical_enter();
 
    if (current_task->timed_out)
    {
        port_critical_exit();
        return QUEUE_TIMED_OUT;
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
 
    port_critical_exit();
    return status;
}
 
/* 
	Receives an item from a kernel queue. If the queue is empty, the calling task may block until data becomes available
	or the specified timeout expires. 
	Waiting sender tasks are awakened when space becomes available.
 
	Mirrors kernel_queue_send() -- see the comment above it for the blocking model.
 */
queue_status_t kernel_queue_receive(kernel_queue_t *q, void *item, uint32_t timeout_ticks){
    queue_status_t status;
 
    if ((q == NULL) || (item == NULL))
    {
        return QUEUE_FAILED;
    }
 
    port_critical_enter();
 
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
 
        port_critical_exit();
        return QUEUE_PASSED;
    }
 
    if (timeout_ticks == 0U)
    {
        /* Queue empty and caller doesn't want to wait. */
        port_critical_exit();
        return QUEUE_FAILED;
    }
 
    /* Queue is empty: block the calling task until data arrives or timeout expires.
       tick_counter now holds an ABSOLUTE deadline tick, not a countdown. */
    current_task->timed_out = 0U;
    current_task->tick_counter = kernel_get_tick_count() + timeout_ticks;
    current_task->current_wait_queue = &q->receivers_waiting;
    current_task->state = TASK_STATE_BLOCKED;
    task_queue_push_task(&q->receivers_waiting, current_task);
 
    port_critical_exit();
 
    /* Give up the CPU; we resume here once woken (by a sender) or timed out. */
    kernel_yield();
 
    port_critical_enter();
 
    if (current_task->timed_out)
    {
        port_critical_exit();
        return QUEUE_TIMED_OUT;
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
 
    port_critical_exit();
    return status;
}