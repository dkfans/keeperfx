/******************************************************************************/
// Dungeon Keeper fan extension.
/******************************************************************************/
/** @file keeperfx.hpp
 *     Header file for main.cpp.
 * @par Purpose:
 *     Header file. Defines exported routines from keeperfx.dll.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     27 May 2008 - 11 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_KEEPERFX_H
#define DK_KEEPERFX_H

#include "globals.h"
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "packets.h"
#include "thing_data.h"
#include "thing_list.h"
#include "thing_doors.h"
#include "dungeon_data.h"
#include "player_computer.h"
#include "game_merge.h"
#include "game_saves.h"
#include "thing_creature.h"
#include "creature_control.h"
#include "player_data.h"
#include "slab_data.h"
#include "light_data.h"
#include "actionpt.h"
#include "map_data.h"
#include "map_columns.h"
#include "room_data.h"
#include "config.h"
#include "config_magic.hpp"
#include "net_game.h"
#include "sounds.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LEGAL_WIDTH  640
#define LEGAL_HEIGHT 480

#define PLAYERS_EXT_COUNT       6
#define ROOMS_COUNT           150
#define GUI_MESSAGES_COUNT      3
#define CREATURE_PARTYS_COUNT  16
#define PARTY_MEMBERS_COUNT     8
#define PARTY_TRIGGERS_COUNT   48
#define TUNNELLER_TRIGGERS_COUNT 16
#define WIN_CONDITIONS_COUNT    4
#define CONDITIONS_COUNT       48
#define MANUFACTURED_ITEMS_LIMIT 199
#define GAME_KEYS_COUNT        32
#define LENSES_COUNT           15
#define MINMAXS_COUNT          64
#define SHADOW_LIMITS_COUNT  2048
#define SPIRAL_STEPS_COUNT   2500
#define GOLD_LOOKUP_COUNT      40
#define SPELL_POINTER_GROUPS   14
#define SLABSET_COUNT        1304
#define SLABOBJS_COUNT        512
#define BOOKMARKS_COUNT         5
#define SHADOW_CACHE_COUNT          40
// Amount of instances; it's 17, 18 or 19
#define PLAYER_INSTANCES_COUNT 19
#define PLAYER_STATES_COUNT    32
#define ZOOM_KEY_ROOMS_COUNT   14
#define SMALL_MAP_RADIUS       58
#define BLOOD_TYPES_COUNT       7
#define AROUND_TILES_COUNT      9
// Static textures
#define TEXTURE_BLOCKS_STAT_COUNT   544
// Animated textures
#define TEXTURE_BLOCKS_ANIM_COUNT    48
#define TEXTURE_BLOCKS_COUNT TEXTURE_BLOCKS_STAT_COUNT+TEXTURE_BLOCKS_ANIM_COUNT
// Types of objects
#define SLAB_TYPES_COUNT      58
#define SCRIPT_VALUES_COUNT   64
// Camera constants; max zoom is when everything is large
#define CAMERA_ZOOM_MIN     4100
#define CAMERA_ZOOM_MAX    12000
// Strings length
#define CAMPAIGN_FNAME_LEN    64

enum PlayerViewType {
    PVT_DungeonTop          =  1,
    PVT_CreatureContrl      =  2,
    PVT_CreaturePasngr      =  3,
    PVT_MapScreen           =  4,
};

enum PlayerVictoryState {
    VicS_Undecided          =  0,
    VicS_WonLevel           =  1,
    VicS_LostLevel          =  2,
    VicS_State3             =  3,
};

enum ModeFlags {
    MFlg_IsDemoMode         =  0x01,
    MFlg_EyeLensReady       =  0x02,
    MFlg_unk04              =  0x04,
    MFlg_DeadBackToPool     =  0x08,
    MFlg_NoMusic            =  0x10,
    MFlg_unk20              =  0x20,
    MFlg_unk40              =  0x40,
    MFlg_unk80              =  0x80,
};

enum FFlags {
    FFlg_unk01              =  0x01,
    FFlg_unk02              =  0x02,
    FFlg_unk04              =  0x04,
    FFlg_unk08              =  0x08,
    FFlg_unk10              =  0x10,
    FFlg_AlexCheat          =  0x20,
    FFlg_UsrSndFont         =  0x40,
    FFlg_unk80              =  0x80,
};

enum DebugFlags {
    DFlg_ShotsDamage        =  0x01,
    DFlg_unk02              =  0x02,
};

struct TbLoadFiles;
struct RoomFlag;
struct Number;
struct JontySpr;

#ifdef __cplusplus
#pragma pack(1)
#endif

// Windows-standard structure
/*struct _GUID {
     unsigned long Data1;
     unsigned short Data2;
     unsigned short Data3;
     unsigned char Data4[8];
};*/

struct KeycodeString {
    TbKeyCode keys[LINEMSG_SIZE];
    long length;
};

struct StartupParameters {
    LevelNumber selected_level_number;
    unsigned char no_intro;
    unsigned char one_player;
    unsigned char numfield_C;
    unsigned char flags_font;
    unsigned char flags_cd;
    unsigned char debug_flags;
    long num_fps;
    unsigned char packet_save_enable;
    unsigned char packet_load_enable;
    char packet_fname[150];
    unsigned char packet_checksum;
};

struct GuiMessage { // sizeof = 0x45 (69)
    char text[64];
unsigned char field_40;
unsigned long field_41;
};

