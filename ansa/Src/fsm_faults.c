#include <stddef.h>
#include "fsm_internal.h"

void fsm_fault_report(const fsm_fault_t *fault)
{
    if (fault == NULL || fault->version != FSM_STRUCT_VERSION)
        return;

    fsm_event_type_t event = EVT_NONE;

    switch (fault->code) {
        case FSM_FAULT_IMU_DIVERGENCE:
        case FSM_FAULT_MOTOR_SATURATION:
        case FSM_FAULT_WATCHDOG:
        case FSM_FAULT_SENSOR_STALE:
            event = EVT_SENSOR_FAULT;
            break;
        case FSM_FAULT_BATTERY_CRITICAL:
            event = EVT_BATTERY_CRITICAL;
            break;
        case FSM_FAULT_LINK_LOST:
            event = EVT_LINK_LOST;
            break;
        case FSM_FAULT_GEOFENCE:
            event = EVT_GEOFENCE_BREACH;
            break;
        default:
            return;
    }

    fsm_event_t e = {
        FSM_STRUCT_VERSION,
        event,
        fault->timestamp_us,
        (uint32_t)fault->code
    };
    (void)fsm_post_event(&e);
}
