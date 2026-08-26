#include "../Inc/watchdog.h"
#include "../Inc/watchdog_port.h"
#include "../Inc/scheduler.h"
#include "../Port/Cortex_M4/Inc/port.h"
#include <stddef.h>

//instantiating the classes
static watchdog_task_entry_t watchdog_table[WATCHDOG_MAX_TASKS];
static watchdog_status_t watchdog_status = WATCHDOG_NOT_INITIALIZED;
static TCB_t *watchdog_failed_task = NULL;



//Initialize the software watchdog table.s
void kernel_watchdog_init(void){
    port_critical_enter();

    for (uint8_t i = 0U; i < WATCHDOG_MAX_TASKS; i++) //initialising as much number of watcdogs as it is needed
    {   //assigning the initial value
        watchdog_table[i].enabled = false;
        watchdog_table[i].timeout_ticks = 0U;
        watchdog_table[i].last_heartbeat_tick = 0U;
        watchdog_table[i].fault = false;
    }

    watchdog_failed_task = NULL;
    watchdog_status = WATCHDOG_OK; //watchdogs have been initialisedf
    port_critical_exit();
}


//Register a task with the watchdog.
bool kernel_watchdog_register_task(TCB_t *task,uint32_t timeout_ticks){
    if ((task == NULL) || (timeout_ticks == 0U)){ // invalid task/insufficient tick time
        return false;
    }

    int8_t task_id = task_get_id(task); //get the task id
    if (task_id < 0 || task_id >= WATCHDOG_MAX_TASKS){ //check for bad id received
        return false;
    }

    //actual registration
    port_critical_enter();

    watchdog_table[task_id].enabled = true;
    watchdog_table[task_id].timeout_ticks = timeout_ticks;
    watchdog_table[task_id].last_heartbeat_tick = kernel_get_tick_count(); 
    watchdog_table[task_id].fault = false;

    port_critical_exit();

    return true;
}

//Remove a task from watchdog monitoring
bool kernel_watchdog_unregister_task(TCB_t *task){ 
    if (task == NULL){ // bad task handle provided
        return false;
    }

    int8_t task_id = task_get_id(task);

    if (task_id < 0 || task_id >= WATCHDOG_MAX_TASKS)
    {
        return false;
    }


    //deregistering
    port_critical_enter();

    watchdog_table[task_id].enabled = false;
    watchdog_table[task_id].fault = false;
    watchdog_table[task_id].timeout_ticks = 0U;
    watchdog_table[task_id].last_heartbeat_tick = 0U;

    port_critical_exit();

    return true;
}



//Current task reports that it is alive , only called by current task to check it self life : active or dead 
void kernel_watchdog_heartbeat(void){
    if (current_task == NULL){
        return;
    }

    int8_t task_id = task_get_id(current_task);

    if (task_id < 0 || task_id >= WATCHDOG_MAX_TASKS){
        return;
    }

    //Do not allow the idle task to become a monitored task.
    if (current_task->priority == 0xFFU){ //larger number means less priority
        return;
    }

    watchdog_table[task_id].last_heartbeat_tick = kernel_get_tick_count();
    watchdog_table[task_id].fault = false; //fault = 0 , means task is alive
}



//Check all registered tasks.
//A BLOCKED task is not considered failed.
//Blocking is a normal kernel operation caused by:
//    ->semaphore
//    ->mutex
//    ->notification
//    ->queue
//    ->delay
//only apply heartbeat deadlines while a task is READY or RUNNING.
void kernel_watchdog_check(void){
    uint32_t now = kernel_get_tick_count();

    for (uint8_t i = 0U; i < WATCHDOG_MAX_TASKS; i++)
    {
        watchdog_task_entry_t *entry = &watchdog_table[i];

        if (!entry->enabled){ // for ith iteration enabled = 0 ; meaning the watchdog has been enabled
            continue;
        }

        TCB_t *task = task_get_TCB(i) ; //
        if (task == NULL){
            continue;
        }

        //A blocked task is allowed to remain without a heartbeat.
        if (task->state == TASK_STATE_BLOCKED){
            continue;
        }

        //Suspended tasks are also not monitored while suspended.
        if (task->state == TASK_STATE_SUSPENDED){
            continue;
        }

        //Unsigned subtraction naturally handles uint32_t
        //tick-counter wraparound
        if ((uint32_t)(now - entry->last_heartbeat_tick) >= entry->timeout_ticks){ // means the last time the task was active is greater than the timeout tick
            entry->fault = true; // there is a fault in the task

            watchdog_failed_task = task; // current task has failed 
            watchdog_status = WATCHDOG_TASK_TIMEOUT; 

            return;
        }
    }

    /*
        All monitored tasks are healthy.
        Feed the hardware watchdog.
     */
    port_watchdog_kick();
}



//Start hardware watchdog.
void kernel_watchdog_start(void){
    port_watchdog_start();
}



//Return current watchdog state.
watchdog_status_t kernel_watchdog_get_status(void){
    return watchdog_status;
}

//Returns true if a watchdog fault has occurred.
bool kernel_watchdog_faulted(void){
    return (watchdog_status != WATCHDOG_OK);
}

//Returns the task that caused the watchdog failure.
TCB_t *kernel_watchdog_fault_task(void){
    return watchdog_failed_task;
}