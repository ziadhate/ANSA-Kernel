#include "../Inc/semaphore.h"
#include "../Inc/scheduler.h"
#include "../Port/Cortex_M4/Inc/port.h"

queue_status_t kernel_sem_init(kernel_sem_t *sem, uint32_t initial_count, uint32_t max_count)
{
    if ((sem == NULL) || (max_count == 0U) || (initial_count > max_count))
    {
        return QUEUE_INIT_FAILED;
    }

    port_critical_enter();

    sem->count        = initial_count;
    sem->max_count    = max_count;
    sem->waiters.head = NULL;
    sem->waiters.tail = NULL;

    port_critical_exit();
    return QUEUE_INIT_SUCCESS;
}

queue_status_t kernel_sem_set_max_count(kernel_sem_t *sem, uint32_t new_max_count)
{
    if ((sem == NULL) || (new_max_count == 0U))
    {
        return QUEUE_FAILED;
    }

    port_critical_enter();

    if (new_max_count < sem->count)
    {
        port_critical_exit();
        return QUEUE_FAILED;
    }

    sem->max_count = new_max_count;

    port_critical_exit();
    return QUEUE_PASSED;
}

queue_status_t kernel_sem_take(kernel_sem_t *sem, uint32_t timeout_ticks)
{
    if (sem == NULL)
    {
        return QUEUE_FAILED;
    }

    port_critical_enter();

    /* Fast path: Token available immediately */
    if (sem->count > 0U)
    {
        sem->count--;
        port_critical_exit();
        return QUEUE_PASSED;
    }

    /* Cannot block: zero timeout requested or scheduler not ready */
    if ((timeout_ticks == 0U) || (current_task == NULL))
    {
        port_critical_exit();
        return QUEUE_FAILED;
    }

    /* Set up blocked state metadata */
    current_task->timed_out          = 0U;
    current_task->current_wait_queue = &sem->waiters;
    current_task->state              = TASK_STATE_BLOCKED;

    if (timeout_ticks == SEM_WAIT_FOREVER)
    {
        /*
         * NOTE (design limitation, not fixed here): "non-expiring" is encoded
         * as tick_counter == 0. A finite timeout can also compute to exactly
         * 0 on tick-counter wraparound (kernel_get_tick_count() + timeout_ticks
         * overflowing back to 0), which would be indistinguishable from
         * SEM_WAIT_FOREVER and cause that task to never time out. A robust
         * fix needs a dedicated "wait forever" flag on TCB_t (checked by the
         * SysTick timeout handler) rather than overloading tick_counter.
         */
        current_task->tick_counter = 0U; /* Handled as non-expiring by SysTick */
    }
    else
    {
        current_task->tick_counter = kernel_get_tick_count() + timeout_ticks;
    }

    task_queue_push_task(&sem->waiters, current_task);

    port_critical_exit();

    /* Yield CPU to next ready task */
    kernel_yield();

    /* Resumed execution after being woken or timed out */
    port_critical_enter();

    /* Case 1: Woken due to timeout */
    if (current_task->timed_out)
    {
        current_task->timed_out = 0U;

        /* Unlink task from the semaphore wait queue */
        if (current_task->current_wait_queue != NULL)
        {
            task_queue_remove(current_task->current_wait_queue, current_task);
            current_task->current_wait_queue = NULL;
        }

        port_critical_exit();
        return QUEUE_TIMED_OUT;
    }

    /* Case 2: Woken via direct handoff (already unlinked by give/give_from_isr) */
    current_task->current_wait_queue = NULL;

    port_critical_exit();
    return QUEUE_PASSED;
}

