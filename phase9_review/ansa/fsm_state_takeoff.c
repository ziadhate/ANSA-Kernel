#include "fsm_internal.h"

static void command_takeoff(void)
{
    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_POSITION;
    g_command.latitude_deg = g_sensor.latitude_deg;
    g_command.longitude_deg = g_sensor.longitude_deg;
    g_command.altitude_m = FSM_TAKEOFF_ALTITUDE_M;
}

void fsm_state_takeoff_enter(uint32_t now_us)
{
    (void)now_us;
    command_takeoff();
}

void fsm_state_takeoff_periodic(uint32_t now_us)
{
    (void)now_us;
    command_takeoff();
}

void fsm_state_takeoff_exit(void)
{
}
