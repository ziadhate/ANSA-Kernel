#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "../Port/Cortex_M4/Inc/port.h" 
#include "task.h"
#include "queue.h"

/* Kernel tick frequency */
#define TICK_RATE_HZ      1000UL

/* Initializes the kernel and its core components. This function must be called before starting the scheduler */
void kernel_init(void);

/*
    Starts the kernel scheduler.
    This function does not return because control is transferred to the scheduler and subsequently to the tasks.
*/
void kernel_start(void) __attribute__((noreturn));

/* Voluntarily gives up the CPU. The scheduler is invoked to select the next task to execute */
void kernel_yield(void);

/* ISR-safe version of kernel_yield(). Requests a context switch after the ISR if a higher-priority task has been woken */
void kernel_yield_from_isr(bool higher_priority_task_woken);

/*  Updates the kernel time base.
    Typically called from the system tick interrupt to increment the global tick counter and process time-related task events.
 */
void kernel_update_ticks(void);

/* Returns the current system tick count */
uint32_t kernel_get_tick_count(void);

/* 
    Selects the highest-priority task that is ready to run
*/
void scheduler_select_next_task(void);

/*
    Blocks the current task for the specified number of system ticks.
    delay_ticks : Number of ticks for which the task must remain blocked.
*/
void task_delay(uint32_t delay_ticks);

/*
    Blocks the current task until the next periodic release point.
    previous_wake_tick : Previous scheduled wake-up tick
    period : Task period in system ticks
    
    This function is intended for periodic tasks and helps maintain a regular execution period.
*/
void task_delay_until(uint32_t *previous_wake_tick, uint32_t period);


/* Converts system ticks to milliseconds */
uint32_t kernel_ticks_to_ms(uint32_t ticks);

/* Converts milliseconds to system ticks */
uint32_t kernel_ms_to_ticks(uint32_t ms);

/* Pointer to the TCB of the task currently executing */
extern TCB_t *current_task;

/* Pointer to the TCB selected by the scheduler for the next context switch */
extern TCB_t *next_task;

#endif
