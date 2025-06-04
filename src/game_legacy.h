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
#include "config_campaigns.h"
#include "config_magic.h"
#include "config_trapdoor.h"
#include "config_objects.h"
#include "config_cubes.h"
#include "config_powerhands.h"
#include "config_cubes.h"
#include "config_creature.h"
#include "config_crtrmodel.h"
#include "config_effects.h"
#include "config_objects.h"
#include "config_rules.h"
#include "config_players.h"
#include "config_slabsets.h"
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
#include "lua_cfg_funcs.h"
#include "engine_textures.h"

#define BOOKMARKS_COUNT               5

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

struct CreaturePool {
  long crtr_kind[CREATURE_TYPES_MAX];
  unsigned char is_empty;
};

struct PerExpLevelValues {
  unsigned char value[10];
};

struct Configs {
    struct SlabsConfig slab_conf;
    struct PowerHandConfig power_hand_conf;
    struct MagicConfig magic_conf;
    struct CubesConfig cube_conf;
    struct TrapDoorConfig trapdoor_conf;
    struct EffectsConfig effects_conf;
    struct CreatureConfig crtr_conf;
    struct ObjectsConfig object_conf;
    struct RulesConfig rules;
    struct PlayerStateConfig plyr_conf;
    struct ColumnConfig column_conf;
    struct LuaFuncsConf lua;
};

