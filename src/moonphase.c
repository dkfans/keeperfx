#include "moonphase.h"

/**
 * Calculate the moon phase.
 *
 * 0.0  = new moon,
 * 0.25 = first quarter,
 * 0.5  = full moon,
 * 0.75 = third quarter
 *
 * Returns -1 on failure
 */
double moonphase_calculate()
{
    // Get current time
    astro_time_t time = Astronomy_CurrentTime();

    // Get phase of the moon
    astro_angle_result_t phase = Astronomy_MoonPhase(time);
    if (phase.status != ASTRO_SUCCESS)
    {
        WARNLOG("Failed to get moon phase");
        return -1;
    }

    return phase.angle / 360;
}