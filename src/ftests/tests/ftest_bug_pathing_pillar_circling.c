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
struct ftest_bug_pathing_pillar_circling__variables
{
    const MapSlabCoord slb_x_tunneler_start;
    const MapSlabCoord slb_y_tunneler_start;

    const MapSlabCoord slb_x_pillar;
    const MapSlabCoord slb_y_pillar;

    TbBool is_tunneler_setup;
    struct Thing* tunneler;

    unsigned char pillar_base_slab_type;
};
struct ftest_bug_pathing_pillar_circling__variables ftest_bug_pathing_pillar_circling__vars = {
    .slb_x_tunneler_start = 66,
    .slb_y_tunneler_start = 43,

    .slb_x_pillar = 57,
    .slb_y_pillar = 43,

    .is_tunneler_setup = false,
    .tunneler = NULL,

    .pillar_base_slab_type = SlbT_ROCK_FLOOR
};
struct ftest_bug_pathing_pillar_circling__variables ftest_bug_pathing_pillar_circling__vars2 = {
    .slb_x_tunneler_start = 42,
    .slb_y_tunneler_start = 56,

    .slb_x_pillar = 42,
    .slb_y_pillar = 49,

    .is_tunneler_setup = false,
    .tunneler = NULL,

    .pillar_base_slab_type = SlbT_EARTH
};



// forward declarations - tests
FTestActionResult ftest_bug_pathing_pillar_circling_action001__tunneler_dig_towards_pillar_test(struct FTestActionArgs* const args);

TbBool ftest_bug_pathing_pillar_circling_init()
{
    // this test will showcase multiple sub-tests, one sub-test per action
    // passing of variables to actions through void* (ftest_bug_pathing_pillar_circling__vars) allows some flexibility here

    ftest_append_action(ftest_bug_pathing_pillar_circling_action001__tunneler_dig_towards_pillar_test, 20, &ftest_bug_pathing_pillar_circling__vars);
    ftest_append_action(ftest_bug_pathing_pillar_circling_action001__tunneler_dig_towards_pillar_test, 20, &ftest_bug_pathing_pillar_circling__vars2);

    return true;
}

/**
 * @brief This action will be a sub-test, it will setup the situation of a single tunneler digging towards you, and replicate getting stuck on a pillar (portal?)
 */
FTestActionResult ftest_bug_pathing_pillar_circling_action001__tunneler_dig_towards_pillar_test(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    // in this case we are grabbing the data from the argument, allowing different action setups!
    struct ftest_bug_pathing_pillar_circling__variables* const vars = args->data;

    ftest_util_reveal_map(PLAYER0);

    // example stage001 of using an argument variable to support multiple test stages inside of a single test action
    if(!vars->is_tunneler_setup)
    {
        // clear starting position for tunneler
        ftest_util_replace_slabs(vars->slb_x_tunneler_start, vars->slb_y_tunneler_start, vars->slb_x_tunneler_start, vars->slb_y_tunneler_start, SlbT_PATH, PLAYER_NEUTRAL);

        // place pillar/column in the way
        {
            ftest_util_replace_slab_columns(vars->slb_x_pillar, vars->slb_y_pillar, PLAYER_NEUTRAL, vars->pillar_base_slab_type, 26, 26, 26
                                                                                                                               , 26, 01, 26
                                                                                                                               , 26, 26, 26); // 01 - dirt pillar, 26 - path
        }

        struct Coord3d tunneler_pos;
        set_coords_to_slab_center(&tunneler_pos, vars->slb_x_tunneler_start, vars->slb_y_tunneler_start);

        // create tunneler
        vars->tunneler = create_owned_special_digger(tunneler_pos.x.val, tunneler_pos.y.val, PLAYER_GOOD);
        if(thing_is_invalid(vars->tunneler))
        {
            FTEST_FAIL_TEST("Failed to create tunneler");
            return FTRs_Go_To_Next_Action;
        }

        vars->is_tunneler_setup = true;
        return FTRs_Repeat_Current_Action;
    }

    // snap camera to tunneler
    ftest_util_move_camera_to_thing(vars->tunneler, PLAYER0);

    // delay for a while so we can watch what's going on
    if(game.play_gameturn < args->actual_started_at_game_turn + 1000)
    {
        return FTRs_Repeat_Current_Action;
    }

    // todo - insert check stage here to verify if tunneler made it past the column or not
    // another variable can be added to ftest_bug_pathing_pillar_circling__variables (eg: should_tunneler_pass_column) for the test condition

    // example stage002 of using an argument variable to support multiple test stages inside of a single test action
    if(thing_is_invalid(vars->tunneler))
    {
        FTEST_FAIL_TEST("Expected tunneler but it didn't exist?");
        return FTRs_Go_To_Next_Action;
    }
    else
    {
        vars->tunneler = kill_creature(vars->tunneler, INVALID_THING, PLAYER0, CrDed_Default);
        if(thing_is_invalid(vars->tunneler))
        {
            FTEST_FAIL_TEST("Failed to cleanup tunneler on map");
            return FTRs_Go_To_Next_Action;
        }
    }

    return FTRs_Go_To_Next_Action; //proceed to next test action
}



#endif
