#ifndef KERNEL_H
#define KERNEL_H

/*
	Single aggregation header for the ANSA kernel.
 	This file is intended to be included by application code instead of including individual kernel headers separately.

	Include order matters:
 	- task.h defines TCB_t and task-related types.
	- queue.h uses TCB_t and defines queue/IPC types.
 	- scheduler.h depends on task and queue definitions.

	Keep this order when adding new kernel headers to avoid type dependency and declaration-order issues.
*/

#include "task.h"          /* Task Control Block, task states and task management */
#include "queue.h"         /* Kernel queues and queue management */
#include "scheduler.h"     /* Scheduler APIs and kernel timing functions */
#include "mutex.h"         /* Mutex synchronization primitives */
#include "semaphore.h"     /* Semaphore synchronization primitives */
#include "Notification.h"  /* Task notification mechanisms */
#include "mailbox.h"       /* Mailbox IPC mechanisms */
#include "watchdog.h"      /* Watchdog management and supervision */

#endif /* KERNEL_H */
