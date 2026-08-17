#include "../Inc/scheduler.h"
#include "../Inc/queue.h"
#include <stddef.h>
#include <stdint.h>

TCB_t *current_task = NULL;
TCB_t *next_task = NULL;
 
static int8_t current_task_id = -1;
/*
 * Global Monotonic Tick Counter
 * At TICK_RATE_HZ = 1000, this 32-bit counter will overflow back to 0
 * every 49.71 days. System delays and timeouts must use unsigned subtraction
 * (current_tick - start_tick) to safely calculate elapsed time across wraparounds.
 */
static volatile uint32_t g_tick_count = 0;

void kernel_init(void)
{
    task_pool_init();
}

static TCB_t *pick_next_task(void)
{
    uint8_t count = kernel_task_count();

    if (count == 0) {
        return NULL;
    }

    /* Idle task is always the last task created */
    uint8_t idle_task_id = count - 1;

    uint16_t best_priority = 256;
    int8_t best_id = -1;

    /* Search all runnable tasks except the Idle Task */
    for (uint8_t offset = 1; offset <= count; offset++) {

        uint8_t id = (uint8_t)((current_task_id + offset) % count);

        /* Skip Idle Task during normal scheduling */
        if (id == idle_task_id){
            continue;
        }

        TCB_t *t = task_get_TCB(id);

        if ((t->state == TASK_STATE_READY) && (t->priority < best_priority)) {
            best_priority = t->priority;
            best_id = id;
        }
    }

    /* If no runnable user task exists, run the Idle Task */
    if(best_id < 0){
        current_task_id = idle_task_id;
        return task_get_TCB(idle_task_id);
    }

    current_task_id = best_id;
    return task_get_TCB(best_id);
}

void scheduler_select_next_task(void) {
    //if task is running and completed it is now sent back to ready state
    if ((current_task != NULL) && (current_task->state == TASK_STATE_RUNNING)) {
        current_task->state = TASK_STATE_READY;
    }
 
    next_task = pick_next_task();
 
    if (next_task != NULL && next_task->state==TASK_STATE_READY){
        next_task->state = TASK_STATE_RUNNING;
    }
}

void kernel_update_ticks(void)
{
    g_tick_count++;
    uint32_t now = kernel_get_tick_count();
    bool task_woken = false;

    for (uint8_t i=0U; i<total_tasks; i++)
    {
        TCB_t *task = &task_table[i];

        if (task->state != TASK_STATE_BLOCKED)
        {
            continue;
        }

        if ((int32_t)(now-task->wakeup_tick) >= 0)
        {
            if (task->current_wait_queue != NULL)
            {
                (void)task_queue_remove(task->current_wait_queue, task);
                task->current_wait_queue = NULL;
                task->timed_out = 1U;
            }

            task->state = TASK_STATE_READY;
            task_woken = true;
        }
    }

    if (task_woken)
    {
        port_trigger_context_switch();
    }
}

uint32_t kernel_get_tick_count(void) {
    return g_tick_count;
}

void kernel_yield(void) {
    port_critical_enter();
    if ((current_task != NULL) && (current_task->state==TASK_STATE_RUNNING)){
        current_task->state=TASK_STATE_READY;
    }
    port_critical_exit();

    port_trigger_context_switch();
}

void kernel_start(void) {
    if (kernel_task_count() == 0U) {
        while (1) {}
    }

    /* Create Idle Task LAST so it reliably occupies (total_tasks - 1) */
    idle_task_create();

    /* Initialize SysTick to 1ms */
    port_systick_init(TICK_RATE_HZ);

    current_task_id = -1;
    current_task = NULL;

    port_start_first_task();
 
    while (1) {}
}

void kernel_yield_from_isr(bool higher_priority_task_woken)
{
    if (higher_priority_task_woken)
    {
        port_trigger_context_switch();
    }
}


void task_delay(uint32_t delay_ticks) {
    if (delay_ticks == 0U)
    {
        return;
    }
    port_critical_enter();

    current_task->wakeup_tick = kernel_get_tick_count()+delay_ticks;     
    current_task->state = TASK_STATE_BLOCKED;

    port_critical_exit();

    port_trigger_context_switch();
}

void task_delay_until(uint32_t *previous_wake_tick, uint32_t period_ticks)
{
    if (previous_wake_tick == NULL || period_ticks == 0U)
    {
        return;
    }

    port_critical_enter();

    uint32_t current_tick = kernel_get_tick_count();
    uint32_t next_wake_tick = *previous_wake_tick + period_ticks;

    /*
        If the task has already missed its next release, it is not blocked
    */
    if ((int32_t)(current_tick - next_wake_tick) >= 0)
    {
        *previous_wake_tick = current_tick;
        port_critical_exit();
        return;
    }

    current_task->wakeup_tick = next_wake_tick;
    current_task->state = TASK_STATE_BLOCKED;

    *previous_wake_tick = next_wake_tick;

    port_critical_exit();
    port_trigger_context_switch();
}

/* Converts system ticks to milliseconds */
uint32_t kernel_ticks_to_ms(uint32_t ticks) {
    /* Multiplies first to prevent integer division truncation */
    return (ticks * 1000UL) / TICK_RATE_HZ;
}

/* Converts milliseconds to system ticks */
uint32_t kernel_ms_to_ticks(uint32_t ms) {
    return (ms * TICK_RATE_HZ) / 1000UL;
}