struct Game {
    LevelNumber continue_level_number;
    unsigned char system_flags;
    /** Flags which control how the game operates, mostly defined by command line. */
    unsigned char operation_flags;
    unsigned char numfield_D; //flags in enum GameNumfieldDFlags
    unsigned char flags_font;
    unsigned char flags_gui;
    unsigned char eastegg01_cntr;
    unsigned char flags_cd;
    unsigned char eastegg02_cntr;
    char music_track; // cdrom / default music track to resume after load
    char music_fname[DISKPATH_SIZE]; // custom music file to resume after load
    char numfield_15;
    LevelNumber selected_level_number;
    char numfield_1A;
    unsigned char numfield_1B;
    struct PlayerInfo players[PLAYERS_COUNT];
    struct Column columns_data[COLUMNS_COUNT];
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
    NavColour navigation_map[MAX_SUBTILES_X*MAX_SUBTILES_Y];
    struct Map map[MAX_SUBTILES_X*MAX_SUBTILES_Y];
    struct ComputerTask computer_task[COMPUTER_TASKS_COUNT];
    struct Computer2 computer[PLAYERS_COUNT];
    struct SlabMap slabmap[MAX_TILES_X*MAX_TILES_Y];
    struct Room rooms[ROOMS_COUNT];
    struct Dungeon dungeon[DUNGEONS_COUNT];
    struct StructureList thing_lists[13];
    ColumnIndex unrevealed_column_idx;
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
    PlayerNumber local_plyr_idx;
    unsigned char numfield_149F47; // something with packetload
    // Originally, save_catalogue was here.
    char campaign_fname[CAMPAIGN_FNAME_LEN];
    struct Event event[EVENTS_COUNT];
    unsigned long ceiling_height_max;
    unsigned long ceiling_height_min;
    unsigned long ceiling_dist;
    unsigned long ceiling_search_dist;
    unsigned long ceiling_step;
    short col_static_entries[18];
    //unsigned char level_file_number; // merged with level_number to get maps > 255
    short loaded_level_number;
    short texture_animation[TEXTURE_BLOCKS_ANIM_FRAMES*TEXTURE_BLOCKS_ANIM_COUNT];
    unsigned char texture_id;
    unsigned short free_things[THINGS_COUNT-1];
    /** Index of the first used element in free things array. All elements BEYOND this index are free. If all things are free, it is set to 0. */
    ThingIndex free_things_start_index;
    GameTurn play_gameturn;
    GameTurn pckt_gameturn;
    /** Synchronized random seed. used for game actions, as it's always identical for clients of network game. */
    unsigned long action_rand_seed;
    /** Unsynchronized random seed. Shouldn't affect game actions, because it's local - other clients have different value. */
    unsigned long unsync_rand_seed;
    int something_light_x;
    int something_light_y;
    unsigned long time_delta;
    short top_cube[TEXTURE_BLOCKS_COUNT];// if you ask for top cube on a column without cubes, it'll return the first cube it finds with said texture at the top
    unsigned char small_map_state;
    struct Coord3d mouse_light_pos;
    struct Packet packets[PACKETS_COUNT];
    char active_players_count;
    PlayerNumber neutral_player_num;
    struct GoldLookup gold_lookup[GOLD_LOOKUP_COUNT];
    unsigned short ambient_sound_thing_idx;
    HitPoints block_health[10];
    unsigned short generate_speed;
    unsigned long entrance_last_generate_turn;
    unsigned short entrance_room_id;
    unsigned short entrances_count;
    unsigned short nodungeon_creatr_list_start; /**< Linked list of creatures which have no dungeon (neutral and owned by nonexisting players) */
    enum GameKinds game_kind; /**< Kind of the game being played, from GameKinds enumeration. Originally was GameMode. */
    TbBool map_changed_for_nagivation; // something with navigation
    struct PerExpLevelValues creature_scores[CREATURE_TYPES_MAX];
    struct Bookmark bookmark[BOOKMARKS_COUNT];
    struct CreaturePool pool;
    long frame_skip;
    TbBool frame_step;
    TbBool paused_at_gameturn;
    GameTurnDelta pay_day_progress;
    GameTurn armageddon_cast_turn;
    GameTurn armageddon_over_turn;
    PlayerNumber armageddon_caster_idx;
    struct SoundSettings sound_settings;
    struct CreatureBattle battles[BATTLES_COUNT];
    char evntbox_text_objective[MESSAGE_TEXT_LEN];
    char evntbox_text_buffer[MESSAGE_TEXT_LEN];
    struct TextScrollWindow evntbox_scroll_window;
    long flash_button_index; /**< GUI Button Designation ID of a button which is supposed to flash, as part of tutorial. */
    char loaded_swipe_idx;
    unsigned char active_messages_count;
    long bonus_time;
    struct Armageddon armageddon;
    char active_panel_mnu_idx; /**< The MenuID of currently active panel menu, or 0 if none. */
    char comp_player_aggressive;
    char comp_player_defensive;
    char comp_player_construct;
    char comp_player_creatrsonly;
    /** Imprisonment tendency variable. Used for GUI only; the real tendency is a flag inside Dungeon. */
    TbBool creatures_tend_imprison;
    /** Flee tendency variable. Used for GUI only; the real tendency is a flag inside Dungeon. */
    TbBool creatures_tend_flee;
    MapSubtlCoord hand_over_subtile_x;
    MapSubtlCoord hand_over_subtile_y;
    int chosen_room_kind;
    int chosen_room_spridx;
    int chosen_room_tooltip;
    int chosen_spell_type;
    int chosen_spell_spridx;
    int chosen_spell_tooltip;
    int manufactr_element;
    int manufactr_spridx;
    int manufactr_tooltip;
    struct Configs conf;
    unsigned long turn_last_checked_for_gold;
    unsigned short computer_chat_flags;
    char quick_messages[QUICK_MESSAGES_COUNT][MESSAGE_TEXT_LEN];
    struct GuiMessage messages[GUI_MESSAGES_COUNT];
    struct LightSystemState lightst;
    uint8_t               max_custom_box_kind;
    unsigned long         current_player_turn; // Actually it is a hack. We need to rewrite scripting for current player
    int                   script_current_player;
    struct Coord3d        triggered_object_location; //Position of `TRIGGERED_OBJECT`
    char                  box_tooltip[CUSTOM_BOX_COUNT][MESSAGE_TEXT_LEN];
    struct ScriptFxLine   fx_lines[FX_LINES_COUNT];
    int                   active_fx_lines;
    struct ActionPoint action_points[ACTN_POINTS_COUNT];
    LevelNumber last_level; // Used to restore custom sprites
    struct LevelScript script;
    PlayerNumber script_timer_player;
    unsigned char script_timer_id;
    unsigned long script_timer_limit;
    TbBool timer_real;
    unsigned char script_value_type;
    unsigned char script_value_id;
    PlayerNumber script_variable_player;
    long script_variable_target;
    unsigned char script_variable_target_type;
    TbBool heart_lost_display_message;
    TbBool heart_lost_quick_message;
    unsigned long heart_lost_message_id;
    long heart_lost_message_target;
    unsigned char slab_ext_data[MAX_TILES_X*MAX_TILES_Y];
    unsigned char slab_ext_data_initial[MAX_TILES_X*MAX_TILES_Y];
    float delta_time;
    long double process_turn_time;
    float flash_button_time;
    MapSubtlCoord map_subtiles_x;
    MapSubtlCoord map_subtiles_y;
    MapSlabCoord map_tiles_x;
    MapSlabCoord map_tiles_y;
    long navigation_map_size_x;
    long navigation_map_size_y;
    short around_map[AROUND_MAP_LENGTH];
    short around_slab[AROUND_SLAB_LENGTH];
    short around_slab_eight[AROUND_SLAB_EIGHT_LENGTH];
    short small_around_slab[SMALL_AROUND_SLAB_LENGTH];
};

#pragma pack()
/******************************************************************************/
extern struct Game game;
extern long game_num_fps;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
