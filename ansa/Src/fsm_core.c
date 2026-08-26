#include <math.h>
#include <string.h>
#include "fsm_internal.h"

uint8_t fsm_event_priority(fsm_event_type_t e)
{
    switch (e) {
        case EVT_SENSOR_FAULT:
        case EVT_WATCHDOG_TIMEOUT:
        case EVT_BATTERY_CRITICAL:
            return FSM_EVENT_PRIORITY_SAFETY;
        case EVT_LINK_LOST:
        case EVT_GEOFENCE_BREACH:
            return FSM_EVENT_PRIORITY_MAJOR;
        case EVT_RTL_REQUEST:
        case EVT_LAND_REQUEST:
        case EVT_MISSION_ABORT:
            return FSM_EVENT_PRIORITY_CONTROL;
        default:
            return FSM_EVENT_PRIORITY_MISSION;
    }
}

static bool queue_push(const fsm_event_t *e)
{
    if (e == NULL || e->version != FSM_STRUCT_VERSION ||
        e->type >= EVT_COUNT)
        return false;

    if (g_events.count >= FSM_EVENT_QUEUE_DEPTH) {
        ++g_events.overflow_count;
        g_fsm.fault_flags |= (1UL << FSM_FAULT_QUEUE_OVERFLOW);
        return false;
    }

    g_events.items[g_events.count++] = *e;
    return true;
}

bool fsm_queue_pop(fsm_event_t *out)
{
    if (out == NULL || g_events.count == 0U)
        return false;

    uint8_t best = 0U;
    uint8_t best_priority =
        fsm_event_priority(g_events.items[0].type);

    for (uint8_t i = 1U; i < g_events.count; ++i) {
        uint8_t p = fsm_event_priority(g_events.items[i].type);
        if (p < best_priority) {
            best = i;
            best_priority = p;
        }
    }

    *out = g_events.items[best];

    for (uint8_t i = best; i + 1U < g_events.count; ++i)
        g_events.items[i] = g_events.items[i + 1U];

    --g_events.count;
    return true;
}

bool fsm_post_event(const fsm_event_t *event)
{
    return queue_push(event);
}

void fsm_state_enter(fsm_state_t state, uint8_t substate, uint32_t now_us)
{
    switch (state) {
        case FSM_STATE_BOOT: fsm_state_boot_enter(now_us); break;
        case FSM_STATE_IDLE: fsm_state_idle_enter(now_us); break;
        case FSM_STATE_ARMING: fsm_state_arming_enter(now_us); break;
        case FSM_STATE_ARMED: fsm_state_armed_enter(now_us); break;
        case FSM_STATE_TAKEOFF: fsm_state_takeoff_enter(now_us); break;
        case FSM_STATE_HOVER: fsm_state_hover_enter(now_us); break;
        case FSM_STATE_MISSION: fsm_state_mission_enter(substate, now_us); break;
        case FSM_STATE_RTL: fsm_state_rtl_enter(substate, now_us); break;
        case FSM_STATE_LAND: fsm_state_land_enter(now_us); break;
        case FSM_STATE_EMERGENCY: fsm_state_emergency_enter(now_us); break;
        case FSM_STATE_FAILSAFE: fsm_state_failsafe_enter(now_us); break;
        default: break;
    }

    g_fsm.entry_us = now_us;
}

void fsm_state_exit(fsm_state_t state, uint8_t substate)
{
    (void)substate;

    switch (state) {
        case FSM_STATE_BOOT: fsm_state_boot_exit(); break;
        case FSM_STATE_IDLE: fsm_state_idle_exit(); break;
        case FSM_STATE_ARMING: fsm_state_arming_exit(); break;
        case FSM_STATE_ARMED: fsm_state_armed_exit(); break;
        case FSM_STATE_TAKEOFF: fsm_state_takeoff_exit(); break;
        case FSM_STATE_HOVER: fsm_state_hover_exit(); break;
        case FSM_STATE_MISSION: fsm_state_mission_exit(); break;
        case FSM_STATE_RTL: fsm_state_rtl_exit(); break;
        case FSM_STATE_LAND: fsm_state_land_exit(); break;
        case FSM_STATE_EMERGENCY: fsm_state_emergency_exit(); break;
        case FSM_STATE_FAILSAFE: fsm_state_failsafe_exit(); break;
        default: break;
    }
}

