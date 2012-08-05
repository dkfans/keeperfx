/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_list.h
 *     Header file for thing_list.c.
 * @par Purpose:
 *     Things list support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     12 Feb 2009 - 24 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THINGLIST_H
#define DK_THINGLIST_H

#include "globals.h"
#include "bflib_basics.h"

#include "thing_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define THING_CLASSES_COUNT    14
#define THINGS_COUNT         2048

enum ThingClassIndex {
    TCls_Empty        =  0,
    TCls_Object       =  1,
    TCls_Shot         =  2,
    TCls_EffectElem   =  3,
    TCls_DeadCreature =  4,
    TCls_Creature     =  5,
    TCls_Effect       =  6,
    TCls_EffectGen    =  7,
    TCls_Trap         =  8,
    TCls_Door         =  9,
    TCls_Unkn10       = 10,
    TCls_Unkn11       = 11,
    TCls_AmbientSnd   = 12,
    TCls_CaveIn       = 13,
};

enum ThingListIndex {
    TngList_Creatures    =  0,
    TngList_Shots        =  1,
    TngList_Objects      =  2,
    TngList_EffectElems  =  3,
    TngList_DeadCreatrs  =  4,
    TngList_Effects      =  5,
    TngList_EffectGens   =  6,
    TngList_Traps        =  7,
    TngList_Doors        =  8,
    TngList_AmbientSnds  =  9,
    TngList_CaveIns      = 10,
    TngList_unk11        = 11,
    TngList_unk12        = 12,
};
/******************************************************************************/
#pragma pack(1)

struct PlayerInfo;
struct Thing;
struct CompoundFilterParam;

typedef long FilterParam;
typedef struct CompoundFilterParam * MaxFilterParam;

typedef long (*Thing_State_Func)(struct Thing *);
typedef long (*Thing_Class_Func)(struct Thing *);
typedef long (*Thing_Filter)(const struct Thing *, FilterParam);
typedef long (*Thing_Maximizer_Filter)(const struct Thing *, MaxFilterParam, long);
typedef long (*Thing_Collide_Func)(struct Thing *, struct Thing *, long, long);

struct CompoundFilterParam {
     long plyr_idx;
     long class_id;
     long model_id;
     union {
     long num1;
     void *ptr1;
     };
     union {
     long num2;
     void *ptr2;
     };
     union {
     long num3;
     void *ptr3;
     };
};

struct StructureList {
     unsigned long count;
     unsigned long index;
};

struct Things {
    struct Thing *lookup[THINGS_COUNT];
    struct Thing *end;
};


#pragma pack()
/******************************************************************************/
extern Thing_Class_Func class_functions[];
extern unsigned long thing_create_errors;
/******************************************************************************/
void add_thing_to_list(struct Thing *thing, struct StructureList *list);
void remove_thing_from_list(struct Thing *thing, struct StructureList *slist);
void remove_thing_from_its_class_list(struct Thing *thing);
void add_thing_to_its_class_list(struct Thing *thing);

long creature_near_filter_not_imp(const struct Thing *thing, FilterParam val);
long creature_near_filter_is_enemy_of_and_not_specdigger(const struct Thing *thing, FilterParam val);
long creature_near_filter_is_owned_by(const struct Thing *thing, FilterParam val);