struct TextScrollWindow {
    char text[MESSAGE_TEXT_LEN];
    long start_y;
    char action;
    long text_height;
    long window_height;
};

struct PartyMember { // sizeof = 13
  unsigned char flags;
  unsigned char field_65;
  unsigned char crtr_kind;
  unsigned char objectv;
  long countdown;
  unsigned char crtr_level;
  unsigned short carried_gold;
  unsigned short field_6F;
};


struct Party { // sizeof = 208
  char prtname[100];
  struct PartyMember members[PARTY_MEMBERS_COUNT];
  unsigned long members_num;
};

struct InitEffect { // sizeof = 39
  short start_health;
  unsigned char generation_type;
  short accel_xy_min;
  short accel_xy_max;
  short accel_z_min;
  short accel_z_max;
  unsigned char field_B;
  short field_C;
  unsigned char kind_min;
  unsigned char kind_max;
  unsigned char area_affect_type;
  unsigned char field_11;
  struct InitLight ilght;
  unsigned char field_26;
};

struct MapOffset {
  char v;
  char h;
  unsigned short both;
};

struct GoldLookup { // sizeof = 28
unsigned char field_0;
unsigned char field_1;
unsigned long field_2;
unsigned short field_6;
unsigned short field_8;
unsigned short field_A;
unsigned short field_C;
unsigned short field_E;
unsigned long field_10;
unsigned char field_14[6];
unsigned short field_1A;
};

struct UnkStruc5 { // sizeof=0x12
    unsigned short texture_0[3];
char field_6[2];
short field_8[5];
};

struct UnkStruc6 { // sizeof = 8
  unsigned char field_0;
  unsigned char field_1;
  unsigned char field_2;
  unsigned char field_3;
  unsigned long field_4;
};

struct UnkStruc7 { // sizeof = 17
  unsigned long field_0;
  unsigned char field_4[9];
  unsigned short field_D;
  unsigned short field_F;
};

struct ShadowCache { // sizeof = 129
  unsigned char flags;
  unsigned char field_1[127];
  unsigned char field_80;
};

struct ObjectConfig { // sizeof=0x1D
    long health;
char field_4;
char field_5;
char field_6;
char field_7;
char field_8;
short light;
char field_B;
char field_C[14];
char field_1A;
short field_1B;
};

struct Armageddon { // sizeof = 14
  unsigned long count_down;
  unsigned long duration;
  struct Coord3d mappos;
};

struct ManfctrConfig { // sizeof=0x14
  int manufct_level;
  int manufct_required;
  int shots;
  int shots_delay;
  long selling_value;
};

struct KeyToStringInit { // sizeof = 5
  unsigned char chr;
  long str_idx;
};

struct Event { // sizeof=0x15
unsigned char field_0;
unsigned char field_1;
    long mappos_x;
    long mappos_y;
    unsigned char owner;
    unsigned char kind;
    long target;
    long birth_turn;
unsigned char field_14;
};

struct PerExpLevelValues { // sizeof = 10
  unsigned char value[10];
};

struct RoomStats {
  short cost;
  unsigned short health;
};

struct CreaturePool { // sizeof = 129
  long crtr_kind[CREATURE_TYPES_COUNT];
  unsigned char is_empty;
};

struct Bookmark { // sizeof = 3
  unsigned char x;
  unsigned char y;
  unsigned char flags;
};

struct GameKey { // sizeof = 2
  unsigned char code;
  unsigned char mods;
};

struct GameSettings { // sizeof = 0x52 (82)
    unsigned char field_0;
    unsigned char video_shadows;
    unsigned char view_distance;
    unsigned char field_3;
    unsigned char video_textures;
    unsigned char video_cluedo_mode;
    unsigned char sound_volume;
    unsigned char redbook_volume;
    unsigned char field_8;
    unsigned short gamma_correction;
    int video_scrnmode;
    struct GameKey kbkeys[GAME_KEYS_COUNT];
    unsigned char tooltips_on;
    unsigned char first_person_move_invert;
    unsigned char first_person_move_sensitivity;
    };

struct TunnellerTrigger { // sizeof = 18
  unsigned char flags;
  char condit_idx;
  unsigned char plyr_idx;
  unsigned long location;
  unsigned char heading;
  long target;
  long carried_gold;
  unsigned char crtr_level;
  char party_id;
};

struct PartyTrigger { // sizeof = 13
  unsigned char flags;
  char condit_idx;
  char creatr_id;
  unsigned char plyr_idx;
  unsigned long location;
  unsigned char crtr_level;
  unsigned short carried_gold;
  unsigned short ncopies;
};

struct ScriptValue { // sizeof = 16
  unsigned char flags;
  char condit_idx;
  unsigned char valtype;
  unsigned char field_3;
  long field_4;
  long field_8;
  long field_C;
};

struct Condition { // sizeof = 12
  short condit_idx;
  unsigned char status;
  unsigned char plyr_idx;
  unsigned char variabl_type;
  unsigned short variabl_idx;
  unsigned char operation;
  unsigned long rvalue;
};

