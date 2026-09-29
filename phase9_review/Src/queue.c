#include "../Inc/queue.h"
#include "../Inc/scheduler.h"
#include "../Inc/port.h"
#include <string.h>

queue_status_t kernel_queue_init(kernel_queue_t *q, void *buffer, uint8_t capacity, uint16_t item_size)
{
    if (q == NULL || buffer == NULL || capacity == 0 || item_size == 0)
    {
        return QUEUE_INIT_FAILED;
    }

    q->buffer = (uint8_t *)buffer;
    q->capacity = capacity;
    q->item_size = item_size;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->overflow_flag = 0;

    q->senders_waiting.head = NULL;
    q->senders_waiting.tail = NULL;
    q->receivers_waiting.head = NULL;
    q->receivers_waiting.tail = NULL;

    return QUEUE_INIT_SUCCESS;
}

queue_status_t kernel_queue_push(kernel_queue_t *q, const void *data)
{
    if (q == NULL || data == NULL || q->count >= q->capacity)
    {
        return QUEUE_FAILED;
    }

    uint8_t *dest = q->buffer + (q->tail * q->item_size);
    memcpy(dest, data, q->item_size);

    q->tail = (q->tail + 1) % q->capacity;
    q->count++;

    return QUEUE_PASSED;
}

queue_status_t kernel_queue_pop(kernel_queue_t *q, void *data)
{
    if (q == NULL || data == NULL || q->count == 0)
    {
        return QUEUE_FAILED;
    }

    uint8_t *src = q->buffer + (q->head * q->item_size);
    memcpy(data, src, q->item_size);

    q->head = (q->head + 1) % q->capacity;
    q->count--;

    return QUEUE_PASSED;
}

void task_queue_push_task(task_queue_t *q, TCB_t *task_tcb)
{
    if (q == NULL || task_tcb == NULL) return;

    task_tcb->next = NULL;

    if (q->head == NULL)
    {
        q->head = task_tcb;
        q->tail = task_tcb;
        return;
    }

    /* Insert based on priority (Higher priority first) */
    if (task_tcb->priority < q->head->priority)
    {
        task_tcb->next = q->head;
        q->head = task_tcb;
        return;
    }

    TCB_t *curr = q->head;
    while (curr->next != NULL && curr->next->priority <= task_tcb->priority)
    {
        curr = curr->next;
    }

    task_tcb->next = curr->next;
    curr->next = task_tcb;

    if (task_tcb->next == NULL)
    {
        q->tail = task_tcb;
    }
}

TCB_t *task_queue_pop_task(task_queue_t *q)
{
    if (q == NULL || q->head == NULL) return NULL;

    TCB_t *task = q->head;
    q->head = q->head->next;

    if (q->head == NULL)
    {
        q->tail = NULL;
    }

    task->next = NULL;
    return task;
}

bool task_queue_empty(task_queue_t *q)
{
    return (q == NULL || q->head == NULL);
}

bool task_queue_remove(task_queue_t *q, TCB_t *task)
{
    if (q == NULL || q->head == NULL || task == NULL) return false;

    if (q->head == task)
    {
        q->head = q->head->next;
        if (q->head == NULL) q->tail = NULL;
        task->next = NULL;
        return true;
    }

    TCB_t *curr = q->head;
    while (curr->next != NULL && curr->next != task)
    {
        curr = curr->next;
    }

    if (curr->next == task)
    {
        curr->next = task->next;
        if (curr->next == NULL) q->tail = curr;
        task->next = NULL;
        return true;
    }

    return false;
}

/* Timed / Blocking Queue Send */
queue_status_t kernel_queue_send(kernel_queue_t *q, const void *item, uint32_t timeout_ticks)
{
    if (q == NULL || item == NULL) return QUEUE_FAILED;

    port_critical_enter();

    if (q->count < q->capacity)
    {
        kernel_queue_push(q, item);

        if (!task_queue_empty(&q->receivers_waiting))
        {
            TCB_t *waiting_task = task_queue_pop_task(&q->receivers_waiting);
            waiting_task->current_wait_queue = NULL;
            waiting_task->has_timeout = false;
            waiting_task->state = TASK_STATE_READY;
            port_critical_exit();
            port_trigger_context_switch();
            return QUEUE_PASSED;
        }

        port_critical_exit();
        return QUEUE_PASSED;
    }

    if (timeout_ticks == 0U)
    {
        port_critical_exit();
        return QUEUE_BUSY;
    }

    current_task->pending_item = (void *)item;
    current_task->current_wait_queue = &q->senders_waiting;
    current_task->timed_out = 0U;

    if (timeout_ticks != KERNEL_MAX_DELAY)
    {
        current_task->wakeup_tick = kernel_get_tick_count() + timeout_ticks;
        current_task->has_timeout = true;
    }
    else
    {
        current_task->has_timeout = false;
    }

    task_queue_push_task(&q->senders_waiting, current_task);
    current_task->state = TASK_STATE_BLOCKED;

    port_critical_exit();
    port_trigger_context_switch();

    if (current_task->timed_out)
    {
        return QUEUE_TIMED_OUT;
    }

    return QUEUE_PASSED;
}

