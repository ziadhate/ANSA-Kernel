#include "fsm_internal.h"

void fsm_state_emergency_enter(uint32_t now_us)
{
    (void)now_us;
    g_mission.active = false;
    g_mission.paused = false;
    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_LAND;
    fsm_mission_runtime_on_stop();
}

void fsm_state_emergency_periodic(uint32_t now_us)
{
    (void)now_us;
    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_LAND;
}

void fsm_state_emergency_exit(void)
{
}