struct LevelScript { // sizeof = 5884
    struct TunnellerTrigger tunneller_triggers[TUNNELLER_TRIGGERS_COUNT];
    unsigned long tunneller_triggers_num;
    struct PartyTrigger party_triggers[PARTY_TRIGGERS_COUNT];
    unsigned long party_triggers_num;
    struct ScriptValue values[SCRIPT_VALUES_COUNT];
    unsigned long values_num;
    struct Condition conditions[CONDITIONS_COUNT];
    unsigned long conditions_num;
    struct Party creature_partys[CREATURE_PARTYS_COUNT];
    unsigned long creature_partys_num;
    unsigned short win_conditions[WIN_CONDITIONS_COUNT];
    unsigned long win_conditions_num;
    unsigned short lose_conditions[WIN_CONDITIONS_COUNT];
    unsigned long lose_conditions_num;
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
    unsigned char numfield_C;
char numfield_D;
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
    struct Column columns[COLUMNS_COUNT];
struct UnkStruc5 struc_D8C7[512];
struct ObjectConfig objects_config[239];
char field_117DA[14];
    struct ManfctrConfig traps_config[TRAP_TYPES_COUNT];
    struct ManfctrConfig doors_config[DOOR_TYPES_COUNT];
    struct SpellConfig spells_config[30];
    struct Thing *things_lookup[THINGS_COUNT];
    struct Thing *things_end;
    struct Persons persons;
    struct Column *columns_lookup[COLUMNS_COUNT];
    struct Column *columns_end;
    unsigned short slabset_num;
    struct SlabSet slabset[SLABSET_COUNT];
    unsigned short slabobjs_num;
    short slabobjs_idx[SLABSET_COUNT];
    struct SlabObj slabobjs[SLABOBJS_COUNT];
unsigned char land_map_start;
// probably one structure.. LightSystem ?
    struct UnkStruc6 field_1DD41[1024];
    unsigned char shadow_limits[SHADOW_LIMITS_COUNT];
    struct Light lights[LIGHTS_COUNT];
    struct ShadowCache shadow_cache[SHADOW_CACHE_COUNT];
    unsigned short stat_light_map[256*256];
long field_46149;
char field_4614D;
char field_4614E;
int field_4614F;
int field_46153;
// end of "LightSystem"

    unsigned short field_46157[256*256];
    struct CreatureControl cctrl_data[CREATURES_COUNT];
    struct Thing things_data[THINGS_COUNT];
    unsigned char navigation_map[256*256];
    struct Map map[256*256];
    struct ComputerTask computer_task[COMPUTER_TASKS_COUNT];
    struct Computer2 computer[PLAYERS_COUNT];
    struct SlabMap slabmap[85*85];
    struct Room rooms[ROOMS_COUNT];
    struct Dungeon dungeon[DUNGEONS_COUNT];
char field_149E05;
    struct StructureList thing_lists[13];
    int field_149E6E; // signed
char field_149E72[5];
unsigned int field_149E77;
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
    unsigned char packet_checksum;
    unsigned long log_things_start_turn;
    unsigned long log_things_end_turn;
    int numfield_149F42;
unsigned char numfield_149F46;
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
    unsigned short free_things[THINGS_COUNT];
    unsigned long play_gameturn;
    unsigned long pckt_gameturn;
    unsigned long action_rand_seed;
    unsigned long unsync_rand_seed; // primary random seed; shouldn't affect game actions
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
    struct CreatureStats creature_stats[CREATURE_TYPES_COUNT];
    struct RoomStats room_stats[ROOM_TYPES_COUNT];
    struct MagicStats magic_stats[POWER_TYPES_COUNT];
    struct ActionPoint action_points[ACTN_POINTS_COUNT];
char field_14E495;
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
unsigned short field_14E93A;
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
unsigned short field_14EA46;
    unsigned short food_generation_speed;
char flagfield_14EA4A;
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
    struct LevelScript script;
    struct Bookmark bookmark[BOOKMARKS_COUNT];
    struct CreaturePool pool;
    long frame_skip;
    unsigned long pay_day_gap;
unsigned long field_15033A;
    unsigned long power_hand_gold_grab_amount;
    unsigned char no_intro;
    unsigned long hero_door_wait_time;
    unsigned long dungeon_heart_heal_time;
    long dungeon_heart_heal_health;
    unsigned long dungeon_heart_health;
    unsigned char disease_transfer_percentage;
    unsigned char disease_lose_percentage_health;
    unsigned char disease_lose_health_time;
unsigned long field_150356;
unsigned long field_15035A;
unsigned char field_15035E;
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
    unsigned long temple_scavenge_protection_time;
char numfield_1503A2;
    unsigned char bodies_for_vampire;
    struct UnkStruc7 field_1503A4[48];
unsigned char field_1506D4;
long field_1506D5;
    char evntbox_text_objective[MESSAGE_TEXT_LEN];
    char evntbox_text_buffer[MESSAGE_TEXT_LEN];
    struct TextScrollWindow evntbox_scroll_window;
char field_1512E6[1037];
    long flash_button_index;
    long flash_button_gameturns; // signed
long field_1516FB;
char field_1516FF;
    long boulder_reduce_health_wall;
    long boulder_reduce_health_slap;
    long boulder_reduce_health_room;
    struct GuiMessage messages[GUI_MESSAGES_COUNT];
    unsigned char active_messages_count;
    unsigned char bonuses_found[BONUS_LEVEL_STORAGE_COUNT];
    long bonus_time;
    struct CreatureStorage transfered_creature;
    struct Armageddon armageddon;
char field_1517F6;
    char comp_player_aggressive;
    char comp_player_defensive;
    char comp_player_construct;
    char comp_player_creatrsonly;
    char creatures_tend_1;
    char creatures_tend_2;
    short hand_over_subtile_x;
    short hand_over_subtile_y;
int field_151801;
int field_151805;
int field_151809;
    int chosen_spell_type;
    int chosen_spell_look;
    int chosen_spell_tooltip;
int numfield_151819;
int numfield_15181D;
int numfield_151821;
};

