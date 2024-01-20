#include "ftest_bug_imp_tp_job_attack_door.h"

#ifdef FUNCTESTING

#include "../../pre_inc.h"

#include "../ftest.h"
#include "../ftest_util.h"

#include "../../game_legacy.h"
#include "../../keeperfx.hpp"
#include "../../player_instances.h"
#include "../../power_specials.h"

#include "../../post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

// example of test variables wraped in a struct, this prevents variable name collisions with other tests, allowing you to name your variables how you like!
struct ftest_bug_imp_tp_job_attack_door__variables
{
    MapSlabCoord slb_x_tunnel_1;
    MapSlabCoord slb_y_tunnel_1;
    MapSlabCoord slb_x_tunnel_2;
    MapSlabCoord slb_y_tunnel_2;
    MapSlabCoord slb_x_tunnel_3;
    MapSlabCoord slb_y_tunnel_3;

    MapSlabCoord slb_x_bridge_1;
    MapSlabCoord slb_y_bridge_1;
    MapSlabCoord slb_x_bridge_2;
    MapSlabCoord slb_y_bridge_2;
    MapSlabCoord slb_x_bridge_3;
    MapSlabCoord slb_y_bridge_3;

    MapSlabCoord slb_x_door;
    MapSlabCoord slb_y_door;

    MapSlabCoord slb_x_room_start;
    MapSlabCoord slb_y_room_start;
    MapSlabCoord slb_x_room_end;
    MapSlabCoord slb_y_room_end;

    unsigned char total_imps_to_create;

    const unsigned char HUMAN_PLAYER;
    const unsigned char ENEMY_PLAYER;

    struct Thing* new_imp;
    struct Thing* door;

    TbBool should_create_graveyard;
    TbBool should_kill_hero;
};
struct ftest_bug_imp_tp_job_attack_door__variables ftest_bug_imp_tp_job_attack_door__vars = {
    .slb_x_tunnel_1 = 27,
    .slb_y_tunnel_1 = 41,
    .slb_x_tunnel_2 = 27,
    .slb_y_tunnel_2 = 55,

    .slb_x_bridge_1 = 16,
    .slb_y_bridge_1 = 40,
    .slb_x_bridge_2 = 27,
    .slb_y_bridge_2 = 40,

    .slb_x_door = 16,
    .slb_y_door = 38,

    .slb_x_room_start = 21,
    .slb_y_room_start = 70,
    .slb_x_room_end = 25,
    .slb_y_room_end = 72,

    .total_imps_to_create = 32,

    .HUMAN_PLAYER = PLAYER0,
    .ENEMY_PLAYER = PLAYER_GOOD,

    .new_imp = NULL,

    .should_create_graveyard = false,
    .should_kill_hero = false
};

// forward declarations - tests
FTestActionResult ftest_bug_imp_tp_job_attack_door_action001__setup_map(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_imp_tp_job_attack_door_action002__spawn_crippled_hero(struct FTestActionArgs* const args);
FTestActionResult ftest_bug_imp_tp_job_attack_door_action003__end_test(struct FTestActionArgs* const args);

TbBool ftest_tmp_delete_me(struct FTestActionArgs* const args);

TbBool ftest_bug_imp_tp_attack_door__claim_init()
{
    //here we can see that for each action we want to implement, we append the FTest_Action_Func and the game turn offset
    //note: you can add the same action multiple times
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action001__setup_map,              0,     NULL);
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action003__end_test,               350,   NULL);

    return true;
}

TbBool ftest_bug_imp_tp_attack_door__prisoner_init()
{
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action001__setup_map,              0,     NULL);
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action002__spawn_crippled_hero,    40,     NULL);
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action003__end_test,               350,   NULL);

    return true;
}

