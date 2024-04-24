/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file power_hand.h
 *     Header file for power_hand.c.
 * @par Purpose:
 *     power_hand functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_POWERHAND_H
#define DK_POWERHAND_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct PlayerInfo;
struct Dungeon;

#define HAND_RULE_SLOTS_COUNT 8

#pragma pack()
/******************************************************************************/
void add_creature_to_sacrifice_list(PlayerNumber owner, long model, long explevel);
void place_thing_in_limbo(struct Thing *thing);
void remove_thing_from_limbo(struct Thing *thing);
unsigned long object_is_pickable_by_hand_for_use(const struct Thing *thing, long a2);
TbBool thing_is_pickable_by_hand(struct PlayerInfo *player, const struct Thing *thing);
struct Thing *process_object_being_picked_up(struct Thing *thing, long a2);
void set_power_hand_graphic(unsigned char plyr_idx, long HandAnimationID);
TbBool power_hand_is_empty(const struct PlayerInfo *player);
TbBool power_hand_is_full(const struct PlayerInfo *player);
struct Thing *get_first_thing_in_power_hand(struct PlayerInfo *player);
void draw_power_hand(void);
void clear_things_in_hand(struct PlayerInfo *player);
TbResult magic_use_power_hand(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short tng_idx);
struct Thing *get_nearest_thing_for_hand_or_slap(PlayerNumber plyr_idx, MapCoord x, MapCoord y);

TbBool insert_thing_into_power_hand_list(struct Thing *thing, PlayerNumber plyr_idx);
TbBool remove_thing_from_power_hand_list(struct Thing *thing, PlayerNumber plyr_idx);
TbBool remove_first_thing_from_power_hand_list(PlayerNumber plyr_idx);
TbBool thing_is_in_power_hand_list(const struct Thing *thing, PlayerNumber plyr_idx);

long can_thing_be_picked_up_by_player(const struct Thing *thing, PlayerNumber plyr_idx);
TbBool can_thing_be_picked_up2_by_player(const struct Thing *thing, PlayerNumber plyr_idx);

TbBool thing_is_picked_up(const struct Thing *thing);
TbBool thing_is_picked_up_by_owner(const struct Thing *thing);
TbBool thing_is_picked_up_by_enemy(const struct Thing *thing);
TbBool thing_is_picked_up_by_player(const struct Thing *thing, PlayerNumber plyr_idx);
long get_thing_in_hand_id(const struct Thing* thing, PlayerNumber plyr_idx);

TbBool slap_object(struct Thing *thing);
TbBool object_is_slappable(const struct Thing *thing, long plyr_idx);
TbBool thing_slappable(const struct Thing *thing, long plyr_idx);

struct Thing *create_power_hand(PlayerNumber owner);
void delete_power_hand(PlayerNumber owner);
void stop_creatures_around_hand(PlayerNumber plyr_idx, MapSubtlCoord stl_x,  MapSubtlCoord stl_y);

TbBool place_thing_in_power_hand(struct Thing *thing, PlayerNumber plyr_idx);
TbBool remove_creature_from_power_hand(struct Thing *thing, PlayerNumber plyr_idx);
void drop_held_thing_on_ground(struct Dungeon *dungeon, struct Thing *droptng, const struct Coord3d *dstpos);
void drop_gold_coins(const struct Coord3d *pos, long value, long plyr_idx);
TbBool is_dangerous_drop_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
short can_place_thing_here(struct Thing *thing, long x, long y, long dngn_idx);
TbBool can_drop_thing_here(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, unsigned long allow_unclaimed);
TbBool armageddon_blocks_creature_pickup(const struct Thing *thing, PlayerNumber plyr_idx);
TbBool thing_pickup_is_blocked_by_hand_rule(const struct Thing *thing_to_pick, PlayerNumber plyr_idx);

enum HandRuleType {
    // hand_rule_test_fns are indexed by these enum values -> reordering or adding new types affects test_fns
    HandRule_Unset,
    HandRule_Always,
    HandRule_AgeLower,
    HandRule_AgeHigher,
    HandRule_LvlLower,
    HandRule_LvlHigher,
    HandRule_AtActionPoint,
    HandRule_AffectedBy,
    HandRule_Wandering,
    HandRule_Working,
    HandRule_Fighting,
    HandRule_DroppedTimeHigher,
    HandRule_DroppedTimeLower
};

enum HandRuleAction {
    HandRuleAction_Deny,
    HandRuleAction_Allow,
    HandRuleAction_Enable,
    HandRuleAction_Disable,
};

struct HandRule {
    char type;
    char enabled;
    char allow; // allow: 1, deny: 0
    long param;
};

TbBool eval_hand_rule_for_thing(struct HandRule *rule, const struct Thing *thing_to_pick);

extern float global_hand_scale;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
