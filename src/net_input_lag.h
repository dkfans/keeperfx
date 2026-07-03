#ifndef INPUT_LAG_H
#define INPUT_LAG_H

#include "globals.h"
#include "bflib_basics.h"

#define MAXIMUM_INPUT_LAG_TURNS 12

#ifdef __cplusplus
extern "C" {
#endif

TbBool input_lag_skips_initial_processing(void);
unsigned short calculate_skip_input(void);
void LbNetwork_UpdateInputLagIfHost(void);

#ifdef __cplusplus
}
#endif

#endif
