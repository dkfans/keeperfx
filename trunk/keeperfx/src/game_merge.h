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
#include "config_rules.h"
#include "thing_creature.h"
#include "creature_control.h"
#include "light_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define MESSAGE_TEXT_LEN           1024
#define QUICK_MESSAGES_COUNT         50
#define BONUS_LEVEL_STORAGE_COUNT     6

#define SOUND_RANDOM(range) LbRandomSeries(range, &sound_seed, __func__, __LINE__)
#define UNSYNC_RANDOM(range) LbRandomSeries(range, &game.unsync_rand_seed, __func__, __LINE__)
#define ACTION_RANDOM(range) LbRandomSeries(range, &game.action_rand_seed, __func__, __LINE__)

enum GameSystemFlags {
    GSF_NetworkActive    = 0x0001,
    GSF_NetGameNoSync    = 0x0002,
    GSF_NetSeedNoSync    = 0x0004,
    GSF_CaptureMovie     = 0x0008,
    GSF_CaptureSShot     = 0x0010,
    GSF_AllowOnePlayer   = 0x0040,
};

enum GameGUIFlags {
    GGUI_CountdownTimer  = 0x0002,
};

enum ClassicBugFlags {
    ClscBug_None                   = 0x0000,
    ClscBug_ResurrectForever       = 0x0001,
    ClscBug_Overflow8bitVal        = 0x0002,
    ClscBug_ClaimRoomAllThings     = 0x0004,
    ClscBug_ResurrectRemoved       = 0x0008,
    ClscBug_NoHandPurgeOnDefeat    = 0x0010,
    ClscBug_MustObeyKeepsNotDoJobs = 0x0020,
};

/******************************************************************************/
#pragma pack(1)

struct PlayerInfo;

// Scroll bar with text.
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
};

/**
 * Defines additional elements, which are not stored in main 'Game' struct.
 */
struct GameAdd {
    struct CreatureStats creature_stats[CREATURE_TYPES_MAX];
    unsigned long turn_last_checked_for_gold;
    unsigned long flee_zone_radius;
    unsigned long time_between_prison_break;
    unsigned short game_turns_unconscious;
    long critical_health_permil;
    long friendly_fight_area_damage_permil;
    long friendly_fight_area_range_permil;
    unsigned char torture_convert_chance;
    TbBool scavenge_good_allowed;
    TbBool scavenge_neutral_allowed;
    TbBool armegeddon_teleport_neutrals;
    unsigned short classic_bugs_flags;
    unsigned short computer_chat_flags;
    char quick_messages[QUICK_MESSAGES_COUNT][MESSAGE_TEXT_LEN];
    struct SacrificeRecipe sacrifice_recipes[MAX_SACRIFICE_RECIPES];
    struct LightSystemState lightst;
	struct SacrificeInfo sacrifice_info;
};

#pragma pack()
/******************************************************************************/
extern struct GameAdd gameadd;
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
#ifdef __cplusplus
}
#endif
#endif
