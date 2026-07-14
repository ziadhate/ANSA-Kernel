#include "scheduler.h"
#include "logger.h"
#include "preempt_flag.h"
#include <stdint.h>
#include <string.h> // for strncpy

/* flag for preemption */
atomic_int g_preempt_flag = 0;

/* Global tick counter – incremented by tick_increment() */
static volatile uint32_t g_tick = 0;
static SchedMode_t g_mode = SCHED_COOPERATIVE;
static uint32_t g_timeslice = 10;
static int g_rr_current = -1;
static uint32_t g_slice_start = 0;

/* Task table */
static TCB_t g_tasks[MAX_TASKS];

/* Optional flag if you ever want to inspect “yield” usage */
static volatile bool g_yield_called = false;

/* Drain the injection queue: activate any tasks enqueued by other threads */
static TaskQueue_t *g_inject_q = NULL;

static bool task_is_due(int i) {
  TCB_t *t = &g_tasks[i];
  if (!t->active || !t->func)
    return false;
  if (t->state == TASK_BLOCKED)
    return false;
  if (t->state != TASK_READY)
    return false;
  return (g_tick - t->last_run) >= t->period_ms;
}

static bool task_is_runnable(int i) {
  TCB_t *t = &g_tasks[i];

  if (!t->active || !t->func)
    return false;

  if (t->state != TASK_READY)
    return false;

  return true;
}

static void run_task(int i) {
  TCB_t *t = &g_tasks[i];

  // capture tick before starting task
  uint32_t start_tick = g_tick;

  t->state = TASK_RUNNING;
  g_yield_called = false;

  // Log task start with timing and priority info
  // Pass 255 as priority sentinel when mode is not SCHED_PRIORITY,
  // so the macro omits the prio field from the log line
  uint8_t log_prio = (g_mode == SCHED_PRIORITY) ? t->priority : 255;
  LOG_TASK_START(t->name, start_tick, log_prio, t->period_ms);

  t->func();

  uint32_t end_tick = g_tick;

  t->last_run = end_tick;
  t->exec_count++;

  t->run_ticks_total += (end_tick - start_tick);

  // Compute ticks_left: how many ticks until this task is due again
  // If preempted mid-work the task has not consumed its full period,
  // so next_due is last_run + period_ms from the perspective of next scheduling
  uint32_t next_due = t->last_run + t->period_ms;
  uint32_t ticks_left = (next_due > end_tick) ? (next_due - end_tick) : 0;

  // ADD — detect preemption: flag was set by sched_round_robin
  bool was_preempted =
      atomic_load_explicit(&g_preempt_flag, memory_order_acquire);

  LOG_TASK_END(t->name, start_tick, end_tick, log_prio, was_preempted,
               ticks_left);

  t->state = TASK_READY;
  if (g_yield_called) {
    LOG_TASK_YIELD(t->name);
  }
}

void scheduler_init(void) {
  g_tick = 0;
  for (int i = 0; i < MAX_TASKS; ++i) {
    g_tasks[i].func = 0;
    g_tasks[i].name[0] = '\0';
    g_tasks[i].period_ms = 0;
    g_tasks[i].last_run = 0;
    g_tasks[i].delay_ticks = 0;
    g_tasks[i].priority = 128;
    g_tasks[i].state = TASK_SUSPENDED;
    g_tasks[i].active = 0;
    g_tasks[i].run_ticks_total = 0;
    g_tasks[i].preempt_count = 0;
  }
}

void tick_increment(void) {
  g_tick++; // 1 tick (treat as 1 ms for now)
}

uint32_t scheduler_now(void) { return g_tick; }

