#include "../Inc/task.h"
#include "../Inc/scheduler.h"
#include <stdint.h>
#include <string.h>

TCB_t task_table[MAX_TASKS];
uint8_t total_tasks = 0;

static uint32_t stack_pool[KERNEL_STACK_POOL_SIZE] __attribute__((aligned(8)));
static uint32_t next_free_stack_word = 0;

#define IDLE_STACK_WORDS   32   //minimum number of words
#define IDLE_PRIORITY      0xFF //least prior

static void idle_task_function(void *arg) {
    (void)arg;  
    for (;;) {
        port_idle();  
    }
}

task_create_status_t idle_task_create(void) {
    return kernel_task_create(idle_task_function, NULL, IDLE_PRIORITY, IDLE_STACK_WORDS, 0U);
}

void task_pool_init(void) {
    // Replaces memset: manually zero out the task_table memory
    uint8_t *ptr = (uint8_t *)task_table;
    for (uint32_t i = 0; i < sizeof(task_table); i++) {
        ptr[i] = 0;
    }
    
    total_tasks = 0;
    next_free_stack_word = 0;
}


task_create_status_t kernel_task_create(TaskFunction_t fn, void *arg, uint8_t priority, uint32_t stack_words, uint32_t period) {
    if (fn == NULL || stack_words < TASK_STACK_MIN_WORDS){
        return TASK_CREATE_ERR_BAD_ARGS;
    }
    if (total_tasks >= MAX_TASKS) {
        return TASK_CREATE_ERR_MAX_TASKS;
    }
 
    // Enforce 8-byte stack alignment 
    if (stack_words & 1U) {
        stack_words++;
    }
 
    task_create_status_t err = TASK_CREATE_OK;
 
    port_critical_enter();
 
    if ((next_free_stack_word + stack_words) > KERNEL_STACK_POOL_SIZE) {
        err = TASK_CREATE_ERR_NO_MEMORY;
    } else {
        uint32_t *stack_base = &stack_pool[next_free_stack_word];
        uint32_t *stack_top = stack_base + stack_words; /* exclusive top; stack grows down */
        uint8_t id = total_tasks;
 
        next_free_stack_word += stack_words;
 
        task_table[id].sp = port_stack_init(stack_top, fn, arg);
        task_table[id].task_function = fn;
        task_table[id].arg = arg;
        task_table[id].priority = priority;
        task_table[id].base_priority = priority;
        task_table[id].period = period;
        task_table[id].state = TASK_STATE_READY;
        task_table[id].tick_counter  = 0;
        task_table[id].stack_base = stack_base;
        task_table[id].stack_words = stack_words;
        task_table[id].owned_mutexes = NULL;
        task_table[id].blocked_on = NULL;
        task_table[id].current_wait_queue = NULL;
        task_table[id].timed_out = 0U;
        task_table[id].next = NULL;
 
        total_tasks++;
    }
 
    port_critical_exit();
    return err;
}

TCB_t *task_get_TCB(uint8_t id) {
    if (id >= total_tasks) {
        return NULL;
    }
    return &task_table[id];
}

int8_t task_get_id(const TCB_t *task)
{
    if (task == NULL)
    {
        return -1;
    }

    if (task < task_table || task >= (task_table + total_tasks))
    {
        return -1;
    }

    return (int8_t)(task - task_table);
}

uint8_t kernel_task_count(void) {
    return total_tasks;
}
