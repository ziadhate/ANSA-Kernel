#include "scheduler.h"
#include <string.h>

#ifdef UNIT_TEST
void kernel_test_reset(void) {
    total_tasks = 0;
    next_free_stack_word = 0;
    current_task_id = -1;
    memset(task_table, 0, sizeof(task_table));
}
#endif

/* Bare-Metal Core Register Definitions (Cortex-M4 SysTick) */
#define SysTick_BASE        (0xE000E010UL)
#define SysTick_CTRL        (*(volatile uint32_t *)(SysTick_BASE + 0x00))
#define SysTick_LOAD        (*(volatile uint32_t *)(SysTick_BASE + 0x04))
#define SysTick_VAL         (*(volatile uint32_t *)(SysTick_BASE + 0x08))
#define SYSTEM_CLOCK_HZ     16000000UL

/* Global Variables */
TCB_t task_table[MAX_TASKS];         
int8_t current_task_id = -1; 
uint8_t total_tasks = 0;     

/* ======================================================================== */
/* DAY 15: DYNAMIC STATIC STACK ALLOCATION                                  */
/* ======================================================================== */
/* We pre-allocate a massive chunk of RAM, then distribute it to tasks      */
/* CHECKLIST: Ensure 8-byte stack alignment using GCC attribute             */
uint32_t stack_pool[KERNEL_STACK_POOL_SIZE] __attribute__((aligned(8)));
uint32_t next_free_stack_word = 0; /* Tracks where the next stack begins */

void kernel_init(void) {
    SysTick_LOAD = (SYSTEM_CLOCK_HZ / 1000) - 1;
    SysTick_VAL = 0;
    SysTick_CTRL = (1UL << 2) | (1UL << 1) | (1UL << 0);
}

/* ======================================================================== */
/* CHECKLIST: Implement kernel_stack_init()                                 */
/* ======================================================================== */
uint32_t* kernel_stack_init(uint32_t *stack_top, void (*fn)(void *), void *arg) {
    uint32_t *sp = stack_top;
    
    *(--sp) = 0x01000000;       /* xPSR: Set Thumb bit */
    *(--sp) = (uint32_t)fn;     /* PC: Task entry point */
    *(--sp) = 0xFFFFFFFE;       /* LR/EXC_RETURN: Return booby trap */
    *(--sp) = 0x12121212;       /* R12 */
    *(--sp) = 0x03030303;       /* R3 */
    *(--sp) = 0x02020202;       /* R2 */
    *(--sp) = 0x01010101;       /* R1 */
    *(--sp) = (uint32_t)arg;    /* R0: First argument (from void *arg) */

    /* Fake the Software Context Stack Frame */
    for (int i = 0; i < 8; i++) {
        *(--sp) = 0x00000000;   /* R4-R11 */
    }
    
    return sp;
}

/* ======================================================================== */
/* DAY 15: CUSTOM STACK SIZES IN TASK CREATION ENGINE                       */
/* ======================================================================== */
uint8_t kernel_task_create(void (*fn)(void *), void *arg, uint8_t priority, uint32_t stack_size) {
    if (total_tasks >= MAX_TASKS) {
        return 1; /* Max tasks reached */
    }
    
    CRITICAL_SECTION_ENTER();
    
    /* CHECKLIST: Enforce 8-byte alignment (must be even number of 32-bit words) */
    if (stack_size % 2 != 0) {
        stack_size++; 
    }
    
    /* 1. Ensure we have enough memory left in the pool */
    if ((next_free_stack_word + stack_size) > KERNEL_STACK_POOL_SIZE) {
        CRITICAL_SECTION_EXIT();
        return 2; /* Out of stack memory! */
    }
    
    /* 2. Carve out the requested memory */
    uint32_t *stack_top = &stack_pool[next_free_stack_word + stack_size];
    
    /* Move the allocator forward for the next task that gets created */
    next_free_stack_word += stack_size;
    
    /* 3. Initialize the stack frame using our new dedicated function */
    uint32_t *sp = kernel_stack_init(stack_top, fn, arg);

    /* 4. Initialize the TCB */
    task_table[total_tasks].sp = sp;
    task_table[total_tasks].task_function = fn;
    task_table[total_tasks].arg = arg;
    task_table[total_tasks].priority = priority;
    task_table[total_tasks].state = TASK_STATE_READY;
    task_table[total_tasks].tick_counter = 0;
    
    total_tasks++;
    
    CRITICAL_SECTION_EXIT();
    return 0;
}

void kernel_start(void) {
    kernel_init();
    
    while(1) {
        if (total_tasks > 0) {
            current_task_id = (current_task_id + 1) % total_tasks;
            
            if (task_table[current_task_id].state == TASK_STATE_READY) {
                task_table[current_task_id].state = TASK_STATE_RUNNING;
                /* Pass the stored argument to the function when it runs */
                task_table[current_task_id].task_function(task_table[current_task_id].arg);
                task_table[current_task_id].state = TASK_STATE_READY;
            }
        }
    }
}

/* DAY 8 Queue Functions (Unchanged) */
void kernel_queue_init(RingQueue_t *q, uint8_t *buf, uint16_t cap){
    CRITICAL_SECTION_ENTER();
    q->buffer = buf;
    q->capacity = cap;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->overflow_flag = 0;
    CRITICAL_SECTION_EXIT();
}

uint8_t kernel_queue_push(RingQueue_t *q, uint8_t data) {
    CRITICAL_SECTION_ENTER();
    /* Fixed overflow check utilizing full capacity without wasting a slot */
    if (q->count == q->capacity) {
        q->overflow_flag = 1;
        CRITICAL_SECTION_EXIT();         // restores exactly the previous state (nesting-safe)
        return 0; 
    }

    q->buffer[q->tail] = data;
    q->tail = (q->tail + 1) % q->capacity; /* Dynamic wrap-around based on queue capacity */
    q->count++;
    CRITICAL_SECTION_EXIT();
    return 0; 
}

uint8_t kernel_queue_pop(RingQueue_t *q, uint8_t *data) {
    CRITICAL_SECTION_ENTER();
    /* Direct explicit check for empty queue conditions */
    if (q->count == 0) {
        CRITICAL_SECTION_EXIT();
        return 0; 
    }

    *data = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity; /* Dynamic wrap-around based on queue capacity */
    q->count--;
    CRITICAL_SECTION_EXIT();
    return 0;
}

void kernel_yield(void) {}
void kernel_update_ticks(void) {}
