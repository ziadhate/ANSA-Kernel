#ifndef KERNEL_CONFIG_H
#define KERNEL_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/* ========================================================================== */
/*                           KERNEL VERSION & RELEASE                         */
/* ========================================================================== */
#define KERNEL_VERSION_MAJOR      1
#define KERNEL_VERSION_MINOR      0
#define KERNEL_VERSION_PATCH      0
#define KERNEL_VERSION_STRING     "1.0.0-Production"

/* ========================================================================== */
/*                       BUILD CONFIGURATION SWITCHES                         */
/* ========================================================================== */
#define KERNEL_CFG_DEBUG_ENABLE   1

/* ========================================================================== */
/*                      SYSTEM TIMING & SCHEDULER SPECS                      */
/* ========================================================================== */
#define TICK_RATE_HZ              1000U      /* 1 Tick = 1ms */
#define MAX_TASKS                 20U        /* Max tasks allowed */
#define KERNEL_STACK_POOL_SIZE    8192U      /* Total stack pool size (in 32-bit words) */
#define TASK_STACK_MIN_WORDS      32U        /* Minimum stack per task */

/* ========================================================================== */
/*                   SAFETY, ASSERTIONS & DIAGNOSTICS                         */
/* ========================================================================== */
#define KERNEL_CFG_STACK_CANARY   0xDEADBEEFU
#define KERNEL_CFG_STACK_WATERMARK_FILL 0xA5A5A5A5U

#if (KERNEL_CFG_DEBUG_ENABLE == 1)
    #define KERNEL_ASSERT(expr) \
        do { \
            if (!(expr)) { \
                kernel_panic(__FILE__, __LINE__); \
            } \
        } while(0)
#else
    #define KERNEL_ASSERT(expr) ((void)0)
#endif

void kernel_panic(const char *file, int line);

#endif /* KERNEL_CONFIG_H */
