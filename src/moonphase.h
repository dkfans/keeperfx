#ifndef MOONPHASE_H
#define MOONPHASE_H

#ifdef __cplusplus
extern "C"
{
#endif

    extern short is_full_moon;
    extern short is_near_full_moon;
    extern short is_new_moon;
    extern short is_near_new_moon;
    
    short calculate_moon_phase(short do_calculate, short add_to_log)

#ifdef __cplusplus
}
#endif

#endif // MOONPHASE_H
