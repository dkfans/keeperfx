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

#ifdef FUNCTESTING
#include "ftests/ftest.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Max length of the command line
#define CMDLN_MAXLEN 259

#define LEGAL_WIDTH  640
#define LEGAL_HEIGHT 480

#define LENSES_COUNT           15
#define SPELL_POINTER_GROUPS   14
#define ZOOM_KEY_ROOMS_COUNT   15

#define CMDLINE_OVERRIDES      4

/** Command Line overrides for config settings. Checked after the config file is loaded. */
enum CmdLineOverrides {
    Clo_ConfigFile = 0, /**< Special: handled before the config file is loaded. */
    Clo_CDMusic,
    Clo_GameTurns,
    Clo_FramesPerSecond,
};

enum ModeFlags {
    MFlg_IsDemoMode         =  0x01,
    MFlg_EyeLensReady       =  0x02,
    MFlg_Unusedparam04      =  0x04,
    MFlg_DeadBackToPool     =  0x08,
    MFlg_NoCdMusic          =  0x10, // unused
    MFlg_Unusedparam20      =  0x20,
    MFlg_DemoMode           =  0x40,
    MFlg_NoHeroHealthFlower =  0x80,
};

enum DebugFlags {
    DFlg_ShotsDamage        =  0x01,
    DFlg_CreatrPaths        =  0x02,
    DFlg_ShowGameTurns      =  0x04,
    DFlg_FrameStep          =  0x08,
    DFlg_PauseAtGameTurn    =  0x10,
};

#ifdef FUNCTESTING
enum FunctestFlags {
    FTF_Enabled             = 0x01, // Functional Tests Enabled
    FTF_TestFailed          = 0x02, // Test failed, causes exit code -1 for cmd line automation
    FTF_Abort               = 0x04, // Something went wrong, aborting
    FTF_LevelLoaded         = 0x08, // For tracking if map is ready
    FTF_ExitOnTestFailure   = 0x10, // If users want to exit on any test failure
    FTF_IncludeLongTests    = 0x20, // If users want to run the long running test list
};
#endif

#pragma pack(1)

struct TbLoadFiles;

// Windows-standard structure
/*struct _GUID {
     unsigned long Data1;
     unsigned short Data2;
     unsigned short Data3;
     unsigned char Data4[8];
};*/

struct StartupParameters {
    LevelNumber selected_level_number;
    TbBool no_intro;
    TbBool one_player;
    TbBool easter_egg;
    TbBool ignore_mods;
    unsigned char operation_flags;
    unsigned char flags_font;
    unsigned char mode_flags;
    unsigned char debug_flags;
    unsigned short computer_chat_flags;
    long num_fps;
    long num_fps_draw_main; // -1 if auto
    long num_fps_draw_secondary;
    TbBool packet_save_enable;
    TbBool packet_load_enable;
    char packet_fname[150];
    unsigned char packet_checksum_verify;
    int frame_skip;
    char selected_campaign[CMDLN_MAXLEN+1];
    TbBool overrides[CMDLINE_OVERRIDES];
    char config_file[CMDLN_MAXLEN+1];
    GameTurn pause_at_gameturn;
    unsigned char startup_flags;
#ifdef FUNCTESTING
    unsigned char functest_flags;
    char functest_name[FTEST_MAX_NAME_LENGTH];
    unsigned int functest_seed;
#endif
};

// Global variables migration between DLL and the program

extern unsigned char *blue_palette;
extern unsigned char *red_palette;
extern unsigned char *dog_palette;
extern unsigned char *vampire_palette;
extern unsigned char* engine_palette;
extern unsigned char exit_keeper;
extern unsigned char quit_game;
extern unsigned char is_running_under_wine;
extern int continue_game_option_available;
extern long last_mouse_x;
extern long last_mouse_y;
extern int FatalError;
extern long define_key_scroll_offset;
extern unsigned long time_last_played_demo;
extern short drag_menu_x;
extern short drag_menu_y;
extern unsigned short tool_tip_time;
extern unsigned short help_tip_time;
extern long pointer_x;
extern long pointer_y;
extern long block_pointed_at_x;
extern long block_pointed_at_y;
extern long pointed_at_frac_x;
extern long pointed_at_frac_y;
extern long top_pointed_at_x;
extern long top_pointed_at_y;
extern long top_pointed_at_frac_x;
extern long top_pointed_at_frac_y;
extern char level_name[88];
extern char top_of_breed_list;
/** Amount of different creature kinds the local player has. Used for creatures tab in panel menu. */
extern char no_of_breeds_owned;
extern long optimised_lights;
extern long total_lights;
extern unsigned char do_lights;
extern struct Thing *thing_pointed_at;
extern struct Map *me_pointed_at;
extern long my_mouse_x;
extern long my_mouse_y;
extern char *level_names_data;
extern char *end_level_names_data;
extern unsigned char *frontend_backup_palette;
extern unsigned char zoom_to_heart_palette[768];
extern unsigned char EngineSpriteDrawUsingAlpha;
extern unsigned char temp_pal[768];
extern unsigned char *lightning_palette;

