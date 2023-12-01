#include "ftest_bug_imp_goldseam_dig.h"

#ifdef FUNCTESTING

#include "pre_inc.h"

#include "ftest.h"
#include "ftest_util.h"

#include "game_legacy.h"
#include "keeperfx.hpp"
#include "player_instances.h"
#include "player_utils.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

GoldAmount ftest_bug_imp_goldseam__game_gold_amount = 0;

// forward declarations - tests
TbBool ftest_bug_imp_goldseam_dig_action001__map_setup();
TbBool ftest_bug_imp_goldseam_dig_action002__spawn_imp();
TbBool ftest_bug_imp_goldseam_dig_action003__send_imp_to_dig();
TbBool ftest_bug_imp_goldseam_dig_action004__end_test();

TbBool ftest_bug_imp_goldseam_dig_init()
{
    ftest_append_action(ftest_bug_imp_goldseam_dig_action001__map_setup, 10);
    ftest_append_action(ftest_bug_imp_goldseam_dig_action002__spawn_imp, 20);
    ftest_append_action(ftest_bug_imp_goldseam_dig_action003__send_imp_to_dig, 30);
    ftest_append_action(ftest_bug_imp_goldseam_dig_action004__end_test, 600);

    return true;
}

TbBool ftest_bug_imp_goldseam_dig_action001__map_setup()
{
    ftest_util_reveal_map(PLAYER0); // we might want to see the entire map for testing purposes

    //set player gold to 0 for this test
    struct Dungeon* dungeon = get_players_num_dungeon(PLAYER0);
    if(dungeon_invalid(dungeon))
    {
        FTEST_FAIL_TEST("Failed to find dungeon");
        return true;
    }
    take_money_from_dungeon(PLAYER0, dungeon->total_money_owned, false);

    // place gold room below dungeon heart
    ftest_util_replace_slabs(3, 6, 3, 6, SlbT_CLAIMED, PLAYER0);
    ftest_util_replace_slabs(1, 7, 6, 12, SlbT_WALLDRAPE, PLAYER0);
    ftest_util_replace_slabs(1, 7, 5, 11, SlbT_TREASURE, PLAYER0);

    // spawn gold
    ftest_util_replace_slabs(6, 1, 7, 5, SlbT_PATH, PLAYER_NEUTRAL);
    ftest_util_replace_slabs(6, 1, 7, 5, SlbT_GOLD, PLAYER_NEUTRAL);

    // store/broadcast the gold stored in a single tile
    ftest_bug_imp_goldseam__game_gold_amount = game.gold_per_gold_block;
    message_add_fmt(PLAYER0, "Game gold per gold block: %ld", ftest_bug_imp_goldseam__game_gold_amount);

    return true;
}

TbBool ftest_bug_imp_goldseam_dig_action002__spawn_imp()
{
    // create an imp
    struct Coord3d impPos;
    impPos.x.val = 0; // setting to 0 for x/y is required before setting stl.num/pos, otherwise val will be incorrect...
    impPos.y.val = 0;
    impPos.x.stl.num = slab_subtile_center(1);
    impPos.x.stl.pos = 128; //128 == 50% - center of tile
    impPos.y.stl.num = slab_subtile_center(1);
    impPos.y.stl.pos = 128;
    
    struct Thing* ftest_template_target_imp = create_owned_special_digger(impPos.x.val, impPos.y.val, PLAYER0);
    if(thing_is_invalid(ftest_template_target_imp))
    {
        FTEST_FAIL_TEST("Failed to create imp");
        return true;
    }

    // level up the imp to 10 for faster digging
    if(!creature_change_multiple_levels(ftest_template_target_imp, 9))
    {
        FTEST_FAIL_TEST("Failed to level up imp");
        return true;
    }

    return true; //proceed to next test action
}

TbBool ftest_bug_imp_goldseam_dig_action003__send_imp_to_dig()
{
    // select one of the gold slabmap blocks
    const MapSlabCoord slb_x_gold_block = 6;
    const MapSlabCoord slb_y_gold_block = 3;

    struct SlabMap* slabMapBlock = get_slabmap_block(slb_x_gold_block, slb_y_gold_block);
    if(slabmap_block_invalid(slabMapBlock))
    {
        FTEST_FAIL_TEST("Failed to find gold slabmap block");
        return true;
    }

    // store/report the blocks health to user
    struct SlabAttr *slbattr = get_slab_attrs(slabMapBlock);
    unsigned short goldBlockHealth = game.block_health[slbattr->block_health_index];
    message_add_fmt(PLAYER0, "Gold block at (%d,%d) has %d health", slb_x_gold_block, slb_y_gold_block, goldBlockHealth);

    // mark the block for digging
    TbResult markForDigResult = game_action(PLAYER0, GA_MarkDig, 0, slab_subtile_center(slb_x_gold_block), slab_subtile_center(slb_y_gold_block), 1, 1);
    if (markForDigResult != Lb_OK && markForDigResult != Lb_SUCCESS)
    {
        FTEST_FAIL_TEST("Failed to mark the gold block for digging");
        return true;
    }

    return true; //proceed to next test action
}

TbBool ftest_bug_imp_goldseam_dig_action004__end_test()
{
    // count gold acquired by imp and stored in the treasure room
    struct Dungeon* dungeon = get_players_num_dungeon(PLAYER0);
    if(dungeon_invalid(dungeon))
    {
        FTEST_FAIL_TEST("Failed to find dungeon");
        return true;
    }
    
    // report total gold
    message_add_fmt(PLAYER0, "Imp returned %d gold", dungeon->total_money_owned);
    if(dungeon->total_money_owned != ftest_bug_imp_goldseam__game_gold_amount)
    {
        FTEST_FAIL_TEST("Goldseams have %ld gold, but imp returned %d gold!", ftest_bug_imp_goldseam__game_gold_amount, dungeon->total_money_owned);
        return true;
    }

    return true;
}

#endif