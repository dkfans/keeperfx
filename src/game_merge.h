/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_merge.h
 *     Header file for game_merge.c.
 * @par Purpose:
 *     Handles game state serialization, campaign progression, and random number generation systems.
 * @par Comment:
 *     Defines data structures for persistent campaign data, random number generation macros,
 *     and various game system flags used throughout the engine.
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
#include "bflib_math.h"
#include "globals.h"

#include "actionpt.h"
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
#define QUICK_MESSAGES_COUNT        256
#define BONUS_LEVEL_STORAGE_COUNT     6
#define PLAYERS_FOR_CAMPAIGN_FLAGS    5
#define CAMPAIGN_FLAGS_PER_PLAYER     8
#define TRANSFER_CREATURE_STORAGE_COUNT     255

#define AROUND_MAP_LENGTH 9
#define AROUND_SLAB_LENGTH 9
#define AROUND_SLAB_EIGHT_LENGTH 8
#define SMALL_AROUND_SLAB_LENGTH 4

// Random number generation system with synchronized and unsynchronized variants

// Thing-specific random for deterministic per-thing behavior (creatures, objects, etc.)
#define THING_RANDOM(thing, range) LbRandomSeries(range, &((struct Thing*)(thing))->random_seed, __func__, __LINE__)
// Global game events requiring synchronization across network (scripts, AI decisions, player events)
#define GAME_RANDOM(range) LbRandomSeries(range, &game.action_random_seed, __func__, __LINE__)
// Unsynchronized random for visual/audio effects that don't affect game state (lighting, particles)
#define UNSYNC_RANDOM(range) LbRandomSeries(range, &game.unsync_random_seed, __func__, __LINE__)
// Works like UNSYNC_RANDOM - sound-specific random for audio effects and sound variations
#define SOUND_RANDOM(range) LbRandomSeries(range, &game.sound_random_seed, __func__, __LINE__)
// AI-specific random seed for computer player decisions
#define AI_RANDOM(range) LbRandomSeries(range, &game.ai_random_seed, __func__, __LINE__)
// Player-specific random for actions tied to a particular player
#define PLAYER_RANDOM(plyr, range) LbRandomSeries(range, &game.player_random_seed, __func__, __LINE__)

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
    ClscBug_None                          = 0x0000,
    ClscBug_ResurrectForever              = 0x0001,
    ClscBug_Overflow8bitVal               = 0x0002,
    ClscBug_ClaimRoomAllThings            = 0x0004,
    ClscBug_ResurrectRemoved              = 0x0008,
    ClscBug_NoHandPurgeOnDefeat           = 0x0010,
    ClscBug_MustObeyKeepsNotDoJobs        = 0x0020,
    ClscBug_BreakNeutralWalls             = 0x0040,
    ClscBug_AlwaysTunnelToRed             = 0x0080,
    ClscBug_FullyHappyWithGold            = 0x0100,
    ClscBug_FaintedImmuneToBoulder        = 0x0200,
    ClscBug_RebirthKeepsSpells            = 0x0400,
    ClscBug_FriendlyFaint                 = 0x0800,
    ClscBug_PassiveNeutrals               = 0x1000,
    ClscBug_NeutralTortureConverts        = 0x2000,
    ClscBug_ListEnd                       = 0x4000,
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
    struct CreatureStorage transferred_creatures[PLAYERS_COUNT][TRANSFER_CREATURE_STORAGE_COUNT];
    long campaign_flags[PLAYERS_FOR_CAMPAIGN_FLAGS][CAMPAIGN_FLAGS_PER_PLAYER];
    char next_level;
};


extern unsigned long game_flags2; // Should be reset to zero on new level

#pragma pack()

/******************************************************************************/
extern struct IntralevelData intralvl;
/******************************************************************************/
LevelNumber get_loaded_level_number(void);
LevelNumber set_loaded_level_number(LevelNumber lvnum);
LevelNumber get_continue_level_number(void);
LevelNumber set_continue_level_number(LevelNumber lvnum);
LevelNumber get_selected_level_number(void);
LevelNumber set_selected_level_number(LevelNumber lvnum);
TbBool activate_bonus_level(struct PlayerInfo *player);
TbBool is_bonus_level_visible(struct PlayerInfo *player, LevelNumber bn_lvnum);
void hide_all_bonus_levels(struct PlayerInfo *player);
unsigned short get_extra_level_kind_visibility(unsigned short elv_kind);
void update_extra_levels_visibility(void);
TbBool set_bonus_level_visibility_for_singleplayer_level(struct PlayerInfo *player, unsigned long sp_lvnum, short visible);
TbBool set_bonus_level_visibility(LevelNumber bn_lvnum, TbBool visible);
TbBool emulate_integer_overflow(unsigned short nbits);
/******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif
