/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_merge.h
 *     Header file for game_merge.c.
 * @par Purpose:
 *     Saved games maintain functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     21 Oct 2009 - 25 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_GAMEMERGE_H
#define DK_GAMEMERGE_H

#include "bflib_basics.h"
#include "globals.h"

#include "config_creature.h"
#include "config_crtrmodel.h"
#include "config_objects.h"
#include "config_rules.h"
#include "creature_control.h"
#include "dungeon_data.h"
#include "gui_msgs.h"
#include "thing_creature.h"
#include "thing_objects.h"
#include "light_data.h"
#include "lvl_script.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define MESSAGE_TEXT_LEN           1024
#define QUICK_MESSAGES_COUNT         50
#define BONUS_LEVEL_STORAGE_COUNT     6
#define PLAYERS_FOR_CAMPAIGN_FLAGS    5
#define CAMPAIGN_FLAGS_PER_PLAYER     8

// UNSYNC_RANDOM is not synced at all. For synced choices the more specific random is better.
// So priority is  CREATURE_RANDOM >> PLAYER_RANDOM >> GAME_RANDOM

// Deprecated. Used only once. Maybe it is sound-specific UNSYNC_RANDOM
#define SOUND_RANDOM(range) LbRandomSeries(range, &sound_seed, __func__, __LINE__, "sound")
// This RNG should not be used to affect anything related affecting game state
#define UNSYNC_RANDOM(range) LbRandomSeries(range, &game.unsync_rand_seed, __func__, __LINE__, "unsync")
// This RNG should be used only for "whole game" events (i.e. from script)
#define GAME_RANDOM(range) LbRandomSeries(range, &game.action_rand_seed, __func__, __LINE__, "game")
// This RNG is for anything related to creatures or their shots. So creatures should act independent
#define CREATURE_RANDOM(thing, range) \
    LbRandomSeries(range, &game.action_rand_seed, __func__, __LINE__, "creature")
// This is messy. Used only for AI choices. Maybe it should be merged with PLAYER_RANDOM.
#define AI_RANDOM(range) LbRandomSeries(range, &game.action_rand_seed, __func__, __LINE__, "ai")
// This RNG is about something related to specific player
#define PLAYER_RANDOM(plyr, range) LbRandomSeries(range, &game.action_rand_seed, __func__, __LINE__, "player")
// RNG related to effects. I am unsure about its relationship with game state.
// It should be replaced either with CREATURE_RANDOM or with UNSYNC_RANDOM on case by case basis.
#define EFFECT_RANDOM(thing, range) \
    LbRandomSeries(range, &game.action_rand_seed, __func__, __LINE__, "effect")

enum GameSystemFlags {
    GSF_NetworkActive    = 0x0001,
    GSF_NetGameNoSync    = 0x0002,
    GSF_NetSeedNoSync    = 0x0004,
    GSF_CaptureMovie     = 0x0008,
    GSF_CaptureSShot     = 0x0010,
    GSF_AllowOnePlayer   = 0x0040,
    GSF_RunAfterVictory  = 0x0080,
};

enum GameGUIFlags {
    GGUI_1Player         = 0x0001,
    GGUI_CountdownTimer  = 0x0002,
    GGUI_ScriptTimer     = 0x0004,
    GGUI_Variable        = 0x0008,
    GGUI_SoloChatEnabled = 0x0080
};

enum ClassicBugFlags {
    ClscBug_None                   = 0x0000,
    ClscBug_ResurrectForever       = 0x0001,
    ClscBug_Overflow8bitVal        = 0x0002,
    ClscBug_ClaimRoomAllThings     = 0x0004,
    ClscBug_ResurrectRemoved       = 0x0008,
    ClscBug_NoHandPurgeOnDefeat    = 0x0010,
    ClscBug_MustObeyKeepsNotDoJobs = 0x0020,
    ClscBug_BreakNeutralWalls      = 0x0040,
    ClscBug_AlwaysTunnelToRed      = 0x0080,
    ClscBug_FullyHappyWithGold     = 0x0100,
    ClscBug_FaintedImmuneToBoulder = 0x0200,
    ClscBug_RebirthKeepsSpells     = 0x0400,
    ClscBug_FriendlyFaint          = 0x0800,
};

enum GameFlags2 {
    GF2_ClearPauseOnSync          = 0x0001,
    GF2_ClearPauseOnPacket        = 0x0002,
    GF2_Timer                     = 0x0004,
    GF2_Server                    = 0x0008,
    GF2_Connect                   = 0x0010,
    GF2_ShowEventLog              = 0x00010000,
    GF2_PERSISTENT_FLAGS          = 0xFFFF0000
};
/******************************************************************************/
#pragma pack(1)

struct PlayerInfo;

/** Structure which stores state of scrollable message with text.
 */
struct TextScrollWindow {
    char text[MESSAGE_TEXT_LEN];
    long start_y;
    char action;
    long text_height;
    long window_height;
};

/**
 * Structure which stores data copied between levels.
 * This data is not lost between levels of a campaign.
 */
