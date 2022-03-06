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
#include "bflib_filelst.h"
#include "bflib_sprite.h"
#include "packets.h"
#include "thing_data.h"
#include "thing_list.h"
#include "thing_doors.h"
#include "dungeon_data.h"
#include "player_computer.h"
#include "game_merge.h"
#include "game_saves.h"
#include "map_events.h"
#include "game_lghtshdw.h"
#include "thing_creature.h"
#include "creature_control.h"
#include "creature_battle.h"
#include "player_data.h"
#include "slab_data.h"
#include "light_data.h"
#include "actionpt.h"
#include "map_data.h"
#include "map_columns.h"
#include "room_data.h"
#include "config.h"
#include "config_magic.h"
#include "net_game.h"
#include "sounds.h"

#ifdef __cplusplus
extern "C" {
#endif

// Max length of the command line
#define CMDLN_MAXLEN 259

#define LEGAL_WIDTH  640
#define LEGAL_HEIGHT 480

#define LENSES_COUNT           15
#define MINMAXS_COUNT          64
#define SPELL_POINTER_GROUPS   14
// Amount of instances; it's 17, 18 or 19
#define PLAYER_INSTANCES_COUNT 19
#define ZOOM_KEY_ROOMS_COUNT   14

enum ModeFlags {
    MFlg_IsDemoMode         =  0x01,
    MFlg_EyeLensReady       =  0x02,
    MFlg_unk04              =  0x04,
    MFlg_DeadBackToPool     =  0x08,
    MFlg_NoCdMusic          =  0x10,
    MFlg_unk20              =  0x20,
    MFlg_unk40              =  0x40,
    MFlg_NoHeroHealthFlower              =  0x80,
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
    DFlg_CreatrPaths        =  0x02,
};

#ifdef AUTOTESTING
enum AutotestFlags {
    ATF_ExitOnTurn          = 0x01, // Exit from a game after some time
    ATF_FixedSeed           = 0x02, // Set randomseed to 1 on game start
    ATF_AI_Player           = 0x04, // Activate Ai player on level start
    ATF_TestsCampaign       = 0x08  // Switch to testing levels
};
#endif

#pragma pack(1)

struct TbLoadFiles;
struct RoomFlag;
struct Number;
struct JontySpr;

// Windows-standard structure
/*struct _GUID {
     unsigned long Data1;
     unsigned short Data2;
     unsigned short Data3;
     unsigned char Data4[8];
};*/

struct StartupParameters {
    LevelNumber selected_level_number;
    unsigned char no_intro;
    unsigned char one_player;
    unsigned char operation_flags;
    unsigned char flags_font;
    unsigned char flags_cd;
    unsigned char debug_flags;
    unsigned short computer_chat_flags;
    long num_fps;
    unsigned char packet_save_enable;
    unsigned char packet_load_enable;
    char packet_fname[150];
    unsigned char packet_checksum_verify;
    unsigned char force_ppro_poly;
    int frame_skip;
    char selected_campaign[CMDLN_MAXLEN+1];
#ifdef AUTOTESTING
    unsigned char autotest_flags;
    unsigned long autotest_exit_turn;
#endif
};

// Global variables migration between DLL and the program

DLLIMPORT extern unsigned char *_DK_blue_palette;
#define blue_palette _DK_blue_palette
DLLIMPORT extern unsigned char *_DK_red_palette;
#define red_palette _DK_red_palette
DLLIMPORT extern unsigned char *_DK_dog_palette;
#define dog_palette _DK_dog_palette
DLLIMPORT extern unsigned char *_DK_vampire_palette;
#define vampire_palette _DK_vampire_palette
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
DLLIMPORT extern int _DK_FatalError;
#define FatalError _DK_FatalError
DLLIMPORT extern long _DK_define_key_scroll_offset;
#define define_key_scroll_offset _DK_define_key_scroll_offset
DLLIMPORT extern unsigned long _DK_time_last_played_demo;
#define time_last_played_demo _DK_time_last_played_demo
DLLIMPORT extern short _DK_drag_menu_x;
#define drag_menu_x _DK_drag_menu_x
DLLIMPORT extern short _DK_drag_menu_y;
#define drag_menu_y _DK_drag_menu_y
DLLIMPORT extern unsigned short _DK_tool_tip_time;
#define tool_tip_time _DK_tool_tip_time
DLLIMPORT extern unsigned short _DK_help_tip_time;
#define help_tip_time _DK_help_tip_time
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
DLLIMPORT long _DK_frame_number;
#define frame_number _DK_frame_number
DLLIMPORT long _DK_draw_spell_cost;
#define draw_spell_cost _DK_draw_spell_cost
DLLIMPORT char _DK_level_name[88];
#define level_name _DK_level_name
DLLIMPORT char _DK_top_of_breed_list;
#define top_of_breed_list _DK_top_of_breed_list
/** Amount of different creature kinds the local player has. Used for creatures tab in panel menu. */
DLLIMPORT char _DK_no_of_breeds_owned;
#define no_of_breeds_owned _DK_no_of_breeds_owned
DLLIMPORT long _DK_optimised_lights;
#define optimised_lights _DK_optimised_lights
DLLIMPORT long _DK_total_lights;
#define total_lights _DK_total_lights
DLLIMPORT unsigned char _DK_do_lights;
#define do_lights _DK_do_lights
DLLIMPORT struct Thing *_DK_thing_pointed_at;
#define thing_pointed_at _DK_thing_pointed_at
DLLIMPORT struct Map *_DK_me_pointed_at;
#define me_pointed_at _DK_me_pointed_at
DLLIMPORT long _DK_my_mouse_x;
#define my_mouse_x _DK_my_mouse_x
DLLIMPORT long _DK_my_mouse_y;
#define my_mouse_y _DK_my_mouse_y
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
DLLIMPORT unsigned char _DK_zoom_to_heart_palette[768];
#define zoom_to_heart_palette _DK_zoom_to_heart_palette
DLLIMPORT unsigned char _DK_EngineSpriteDrawUsingAlpha;
#define EngineSpriteDrawUsingAlpha _DK_EngineSpriteDrawUsingAlpha
DLLIMPORT unsigned char _DK_temp_pal[768];
#define temp_pal _DK_temp_pal
DLLIMPORT unsigned char *_DK_lightning_palette;
#define lightning_palette _DK_lightning_palette

#pragma pack()
/******************************************************************************/
// Variables inside the main module
extern TbClockMSec last_loop_time;
extern short default_loc_player;
extern struct GuiBox *gui_box;
extern struct GuiBox *gui_cheat_box;
extern int test_variable;
extern struct StartupParameters start_params;

//Functions - reworked
short setup_game(void);
void game_loop(void);
short reset_game(void);
void update(void);

TbBool can_thing_be_queried(struct Thing *thing, PlayerNumber plyr_idx);
struct Thing *get_queryable_object_near(MapCoord pos_x, MapCoord pos_y, long plyr_idx);
TbBool tag_cursor_blocks_sell_area(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long full_slab);
long packet_place_door(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel dormodel, unsigned char a5);
TbBool tag_cursor_blocks_place_room(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long full_slab);
TbBool tag_cursor_blocks_place_door(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool all_dungeons_destroyed(const struct PlayerInfo *win_player);
void reset_gui_based_on_player_mode(void);
void reinit_tagged_blocks_for_player(PlayerNumber plyr_idx);
void draw_flame_breath(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4);
void draw_lightning(const struct Coord3d* pos1, const struct Coord3d* pos2, long eeinterspace, long eemodel);
void toggle_hero_health_flowers(void);
void check_players_won(void);
void check_players_lost(void);
void process_things_in_dungeon_hand(void);
void process_payday(void);

TbBool toggle_computer_player(PlayerNumber plyr_idx);
void PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *pal);
void set_player_cameras_position(struct PlayerInfo *player, long pos_x, long pos_y);
void init_good_player_as(PlayerNumber plr_idx);
void init_keepers_map_exploration(void);
void clear_creature_pool(void);
void reset_creature_max_levels(void);
void reset_script_timers_and_flags(void);
void add_creature_to_pool(long kind, long amount, unsigned long a3);
void draw_texture(long a1, long a2, long a3, long a4, long a5, long a6, long a7);

void tag_cursor_blocks_dig(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long full_slab);
void tag_cursor_blocks_thing_in_hand(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, int is_special_digger, long full_slab);
short zoom_to_next_annoyed_creature(void);

TbBool LbIsFrozenOrPaused(void); // from bflib_inputctrl.cpp

short ceiling_set_info(long height_max, long height_min, long step);
void set_mouse_light(struct PlayerInfo *player);
void delete_all_structures(void);
void clear_map(void);
void clear_game(void);
void clear_game_for_save(void);
void clear_complete_game(void);
void clear_things_and_persons_data(void);
void clear_computer(void);
TbBool swap_creature(long ncrt_id, long crtr_id);
void engine(struct PlayerInfo *player, struct Camera *cam);
void draw_gold_total(PlayerNumber plyr_idx, long scr_x, long scr_y, long units_per_px, long long value);
void draw_mini_things_in_hand(long x, long y);
TbBool screen_to_map(struct Camera *camera, long screen_x, long screen_y, struct Coord3d *mappos);
void update_creatr_model_activities_list(void);
void find_map_location_coords(long location, long *x, long *y, int plyr_idx, const char *func_name);
TbBool any_player_close_enough_to_see(const struct Coord3d *pos);
void affect_nearby_enemy_creatures_with_wind(struct Thing *thing);
void affect_nearby_stuff_with_vortex(struct Thing *thing);
void affect_nearby_friends_with_alarm(struct Thing *thing);
long apply_wallhug_force_to_boulder(struct Thing *thing);
void lightning_modify_palette(struct Thing *thing);
unsigned long lightning_is_close_to_player(struct PlayerInfo *player, struct Coord3d *pos);

unsigned long seed_check_random(unsigned long range, unsigned long *seed, const char *func_name, unsigned long place);
void init_lookups(void);
void place_single_slab_type_on_map(SlabKind slbkind, MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx);
void shuffle_unattached_things_on_slab(long a1, long a2);
void turn_off_query(PlayerNumber plyr_idx);
TbBool set_gamma(char corrlvl, TbBool do_set);
void level_lost_go_first_person(PlayerNumber plyr_idx);
short winning_player_quitting(struct PlayerInfo *player, long *plyr_count);
short lose_level(struct PlayerInfo *player);
short resign_level(struct PlayerInfo *player);
short complete_level(struct PlayerInfo *player);
void set_general_information(long msg_id, long target, long x, long y);
void set_quick_information(long msg_id, long target, long x, long y);
void process_objective(const char *msg_text, long target, long x, long y);
void set_general_objective(long msg_id, long target, long x, long y);
void turn_off_power_sight_of_evil(PlayerNumber plridx);
void turn_off_power_obey(PlayerNumber plyr_idx);

short dump_first_held_thing_on_map(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool update_hand);
int dump_all_held_things_on_map(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void dump_thing_held_by_any_player(struct Thing *thing);

void instant_instance_selected(CrInstance check_inst_id);
void centre_engine_window(void);
void change_engine_window_relative_size(long w_delta, long h_delta);
void init_messages(void);
void update_thing_animation(struct Thing *thing);
long update_cave_in(struct Thing *thing);
void initialise_map_collides(void);
void initialise_map_health(void);
void setup_3d(void);
void setup_stuff(void);
long ceiling_init(unsigned long a1, unsigned long a2);
void give_shooter_drained_health(struct Thing *shooter, long health_delta);
long get_foot_creature_has_down(struct Thing *thing);
void process_keeper_spell_effect(struct Thing *thing);

TbPixel get_player_path_colour(unsigned short owner);

void startup_saved_packet_game(void);
void faststartup_saved_packet_game(void);
void reinit_level_after_load(void);
void update_time(void);
extern TbClockMSec timerstarttime;
struct TimerTime {
        unsigned char Hours;
        unsigned char Minutes;
        unsigned char Seconds;
        unsigned short MSeconds;
};
extern struct TimerTime Timer;
extern TbBool TimerGame;
extern TbBool TimerNoReset;
extern TbBool TimerFreeze;
struct GameTime {
    unsigned char Seconds;
    unsigned char Minutes;
    unsigned char Hours;
};

__attribute__((regparm(3))) struct GameTime get_game_time(unsigned long turns, unsigned long fps);

#ifdef __cplusplus
}
#endif
#endif // DK_KEEPERFX_H
