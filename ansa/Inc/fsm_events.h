#ifndef SPACEBORN_FSM_EVENTS_H
#define SPACEBORN_FSM_EVENTS_H

#include <stdint.h>

typedef enum {
    EVT_NONE = 0,
    EVT_INIT_DONE,
    EVT_ARM_REQUEST,
    EVT_DISARM_REQUEST,
    EVT_ARM_SUCCESS,
    EVT_TAKEOFF_REQUEST,
    EVT_MISSION_START,
    EVT_MISSION_PAUSE,
    EVT_MISSION_RESUME,
    EVT_MISSION_ABORT,
    EVT_WAYPOINT_REACHED,
    EVT_MISSION_COMPLETE,
    EVT_RTL_REQUEST,
    EVT_LAND_REQUEST,
    EVT_SENSOR_FAULT,
    EVT_LINK_LOST,
    EVT_BATTERY_CRITICAL,
    EVT_GEOFENCE_BREACH,
    EVT_WATCHDOG_TIMEOUT,
    EVT_TOUCHDOWN,
    EVT_INTERNAL_TICK,
    EVT_COUNT
} fsm_event_type_t;

typedef struct {
    uint32_t version;
    fsm_event_type_t type;
    uint32_t timestamp_us;
    uint32_t payload;
} fsm_event_t;

#endif