queue_status_t kernel_sem_give(kernel_sem_t *sem)
{
    if (sem == NULL)
    {
        return QUEUE_FAILED;
    }

    uint8_t higher_priority_task_woken = 0U;

    port_critical_enter();

    if (!task_queue_empty(&sem->waiters))
    {
        /* Direct handoff: Pop task without incrementing counter */
        TCB_t *woken = task_queue_pop_task(&sem->waiters);

        if (woken != NULL)
        {
            woken->current_wait_queue = NULL;
            woken->timed_out          = 0U;
            woken->state              = TASK_STATE_READY;

            if ((current_task == NULL) || (woken->priority > current_task->priority))
            {
                higher_priority_task_woken = 1U;
            }
        }
        else
        {
            /*
             * Defensive fallback: task_queue_empty() reported waiters present
             * but the pop returned NULL (queue-consistency bug elsewhere).
             * Without this fallback the give() would silently vanish - no
             * task woken AND no count increment - while still reporting
             * QUEUE_PASSED to the caller. Treat it like the "no waiters"
             * case instead of dropping the signal.
             */
            if (sem->count >= sem->max_count)
            {
                port_critical_exit();
                return QUEUE_FAILED;
            }

            sem->count++;
        }
    }
    else
    {
        if (sem->count >= sem->max_count)
        {
            port_critical_exit();
            return QUEUE_FAILED;
        }

        sem->count++;
    }

    port_critical_exit();

    /* Trigger context switch outside critical section */
    if (higher_priority_task_woken)
    {
        port_trigger_context_switch();
    }

    return QUEUE_PASSED;
}

uint32_t kernel_sem_get_count(const kernel_sem_t *sem)
{
    if (sem == NULL)
    {
        return SEM_INVALID_COUNT;
    }

    port_critical_enter();
    uint32_t count = sem->count;
    port_critical_exit();

    return count;
}

queue_status_t kernel_sem_give_from_isr(kernel_sem_t *sem, uint8_t *px_higher_priority_task_woken)
{
    if (sem == NULL)
    {
        return QUEUE_FAILED;
    }

    if (px_higher_priority_task_woken != NULL)
    {
        *px_higher_priority_task_woken = 0U;
    }

    uint32_t isr_mask = port_set_interrupt_mask_from_isr();

    if (!task_queue_empty(&sem->waiters))
    {
        TCB_t *woken = task_queue_pop_task(&sem->waiters);

        if (woken != NULL)
        {
            woken->current_wait_queue = NULL;
            woken->timed_out          = 0U;
            woken->state              = TASK_STATE_READY;

            if ((px_higher_priority_task_woken != NULL) &&
                ((current_task == NULL) || (woken->priority > current_task->priority)))
            {
                *px_higher_priority_task_woken = 1U;
            }
        }
        else
        {
            /* Same defensive fallback as kernel_sem_give() - see comment there. */
            if (sem->count >= sem->max_count)
            {
                port_clear_interrupt_mask_from_isr(isr_mask);
                return QUEUE_FAILED;
            }

            sem->count++;
        }
    }
    else
    {
        if (sem->count >= sem->max_count)
        {
            port_clear_interrupt_mask_from_isr(isr_mask);
            return QUEUE_FAILED;
        }

        sem->count++;
    }

    port_clear_interrupt_mask_from_isr(isr_mask);
    return QUEUE_PASSED;
}

queue_status_t kernel_sem_take_from_isr(kernel_sem_t *sem)
{
    if (sem == NULL)
    {
        return QUEUE_FAILED;
    }

    uint32_t isr_mask = port_set_interrupt_mask_from_isr();

    if (sem->count > 0U)
    {
        sem->count--;
        port_clear_interrupt_mask_from_isr(isr_mask);
        return QUEUE_PASSED;
    }

    port_clear_interrupt_mask_from_isr(isr_mask);
    return QUEUE_FAILED;
}

uint32_t kernel_sem_get_count_from_isr(const kernel_sem_t *sem)
{
    if (sem == NULL)
    {
        return SEM_INVALID_COUNT;
    }

    uint32_t isr_mask = port_set_interrupt_mask_from_isr();
    uint32_t count    = sem->count;
    port_clear_interrupt_mask_from_isr(isr_mask);

    return count;
}
