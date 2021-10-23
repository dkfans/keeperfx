/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_legacy.h
 *     Header file for game_legacy.c.
 * @par Purpose:
 *     Game structure maintain functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     21 Oct 2009 - 23 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_GAMELEGACY_H
#define DK_GAMELEGACY_H

#include "bflib_basics.h"
#include "globals.h"

#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_traps.h"
#include "thing_doors.h"
#include "thing_objects.h"
#include "thing_creature.h"
#include "room_data.h"
#include "slab_data.h"
#include "map_data.h"
#include "actionpt.h"
#include "creature_control.h"
#include "creature_battle.h"
#include "config_campaigns.h"
#include "config_magic.h"
#include "config_trapdoor.h"
#include "config_objects.h"
#include "config_cubes.h"
#include "map_columns.h"
#include "map_events.h"
#include "lvl_script.h"
#include "gui_msgs.h"
#include "player_computer.h"
#include "player_complookup.h"
#include "power_process.h"
#include "net_game.h"
#include "packets.h"
#include "sounds.h"
#include "game_lghtshdw.h"
#include "game_merge.h"

#define BOOKMARKS_COUNT               5
// Static textures
#define TEXTURE_BLOCKS_STAT_COUNT   544
// Animated texture frames count
#define TEXTURE_BLOCKS_ANIM_FRAMES    8
// Animated textures amount
#define TEXTURE_BLOCKS_ANIM_COUNT    48
#define TEXTURE_BLOCKS_COUNT         (TEXTURE_BLOCKS_STAT_COUNT+TEXTURE_BLOCKS_ANIM_COUNT)

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
enum GameKinds {
    GKind_Unknown0 = 0,
    GKind_Unknown1,
    GKind_LocalGame,
    GKind_Unknown3,
    GKind_Unknown4,
    GKind_MultiGame,
};

enum GameOperationFlags {
    GOF_Paused           = 0x01,
    GOF_SingleLevel      = 0x02, /**< Play single level and then exit. */
    GOF_Unkn04           = 0x04,
    GOF_ColumnConvert    = 0x08, /**< Converts old column format to current. Deprecated, does nothing. */
    GOF_LightConvert     = 0x10, /**< Converts old lights format to current. */
    GOF_ShowGui          = 0x20, /**< Showing main Gui. */
    GOF_ShowPanel        = 0x40, /**< Showing the tabbed panel. */
    GOF_WorldInfluence   = 0x80, /**< Input to the in-game world is allowed. */
};

enum GameNumfieldDFlags {
    GNFldD_Unkn01 = 0x01,
    GNFldD_Unkn02 = 0x02,
    GNFldD_Unkn04 = 0x04,
    GNFldD_CreaturePasngr = 0x08, // Possessing a creature as a passenger (no direct control)
    GNFldD_Unkn10 = 0x10,
    GNFldD_Unkn20 = 0x20,
    GNFldD_Unkn40 = 0x40,
    GNFldD_Unkn80 = 0x80,
};
/******************************************************************************/
#pragma pack(1)

struct CreaturePool { // sizeof = 129
  long crtr_kind[CREATURE_TYPES_COUNT];
  unsigned char is_empty;
};

struct PerExpLevelValues { // sizeof = 10
  unsigned char value[10];
};

#define SIZEOF_Game 1382437

