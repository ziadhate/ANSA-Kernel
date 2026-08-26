#include "fsm.h"

extern void fsm_set_state(State_t);
extern bool global_transition(Event_t);
extern bool guard_arm(void);
extern bool guard_takeoff(void);
extern bool guard_hover_altitude(void);
extern bool guard_landed(void);
extern void action_arm(void);
extern void action_disarm(void);
extern void action_takeoff(void);
extern void action_hover(void);
extern void action_start_mission(void);
extern void action_start_rtl(void);
extern void action_land(void);
extern void action_emergency(void);
extern void action_failsafe(void);

//
void state_boot(Event_t evt)
{
    if (global_transition(evt))
        return;
    if(evt == EVT_INIT_OK)
    {
        fsm_set_state(STATE_IDLE);
    }
}

//
void state_idle(
    Event_t evt)
{
    if(global_transition(evt))
        return;
    if(evt ==
       EVT_ARM_CMD)
    {
        fsm_set_state(
            STATE_ARMING);
    }
}

//
void state_arming(
      Event_t evt)
{
    if(global_transition(evt))
        return;

    if(guard_arm())
    {
        action_arm();

        fsm_set_state(
            STATE_ARMED);
    }
    else
    {
        fsm_set_state(
            STATE_IDLE);
    }
}

//
void state_armed(
     Event_t evt)
{
    if(global_transition(evt))
        return;

    if(evt ==
       EVT_TAKEOFF_CMD

       &&

       guard_takeoff())
    {
        action_takeoff();

        fsm_set_state(
            STATE_TAKEOFF);
    }
}

//
void state_takeoff(
      Event_t evt)
{
    if(global_transition(evt))
        return;

    if(guard_hover_altitude())
    {
        action_hover();

        fsm_set_state(
            STATE_HOVER);
    }
}

//
void state_hover(
      Event_t evt)
{
    if(global_transition(evt))
        return;

    if(evt ==
       EVT_MISSION_CMD)
    {
        action_start_mission();

        fsm_set_state(
            STATE_MISSION);
    }
}

//
void state_mission(
      Event_t evt)
{
    if(global_transition(evt))
        return;

    if(evt ==
       EVT_MISSION_COMPLETE)
    {
        action_start_rtl();

        fsm_set_state(
            STATE_RTL);
    }
}

//
void state_rtl(
      Event_t evt)
{
    if(global_transition(evt))
        return;

    if(evt ==
       EVT_HOME_REACHED)
    {
        action_land();

        fsm_set_state(
            STATE_LAND);
    }
}

//
void state_land(
      Event_t evt)
{
    if(global_transition(evt))
        return;

    if(guard_landed())
    {
        action_disarm();

        fsm_set_state(
            STATE_IDLE);
    }
}

//
void state_emergency(
      Event_t evt)
{
    action_emergency();

    fsm_set_state(
        STATE_LAND);
}

//
void state_failsafe(
      Event_t evt)
{
    action_failsafe();
}