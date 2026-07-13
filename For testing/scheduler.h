#include <stdint.h>

#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_TASKS           8
#define KERNEL_STACK_POOL_SIZE 2048 /* Total pool size: 2048 words (8 KB total) */

/* Task States Definitions */
typedef enum {
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED,
    TASK_STATE_SUSPENDED
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
#ifdef UNIT_TEST
/* Host build: no real interrupts to mask, stub these out */
#define CRITICAL_SECTION_ENTER()   ((void)0)
#define CRITICAL_SECTION_EXIT()    ((void)0)
#else
/* CPSID i: Disables all configurable interrupts/exceptions immediately */
#define CRITICAL_SECTION_ENTER()   __asm volatile ("CPSID i" : : : "memory")
/* CPSIE i: Enables interrupts back to normal operational state */
#define CRITICAL_SECTION_EXIT()    __asm volatile ("CPSIE i" : : : "memory")
#endif

/* ======================================================================== */
/* ATOMIC RING-QUEUE STRUCTURE & PROTOTYPES                                 */
/* ======================================================================== */

typedef struct {
    uint8_t *buffer;       /* Pointer to externally allocated array (Flexible Size) */
    uint16_t capacity;     /* Dynamic maximum capacity of this specific queue */
    uint16_t head;         /* Points to the oldest element (for pop) */
    uint16_t tail;         /* Points to the next free slot (for push) */
    uint16_t count;        /* Explicit tracking of current elements (Fixes overflow logic) */
    uint8_t overflow_flag; /* Maintained for legacy/error tracking flags */
} RingQueue_t;

/* Queue API Function Prototypes */
void kernel_queue_init(RingQueue_t *q, uint8_t *buf, uint16_t cap);
uint8_t kernel_queue_push(RingQueue_t *q, uint8_t data);
uint8_t kernel_queue_pop(RingQueue_t *q, uint8_t *data);

/* Core Scheduler API Function Prototypes */
void kernel_init(void);
uint8_t kernel_task_create(void (*fn)(void *), void *arg, uint8_t priority, uint32_t stack_size);
uint32_t* kernel_stack_init(uint32_t *stack_top, void (*fn)(void *), void *arg);
void kernel_start(void);
void kernel_yield(void);
void kernel_update_ticks(void);

// scheduler.h additions needed for host testing (guard with UNIT_TEST):
#ifdef UNIT_TEST
void kernel_test_reset(void);
extern TCB_t task_table[MAX_TASKS];
extern int8_t current_task_id; 
extern uint8_t total_tasks;
extern uint32_t stack_pool[KERNEL_STACK_POOL_SIZE];
extern uint32_t next_free_stack_word;
#endif

#endif /* SCHEDULER_H */
