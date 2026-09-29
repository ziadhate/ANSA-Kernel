#ifndef TASK_H
#define TASK_H

#include<stdint.h>
#include<stddef.h>

/* Maximum number of tasks that can exist simultaneously */
#ifndef MAX_TASKS
#ifndef MAX_TASKS
#ifndef MAX_TASKS
#define MAX_TASKS 20
#endif
#endif
#endif

/* Total size of the memory pool dedicated to task stacks, in words */
#ifndef KERNEL_STACK_POOL_SIZE
#ifndef KERNEL_STACK_POOL_SIZE
#ifndef KERNEL_STACK_POOL_SIZE
#define KERNEL_STACK_POOL_SIZE  8192U
#endif
#endif
#endif

/* Minimum stack size allowed for a task, in words */
#ifndef TASK_STACK_MIN_WORDS
#ifndef TASK_STACK_MIN_WORDS
#ifndef TASK_STACK_MIN_WORDS
#define TASK_STACK_MIN_WORDS    32
#endif
#endif
#endif

// Possible states of a task
typedef enum {
    TASK_STATE_READY=0,       // Task is ready to run and waiting for CPU time
    TASK_STATE_RUNNING,     // Task is currently executing on the CPU
    TASK_STATE_BLOCKED,     // Task is waiting for an event, resource, or timeout
    TASK_STATE_SUSPENDED,   // Task is suspended and cannot be scheduled
} TaskState_t;

/* Function prototype executed by a task */
typedef void (*TaskFunction_t)(void *arg);

/* Forward declarations of structures used by the TCB */
typedef struct task_queue_t task_queue_t;
typedef struct kernel_mutex_t kernel_mutex_t; 

/*
 * Task Control Block (TCB)
 *
 * Contains all information required by the kernel
 * to manage and schedule a task.
 */
typedef struct TCB_t
{
    uint32_t *sp;                       // Current saved stack pointer of the task

    TaskFunction_t task_function;       // Entry function executed by the task
    void *arg;                          // Argument passed to the task function

    uint8_t priority;                   // Current task priority (may change due to priority inheritance)
    uint8_t base_priority;              // Original task priority before any priority inheritance

    uint32_t period;                    // Task period, expressed in system ticks
    TaskState_t state;                  // Current state of the task

    uint32_t wakeup_tick;               // System tick at which a blocked task can wake up

    uint32_t *stack_base;               // Base address of the task's stack
    uint32_t stack_words;               // Stack size in words (1 word = 4 bytes)

    /* Linked list of mutexes currently owned by this task. Used by the mutex subsystem, for example for priority inheritance */
    kernel_mutex_t *owned_mutexes;

    /* Mutex on which this task is currently blocked. NULL if the task is not waiting for a mutex */
    kernel_mutex_t *blocked_on;

    /* Pointer to the next TCB in a waiting queue. Used to implement linked-list based task queues */
    struct TCB_t *next;

    /* Queue on which the task is currently blocked. NULL when the task is not waiting in a queue */
    task_queue_t *current_wait_queue;

    /* Indicates whether the task was unblocked because its waiting timeout expired */
    volatile uint8_t timed_out;
    
    /* Scratch pointer used only while this task is blocked inside
       kernel_queue_send()/kernel_queue_receive(): the sender's source
       buffer, or the receiver's destination buffer. NULL whenever the
       task is not blocked on a queue. */
    void *pending_item;

} TCB_t;

/*
    Return status of task creation.
    The kernel uses these values to report whether task creation succeeded or why it failed.
*/
typedef enum
{
    TASK_CREATE_OK = 0,             // Task successfully created
    TASK_CREATE_ERR_MAX_TASKS,      // Maximum number of tasks already reached
    TASK_CREATE_ERR_NO_MEMORY,      // Not enough memory for the task stack
    TASK_CREATE_ERR_BAD_ARGS        // Invalid task creation parameters
} task_create_status_t;

/* Initializes the kernel task/stack memory pool */
void task_pool_init(void);

/*
    Creates a new task and initializes its TCB and stack.

    fn : Task entry function
    arg : Argument passed to the task
    priority : Initial task priority
    stack_words : Requested stack size in words
    period : Task period in system ticks
*/
task_create_status_t kernel_task_create(TaskFunction_t fn, void *arg, uint8_t priority, uint32_t stack_words, uint32_t period);


/* Returns a pointer to the TCB associated with the given task ID */
TCB_t *task_get_TCB(uint8_t id);

/* Returns the task ID associated with the given TCB, or -1 if invalid */
int8_t task_get_id(const TCB_t *task);

/* Returns the current number of created tasks */
uint8_t kernel_task_count(void);

/* Creates the kernel's idle task. This task runs when no other task is ready to execute */
task_create_status_t idle_task_create(void);

/* Global task table containing the TCB of every created task */
extern TCB_t task_table[MAX_TASKS];

/* Number of currently created tasks */
extern uint8_t total_tasks;

#endif