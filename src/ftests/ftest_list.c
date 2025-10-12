#include "../globals.h"

#ifdef FUNCTESTING

#include "../pre_inc.h"

#include "ftest.h"

/**
 * Add the header files for all tests below here
 */
#include "tests/ftest_template.h"
#include "tests/ftest_bug_imp_tp_job_attack_door.h"
#include "tests/ftest_bug_pathing_pillar_circling.h"
#include "tests/ftest_bug_imp_goldseam_dig.h"
#include "tests/ftest_bug_invisible_units_cant_select.h"
#include "tests/ftest_bug_pathing_stair_treasury.h"
#include "tests/ftest_bug_ai_bridge.h"
// append your test include here, eg: #include "tests/ftest_your_test_header.h"

#include "../post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Append the name/init function of your test here so it can be found/executed.
 */
struct ftest_onlyappendtests__config ftest_onlyappendtests__conf = {

    // place regular tests in this list
    .tests_list = {
         { .test_name="example_template_test",              .init_func=ftest_template_init,                         .level_file="keeporig", .level=8,  .fastforward_speed=8 },
         { .test_name="bug_imp_tp_attack_door__claim",      .init_func=ftest_bug_imp_tp_attack_door__claim_init,    .level_file="deepdngn", .level=80, .fastforward_speed=8 },
         { .test_name="bug_imp_tp_attack_door__prisoner",   .init_func=ftest_bug_imp_tp_attack_door__prisoner_init, .level_file="deepdngn", .level=80, .fastforward_speed=8 },
         { .test_name="bug_imp_tp_attack_door__deadbody",   .init_func=ftest_bug_imp_tp_attack_door__deadbody_init, .level_file="deepdngn", .level=80, .fastforward_speed=8 },
         { .test_name="bug_imp_goldseam_dig",               .init_func=ftest_bug_imp_goldseam_dig_init,             .level_file="keeporig", .level=1,  .fastforward_speed=8 },
         { .test_name="bug_pathing_stair_treasury",         .init_func=ftest_bug_pathing_stair_treasury_init,       .level_file="keeporig", .level=1,  .fastforward_speed=8 },
         { .test_name="bug_invisible_units_cant_select",    .init_func=ftest_bug_invisible_units_cant_select_init,  .level_file="keeporig", .level=1,  .fastforward_speed=0 },

         // WIP TEST { .test_name="bug_pathing_pillar_circling",        .init_func=ftest_bug_pathing_pillar_circling_init,      .level_file="keeporig", .level=1, .fastforward_speed=0 },
         // WIP TEST { .test_name="bug_invisible_units_cant_select",    .init_func=ftest_bug_invisible_units_cant_select_init,  .level_file="lostlvls", .level=103, .fastforward_speed=0 },
         // append your test to tests_list here, eg: { .test_name="your_test_name",    .init_func=ftest_your_test_name_init, .level_file="lostlvls", .level=103 },
    },

    // place long-running tests in this list, to include them use the -includelongtests flag
    .long_running_tests_list = {
        { .test_name="bug_ai_bridge",                      .init_func=ftest_bug_ai_bridge_init,                    .level_file="keeporig", .level=15, .fastforward_speed=128, .seed=1, .repeat_n_times=100 },
    }
};


#ifdef __cplusplus
}
#endif

#endif

