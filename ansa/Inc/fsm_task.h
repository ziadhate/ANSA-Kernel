
/* ============================================================
 * ANSA INTEGRATION REQUIREMENTS
 * ============================================================
 *
 * The final ANSA application must perform the following
 * initialization sequence before starting the scheduler:
 *     1. kernel_init();
 *     2. Sensor Stack initialization
 *        (as required by the Sensor Stack team)
 *     3. fsm_task_init();
 *     4. kernel_task_create(
 *            fsm_task,
 *            NULL,
 *            FSM_TASK_PRIORITY,
 *            FSM_TASK_STACK_WORDS,
 *            FSM_TASK_PERIOD_TICKS
 *        );
 *     5. kernel_start();
 *
 *
 * IMPORTANT:
 *
 * - The FSM event queue must be initialized before the FSM
 *   task starts executing.
 *
 * - The FSM task is periodic.
 *
 * - The FSM uses the non-blocking Kernel queue receive API.
 *
 * - The FSM must not block waiting for Sensor Stack events.
 *
 * - Sensor data is provided through sensor_snapshot defined
 *   by the Sensor Stack interface in sensors.h.
 *
 * - The final Sensor Stack snapshot access mechanism must
 *   be used according to the implementation provided by the
 *   Sensor Stack team.
 *
 * ============================================================
 */

#ifndef FSM_TASK_H
#define FSM_TASK_H

#include <stdint.h>

#include "queue.h"
#include "fsm_events.h"


/* FSM TASK CONFIGURATION
 * Kernel tick rate:
 *      1000 Hz = 1 ms/tick
 *
 * FSM period:
 *      20 ms = 20 ticks
 *
 * Therefore:
 *      FSM executes at 50 Hz.
 */
#define FSM_TASK_PRIORITY       2U
#define FSM_TASK_STACK_WORDS    128U
#define FSM_TASK_PERIOD_TICKS   20U

//Maximum number of pending FSM events.
#define FSM_EVENT_QUEUE_LENGTH  8U


/* Initializes resources required by the FSM task.
 * This must be called before the FSM task is created. */
void fsm_task_init(void);


//Kernel task entry point.
void fsm_task(void *arg);


/* FSM EVENT QUEUE
 * The Sensor Stack / integration layer uses this queue to
 * provide events to the FSM.
 * Queue operations are non-blocking */
extern kernel_queue_t fsm_event_queue;

#endif 