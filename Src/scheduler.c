#include "../Inc/scheduler.h"
#include "../Inc/queue.h"
#include <stddef.h>
#include <stdint.h>

TCB_t *current_task = NULL;
TCB_t *next_task     = NULL;
 
static int8_t          current_task_id = -1;
/*
 * Global Monotonic Tick Counter
 * At TICK_RATE_HZ = 1000, this 32-bit counter will overflow back to 0
 * every 49.71 days. System delays and timeouts must use unsigned subtraction
 * (current_tick - start_tick) to safely calculate elapsed time across wraparounds.
 */
static volatile uint32_t g_tick_count = 0;
volatile uint32_t critical_nesting = 0;

void kernel_init(void) {
    task_pool_init();
    idle_task_create();
 
    // 1ms tick
    SysTick_LOAD = (SYSTEM_CLOCK_HZ / TICK_RATE_HZ) - 1;
    SysTick_VAL  = 0;
    SysTick_CTRL = (1 << 2)   // CLKSOURCE 
                 | (1 << 1)   // SysTick interrupt 
                 | (1 << 0);  // ENABLE 
 
    // PendSV at the lowest priority 
    SCB_SHPR3 |= (0xFF << 16);
}

static TCB_t *pick_next_task(void)
{
    uint8_t count = task_count();

    if (count == 0) {
        return NULL;
    }

    uint16_t best_priority = 256;
    int8_t best_id = -1;

   
    for (uint8_t offset = 1; offset <= count; offset++) {

        uint8_t id = (uint8_t)((current_task_id + offset) % count);

       
        if (id == 0)
            continue;

        TCB_t *t = task_get(id);

        if ((t->state == TASK_STATE_READY ||
             t->state == TASK_STATE_RUNNING) &&
            (t->priority < best_priority)) {

            best_priority = t->priority;
            best_id = id;
        }
    }


    if (best_id < 0) {
        current_task_id = 0;
        return task_get(0);
    }

    current_task_id = best_id;
    return task_get(best_id);
}

void scheduler_select_next_task(void) {
    //if task is running and completed it is now send back to ready state
    if ((current_task != NULL) && (current_task->state == TASK_STATE_RUNNING)) {
        current_task->state = TASK_STATE_READY;
    }
 
    next_task = pick_next_task();
 
    if (next_task != NULL) {
        next_task->state = TASK_STATE_RUNNING;
        next_task->tick_counter = TIME_SLICE_TICKS; 
        //tick counter (10,9,8,7... 0) -> (again it can start new task from new tick value/time)  
    }
}

void SysTick_Handler(void) {
    g_tick_count++;
 
    if (current_task != NULL) {
        //if task has time remaining if yess then decreament it 
        if (current_task->tick_counter > 0) {
            current_task->tick_counter--;
        }
        //if no time is left perform a context switch
        if (current_task->tick_counter == 0) {
            /* Request a context switch. PendSV runs at the lowest priority,
             * so it's always safe to set this pending bit from any ISR. */
            SCB_ICSR = ICSR_PENDSVSET;
        }
    }
 
    kernel_update_ticks();
}

void kernel_update_ticks(void) {
    g_tick_count++;
    uint8_t task_woken = 0;

    for (uint8_t i = 0; i < total_tasks; i++) {
        if (task_table[i].state == TASK_STATE_BLOCKED && task_table[i].tick_counter > 0) {
            task_table[i].tick_counter--;
            if (task_table[i].tick_counter == 0) {
                task_table[i].state = TASK_STATE_READY;
                task_woken = 1;
            }
        }
    }

    if (task_woken) {
        SCB_ICSR = ICSR_PENDSVSET;
    }
}

uint32_t kernel_get_tick_count(void) {
    return g_tick_count;
}

void kernel_yield(void) {
    SCB_ICSR = ICSR_PENDSVSET;
    __asm volatile ("dsb" ::: "memory");
    __asm volatile ("isb" ::: "memory");
}

__attribute__((naked)) void PendSV_Handler(void) {
    __asm volatile (
        "cpsid   i                          \n"
        "mrs     r0, psp                    \n"
        "cbz     r0, save_done              \n"
 
        "stmdb   r0!, {r4-r11}              \n" /* save R4-R11 to old task's stack */
        "ldr     r1, =current_task          \n"
        "ldr     r1, [r1]                   \n"
        "str     r0, [r1]                   \n" /* current_task->sp = r0 (sp is 1st member) */
 
        "save_done:                         \n"
        "push    {lr}                       \n"
        "bl      scheduler_select_next_task \n"
        "pop     {lr}                       \n"
 
        "ldr     r1, =current_task          \n"
        "ldr     r2, =next_task             \n"
        "ldr     r2, [r2]                   \n"
        "str     r2, [r1]                   \n" /* current_task = next_task        */
 
        "ldr     r0, [r2]                   \n" /* r0 = next_task->sp              */
        "ldmia   r0!, {r4-r11}              \n" /* restore R4-R11 for the new task */
        "msr     psp, r0                    \n"
 
        "ldr     lr, =0xFFFFFFFD            \n" /* force: Thread mode, use PSP     */
        "cpsie   i                          \n"
        "bx      lr                         \n"
    );
}

void kernel_start(void) {
    //kernel_init();
 
    if (task_count() == 0U) {
        while (1) { //cpu waits here instead of crashing till tasks are assigned 
        }
    }
 
    current_task_id = -1;
    current_task    = NULL; //  first context switch: no previous task to save

    __asm volatile ("movs r0, #0");
    __asm volatile ("msr psp, r0");
 
    SCB_ICSR = ICSR_PENDSVSET;
    __asm volatile ("cpsie i" ::: "memory");
    __asm volatile ("isb" ::: "memory");
 
    while (1) { // safety net if pendsv is not called or it fails 
    }
}


void task_delay(uint32_t delay_ticks) {
    if (delay_ticks > 0 && current_task != NULL) {
        CRITICAL_SECTION_ENTER();

        current_task->tick_counter = delay_ticks;
        current_task->state = TASK_STATE_BLOCKED;

        CRITICAL_SECTION_EXIT();

        /* Voluntarily yield the CPU since we are now asleep */
        kernel_yield();
    }
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