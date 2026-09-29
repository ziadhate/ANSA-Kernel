#include <stdbool.h>
#include "fsm.h"

extern SensorData_t* fsm_sensor(void);

bool guard_arm(void)
{
    SensorData_t *s = fsm_sensor();

    return 
    s->battery_percentage > 20.0f &&
    s->imu_ok &&
    s->mag_ok &&
    s->baro_ok ;
}

bool guard_takeoff(void)
{
    SensorData_t *s = fsm_sensor();

    return
    s->battery_percentage > 30.0f;
}

bool guard_hover_altitude(void)
{
    SensorData_t *s = fsm_sensor();

    return s->altitude_m > 2.0f;
}

bool guard_landed(void)
{
    SensorData_t *s = fsm_sensor();
    return s->altitude_m <= 0.3f;
}