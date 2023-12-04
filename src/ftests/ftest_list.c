#include "../globals.h"

#ifdef FUNCTESTING

#include "../pre_inc.h"

#include "ftest.h"

/**
 * Add the header files for your tests here
 */
#include "tests/ftest_template.h"
#include "tests/ftest_bug_imp_tp_job_attack_door.h"
#include "tests/ftest_bug_pathing_pillar_circling.h"
#include "tests/ftest_bug_imp_goldseam_dig.h"
#include "tests/ftest_bug_invisible_units_cant_select.h"
//#include "tests/ftest_your_test_header.h"

#include "../post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Append the name/init function of your test here so it can be found/executed.
 */
struct ftest_onlyappendtests__config ftest_onlyappendtests__conf = {
    .tests_list = {
        { "example_template_test", ftest_template_init },
        { "bug_imp_teleport_attack_door", ftest_bug_imp_tp_attack_door_init },
        { "bug_pathing_pillar_circling", ftest_bug_pathing_pillar_circling_init },
        { "bug_imp_goldseam_dig", ftest_bug_imp_goldseam_dig_init },
        { "bug_invisible_units_cant_select", ftest_bug_invisible_units_cant_select_init }
    }
};


#ifdef __cplusplus
}
#endif

#endif

