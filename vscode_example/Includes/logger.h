#ifndef LOGGER_H
#define LOGGER_H

#define LOG_ERROR(fmt, ...)
#define LOG_WARN(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_DEBUG(fmt, ...)
#define LOG_VERBOSE(fmt, ...)
#define LOG_TASK_START(...)
#define LOG_TASK_END(...)
#define LOG_TASK_RUN(name)
#define LOG_TASK_SKIP(name)
#define LOG_TASK_YIELD(name)
#define LOG_TASK_CREATED(name, period, slot)
#define LOG_TASK_FULL()
#define LOG_ACCURACY(name, expected, actual)
#define LOG_RR_FAIRNESS(name, starts, preempts, run_ticks, cpu_share, fair_err)

static inline void logger_init(const char *filename) { (void)filename; }
static inline void logger_close(void) {}

#endif /* LOGGER_H */