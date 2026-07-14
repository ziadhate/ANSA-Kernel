#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#include "task_queue.h"

#define MAX_TASKS 8

typedef void (*TaskFunc_t)(void);

/* Task states */
typedef enum {
  TASK_READY,
  TASK_RUNNING,
  TASK_BLOCKED,
  TASK_SUSPENDED
} TaskState_t;

/* Scheduler modes */
typedef enum {
  SCHED_COOPERATIVE,
  SCHED_ROUND_ROBIN,
  SCHED_PRIORITY,
} SchedMode_t;

/* Task Control Block (TCB) */
typedef struct {
  TaskFunc_t func;    // Task entry function
  char name[16];      // Short name (debug/logging)
  uint32_t period_ms; // Period in ticks (1 tick = 1 ms in this example)
  uint32_t last_run;  // Tick value when the task last ran
  uint32_t
      delay_ticks; // Earliest tick when task is allowed to run
                   // Startup delay: task won't run until g_tick >= this value
  uint32_t exec_count; // Actual execution count (for accuracy check)
  uint8_t priority;    // Task priority, highest=0 , lowest=255
  TaskState_t state;   // Task state enum: READY, RUNNING, BLOCKED, SUSPENEDED
  uint8_t active;      // 1 = in use, 0 = free
  uint32_t run_ticks_total; // ticks spent running
  uint32_t preempt_count;   // total preempts for this task
} TCB_t;

/* Global tick API (fake timer on PC, real timer later) */
void scheduler_init(void);
void tick_increment(void);    // Call once per “tick” (e.g., 1 ms)
uint32_t scheduler_now(void); // Read current tick

/* Core scheduler */
void scheduler_run(void); // Call from super-loop

void sched_cooperative(void); // The cooperative scheduler

/* Task management */
int create_task(TaskFunc_t func, const char *name,
                uint32_t period_ms); // returns slot index or -1

/* Optional helpers */
void scheduler_set_delay(int task_id, uint32_t delay_ticks);
void scheduler_activate(int task_id, bool active);
uint32_t scheduler_get_exec_count(int task_id);

void scheduler_set_mode(SchedMode_t mode);
void scheduler_set_timeslice(uint32_t ticks);
void scheduler_set_priority(int task_id, uint8_t priority);
TaskState_t scheduler_get_state(int task_id);

/* Call once after scheduler_init().
   Pass NULL if running single-threaded — drain step is skipped. */
void scheduler_set_injection_queue(TaskQueue_t *q);

uint32_t scheduler_get_run_ticks(int task_id);
uint32_t scheduler_get_preempt_count(int task_id);

/* Cooperative yield hook – called from tasks when they have nothing to do */
void yield(void);

#endif // SCHEDULER_H