#ifndef SPACEBORN_FSM_CONFIG_H
#define SPACEBORN_FSM_CONFIG_H

#define FSM_TICK_HZ                       50U
#define FSM_TICK_PERIOD_US                20000U
#define FSM_MAX_EVENTS_PER_TICK          4U
#define FSM_EVENT_QUEUE_DEPTH             32U
#define FSM_MAX_WAYPOINTS                 64U
#define FSM_WCET_BUDGET_US                200U

/* Vehicle-specific defaults: validate on the real airframe before flight. */
#define FSM_TAKEOFF_ALTITUDE_M            2.0f
#define FSM_TAKEOFF_ALT_TOLERANCE_M       0.20f
#define FSM_MIN_ARMED_TIME_MS             1000U
#define FSM_MIN_BATTERY_PERCENT_ARM       30.0f
#define FSM_MIN_GPS_FIX_TYPE              2U
#define FSM_RTL_ALTITUDE_M                5.0f
#define FSM_HOME_RADIUS_M                 2.0f
#define FSM_LAND_ALT_THRESHOLD_M          0.5f
#define FSM_FAILSAFE_TIMEOUT_MS           2000U
#define FSM_BATTERY_CRITICAL_PERCENT     10.0f
#define FSM_BATTERY_CRITICAL_SAMPLES      5U
#define FSM_SENSOR_STALE_TIMEOUT_MS       100U
#define FSM_DEFAULT_ACCEPT_RADIUS_M       2.0f
#define FSM_MAX_LOITER_MS                 86400000UL

#define FSM_EVENT_PRIORITY_SAFETY         0U
#define FSM_EVENT_PRIORITY_MAJOR          1U
#define FSM_EVENT_PRIORITY_CONTROL        2U
#define FSM_EVENT_PRIORITY_MISSION        3U

#define FSM_STRUCT_VERSION                1U

#endif
