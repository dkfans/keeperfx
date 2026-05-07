/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_rules.h
 *     Header file for config_rules.c.
 * @par Purpose:
 *     Various game configuration options support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 31 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGRULES_H
#define DK_CFGRULES_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#define MAX_SACRIFICE_VICTIMS 6
#define MAX_SACRIFICE_RECIPES 100

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
enum SacrificeAction {
    SacA_None = 0,
    SacA_MkCreature,
    SacA_MkGoodHero,
    SacA_NegSpellAll,
    SacA_PosSpellAll,
    SacA_NegUniqFunc,
    SacA_PosUniqFunc,
    SacA_CustomReward,
    SacA_CustomPunish,
};

enum UniqueFunctions {
    UnqF_None = 0,
    UnqF_MkAllAngry,
    UnqF_MkAllVerAngry,
    UnqF_ComplResrch,
    UnqF_ComplManufc,
    UnqF_KillChickns,
    UnqF_CheaperImp,
    UnqF_CostlierImp,
    UnqF_MkAllHappy,
};

enum SacrificeReturn {
    SacR_AngryWarn    = -1,
    SacR_DontCare     =  0,
    SacR_Pleased      =  1,
    SacR_Awarded      =  2,
    SacR_Punished     =  3,
};

struct SacrificeRecipe {
    ThingModel victims[MAX_SACRIFICE_VICTIMS];
    int32_t action;
    int32_t param;
};

struct GameRulesConfig {
    GoldAmount pot_of_gold_holds;
    GoldAmount chest_gold_hold;
    GoldAmount gold_pile_value;
    GoldAmount gold_pile_maximum;
    GoldAmount gold_per_hoard;
    GoldAmount bag_gold_hold;
    unsigned short food_life_out_of_hatchery;
    HitPoints boulder_reduce_health_wall;
    HitPoints boulder_reduce_health_slap;
    HitPoints boulder_reduce_health_room;
    GameTurnDelta pay_day_gap;
    uint32_t dungeon_heart_heal_time;
    HitPoints dungeon_heart_heal_health;
    uint32_t hero_door_wait_time;
    uint32_t classic_bugs_flags;
    int32_t door_sale_percent;
    int32_t room_sale_percent;
    int32_t trap_sale_percent;
    uint32_t pay_day_speed;
    TbBool allies_share_vision;
    TbBool allies_share_drop;
    TbBool allies_share_cta;
    TbBool display_portal_limit;
    unsigned char max_things_in_hand;
    unsigned short torture_payday;
    short torture_training_cost;
    short torture_scavenging_cost;
    uint32_t easter_egg_speech_chance;
    uint32_t easter_egg_speech_interval;
    int32_t global_ambient_light;
    int32_t thing_minimum_illumination;
    TbBool light_enabled;
    unsigned short creatures_count;
};

struct ComputerRulesConfig {
    unsigned short disease_to_temple_pct;
};

struct CreatureRulesConfig {
    unsigned char recovery_frequency;
    unsigned short body_remains_for;
    uint32_t flee_zone_radius;
    GameTurnDelta game_turns_in_flee;
    unsigned short game_turns_unconscious;
    HitPoints critical_health_permil;
    unsigned char stun_enemy_chance_evil;
    unsigned char stun_enemy_chance_good;
    unsigned char stun_without_prison_chance;
};

struct MagicRulesConfig {
    GameTurnDelta hold_audience_time;
    GameTurnDelta armageddon_teleport_your_time_gap;
    GameTurnDelta armageddon_teleport_enemy_time_gap;
    GameTurnDelta armageddon_count_down;
    GameTurnDelta armageddon_duration;
    unsigned char disease_transfer_percentage;
    unsigned char disease_lose_percentage_health;
    unsigned char disease_lose_health_time;
    MapSubtlDelta min_distance_for_teleport;
    int32_t collapse_dungeon_damage;
    GameTurnDelta turns_per_collapse_dngn_dmg;
    int32_t friendly_fight_area_damage_percent;
    int32_t friendly_fight_area_range_percent;
    TbBool armageddon_teleport_neutrals;
    short weight_calculate_push;
    TbBool allow_instant_charge_up;
};

struct RoomRulesConfig {
    GameTurnDelta scavenge_cost_frequency;
    uint32_t temple_scavenge_protection_turns;
    GameTurnDelta train_cost_frequency;
    unsigned char ghost_convert_chance;
    unsigned short default_generate_speed;
    uint32_t default_max_crtrs_gen_entrance;
    GameTurnDelta food_generation_speed;
    unsigned char prison_skeleton_chance;
    unsigned char bodies_for_vampire;
    unsigned short graveyard_convert_time;
    short barrack_max_party_size;
    CrtrExpLevel training_room_max_level;
    TbBool scavenge_good_allowed;
    TbBool scavenge_neutral_allowed;
    uint32_t time_between_prison_break;
    unsigned char prison_break_chance;
    unsigned char torture_death_chance;
    unsigned char torture_convert_chance;
    uint32_t time_in_prison_without_break;
    unsigned short train_efficiency;
    unsigned short work_efficiency;
    unsigned short scavenge_efficiency;
    unsigned short research_efficiency;
};
struct WorkersRulesConfig {
    unsigned char hits_per_slab;
    uint32_t default_imp_dig_damage;
    uint32_t default_imp_dig_own_damage;
    int32_t digger_work_experience;
    unsigned short drag_to_lair;
};

struct HealthRulesConfig {
    HitPoints hunger_health_loss;
    unsigned short turns_per_hunger_health_loss;
    HitPoints food_health_gain;
    HitPoints torture_health_loss;
    unsigned short turns_per_torture_health_loss;
};

struct SacrificesRulesConfig {
    struct SacrificeRecipe sacrifice_recipes[MAX_SACRIFICE_RECIPES];
    /** The creature model used for determining amount of sacrifices which decrease digger cost. */
    ThingModel cheaper_diggers_sacrifice_model;
};
struct RulesConfig {
    struct GameRulesConfig game;
    struct ComputerRulesConfig computer;
    struct CreatureRulesConfig creature;
    struct MagicRulesConfig magic;
    struct RoomRulesConfig rooms;
    struct WorkersRulesConfig workers;
    struct HealthRulesConfig health;
    struct SacrificesRulesConfig sacrifices;
};
/******************************************************************************/
extern const struct ConfigFileData keeper_rules_file_data;
extern const struct NamedCommand research_desc[];
extern const struct NamedCommand rules_game_classicbugs_commands[];
/******************************************************************************/
long get_research_id(long item_type, const char *trg_name, const char *func_name);
struct SacrificeRecipe *get_unused_sacrifice_recipe_slot(void);

const char *player_code_name(PlayerNumber plyr_idx);
int sac_compare_fn(const void* ptr_a, const void* ptr_b);

extern const struct NamedCommand rules_sacrifices_commands[];
extern const struct NamedCommand sacrifice_unique_desc[];

extern const struct NamedField* ruleblocks[8];

extern const struct NamedFieldSet rules_named_fields_set;

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