#ifdef __cplusplus
#pragma pack()
#endif

#ifdef __cplusplus
extern "C" {
#endif
// Variables inside the main module
extern TbClockMSec last_loop_time;
extern short default_loc_player;
extern struct GuiBox *gui_box;
extern struct GuiBox *gui_cheat_box;
extern int test_variable;
extern unsigned short const player_state_to_spell[];
extern struct RoomInfo room_info[];
extern const struct Around around[];
extern struct StartupParameters start_params;
//Functions - exported by the DLL

DLLIMPORT int __stdcall _DK_LbErrorLogSetup(char *directory, char *filename, unsigned char flag);
DLLIMPORT void _DK_set_cpu_mode(int mode);
DLLIMPORT int _DK_setup_heaps(void);
DLLIMPORT void _DK_input_eastegg(void);
DLLIMPORT void _DK_update(void);
DLLIMPORT void _DK_wait_at_frontend(void);
DLLIMPORT void _DK_delete_all_structures(void);
DLLIMPORT void _DK_PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *palette);
DLLIMPORT void _DK_initialise_eye_lenses(void);
DLLIMPORT void _DK_setup_eye_lens(long);
DLLIMPORT char *_DK_mdlf_for_cd(struct TbLoadFiles *);
DLLIMPORT char *_DK_mdlf_default(struct TbLoadFiles *);


// Global variables migration between DLL and the program

