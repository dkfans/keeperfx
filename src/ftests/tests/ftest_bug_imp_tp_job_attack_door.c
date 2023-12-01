#include "ftest_bug_imp_tp_job_attack_door.h"

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
struct ftest_bug_imp_tp_job_attack_door__variables
{
    MapSlabCoord slb_x_tunnel_start;
    MapSlabCoord slb_y_tunnel_start;
    MapSlabCoord slb_x_tunnel_end;
    MapSlabCoord slb_y_tunnel_end;

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
};
struct ftest_bug_imp_tp_job_attack_door__variables ftest_bug_imp_tp_job_attack_door__vars = {
    .slb_x_tunnel_start = 3,
    .slb_y_tunnel_start = 6,
    .slb_x_tunnel_end = 3,
    .slb_y_tunnel_end = 75,

    .slb_x_door = 6,
    .slb_y_door = 3,

    .slb_x_room_start = 7,
    .slb_y_room_start = 1,
    .slb_x_room_end = 11,
    .slb_y_room_end = 5,

    .total_imps_to_create = 32,

    .HUMAN_PLAYER = PLAYER0,
    .ENEMY_PLAYER = PLAYER_GOOD,

    .new_imp = NULL
};

// forward declarations - tests
TbBool ftest_bug_imp_tp_job_attack_door_action001__setup_map(struct FTestActionArgs* const args);
TbBool ftest_bug_imp_tp_job_attack_door_action002__spawn_crippled_hero(struct FTestActionArgs* const args);
TbBool ftest_bug_imp_tp_job_attack_door_action003__end_test(struct FTestActionArgs* const args);

TbBool ftest_bug_imp_tp_attack_door_init()
{
    //here we can see that for each action we want to implement, we append the FTest_Action_Func and the game turn offset
    //note: you can add the same action multiple times
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action001__setup_map,              20,     NULL);
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action002__spawn_crippled_hero,    40,     NULL);
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action003__end_test,               1000,   NULL);

    return true;
}

