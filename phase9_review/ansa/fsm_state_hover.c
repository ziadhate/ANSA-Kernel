#include "fsm_internal.h"

static void command_hover(void)
{
    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_POSITION;
    g_command.latitude_deg = g_sensor.latitude_deg;
    g_command.longitude_deg = g_sensor.longitude_deg;
    g_command.altitude_m = g_sensor.altitude_m;
}

void fsm_state_hover_enter(uint32_t now_us)
{
    (void)now_us;
    g_mission.active = false;
    g_mission.paused = false;
    command_hover();
}

void fsm_state_hover_periodic(uint32_t now_us)
{
    (void)now_us;
    command_hover();
}

void fsm_state_hover_exit(void)
{
}
