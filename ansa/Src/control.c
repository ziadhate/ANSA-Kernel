#include <stdio.h>
#include "control.h"

void control_init(void)
{
    printf("[CONTROL] Initialized\n");
}

void control_arm(void)
{
    printf("[CONTROL] Motors Armed\n");
}

void control_disarm(void)
{
    printf("[CONTROL] Motors Disarmed\n");
}

void control_takeoff(void)
{
    printf("[CONTROL] Takeoff initiated\n");
}

void control_hover(void)
{
    printf("[CONTROL] Hovering\n");
}

void control_land(void)
{
    printf("[CONTROL] Landing initiated\n");
}

void control_emergency(void)
{
    printf("[CONTROL] Emergency landing initiated\n");
}
