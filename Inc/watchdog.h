#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>
#include <stdbool.h>
#include "task.h"


//Maximum number of tasks that can be monitored.
//i Kept this equal to MAX_TASKS because the watchdog table
//uses task IDs.
#define WATCHDOG_MAX_TASKS    MAX_TASKS

typedef enum
{
    WATCHDOG_OK = 0,
    WATCHDOG_TASK_TIMEOUT,
    WATCHDOG_NOT_INITIALIZED
} watchdog_status_t;

//Information maintained for each monitored task.
typedef struct{
    bool enabled;            
    uint32_t timeout_ticks ; //Maximum allowed time between heartbeats,expressed in kernel ticks. 
    uint32_t last_heartbeat_tick; //Tick at which the task last reported that it was alive
    bool fault;  //Set when the watchdog detects a failure,reset is needed if fault = 1 

} watchdog_task_entry_t;

//Initializes the software watchdog subsystem.
void kernel_watchdog_init(void);

// this function registers a task with the watchdog
//task           : task to monitor
//timeout_ticks  : maximum heartbeat interval in system ticks , given by user
//Returns false if registration failed
bool kernel_watchdog_register_task(TCB_t *task,uint32_t timeout_ticks);

//this function removes a task from watchdog monitoring.
bool kernel_watchdog_unregister_task(TCB_t *task);

//This function should normally be called by the task being monitored.Reports that the currently executing task is alive.
void kernel_watchdog_heartbeat(void);

//Checks all registered tasks.
//Called periodically from the kernel tick.
void kernel_watchdog_check(void);

//Starts the hardware watchdog.
void kernel_watchdog_start(void);

//Returns the current watchdog status.
watchdog_status_t kernel_watchdog_get_status(void);

//Returns true if a watchdog fault has occurred.
bool kernel_watchdog_faulted(void);

//Returns the task that caused the watchdog failure.
TCB_t *kernel_watchdog_fault_task(void);

#endif