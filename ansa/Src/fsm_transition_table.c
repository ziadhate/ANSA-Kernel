#include "fsm.h"

extern void fsm_set_state(State_t state);

bool global_transition(Event_t evt)
{
    if (evt == EVT_SENSOR_FAIL)
    {
        fsm_set_state(STATE_FAILSAFE);
        return true;
    }

    if (evt == EVT_SENSOR_FAIL)
     {
        fsm_set_state(STATE_RTL);
        return true;
    }

    if (evt == EVT_BATTERY_CRITICAL)
    {
        fsm_set_state(STATE_EMERGENCY);
        return true;
    }
    return false;
}