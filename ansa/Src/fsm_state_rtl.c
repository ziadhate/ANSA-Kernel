#include "fsm_internal.h"

static void command_rtl(uint8_t substate)
{
    g_command.version = FSM_STRUCT_VERSION;

    if (substate == FSM_RTL_DESCEND) {
        g_command.mode = FSM_CTRL_LAND;
        return;
    }

    g_command.mode = FSM_CTRL_POSITION;
    g_command.latitude_deg = g_sensor.latitude_deg;
    g_command.longitude_deg = g_sensor.longitude_deg;
    g_command.altitude_m = FSM_RTL_ALTITUDE_M;
}

void fsm_state_rtl_enter(uint8_t substate, uint32_t now_us)
{
    (void)now_us;
    g_mission.active = false;
    g_mission.paused = false;
    command_rtl(substate);
}

void fsm_state_rtl_periodic(uint8_t substate, uint32_t now_us)
{
    (void)now_us;
    command_rtl(substate);
}

void fsm_state_rtl_exit(void)
{
}
