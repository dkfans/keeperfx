#include "ftest_bug_ai_bridge.h"

#ifdef FUNCTESTING

#include "../../pre_inc.h"

#include "../ftest.h"
#include "../ftest_util.h"

#include "../../game_legacy.h"
#include "../../keeperfx.hpp"
#include "../../player_instances.h"
#include "../../config_objects.h"
#include "../../gui_parchment.h"
#include "../../scrcapt.h"

#include "../../post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FTEST_BUG_AI_BRIDGE__NUMBER_OF_CREATURES_TO_NERF 3

// example of test variables wraped in a struct, this prevents variable name collisions with other tests, allowing you to name your variables how you like!
struct ftest_bug_ai_bridge__variables
{
    MapSubtlCoord stl_enemies_to_nerf[FTEST_BUG_AI_BRIDGE__NUMBER_OF_CREATURES_TO_NERF][2]; // 2 subtl coords per creature (x,y)

    GameTurn end_test_after_n_turns;

    // we will check this slab area for bridges every turn
    MapSlabCoord slb_x_min_bridge_check_area;
    MapSlabCoord slb_x_max_bridge_check_area;
    MapSlabCoord slb_y_min_bridge_check_area;
    MapSlabCoord slb_y_max_bridge_check_area;

    unsigned long test_runs;
    unsigned long test_runs_with_bridges;
    unsigned long test_runs_without_bridges;

    TbBool take_screenshot;
};
struct ftest_bug_ai_bridge__variables ftest_bug_ai_bridge__vars = {
    .stl_enemies_to_nerf = {
        {121, 109}, // good samurai - level 10
        {127, 109}, // good knight - level 10
        {133, 109}, // good samurai - level 10
    },

    .end_test_after_n_turns = 35000ul,

    .slb_x_min_bridge_check_area = 1,
    .slb_x_max_bridge_check_area = 21,
    .slb_y_min_bridge_check_area = 40,
    .slb_y_max_bridge_check_area = 46,

    .take_screenshot = false,
};

