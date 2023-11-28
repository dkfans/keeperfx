#include "ftest_bug_imp_tp_job_attack_door.h"

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


MapSlabCoord slb_x_tunnel_start = 3;
MapSlabCoord slb_y_tunnel_start = 6;
MapSlabCoord slb_x_tunnel_end = 3;
MapSlabCoord slb_y_tunnel_end = 75;

MapSlabCoord slb_x_door = 6;
MapSlabCoord slb_y_door = 3;

MapSlabCoord slb_x_room_start = 7;
MapSlabCoord slb_y_room_start = 1;
MapSlabCoord slb_x_room_end = 11;
MapSlabCoord slb_y_room_end = 5;

unsigned char total_imps_to_create = 12;

const unsigned char HUMAN_PLAYER = PLAYER0;
const unsigned char ENEMY_PLAYER = PLAYER_GOOD;

struct Thing* new_imp = NULL;

// forward declarations - tests
TbBool ftest_bug_imp_tp_job_attack_door_action001__setup_map(GameTurn game_turn);
TbBool ftest_bug_imp_tp_job_attack_door_action002__spawn_crippled_hero(GameTurn game_turn);
TbBool ftest_bug_imp_tp_job_attack_door_action003__end_test(GameTurn game_turn);

TbBool ftest_bug_imp_tp_attack_door_init()
{
    //here we can see that for each action we want to implement, we append the FTest_Action_Func and the game turn offset
    //note: you can add the same action multiple times
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action001__setup_map, 20);
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action002__spawn_crippled_hero, 40);
    ftest_append_action(ftest_bug_imp_tp_job_attack_door_action003__end_test, 1000);

    return true;
}