TbBool ftest_bug_imp_tp_job_attack_door_action001__setup_map(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    struct ftest_bug_imp_tp_job_attack_door__variables* const vars = &ftest_bug_imp_tp_job_attack_door__vars;

    struct Thing* heartng = get_player_soul_container(vars->HUMAN_PLAYER);
    if (!thing_exists(heartng))
    {
        FTEST_FAIL_TEST("No dungeon heart found for player %d", vars->HUMAN_PLAYER);
        return false;
    }

    struct Dungeon* dungeon = get_dungeon(vars->HUMAN_PLAYER);
    if(dungeon_invalid(dungeon))
    {
        FTEST_FAIL_TEST("Dungeon for player %d not valid", vars->HUMAN_PLAYER);
        return true;
    }

    struct PlayerInfo* player = get_player(dungeon->owner);
    if(player_invalid(player))
    {
        FTEST_FAIL_TEST("Player %d not found", vars->HUMAN_PLAYER);
        return true;
    }

    ftest_util_reveal_map(vars->HUMAN_PLAYER); // we might want to see the entire map for testing purposes

    // carve long tunnel to spawn imp at the end (imp will prefer to teleport to job site)
    ftest_util_replace_slabs(vars->slb_x_tunnel_start, vars->slb_y_tunnel_start, vars->slb_x_tunnel_end, vars->slb_y_tunnel_end, SlbT_CLAIMED, vars->HUMAN_PLAYER); // then carve tunnel

    // carve door frame for enemy player
    ftest_util_replace_slabs(vars->slb_x_door, vars->slb_y_door, vars->slb_x_door, vars->slb_y_door, SlbT_CLAIMED, vars->ENEMY_PLAYER);

    // carve out empty room (ownership will be checked at end of test)
    ftest_util_replace_slabs(vars->slb_x_room_start, vars->slb_y_room_start, vars->slb_x_room_end, vars->slb_y_room_end, SlbT_PATH, PLAYER_NEUTRAL);

    // create enemy wooden door with low health
    struct Coord3d doorPos;
    set_coords_to_slab_center(&doorPos, vars->slb_x_door, vars->slb_y_door);

    //ThingModel doorModel = gameadd.trapdoor_conf.door_to_object[1]; // couldn't find proper type mapping
    ThingModel doorModel = 1; // wooden door == 1 (hardcoded for now)
    unsigned char orient = find_door_angle(doorPos.x.stl.num, doorPos.y.stl.num, vars->ENEMY_PLAYER);
    struct Thing* new_door = create_door(&doorPos, doorModel, orient, vars->ENEMY_PLAYER, true); 
    if(thing_is_invalid(new_door))
    {
        FTEST_FAIL_TEST("Failed to create locked door");
        return true;
    }
    new_door->health = 10;

    // create prison tile at end of tunnel
    ftest_util_replace_slabs(vars->slb_x_tunnel_end, vars->slb_y_tunnel_end, vars->slb_x_tunnel_end, vars->slb_y_tunnel_end, SlbT_PRISON, vars->HUMAN_PLAYER);

    // create imps at end of tunnel and max-level them
    struct Coord3d impPos;
    set_coords_to_slab_center(&impPos, vars->slb_x_tunnel_end, vars->slb_y_tunnel_end);
    
    vars->new_imp = INVALID_THING;
    for(unsigned char i = 0; i < vars->total_imps_to_create; ++i)
    {
        vars->new_imp = create_owned_special_digger(impPos.x.val, impPos.y.val, vars->HUMAN_PLAYER);
        if(thing_is_invalid(vars->new_imp))
        {
            FTEST_FAIL_TEST("Failed to create imp");
            return true;
        }
        dungeon = get_dungeon(vars->new_imp->owner);
        if(dungeon_invalid(dungeon))
        {
            FTEST_FAIL_TEST("Invalid dungeon for created imp");
            return true;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(vars->new_imp);
        if(creature_control_invalid(cctrl))
        {
            FTEST_FAIL_TEST("Creature control invalid for imp");
            return true;
        }

        if(!creature_change_multiple_levels(vars->new_imp, 9))
        {
            FTEST_FAIL_TEST("Failed to level up imp");
            return true;
        }
    }

    // enable inprisonment
    if (!set_creature_tendencies(player, CrTend_Imprison, true))
    {
        FTEST_FAIL_TEST("Failed to set imprison true for player %d", vars->HUMAN_PLAYER);
        return true;
    }

    return true;
}

TbBool ftest_bug_imp_tp_job_attack_door_action002__spawn_crippled_hero(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    struct ftest_bug_imp_tp_job_attack_door__variables* const vars = &ftest_bug_imp_tp_job_attack_door__vars;

    // create an enemy hero in front of the door, cripple them
    struct Coord3d heroPos;
    set_coords_to_slab_center(&heroPos, vars->slb_x_door-1, vars->slb_y_door);
    //fudge x val to get hero closer to door
    heroPos.x.val += 512;

    struct Thing* new_hero = create_creature(&heroPos, 5, vars->ENEMY_PLAYER);
    if(thing_is_invalid(new_hero))
    {
        FTEST_FAIL_TEST("Failed to create hero");
        return true;
    }
    
    if(!kill_creature(new_hero, vars->new_imp, vars->HUMAN_PLAYER, CrDed_Default))
    {
        if(new_hero->health != 1)
        {
            FTEST_FAIL_TEST("Failed to cripple hero");
            return true;
        }
    }

    return true;
}

TbBool ftest_bug_imp_tp_job_attack_door_action003__end_test(struct FTestActionArgs* const args)
{
    // to make the test variable names shorter, use a pointer!
    struct ftest_bug_imp_tp_job_attack_door__variables* const vars = &ftest_bug_imp_tp_job_attack_door__vars;

    // check ownership of tiles in room (if player0 owns any at end of test, it means failure, imp broke door)
    if(ftest_util_does_player_own_any_slabs(vars->slb_x_room_start, vars->slb_y_room_start, vars->slb_x_room_end, vars->slb_y_room_end, vars->HUMAN_PLAYER))
    {
        FTEST_FAIL_TEST("Failed because human player should not own any slabs in the area (%u,%d,%d,%d)", vars->slb_x_room_start, vars->slb_y_room_start, vars->slb_x_room_end, vars->slb_y_room_end);
    }

    return true;
}

#endif