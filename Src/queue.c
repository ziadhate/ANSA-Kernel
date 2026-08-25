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

    return QUEUE_PASSED;
}
 
/*
    Inserts a task into the waiting queue while maintaining ascending
    priority order. Tasks with the same priority are kept in FIFO order,
    meaning the task that arrived first is placed before later tasks with the same priority.
*/
void task_queue_push_task(task_queue_t *q, TCB_t *task_tcb)
{
    if ((q == NULL) || (task_tcb == NULL))
    {
        return;
    }

    task_tcb->next = NULL;

    /* Queue empty */
    if (q->head == NULL)
    {
        q->head = task_tcb;
        q->tail = task_tcb;
        return;
    }

    /* New task has the highest priority */
    if (task_tcb->priority < q->head->priority)
    {
        task_tcb->next = q->head;
        q->head = task_tcb;
        return;
    }

    /* Find insertion position */
    TCB_t *current = q->head;

    while ((current->next != NULL) &&
           (current->next->priority <= task_tcb->priority))
    {
        current = current->next;
    }

    task_tcb->next = current->next;
    current->next = task_tcb;

    /* New task was inserted at the end */
    if (task_tcb->next == NULL)
    {
        q->tail = task_tcb;
    }
}

/*
    Removes and returns the highest-priority task from the waiting queue.
    The queue is already sorted by priority, so the task at the head
    of the queue is always the next task to be released.
*/
TCB_t *task_queue_pop_task(task_queue_t *q)
{
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
// Useful when we remove tasks based on a specific parameter (ex: emergency)
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
	    current_task->wakeup_tick (kernel_get_tick_count() + timeout_ticks),
	    linking the task into the queue's wait list (plain priority-based sorted append, 1st task is the 
        Highest-priority task--
	    see task_queue_push_task()), then calling kernel_yield() to force a
	    context switch. kernel_yield() is assumed to block until this task
	    is scheduled again (same pattern as task_delay()).
	  - Whichever side wakes a waiting task (either the opposite send/receive
	    call via task_queue_pop_task(), or kernel_tick_check_timeouts() via
	    task_queue_remove() on expiry) is responsible for unlinking it from
	    the wait list and setting state back to READY -- so a task never
	    needs to remove itself from the list it was on.
	  - task_queue_pop_task() removes the HIGHEST-PRIORITY task which is the first task --
	    see the comment on task_queue_pop_task().
	  - task->timed_out distinguishes why we woke up.
	  - Wakeup_tick is now an ABSOLUTE deadline tick rather than a
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

    bool yield_requested=false;

    /*
        Direct hand-off: a receiver is already blocked waiting for the
        next item. Copy straight into its destination buffer instead of
        going through the ring buffer, that gap (drop item, then wake
        the receiver to come fetch it later) is what let a third task's
        fast path steal the item before the woken receiver got to run.
    */
    if (!task_queue_empty(&q->receivers_waiting))
    {
        TCB_t *woken = task_queue_pop_task(&q->receivers_waiting);
        memcpy(woken->pending_item, item, q->item_size);
        woken->current_wait_queue = NULL;
        woken->state = TASK_STATE_READY;

        if(woken->priority<current_task->priority){
            yield_requested=true;
        }

        port_critical_exit();

        if (yield_requested){
            port_trigger_context_switch();
        }

        return QUEUE_PASSED;
    }

    if (q->count < q->capacity)
    {
        /* Fast path: room available, nobody waiting, nothing to race with. */
        status=kernel_queue_push(q, item);

        port_critical_exit();

        return status;
    }

    if (timeout_ticks == 0U)
    {
        /* Queue full and caller doesn't want to wait. */
        port_critical_exit();
        return QUEUE_FAILED;
    }

    /* 
        Queue is full: block the calling task until space frees up or timeout expires.
        tick_counter now holds an ABSOLUTE deadline tick, not a countdown. 
    */
    current_task->timed_out = 0U;
    current_task->wakeup_tick = kernel_get_tick_count() + timeout_ticks;
    current_task->current_wait_queue = &q->senders_waiting;
    current_task->pending_item = (void *)item;   /* our source buffer, for whoever wakes us */
    current_task->state = TASK_STATE_BLOCKED;
    task_queue_push_task(&q->senders_waiting, current_task);

    port_critical_exit();

    /* Give up the CPU; we resume here once woken by a receiver or timed out. */
    port_trigger_context_switch();

    port_critical_enter();

    current_task->pending_item = NULL;

    if (current_task->timed_out)
    {
        port_critical_exit();
        return QUEUE_TIMED_OUT;
    }

    /*
        Whoever woke us (kernel_queue_receive()'s fast path) already
        copied our item into the buffer for us -- nothing left to do here.
    */
    port_critical_exit();

    return QUEUE_PASSED;
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
    
    bool yield_requested=false;
    if (q->count > 0U)
    {
        /* Fast path: data available, no blocking needed. */
        status=kernel_queue_pop(q, item);

        /*
            A slot just freed up. If a sender is already blocked waiting
            for exactly this slot, refill the buffer from their pending
            item right now, in this same critical section -- otherwise a
            third task's fast-path send() could steal the freed slot
            before the waiting sender gets to run.
        */
        if (!task_queue_empty(&q->senders_waiting))
        {
            TCB_t *woken = task_queue_pop_task(&q->senders_waiting);
            (void)kernel_queue_push(q, woken->pending_item);
            woken->current_wait_queue = NULL;
            woken->state = TASK_STATE_READY;

            if(woken->priority < current_task->priority){
                yield_requested=true;
            }
        }

        port_critical_exit();

        if(yield_requested)
        {
            port_trigger_context_switch();
        }

        return status;
    }

    if (timeout_ticks == 0U)
    {
        /* Queue empty and caller doesn't want to wait. */
        port_critical_exit();
        return QUEUE_FAILED;
    }

    /* Queue is empty: block the calling task until data arrives or timeout expires.
       wakeup_tick now holds an ABSOLUTE deadline tick, not a countdown. */
    current_task->timed_out = 0U;
    current_task->wakeup_tick = kernel_get_tick_count() + timeout_ticks;
    current_task->current_wait_queue = &q->receivers_waiting;
    current_task->pending_item = item;   /* our destination buffer, for whoever wakes us */
    current_task->state = TASK_STATE_BLOCKED;
    task_queue_push_task(&q->receivers_waiting, current_task);

    port_critical_exit();

    /* Give up the CPU; we resume here once woken (by a sender) or timed out. */
    port_trigger_context_switch();

    port_critical_enter();

    current_task->pending_item = NULL;

    if (current_task->timed_out)
    {
        port_critical_exit();
        return QUEUE_TIMED_OUT;
    }

    /*
        Whoever woke us (kernel_queue_send()'s direct hand-off) already
        copied the item into *item for us -- nothing left to do here.
    */
    port_critical_exit();
    return QUEUE_PASSED;
}

