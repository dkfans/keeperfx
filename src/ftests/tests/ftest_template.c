#include "ftest_template.h"

#ifdef FUNCTESTING

#include "../../pre_inc.h"

#include "../ftest.h"
#include "../ftest_util.h"

#include "../../game_legacy.h"
#include "../../keeperfx.hpp"
#include "../../player_instances.h"
#include "../../gui_msgs.h"

#include "../../post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

// example of test variables wraped in a struct, this prevents variable name collisions with other tests, allowing you to name your variables how you like!
struct ftest_template__variables
{
    MapSlabCoord slb_x_spawn_imp;
    MapSlabCoord slb_y_spawn_imp;

    struct Thing* target_imp;
    GameTurn turn_delay_counter;
    unsigned short slap_counter;
};
struct ftest_template__variables ftest_template__vars = {
    .slb_x_spawn_imp = 17,
    .slb_y_spawn_imp = 74,

    .target_imp = NULL,
    .turn_delay_counter = 0,
    .slap_counter = 0
};

// forward declarations - tests
FTestActionResult ftest_template_action001__spawn_imp(struct FTestActionArgs* const args);
FTestActionResult ftest_template_action002__slap_imp_to_death(struct FTestActionArgs* const args);
FTestActionResult ftest_template_action003__end_test(struct FTestActionArgs* const args);

TbBool ftest_template_init()
{
    //here we can see that for each action we want to implement, we append the FTest_Action_Func and the game turn offset
    //note: you can add the same action multiple times
    //note: you can add existing actions from other tests as well!
    ftest_append_action(ftest_template_action001__spawn_imp, 20, &ftest_template__vars); //spawn imp after 20 game turns
    ftest_append_action(ftest_template_action002__slap_imp_to_death, 40, &ftest_template__vars); //start slapping the imp to death after 40 game turns
    ftest_append_action(ftest_template_action003__end_test, 500, &ftest_template__vars); //end the test after 20+40+500 game turns

    return true;
}

FTestActionResult ftest_template_action001__spawn_imp(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    // in this case we are grabbing the data from the argument, allowing different action setups!
    struct ftest_template__variables* const vars = args->data;

    ftest_util_reveal_map(PLAYER0); // we might want to see the entire map for testing purposes

    // create an imp
    struct Coord3d impPos;
    impPos.x.val = 0; // setting to 0 for x/y is required before setting stl.num/pos, otherwise val will be incorrect...
    impPos.y.val = 0;
    impPos.x.stl.num = slab_subtile_center(vars->slb_x_spawn_imp);
    impPos.x.stl.pos = COORD_PER_STL / 2; //128 == 50% - center of tile
    impPos.y.stl.num = slab_subtile_center(vars->slb_y_spawn_imp);
    impPos.y.stl.pos = 128; 
    
    // jump camera to location to see
    ftest_util_move_camera_to_slab(vars->slb_x_spawn_imp, vars->slb_y_spawn_imp, PLAYER0);
    
    vars->target_imp = create_owned_special_digger(impPos.x.val, impPos.y.val, PLAYER0);
    if(thing_is_invalid(vars->target_imp))
    {
        FTEST_FAIL_TEST("Failed to create imp");
        return FTRs_Go_To_Next_Action;
    }

    // level up the imp a couple times for fun
    if(!creature_change_multiple_levels(vars->target_imp, 2))
    {
        FTEST_FAIL_TEST("Failed to level up imp");
        return FTRs_Go_To_Next_Action;
    }

    ftest_util_move_camera_to_slab(vars->slb_x_spawn_imp, vars->slb_y_spawn_imp, PLAYER0);

    return FTRs_Go_To_Next_Action; //proceed to next test action
}

FTestActionResult ftest_template_action002__slap_imp_to_death(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    // in this case we are grabbing the data from the argument, allowing different action setups!
    struct ftest_template__variables* const vars = args->data;

    // delay the test for a bit as an example
    if(game.play_gameturn < 100)
    {
        return FTRs_Repeat_Current_Action;
    }

    // delay the test after each slap
    if(game.play_gameturn < vars->turn_delay_counter)
    {
        return FTRs_Repeat_Current_Action;
    }

    ftest_util_move_camera_to_thing(vars->target_imp, PLAYER0);

    // slap hehehe
    if(game_action(PLAYER0, GA_UsePwrSlap, 0, 0, 0, vars->target_imp->index, 0) > Lb_OK)
    {
        message_add_fmt(MsgType_Player, PLAYER0, "Slap %d", ++vars->slap_counter);

        vars->turn_delay_counter = game.play_gameturn + 20;
        return FTRs_Repeat_Current_Action;
    }

    if (vars->target_imp->health <= 0)
    {
        message_add_fmt(MsgType_Player, PLAYER0, "Oops...");
        return FTRs_Go_To_Next_Action;
    }

    return FTRs_Go_To_Next_Action;
}

FTestActionResult ftest_template_action003__end_test(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    // in this case we are grabbing the data from the argument, allowing different action setups!
    struct ftest_template__variables* const vars = args->data;

    if(vars->slap_counter != 26)
    {
        FTEST_FAIL_TEST("Expected 26 slaps for level 3 imp, only counted %d", vars->slap_counter);
        return FTRs_Go_To_Next_Action;
    }

    return FTRs_Go_To_Next_Action;
}

#endif