/* Timed / Blocking Queue Receive */
queue_status_t kernel_queue_receive(kernel_queue_t *q, void *item, uint32_t timeout_ticks)
{
    if (q == NULL || item == NULL) return QUEUE_FAILED;

    port_critical_enter();

    if (q->count > 0)
    {
        kernel_queue_pop(q, item);

        if (!task_queue_empty(&q->senders_waiting))
        {
            TCB_t *waiting_task = task_queue_pop_task(&q->senders_waiting);
            if (waiting_task->pending_item != NULL)
            {
                kernel_queue_push(q, waiting_task->pending_item);
                waiting_task->pending_item = NULL;
            }
            waiting_task->current_wait_queue = NULL;
            waiting_task->has_timeout = false;
            waiting_task->state = TASK_STATE_READY;
            port_critical_exit();
            port_trigger_context_switch();
            return QUEUE_PASSED;
        }

        port_critical_exit();
        return QUEUE_PASSED;
    }

    if (timeout_ticks == 0U)
    {
        port_critical_exit();
        return QUEUE_BUSY;
    }

    current_task->pending_item = item;
    current_task->current_wait_queue = &q->receivers_waiting;
    current_task->timed_out = 0U;

    if (timeout_ticks != KERNEL_MAX_DELAY)
    {
        current_task->wakeup_tick = kernel_get_tick_count() + timeout_ticks;
        current_task->has_timeout = true;
    }
    else
    {
        current_task->has_timeout = false;
    }

    task_queue_push_task(&q->receivers_waiting, current_task);
    current_task->state = TASK_STATE_BLOCKED;

    port_critical_exit();
    port_trigger_context_switch();

    if (current_task->timed_out)
    {
        return QUEUE_TIMED_OUT;
    }

    return QUEUE_PASSED;
}

/* Non-blocking Try APIs */
queue_status_t kernel_queue_send_try(kernel_queue_t *q, const void *item)
{
    return kernel_queue_send(q, item, 0U);
}

queue_status_t kernel_queue_receive_try(kernel_queue_t *q, void *item)
{
    return kernel_queue_receive(q, item, 0U);
}

/* Hardened ISR APIs */
queue_status_t kernel_queue_send_from_isr(kernel_queue_t *q, const void *item, bool *pxHigherPriorityTaskWoken)
{
    if (q == NULL || item == NULL) return QUEUE_FAILED;

    uint32_t status = port_set_interrupt_mask_from_isr();

    if (q->count >= q->capacity)
    {
        q->overflow_flag = 1U;
        port_clear_interrupt_mask_from_isr(status);
        return QUEUE_BUSY;
    }

    kernel_queue_push(q, item);

    if (!task_queue_empty(&q->receivers_waiting))
    {
        TCB_t *waiting_task = task_queue_pop_task(&q->receivers_waiting);
        waiting_task->current_wait_queue = NULL;
        waiting_task->has_timeout = false;
        waiting_task->state = TASK_STATE_READY;

        if (pxHigherPriorityTaskWoken != NULL)
        {
            *pxHigherPriorityTaskWoken = true;
        }
    }

    port_clear_interrupt_mask_from_isr(status);
    return QUEUE_PASSED;
}

queue_status_t kernel_queue_receive_from_isr(kernel_queue_t *q, void *item, bool *pxHigherPriorityTaskWoken)
{
    if (q == NULL || item == NULL) return QUEUE_FAILED;

    uint32_t status = port_set_interrupt_mask_from_isr();

    if (q->count == 0)
    {
        port_clear_interrupt_mask_from_isr(status);
        return QUEUE_BUSY;
    }

    kernel_queue_pop(q, item);

    if (!task_queue_empty(&q->senders_waiting))
    {
        TCB_t *waiting_task = task_queue_pop_task(&q->senders_waiting);
        if (waiting_task->pending_item != NULL)
        {
            kernel_queue_push(q, waiting_task->pending_item);
            waiting_task->pending_item = NULL;
        }
        waiting_task->current_wait_queue = NULL;
        waiting_task->has_timeout = false;
        waiting_task->state = TASK_STATE_READY;

        if (pxHigherPriorityTaskWoken != NULL)
        {
            *pxHigherPriorityTaskWoken = true;
        }
    }

    port_clear_interrupt_mask_from_isr(status);
    return QUEUE_PASSED;
}
