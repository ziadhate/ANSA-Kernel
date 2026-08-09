#ifndef NOTIFICATION_H
#define NOTIFICATION_H

//user includes
#include <stdbool.h>

#include "queue.h"

// This is actual notification 
typedef struct
{
    bool pending; // pending = YES -> means a notification has arrived , = NO means no new notification

    // Tasks blocked in the queue waiting for the notification 
    task_queue_t waiters;

} kernel_notification_t;


//this is a class which stores status of the notification and used as the return type for the implementation of function
typedef enum
{
    NOTIFICATION_SUCCESS = 0, // notification sent successfully
    NOTIFICATION_ERROR,       // notification not sent bcz no notification instance is available,etc
    NOTIFICATION_EMPTY        // no notification available to be consumed

} notification_status_t;
 
// Initialize a notification object.
notification_status_t kernel_notification_init(kernel_notification_t *notification); // the pointer points to the notification object created

//this fxn a task will call to be in sleep mode untill a notification has addressed them 
notification_status_t kernel_notification_wait(kernel_notification_t *notification);

//this is the actual messenger that tells a task to wake up from sleep mode/blocked mode
notification_status_t kernel_notification_notify(kernel_notification_t *notification);

#endif

