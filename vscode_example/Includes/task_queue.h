#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct { int dummy; } TaskQueue_t;

static inline void tq_init(TaskQueue_t *q) { (void)q; }
static inline bool tq_enqueue(TaskQueue_t *q, uint8_t id) { (void)q; (void)id; return false; }
static inline bool tq_dequeue(TaskQueue_t *q, uint8_t *id) { (void)q; (void)id; return false; }

#endif /* TASK_QUEUE_H */