TbBool ftest_bug_imp_tp_job_attack_door_action001__setup_map(GameTurn game_turn)
{
    struct Thing* heartng = get_player_soul_container(HUMAN_PLAYER);
    if (!thing_exists(heartng))
    {
        FTEST_FAIL_TEST("No dungeon heart found for player %d", HUMAN_PLAYER);
        return false;
    }

    struct Dungeon* dungeon = get_dungeon(HUMAN_PLAYER);
    if(dungeon_invalid(dungeon))
    {
        FTEST_FAIL_TEST("Dungeon for player %d not valid", HUMAN_PLAYER);
        return true;
    }

    struct PlayerInfo* player = get_player(dungeon->owner);
    if(player_invalid(player))
    {
        FTEST_FAIL_TEST("Player %d not found", HUMAN_PLAYER);
        return true;
    }

    ftest_reveal_map(HUMAN_PLAYER); // we might want to see the entire map for testing purposes

    // carve long tunnel to spawn imp at the end (imp will prefer to teleport to job site)
    ftest_replace_slabs(slb_x_tunnel_start, slb_y_tunnel_start, slb_x_tunnel_end, slb_y_tunnel_end, SlbT_CLAIMED, HUMAN_PLAYER);

    // carve door frame for enemy player
    ftest_replace_slabs(slb_x_door, slb_y_door, slb_x_door, slb_y_door, SlbT_CLAIMED, ENEMY_PLAYER);

    // carve out empty room (ownership will be checked at end of test)
    ftest_replace_slabs(slb_x_room_start, slb_y_room_start, slb_x_room_end, slb_y_room_end, SlbT_PATH, -1);

    // create enemy wooden door with low health
    struct Coord3d doorPos;
    doorPos.x.val = 0; // setting to 0 for x/y is required before setting stl.num/pos, otherwise val will be incorrect...
    doorPos.y.val = 0;
    doorPos.x.stl.num = slab_subtile_center(slb_x_door);
    doorPos.x.stl.pos = 128; //128 == 50% - center of tile
    doorPos.y.stl.num = slab_subtile_center(slb_y_door);
    doorPos.y.stl.pos = 128;

    //ThingModel doorModel = gameadd.trapdoor_conf.door_to_object[1]; // couldn't find proper type mapping
    ThingModel doorModel = 1; // wooden door == 1 (hardcoded for now)
    unsigned char orient = find_door_angle(doorPos.x.stl.num, doorPos.y.stl.num, ENEMY_PLAYER);
    struct Thing* new_door = create_door(&doorPos, doorModel, orient, ENEMY_PLAYER, true); 
    if(thing_is_invalid(new_door))
    {
        FTEST_FAIL_TEST("Failed to create locked door");
        return true;
    }
    new_door->health = 10;

    // create prison tile at end of tunnel
    ftest_replace_slabs(slb_x_tunnel_end, slb_y_tunnel_end, slb_x_tunnel_end, slb_y_tunnel_end, SlbT_PRISON, HUMAN_PLAYER);

    // create imps at end of tunnel and max-level them
    struct Coord3d impPos;
    impPos.x.val = 0; // setting to 0 for x/y is required before setting stl.num/pos, otherwise val will be incorrect...
    impPos.y.val = 0;
    impPos.x.stl.num = slab_subtile_center(slb_x_tunnel_end);
    impPos.x.stl.pos = 128; //128 == 50% - center of tile
    impPos.y.stl.num = slab_subtile_center(slb_y_tunnel_end);
    impPos.y.stl.pos = 128; 
    
    new_imp = INVALID_THING;
    for(unsigned char i = 0; i < total_imps_to_create; ++i)
    {
        new_imp = create_owned_special_digger(impPos.x.val, impPos.y.val, HUMAN_PLAYER);
        if(thing_is_invalid(new_imp))
        {
            FTEST_FAIL_TEST("Failed to create imp");
            return true;
        }
        dungeon = get_dungeon(new_imp->owner);
        if(dungeon_invalid(dungeon))
        {
            FTEST_FAIL_TEST("Invalid dungeon for created imp");
            return true;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(new_imp);
        if(creature_control_invalid(cctrl))
        {
            FTEST_FAIL_TEST("Creature control invalid for imp");
            return true;
        }

        if(!creature_change_multiple_levels(new_imp, 9))
        {
            FTEST_FAIL_TEST("Failed to level up imp");
            return true;
        }
    }

    // enable inprisonment
    if (!set_creature_tendencies(player, CrTend_Imprison, true))
    {
        FTEST_FAIL_TEST("Failed to set imprison true for player %d", HUMAN_PLAYER);
        return true;
    }

    return true;
}

TbBool ftest_bug_imp_tp_job_attack_door_action002__spawn_crippled_hero(GameTurn game_turn)
{
    // create an enemy hero in front of the door, cripple them
    struct Coord3d heroPos;
    heroPos.x.val = 0; // setting to 0 for x/y is required before setting stl.num/pos, otherwise val will be incorrect...
    heroPos.y.val = 0;
    heroPos.x.stl.num = slab_subtile_center(slb_x_door-1);
    heroPos.x.stl.pos = 255; //100% - right of tile
    heroPos.y.stl.num = slab_subtile_center(slb_y_door);
    heroPos.y.stl.pos = 128;
    //fudge x val to get hero closer to door
    heroPos.x.val += 512;

    struct Thing* new_hero = create_creature(&heroPos, 5, ENEMY_PLAYER);
    if(thing_is_invalid(new_hero))
    {
        FTEST_FAIL_TEST("Failed to create hero");
        return true;
    }
    
    if(!kill_creature(new_hero, new_imp, HUMAN_PLAYER, CrDed_Default))
    {
        if(new_hero->health != 1)
        {
            FTEST_FAIL_TEST("Failed to cripple hero");
            return true;
        }
    }

    return true;
}

TbBool ftest_bug_imp_tp_job_attack_door_action003__end_test(GameTurn game_turn)
{
    // check ownership of tiles in room (if player0 owns any at end of test, it means failure, imp broke door)
    if(ftest_does_player_own_any_slabs(slb_x_room_start, slb_y_room_start, slb_x_room_end, slb_y_room_end, HUMAN_PLAYER))
    {
        FTEST_FAIL_TEST("Failed because human player should not own any slabs in the area (%u,%d,%d,%d)", slb_x_room_start, slb_y_room_start, slb_x_room_end, slb_y_room_end);
    }

    return true;
}

#endif