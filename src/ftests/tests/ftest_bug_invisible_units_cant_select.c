#include "ftest_template.h"

#ifdef FUNCTESTING

#include "../../pre_inc.h"

#include "../ftest.h"
#include "../ftest_util.h"

#include "../../game_legacy.h"
#include "../../keeperfx.hpp"
#include "../../player_instances.h"
#include "../../magic.h"
#include "../../player_states.h"
#include "../../front_input.h"
#include "../../frontend.h"
#include "../../bflib_mouse.h"
#include "../../bflib_planar.h"

#include "../../post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

// example of test variables wraped in a struct, this prevents variable name collisions with other tests, allowing you to name your variables how you like!
struct ftest_bug_invisible_units_cant_select__variables
{
    const MapSlabCoord slb_x_arena_start;
    const MapSlabCoord slb_y_arena_start;

    const MapSlabCoord arena_width;
    const MapSlabCoord arena_height;

    TbBool is_unit_spawned;
    TbBool is_unit_in_hand;
    TbBool was_unit_dropped;

    struct Thing* unit;

    GameTurn unit_spawned_at_turn;
    GameTurn unit_grabbed_at_turn;
    GameTurn unit_dropped_at_turn;
};
struct ftest_bug_invisible_units_cant_select__variables ftest_bug_invisible_units_cant_select__vars = {
    .slb_x_arena_start = 12,
    .slb_y_arena_start = 50,

    .arena_width = 25,
    .arena_height = 25,

    .is_unit_spawned = false,
    .is_unit_in_hand = false,
    .was_unit_dropped = false,

    .unit = NULL,

    .unit_spawned_at_turn = UINT32_MAX,
    .unit_grabbed_at_turn = UINT32_MAX,
    .unit_dropped_at_turn = UINT32_MAX
};

