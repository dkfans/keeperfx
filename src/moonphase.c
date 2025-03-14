#include "pre_inc.h"
#include "moonphase.h"
#include <astronomy.h>
#include "post_inc.h"

short is_full_moon = 0;
short is_near_full_moon = 0;
short is_new_moon = 0;
short is_near_new_moon = 0;


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
static double moonphase_calculate()
{
    // Get current time
    astro_time_t time = Astronomy_CurrentTime();

    // Get phase of the moon
    astro_angle_result_t phase = Astronomy_MoonPhase(time);
    if (phase.status != ASTRO_SUCCESS)
    {
        return -1;
    }

    return phase.angle / 360;
}

short calculate_moon_phase(short do_calculate, short add_to_log)
{
    // Moon phase calculation
    if (do_calculate)
    {
        phase_of_moon = (float)moonphase_calculate();
    }

    // Handle moon phases
    if ((phase_of_moon > 0.475) && (phase_of_moon < 0.525)) // Approx 33 hours
    {
        if (add_to_log)
        {
            SYNCMSG("Full moon %.4f", phase_of_moon);
        }

        is_full_moon = 1;
        is_near_full_moon = 0;
        is_new_moon = 0;
        is_near_new_moon = 0;
    }
    else if ((phase_of_moon > 0.45) && (phase_of_moon < 0.55)) // Approx 70 hours
    {
        if (add_to_log)
        {
            SYNCMSG("Near Full moon %.4f", phase_of_moon);
        }

        is_full_moon = 0;
        is_near_full_moon = 1;
        is_new_moon = 0;
        is_near_new_moon = 0;
    }
    else if ((phase_of_moon < 0.025) || (phase_of_moon > 0.975))
    {
        if (add_to_log)
        {
            SYNCMSG("New moon %.4f", phase_of_moon);
        }

        is_full_moon = 0;
        is_near_full_moon = 0;
        is_new_moon = 1;
        is_near_new_moon = 0;
    }
    else if ((phase_of_moon < 0.05) || (phase_of_moon > 0.95))
    {
        if (add_to_log)
        {
            SYNCMSG("Near new moon %.4f", phase_of_moon);
        }

        is_full_moon = 0;
        is_near_full_moon = 0;
        is_new_moon = 0;
        is_near_new_moon = 1;
    }
    else
    {
        if (add_to_log)
        {
            SYNCMSG("Moon phase %.4f", phase_of_moon);
        }

        is_full_moon = 0;
        is_near_full_moon = 0;
        is_new_moon = 0;
        is_near_new_moon = 0;
    }

    //! CHEAT! always show extra levels
    // TODO: make this a command line option?
    //  is_full_moon = 1; is_new_moon = 1;

    return is_full_moon;
}