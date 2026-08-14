#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "../Port/Cortex_M4/Inc/port.h" 
#include "task.h"
#include "queue.h"

#define TICK_RATE_HZ      1000UL
#define TIME_SLICE_TICKS  5U

void     kernel_init(void);
void     kernel_start(void) __attribute__((noreturn));
void     kernel_yield(void);
void     kernel_yield_from_isr(uint8_t higher_priority_task_woken);
void     kernel_update_ticks(void);
uint32_t kernel_get_tick_count(void);
void     scheduler_select_next_task(void);
void task_delay(uint32_t delay_ticks);
void task_delay_until(uint32_t *previous_wake_tick, uint32_t period);
/* Converts system ticks to milliseconds */
uint32_t kernel_ticks_to_ms(uint32_t ticks);
/* Converts milliseconds to system ticks */
uint32_t kernel_ms_to_ticks(uint32_t ms);
extern TCB_t *current_task;
extern TCB_t *next_task;

#endif
