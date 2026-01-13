/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file spdigger_stack.h
 *     Header file for spdigger_stack.c.
 * @par Purpose:
 *     Special diggers task stack support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 04 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_SPDIGGER_STACK_H
#define DK_SPDIGGER_STACK_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
enum SpecialDiggerTask {
    DigTsk_None = 0,
    DigTsk_ImproveDungeon,
    DigTsk_ConvertDungeon,
    DigTsk_ReinforceWall,
    DigTsk_PickUpUnconscious,
    DigTsk_PickUpCorpse,
    DigTsk_PicksUpSpellBook,
    DigTsk_PicksUpCrateToArm,
    DigTsk_PicksUpCrateForWorkshop,
    DigTsk_DigOrMine,
    DigTsk_PicksUpGoldPile, // 10
    DigTsk_SaveUnconscious
};

enum SpecialLastJobKinds {
    SDLstJob_None = 0,
    SDLstJob_DigOrMine,
    SDLstJob_ConvImprDungeon,
    SDLstJob_ReinforceWallUnprompted,
    SDLstJob_NonDiggerTask,
    SDLstJob_Unkn5,
    SDLstJob_Unkn6,
    SDLstJob_Unkn7,
    SDLstJob_Unkn8,
    SDLstJob_ReinforceWallAssigned,
};

enum SpecialDiggerDigTaskKinds {
    SDDigTask_None = 0,
    SDDigTask_DigEarth,
    SDDigTask_MineGold,
    SDDigTask_MineGems,
};

enum ThingForRoomPickabilityFlags {
    TngFRPickF_Default = 0,
    TngFRPickF_AllowStoredInOwnedRoom = 0x0001, //*< Allow picking up things which already are in their designated rooms
};

/******************************************************************************/
#pragma pack(1)

struct Dungeon;
struct Thing;

struct SlabCoord {

    unsigned char x;
    unsigned char y;
};

struct ExtraSquares {
    unsigned long index;
    unsigned char flgmask;
};

#pragma pack()
/******************************************************************************/
TbBool creature_task_needs_check_out_after_digger_stack_change(const struct Thing *creatng);
void remove_task_from_all_other_players_digger_stacks(PlayerNumber skip_plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);

long find_in_imp_stack_using_pos(SubtlCodedCoords stl_num, SpDiggerTaskType task_type, const struct Dungeon *dungeon);
long find_in_imp_stack_starting_at(SpDiggerTaskType task_type, long start_pos, const struct Dungeon *dungeon);
long find_in_imp_stack_task_other_than_starting_at(SpDiggerTaskType excl_task_type, long start_pos, const struct Dungeon *dungeon);

TbBool add_to_imp_stack_using_pos(SubtlCodedCoords stl_num, SpDiggerTaskType task_type, struct Dungeon *dungeon);
TbBool add_object_for_trap_to_imp_stack(struct Dungeon *dungeon, struct Thing *thing);
void setup_imp_stack(struct Dungeon *dungeon);
int add_undug_to_imp_stack(struct Dungeon *dungeon, int max_tasks);
int add_gems_to_imp_stack(struct Dungeon *dungeon, int max_tasks);
int add_pretty_and_convert_to_imp_stack(struct Dungeon *dungeon, int max_tasks);
int add_unclaimed_gold_to_imp_stack(struct Dungeon *dungeon, int max_tasks);
int add_unclaimed_unconscious_bodies_to_imp_stack(struct Dungeon *dungeon, int max_tasks);
int add_unclaimed_dead_bodies_to_imp_stack(struct Dungeon *dungeon, int max_tasks);
int add_unclaimed_spells_to_imp_stack(struct Dungeon *dungeon, int max_tasks);
int add_empty_traps_to_imp_stack(struct Dungeon *dungeon, int max_tasks);
int add_unclaimed_traps_to_imp_stack(struct Dungeon *dungeon, int max_tasks);
int add_reinforce_to_imp_stack(struct Dungeon *dungeon, int max_tasks);
int add_unsaved_unconscious_creature_to_imp_stack(struct Dungeon *dungeon, int max_tasks);

TbBool imp_will_soon_be_arming_trap(struct Thing *traptng);
TbBool imp_will_soon_be_working_at_excluding(const struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool imp_will_soon_be_getting_object(PlayerNumber plyr_idx, const struct Thing *objtng);
TbBool is_digging_indestructible_place(const struct Thing *creatng);
TbBool imp_already_reinforcing_at_excluding(struct Thing *spdigtng, MapSubtlCoord stl_x, MapSubtlCoord stl_y);

TbBool thing_can_be_picked_to_place_in_player_room_of_role(const struct Thing* thing, PlayerNumber plyr_idx, RoomRole rrole, unsigned short flags);
long get_random_mining_undug_area_position_for_digger_drop(PlayerNumber plyr_idx, MapSubtlCoord *retstl_x, MapSubtlCoord *retstl_y);
TbBool creature_can_pickup_library_object_at_subtile(struct Thing* spdigtng, MapSubtlCoord stl_x, MapSubtlCoord stl_y);

TbBool imp_stack_update(struct Thing *creatng);
TbBool check_out_imp_stack(struct Thing *creatng);
long check_out_imp_last_did(struct Thing *creatng);
long check_place_to_convert_excluding(struct Thing *thing, MapSlabCoord slb_x, MapSlabCoord slb_y);
long check_place_to_pretty_excluding(struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y);
long check_out_imp_has_money_for_treasure_room(struct Thing *thing);
long check_out_available_imp_tasks(struct Thing *thing);
long check_out_imp_tokes(struct Thing *thing);
long check_place_to_dig_and_get_position(struct Thing *thing, SubtlCodedCoords stl_num, MapSubtlCoord *retstl_x, MapSubtlCoord *retstl_y);
long check_place_to_dig_and_get_drop_position(PlayerNumber plyr_idx, SubtlCodedCoords stl_num, MapSubtlCoord *retstl_x, MapSubtlCoord *retstl_y);
long check_place_to_reinforce(struct Thing *creatng, MapSlabCoord slb_x, MapSlabCoord slb_y);
long check_out_uncrowded_reinforce_position(struct Thing *thing, SubtlCodedCoords stl_num, MapSubtlCoord *retstl_x, MapSubtlCoord *retstl_y);
long check_out_unconverted_spiral(struct Thing *thing, long nslabs);
void force_any_creature_dragging_owned_thing_to_drop_it(struct Thing *dragtng);
void force_any_creature_dragging_thing_to_drop_it(struct Thing *dragtng);
long check_out_unprettied_spiral(struct Thing *thing, long nslabs);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
