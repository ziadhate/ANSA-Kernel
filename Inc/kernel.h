#ifndef KERNEL_H
#define KERNEL_H

/*
	Single aggregation header for the ANSA kernel
	This file is incldued in application code instead of pulling individual kernel headers.

	Include order matters: task.h and queue.h define types (TCB_t, queue structs) that scheduler.h depend on.
 	Keep this order if you add new headers here.
*/

#include "task.h"         /* TCB_t, task states */
#include "queue.h"         /* IPC queue types */
#include "scheduler.h"     /* kernel_init, kernel_start — also pulls in port.h transitively (port_critical_enter/exit, port_trigger_context_switch) */

#endif /* KERNEL_H */