// only one such struct exists at .data:005F0310
// it ends at 00741B35
struct Game { // sizeof=0x151825
    // This was a level and version before, but now saved games have another versioning system.
    unsigned short unused_version[3];
    LevelNumber continue_level_number;
    unsigned char system_flags;
char align_B;
    /** Flags which control how the game operates, mostly defined by command line. */
    unsigned char operation_flags;
    unsigned char numfield_D;
    unsigned char flags_font;
    unsigned char flags_gui;
    unsigned char eastegg01_cntr;
    unsigned char flags_cd;
    unsigned char eastegg02_cntr;
    char audiotrack;
char numfield_14;
char numfield_15;
    LevelNumber selected_level_number;
char numfield_1A;
    unsigned char numfield_1B;
    struct PlayerInfo players[PLAYERS_COUNT];
    struct Column columns_data[COLUMNS_COUNT];
    struct CubeAttribs cubes_data[CUBE_ITEMS_MAX];
    struct ObjectConfig objects_config[OBJECT_TYPES_COUNT_ORIGINAL];
struct ObjectConfig objects_config_UNUSED[103];
char field_117DA[14];
    // Traps and doors config; note that eventually we'll want to merge it with trapdoor_conf
    struct ManfctrConfig traps_config_[TRAP_TYPES_COUNT];
    struct ManfctrConfig doors_config_[DOOR_TYPES_COUNT];
    struct SpellConfig spells_config[30];
    struct Things things;
    struct Persons persons;
    struct Columns columns;
    unsigned short slabset_num;
    struct SlabSet slabset[SLABSET_COUNT];
    unsigned short slabobjs_num;
    short slabobjs_idx[SLABSET_COUNT];
    struct SlabObj slabobjs[SLABOBJS_COUNT];
    unsigned char land_map_start;
    struct LightsShadows lish;
    struct CreatureControl cctrl_data[CREATURES_COUNT];
    struct Thing things_data[THINGS_COUNT];
    unsigned char navigation_map[256*256];
    struct Map map[256*256]; // field offset 0xDC157
    struct ComputerTask computer_task[COMPUTER_TASKS_COUNT];
    struct Computer2 computer[PLAYERS_COUNT];
    struct SlabMap slabmap[85*85];
    struct Room rooms[ROOMS_COUNT];
    struct Dungeon dungeon[DUNGEONS_COUNT];
char field_149E05;
    struct StructureList thing_lists[13];
    int field_149E6E; // signed
char field_149E72[5];
    unsigned int unrevealed_column_idx;
unsigned char field_149E7B;
unsigned int field_149E7C;
    unsigned char packet_save_enable;
    unsigned char packet_load_enable;
    char packet_fname[150];
    char packet_fopened;
    TbFileHandle packet_save_fp;
unsigned int packet_file_pos;
    struct PacketSaveHead packet_save_head;
    unsigned long turns_stored;
    unsigned long turns_fastforward;
unsigned char numfield_149F38;
    unsigned char packet_checksum_verify;
    unsigned long log_things_start_turn;
    unsigned long log_things_end_turn;
    unsigned long turns_packetoff;
    unsigned char local_plyr_idx;
unsigned char numfield_149F47;
// Originally, save_catalogue was here.
    char campaign_fname[CAMPAIGN_FNAME_LEN];
    char save_catalogue_UNUSED[72];
    struct Event event[EVENTS_COUNT];
unsigned long field_14A804;
unsigned long field_14A808;
unsigned long field_14A80C;
unsigned long field_14A810;
unsigned long field_14A814;
short field_14A818[18];
char field_14A83C;
    //unsigned char level_file_number; // merged with level_number to get maps > 255
    short loaded_level_number;
    short texture_animation[8*TEXTURE_BLOCKS_ANIM_COUNT];
unsigned short field_14AB3F;
    unsigned char texture_id;
    unsigned short free_things[THINGS_COUNT-1];
    /** Index of the first used element in free things array. All elements BEYOND this index are free. If all things are free, it is set to 0. */
    unsigned short free_things_start_index;
    unsigned long play_gameturn;
    unsigned long pckt_gameturn;
    /** Synchronized random seed. used for game actions, as it's always identical for clients of network game. */
    unsigned long action_rand_seed;
    /** Unsynchronized random seed. Shouldn't affect game actions, because it's local - other clients have different value. */
    unsigned long unsync_rand_seed;
short field_14BB52;
unsigned char field_14BB54;
int field_14BB55;
int field_14BB59;
int field_14BB5D;
    unsigned long time_delta;
short field_14BB65[592];
    unsigned char small_map_state;
    struct Coord3d pos_14C006;
    struct Packet packets[PACKETS_COUNT];
    struct CreatureStatsOLD creature_stats_OLD[CREATURE_TYPES_COUNT]; // New stats are in GameAdd
    struct RoomStats room_stats[ROOM_TYPES_COUNT];
    struct MagicStats keeper_power_stats[POWER_TYPES_COUNT];
    struct ActionPoint action_points[ACTN_POINTS_COUNT];
char active_players_count;
    unsigned char hero_player_num;
    unsigned char neutral_player_num;
int field_14E498;
short field_14E49C;
short field_14E49E;
int field_14E4A0;
short field_14E4A4;
    struct GoldLookup gold_lookup[GOLD_LOOKUP_COUNT];
    unsigned short ambient_sound_thing_idx;
    unsigned short block_health[9];
    unsigned short minimum_gold;
    unsigned short max_gold_lookup;
    unsigned short min_gold_to_record;
    unsigned short wait_for_room_time;
    unsigned short check_expand_time;
    unsigned short max_distance_to_dig;
    unsigned short wait_after_room_area;
    unsigned short per_imp_gold_dig_size;
    unsigned short default_generate_speed;
    unsigned short generate_speed;
    unsigned short field_14E92E;
unsigned char field_14E930[4];
    unsigned long entrance_last_generate_turn;
    unsigned short entrance_room_id;
    unsigned short entrances_count;
    unsigned short gold_per_gold_block;
    unsigned short pot_of_gold_holds;
    unsigned short chest_gold_hold;
    unsigned short gold_pile_value;
    unsigned short gold_pile_maximum;
    unsigned short fight_max_hate;
    unsigned short fight_borderline;
    unsigned short fight_max_love;
    unsigned short food_life_out_of_hatchery;
    unsigned short fight_hate_kill_value;
    unsigned short body_remains_for;
    unsigned short graveyard_convert_time;
    unsigned short tile_strength;
    unsigned short gold_tile_strength;
unsigned char field_14E958[208];
unsigned short field_14EA28;
unsigned short field_14EA2A;
unsigned short field_14EA2C;
unsigned short field_14EA2E;
unsigned long field_14EA30;
unsigned short field_14EA34;
unsigned short field_14EA36;
unsigned short field_14EA38;
unsigned char field_14EA3A[8];
    unsigned char min_distance_for_teleport;
    unsigned char recovery_frequency;
unsigned short field_14EA44;
    unsigned short nodungeon_creatr_list_start; /**< Linked list of creatures which have no dungeon (neutral and owned by nonexisting players) */
    unsigned short food_generation_speed;
    char game_kind; /**< Kind of the game being played, from GameKinds enumeration. Originally was GameMode. */
char field_14EA4B;
    struct PerExpLevelValues creature_scores[CREATURE_TYPES_COUNT];
    unsigned long default_max_crtrs_gen_entrance;
    unsigned long default_imp_dig_damage;
    unsigned long default_imp_dig_own_damage;
    unsigned short game_turns_in_flee;
    unsigned short hunger_health_loss;
    unsigned short turns_per_hunger_health_loss;
    unsigned short food_health_gain;
    unsigned char prison_skeleton_chance;
    unsigned short torture_health_loss;
    unsigned short turns_per_torture_health_loss;
    unsigned char ghost_convert_chance;
    struct LevelScriptOld script;
    struct Bookmark bookmark[BOOKMARKS_COUNT];
    struct CreaturePool pool;
    long frame_skip;
    unsigned long pay_day_gap;
unsigned long pay_day_progress;
    unsigned long power_hand_gold_grab_amount;
    unsigned char no_intro;
    unsigned long hero_door_wait_time;
    unsigned long dungeon_heart_heal_time;
    long dungeon_heart_heal_health;
    unsigned long dungeon_heart_health;
    unsigned char disease_transfer_percentage;
    unsigned char disease_lose_percentage_health;
    unsigned char disease_lose_health_time;
    unsigned long armageddon_cast_turn;
unsigned long armageddon_field_15035A;
    unsigned char armageddon_caster_idx;
    unsigned long hold_audience_time;
    unsigned long armagedon_teleport_your_time_gap;
    unsigned long armagedon_teleport_enemy_time_gap;
    unsigned char hits_per_slab;
    long collapse_dungeon_damage;
    unsigned long turns_per_collapse_dngn_dmg;
    struct SoundSettings sound_settings;
long field_15038E;
    long num_fps;
    unsigned long train_cost_frequency;
    unsigned long scavenge_cost_frequency;
    unsigned long temple_scavenge_protection_turns;
char numfield_1503A2;
    unsigned char bodies_for_vampire;
    struct CreatureBattle battles[BATTLES_COUNT];
unsigned char field_1506D4;
    long music_track_index;
    char evntbox_text_objective[MESSAGE_TEXT_LEN];
    char evntbox_text_buffer[MESSAGE_TEXT_LEN];
    struct TextScrollWindow evntbox_scroll_window;
char field_1512E6[1037];
    long flash_button_index; /**< GUI Button Designation ID of a button which is supposed to flash, as part of tutorial. */
    long flash_button_gameturns; // signed
long field_1516FB;
    char loaded_swipe_idx;
    long boulder_reduce_health_wall;
    long boulder_reduce_health_slap;
    long boulder_reduce_health_room;
    struct GuiMessage messages_[3];
    unsigned char active_messages_count;
    // Moved bonuses_foudn to IntralevelData
    unsigned char ex_intralvl_plug[6];
    long bonus_time;
    // Moved transfered_creauture to IntralevelData
    unsigned char ex_transfered_creature_plug[2];
    struct Armageddon armageddon;
    char active_panel_mnu_idx; /**< The MenuID of currently active panel menu, or 0 if none. */
    char comp_player_aggressive;
    char comp_player_defensive;
    char comp_player_construct;
    char comp_player_creatrsonly;
    /** Imprisonment tendency variable. Used for GUI only; the real tendency is a flag inside Dungeon. */
    char creatures_tend_imprison;
    /** Flee tendency variable. Used for GUI only; the real tendency is a flag inside Dungeon. */
    char creatures_tend_flee;
    short hand_over_subtile_x;
    short hand_over_subtile_y;
    int chosen_room_kind;
    int chosen_room_spridx;
    int chosen_room_tooltip;
    int chosen_spell_type;
    int chosen_spell_spridx;
    int chosen_spell_tooltip;
    int manufactr_element;
    int manufactr_spridx;
    int manufactr_tooltip;
};

#pragma pack()
/******************************************************************************/
DLLIMPORT extern struct Game _DK_game;
#define game _DK_game
/******************************************************************************/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