DLLIMPORT extern struct Game _DK_game;
#define game _DK_game
DLLIMPORT extern unsigned char *_DK_blue_palette;
#define blue_palette _DK_blue_palette
DLLIMPORT extern struct TbLoadFiles _DK_game_load_files[];
#define game_load_files _DK_game_load_files
DLLIMPORT extern struct GameSettings _DK_settings;
#define settings _DK_settings
DLLIMPORT extern unsigned char _DK_exit_keeper;
#define exit_keeper _DK_exit_keeper
DLLIMPORT extern unsigned char _DK_quit_game;
#define quit_game _DK_quit_game
DLLIMPORT extern int _DK_continue_game_option_available;
#define continue_game_option_available _DK_continue_game_option_available
DLLIMPORT extern long _DK_last_mouse_x;
#define last_mouse_x _DK_last_mouse_x
DLLIMPORT extern long _DK_last_mouse_y;
#define last_mouse_y _DK_last_mouse_y
DLLIMPORT extern long _DK_frontstory_text_no;
#define frontstory_text_no _DK_frontstory_text_no
DLLIMPORT extern int _DK_FatalError;
#define FatalError _DK_FatalError
DLLIMPORT extern struct TbLoadFiles _DK_frontstory_load_files[4];
#define frontstory_load_files _DK_frontstory_load_files
DLLIMPORT extern struct TbLoadFiles _DK_netmap_flag_load_files[7];
#define netmap_flag_load_files _DK_netmap_flag_load_files
DLLIMPORT extern int _DK_fe_high_score_table_from_main_menu;
#define fe_high_score_table_from_main_menu _DK_fe_high_score_table_from_main_menu
DLLIMPORT extern long _DK_define_key_scroll_offset;
#define define_key_scroll_offset _DK_define_key_scroll_offset
DLLIMPORT extern struct TbSetupSprite _DK_frontstory_setup_sprites[2];
#define frontstory_setup_sprites _DK_frontstory_setup_sprites
DLLIMPORT extern unsigned long _DK_time_last_played_demo;
#define time_last_played_demo _DK_time_last_played_demo
DLLIMPORT extern unsigned short _DK_battle_creature_over;
#define battle_creature_over _DK_battle_creature_over
DLLIMPORT extern char _DK_gui_room_type_highlighted;
#define gui_room_type_highlighted _DK_gui_room_type_highlighted
DLLIMPORT extern char _DK_gui_door_type_highlighted;
#define gui_door_type_highlighted _DK_gui_door_type_highlighted
DLLIMPORT extern char _DK_gui_trap_type_highlighted;
#define gui_trap_type_highlighted _DK_gui_trap_type_highlighted
DLLIMPORT extern char _DK_gui_creature_type_highlighted;
#define gui_creature_type_highlighted _DK_gui_creature_type_highlighted
DLLIMPORT extern short _DK_drag_menu_x;
#define drag_menu_x _DK_drag_menu_x
DLLIMPORT extern short _DK_drag_menu_y;
#define drag_menu_y _DK_drag_menu_y
DLLIMPORT extern unsigned short _DK_tool_tip_time;
#define tool_tip_time _DK_tool_tip_time
DLLIMPORT extern unsigned short _DK_help_tip_time;
#define help_tip_time _DK_help_tip_time
DLLIMPORT extern unsigned char _DK_magic_to_object[24];
#define magic_to_object _DK_magic_to_object
DLLIMPORT extern unsigned char _DK_door_to_object[DOOR_TYPES_COUNT];
#define door_to_object _DK_door_to_object
DLLIMPORT extern unsigned short _DK_specials_text[10];
#define specials_text _DK_specials_text
DLLIMPORT extern unsigned char _DK_eastegg_skeksis_cntr;
#define eastegg_skeksis_cntr _DK_eastegg_skeksis_cntr
DLLIMPORT extern long _DK_pointer_x;
#define pointer_x _DK_pointer_x
DLLIMPORT extern long _DK_pointer_y;
#define pointer_y _DK_pointer_y
DLLIMPORT extern long _DK_block_pointed_at_x;
#define block_pointed_at_x _DK_block_pointed_at_x
DLLIMPORT extern long _DK_block_pointed_at_y;
#define block_pointed_at_y _DK_block_pointed_at_y
DLLIMPORT extern long _DK_pointed_at_frac_x;
#define pointed_at_frac_x _DK_pointed_at_frac_x
DLLIMPORT extern long _DK_pointed_at_frac_y;
#define pointed_at_frac_y _DK_pointed_at_frac_y
DLLIMPORT extern long _DK_top_pointed_at_x;
#define top_pointed_at_x _DK_top_pointed_at_x
DLLIMPORT extern long _DK_top_pointed_at_y;
#define top_pointed_at_y _DK_top_pointed_at_y
DLLIMPORT extern long _DK_top_pointed_at_frac_x;
#define top_pointed_at_frac_x _DK_top_pointed_at_frac_x
DLLIMPORT extern long _DK_top_pointed_at_frac_y;
#define top_pointed_at_frac_y _DK_top_pointed_at_frac_y
DLLIMPORT long _DK_anim_counter;
#define anim_counter _DK_anim_counter
DLLIMPORT unsigned char *_DK_block_ptrs[592];
#define block_ptrs _DK_block_ptrs
DLLIMPORT long _DK_frame_number;
#define frame_number _DK_frame_number
DLLIMPORT unsigned char _DK_grabbed_small_map;
#define grabbed_small_map _DK_grabbed_small_map
DLLIMPORT long _DK_draw_spell_cost;
#define draw_spell_cost _DK_draw_spell_cost
DLLIMPORT char _DK_level_name[88];
#define level_name _DK_level_name
DLLIMPORT unsigned char *_DK_block_mem;
#define block_mem _DK_block_mem
DLLIMPORT struct Room *_DK_start_rooms;
#define start_rooms _DK_start_rooms
DLLIMPORT struct Room *_DK_end_rooms;
#define end_rooms _DK_end_rooms
DLLIMPORT unsigned char _DK_smooth_on;
#define smooth_on _DK_smooth_on
DLLIMPORT struct BigSprite _DK_status_panel;
#define status_panel _DK_status_panel
DLLIMPORT char _DK_top_of_breed_list;
#define top_of_breed_list _DK_top_of_breed_list
DLLIMPORT char _DK_no_of_breeds_owned;
#define no_of_breeds_owned _DK_no_of_breeds_owned
DLLIMPORT long _DK_optimised_lights;
#define optimised_lights _DK_optimised_lights
DLLIMPORT long _DK_total_lights;
#define total_lights _DK_total_lights
DLLIMPORT unsigned char _DK_do_lights;
#define do_lights _DK_do_lights
DLLIMPORT long _DK_imp_spangle_effects[];
#define imp_spangle_effects _DK_imp_spangle_effects
DLLIMPORT struct Thing *_DK_thing_pointed_at;
#define thing_pointed_at _DK_thing_pointed_at
DLLIMPORT struct Map *_DK_me_pointed_at;
#define me_pointed_at _DK_me_pointed_at
DLLIMPORT long _DK_my_mouse_x;
#define my_mouse_x _DK_my_mouse_x
DLLIMPORT long _DK_my_mouse_y;
#define my_mouse_y _DK_my_mouse_y
DLLIMPORT unsigned char _DK_gui_slab;
#define gui_slab _DK_gui_slab
DLLIMPORT char *_DK_level_names_data;
#define level_names_data _DK_level_names_data
DLLIMPORT char *_DK_end_level_names_data;
#define end_level_names_data _DK_end_level_names_data
DLLIMPORT unsigned char *_DK_frontend_backup_palette;
#define frontend_backup_palette _DK_frontend_backup_palette
DLLIMPORT long _DK_dummy_x;
#define dummy_x _DK_dummy_x
DLLIMPORT long _DK_dummy_y;
#define dummy_y _DK_dummy_y
DLLIMPORT long _DK_dummy;
#define dummy _DK_dummy
DLLIMPORT long _DK_clicked_on_small_map;
#define clicked_on_small_map _DK_clicked_on_small_map
DLLIMPORT long _DK_old_mx;
#define old_mx _DK_old_mx
DLLIMPORT long _DK_old_my;
#define old_my _DK_old_my
DLLIMPORT unsigned char _DK_zoom_to_heart_palette[768];
#define zoom_to_heart_palette _DK_zoom_to_heart_palette
DLLIMPORT struct MapOffset _DK_spiral_step[SPIRAL_STEPS_COUNT];
#define spiral_step _DK_spiral_step
DLLIMPORT unsigned char _DK_EngineSpriteDrawUsingAlpha;
#define EngineSpriteDrawUsingAlpha _DK_EngineSpriteDrawUsingAlpha
DLLIMPORT long _DK_sound_heap_size;
#define sound_heap_size _DK_sound_heap_size
DLLIMPORT unsigned char *_DK_sound_heap_memory;
#define sound_heap_memory _DK_sound_heap_memory
DLLIMPORT long _DK_heap_size;
#define heap_size _DK_heap_size
DLLIMPORT unsigned char *_DK_heap;
#define heap _DK_heap
DLLIMPORT long _DK_key_to_string[256];
#define key_to_string _DK_key_to_string
DLLIMPORT struct InitEffect _DK_effect_info[];
#define effect_info _DK_effect_info
DLLIMPORT unsigned char _DK_temp_pal[768];
#define temp_pal _DK_temp_pal
DLLIMPORT unsigned char *_DK_lightning_palette;
#define lightning_palette _DK_lightning_palette