bool fsm_apply_transition(
    fsm_state_t to_state,
    uint8_t to_substate,
    fsm_transition_reason_t reason,
    uint32_t now_us)
{
    if (to_state >= FSM_STATE_COUNT)
        return false;

    if (g_fsm.state == to_state &&
        g_fsm.substate == to_substate)
        return true;

    fsm_state_t old_state = g_fsm.state;
    uint8_t old_substate = g_fsm.substate;

    fsm_state_exit(old_state, old_substate);

    g_fsm.previous = old_state;
    g_fsm.state = to_state;
    g_fsm.substate = to_substate;
    g_fsm.last_reason = reason;
    ++g_fsm.transitions;

    fsm_state_enter(to_state, to_substate, now_us);
    return true;
}

void fsm_state_periodic(
    fsm_state_t state,
    uint8_t substate,
    uint32_t now_us)
{
    switch (state) {
        case FSM_STATE_BOOT: fsm_state_boot_periodic(now_us); break;
        case FSM_STATE_IDLE: fsm_state_idle_periodic(now_us); break;
        case FSM_STATE_ARMING: fsm_state_arming_periodic(now_us); break;
        case FSM_STATE_ARMED: fsm_state_armed_periodic(now_us); break;
        case FSM_STATE_TAKEOFF: fsm_state_takeoff_periodic(now_us); break;
        case FSM_STATE_HOVER: fsm_state_hover_periodic(now_us); break;
        case FSM_STATE_MISSION: fsm_state_mission_periodic(substate, now_us); break;
        case FSM_STATE_RTL: fsm_state_rtl_periodic(substate, now_us); break;
        case FSM_STATE_LAND: fsm_state_land_periodic(now_us); break;
        case FSM_STATE_EMERGENCY: fsm_state_emergency_periodic(now_us); break;
        case FSM_STATE_FAILSAFE: fsm_state_failsafe_periodic(now_us); break;
        default: break;
    }
}

static void process_event(const fsm_event_t *e, uint32_t now_us)
{
    fsm_state_t to_state;
    uint8_t to_substate;
    fsm_transition_reason_t reason;
    fsm_event_type_t lookup_event;

    if (e == NULL || e->version != FSM_STRUCT_VERSION || e->type >= EVT_COUNT)
        return;

    g_fsm.last_event = e->type;
    ++g_fsm.consumed_events;

    lookup_event = e->type;

    if (e->type == EVT_INTERNAL_TICK &&
        g_fsm.state == FSM_STATE_MISSION &&
        g_fsm.substate == FSM_MISSION_WAYPOINT_NAV &&
        guard_waypoint_reached()) {
        lookup_event = EVT_WAYPOINT_REACHED;
    }

    if (fsm_table_lookup(g_fsm.state, g_fsm.substate,
                         lookup_event, &to_state, &to_substate, &reason)) {

        if (lookup_event == EVT_WAYPOINT_REACHED &&
            g_fsm.state == FSM_STATE_MISSION &&
            g_fsm.substate == FSM_MISSION_WAYPOINT_NAV &&
            g_mission.index < g_mission.count) {

            /*
             * Loiter belongs to the waypoint just reached. Do not advance
             * until its loiter interval has completed.
             */
            if (g_mission.items[g_mission.index].loiter_time_ms > 0U) {
                g_mission.loiter_start_us = now_us;
                to_state = FSM_STATE_MISSION;
                to_substate = FSM_MISSION_LOITER;
                reason = FSM_REASON_WAYPOINT_REACHED;
            } else {
                ++g_mission.index;
            }
        } else if (g_fsm.state == FSM_STATE_MISSION &&
                   g_fsm.substate == FSM_MISSION_LOITER &&
                   lookup_event == EVT_INTERNAL_TICK &&
                   g_mission.index < g_mission.count) {
            ++g_mission.index;
        }

        (void)fsm_apply_transition(to_state, to_substate, reason, now_us);
    }
}

