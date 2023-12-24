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
#define MAX_SACRIFICE_RECIPES 60

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
    UnqF_ComplResrch,
    UnqF_ComplManufc,
    UnqF_KillChickns,
    UnqF_CheaperImp,
    UnqF_CostlierImp,
};

enum SacrificeReturn {
    SacR_AngryWarn    = -1,
    SacR_DontCare     =  0,
    SacR_Pleased      =  1,
    SacR_Awarded      =  2,
    SacR_Punished     =  3,
};

struct SacrificeRecipe {
    long victims[MAX_SACRIFICE_VICTIMS];
    long action;
    long param;
};

struct GameRulesConfig {
    GoldAmount gold_per_gold_block;
    GoldAmount pot_of_gold_holds;
    GoldAmount chest_gold_hold;
    GoldAmount gold_pile_value;
    GoldAmount gold_pile_maximum;
    GoldAmount gold_per_hoard;
    GoldAmount bag_gold_hold;
    unsigned short food_life_out_of_hatchery;
    long boulder_reduce_health_wall;
    long boulder_reduce_health_slap;
    long boulder_reduce_health_room;
    GameTurnDelta pay_day_gap;
    unsigned long dungeon_heart_heal_time;
    long dungeon_heart_heal_health;
    unsigned long hero_door_wait_time;
    unsigned long classic_bugs_flags;
    unsigned long gem_effectiveness;
    long door_sale_percent;
    long room_sale_percent;
    long trap_sale_percent;
    unsigned long pay_day_speed;
    TbBool place_traps_on_subtiles;
};

struct ComputerRulesConfig {
    unsigned short disease_to_temple_pct;
};

struct CreatureRulesConfig {
    unsigned char recovery_frequency;
    unsigned short fight_max_hate;
    unsigned short fight_borderline;
    unsigned short fight_max_love;
    unsigned short body_remains_for;
    unsigned short fight_hate_kill_value;
    unsigned long flee_zone_radius;
    GameTurnDelta game_turns_in_flee;
    unsigned short game_turns_unconscious;
    long critical_health_permil;
    unsigned char stun_enemy_chance_evil;
    unsigned char stun_enemy_chance_good;
};

struct MagicRulesConfig {
    GameTurnDelta hold_audience_time;
    unsigned long armagedon_teleport_your_time_gap;
    unsigned long armagedon_teleport_enemy_time_gap;
    unsigned char disease_transfer_percentage;
    unsigned char disease_lose_percentage_health;
    unsigned char disease_lose_health_time;
    MapSubtlDelta min_distance_for_teleport;
    long collapse_dungeon_damage;
    GameTurnDelta turns_per_collapse_dngn_dmg;
    GoldAmount power_hand_gold_grab_amount;
};

struct RoomRulesConfig {
    GameTurnDelta scavenge_cost_frequency;
    unsigned long temple_scavenge_protection_turns;
    GameTurnDelta train_cost_frequency;
    unsigned char ghost_convert_chance;
    unsigned short default_generate_speed;
    unsigned long default_max_crtrs_gen_entrance;
    GameTurnDelta food_generation_speed;
    unsigned char prison_skeleton_chance;
    unsigned char bodies_for_vampire;
    unsigned short graveyard_convert_time;
    short barrack_max_party_size;
    unsigned short training_room_max_level;
    TbBool scavenge_good_allowed;
    TbBool scavenge_neutral_allowed;
    unsigned long time_between_prison_break;
};
struct WorkersRulesConfig {
    unsigned char hits_per_slab;
    unsigned long default_imp_dig_damage;
    unsigned long default_imp_dig_own_damage;
};

/*
struct HealthRulesConfig {
};

struct ResearchRulesConfig {
};

struct SacrificesRulesConfig {
};

*/
struct RulesConfig {
    struct GameRulesConfig game;
    struct ComputerRulesConfig computer;
    struct CreatureRulesConfig creature;
    struct MagicRulesConfig magic;
    struct RoomRulesConfig rooms;
    struct WorkersRulesConfig workers;
    /*
    struct HealthRulesConfig health;
    struct ResearchRulesConfig research;
    struct SacrificesRulesConfig sacrifices;
    */

};
/******************************************************************************/
extern const char keeper_rules_file[];
extern const struct NamedCommand research_desc[];
extern const struct NamedCommand rules_game_classicbugs_commands[];
/******************************************************************************/
long get_research_id(long item_type, const char *trg_name, const char *func_name);
TbBool load_rules_config(const char *conf_fname, unsigned short flags);
struct SacrificeRecipe *get_unused_sacrifice_recipe_slot(void);

const char *player_code_name(PlayerNumber plyr_idx);

extern const struct NamedCommand rules_sacrifices_commands[];
extern const struct NamedCommand sacrifice_unique_desc[];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
