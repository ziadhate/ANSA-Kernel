#include "fsm_internal.h"

static void command_hold(void)
{
    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_POSITION;
    g_command.latitude_deg = g_sensor.latitude_deg;
    g_command.longitude_deg = g_sensor.longitude_deg;
    g_command.altitude_m = g_sensor.altitude_m;
}

static void command_waypoint(void)
{
    if (!g_mission.loaded ||
        g_mission.count == 0U ||
        g_mission.index >= g_mission.count) {
        command_hold();
        return;
    }

    const fsm_waypoint_t *wp = &g_mission.items[g_mission.index];

    g_command.version = FSM_STRUCT_VERSION;
    g_command.mode = FSM_CTRL_POSITION;
    g_command.latitude_deg = wp->latitude_deg;
    g_command.longitude_deg = wp->longitude_deg;
    g_command.altitude_m = wp->altitude_m;
}

void fsm_state_mission_enter(uint8_t substate, uint32_t now_us)
{
    g_mission.active = true;
    g_mission.paused = (substate == FSM_MISSION_PAUSED);

    if (g_mission.index >= g_mission.count)
        g_mission.index = 0U;

    if (substate == FSM_MISSION_LOITER)
        g_mission.loiter_start_us = now_us;

    if (g_mission.paused)
        command_hold();
    else
        command_waypoint();

    fsm_mission_runtime_on_start();
}

void fsm_state_mission_periodic(uint8_t substate, uint32_t now_us)
{
    (void)now_us;

    if (!g_mission.loaded || !g_mission.active) {
        command_hold();
        return;
    }

    if (substate == FSM_MISSION_PAUSED || g_mission.paused)
        command_hold();
    else
        command_waypoint();
}

void fsm_state_mission_exit(void)
{
    g_mission.active = false;
    g_mission.paused = false;
    fsm_mission_runtime_on_stop();
}