static void generate_internal_events(uint32_t now_us)
{
    fsm_event_t e = {
        FSM_STRUCT_VERSION,
        EVT_INTERNAL_TICK,
        now_us,
        0U
    };
    (void)queue_push(&e);
}

void fsm_tick(uint32_t timestamp_us)
{
    /*
     * Sensor snapshot must have been supplied before the tick.
     * No synchronous Sensor Stack calls occur here.
     */
    if (g_sensor.timestamp_us != timestamp_us)
        g_sensor.timestamp_us = timestamp_us;

    /*
     * Safety events are already priority-ranked by the bounded queue.
     * Only the configured maximum number are consumed per tick.
     */
    for (uint8_t i = 0U; i < FSM_MAX_EVENTS_PER_TICK; ++i) {
        fsm_event_t e;
        if (!fsm_queue_pop(&e))
            break;
        process_event(&e, timestamp_us);
    }

    /*
     * Every guard-only/internal transition goes through the same
     * transition table, as required by the architecture.
     */
    generate_internal_events(timestamp_us);

    for (uint8_t i = 0U; i < FSM_MAX_EVENTS_PER_TICK; ++i) {
        fsm_event_t e;
        if (!fsm_queue_pop(&e))
            break;
        process_event(&e, timestamp_us);
    }

    fsm_state_periodic(g_fsm.state, g_fsm.substate, timestamp_us);

    g_telemetry.version = FSM_STRUCT_VERSION;
    g_telemetry.state = g_fsm.state;
    g_telemetry.previous_state = g_fsm.previous;
    g_telemetry.last_event = g_fsm.last_event;
    g_telemetry.last_reason = g_fsm.last_reason;
    g_telemetry.current_waypoint = g_mission.index;
    g_telemetry.waypoint_count = g_mission.count;
    g_telemetry.mission_substate =
        (g_fsm.state == FSM_STATE_MISSION)
            ? (fsm_mission_substate_t)g_fsm.substate
            : FSM_MISSION_WAYPOINT_NAV;
    g_telemetry.rtl_substate =
        (g_fsm.state == FSM_STATE_RTL)
            ? (fsm_rtl_substate_t)g_fsm.substate
            : FSM_RTL_CLIMB;
    g_telemetry.state_entry_us = g_fsm.entry_us;
    g_telemetry.state_time_us = timestamp_us - g_fsm.entry_us;
    g_telemetry.transition_count = g_fsm.transitions;
    g_telemetry.fault_flags = g_fsm.fault_flags;
    g_telemetry.queue_overflow_count = g_events.overflow_count;
    g_telemetry.consumed_event_count = g_fsm.consumed_events;
    g_telemetry.timestamp_us = timestamp_us;

    fsm_control_publish(&g_command);
    fsm_telemetry_publish(&g_telemetry);
}

void fsm_init(void)
{
    memset(&g_fsm, 0, sizeof(g_fsm));
    memset(&g_sensor, 0, sizeof(g_sensor));
    memset(&g_command, 0, sizeof(g_command));
    memset(&g_telemetry, 0, sizeof(g_telemetry));
    memset(&g_events, 0, sizeof(g_events));
    memset(&g_mission, 0, sizeof(g_mission));

    g_fsm.state = FSM_STATE_BOOT;
    g_fsm.previous = FSM_STATE_BOOT;
    g_fsm.substate = 0U;
    g_fsm.last_event = EVT_NONE;
    g_fsm.last_reason = FSM_REASON_NONE;

    fsm_state_enter(FSM_STATE_BOOT, 0U, 0U);
}

