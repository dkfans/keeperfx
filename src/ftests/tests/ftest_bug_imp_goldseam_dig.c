#include "ftest_bug_imp_goldseam_dig.h"

#ifdef FUNCTESTING

#include "../../pre_inc.h"

#include "../ftest.h"
#include "../ftest_util.h"

#include "../../game_legacy.h"
#include "../../keeperfx.hpp"
#include "../../player_instances.h"
#include "../../player_utils.h"
#include "../../gui_msgs.h"

#include "../../post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

// example of test variables wraped in a struct, this prevents variable name collisions with other tests, allowing you to name your variables how you like!
struct ftest_bug_imp_goldseam_dig__variables
{
    GoldAmount game_gold_amount;
};
struct ftest_bug_imp_goldseam_dig__variables ftest_bug_imp_goldseam_dig__vars = {
    .game_gold_amount = 0
};

// forward declarations - tests
FTestActionResult ftest_bug_imp_goldseam_dig_action001__map_setup(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_imp_goldseam_dig_action002__send_imp_to_dig(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_imp_goldseam_dig_action003__end_test(struct FTestActionArgs* const args);

TbBool ftest_bug_imp_goldseam_dig_init()
{
    ftest_append_action(ftest_bug_imp_goldseam_dig_action001__map_setup,        10,     NULL);
    ftest_append_action(ftest_bug_imp_goldseam_dig_action002__send_imp_to_dig,  30,     NULL);
    ftest_append_action(ftest_bug_imp_goldseam_dig_action003__end_test,         400,    NULL);

    return true;
}

FTestActionResult ftest_bug_imp_goldseam_dig_action001__map_setup(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    struct ftest_bug_imp_goldseam_dig__variables* const vars = &ftest_bug_imp_goldseam_dig__vars;

    ftest_util_reveal_map(PLAYER0); // we might want to see the entire map for testing purposes

    //set player gold to 0 for this test
    struct Dungeon* dungeon = get_players_num_dungeon(PLAYER0);
    if(dungeon_invalid(dungeon))
    {
        FTEST_FAIL_TEST("Failed to find dungeon");
        return FTRs_Go_To_Next_Action;
    }
    take_money_from_dungeon(PLAYER0, dungeon->total_money_owned, false);

    // place gold room to left of dungeon heart
    ftest_util_replace_slabs(36, 42, 38, 44, SlbT_TREASURE, PLAYER0);

    // store/broadcast the gold stored in a single tile
    vars->game_gold_amount = game.conf.rules.game.gold_per_gold_block;
    message_add_fmt(MsgType_Player, PLAYER0, "Game gold per gold block: %ld", vars->game_gold_amount);

    return FTRs_Go_To_Next_Action;
}

FTestActionResult ftest_bug_imp_goldseam_dig_action002__send_imp_to_dig(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    //struct ftest_bug_imp_goldseam_dig__variables* const vars = &ftest_bug_imp_goldseam_dig__vars;

    // select one of the gold slabmap blocks
    const MapSlabCoord slb_x_gold_block = 46;
    const MapSlabCoord slb_y_gold_block = 43;

    struct SlabMap* slabMapBlock = get_slabmap_block(slb_x_gold_block, slb_y_gold_block);
    if(slabmap_block_invalid(slabMapBlock))
    {
        FTEST_FAIL_TEST("Failed to find gold slabmap block");
        return FTRs_Go_To_Next_Action;
    }

    // store/report the blocks health to user
    struct SlabAttr *slbattr = get_slab_attrs(slabMapBlock);
    HitPoints goldBlockHealth = game.block_health[slbattr->block_health_index];
    message_add_fmt(MsgType_Player, PLAYER0, "Gold block at (%d,%d) has %d health", slb_x_gold_block, slb_y_gold_block, goldBlockHealth);

    // mark the block for digging
    TbResult markForDigResult = game_action(PLAYER0, GA_MarkDig, 0, slab_subtile_center(slb_x_gold_block), slab_subtile_center(slb_y_gold_block), 1, 1);
    if (markForDigResult != Lb_OK && markForDigResult != Lb_SUCCESS)
    {
        FTEST_FAIL_TEST("Failed to mark the gold block for digging");
        return FTRs_Go_To_Next_Action;
    }

    return FTRs_Go_To_Next_Action; //proceed to next test action
}

FTestActionResult ftest_bug_imp_goldseam_dig_action003__end_test(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    struct ftest_bug_imp_goldseam_dig__variables* const vars = &ftest_bug_imp_goldseam_dig__vars;

    // count gold acquired by imp and stored in the treasure room
    struct Dungeon* dungeon = get_players_num_dungeon(PLAYER0);
    if(dungeon_invalid(dungeon))
    {
        FTEST_FAIL_TEST("Failed to find dungeon");
        return FTRs_Go_To_Next_Action;
    }
    
    // report total gold
    message_add_fmt(MsgType_Player, PLAYER0, "Imp returned %d gold", dungeon->total_money_owned);
    if(dungeon->total_money_owned != vars->game_gold_amount)
    {
        FTEST_FAIL_TEST("Goldseams have %ld gold, but imp returned %d gold!", vars->game_gold_amount, dungeon->total_money_owned);
        return FTRs_Go_To_Next_Action;
    }

    return FTRs_Go_To_Next_Action;
}

#endif