queue_status_t kernel_queue_send_from_isr(kernel_queue_t *q, const void *item)
{
    if ((q == NULL) || (item == NULL))
    {
        return QUEUE_FAILED;
    }

    bool yield_requested=false;

    /* Direct hand-off, same reasoning as the task-context fast path. */
    if (!task_queue_empty(&q->receivers_waiting))
    {
        TCB_t *woken = task_queue_pop_task(&q->receivers_waiting);

        memcpy(woken->pending_item, item, q->item_size);
        woken->current_wait_queue = NULL;
        woken->state = TASK_STATE_READY;

        if ((current_task != NULL) && (woken->priority < current_task->priority))
        {
            yield_requested=true;
        }

        kernel_yield_from_isr(yield_requested);
        return QUEUE_PASSED;
    }

    /* ISR-safe API: Never block. If the queue is full, return immediately */
    if (q->count >= q->capacity)
    {
        return QUEUE_FAILED;
    }

    (void)kernel_queue_push(q, item);

    return QUEUE_PASSED;
}

queue_status_t kernel_queue_receive_from_isr(kernel_queue_t *q, void *item)
{
    if ((q == NULL) || (item == NULL))
    {
        return QUEUE_FAILED;
    }

    /* ISR-safe API: Never block. If the queue is empty, return immediately */
    if (q->count == 0U)
    {
        return QUEUE_FAILED;
    }

    (void)kernel_queue_pop(q, item);

    /* A slot just freed -- refill it from a waiting sender's pending item
       right now, same reasoning as the task-context fast path. */
    bool yield_requested=false;
    if (!task_queue_empty(&q->senders_waiting))
    {
        TCB_t *woken = task_queue_pop_task(&q->senders_waiting);

        (void)kernel_queue_push(q, woken->pending_item);
        woken->current_wait_queue = NULL;
        woken->state = TASK_STATE_READY;

        /* Request a context switch if the awakened task has a higher priority */
        if ((current_task != NULL) && (woken->priority < current_task->priority))
        {
            yield_requested=true;
        }
    }
    kernel_yield_from_isr(yield_requested);

    return QUEUE_PASSED;
}