int create_task(TaskFunc_t func, const char *name, uint32_t period_ms) {
  if (func == 0 || period_ms == 0) {
    LOG_ERROR("create_task() called with NULL func or zero period");
    return -1;
  }

  for (int i = 0; i < MAX_TASKS; ++i) {
    if (g_tasks[i].active == 0) {
      g_tasks[i].func = func;
      g_tasks[i].period_ms = period_ms;
      g_tasks[i].last_run = 0;
      g_tasks[i].delay_ticks = 0;
      g_tasks[i].exec_count = 0;
      g_tasks[i].priority = 128;
      g_tasks[i].state = TASK_READY;
      g_tasks[i].active = 1;
      g_tasks[i].run_ticks_total = 0;
      g_tasks[i].preempt_count = 0;

      if (name) {
        strncpy(g_tasks[i].name, name, sizeof(g_tasks[i].name) - 1);
        g_tasks[i].name[sizeof(g_tasks[i].name) - 1] = '\0';
      }

      LOG_TASK_CREATED(g_tasks[i].name, period_ms, i);
      return i;
    }
  }

  LOG_TASK_FULL();
  return -1;
}

void scheduler_set_delay(int task_id, uint32_t delay_ticks) {
  if (task_id < 0 || task_id >= MAX_TASKS)
    return;

  g_tasks[task_id].delay_ticks = delay_ticks;
  if (delay_ticks > 0) {
    g_tasks[task_id].state = TASK_BLOCKED;
  }
}

void scheduler_activate(int task_id, bool active) {
  if (task_id < 0 || task_id >= MAX_TASKS)
    return;

  g_tasks[task_id].active = active ? 1 : 0;
  g_tasks[task_id].state = active ? TASK_READY : TASK_SUSPENDED;
}

void scheduler_set_mode(SchedMode_t mode) {
  g_mode = mode;
  g_rr_current = -1;
  g_slice_start = g_tick;
  const char *names[] = {"COOPERATIVE", "ROUND_ROBIN", "PRIORITY"};
  LOG_INFO("SCHED mode switched to %s", names[mode]);
}

void scheduler_set_timeslice(uint32_t ticks) {
  g_timeslice = (ticks > 0) ? ticks : 1;
}

void scheduler_set_priority(int id, uint8_t priority) {
  if (id < 0 || id >= MAX_TASKS)
    return;
  g_tasks[id].priority = priority;
}

bool scheduler_update_priority(int id, uint8_t new_priority) {
  if (id < 0 || id >= MAX_TASKS) {
    return false;
  }

  scheduler_set_priority(id, new_priority);
  return true;
}

TaskState_t scheduler_get_state(int id) {
  if (id < 0 || id >= MAX_TASKS)
    return TASK_SUSPENDED;
  return g_tasks[id].state;
}

uint32_t scheduler_get_exec_count(int task_id) {
  if (task_id < 0 || task_id >= MAX_TASKS) {
    LOG_WARN("scheduler_get_exec_count() called with invalid task_id=%d",
             task_id);
    return 0;
  }
  return g_tasks[task_id].exec_count;
}

/* Simple cooperative yield: just mark that the task chose to stop early.
   You can inspect g_yield_called in a debugger if you want. */
void yield(void) { g_yield_called = true; }

/* Run through all tasks once; call any that are “due”. */
void sched_cooperative(void) {
  uint32_t now = g_tick;

  for (int i = 0; i < MAX_TASKS; ++i) {
    TCB_t *t = &g_tasks[i];

    if (!t->active || t->func == 0) {
      continue;
    }

    /* Respect initial delay, if any */
    if (now < t->delay_ticks) {
      LOG_VERBOSE("TASK  %-8s | startup delay, waiting until tick %u", t->name,
                  t->delay_ticks);
      continue;
    }

    /* Time-based check: has period elapsed since last run? */
    if ((now - t->last_run) >= t->period_ms) {
      LOG_TASK_RUN(t->name);
      g_yield_called = false; // reset flag for this run

      /* Run task once, non-preemptive */
      t->func();

      /* Update last_run after function returns */
      t->last_run = now;

      t->exec_count++;

      if (g_yield_called) {
        LOG_TASK_YIELD(t->name);
      }
    } else {
      LOG_TASK_SKIP(t->name);
    }
  }
}

