#include "ftest_template.h"

#ifdef FUNCTESTING

#include "pre_inc.h"

#include "ftest.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "room_util.h"
#include "player_instances.h"
//#include "thing_data.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif


struct Thing* ftest_template_target_imp = NULL;

GameTurn ftest_template_turn_delay_counter = 0;
unsigned short ftest_template_turn_slap_counter = 0;

// forward declarations - tests
TbBool ftest_template_action001__spawn_imp();
TbBool ftest_template_action002__slap_imp_to_death();
TbBool ftest_template_action003__end_test();

TbBool ftest_template_init()
{
    //here we can see that for each action we want to implement, we append the FTest_Action_Func and the game turn offset
    //note: you can add the same action multiple times
    //note: you can add existing actions from other tests as well!
    ftest_append_action(ftest_template_action001__spawn_imp, 20); //spawn imp after 20 game turns
    ftest_append_action(ftest_template_action002__slap_imp_to_death, 40); //start slapping the imp to death after 40 game turns
    ftest_append_action(ftest_template_action003__end_test, 500); //end the test after 20+40+500 game turns

    return true;
}

TbBool ftest_template_action001__spawn_imp()
{
    ftest_reveal_map(PLAYER0); // we might want to see the entire map for testing purposes

    // create an imp
    struct Coord3d impPos;
    impPos.x.val = 0; // setting to 0 for x/y is required before setting stl.num/pos, otherwise val will be incorrect...
    impPos.y.val = 0;
    impPos.x.stl.num = slab_subtile_center(5);
    impPos.x.stl.pos = 128; //128 == 50% - center of tile
    impPos.y.stl.num = slab_subtile_center(3);
    impPos.y.stl.pos = 128; 
    
    
    ftest_template_target_imp = create_owned_special_digger(impPos.x.val, impPos.y.val, PLAYER0);
    if(thing_is_invalid(ftest_template_target_imp))
    {
        FTEST_FAIL_TEST("Failed to create imp");
        return true;
    }

    // level up the imp a couple times for fun
    if(!creature_change_multiple_levels(ftest_template_target_imp, 2))
    {
        FTEST_FAIL_TEST("Failed to level up imp");
        return true;
    }

    return true; //proceed to next test action
}

TbBool ftest_template_action002__slap_imp_to_death()
{
    // delay the test for a bit as an example
    if(game.play_gameturn < 100)
    {
        return false;
    }

    // delay the test after each slap
    if(game.play_gameturn < ftest_template_turn_delay_counter)
    {
        return false;
    }

    // slap hehehe
    if(game_action(PLAYER0, GA_UsePwrSlap, 0, 0, 0, ftest_template_target_imp->index, 0) > Lb_OK)
    {
        message_add_fmt(PLAYER0, "Slap %d", ++ftest_template_turn_slap_counter);

        ftest_template_turn_delay_counter = game.play_gameturn + 20;
        return false;
    }

    if (ftest_template_target_imp->health <= 0)
    {
        message_add_fmt(PLAYER0, "Oops...");
        return true;
    }

    return true;
}

TbBool ftest_template_action003__end_test()
{
    if(ftest_template_turn_slap_counter != 26)
    {
        FTEST_FAIL_TEST("Expected 26 slaps for level 3 imp, only counted %d", ftest_template_turn_slap_counter);
    }

    return true;
}

#endif