#pragma pack()
/******************************************************************************/
// Variables inside the main module
extern short default_loc_player;
extern struct GuiBox *gui_cheat_box_1;
extern struct GuiBox *gui_cheat_box_2;
extern struct GuiBox *gui_cheat_box_3;
extern struct StartupParameters start_params;

//Functions - reworked
short setup_game(void);
void game_loop(void);
short reset_game(void);
void update(void);

TbBool can_thing_be_queried(struct Thing *thing, PlayerNumber plyr_idx);
struct Thing *get_queryable_object_near(MapCoord pos_x, MapCoord pos_y, long plyr_idx);
long packet_place_door(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel dormodel, TbBool allowed);
TbBool all_dungeons_destroyed(const struct PlayerInfo *win_player);
void reset_gui_based_on_player_mode(void);
void reinit_tagged_blocks_for_player(PlayerNumber plyr_idx);
void draw_flame_breath(struct Coord3d *pos1, struct Coord3d *pos2, long delta_step, long num_per_step, short ef_or_efel_model, ThingIndex parent_idx);
void draw_lightning(const struct Coord3d* pos1, const struct Coord3d* pos2, long eeinterspace, short ef_or_efel_model);
void toggle_hero_health_flowers(void);
void check_players_won(void);
void check_players_lost(void);
void process_things_in_dungeon_hand(void);
void process_payday(void);

TbBool toggle_computer_player(PlayerNumber plyr_idx);
void PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *pal);
void set_player_cameras_position(struct PlayerInfo *player, long pos_x, long pos_y);
void init_player_types();
void init_keepers_map_exploration(void);
void clear_creature_pool(void);
void reset_creature_max_levels(void);
void reset_script_timers_and_flags(void);
void add_creature_to_pool(ThingModel kind, long amount);
void draw_texture(long a1, long a2, long a3, long a4, long a5, long a6, long a7);

short zoom_to_next_annoyed_creature(void);

TbBool LbIsFrozenOrPaused(void); // from bflib_inputctrl.cpp

void set_mouse_light(struct PlayerInfo *player);
void delete_all_structures(void);
void clear_map(void);
void clear_game(void);
void clear_game_for_save(void);
void clear_complete_game(void);
void clear_things_and_persons_data(void);
void clear_computer(void);
void engine(struct PlayerInfo *player, struct Camera *cam);
void draw_gold_total(PlayerNumber plyr_idx, long scr_x, long scr_y, long units_per_px, long long value);
void draw_mini_things_in_hand(long x, long y);
TbBool screen_to_map(struct Camera *camera, long screen_x, long screen_y, struct Coord3d *mappos);
void update_creatr_model_activities_list(TbBool forced);
TbBool any_player_close_enough_to_see(const struct Coord3d *pos);
void affect_nearby_stuff_with_vortex(struct Thing *thing);
void affect_nearby_friends_with_alarm(struct Thing *thing);
long apply_wallhug_force_to_boulder(struct Thing *thing);
long process_boulder_collision(struct Thing *boulder, struct Coord3d *pos, int direction_x, int direction_y);
void lightning_modify_palette(struct Thing *thing);
unsigned long lightning_is_close_to_player(struct PlayerInfo *player, struct Coord3d *pos);

unsigned long seed_check_random(unsigned long range, uint32_t *seed, const char *func_name, unsigned long place);
void init_lookups(void);
void place_single_slab_type_on_map(SlabKind slbkind, MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx);
void turn_off_query(PlayerNumber plyr_idx);
TbBool set_gamma(char corrlvl, TbBool do_set);
void level_lost_go_first_person(PlayerNumber plyr_idx);
short winning_player_quitting(struct PlayerInfo *player, int32_t *plyr_count);
short lose_level(struct PlayerInfo *player);
short resign_level(struct PlayerInfo *player);
short complete_level(struct PlayerInfo *player);
void set_general_information(long msg_id, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y);
void set_quick_information(long msg_id, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y);
void process_objective(const char *msg_text, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y);
void set_general_objective(long msg_id, TbMapLocation target, long x, long y);
void turn_off_power_sight_of_evil(PlayerNumber plridx);
void turn_off_power_obey(PlayerNumber plyr_idx);

short dump_first_held_thing_on_map(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, TbBool update_hand);
int dump_all_held_things_on_map(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
void dump_thing_held_by_any_player(struct Thing *thing);

void instant_instance_selected(CrInstance check_inst_id);
void centre_engine_window(void);
void change_engine_window_relative_size(long w_delta, long h_delta);
void update_thing_animation(struct Thing *thing);
long update_cave_in(struct Thing *thing);
void initialise_map_collides(void);
void initialise_map_health(void);
void setup_mesh_randomizers(void);
void setup_stuff(void);
void give_shooter_drained_health(struct Thing *shooter, HitPoints health_delta);
long get_foot_creature_has_down(struct Thing *thing);
void process_keeper_spell_aura(struct Thing *thing);
void init_seeds();


TbPixel get_player_path_colour(unsigned short owner);

void startup_saved_packet_game(void);
void faststartup_saved_packet_game(void);
void reinit_level_after_load(void);
void redetect_screen_refresh_rate_for_draw();
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

struct GameTime get_game_time(unsigned long turns, unsigned long fps);

#ifdef __cplusplus
}
#endif
#endif // DK_KEEPERFX_H