static void sched_round_robin(void) {
  bool slice_done = (g_tick - g_slice_start) >= g_timeslice;
  bool first_run = (g_rr_current < 0);

  if (first_run || slice_done) {
    int start = (g_rr_current < 0) ? 0 : (g_rr_current + 1) % MAX_TASKS;
    int found = -1;
    for (int n = 0; n < MAX_TASKS; n++) {
      int idx = (start + n) % MAX_TASKS;
      if (task_is_due(idx)) {
        found = idx;
        break;
      }
    }
    if (found >= 0) {
      if (g_rr_current >= 0 && g_rr_current != found) {
        // signal the outgoing task it was preempted
        atomic_store_explicit(&g_preempt_flag, 1, memory_order_release);
        g_tasks[g_rr_current].preempt_count++;
        LOG_DEBUG("PREEMPT %-8s → %-8s  (slice=%u ticks)",
                  g_tasks[g_rr_current].name, g_tasks[found].name,
                  g_tick - g_slice_start);
        g_tasks[g_rr_current].state = TASK_READY;
      }
      g_rr_current = found;
      g_slice_start = g_tick;
      // clear flag before running the incoming task
      atomic_store_explicit(&g_preempt_flag, 0, memory_order_release);
      run_task(found);
    }
  }
}

static void sched_priority(void) {
  int best = -1;
  uint8_t best_prio = 255;

  for (int i = 0; i < MAX_TASKS; i++) {
    if (!task_is_due(i))
      continue;

    if (best < 0 || g_tasks[i].priority < best_prio ||
        (g_tasks[i].priority == best_prio &&
         g_tasks[i].last_run < g_tasks[best].last_run)) {
      best_prio = g_tasks[i].priority;
      best = i;
    }
  }

  if (best >= 0) {
    run_task(best);
  }
}

uint32_t scheduler_get_run_ticks(int task_id) {
  if (task_id < 0 || task_id >= MAX_TASKS)
    return 0;
  return g_tasks[task_id].run_ticks_total;
}

uint32_t scheduler_get_preempt_count(int task_id) {
  if (task_id < 0 || task_id >= MAX_TASKS)
    return 0;
  return g_tasks[task_id].preempt_count;
}

void scheduler_set_injection_queue(TaskQueue_t *q) { g_inject_q = q; }

static void scheduler_consume_injected_events(void) {
  if (!g_inject_q)
    return;

  uint8_t id;
  while (tq_dequeue(g_inject_q, &id)) {
    if (id < MAX_TASKS && g_tasks[id].active &&
        g_tasks[id].state != TASK_BLOCKED &&
        g_tasks[id].state != TASK_SUSPENDED) {
      g_tasks[id].state = TASK_READY;
    }
  }
}

static void scheduler_release_periodic_tasks(void) {
  for (int i = 0; i < MAX_TASKS; i++) {
    TCB_t *t = &g_tasks[i];

    if (!t->active || !t->func)
      continue;

    if (t->state == TASK_SUSPENDED)
      continue;

    if (t->state == TASK_BLOCKED) {
      if (g_tick >= t->delay_ticks) {
        t->state = TASK_READY;
      }
      continue;
    }

    if ((g_tick - t->last_run) >= t->period_ms) {
      t->state = TASK_READY;
    }
  }
}

void scheduler_run(void) {
  /* 1. Consume externally injected events */
  scheduler_consume_injected_events();

  /* 2. Release periodic tasks */
  scheduler_release_periodic_tasks();

  /* 3. Dispatch according to scheduler mode */
  switch (g_mode) {
  case SCHED_COOPERATIVE:
    sched_cooperative();
    break;
  case SCHED_ROUND_ROBIN:
    sched_round_robin();
    break;
  case SCHED_PRIORITY:
    sched_priority();
    break;
  }
}