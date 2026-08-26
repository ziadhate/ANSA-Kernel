#include <stdio.h>

void action_arm(void)
{
    printf("ARM\n");
}

void action_disarm(void)
{
    printf("DISARM\n");
}

void action_takeoff(void)
{
    printf("TAKEOFF\n");
}

void action_hover(void)
{
    printf("HOVER\n");
}

void action_mission(void)
{
    printf("MISSION START\n");
}

void action_start_rtl(void)
{
    printf("START RTL\n");
}

void action_land(void)
{
    printf("LANDING\n");
}

void action_emergency(void)
{
    printf("EMERGENCY\n");
}

void action_failsafe(void)
{
    printf("FAILSAFE\n");
}
