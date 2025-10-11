#pragma once

#include "../../globals.h"

#ifdef FUNCTESTING

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char TbBool;

/**
 * @brief Rename this init function to reflect the name of your test, eg: ftest_myexampletest_init
 * 
 */
TbBool ftest_bug_pathing_stair_treasury_init();


#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING
