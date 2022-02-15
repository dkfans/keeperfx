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
    TngList_StaticLights = 11,
    TngList_DynamLights  = 12,
};

enum ThingUpdateFuncReturns {
    TUFRet_Deleted       = -1, /**< Returned if the thing being updated no longer exists. */
    TUFRet_Unchanged     =  0, /**< Returned if no change was made to the thing data. */
    TUFRet_Modified      =  1, /**< Returned if the thing was updated and possibly some variables have changed inside. */
};

enum CreatureSelectCriteria {
    CSelCrit_Any               =  0,
    CSelCrit_MostExperienced   =  1,
    CSelCrit_MostExpWandering  =  2,
    CSelCrit_MostExpWorking    =  3,
    CSelCrit_MostExpFighting   =  4,
    CSelCrit_LeastExperienced  =  5,
    CSelCrit_LeastExpWandering =  6,
    CSelCrit_LeastExpWorking   =  7,
    CSelCrit_LeastExpFighting  =  8,
    CSelCrit_NearOwnHeart      =  9,
    CSelCrit_NearEnemyHeart    = 10,
    CSelCrit_OnEnemyGround     = 11,
    CSelCrit_OnFriendlyGround  = 12,
    CSelCrit_OnNeutralGround   = 13,
    CSelCrit_NearAP            = 14,
};

//TODO replace HitType with these
/**
 * Flags which determine which target things can be hit by something.
 */
enum HitTargetFlagsList {
    HitTF_None                = 0x00000000,//!< Zero flag
    HitTF_EnemyCreatures      = 0x00000001,//!< Allow targeting enemy creatures.
    HitTF_AlliedCreatures     = 0x00000002,//!< Allow targeting allied and neutral creatures.
    HitTF_OwnedCreatures      = 0x00000004,//!< Allow targeting owned creatures.
    HitTF_ArmourAffctdCreatrs = 0x00000008,//!< Allow targeting creatures affected by Armour spell.
    HitTF_PreventDmgCreatrs   = 0x00000010,//!< Allow targeting creatures with damage prevention flag, ie unconscious.
    HitTF_EnemyShotsCollide   = 0x00000400,//!< Allow colliding with enemy shots which can be collided with.
    HitTF_AlliedShotsCollide  = 0x00000800,//!< Allow colliding with allied and neutral shots which can be collided with.
    HitTF_OwnedShotsCollide   = 0x00001000,//!< Allow colliding with own shots which can be collided with.
    HitTF_EnemySoulContainer  = 0x00002000,//!< Allow targeting enemy soul containers.
    HitTF_AlliedSoulContainer = 0x00004000,//!< Allow targeting allied and neutral containers.
    HitTF_OwnedSoulContainer  = 0x00008000,//!< Allow targeting own soul container.
    HitTF_AnyWorkshopBoxes    = 0x00040000,//!< Allow targeting Workshop boxes owned by anyone.
    HitTF_AnySpellbooks       = 0x00080000,//!< Allow targeting spellbook objects owned by anyone.
    HitTF_AnyDnSpecialBoxes   = 0x00100000,//!< Allow targeting Dnungeon Special boxes owned by anyone.
    HitTF_AnyGoldHoards       = 0x00200000,//!< Allow targeting Gold Hoards owned by anyone.
    HitTF_AnyFoodObjects      = 0x00400000,//!< Allow targeting Food Objects owned by anyone.
    HitTF_AnyGoldPiles        = 0x00800000,//!< Allow targeting gold laying on ground before storing in treasury, pots and piles.
    HitTF_AnyDecorations      = 0x01000000,//!< Allow targeting things which are not used for anything, ie potions and barrels.
    HitTF_EnemyDeployedDoors  = 0x02000000,//!< Allow targeting Enemy Deployed Doors.
    HitTF_AlliedDeployedDoors = 0x04000000,//!< Allow targeting Allied and neutral Deployed Doors.
    HitTF_OwnedDeployedDoors  = 0x08000000,//!< Allow targeting Owned Deployed Doors.
    HitTF_EnemyDeployedTraps  = 0x10000000,//!< Allow targeting enemy Deployed Traps.
    HitTF_AlliedDeployedTraps = 0x20000000,//!< Allow targeting allied and neutral deployed Traps.
    HitTF_OwnedDeployedTraps  = 0x40000000,//!< Allow targeting Owned Deployed Traps.
    HitTF_CreatureDeadBodies  = 0x80000000,//!< Allow targeting Creature Dead Bodies.
};