#ifdef __cplusplus
}
#endif

//Functions - reworked
short setup_game(void);
void game_loop(void);
short reset_game(void);
void update(void);

void intro(void);
void outro(void);

TbBool slap_object(struct Thing *thing);
TbBool object_is_slappable(const struct Thing *thing, long plyr_idx);
unsigned char external_set_thing_state(struct Thing *thing, long state);
void external_activate_trap_shot_at_angle(struct Thing *thing, long a2);
long is_thing_passenger_controlled(struct Thing *thing);
void remove_events_thing_is_attached_to(struct Thing *thing);

int can_thing_be_queried(struct Thing *thing, long a2);
int can_thing_be_possessed(struct Thing *thing, long a2);
long remove_workshop_object_from_player(long a1, long a2);
unsigned char tag_cursor_blocks_place_trap(unsigned char a1, long a2, long a3);
void stop_creatures_around_hand(char a1, unsigned short a2, unsigned short a3);
struct Thing *get_queryable_object_near(MapCoord pos_x, MapCoord pos_y, long plyr_idx);
long tag_blocks_for_digging_in_rectangle_around(long a1, long a2, char a3);
void untag_blocks_for_digging_in_rectangle_around(long a1, long a2, char a3);
void tag_cursor_blocks_sell_area(unsigned char a1, long a2, long a3, long a4);
long packet_place_door(long a1, long a2, long a3, long a4, unsigned char a5);
unsigned char find_door_of_type(unsigned long a1, unsigned char a2);
void delete_room_slabbed_objects(long a1);
unsigned char tag_cursor_blocks_place_door(unsigned char a1, long a2, long a3);
long remove_workshop_item(long a1, long a2, long a3);
void set_chosen_spell(long sptype, long sptooltip);
void set_chosen_spell_none(void);
unsigned char sight_of_evil_expand_check(void);
unsigned char call_to_arms_expand_check(void);
unsigned char general_expand_check(void);
TbBool add_spell_to_player(long spl_idx, long plyr_idx);
unsigned char tag_cursor_blocks_place_room(unsigned char a1, long a2, long a3, long a4);
TbBool delete_room_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned char gnd_slab);
TbBool all_dungeons_destroyed(struct PlayerInfo *win_player);
long add_gold_to_hoarde(struct Thing *thing, struct Room *room, long amount);
void check_map_for_gold(void);
short init_animating_texture_maps(void);
void reset_gui_based_on_player_mode(void);
void reinit_tagged_blocks_for_player(unsigned char idx);
void restore_computer_player_after_load(void);
void sound_reinit_after_load(void);
void draw_swipe(void);
void draw_bonus_timer(void);
void draw_flame_breath(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4);
void draw_lightning(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4);
void zoom_from_map(void);
void zoom_to_map(void);
void toggle_hero_health_flowers(void);
void check_players_won(void);
void check_players_lost(void);
void process_dungeon_power_magic(void);
void process_dungeon_devastation_effects(void);
void process_entrance_generation(void);
void process_things_in_dungeon_hand(void);
void process_payday(void);
TbBool bonus_timer_enabled(void);

short toggle_computer_player(int idx);
short save_settings(void);
void setup_engine_window(long x1, long y1, long x2, long y2);
void store_engine_window(TbGraphicsWindow *ewnd,int divider);
void load_engine_window(TbGraphicsWindow *ewnd);
void PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *pal);
void set_engine_view(struct PlayerInfo *player, long val);
unsigned char find_first_battle_of_mine(unsigned char idx);
unsigned long scale_camera_zoom_to_screen(unsigned long zoom_lvl);
void set_player_cameras_position(struct PlayerInfo *player, long pos_x, long pos_y);
short player_has_won(long plyr_idx);
short player_has_lost(long plyr_idx);
void set_player_as_won_level(struct PlayerInfo *player);
void set_player_as_lost_level(struct PlayerInfo *player);
void init_good_player_as(long plr_idx);
void init_keepers_map_exploration(void);
void init_dungeons_research(void);
TbBool add_research_to_player(long plyr_idx, long rtyp, long rkind, long amount);
TbBool add_research_to_all_players(long rtyp, long rkind, long amount);
TbBool remove_all_research_from_player(long plyr_idx);
TbBool clear_research_for_all_players(void);
TbBool research_overriden_for_player(long plyr_idx);
TbBool update_players_research_amount(long plyr_idx, long rtyp, long rkind, long amount);
TbBool update_or_add_players_research_amount(long plyr_idx, long rtyp, long rkind, long amount);
void clear_creature_pool(void);
void reset_creature_max_levels(void);
void reset_script_timers_and_flags(void);
void add_creature_to_pool(long kind, long amount, unsigned long a3);
TbBool load_stats_files(void);
void check_and_auto_fix_stats(void);
long update_dungeon_scores(void);
long update_dungeon_generation_speeds(void);
void calculate_dungeon_area_scores(void);
short send_creature_to_room(struct Thing *thing, struct Room *room);
struct Room *get_room_thing_is_on(struct Thing *thing);
void init_creature_state(struct Thing *thing);
void gui_set_button_flashing(long btn_idx, long gameturns);
void draw_texture(long a1, long a2, long a3, long a4, long a5, long a6, long a7);
TbBool create_random_evil_creature(long x, long y, PlayerNumber owner, long max_lv);
TbBool create_random_hero_creature(long x, long y, PlayerNumber owner, long max_lv);
TbBool create_hero_special_worker(long x, long y, PlayerNumber owner);