void fsm_set_sensor_snapshot(
    const fsm_sensor_snapshot_t *snapshot)
{
    if (snapshot == NULL ||
        snapshot->version != FSM_STRUCT_VERSION)
        return;

    /*
     * Basic range rejection before guards use the snapshot.
     */
    if (!isfinite(snapshot->latitude_deg) ||
        !isfinite(snapshot->longitude_deg) ||
        !isfinite(snapshot->altitude_m) ||
        !isfinite(snapshot->battery_percent) ||
        snapshot->latitude_deg < -90.0f ||
        snapshot->latitude_deg > 90.0f ||
        snapshot->longitude_deg < -180.0f ||
        snapshot->longitude_deg > 180.0f ||
        snapshot->battery_percent < 0.0f ||
        snapshot->battery_percent > 100.0f)
        return;

    g_sensor = *snapshot;
}

bool fsm_mission_load(
    const fsm_waypoint_t *waypoints,
    uint16_t count)
{
    if (waypoints == NULL ||
        count == 0U ||
        count > FSM_MAX_WAYPOINTS)
        return false;

    if (g_fsm.state != FSM_STATE_IDLE &&
        g_fsm.state != FSM_STATE_HOVER)
        return false;

    for (uint16_t i = 0U; i < count; ++i) {
        const fsm_waypoint_t *wp = &waypoints[i];

        if (!isfinite(wp->latitude_deg) ||
            !isfinite(wp->longitude_deg) ||
            !isfinite(wp->altitude_m) ||
            !isfinite(wp->acceptance_radius_m))
            return false;

        if (wp->latitude_deg < -90.0f ||
            wp->latitude_deg > 90.0f ||
            wp->longitude_deg < -180.0f ||
            wp->longitude_deg > 180.0f ||
            wp->altitude_m < 0.0f ||
            wp->acceptance_radius_m <= 0.0f ||
            wp->loiter_time_ms > FSM_MAX_LOITER_MS)
            return false;
    }

    memcpy(g_mission.items, waypoints,
           sizeof(fsm_waypoint_t) * count);

    g_mission.count = count;
    g_mission.index = 0U;
    g_mission.loaded = true;
    g_mission.active = false;
    g_mission.paused = false;
    return true;
}

bool fsm_mission_clear(void)
{
    if (g_mission.active)
        return false;

    memset(&g_mission, 0, sizeof(g_mission));
    return true;
}

static bool post_simple(fsm_event_type_t type)
{
    fsm_event_t e = {
        FSM_STRUCT_VERSION,
        type,
        g_sensor.timestamp_us,
        0U
    };
    return fsm_post_event(&e);
}

bool fsm_mission_start(void)  { return post_simple(EVT_MISSION_START); }
bool fsm_mission_pause(void)  { return post_simple(EVT_MISSION_PAUSE); }
bool fsm_mission_resume(void) { return post_simple(EVT_MISSION_RESUME); }
bool fsm_mission_abort(void)  { return post_simple(EVT_MISSION_ABORT); }

fsm_state_t fsm_get_state(void) { return g_fsm.state; }

fsm_mission_substate_t fsm_get_mission_substate(void)
{
    return (fsm_mission_substate_t)g_fsm.substate;
}

fsm_rtl_substate_t fsm_get_rtl_substate(void)
{
    return (fsm_rtl_substate_t)g_fsm.substate;
}

bool fsm_mission_is_loaded(void) { return g_mission.loaded; }
bool fsm_mission_is_running(void) { return g_mission.active; }
uint16_t fsm_mission_current_index(void) { return g_mission.index; }
uint16_t fsm_mission_count(void) { return g_mission.count; }

void fsm_get_control_command(fsm_control_command_t *out)
{
    if (out != NULL)
        *out = g_command;
}

void fsm_get_telemetry(fsm_telemetry_t *out)
{
    if (out != NULL)
        *out = g_telemetry;
}
