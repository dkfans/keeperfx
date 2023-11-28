#include "globals.h"

#ifdef FUNCTESTING

#include "pre_inc.h"

#include "ftest.h"

/**
 * Add the header files for your tests here
 */
#include "ftest_bug_imp_tp_job_attack_door.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Append the init function of your test here to access it via the -level argument.
 * the -level argument will be converted to a 0 based index for this list
 */
FTest_Init_Func ftest_init_func_list[FTEST_TESTS_MAX] = {
    ftest_bug_imp_tp_attack_door_init
};

#ifdef __cplusplus
}
#endif

#endif

