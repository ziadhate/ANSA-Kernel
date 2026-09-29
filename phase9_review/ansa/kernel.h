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

#include "task.h"   	/* TCB_t, task states, task management */
#include "queue.h"      /* Task queues and IPC queue types */
#include "scheduler.h"  /* Scheduler APIs and kernel timing functions */
#endif /* KERNEL_H */
