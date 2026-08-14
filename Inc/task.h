#ifndef TASK_H
#define TASK_H

#include<stdint.h>
#include<stddef.h>

#define MAX_TASKS 8
#define KERNEL_STACK_POOL_SIZE  2048
#define TASK_STACK_MIN_WORDS    32

typedef enum {
    TASK_STATE_READY,                       //next task to run
    TASK_STATE_RUNNING,                     //currently running
    TASK_STATE_BLOCKED,                     //waiting task
    TASK_STATE_SUSPENDED,                   //never scheduled
} TaskState_t;

typedef void (*TaskFunction_t)(void *arg);
typedef struct task_queue_t task_queue_t;   // Forward declaration
typedef struct kernel_mutex_t kernel_mutex_t; 

typedef struct TCB_t{
    uint32_t *sp;
    TaskFunction_t task_function;
    void *arg;
    uint8_t priority;
    uint8_t base_priority;
    uint32_t period;
    TaskState_t state;
    uint32_t wakeup_tick;
    uint32_t tick_counter;
    uint32_t *stack_base;                   //starting memory of task
    uint32_t stack_words;                   //size of stack 32 words = 128bytes (1 word = 4 bytes)
    kernel_mutex_t *owned_mutexes;   // Linked list of mutexes owned by this task
    kernel_mutex_t *blocked_on
    // Added for queue support
    struct TCB_t *next; // Pointer to the next task in a waiting queue.
    task_queue_t *current_wait_queue; // // Queue on which the task is currently blocked.
    volatile uint8_t timed_out; // Indicates whether the task was unblocked due to a timeout.
} TCB_t;

/*to check functioning, means if the functions are working properly they will
return this values*/
typedef enum
{
    TASK_CREATE_OK = 0,
    TASK_CREATE_ERR_MAX_TASKS,
    TASK_CREATE_ERR_NO_MEMORY,
    TASK_CREATE_ERR_BAD_ARGS
} task_create_status_t;

void task_pool_init(void);
task_create_status_t kernel_task_create(TaskFunction_t fn, void *arg, uint8_t priority, uint32_t stack_words, uint32_t period);
TCB_t  *task_get_TCB(uint8_t id);
uint8_t kernel_task_count(void);
task_create_status_t idle_task_create(void);

extern TCB_t task_table[MAX_TASKS];
extern uint8_t total_tasks;

#endif 