struct IntralevelData {
    unsigned char bonuses_found[BONUS_LEVEL_STORAGE_COUNT];
    struct CreatureStorage transferred_creature;
    long campaign_flags[PLAYERS_FOR_CAMPAIGN_FLAGS][CAMPAIGN_FLAGS_PER_PLAYER];
};

/**
 * Defines additional elements, which are not stored in main 'Game' struct.
 */
struct GameAdd {
    struct CreatureStats creature_stats[CREATURE_TYPES_MAX];
    struct CreatureConfig crtr_conf;
    unsigned long turn_last_checked_for_gold;
    unsigned long flee_zone_radius;
    unsigned long time_between_prison_break;
    unsigned long time_in_prison_without_break;
    unsigned char prison_break_chance;
    unsigned short game_turns_unconscious;
    unsigned char stun_enemy_chance_evil;
    unsigned char stun_enemy_chance_good;
    long critical_health_permil;
    long friendly_fight_area_damage_permil;
    long friendly_fight_area_range_permil;
    unsigned char torture_death_chance;
    unsigned char torture_convert_chance;
    unsigned short bag_gold_hold;
    TbBool scavenge_good_allowed;
    TbBool scavenge_neutral_allowed;
    long scavenge_effectiveness_evil; //unused
    long scavenge_effectiveness_good; //unused
    TbBool armegeddon_teleport_neutrals;
    unsigned long classic_bugs_flags;
    unsigned short computer_chat_flags;
    /** The creature model used for determining amount of sacrifices which decrease digger cost. */
    ThingModel cheaper_diggers_sacrifice_model;
    char quick_messages[QUICK_MESSAGES_COUNT][MESSAGE_TEXT_LEN];
    struct GuiMessage messages[GUI_MESSAGES_COUNT];
    struct SacrificeRecipe sacrifice_recipes[MAX_SACRIFICE_RECIPES];
    struct LightSystemState lightst;
    long digger_work_experience;
    unsigned long gem_effectiveness;
    long door_sale_percent;
    long room_sale_percent;
    long trap_sale_percent;
    unsigned long pay_day_speed;
    unsigned short disease_to_temple_pct;
    TbBool place_traps_on_subtiles;
    unsigned long gold_per_hoard;

#define TRAPDOOR_TYPES_MAX 128

    struct ManfctrConfig traps_config[TRAPDOOR_TYPES_MAX];
    struct ManfctrConfig doors_config[TRAPDOOR_TYPES_MAX];
    struct TrapStats trap_stats[TRAPDOOR_TYPES_MAX];
    struct TrapDoorConfig trapdoor_conf;

    uint8_t               max_custom_box_kind;
    unsigned long         current_player_turn; // Actually it is a hack. We need to rewrite scripting for current player
    int                   script_current_player;
    struct Coord3d        triggered_object_location; //Position of `TRIGGERED_OBJECT`

    char                  box_tooltip[CUSTOM_BOX_COUNT][MESSAGE_TEXT_LEN];
    struct ScriptFxLine   fx_lines[FX_LINES_COUNT];
    int                   active_fx_lines;

    struct DungeonAdd dungeon[DUNGEONS_COUNT];

    struct ThingAdd things[THINGS_COUNT];

    struct Objects thing_objects_data[OBJECT_TYPES_COUNT];
    struct ObjectsConfig object_conf;

    LevelNumber last_level; // Used to restore custom sprites
    struct LevelScript script;
    PlayerNumber script_player;
    unsigned char script_timer_id;
    unsigned long script_timer_limit;
    TbBool timer_real;
    unsigned char script_value_type;
    unsigned char script_value_id;
    long script_variable_target;
    unsigned char script_variable_target_type;
    TbBool heart_lost_display_message;
    TbBool heart_lost_quick_message;
    unsigned long heart_lost_message_id;
    long heart_lost_message_target;
    unsigned char slab_ext_data[85 * 85];
};

extern unsigned long game_flags2; // Should be reset to zero on new level

#pragma pack()

/******************************************************************************/
extern struct GameAdd gameadd;
extern struct IntralevelData intralvl;
/******************************************************************************/
LevelNumber get_loaded_level_number(void);
LevelNumber set_loaded_level_number(LevelNumber lvnum);
LevelNumber get_continue_level_number(void);
LevelNumber set_continue_level_number(LevelNumber lvnum);
LevelNumber get_selected_level_number(void);
LevelNumber set_selected_level_number(LevelNumber lvnum);
TbBool activate_bonus_level(struct PlayerInfo *player);
TbBool is_bonus_level_visible(struct PlayerInfo *player, long bn_lvnum);
void hide_all_bonus_levels(struct PlayerInfo *player);
unsigned short get_extra_level_kind_visibility(unsigned short elv_kind);
short is_extra_level_visible(struct PlayerInfo *player, long ex_lvnum);
void update_extra_levels_visibility(void);
TbBool set_bonus_level_visibility_for_singleplayer_level(struct PlayerInfo *player, unsigned long sp_lvnum, short visible);
/******************************************************************************/

struct ThingAdd *get_thingadd(Thingid thing_idx);

#ifdef __cplusplus
}
#endif
#endif
