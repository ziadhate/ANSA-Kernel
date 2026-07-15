#ifndef TASK_H
#define TASK_H

#include<stdint.h>
#include<stddef.h>

typedef enum {
    TASK_STATE_READY,                       //next task to run
    TASK_STATE_RUNNING,                     //currently running
    TASK_STATE_BLOCKED,                     //waiting task
    TASK_STATE_SUSPENDED,                   //never scheduled
} TaskState_t;

typedef void (*TaskFunction_t)(void *arg);

typedef struct{
    uint32_t *sp;
    TaskFunction_t task_function;
    void (*arg);
    uint8_t priority;
    TaskState_t state;
    uint32_t tick_counter;
    uint32_t *stack_base;                   //starting memory of task
    uint32_t stack_words;                   //size of stack 32 words = 128bytes (1 word = 4 bytes)

} TCB_t;

#define MAX_TASKS 8
#define KERNEL_STACK_POOL_SIZE  2048
#define TASK_STACK_MIN_WORDS    32

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
task_create_status_t task_create(TaskFunction_t fn, void *arg, uint8_t priority, uint32_t stack_words);
uint32_t *kernel_stack_init(uint32_t *stack_top, TaskFunction_t fn, void *arg);
TCB_t  *task_get(uint8_t id);
uint8_t task_count(void);


#endif 