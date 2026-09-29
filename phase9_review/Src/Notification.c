#include "../Inc/Notification.h"
#include "../Inc/scheduler.h"
#include "../Port/Cortex_M4/Inc/port.h"
#include <stddef.h>

//===================================================================================================================================
//creatre a notification
notification_status_t kernel_notification_init(kernel_notification_t *notification){

    if (notification == NULL) // no valid instance of notication passed
    {
        return NOTIFICATION_ERROR;
    }

    port_critical_enter(); // we entr this because we dont want any other task to modify the daa we are modifying right now
    //create a notification
    notification->pending = false;

    notification->waiters.head = NULL;
    notification->waiters.tail = NULL;

    port_critical_exit();

    return NOTIFICATION_SUCCESS;
}


//=================================================================================================================================
//make a task wait untill notification arrives
notification_status_t kernel_notification_wait( kernel_notification_t *notification, uint32_t timeout_ticks)
{
    if (notification == NULL)
    {
        return NOTIFICATION_ERROR;
    }

    if (current_task == NULL)
    {
        return NOTIFICATION_ERROR;
    }

    port_critical_enter();

    /*
     * A notification is already pending.
     * Consume it immediately.
     */
    if (notification->pending)
    {
        notification->pending = false;
        current_task->timed_out = 0U;

        port_critical_exit();

        return NOTIFICATION_SUCCESS;
    }

    /*
     * Zero timeout means:
     * check immediately without blocking.
     */
    if (timeout_ticks == 0U)
    {
        port_critical_exit();

        return NOTIFICATION_EMPTY;
    }

    /*
     * No notification is available.
     * Block the current task until either:
     * ---> A notification arrives, or
     * ---> timeout_ticks expires.
     */
    current_task->timed_out = 0U;

    current_task->wakeup_tick = kernel_get_tick_count() + timeout_ticks;

    current_task->current_wait_queue = &notification->waiters;

    current_task->state = TASK_STATE_BLOCKED;

    task_queue_push_task(&notification->waiters, current_task);

    port_critical_exit();

    /*
     * Give the CPU to another task.
     */
    port_trigger_context_switch();

    /*
     * We have been scheduled again.
     * Determine why we woke up.
     */
    if (current_task->timed_out)
    {
        return NOTIFICATION_EMPTY;
    }

    return NOTIFICATION_SUCCESS;
}


//======================================================================================================================================
//This will do the actual notifying and waking-up job
notification_status_t kernel_notification_notify(kernel_notification_t *notification){

    if (notification == NULL)
    {
        return NOTIFICATION_ERROR;
    }

    port_critical_enter();

    /* If a task is waiting, wake one waiting task */
    bool yield_requested=false;
   
    if (!task_queue_empty(&notification->waiters)) //task_queue_empty is a bool type , so should return 1/0
    {                                              // 1- queue is empty
                                                   // 0- queue is not empty (in this case this block will execute and pop one task from the queue and wake it)
        TCB_t *woken = task_queue_pop_task(&notification->waiters); //Remove-pop the first-highest-priority task from a waiting queue 
                                                                    //and return a pointer to that task's TCB.

        if (woken == NULL) // check if task poped or not 
        {
            port_critical_exit();

            return NOTIFICATION_ERROR;
        }
        
        //unblocking the task
        woken->current_wait_queue = NULL; // removing the current task from waiting queue
        woken->timed_out = 0U;            //task has just unblocked ,, so ticks have been reset
        woken->state = TASK_STATE_READY; // task unblocked
        if(current_task->priority > woken->priority){
            yield_requested=true;
        }
    }
    else //no task is waiting , since waiter is NULL 
        //in this case we should save the notification for future API'S to consume the notification 
        // In future "kernel_notification_wait" , can consume the saved notification because this function was waiting for a notification
    {
        notification->pending = true; // save the unused notification to pending state for future use
    }

    port_critical_exit();

    // if the if condition on line : 81 is true , then by the time when that block completes execution , there is a task ready and
    // we are subjecting that task to be scheduled by the schedular as usual based on the priority this task has as compared to other tasks
    // we do this by doing the below API call
    if(yield_requested){
        kernel_yield();
    }

    return NOTIFICATION_SUCCESS;
}



    //ISR safe API'S implementation

    //======================================================================================================================================================
    notification_status_t kernel_notification_wait_from_isr(kernel_notification_t *notification){
    if (notification == NULL){
        return NOTIFICATION_ERROR;
    }

    if (notification->pending)
    {
        notification->pending = false; // consume the notification and change its status
        return NOTIFICATION_SUCCESS;
    }
    else
    {                              
        return NOTIFICATION_EMPTY; // Interrupts cannot block to wait for a notification
    }

    }

    //=========================================================================================================================================================
    notification_status_t kernel_notification_notify_from_isr(kernel_notification_t *notification){
    if (notification == NULL) //check for bad notification pointer passed
    {
        return NOTIFICATION_ERROR;
    }

    bool yield_requested = false;

    if (!task_queue_empty(&notification->waiters)){                                   
        TCB_t *woken = task_queue_pop_task(&notification->waiters); 

        if (woken == NULL)
        {
            return NOTIFICATION_ERROR;
        }

        woken->current_wait_queue = NULL;
        woken->state = TASK_STATE_READY;

        if ((current_task != NULL) && (woken->priority < current_task->priority)){ //means woken task is having more priority , lesser means more priority , then it needs to be scheduled
            yield_requested = true; 
        }
    }
    else        
    {
        notification->pending = true;
    }

    kernel_yield_from_isr(yield_requested);

    return NOTIFICATION_SUCCESS;
}

    //=============================================================================================================================================================================

