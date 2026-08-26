#include "sensor.h"
static SensorData_t sensors;

//
void sensor_init(void)
{
    sensors.altitude_m = 0;
    sensors.heading_deg = 0;
    sensors.battery_percent = 100;
    sensors.imu_ok = true;
    sensors.mag_ok = true;
    sensors.baro_ok = true;
}

//
void sensor_task(void)
{
    /* BAROMETER */
    if(sensors.altitude_m < 5.0f)
    {
        sensors.altitude_m += 0.25f;
    }

    /* MAGNETOMETER */
    sensors.heading_deg += 2.0f;
    if(sensors.heading_deg >= 360)
    {
        sensors.heading_deg -= 360;
    }

    /* BATTERY */
    sensors.battery_percent -= 0.05f;
    if(sensors.battery_percent < 0)
    {
        sensors.battery_percent = 0;
    }
}

//
SensorData_t sensor_get_data(void)
{
    return sensors;
}