// Filters to select thing belonging to given player
struct Thing *get_player_list_creature_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxFilterParam param);
struct Thing *get_player_list_random_creature_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxFilterParam param);
long count_player_list_creatures_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxFilterParam param);
// Final routines to select thing belonging to given player
struct Thing *get_player_list_nth_creature_of_model(long thing_idx, ThingModel crmodel, long crtr_idx);
struct Thing *get_random_players_creature_of_model(long plyr_idx, ThingModel crmodel);
// Filters to select thing on/near given map position
struct Thing *get_thing_on_map_block_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxFilterParam param, long *maximizer);
struct Thing *get_thing_near_revealed_map_block_with_filter(MapCoord x, MapCoord y, Thing_Maximizer_Filter filter, MaxFilterParam param);
struct Thing *get_thing_spiral_near_map_block_with_filter(MapCoord x, MapCoord y, long spiral_len, Thing_Maximizer_Filter filter, MaxFilterParam param);
long count_things_spiral_near_map_block_with_filter(MapCoord x, MapCoord y, long spiral_len, Thing_Maximizer_Filter filter, MaxFilterParam param);
// Final routines to select thing on/near given map position
struct Thing *get_creature_near_but_not_specdigger(MapCoord pos_x, MapCoord pos_y, long plyr_idx);
struct Thing *get_creature_near_who_is_enemy_of_and_not_specdigger(MapCoord pos_x, MapCoord pos_y, long plyr_idx);
struct Thing *get_creature_near_and_owned_by(MapCoord pos_x, MapCoord pos_y, long plyr_idx);
struct Thing *get_creature_near_and_owned_by_or_allied_with(MapCoord pos_x, MapCoord pos_y, long distance_stl, long plyr_idx);
struct Thing *get_creature_of_model_training_at_subtile_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long model_id, long plyr_idx, long skip_thing_id);
struct Thing *get_object_at_subtile_of_model_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long model, long plyr_idx);
struct Thing *get_trap_at_subtile_of_model_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long model, long plyr_idx);
struct Thing *get_trap_around_of_model_and_owned_by(MapCoord pos_x, MapCoord pos_y, long model, long plyr_idx);
struct Thing *get_door_for_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Thing *get_nearest_object_at_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
long count_creatures_near_and_owned_by_or_allied_with(MapCoord pos_x, MapCoord pos_y, long distance_stl, long plyr_idx);

unsigned long update_things_sounds_in_list(struct StructureList *list);
void stop_all_things_playing_samples(void);
unsigned long update_cave_in_things(void);
unsigned long update_creatures_not_in_list(void);
unsigned long update_things_in_list(struct StructureList *list);
void init_player_start(struct PlayerInfo *player);
void setup_computer_players(void);
void setup_zombie_players(void);
void init_all_creature_states(void);

long creature_of_model_in_prison_or_tortured(ThingModel crmodel);
long count_player_creatures_of_model(PlayerNumber plyr_idx, ThingModel crmodel);
long count_player_list_creatures_of_model(long thing_idx, ThingModel crmodel);
long count_player_creatures_not_counting_to_total(long plyr_idx);
TbBool lord_of_the_land_in_prison_or_tortured(void);
struct Thing *find_nearest_enemy_creature(struct Thing *crtng);
long electricity_affecting_area(struct Coord3d *pos, PlayerNumber immune_plyr_idx, long range, long max_damage);

void update_things(void);

struct Thing *find_base_thing_on_mapwho(ThingClass oclass, ThingModel okind, MapSubtlCoord stl_x, MapSubtlCoord stl_y);

struct Thing *find_hero_gate_of_number(long num);
long get_free_hero_gate_number(void);

struct Thing *find_creature_lair_at_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, ThingModel crmodel);

long thing_is_shootable_by_any_player_including_objects(struct Thing *thing);
long thing_is_shootable_by_any_player_except_own_including_objects(struct Thing *shooter, struct Thing *thing);
long thing_is_shootable_by_any_player_except_own_excluding_objects(struct Thing *shooter, struct Thing *thing);
long thing_is_shootable_by_any_player_excluding_objects(struct Thing *thing);
TbBool imp_already_digging_at_excluding(struct Thing *excltng, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool gold_pile_with_maximum_at_xy(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Thing *smallest_gold_pile_at_xy(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool update_speed_of_player_creatures_of_model(PlayerNumber plyr_idx, ThingModel crmodel);

TbBool update_thing(struct Thing *thing);
TbBigChecksum get_thing_checksum(const struct Thing *thing);
short update_thing_sound(struct Thing *thing);

TbBool thing_slappable(const struct Thing *thing, long plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