// forward declarations - tests
FTestActionResult ftest_bug_ai_bridge_action001__setup_map(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_ai_bridge_action002__end_test(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_ai_bridge_action003__delayed_screenshot(struct FTestActionArgs* const args);

TbBool ftest_bug_ai_bridge_init()
{
    ftest_append_action(ftest_bug_ai_bridge_action001__setup_map, 0, &ftest_bug_ai_bridge__vars);
    ftest_append_action(ftest_bug_ai_bridge_action002__end_test, 0, &ftest_bug_ai_bridge__vars);
    ftest_append_action(ftest_bug_ai_bridge_action003__delayed_screenshot, 0, &ftest_bug_ai_bridge__vars);

    return true;
}

FTestActionResult ftest_bug_ai_bridge_action001__setup_map(struct FTestActionArgs* const args)
{
    struct ftest_bug_ai_bridge__variables* const vars = args->data;

    ftest_util_reveal_map(PLAYER0); // we might want to see the entire map for testing purposes
    
    // nerf all creatures from subtile array
    for(int creature_nerf_index = 0; creature_nerf_index < FTEST_BUG_AI_BRIDGE__NUMBER_OF_CREATURES_TO_NERF; ++creature_nerf_index)
    {
        MapSubtlCoord stl_x = vars->stl_enemies_to_nerf[creature_nerf_index][0]; // 0 == x
        MapSubtlCoord stl_y = vars->stl_enemies_to_nerf[creature_nerf_index][1]; // 1 == y

        struct Map* mapblk = get_map_block_at(stl_x, stl_y);
        if(map_block_invalid(mapblk))
        {
            FTEST_FAIL_TEST("Invalid map block at subtile (%ld,%ld)", stl_x, stl_y)
            return FTRs_Go_To_Next_Action;
        }
        struct Thing* thing = thing_get(get_mapwho_thing_index(mapblk));
        if (thing_is_invalid(thing) || !thing_is_creature(thing))
        {
            FTEST_FAIL_TEST("Failed to find creature to nerf at subtile (%ld,%ld)", stl_x, stl_y);
            return FTRs_Go_To_Next_Action;
        }
        
        FTESTLOG("Nerfing Creature %s at (%ld,%ld) to 'level 1' and '1 health'", creature_code_name(thing->model), stl_x, stl_y);
        set_creature_level(thing, 0); // 0 == level 1
        thing->health = 1;
    }
    
    // jump camera to location to see
    ftest_util_move_camera_to_slab(vars->slb_x_min_bridge_check_area, vars->slb_y_min_bridge_check_area, PLAYER0);

    // mark area to see where we will be detecting the bridges at
    for(MapSlabCoord slb_y = vars->slb_y_min_bridge_check_area; slb_y <= vars->slb_y_max_bridge_check_area; ++slb_y)
    {
        for(MapSlabCoord slb_x = vars->slb_x_min_bridge_check_area; slb_x <= vars->slb_x_max_bridge_check_area; ++slb_x)
        {
            ftest_util_mark_slab_for_highlight(slb_x, slb_y, PLAYER0);
        }   
    }

    // dig tiles to portal
    ftest_util_replace_slabs(20, 14, 21, 14, SlbT_CLAIMED, PLAYER0);
    
    return FTRs_Go_To_Next_Action; //proceed to next test action
}

void ftest_bug_ai_bridge__report_stats_and_increment_seed()
{
    FTESTLOG("test_runs: %lu", ftest_bug_ai_bridge__vars.test_runs);
    FTESTLOG("test_runs_with_bridges: %lu (%.2f%%)", ftest_bug_ai_bridge__vars.test_runs_with_bridges, (double)ftest_bug_ai_bridge__vars.test_runs_with_bridges/(double)ftest_bug_ai_bridge__vars.test_runs * 100.0);
    FTESTLOG("test_runs_without_bridges: %lu (%.2f%%)", ftest_bug_ai_bridge__vars.test_runs_without_bridges, (double)ftest_bug_ai_bridge__vars.test_runs_without_bridges/(double)ftest_bug_ai_bridge__vars.test_runs * 100.0);

    //log seed, increment after for next test
    struct FTestConfig* current_test_config = ftest_get_current_test_config();
    if(current_test_config->seed > 0)
    {
        FTESTLOG("seed %d", current_test_config->seed++);
    }
    else
    {
        FTESTLOG("seed is 0 - defaults to gameturn %d", game.play_gameturn);
    }
}

FTestActionResult ftest_bug_ai_bridge_action002__end_test(struct FTestActionArgs* const args)
{
    struct ftest_bug_ai_bridge__variables* const vars = args->data;

    if(game.play_gameturn == 20) // don't open map too soon, otherwise game will crash...
    {
        zoom_to_parchment_map(); // open map?? might make test faster due to no 3d rendering?
    }

    // check defined slab area to determine if there are any bridges
    TbBool found_bridge = false;
    for(MapSlabCoord slb_y = vars->slb_y_min_bridge_check_area; slb_y <= vars->slb_y_max_bridge_check_area; ++slb_y)
    {
        for(MapSlabCoord slb_x = vars->slb_x_min_bridge_check_area; slb_x <= vars->slb_x_max_bridge_check_area; ++slb_x)
        {
            struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
            if(slabmap_block_invalid(slb))
            {
                FTEST_FAIL_TEST("Invalid slab found at (%d,%d)", slb_x, slb_y);
                return FTRs_Go_To_Next_Action;
            }

            if(slb->kind == SlbT_BRIDGE)
            {
                FTESTLOG("Found bridge at (%d,%d)", slb_x, slb_y);
                found_bridge = true;
            }
        }
    }

    if(found_bridge)
    {
        ++vars->test_runs;
        ++vars->test_runs_with_bridges;
        FTESTLOG("Bridges were found at GameTurn %d, reporting and exiting test", game.play_gameturn);
        vars->take_screenshot = true;
        game.fastforward_speed = 0;
        ftest_bug_ai_bridge__report_stats_and_increment_seed();
        return FTRs_Go_To_Next_Action; // exit test
    }

    if(game.play_gameturn >= vars->end_test_after_n_turns)
    {
        ++vars->test_runs;
        ++vars->test_runs_without_bridges;
        FTESTLOG("Reached GameTurn limit %ld, exiting test.",  vars->end_test_after_n_turns);
        ftest_bug_ai_bridge__report_stats_and_increment_seed();
        return FTRs_Go_To_Next_Action; // exit test
    }

    return FTRs_Repeat_Current_Action; // repeat current action
}

FTestActionResult ftest_bug_ai_bridge_action003__delayed_screenshot(struct FTestActionArgs* const args)
{
    struct ftest_bug_ai_bridge__variables* const vars = args->data;

    const GameTurn screenshot_turn_delay = 0;

    if(vars->take_screenshot)
    {
        if(game.play_gameturn < args->actual_started_at_game_turn + screenshot_turn_delay) // wait until we are supposed to take the screenshot
        {
            return FTRs_Repeat_Current_Action;
        }

        struct FTestConfig* current_test_config = ftest_get_current_test_config();
        char fname[FILENAME_MAX] = "";
        snprintf(fname, sizeof(fname), "scrshots/scr%05u.%s", current_test_config->seed, ".png");
        take_screenshot(fname);
        vars->take_screenshot = false;
    }

    return FTRs_Go_To_Next_Action;
}

#endif
