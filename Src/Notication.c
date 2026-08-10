#include "../Inc/notification.h"
#include "../Inc/scheduler.h"
#include <stddef.h>

//===================================================================================================================================
//creatre a notification
notification_status_t kernel_notification_init(kernel_notification_t *notification){

    if (notification == NULL) // no valid instance of notication passed
    {
        return NOTIFICATION_ERROR;
    }

    CRITICAL_SECTION_ENTER(); // we entr this because we dont want any other task to modify the daa we are modifying right now
    //create a notification
    notification->pending = false;

    notification->waiters.head = NULL;
    notification->waiters.tail = NULL;

    CRITICAL_SECTION_EXIT();

    return NOTIFICATION_SUCCESS;
}


//=================================================================================================================================
//make a task wait untill notification arrives
notification_status_t kernel_notification_wait(kernel_notification_t *notification){

    if (notification == NULL) //checking for bad notification pointer 
    {
        return NOTIFICATION_ERROR;
    }

    if (current_task == NULL) // no current task found to be notified
    {
        return NOTIFICATION_ERROR;
    }

    CRITICAL_SECTION_ENTER();

    //check for existing notification
    if (notification->pending) //means there is already a notification , then dont block the task
    {
        notification->pending = false;

        CRITICAL_SECTION_EXIT();

        return NOTIFICATION_SUCCESS;
    } // if not then go ahead

    //Store the address of this notification's waiter field into the waiting  queue inside Task's TCB.
    current_task->current_wait_queue = &notification->waiters; // current task has to wait for notification in the queue specified by the notification's waiter field 
    current_task->state = TASK_STATE_BLOCKED; // current task has been blocked beacuase no notification is available

    task_queue_push_task(&notification->waiters, current_task); //put the current into the notifications waiting queue

    CRITICAL_SECTION_EXIT();

    //give bac CPU to schedular to decide which task to run next
    kernel_yield();

    return NOTIFICATION_SUCCESS;
}


//======================================================================================================================================
//this will do the actual notifying,waking-up job
notification_status_t kernel_notification_notify(kernel_notification_t *notification){

    if (notification == NULL)
    {
        return NOTIFICATION_ERROR;
    }

    CRITICAL_SECTION_ENTER();

    /*
     * If a task is waiting, wake one waiting task.
     */
    if (!task_queue_empty(&notification->waiters)) //task_queue_empty is a bool type , so should return 1/0
    {
        TCB_t *woken = task_queue_pop_task(&notification->waiters); //push the waiting task into the waiting queue

        if (woken == NULL) // check if task pushed or not 
        {
            CRITICAL_SECTION_EXIT();

            return NOTIFICATION_ERROR;
        }
        
        //unblocking the task
        woken->current_wait_queue = NULL; // removing the task from waiting queue
        woken->state = TASK_STATE_READY; // task unblocked
    }
    else //no task is waiting , since waiter is NULL 
    {
        notification->pending = true;
    }

    CRITICAL_SECTION_EXIT();

    /*
     * Allow scheduler to decide whether
     * the newly READY task should run.
     */
    kernel_yield();

    return NOTIFICATION_SUCCESS;
}