/******************************************************************************/
#pragma pack(1)

struct PlayerInfo;
struct Thing;
struct CompoundTngFilterParam;
struct Dungeon;
struct Map;

typedef struct CompoundTngFilterParam * MaxTngFilterParam;
typedef struct CompoundTngFilterParam * ModTngFilterParam;

/** Definition of a callback type used for updating thing which is in specific state. */
typedef long (*Thing_State_Func)(struct Thing *);
/** Definition of a callback type used for updating thing of specific class or model. */
typedef TngUpdateRet (*Thing_Class_Func)(struct Thing *);
/** Definition of a callback type which can modify the thing and receives additional parameters. */
typedef TngUpdateRet (*Thing_Modifier_Func)(struct Thing *, ModTngFilterParam);
/** Basic thing filtering type. */
typedef long (*Thing_Filter)(const struct Thing *, FilterParam);
/** Definition of a simple callback type which can only return true/false and has no memory of previous checks. */
typedef TbBool (*Thing_Bool_Filter)(const struct Thing *);
/** Definition of a callback type used for selecting best match through all the things by maximizing a value. */
typedef long (*Thing_Maximizer_Filter)(const struct Thing *, MaxTngFilterParam, long);
typedef long (*Thing_Collide_Func)(const struct Thing *, const struct Thing *, long, long);
/** Definition of a simple callback type which can only return true/false and can modify the thing. */
typedef TbBool (*Thing_Bool_Modifier)(struct Thing *);

struct CompoundTngFilterParam {
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
ThingIndex get_thing_class_list_head(ThingClass class_id);
struct StructureList *get_list_for_thing_class(ThingClass class_id);

long creature_near_filter_is_enemy_of_and_not_specdigger(const struct Thing *thing, FilterParam val);
long creature_near_filter_is_owned_by(const struct Thing *thing, FilterParam val);

// Filters to select creature anywhere on map but belonging to given player
struct Thing *get_player_list_creature_with_filter(ThingIndex thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param);
struct Thing *get_player_list_random_creature_with_filter(ThingIndex thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param, PlayerNumber plyr_idx);
long count_player_list_creatures_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param);
long count_player_list_creatures_of_model_matching_bool_filter(PlayerNumber plyr_idx, int tngmodel, Thing_Bool_Filter matcher_cb);
// Final routines to select creature anywhere on map but belonging to given player
struct Thing *get_player_list_nth_creature_of_model(long thing_idx, ThingModel crmodel, long crtr_idx);
struct Thing *get_player_list_nth_creature_of_model_on_territory(long thing_idx, ThingModel crmodel, long crtr_idx, int friendly);
struct Thing *get_random_players_creature_of_model(PlayerNumber plyr_idx, ThingModel crmodel);
struct Thing *get_random_players_creature_of_model_on_territory(PlayerNumber plyr_idx, ThingModel crmodel,int friendly);
struct Thing *find_players_highest_level_creature_of_breed_and_gui_job(long crmodel, long job_idx, PlayerNumber plyr_idx, unsigned char pick_check);
struct Thing *find_players_lowest_level_creature_of_breed_and_gui_job(long crmodel, long job_idx, PlayerNumber plyr_idx, unsigned char pick_check);
long do_to_players_all_creatures_of_model(PlayerNumber plyr_idx, int crmodel, Thing_Bool_Modifier do_cb);
TbBool heal_completely_all_players_creatures(PlayerNumber plyr_idx, ThingModel crmodel);
void setup_all_player_creatures_and_diggers_leave_or_die(PlayerNumber plyr_idx);
TbBool reset_all_players_creatures_affected_by_cta(PlayerNumber plyr_idx);
long count_player_creatures_not_counting_to_total(PlayerNumber plyr_idx);
long count_player_diggers_not_counting_to_total(PlayerNumber plyr_idx);

