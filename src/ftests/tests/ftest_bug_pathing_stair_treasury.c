#include "ftest_bug_pathing_pillar_circling.h"

#ifdef FUNCTESTING

#include "../../pre_inc.h"

#include "../ftest.h"
#include "../ftest_util.h"

#include "../../game_legacy.h"
#include "../../keeperfx.hpp"
#include "../../player_instances.h"

#include "../../post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

// example of test variables wraped in a struct, this prevents variable name collisions with other tests, allowing you to name your variables how you like!
struct ftest_bug_pathing_stair_treasury__variables
{
    const MapSlabCoord slb_x_stair_start;
    const MapSlabCoord slb_y_stair_start;

    const unsigned short stair_length;
    const unsigned short stair_count;

    const MapSlabCoord slb_x_second_dig_action;
    const MapSlabCoord slb_y_second_dig_action;
};
struct ftest_bug_pathing_stair_treasury__variables ftest_bug_pathing_stair_treasury__vars = {
    .slb_x_stair_start = 42,
    .slb_y_stair_start = 47,

    .stair_length = 2,
    .stair_count = 5,

    .slb_x_second_dig_action = 42,
    .slb_y_second_dig_action = 39
};



// forward declarations - tests
FTestActionResult ftest_bug_pathing_stair_treasury_action001__map_setup(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_pathing_stair_treasury_action002__second_dig_imps_stuck(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_pathing_stair_treasury_action003__check_imps_stuck(struct FTestActionArgs* const args);

TbBool ftest_bug_pathing_stair_treasury_init()
{


    ftest_append_action(ftest_bug_pathing_stair_treasury_action001__map_setup, 20, &ftest_bug_pathing_stair_treasury__vars);
    ftest_append_action(ftest_bug_pathing_stair_treasury_action002__second_dig_imps_stuck, 250, &ftest_bug_pathing_stair_treasury__vars);
    ftest_append_action(ftest_bug_pathing_stair_treasury_action003__check_imps_stuck, 500, &ftest_bug_pathing_stair_treasury__vars);

    return true;
}

/**
 * @brief This action will be a sub-test, it will setup the situation of a single tunneler digging towards you, and replicate getting stuck on a pillar (portal?)
 */
FTestActionResult ftest_bug_pathing_stair_treasury_action001__map_setup(struct FTestActionArgs* const args)
{
    struct ftest_bug_pathing_stair_treasury__variables* const vars = args->data;

    ftest_util_reveal_map(PLAYER0);

    MapSlabCoord stair_x = vars->slb_x_stair_start;
    MapSlabCoord stair_y = vars->slb_y_stair_start;

    // clear out stair shape
    for(unsigned short y = 0; y < vars->stair_count; ++y, ++stair_y)
    {
        for(unsigned short x = 0; x < vars->stair_length-1; ++x, ++stair_x)
        {
            ftest_util_replace_slabs(stair_x, stair_y, stair_x+1, stair_y, SlbT_TREASURE, PLAYER0);
        }
    }

    // create gold at end of stairs
    ftest_util_replace_slabs(stair_x+1, stair_y-1, stair_x+1, stair_y-1, SlbT_GOLD, PLAYER_NEUTRAL);

    // mark block for dig
    TbResult markForDigResult = game_action(PLAYER0, GA_MarkDig, 0, slab_subtile_center(stair_x+1), slab_subtile_center(stair_y-1), 1, 1);
    if (markForDigResult != Lb_OK && markForDigResult != Lb_SUCCESS)
    {
        FTEST_FAIL_TEST("Failed to mark the gold block for digging");
        return FTRs_Go_To_Next_Action;
    }

    // focus camera on dig site
    ftest_util_move_camera_to_slab(stair_x+1, stair_y-1, PLAYER0);

    return FTRs_Go_To_Next_Action; //proceed to next test action
}

FTestActionResult ftest_bug_pathing_stair_treasury_action002__second_dig_imps_stuck(struct FTestActionArgs* const args)
{
    struct ftest_bug_pathing_stair_treasury__variables* const vars = args->data;

    // mark second dig site
    TbResult markForDigResult = game_action(PLAYER0, GA_MarkDig, 0, slab_subtile_center(vars->slb_x_second_dig_action), slab_subtile_center(vars->slb_y_second_dig_action), 1, 1);
    if (markForDigResult != Lb_OK && markForDigResult != Lb_SUCCESS)
    {
        FTEST_FAIL_TEST("Failed to mark the block for digging");
        return FTRs_Go_To_Next_Action;
    }

    // focus camera on dig site
    ftest_util_move_camera_to_slab(vars->slb_x_second_dig_action, vars->slb_y_second_dig_action, PLAYER0);

    return FTRs_Go_To_Next_Action;
}

FTestActionResult ftest_bug_pathing_stair_treasury_action003__check_imps_stuck(struct FTestActionArgs* const args)
{
    struct ftest_bug_pathing_stair_treasury__variables* const vars = args->data;

    // focus camera on stairs again
    ftest_util_move_camera_to_slab(vars->slb_x_stair_start, vars->slb_y_stair_start, PLAYER0);

    // allow camera to sit for a moment so we can view the imps behaviour
    if(game.play_gameturn < args->actual_started_at_game_turn + 100)
    {
        return FTRs_Repeat_Current_Action;
    }

    // if the second block isn't reached/dug then the imps are stuck in the stairs...
    if(ftest_util_do_any_slabs_match(vars->slb_x_second_dig_action, vars->slb_y_second_dig_action, vars->slb_x_second_dig_action, vars->slb_y_second_dig_action, SlbT_WALLWWOMAN))
    {
        FTEST_FAIL_TEST("Imps failed to dig second site, they are stuck in the treasury stairs!");
        return FTRs_Go_To_Next_Action;
    }

    return FTRs_Go_To_Next_Action;
}

#endif
