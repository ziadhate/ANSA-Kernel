#include "fsm.h"

extern void state_boot(Event_t);
extern void state_idle(Event_t);
extern void state_arming(Event_t);
extern void state_armed(Event_t);
extern void state_takeoff(Event_t);
extern void state_hover(Event_t);
extern void state_mission(Event_t);
extern void state_rtl(Event_t);
extern void state_land(Event_t);
extern void state_emergency(Event_t);
extern void state_failsafe(Event_t);

static State_t current_state;
static SensorData_t sensors;

//
void fsm_init(void)
{
    current_state = STATE_BOOT;
}

//
State_t fsm_get_state(void)
{
    return current_state;
}

//
void fsm_update_sensor_data(const SensorData_t *data)
{
    sensors = *data;
}

//
void fsm_set_state(State_t state)
{
    current_state = state;
}

//
SensorData_t* fsm_sensor(void)
{
    return &sensors;
}

//
void fsm_tick(Event_t evt)
{
    switch(current_state)
    {
        case STATE_BOOT:
            state_boot(evt);
            break;

        case STATE_IDLE:
            state_idle(evt);
            break;

        case STATE_ARMING:
            state_arming(evt);
            break;

        case STATE_ARMED:
            state_armed(evt);
            break;

        case STATE_TAKEOFF:
            state_takeoff(evt);
            break;

        case STATE_HOVER:
            state_hover(evt);
            break;
        
        case STATE_MISSION:
            state_misssion(evt);
            break;

        case STATE_RTL:
            state_rtl(evt);
            break;
        
        case STATE_LAND:
            state_land(evt);
            break;

        case STATE_EMERGENCY:
            state_emergency(evt);
            break;
        
        case STATE_FAILSAFE:
            state_failsafe(evt);
            break;
            
    }
}