// forward declarations - tests
FTestActionResult ftest_bug_invisible_units_cant_select_action001__spawn_unit(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_invisible_units_cant_select_action002__zoom_to_unit_and_trigger_invisibility_bug(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_invisible_units_cant_select_action003__pickup_unit(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_invisible_units_cant_select_action004__drop_unit(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_invisible_units_cant_select_action005__kill_unit(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_invisible_units_cant_select_action006__restart_actions(struct FTestActionArgs* const args);


TbBool ftest_bug_invisible_units_cant_select_init()
{
    // to make the test variable names shorter, use a pointer!
    // in this case we are grabbing the data from the argument, allowing different action setups!
    struct ftest_bug_invisible_units_cant_select__variables* const vars = &ftest_bug_invisible_units_cant_select__vars;

    // this test showcases how we can do initial map setup outside of the actions queue
    {
        ftest_util_reveal_map(PLAYER0); // we might want to see the entire map for testing purposes

        // carve out area
        ftest_util_replace_slabs(vars->slb_x_arena_start-1, vars->slb_y_arena_start-1, vars->slb_x_arena_start + vars->arena_width+1, vars->slb_y_arena_start + vars->arena_height+1, SlbT_WALLDRAPE, PLAYER0);
        ftest_util_replace_slabs(vars->slb_x_arena_start, vars->slb_y_arena_start, vars->slb_x_arena_start + vars->arena_width, vars->slb_y_arena_start + vars->arena_height, SlbT_CLAIMED, PLAYER0);

        //

        // focus camera to center of arena
        ftest_util_move_camera_to_slab((vars->slb_x_arena_start + (vars->slb_x_arena_start+vars->arena_width)) / 2, (vars->slb_y_arena_start + (vars->slb_y_arena_start+vars->arena_height)) / 2, PLAYER0);
    }

    // setup actions
    //ftest_append_action(ftest_util_action__create_and_fill_torture_room, 20, &ftest_bug_invisible_units_cant_select__torture__vars);

    for(int i = 0; i < 10; ++i)
    {
        ftest_append_action(ftest_bug_invisible_units_cant_select_action001__spawn_unit, 20, &ftest_bug_invisible_units_cant_select__vars);
        ftest_append_action(ftest_bug_invisible_units_cant_select_action002__zoom_to_unit_and_trigger_invisibility_bug, 20, &ftest_bug_invisible_units_cant_select__vars);
        ftest_append_action(ftest_bug_invisible_units_cant_select_action003__pickup_unit, 20, &ftest_bug_invisible_units_cant_select__vars);
        ftest_append_action(ftest_bug_invisible_units_cant_select_action004__drop_unit, 20, &ftest_bug_invisible_units_cant_select__vars);
        ftest_append_action(ftest_bug_invisible_units_cant_select_action005__kill_unit, 20, &ftest_bug_invisible_units_cant_select__vars);
    }



    //ftest_append_action(ftest_bug_invisible_units_cant_select_action005__restart_actions, 0, NULL);

    return FTRs_Go_To_Next_Action;
}



FTestActionResult ftest_bug_invisible_units_cant_select_action001__spawn_unit(struct FTestActionArgs* const args)
{
    struct ftest_bug_invisible_units_cant_select__variables* const vars = args->data;

    // select center for now
    const MapSlabCoord slb_x_arena_random = vars->slb_x_arena_start + (vars->arena_width/2);
    const MapSlabCoord slb_y_arena_random = vars->slb_y_arena_start + (vars->arena_height/2);

    // select random tile in center 33% of area (units too close to walls can be hard to click in iso view, obstructed by tiles...)
    //const MapSlabCoord slb_x_arena_start_33percent = vars->slb_x_arena_start + (vars->arena_width/3);
    //const MapSlabCoord slb_y_arena_start_33percent = vars->slb_y_arena_start + (vars->arena_height/3);
    //const MapSlabCoord slb_x_arena_random = slb_x_arena_start_33percent + rand() % ((vars->arena_width/3)*2);
    //const MapSlabCoord slb_y_arena_random = slb_y_arena_start_33percent + rand() % ((vars->arena_height/3)*2);

    struct Coord3d randPos;
    set_coords_to_slab_center(&randPos, slb_x_arena_random, slb_y_arena_random);

    vars->unit = ftest_util_create_random_creature(randPos.x.val, randPos.y.val, PLAYER0, 1);
    if(thing_is_invalid(vars->unit))
    {
        FTEST_FAIL_TEST("Failed to create random creature");
        return FTRs_Go_To_Next_Action;
    }

    //center cursor/camera on unit pos (using slight offset for better results)
    ftest_util_center_cursor_over_dungeon_view();
    //ftest_util_move_camera(vars->unit->mappos.x.val, vars->unit->mappos.y.val, PLAYER0);

    vars->unit_spawned_at_turn = game.play_gameturn;
    vars->is_unit_spawned = true;

    return FTRs_Go_To_Next_Action;
}

FTestActionResult ftest_bug_invisible_units_cant_select_action002__zoom_to_unit_and_trigger_invisibility_bug(struct FTestActionArgs* const args)
{
    struct ftest_bug_invisible_units_cant_select__variables* const vars = args->data;

    struct PlayerInfo* player = get_my_player();
    if(player_invalid(player))
    {
        FTEST_FAIL_TEST("Failed to find current player!");
        return FTRs_Go_To_Next_Action;
    }

    //center cursor on screen and move the camera using PI_ZoomToPos
    ftest_util_center_cursor_over_dungeon_view();
    player->zoom_to_pos_x = vars->unit->mappos.x.val;
    player->zoom_to_pos_y = vars->unit->mappos.y.val;
    set_player_instance(player, PI_ZoomToPos, 0);

    // slap the creature so the cursor counts as overtop during PI_ZoomToPos, which causes the invisibility bug
    game_action(PLAYER0, GA_UsePwrSlap, 0, 0, 0, vars->unit->index, 0);

    return FTRs_Go_To_Next_Action;
}

FTestActionResult ftest_bug_invisible_units_cant_select_action003__pickup_unit(struct FTestActionArgs* const args)
{
    struct ftest_bug_invisible_units_cant_select__variables* const vars = args->data;

    //center cursor/camera on unit pos (using slight offset for better results)
    ftest_util_center_cursor_over_dungeon_view();
    ftest_util_move_camera(vars->unit->mappos.x.val, vars->unit->mappos.y.val, PLAYER0);

    // check if creature render flag is set, it shoudln't be, if it is, fail the test
    if(flag_is_set(vars->unit->rendering_flags, TRF_Invisible))
    {
        FTEST_FAIL_TEST("Creature %s index %d has render flag %d set when it shouldn't!", thing_model_name(vars->unit), (int)vars->unit->index, TRF_Invisible);
        return FTRs_Go_To_Next_Action;
    }

    // check if creature is under cursor
    struct PlayerInfo* player = get_my_player();
    if(player->thing_under_hand == vars->unit->index)
    {
        // FTEST_FAIL_TEST("Did not find %s index %d under mouse cursor", thing_model_name(vars->unit), (int)vars->unit->index);
        // return FTRs_Go_To_Next_Action;

        // pickup unit without requiring mouse hover (does not require hand over creature)
        // if (!magic_use_available_power_on_thing(PLAYER0, PwrK_HAND, 0, vars->unit->mappos.x.stl.num, vars->unit->mappos.y.stl.num, vars->unit, PwMod_Default))
        // {
        //     FTEST_FAIL_TEST("Cannot pick up %s index %d", thing_model_name(vars->unit), (int)vars->unit->index);
        //     return FTRs_Go_To_Next_Action;
        // }

        // try to pickup creature (hand must be over creature for this to work!)
        TbResult pickup_result = magic_use_power_hand(PLAYER0, vars->unit->mappos.x.stl.num, vars->unit->mappos.y.stl.num, 0);
        if(pickup_result != Lb_SUCCESS)
        {
            FTEST_FAIL_TEST("Cannot pick up %s index %d", thing_model_name(vars->unit), (int)vars->unit->index);
            return FTRs_Go_To_Next_Action;
        }

        vars->unit_grabbed_at_turn = game.play_gameturn;
        vars->is_unit_in_hand = true;
        return FTRs_Go_To_Next_Action;
    }

    return FTRs_Repeat_Current_Action;
}

FTestActionResult ftest_bug_invisible_units_cant_select_action004__drop_unit(struct FTestActionArgs* const args)
{
    struct ftest_bug_invisible_units_cant_select__variables* const vars = args->data;

    if(!vars->is_unit_in_hand)
    {
        FTEST_FAIL_TEST("There is no unit in hand to drop!");
        return FTRs_Go_To_Next_Action;
    }

    // try to drop creature
    struct Coord3d dropPos;
    set_coords_to_slab_center(&dropPos, vars->slb_x_arena_start+1, vars->slb_y_arena_start+1);
    if(!dump_first_held_thing_on_map(PLAYER0, dropPos.x.stl.num, dropPos.y.stl.num, 1))
    {
        FTEST_FAIL_TEST("Cannot drop %s index %d", thing_model_name(vars->unit), (int)vars->unit->index);
    }
    else
    {
        vars->is_unit_in_hand = false;
        vars->unit_dropped_at_turn = game.play_gameturn;
    }

    return FTRs_Go_To_Next_Action;
}

FTestActionResult ftest_bug_invisible_units_cant_select_action005__kill_unit(struct FTestActionArgs* const args)
{
    struct ftest_bug_invisible_units_cant_select__variables* const vars = args->data;

    // cleanup unit & vars
    kill_creature(vars->unit, INVALID_THING, PLAYER0, CrDed_NoEffects);
    vars->unit = INVALID_THING;
    vars->is_unit_spawned = false;
    vars->is_unit_in_hand = false;
    vars->was_unit_dropped = false;
    vars->unit_spawned_at_turn = UINT32_MAX;
    vars->unit_grabbed_at_turn = UINT32_MAX;
    vars->unit_dropped_at_turn = UINT32_MAX;

    return FTRs_Go_To_Next_Action;
}

FTestActionResult ftest_bug_invisible_units_cant_select_action006__restart_actions(struct FTestActionArgs* const args)
{
    ftest_restart_actions();

    return FTRs_Go_To_Next_Action;
}

#endif
