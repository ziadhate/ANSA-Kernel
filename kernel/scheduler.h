#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

/* Task States Definitions */
typedef enum {
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED
} TaskState_t;

/* ======================================================================== */
/* Task Control Block (TCB) Structure */
typedef struct {
    uint32_t *sp;                  /* Stack Pointer: Tracks the task's context */
    void (*task_function)(void *); /* Pointer to the task's main loop (Now takes void *arg) */
    void *arg;                     /* Store the argument to pass to the task */
    uint8_t priority;              /* Scheduling priority */
    TaskState_t state;             /* Current state of the task */
    uint32_t tick_counter;         /* Used for delays and timeouts */
} TCB_t;

/* ======================================================================== */
/* DAY 8 CRITICAL SECTION PRIMITIVES (PRIMASK MANIPULATION)                 */
/* ======================================================================== */
/* CPSID i: Disables all configurable interrupts/exceptions immediately */
#define CRITICAL_SECTION_ENTER()   __asm volatile ("CPSID i" : : : "memory")

/* CPSIE i: Enables interrupts back to normal operational state */
#define CRITICAL_SECTION_EXIT()    __asm volatile ("CPSIE i" : : : "memory")

/* ======================================================================== */
/* ATOMIC RING-QUEUE STRUCTURE & PROTOTYPES                                 */
/* ======================================================================== */
#define QUEUE_MAX_SIZE 16

typedef struct {
    uint8_t buffer[QUEUE_MAX_SIZE];
    uint16_t head;
    uint16_t tail;
    uint8_t overflow_flag; /* Catching full state boundary */
} RingQueue_t;

/* Queue API Function Prototypes */
void kernel_queue_init(RingQueue_t *q);
uint8_t kernel_queue_push(RingQueue_t *q, uint8_t data);
uint8_t kernel_queue_pop(RingQueue_t *q, uint8_t *data);

/* Core Scheduler API Function Prototypes */
void kernel_init(void);
uint8_t kernel_task_create(void (*fn)(void *), void *arg, uint8_t priority, uint32_t stack_size);
void kernel_start(void);
void kernel_yield(void);
void kernel_update_ticks(void);

#endif /* SCHEDULER_H */