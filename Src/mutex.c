#include "../Inc/mutex.h"
#include "../Inc/scheduler.h"
#include "../Port/Cortex_M4/Inc/port.h"
#include <stddef.h>


mutex_status_t kernel_mutex_init(kernel_mutex_t *mutex)
{
    
    if (mutex == NULL)
    {
        return MUTEX_ERROR;
    }

     port_critical_enter();
    
    CRITICAL_SECTION_ENTER();


   
    //Mutex is initially unlocked.
     
    mutex->locked = false;


    
    //No task owns the mutex initially.
     
    mutex->owner = NULL;


    
     //Initialize the waiting task queue.
     //NULL head means no task is waiting.
     
    mutex->waiting_tasks.head = NULL;


    
    mutex->waiting_tasks.tail = NULL;
    mutex->next_owned = NULL;


    port_critical_exit();
    CRITICAL_SECTION_EXIT();


    return MUTEX_SUCCESS;
}


mutex_status_t kernel_mutex_lock(kernel_mutex_t *mutex)
{
     //Check for invalid mutex pointer.
     
    if (mutex == NULL)
    {
        return MUTEX_ERROR;
    }


    
     /*We need a valid current task because
     the mutex owner must be a task.*/
     
    if (current_task == NULL)
    {
        return MUTEX_ERROR;
    }



     /* Enter critical section because we are going
      to modify kernel synchronization data.*/
    port_critical_enter();
    CRITICAL_SECTION_ENTER();


    
     //Check whether mutex is currently free.
     
    // if (mutex->locked == false)
    // {
        
    //       //Mutex is free.
    //       //Give ownership to current task.
         
    //     mutex->locked = true;

    //     mutex->owner = current_task;


        
    //       //Critical section is no longer required.
        
    //     CRITICAL_SECTION_EXIT();


    //     return MUTEX_SUCCESS;
    // }
    if (mutex->locked == false)
    {
    mutex->locked = true;
    mutex->owner = current_task;
    port_critical_exit();
    /* Add mutex to owner's list */
    mutex->next_owned = current_task->owned_mutexes;
    current_task->owned_mutexes = mutex;

    current_task->blocked_on = NULL;

    CRITICAL_SECTION_EXIT();

    return MUTEX_SUCCESS;
}

//Modified
/* Prevent recursive locking */
if (mutex->owner == current_task)
{
    CRITICAL_SECTION_EXIT();
    return MUTEX_ERROR;
}

/*
 * Priority Inheritance
 * Lower number = higher priority.
 */
if (current_task->priority < mutex->owner->priority)
{
    mutex->owner->priority = current_task->priority;
}

/* Remember which mutex is blocking this task */
current_task->blocked_on = mutex;

    
if (task_queue_push_task(&mutex->waiting_tasks, current_task) != 0)
    {

        current_task->blocked_on = NULL;
        /*
         * Queue insertion failed.
         */
         
        CRITICAL_SECTION_EXIT();

        return MUTEX_ERROR;
    }


    
     /*Change current task state to BLOCKED.
     * The scheduler will therefore not select
     * this task while it is waiting.
     */
    current_task->state = TASK_STATE_BLOCKED;


    CRITICAL_SECTION_EXIT();


   
    kernel_yield();


    return MUTEX_SUCCESS;
}


mutex_status_t kernel_mutex_unlock(kernel_mutex_t *mutex)
{
    /*
     Check mutex pointer.
     */
    if (mutex == NULL)
    {
        return MUTEX_ERROR;
    }


   
      //There must be a current task.
     
    if (current_task == NULL)
    {
        return MUTEX_ERROR;
    }

    port_critical_enter();
   
       /* Enter critical section because mutex ownership
       and waiting queue will be modified.*/
     
    CRITICAL_SECTION_ENTER();


    
    // Check whether the current task owns the mutex.
     
    if (mutex->owner != current_task)
    {
        
          //A task other than the owner is trying
          //unlock the mutex.
         port_critical_exit();
        CRITICAL_SECTION_EXIT();

        return MUTEX_ERROR;
    }


    /*
    Check whether any task is waiting for the mutex.
     */
    // if (task_queue_empty(&mutex->waiting_tasks))
    // {
    //     mutex->locked = false;

    //     mutex->owner = NULL;
    // }
    //Modified
    if (task_queue_empty(&mutex->waiting_tasks))
    {
    /*
     * Remove mutex from owner's list.
     */
      if (current_task->owned_mutexes == mutex)
     {
        current_task->owned_mutexes = mutex->next_owned;
        }

    mutex->next_owned = NULL;

    /*
     * Restore original priority.
     */
    current_task->priority = current_task->base_priority;

    mutex->locked = false;

    mutex->owner = NULL;
}
    else
    {
        
         /* At least one task is waiting.
          Remove the first waiting task.*/
        TCB_t *next_owner =task_queue_pop_task(&mutex->waiting_tasks);


      
         //Make sure a valid task was obtained.
     
        if (next_owner == NULL)
        {
            
            //Something went wrong with the queue.
          
            CRITICAL_SECTION_EXIT();
             port_critical_exit();

            return MUTEX_ERROR;
        }


        
        // mutex->locked = true;

        // mutex->owner = next_owner;


        
        //   //Wake the waiting task.
       
        // next_owner->state = TASK_STATE_READY;


        
        // //Clear the task's waiting queue information.
       
        // next_owner->current_wait_queue = NULL;
        // next_owner->next = NULL;

        /*
        //modified
 * Remove mutex from old owner's list.
 */
if (current_task->owned_mutexes == mutex)
{
    current_task->owned_mutexes =
        mutex->next_owned;
}

mutex->next_owned = NULL;


/*
 * Restore old owner's original priority.
 */
    current_task->priority = current_task->base_priority;


/*
 * Transfer ownership to waiting task.
 */
    mutex->locked = true;
    mutex->owner = next_owner;


/*
 * Add mutex to new owner's list.
 */
    mutex->next_owned = next_owner->owned_mutexes;

    next_owner->owned_mutexes =
    mutex;


/*
 * Wake the new owner.
 */
    next_owner->state = TASK_STATE_READY;

    next_owner->blocked_on = NULL;

    next_owner->current_wait_queue = NULL;
    next_owner->next = NULL;
    }
    


    CRITICAL_SECTION_EXIT();


   
      /* Request scheduler to check whether the newly
      READY task should execute.*/
    kernel_yield();


    return MUTEX_SUCCESS;
}



mutex_status_t kernel_mutex_try_lock(kernel_mutex_t *mutex)
{
    
    if (mutex == NULL)
    {
        return MUTEX_ERROR;
    }


    /*  A task must exist to own the mutex.*/
    if (current_task == NULL)
    {
        return MUTEX_ERROR;
    }

     port_critical_enter();
    /*Protect mutex access. */
    CRITICAL_SECTION_ENTER();


    //Check whether mutex is already locked.  
    if (mutex->locked == true)
    {
        /*
         Mutex is busy.
         Do not block the current task.
         */
        CRITICAL_SECTION_EXIT();

        return MUTEX_BUSY;
    }


    /*Mutex is free.
     Give ownership to current task.*/
    // mutex->locked = true;

    // mutex->owner = current_task;
    mutex->locked = true;
    mutex->owner = current_task;
    port_critical_exit();
/*
 * Add mutex to owner's list.
 */
    mutex->next_owned =
    current_task->owned_mutexes;

    current_task->owned_mutexes =
    mutex;

    current_task->blocked_on = NULL;


    //Leave critical section.  
    CRITICAL_SECTION_EXIT();


    return MUTEX_SUCCESS;
}