TbBool ftest_bug_imp_tp_attack_door__deadbody_init()
{
    ftest_bug_imp_tp_job_attack_door__vars.should_create_graveyard = true; //override to get a graveyard instead of prison
    ftest_bug_imp_tp_job_attack_door__vars.should_kill_hero = true; //override to get a dead body

    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action001__setup_map,              0,     NULL);
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action002__spawn_crippled_hero,    40,     NULL);
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action003__end_test,               350,   NULL);

    return true;
}

FTestActionResult ftest_bug_imp_tp_job_attack_door_action001__setup_map(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    struct ftest_bug_imp_tp_job_attack_door__variables* const vars = &ftest_bug_imp_tp_job_attack_door__vars;

    struct Thing* heartng = get_player_soul_container(vars->HUMAN_PLAYER);
    if (!thing_exists(heartng))
    {
        FTEST_FAIL_TEST("No dungeon heart found for player %d", vars->HUMAN_PLAYER);
        return FTRs_Go_To_Next_Action;
    }

    struct Dungeon* dungeon = get_dungeon(vars->HUMAN_PLAYER);
    if(dungeon_invalid(dungeon))
    {
        FTEST_FAIL_TEST("Dungeon for player %d not valid", vars->HUMAN_PLAYER);
        return FTRs_Go_To_Next_Action;
    }

    struct PlayerInfo* player = get_player(dungeon->owner);
    if(player_invalid(player))
    {
        FTEST_FAIL_TEST("Player %d not found", vars->HUMAN_PLAYER);
        return FTRs_Go_To_Next_Action;
    }

    ftest_util_reveal_map(vars->HUMAN_PLAYER); // we might want to see the entire map for testing purposes

    // carve tunnel, build bridge, towards locked door
    ftest_util_replace_slabs(vars->slb_x_tunnel_1, vars->slb_y_tunnel_1, vars->slb_x_tunnel_2, vars->slb_y_tunnel_2, SlbT_CLAIMED, vars->HUMAN_PLAYER); // then carve tunnel
    ftest_util_replace_slabs(vars->slb_x_bridge_1, vars->slb_y_bridge_1, vars->slb_x_bridge_2, vars->slb_y_bridge_2, SlbT_BRIDGE, vars->HUMAN_PLAYER); // then carve tunnel

    // claim tile in front of door (to prevent imps from teleport attacking the door)

    // create prison or graveyard
    if(vars->should_create_graveyard)
    {
        //graveyard
        ftest_util_replace_slabs(vars->slb_x_room_start, vars->slb_y_room_start, vars->slb_x_room_end, vars->slb_y_room_end, SlbT_GRAVEYARD, vars->HUMAN_PLAYER);
    }
    else
    {
        //prison
        ftest_util_replace_slabs(vars->slb_x_room_start, vars->slb_y_room_start, vars->slb_x_room_end, vars->slb_y_room_end, SlbT_PRISON, vars->HUMAN_PLAYER);
    }


    // create imps at start of tunnel
    struct Coord3d impPos;
    set_coords_to_slab_center(&impPos, vars->slb_x_tunnel_2, vars->slb_y_tunnel_2);

    vars->new_imp = INVALID_THING;
    for(unsigned char i = 0; i < vars->total_imps_to_create; ++i)
    {
        vars->new_imp = create_owned_special_digger(impPos.x.val, impPos.y.val, vars->HUMAN_PLAYER);
        if(thing_is_invalid(vars->new_imp))
        {
            FTEST_FAIL_TEST("Failed to create imp");
            return FTRs_Go_To_Next_Action;
        }
        // // example code for leveling creatures manually (not using dungeon special)
        // dungeon = get_dungeon(vars->new_imp->owner);
        // if(dungeon_invalid(dungeon))
        // {
        //     FTEST_FAIL_TEST("Invalid dungeon for created imp");
        //     return true;
        // }
        // struct CreatureControl* cctrl = creature_control_get_from_thing(vars->new_imp);
        // if(creature_control_invalid(cctrl))
        // {
        //     FTEST_FAIL_TEST("Creature control invalid for imp");
        //     return true;
        // }

        // if(!creature_change_multiple_levels(vars->new_imp, 9))
        // {
        //     FTEST_FAIL_TEST("Failed to level up imp");
        //     return true;
        // }
    }

    // use dungeon special to level imps to 10
    increase_level(player, 9);

    // find the enemy door
    MapSubtlCoord stl_x_door = slab_subtile_center(vars->slb_x_door);
    MapSubtlCoord stl_y_door = slab_subtile_center(vars->slb_y_door);
    vars->door = get_door_for_position(stl_x_door, stl_y_door);
    if (thing_is_invalid(vars->door))
    {
        FTEST_FAIL_TEST("Failed to find door at (%d,%d), this should never happen! Was the map changed!?", vars->slb_x_door, vars->slb_y_door);
        return FTRs_Go_To_Next_Action;
    }
    
    // lower door health to speed up test
    vars->door->health = 10;

    // move camera to door
    ftest_util_move_camera_to_slab(vars->slb_x_door, vars->slb_y_door, vars->HUMAN_PLAYER);
    
    // enable inprisonment (bypasses prison gui button, button in gui will not represent actual current state)
    if (!set_creature_tendencies(player, CrTend_Imprison, true))
    {
        FTEST_FAIL_TEST("Failed to set imprison true for player %d", vars->HUMAN_PLAYER);
        return FTRs_Go_To_Next_Action;
    }

    return FTRs_Go_To_Next_Action;
}

