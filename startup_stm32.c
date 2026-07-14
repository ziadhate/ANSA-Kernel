#include "Includes/scheduler.h"
#include <stddef.h>

/* Mock application tasks updated to accept void *arg */
void Sensor_Task(void *arg) {
    while(1) {
        // Read IMU, Mag, Baro
        kernel_yield(); // Prevents the task from hogging the CPU
    }
}

void Control_Task(void *arg) {
    while(1) {
        // Run PID loops and matrix mixer
        kernel_yield();
    }
}

void FSM_Task(void *arg) {
    while(1) {
        // Monitor state transitions
        kernel_yield();
    }
}

/* Hardware Exception Vector Connections */
void SysTick_Handler(void) {
    kernel_update_ticks();
}

void PendSV_Handler(void) {
    // Context switch logic will be implemented here
}

int main(void) {
    /* ======================================================================== */
    /* DAY 15: TASKS REGISTERED AT STARTUP                                      */
    /* ======================================================================== */
    /* We now pass 'NULL' as the argument since we don't have config data yet   */
    
    // Sensor task needs less memory
    kernel_task_create(Sensor_Task, NULL, 1, 256); 
    
    // Control task involves heavy math, needs more stack
    kernel_task_create(Control_Task, NULL, 2, 512); 
    
    // FSM handles simple logic and events
    kernel_task_create(FSM_Task, NULL, 3, 256); 
    
    /* Fire up the OS scheduler. */
    kernel_start();
    
    while(1);
}