// Filters to select thing on/near given map position
struct Thing *get_thing_on_map_block_with_filter(long thing_idx, Thing_Maximizer_Filter filter, MaxTngFilterParam param, long *maximizer);
struct Thing *get_thing_near_revealed_map_block_with_filter(MapCoord x, MapCoord y, Thing_Maximizer_Filter filter, MaxTngFilterParam param);
struct Thing *get_thing_spiral_near_map_block_with_filter(MapCoord x, MapCoord y, long spiral_len, Thing_Maximizer_Filter filter, MaxTngFilterParam param);
long count_things_spiral_near_map_block_with_filter(MapCoord x, MapCoord y, long spiral_len, Thing_Maximizer_Filter filter, MaxTngFilterParam param);
long do_to_things_on_map_block(long thing_idx, Thing_Bool_Modifier do_cb);
long do_to_things_with_param_on_map_block(ThingIndex thing_idx, Thing_Modifier_Func do_cb, ModTngFilterParam param);
long do_to_things_spiral_near_map_block(MapCoord x, MapCoord y, long spiral_len, Thing_Bool_Modifier do_cb);
long do_to_things_with_param_spiral_near_map_block(const struct Coord3d *center_pos, MapCoordDelta max_dist, Thing_Modifier_Func do_cb, ModTngFilterParam param);
long do_to_things_with_param_around_map_block(const struct Coord3d *center_pos, Thing_Modifier_Func do_cb, ModTngFilterParam param);
// Final routines to select thing on/near given map position
struct Thing *get_creature_near_but_not_specdigger(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx);
struct Thing *get_creature_near_who_is_enemy_of_and_not_specdigger(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx);
struct Thing *get_creature_near_to_be_keeper_power_target(MapCoord pos_x, MapCoord pos_y, PowerKind pwmodel, PlayerNumber plyr_idx);
struct Thing *get_nearest_thing_for_slap(PlayerNumber plyr_idx, MapCoord pos_x, MapCoord pos_y);
struct Thing *get_creature_near_and_owned_by(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx, long crmodel);
struct Thing *get_creature_in_range_and_owned_by_or_allied_with(MapCoord pos_x, MapCoord pos_y, MapSubtlDelta distance_stl, PlayerNumber plyr_idx);
struct Thing *get_creature_in_range_of_model_owned_and_controlled_by(MapCoord pos_x, MapCoord pos_y, MapSubtlDelta distance_stl, long crmodel, PlayerNumber plyr_idx);
struct Thing *get_creature_in_range_who_is_enemy_of_able_to_attack_and_not_specdigger(MapCoord pos_x, MapCoord pos_y, long distance_stl, PlayerNumber plyr_idx);
struct Thing *get_creature_of_model_training_at_subtile_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long model_id, PlayerNumber plyr_idx, long skip_thing_id);
struct Thing *get_object_at_subtile_of_model_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long tngmodel, PlayerNumber plyr_idx);
struct Thing *get_cavein_at_subtile_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx);
struct Thing *get_object_around_owned_by_and_matching_bool_filter(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx, Thing_Bool_Filter matcher_cb);
struct Thing *get_food_at_subtile_available_to_eat_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long plyr_idx);
struct Thing *get_trap_at_subtile_of_model_and_owned_by(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long model, long plyr_idx);
struct Thing *get_trap_around_of_model_and_owned_by(MapCoord pos_x, MapCoord pos_y, long model, PlayerNumber plyr_idx);
struct Thing *get_door_for_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Thing *get_door_for_position_for_trap_placement(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool slab_has_door_thing_on(MapSlabCoord slb_x, MapSlabCoord slb_y);
struct Thing *get_nearest_object_at_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Thing *get_nearest_thing_at_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void remove_dead_creatures_from_slab(MapSlabCoord slb_x, MapSlabCoord slb_y);
long count_creatures_near_and_owned_by_or_allied_with(MapCoord pos_x, MapCoord pos_y, long distance_stl, PlayerNumber plyr_idx);
long switch_owned_objects_on_destoyed_slab_to_neutral(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber prev_owner);

// Filters to select thing anywhere on map but only of one given class
struct Thing *get_random_thing_of_class_with_filter(Thing_Maximizer_Filter filter, MaxTngFilterParam param, PlayerNumber plyr_idx);
struct Thing *get_nth_thing_of_class_with_filter(Thing_Maximizer_Filter filter, MaxTngFilterParam param, long tngindex);
long count_things_of_class_with_filter(Thing_Maximizer_Filter filter, MaxTngFilterParam param);
long do_to_all_things_of_class_and_model(int tngclass, int tngmodel, Thing_Bool_Modifier do_cb);
// Final routines to select thing anywhere on map but only of one given class
struct Thing *get_nearest_object_owned_by_and_matching_bool_filter(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx, Thing_Bool_Filter matcher_cb);
struct Thing *get_nearest_thing_of_class_and_model_owned_by(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx, int tngclass, int tngmodel);
struct Thing *get_random_trap_of_model_owned_by_and_armed(ThingModel tngmodel, PlayerNumber plyr_idx, TbBool armed);
struct Thing *get_random_door_of_model_owned_by_and_locked(ThingModel tngmodel, PlayerNumber plyr_idx, TbBool locked);
struct Thing *find_gold_laying_in_dungeon(const struct Dungeon *dungeon);
struct Thing *get_nearest_enemy_creature_possible_to_attack_by(struct Thing *creatng);
#define find_nearest_enemy_creature(creatng) get_nearest_enemy_creature_possible_to_attack_by(creatng);
struct Thing *get_highest_score_enemy_creature_within_distance_possible_to_attack_by(struct Thing *creatng, MapCoordDelta dist);
struct Thing *get_nth_creature_owned_by_and_matching_bool_filter(PlayerNumber plyr_idx, Thing_Bool_Filter matcher_cb, long n);
struct Thing *get_nth_creature_owned_by_and_failing_bool_filter(PlayerNumber plyr_idx, Thing_Bool_Filter matcher_cb, long n);

// Routines to select all players creatures of model matching the criteria
long count_creatures_in_dungeon_of_model_flags(const struct Dungeon *dungeon, unsigned long need_mdflags, unsigned long excl_mdflags);
long count_creatures_in_dungeon_controlled_and_of_model_flags(const struct Dungeon *dungeon, unsigned long need_mdflags, unsigned long excl_mdflags);

unsigned long update_things_sounds_in_list(struct StructureList *list);
void stop_all_things_playing_samples(void);
unsigned long update_cave_in_things(void);
unsigned long update_creatures_not_in_list(void);
unsigned long update_things_in_list(struct StructureList *list);
void init_player_start(struct PlayerInfo *player, TbBool keep_prev);
void setup_computer_players(void);
void setup_zombie_players(void);
void init_all_creature_states(void);

TbBool perform_action_on_all_creatures_in_group(struct Thing *thing, Thing_Bool_Modifier action);

struct Thing *creature_of_model_in_prison_or_tortured(ThingModel crmodel);
long count_player_creatures_of_model(PlayerNumber plyr_idx, int crmodel);
long count_player_list_creatures_of_model(long thing_idx, ThingModel crmodel);
long count_player_list_creatures_of_model_on_territory(long thing_idx, ThingModel crmodel, int friendly);
GoldAmount compute_player_payday_total(const struct Dungeon *dungeon);
TbBool lord_of_the_land_in_prison_or_tortured(void);
struct Thing *lord_of_the_land_find(void);
long electricity_affecting_area(const struct Coord3d *pos, PlayerNumber immune_plyr_idx, long range, long max_damage);

void update_things(void);

struct Thing *find_base_thing_on_mapwho(ThingClass oclass, ThingModel okind, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void remove_thing_from_mapwho(struct Thing *thing);
void place_thing_in_mapwho(struct Thing *thing);

struct Thing *find_hero_gate_of_number(long num);
long get_free_hero_gate_number(void);

struct Thing *find_creature_lair_totem_at_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, ThingModel crmodel);

TbBool thing_is_shootable(const struct Thing *thing, PlayerNumber shot_owner, HitTargetFlags hit_targets);
unsigned long hit_type_to_hit_targets(long hit_type);
long collide_filter_thing_is_of_type(const struct Thing *thing, const struct Thing *sectng, long a3, long a4);

TbBool imp_already_digging_at_excluding(struct Thing *excltng, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool gold_pile_with_maximum_at_xy(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Thing *smallest_gold_pile_at_xy(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool update_speed_of_player_creatures_of_model(PlayerNumber plyr_idx, int crmodel);
TbBool apply_anger_to_all_players_creatures_excluding(PlayerNumber plyr_idx, long anger,
    long reason, const struct Thing *excltng);

void break_mapwho_infinite_chain(const struct Map *mapblk);

TbBool update_thing(struct Thing *thing);
TbBigChecksum get_thing_checksum(const struct Thing *thing);
short update_thing_sound(struct Thing *thing);
struct Thing* find_players_dungeon_heart(PlayerNumber plyridx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