void destroy_food(struct Thing *thing);
unsigned char active_battle_exists(unsigned char a1);
void maintain_my_battle_list(void);
unsigned char step_battles_forward(unsigned char a1);
void process_person_moods_and_needs(struct Thing *thing);
long process_obey_leader(struct Thing *thing);
void tag_cursor_blocks_dig(unsigned char a1, long a2, long a3, long a4);
void tag_cursor_blocks_thing_in_hand(unsigned char a1, long a2, long a3, int a4, long a5);
void draw_spell_cursor(unsigned char a1, unsigned short a2, unsigned char stl_x, unsigned char stl_y);
short make_group_member_leader(struct Thing *leadtng);
short zoom_to_next_annoyed_creature(void);
short zoom_to_fight(unsigned char a1);
void go_to_my_next_room_of_type(unsigned long rkind);

short ceiling_set_info(long height_max, long height_min, long step);
void view_set_camera_y_inertia(struct Camera *cam, long a2, long a3);
void view_set_camera_x_inertia(struct Camera *cam, long a2, long a3);
void view_set_camera_rotation_inertia(struct Camera *cam, long a2, long a3);
void view_zoom_camera_in(struct Camera *cam, long a2, long a3);
void set_camera_zoom(struct Camera *cam, long val);
void view_zoom_camera_out(struct Camera *cam, long a2, long a3);
long get_camera_zoom(struct Camera *camera);
void update_camera_zoom_bounds(struct Camera *cam,unsigned long zoom_max,unsigned long zoom_min);
void keep_local_camera_zoom_level(unsigned long prev_units_per_pixel_size);
void set_mouse_light(struct PlayerInfo *player);
void clear_slab_dig(long a1, long a2, char a3);
void delete_all_structures(void);
void clear_map(void);
void clear_game(void);
void clear_game_for_save(void);
void clear_complete_game(void);
void clear_things_and_persons_data(void);
void clear_rooms(void);
void clear_computer(void);
TbBool swap_creature(long ncrt_id, long crtr_id);
void place_animating_slab_type_on_map(long a1, char a2, unsigned char a3, unsigned char a4, unsigned char a5);
void place_slab_type_on_map(SlabType nslab, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned char owner, unsigned char a5);
void do_slab_efficiency_alteration(unsigned char a1, unsigned char a2);
void update_explored_flags_for_power_sight(struct PlayerInfo *player);
void engine(struct Camera *cam);
void smooth_screen_area(unsigned char *a1, long a2, long a3, long a4, long a5, long a6);
void remove_explored_flags_for_power_sight(struct PlayerInfo *player);
void DrawBigSprite(long x, long y, struct BigSprite *bigspr, struct TbSprite *sprite);
void draw_gold_total(unsigned char a1, long a2, long a3, long a4);
void pannel_map_draw(long x, long y, long zoom);
void draw_overlay_things(long zoom);
void draw_overlay_compass(long a1, long a2);
void draw_mini_things_in_hand(long x, long y);
TbBool screen_to_map(struct Camera *camera, long screen_x, long screen_y, struct Coord3d *mappos);
struct Thing *find_base_thing_on_mapwho(unsigned char oclass, unsigned short okind, unsigned short x, unsigned short y);
TbBool mouse_is_over_small_map(long x, long y);
void do_map_rotate_stuff(long a1, long a2, long *a3, long *a4, long a5);
void update_breed_activities(void);
void set_level_objective(char *msg_text);
void find_map_location_coords(long location, long *x, long *y, const char *func_name);
TbBool any_player_close_enough_to_see(struct Coord3d *pos);
unsigned char line_of_sight_3d(const struct Coord3d *pos1, const struct Coord3d *pos2);
long can_thing_be_picked_up_by_player(const struct Thing *thing, long plyr_idx);
long can_thing_be_picked_up2_by_player(const struct Thing *thing, long plyr_idx);
void affect_nearby_enemy_creatures_with_wind(struct Thing *thing);
void affect_nearby_stuff_with_vortex(struct Thing *thing);
void affect_nearby_friends_with_alarm(struct Thing *thing);
long apply_wallhug_force_to_boulder(struct Thing *thing);
void lightning_modify_palette(struct Thing *thing);
void god_lightning_choose_next_creature(struct Thing *thing);
void draw_god_lightning(struct Thing *thing);
unsigned long lightning_is_close_to_player(struct PlayerInfo *player, struct Coord3d *pos);
void update_god_lightning_ball(struct Thing *thing);

