#include "fsm_task.h"
#include "fsm.h"
#include "fsm_events.h"
#include "scheduler.h"
#include "sensors.h"   // this is in the FSM root directory where sensor stack team will provide sensor_snapshot API



//FSM EVENT QUEUE
static uint8_t fsm_event_queue_buffer[FSM_EVENT_QUEUE_LENGTH * sizeof(fsm_event_t)] ;

//initialisation of queue on which sensor stack will send the notification
kernel_queue_t fsm_event_queue;



//FSM SENSOR SNAPSHOT
static sensor_snapshot fsm_snapshot;


//INITIALIZATION
void fsm_task_init(void)
{
    (void)kernel_queue_init(
        &fsm_event_queue,
        fsm_event_queue_buffer,
        FSM_EVENT_QUEUE_LENGTH,
        sizeof(fsm_event_t)
    );
}



//event adapter function
static Event_t fsm_convert_event(fsm_event_type_t event_type)
{
    switch (event_type)
    {
        case EVT_NONE:
            return EVT_NONE;

        case EVT_INIT_DONE:
            return EVT_INIT_OK;

        case EVT_ARM_REQUEST:
            return EVT_ARM_CMD;

        case EVT_DISARM_REQUEST:
            return EVT_DISARM_CMD;

        case EVT_TAKEOFF_REQUEST:
            return EVT_TAKEOFF_CMD;

        case EVT_MISSION_START:
            return EVT_MISSION_CMD;

        case EVT_MISSION_COMPLETE:
            return EVT_MISSION_COMPLETE;

        case EVT_SENSOR_FAULT:
            return EVT_SENSOR_FAIL;

        case EVT_LINK_LOST:
            return EVT_SIGNAL_LOST;

        case EVT_BATTERY_CRITICAL:
            return EVT_BATTERY_CRITICAL;

        default:
            return EVT_NONE;
    }
}


//FSM PERIODIC TASK
void fsm_task(void *arg)
{
    (void)arg;

    uint32_t previous_wake_tick;
    fsm_event_t received_event;
    Event_t event;

    //nitialize FSM
    fsm_init();

    previous_wake_tick = kernel_get_tick_count();

    for (;;)
    {
        /* SENSOR SNAPSHOT
         * The FSM must obtain one coherent value-copy of the
         * latest sensor snapshot once during every FSM tick.
         *
         * The Sensor Stack team will provide the final API
         * through sensors.h.
         *
         * Expected interface from the integration guide:
         *
         *     sensor_stack_get_snapshot(&fsm_snapshot);
         *
         * The exact function name/signature must match the
         * final sensors.h provided by the Sensor Stack team.
         *
         * Do not access raw Sensor Stack data directly.
         */


        //NON-BLOCKING EVENT PROCESSING
        while (kernel_queue_receive(
                &fsm_event_queue,
                &received_event,
                0U
               ) == QUEUE_PASSED)
        {
            event = fsm_convert_event(received_event.type);

            //Execute FSM transition logic.
            fsm_tick(event);
        }


        //PERIODIC FSM PROCESSING
        /*
         * The FSM is scheduled periodically at 50 Hz.
         * Sensor-dependent guards and state processing must
         * operate on the coherent snapshot obtained above.
         * The final call used here depends on the FSM public
         * API supplied by the FSM team.
         */


        //PERIODIC RELEASE
        task_delay_until( &previous_wake_tick, FSM_TASK_PERIOD_TICKS ) ;
    }
}