FTestActionResult ftest_bug_imp_tp_job_attack_door_action002__spawn_crippled_hero(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    struct ftest_bug_imp_tp_job_attack_door__variables* const vars = &ftest_bug_imp_tp_job_attack_door__vars;

    // create an enemy hero in front of the door, cripple them
    struct Coord3d heroPos;
    set_coords_to_slab_center(&heroPos, vars->slb_x_door-1, vars->slb_y_door);
    //fudge y val to get hero closer to door
    heroPos.y.val += 512;

    struct Thing* new_hero = create_creature(&heroPos, 5, vars->ENEMY_PLAYER);
    if(thing_is_invalid(new_hero))
    {
        FTEST_FAIL_TEST("Failed to create hero");
        return FTRs_Go_To_Next_Action;
    }

    // move camera to hero
    //ftest_util_move_camera_to_thing(new_hero, PLAYER0);

    // either knock hero out (CrDed_Default does this when imprison flag is set) or kill hero (CrDed_NoUnconscious bypasses imprison flag)
    CrDeathFlags death_flags = vars->should_kill_hero ? CrDed_NoUnconscious : CrDed_Default;
    
    if(!kill_creature(new_hero, vars->new_imp, vars->HUMAN_PLAYER, death_flags))
    {
        if(new_hero->health != 1)
        {
            FTEST_FAIL_TEST("Failed to cripple hero");
            return FTRs_Go_To_Next_Action;
        }
    }

    return FTRs_Go_To_Next_Action;
}

FTestActionResult ftest_bug_imp_tp_job_attack_door_action003__end_test(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    struct ftest_bug_imp_tp_job_attack_door__variables* const vars = &ftest_bug_imp_tp_job_attack_door__vars;

    // fail test if the door doesn't exist!
    MapSubtlCoord stl_x_door = slab_subtile_center(vars->slb_x_door);
    MapSubtlCoord stl_y_door = slab_subtile_center(vars->slb_y_door);
    struct Thing* door = get_door_for_position(stl_x_door, stl_y_door);
    if (thing_is_invalid(door))
    {
        FTEST_FAIL_TEST("Failed to find door at (%d,%d), imps destroyed door!", vars->slb_x_door, vars->slb_y_door);
        return FTRs_Go_To_Next_Action;
    }

    if(door != vars->door)
    {
        FTEST_FAIL_TEST("Found different door... what?!");
        return FTRs_Go_To_Next_Action;
    }

    return FTRs_Go_To_Next_Action;
}

#endif