unsigned long seed_check_random(unsigned long range, unsigned long *seed, const char *func_name, unsigned long place);
void setup_heap_manager(void);
TbBool setup_heap_memory(void);
void reset_heap_manager(void);
void reset_heap_memory(void);
TbBool load_settings(void);
unsigned long convert_td_iso(unsigned long n);
void reset_player_mode(struct PlayerInfo *player, unsigned short nmode);
void init_keeper_map_exploration(struct PlayerInfo *player);
void init_player_cameras(struct PlayerInfo *player);
void init_player(struct PlayerInfo *player, short no_explore);
void init_lookups(void);
void pannel_map_update(long x, long y, long w, long h);
void place_single_slab_type_on_map(long a1, unsigned char a2, unsigned char a3, unsigned char a4);
void shuffle_unattached_things_on_slab(long a1, long a2);
unsigned char alter_rock_style(unsigned char a1, signed char a2, signed char a3, unsigned char a4);
short play_smacker_file(char *filename, int nstate);
void turn_off_query(short a);
TbBool set_gamma(char corrlvl, TbBool do_set);
void level_lost_go_first_person(long plridx);
long battle_move_player_towards_battle(struct PlayerInfo *player, long var);
short winning_player_quitting(struct PlayerInfo *player, long *plyr_count);
TbBool move_campaign_to_next_level(void);
TbBool move_campaign_to_prev_level(void);
short lose_level(struct PlayerInfo *player);
short resign_level(struct PlayerInfo *player);
short complete_level(struct PlayerInfo *player);
void directly_cast_spell_on_thing(long plridx, unsigned char a2, unsigned short a3, long a4);
void set_general_information(long msg_id, long target, long x, long y);
void set_quick_information(long msg_id, long target, long x, long y);
void process_objective(char *msg_text, long target, long x, long y);
void set_general_objective(long msg_id, long target, long x, long y);
struct Thing *event_is_attached_to_thing(long ev_idx);
void maintain_my_event_list(struct Dungeon *dungeon);
void kill_oldest_my_event(struct Dungeon *dungeon);
void turn_off_sight_of_evil(long plridx);
short dump_held_things_on_map(unsigned int plyridx, long a2, long a3, short a4);
void set_player_mode(struct PlayerInfo *player, long val);
long set_autopilot_type(unsigned int plridx, long aptype);
void set_player_state(struct PlayerInfo *player, short a1, long a2);
long place_thing_in_power_hand(struct Thing *thing, long var);
void turn_off_call_to_arms(long a);
void instant_instance_selected(long a1);
void centre_engine_window(void);
void change_engine_window_relative_size(long w_delta, long h_delta);
void message_add(char c);
void init_messages(void);
void battle_initialise(void);
struct Thing *create_ambient_sound(struct Coord3d *pos, unsigned short a2, unsigned short owner);
long take_money_from_dungeon(PlayerNumber plyr_idx, long amount, unsigned char a3);
void update_thing_animation(struct Thing *thing);
long update_cave_in(struct Thing *thing);
void move_thing_in_map(struct Thing *thing, struct Coord3d *pos);
long get_floor_height_under_thing_at(struct Thing *thing, struct Coord3d *pos);
long slabs_count_near(long tx,long ty,long rad,unsigned short slbtype);
void initialise_map_collides(void);
void initialise_map_health(void);
void init_creature_scores(void);
void setup_3d(void);
void setup_stuff(void);
long ceiling_init(unsigned long a1, unsigned long a2);
void process_dungeon_destroy(struct Thing *thing);
void apply_damage_to_thing_and_display_health(struct Thing *thing, long a1, char a2);
void give_shooter_drained_health(struct Thing *shooter, long health_delta);
long get_foot_creature_has_down(struct Thing *thing);
void process_disease(struct Thing *thing);
void process_keeper_spell_effect(struct Thing *thing);
void leader_find_positions_for_followers(struct Thing *thing);
void explosion_affecting_area(struct Thing *tngsrc, const struct Coord3d *pos,
      long range, long max_damage, unsigned char hit_type);
long process_creature_self_spell_casting(struct Thing *thing);
struct Thing *create_thing(struct Coord3d *pos, unsigned short tngclass, unsigned short model, unsigned short owner, long a4);
struct Thing *create_gold_for_hand_grab(struct Thing *thing, long a2);
long remove_food_from_food_room_if_possible(struct Thing *thing);
unsigned long setup_move_off_lava(struct Thing *thing);
struct Room *player_has_room_of_type(long plr_idx, long roomkind);
void set_thing_draw(struct Thing *thing, long a2, long a3, long a4, char a5, char a6, unsigned char a7);
long get_next_manufacture(struct Dungeon *dungeon);
void remove_thing_from_mapwho(struct Thing *thing);
void place_thing_in_mapwho(struct Thing *thing);
long get_thing_height_at(struct Thing *thing, struct Coord3d *pos);
unsigned long can_drop_thing_here(long x, long y, long a3, unsigned long a4);
short can_dig_here(long x, long y, long owner);
long thing_in_wall_at(struct Thing *thing, struct Coord3d *pos);
short can_place_thing_here(struct Thing *thing, long x, long y, long dngn_idx);
short do_left_map_drag(long begin_x, long begin_y, long curr_x, long curr_y, long zoom);
short do_left_map_click(long begin_x, long begin_y, long curr_x, long curr_y, long zoom);
short do_right_map_click(long start_x, long start_y, long curr_x, long curr_y, long zoom);

TbPixel get_player_path_colour(unsigned short owner);
long get_scavenge_effect_element(unsigned short owner);

long update_dead_creature(struct Thing *thing);

void startup_network_game(TbBool local);
void startup_saved_packet_game(void);
void faststartup_saved_packet_game(void);
void reinit_level_after_load(void);
long get_radially_decaying_value(long magnitude,long decay_start,long decay_length,long distance);

#ifdef __cplusplus
}
#endif
#endif // DK_KEEPERFX_H
