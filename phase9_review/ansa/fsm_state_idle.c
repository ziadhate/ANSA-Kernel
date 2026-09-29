#include "fsm_internal.h"

void fsm_state_idle_enter(uint32_t now_us)
{
    (void)now_us;
    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_DISARMED;
    g_mission.active = false;
    g_mission.paused = false;
}

void fsm_state_idle_periodic(uint32_t now_us)
{
    (void)now_us;
    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_DISARMED;
}

void fsm_state_idle_exit(void)
{
}
