#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "task.h"

#define SysTick_BASE    (0xE000E010UL)
#define SysTick_CTRL    (*(volatile uint32_t *)(SysTick_BASE + 0x00))
#define SysTick_LOAD    (*(volatile uint32_t *)(SysTick_BASE + 0x04))
#define SysTick_VAL     (*(volatile uint32_t *)(SysTick_BASE + 0x08))

#define SCB_ICSR        (*(volatile uint32_t *)(0xE000ED04UL))
#define SCB_SHPR3       (*(volatile uint32_t *)(0xE000ED20UL))
#define ICSR_PENDSVSET  (1UL << 28)

#define SYSTEM_CLOCK_HZ   16000000UL
#define TICK_RATE_HZ      1000UL
#define TIME_SLICE_TICKS  5U

extern volatile uint32_t critical_nesting;

#define CRITICAL_SECTION_ENTER() do {               \
        __asm volatile ("cpsid i" ::: "memory");    \
        critical_nesting++;                         \
    } while (0)

#define CRITICAL_SECTION_EXIT() do {                \
        if (critical_nesting > 0U) {                \
            critical_nesting--;                     \
        }                                           \
        if (critical_nesting == 0U) {               \
            __asm volatile ("cpsie i" ::: "memory");\
        }                                           \
    } while (0)

typedef struct {
    uint8_t *buffer;
    uint16_t capacity;
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    uint8_t  overflow_flag;
} RingQueue_t;

void     kernel_queue_init(RingQueue_t *q, uint8_t *buf, uint16_t cap);
uint8_t  kernel_queue_push(RingQueue_t *q, uint8_t data);
uint8_t  kernel_queue_pop(RingQueue_t *q, uint8_t *data);

void     kernel_init(void);
void     kernel_start(void) __attribute__((noreturn));
void     kernel_yield(void);
void     kernel_update_ticks(void);
uint32_t kernel_get_tick_count(void);
void     scheduler_select_next_task(void);

extern TCB_t *current_task;
extern TCB_t *next_task;

#endif