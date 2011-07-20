
#include <windows.h>
#include <winbase.h>
#include <math.h>
#include <string>
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include "keeperfx.hpp"

#include "bflib_math.h"
#include "bflib_memory.h"
#include "bflib_heapmgr.h"
#include "bflib_keybrd.h"
#include "bflib_inputctrl.h"
#include "bflib_datetm.h"
#include "bflib_bufrw.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_sndlib.h"
#include "bflib_fmvids.h"
#include "bflib_cpu.h"
#include "bflib_crash.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "bflib_guibtns.h"
#include "bflib_sound.h"
#include "bflib_mouse.h"
#include "bflib_filelst.h"
#include "bflib_network.h"

#include "front_simple.h"
#include "frontend.h"
#include "front_input.h"
#include "gui_draw.h"
#include "gui_tooltips.h"
#include "gui_parchment.h"
#include "gui_frontmenu.h"
#include "scrcapt.h"
#include "vidmode.h"
#include "kjm_input.h"
#include "packets.h"
#include "config.h"
#include "config_campaigns.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "config_rules.h"
#include "config_lenses.h"
#include "config_magic.hpp"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "config_crtrmodel.h"
#include "lvl_script.h"
#include "lvl_filesdk1.h"
#include "thing_list.h"
#include "player_instances.h"
#include "game_saves.h"
#include "engine_render.h"
#include "engine_lenses.h"
#include "engine_camera.h"
#include "engine_arrays.h"
#include "engine_redraw.h"
#include "front_landview.h"
#include "thing_stats.h"
#include "thing_creature.h"
#include "thing_corpses.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_doors.h"
#include "thing_traps.h"
#include "thing_shots.h"
#include "slab_data.h"
#include "room_data.h"
#include "map_columns.h"
#include "map_events.h"
#include "map_utils.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "creature_graphics.h"
#include "creature_states_rsrch.h"
#include "lens_api.h"
#include "light_data.h"
#include "magic.h"
#include "power_hand.h"
#include "game_merge.h"
#include "gui_topmsg.h"
#include "gui_boxmenu.h"
#include "gui_soundmsgs.h"
#include "frontmenu_ingame_tabs.h"
#include "ariadne.h"
#include "net_game.h"
#include "sounds.h"
#include "vidfade.h"
#include "KeeperSpeech.h"

int test_variable;

// Max length of the command line
#define CMDLN_MAXLEN 259
char cmndline[CMDLN_MAXLEN+1];
unsigned short bf_argc;
char *bf_argv[CMDLN_MAXLEN+1];

struct KeyToStringInit key_to_string_init[] = {
  {KC_A,  -65},
  {KC_B,  -66},
  {KC_C,  -67},
  {KC_D,  -68},
  {KC_E,  -69},
  {KC_F,  -70},
  {KC_G,  -71},
  {KC_H,  -72},
  {KC_I,  -73},
  {KC_J,  -74},
  {KC_K,  -75},
  {KC_L,  -76},
  {KC_M,  -77},
  {KC_N,  -78},
  {KC_O,  -79},
  {KC_P,  -80},
  {KC_Q,  -81},
  {KC_R,  -82},
  {KC_S,  -83},
  {KC_T,  -84},
  {KC_U,  -85},
  {KC_V,  -86},
  {KC_W,  -87},
  {KC_X,  -88},
  {KC_Y,  -89},
  {KC_Z,  -90},
  {KC_F1,  515},
  {KC_F2,  516},
  {KC_F3,  517},
  {KC_F4,  518},
  {KC_F5,  519},
  {KC_F6,  520},
  {KC_F7,  521},
  {KC_F8,  522},
  {KC_F9,  523},
  {KC_F10, 524},
  {KC_F11, 525},
  {KC_F12, 526},
  {KC_CAPITAL, 490},
  {KC_LSHIFT,  483},
  {KC_RSHIFT,  484},
  {KC_LCONTROL, 481},
  {KC_RCONTROL, 482},
  {KC_RETURN,  488},
  {KC_BACK,    491},
  {KC_INSERT,  492},
  {KC_DELETE,  493},
  {KC_HOME,    494},
  {KC_END,     495},
  {KC_PGUP,    496},
  {KC_PGDOWN,  497},
  {KC_NUMLOCK, 498},
  {KC_DIVIDE,  499},
  {KC_MULTIPLY, 500},
  {KC_NUMPADENTER, 503},
  {KC_DECIMAL, 504},
  {KC_NUMPAD0, 514},
  {KC_NUMPAD1, 505},
  {KC_NUMPAD2, 506},
  {KC_NUMPAD3, 507},
  {KC_NUMPAD4, 508},
  {KC_NUMPAD5, 509},
  {KC_NUMPAD6, 510},
  {KC_NUMPAD7, 511},
  {KC_NUMPAD8, 512},
  {KC_NUMPAD9, 513},
  {KC_UP,     527},
  {KC_DOWN,   528},
  {KC_LEFT,   529},
  {KC_RIGHT,  530},
  {  0,     0},
};

long const scavenge_effect_element[] = {60, 61, 62, 63, 64, 64,};

struct KeycodeString eastegg_feckoff_codes = {
    {KC_F,KC_E,KC_C,KC_K,KC_O,KC_F,KC_F,KC_UNASSIGNED}, 7,
};
struct KeycodeString eastegg_jlw_codes = {
    {KC_J,KC_L,KC_W,KC_UNASSIGNED}, 3,
};
struct KeycodeString eastegg_skeksis_codes = {
    {KC_S,KC_K,KC_E,KC_K,KC_S,KC_I,KC_S,KC_UNASSIGNED}, 7,
};

const char *sound_fname = "sound.dat";
const char *speech_fname = "speech.dat";

short default_loc_player = 0;
unsigned long gold_per_hoarde = 2000;
struct StartupParameters start_params;

//long const imp_spangle_effects[] = {

const struct Around around[] = {
  {-1,-1},
  {-1, 0},
  {-1, 1},
  { 0,-1},
  { 0, 0},
  { 0, 1},
  { 1,-1},
  { 1, 0},
  { 1, 1},
  { 0, 0}, // this entry shouldn't be used
};

unsigned short const player_state_to_spell[] = {
  0, 0, 0,  0,  0,  0, 6, 7, 5, 0, 18, 18, 0, 0, 0, 0,
  0,10, 0, 11, 12, 13, 8, 0, 2,16, 14, 15, 0, 3, 0, 0,
};

unsigned char const attract_score[CREATURE_TYPES_COUNT] = {
    0, 12, 11, 6, 7, 7, 14, 16,     //0
    6, 6, 12, 5, 6, 13, 14, 10,     //8
    11, 13, 10, 3, 14, 12, 13, 2,   //16
    6, 14, 8, 10, 5, 10, 10, 0      //24
};

//static
TbClockMSec last_loop_time=0;

#ifdef __cplusplus
extern "C" {
#endif
DLLIMPORT void _DK_draw_flame_breath(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4);
DLLIMPORT void _DK_draw_lightning(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4);
DLLIMPORT unsigned char _DK_line_of_sight_3d(const struct Coord3d *pos1, const struct Coord3d *pos2);
DLLIMPORT int _DK_can_thing_be_picked_up_by_player(const struct Thing *thing, unsigned char plyr_idx);
DLLIMPORT int _DK_can_thing_be_picked_up2_by_player(const struct Thing *thing, unsigned char plyr_idx);
DLLIMPORT void _DK_init_alpha_table(void);
DLLIMPORT void _DK_external_activate_trap_shot_at_angle(struct Thing *thing, long a2);
DLLIMPORT void _DK_explosion_affecting_area(struct Thing *thing, const struct Coord3d *pos, long a3, long a4, unsigned char a5);
DLLIMPORT void _DK_engine_init(void);
DLLIMPORT long _DK_load_anim_file(void);
DLLIMPORT long _DK_load_cube_file(void);
DLLIMPORT void _DK_init_colours(void);
DLLIMPORT void _DK_place_animating_slab_type_on_map(long a1, char a2, unsigned char a3, unsigned char a4, unsigned char a5);
DLLIMPORT void _DK_draw_spell_cursor(unsigned char a1, unsigned short a2, unsigned char stl_x, unsigned char stl_y);
DLLIMPORT long _DK_take_money_from_dungeon(short a1, long a2, unsigned char a3);
DLLIMPORT unsigned char _DK_find_door_of_type(unsigned long a1, unsigned char a2);
DLLIMPORT void _DK_maintain_my_event_list(struct Dungeon *dungeon);
DLLIMPORT void _DK_process_armageddon(void);
DLLIMPORT void _DK_update_breed_activities(void);
DLLIMPORT void _DK_maintain_my_battle_list(void);
DLLIMPORT long _DK_remove_workshop_object_from_player(long a1, long a2);
DLLIMPORT unsigned char _DK_tag_cursor_blocks_place_trap(unsigned char a1, long a2, long a3);
DLLIMPORT void _DK_stop_creatures_around_hand(char a1, unsigned short a2, unsigned short a3);
DLLIMPORT struct Thing *_DK_get_queryable_object_near(unsigned short a1, unsigned short a2, long a3);
DLLIMPORT int _DK_can_thing_be_queried(struct Thing *thing, long a2);
DLLIMPORT int _DK_can_thing_be_possessed(struct Thing *thing, long a2);
DLLIMPORT long _DK_tag_blocks_for_digging_in_rectangle_around(long a1, long a2, char a3);
DLLIMPORT void _DK_untag_blocks_for_digging_in_rectangle_around(long a1, long a2, char a3);
DLLIMPORT short _DK_delete_room_slab(long x, long y, unsigned char gnd_slab);
DLLIMPORT void _DK_tag_cursor_blocks_sell_area(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT long _DK_packet_place_door(long a1, long a2, long a3, long a4, unsigned char a5);
DLLIMPORT void _DK_delete_room_slabbed_objects(long a1);
DLLIMPORT unsigned char _DK_tag_cursor_blocks_place_door(unsigned char a1, long a2, long a3);
DLLIMPORT long _DK_remove_workshop_item(long a1, long a2, long a3);
DLLIMPORT unsigned char _DK_tag_cursor_blocks_place_room(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT void _DK_tag_cursor_blocks_dig(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT void _DK_tag_cursor_blocks_thing_in_hand(unsigned char a1, long a2, long a3, int a4, long a5);
DLLIMPORT struct Thing *_DK_create_gold_for_hand_grab(struct Thing *thing, long a2);
DLLIMPORT long _DK_remove_food_from_food_room_if_possible(struct Thing *thing);
DLLIMPORT void _DK_process_person_moods_and_needs(struct Thing *thing);
DLLIMPORT struct Thing *_DK_get_door_for_position(long pos_x, long pos_y);
DLLIMPORT long _DK_process_obey_leader(struct Thing *thing);
DLLIMPORT unsigned char _DK_external_set_thing_state(struct Thing *thing, long state);
DLLIMPORT long _DK_is_thing_passenger_controlled(struct Thing *thing);
DLLIMPORT void _DK_setup_3d(void);
DLLIMPORT void _DK_setup_stuff(void);
DLLIMPORT void _DK_init_keeper(void);
DLLIMPORT void _DK_check_map_for_gold(void);
DLLIMPORT void _DK_set_thing_draw(struct Thing *thing, long a2, long a3, long a4, char a5, char a6, unsigned char a7);
DLLIMPORT struct Thing *_DK_find_base_thing_on_mapwho(unsigned char oclass, unsigned short model, unsigned short x, unsigned short y);
DLLIMPORT unsigned long _DK_can_drop_thing_here(long x, long y, long a3, unsigned long a4);
DLLIMPORT long _DK_thing_in_wall_at(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT void _DK_do_map_rotate_stuff(long a1, long a2, long *a3, long *a4, long a5);
DLLIMPORT unsigned char _DK_active_battle_exists(unsigned char a1);
DLLIMPORT unsigned char _DK_step_battles_forward(unsigned char a1);
DLLIMPORT void _DK_go_to_my_next_room_of_type(unsigned long rkind);
DLLIMPORT void _DK_instant_instance_selected(long a1);
DLLIMPORT void _DK_initialise_map_collides(void);
DLLIMPORT void _DK_initialise_map_health(void);
DLLIMPORT void _DK_initialise_extra_slab_info(unsigned long lv_num);
DLLIMPORT long _DK_add_gold_to_hoarde(struct Thing *thing, struct Room *room, long amount);
DLLIMPORT void _DK_clear_mapwho(void);
DLLIMPORT void _DK_clear_map(void);
DLLIMPORT long _DK_ceiling_init(unsigned long a1, unsigned long a2);
DLLIMPORT long _DK_screen_to_map(struct Camera *camera, long scrpos_x, long scrpos_y, struct Coord3d *mappos);
DLLIMPORT void _DK_draw_swipe(void);
DLLIMPORT void _DK_draw_texture(long a1, long a2, long a3, long a4, long a5, long a6, long a7);
DLLIMPORT void _DK_check_players_won(void);
DLLIMPORT void _DK_check_players_lost(void);
DLLIMPORT void _DK_process_dungeon_power_magic(void);
DLLIMPORT void _DK_process_dungeon_devastation_effects(void);
DLLIMPORT void _DK_process_entrance_generation(void);
DLLIMPORT void _DK_process_payday(void);
DLLIMPORT void _DK_remove_thing_from_mapwho(struct Thing *thing);
DLLIMPORT void _DK_place_thing_in_mapwho(struct Thing *thing);
DLLIMPORT long _DK_get_thing_height_at(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT struct Room *_DK_player_has_room_of_type(long plr_idx, long roomkind);
DLLIMPORT long _DK_get_next_manufacture(struct Dungeon *dungeon);
DLLIMPORT unsigned long _DK_setup_move_off_lava(struct Thing *thing);
DLLIMPORT struct Thing *_DK_create_thing(struct Coord3d *pos, unsigned short a1, unsigned short a2, unsigned short a3, long a4);
DLLIMPORT void _DK_move_thing_in_map(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_get_floor_height_under_thing_at(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_load_texture_map_file(unsigned long lv_num, unsigned char n);
DLLIMPORT void _DK_apply_damage_to_thing_and_display_health(struct Thing *thing, long a1, char a2);
DLLIMPORT long _DK_get_foot_creature_has_down(struct Thing *thing);
DLLIMPORT void _DK_process_disease(struct Thing *thing);
DLLIMPORT void _DK_process_keeper_spell_effect(struct Thing *thing);
DLLIMPORT void _DK_leader_find_positions_for_followers(struct Thing *thing);
DLLIMPORT unsigned long _DK_lightning_is_close_to_player(struct PlayerInfo *player, struct Coord3d *pos);
DLLIMPORT void _DK_affect_nearby_enemy_creatures_with_wind(struct Thing *thing);
DLLIMPORT void _DK_god_lightning_choose_next_creature(struct Thing *thing);
DLLIMPORT void _DK_draw_god_lightning(struct Thing *thing);
DLLIMPORT void _DK_affect_nearby_stuff_with_vortex(struct Thing *thing);
DLLIMPORT void _DK_affect_nearby_friends_with_alarm(struct Thing *thing);
DLLIMPORT long _DK_apply_wallhug_force_to_boulder(struct Thing *thing);
DLLIMPORT void _DK_lightning_modify_palette(struct Thing *thing);
DLLIMPORT void _DK_update_god_lightning_ball(struct Thing *thing);
DLLIMPORT long _DK_process_creature_self_spell_casting(struct Thing *thing);
DLLIMPORT void _DK_gui_set_button_flashing(long a1, long a2);
DLLIMPORT short _DK_send_creature_to_room(struct Thing *thing, struct Room *room);
DLLIMPORT struct Room *_DK_get_room_thing_is_on(struct Thing *thing);
DLLIMPORT long _DK_load_stats_files(void);
DLLIMPORT void _DK_check_and_auto_fix_stats(void);
DLLIMPORT long _DK_update_dungeon_scores(void);
DLLIMPORT long _DK_update_dungeon_generation_speeds(void);
DLLIMPORT void _DK_calculate_dungeon_area_scores(void);
DLLIMPORT void _DK_delete_all_structures(void);
DLLIMPORT void _DK_clear_mapwho(void);
DLLIMPORT void _DK_clear_game(void);
DLLIMPORT void _DK_clear_game_for_save(void);
DLLIMPORT long _DK_update_cave_in(struct Thing *thing);
DLLIMPORT void _DK_update_thing_animation(struct Thing *thing);
DLLIMPORT void _DK_init_messages(void);
DLLIMPORT void _DK_battle_initialise(void);
DLLIMPORT void _DK_message_add(char c);
DLLIMPORT void _DK_toggle_creature_tendencies(struct PlayerInfo *player, char val);
DLLIMPORT void _DK_turn_off_call_to_arms(long a);
DLLIMPORT void _DK_set_player_state(struct PlayerInfo *player, unsigned char a1, long a2);
DLLIMPORT void _DK_event_delete_event(long plridx, long num);
DLLIMPORT long _DK_set_autopilot_type(long plridx, long aptype);
DLLIMPORT void _DK_set_player_mode(struct PlayerInfo *player, long val);
DLLIMPORT void _DK_turn_off_sight_of_evil(long plridx);
DLLIMPORT void _DK_directly_cast_spell_on_thing(unsigned char plridx, unsigned char a2, unsigned short a3, long a4);
DLLIMPORT void _DK_lose_level(struct PlayerInfo *player);
DLLIMPORT long _DK_battle_move_player_towards_battle(struct PlayerInfo *player, long var);
DLLIMPORT void _DK_level_lost_go_first_person(long plridx);
DLLIMPORT void __cdecl _DK_set_gamma(char, int);
DLLIMPORT void _DK_complete_level(struct PlayerInfo *player);
DLLIMPORT void _DK_free_swipe_graphic(void);
DLLIMPORT void _DK_draw_bonus_timer(void);
DLLIMPORT void _DK_engine(struct Camera *cam);
DLLIMPORT void _DK_remove_explored_flags_for_power_sight(struct PlayerInfo *player);
DLLIMPORT void _DK_DrawBigSprite(long x, long y, struct BigSprite *bigspr, struct TbSprite *sprite);
DLLIMPORT void _DK_pannel_map_draw(long x, long y, long zoom);
DLLIMPORT void _DK_draw_overlay_things(long zoom);
DLLIMPORT unsigned char _DK_find_first_battle_of_mine(unsigned char idx);
DLLIMPORT void _DK_set_engine_view(struct PlayerInfo *player, long a2);
DLLIMPORT void _DK_reinit_level_after_load(void);
DLLIMPORT void _DK_reinit_tagged_blocks_for_player(unsigned char idx);
DLLIMPORT void _DK_reset_gui_based_on_player_mode(void);
DLLIMPORT void _DK_init_animating_texture_maps(void);
DLLIMPORT void _DK_init_lookups(void);
DLLIMPORT int _DK_load_settings(void);
DLLIMPORT void _DK_sound_reinit_after_load(void);
DLLIMPORT void _DK_restore_computer_player_after_load(void);
DLLIMPORT void _DK_remove_events_thing_is_attached_to(struct Thing *thing);
DLLIMPORT void _DK_clear_slab_dig(long a1, long a2, char a3);
DLLIMPORT long _DK_thing_is_special(Thing *thing);
DLLIMPORT int _DK_play_smacker_file(char *fname, int);
DLLIMPORT void _DK_reset_heap_manager(void);
DLLIMPORT void _DK_reset_heap_memory(void);
DLLIMPORT int _DK_LoadMcgaData(void);
DLLIMPORT void _DK_setup_heap_manager(void);
DLLIMPORT int _DK_setup_heap_memory(void);
DLLIMPORT void _DK_reset_player_mode(struct PlayerInfo *player, unsigned char a2);
DLLIMPORT void _DK_init_keeper_map_exploration(struct PlayerInfo *player);
DLLIMPORT void _DK_init_player_cameras(struct PlayerInfo *player);
DLLIMPORT void _DK_pannel_map_update(long x, long y, long w, long h);
DLLIMPORT void _DK_view_set_camera_y_inertia(struct Camera *cam, long a2, long a3);
DLLIMPORT void _DK_view_set_camera_x_inertia(struct Camera *cam, long a2, long a3);
DLLIMPORT void _DK_view_set_camera_rotation_inertia(struct Camera *cam, long a2, long a3);
DLLIMPORT int __stdcall _DK_setup_game(void);
DLLIMPORT int __cdecl _DK_initial_setup(void);
DLLIMPORT long _DK_ceiling_set_info(long a1, long a2, long a3);
DLLIMPORT void _DK_startup_saved_packet_game(void);
DLLIMPORT void _DK_set_sprite_view_3d(void);
DLLIMPORT void _DK_set_sprite_view_isometric(void);
DLLIMPORT void _DK_do_slab_efficiency_alteration(unsigned char a1, unsigned char a2);
DLLIMPORT void _DK_place_slab_type_on_map(long a1, unsigned char a2, unsigned char a3, unsigned char a4, unsigned char a5);
DLLIMPORT void _DK_event_kill_all_players_events(long plyr_idx);
DLLIMPORT void __stdcall _DK_IsRunningMark(void);
DLLIMPORT void __stdcall _DK_IsRunningUnmark(void);
DLLIMPORT int __stdcall _DK_play_smk_(char *fname, int smkflags, int plyflags);
DLLIMPORT TbFileHandle _DK_LbFileOpen(const char *fname, int mode);
DLLIMPORT int _DK_LbFileClose(TbFileHandle handle);
DLLIMPORT void _DK_setup_engine_window(long, long, long, long);
DLLIMPORT void _DK_cumulative_screen_shot(void);
DLLIMPORT long _DK_anim_record_frame(unsigned char *screenbuf, unsigned char *palette);
DLLIMPORT void _DK_frontend_set_state(long);
DLLIMPORT void _DK_demo(void);
DLLIMPORT void _DK_draw_gui(void);
DLLIMPORT void _DK_save_settings(void);
DLLIMPORT int _DK_LbSpriteSetupAll(struct TbSetupSprite t_setup[]);
DLLIMPORT void _DK_turn_off_menu(char mnu_idx);
DLLIMPORT void _DK_turn_off_all_panel_menus(void);
DLLIMPORT void _DK_process_rooms(void);
DLLIMPORT void _DK_process_dungeons(void);
DLLIMPORT void _DK_process_player_research(int plr_idx);
DLLIMPORT long _DK_process_player_manufacturing(int plr_idx);
DLLIMPORT void _DK_event_process_events(void);
DLLIMPORT void _DK_update_all_events(void);
DLLIMPORT void _DK_process_level_script(void);
DLLIMPORT void _DK_message_update(void);
DLLIMPORT long _DK_wander_point_update(struct Wander *wandr);
DLLIMPORT void _DK_update_player_camera(struct PlayerInfo *player);
DLLIMPORT void _DK_set_level_objective(char *msg_text);
DLLIMPORT void _DK_update_flames_nearest_camera(struct Camera *camera);
DLLIMPORT void _DK_update_footsteps_nearest_camera(struct Camera *camera);
DLLIMPORT void _DK_process_player_states(void);
DLLIMPORT void _DK_set_mouse_light(struct PlayerInfo *player);
DLLIMPORT void _DK_draw_gui(void);
DLLIMPORT void _DK_turn_off_query(char a);
DLLIMPORT void _DK_post_init_level(void);
DLLIMPORT void _DK_post_init_players(void);
DLLIMPORT void _DK_init_level(void);
DLLIMPORT void _DK_init_player(struct PlayerInfo *player, int a2);
DLLIMPORT int _DK_frontend_is_player_allied(long plyr1, long plyr2);
DLLIMPORT void _DK_process_dungeon_destroy(struct Thing *thing);
DLLIMPORT void _DK_place_single_slab_type_on_map(long a1, unsigned char a2, unsigned char a3, unsigned char a4);
DLLIMPORT void _DK_shuffle_unattached_things_on_slab(long a1, long a2);
DLLIMPORT unsigned char _DK_alter_rock_style(unsigned char a1, signed char a2, signed char a3, unsigned char a4);
DLLIMPORT long _DK_wp_check_map_pos_valid(struct Wander *wandr, long a1);
DLLIMPORT void _DK_startup_network_game(void);
DLLIMPORT int _DK_create_creature_at_entrance(struct Room * room, unsigned short crtr_kind);
DLLIMPORT int _DK_calculate_free_lair_space(struct Dungeon * dungeon);
DLLIMPORT void _DK_load_ceiling_table(void);
// Now variables
DLLIMPORT extern HINSTANCE _DK_hInstance;
#ifdef __cplusplus
}
#endif

/**
 * Returns a value which decays around some epicenter, like blast damage.
 *
 * @param magnitude Magnitude in nearest whereabouts of the epicenter.
 * @param decay_start Distance aftew which the magnitude starts decaying.
 * @param decay_length Length of the decaying region.
 * @param distance Distance at which we want to compute the value.
 * @return Value at specified distane from epicenter.
 */
long get_radially_decaying_value(long magnitude,long decay_start,long decay_length,long distance)
{
  if (distance >= decay_start+decay_length)
    return 0;
  else
  if (distance >= decay_start)
    return magnitude * (decay_length - (distance-decay_start)) / decay_length;
  else
    return magnitude;
}

void view_set_camera_y_inertia(struct Camera *cam, long a2, long a3)
{
  _DK_view_set_camera_y_inertia(cam, a2, a3);
}

void view_set_camera_x_inertia(struct Camera *cam, long a2, long a3)
{
  _DK_view_set_camera_x_inertia(cam, a2, a3);
}

void view_set_camera_rotation_inertia(struct Camera *cam, long a2, long a3)
{
    _DK_view_set_camera_rotation_inertia(cam, a2, a3);
}

long get_smaller_memory_amount(long amount)
{
  if (amount > 64)
    return 64;
  if (amount > 48)
    return 48;
  if (amount > 32)
    return 32;
  if (amount > 24)
    return 24;
  if (amount > 16)
    return 16;
  if (amount >  8)
    return  8;
  return 6;
}

TbBool setup_heap_manager(void)
{
    SYNCDBG(8,"Starting");
    const char *fname;
    long i;
    //_DK_setup_heap_manager();
    if (heap == NULL)
    {
        ERRORLOG("Graphics Heap not allocated");
        return false;
    }
    i = heap_size / 512;
    if (i >= KEEPSPRITE_LENGTH)
      i = KEEPSPRITE_LENGTH-1;
    hmhdr = heapmgr_init(heap, heap_size, i);
    if (hmhdr == NULL)
    {
        ERRORLOG("Not enough memory to initialise heap.");
        return false;
    }
    wait_for_cd_to_be_available();
    fname = prepare_file_path(FGrp_StdData,"creature.jty");
    //TODO CREATURE_SPRITE Use rewritten file handling when reading is rewritten
    file_handle = _DK_LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (file_handle == -1) {
        ERRORLOG("Can not open 'creature.jty'");
        return false;
    }
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        keepsprite[i] = NULL;
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        heap_handle[i] = NULL;
    return true;
}

/**
 * Allocates graphics heap.
 */
TbBool setup_heap_memory(void)
{
  long i;
  SYNCDBG(8,"Starting");
  if (heap != NULL)
  {
    SYNCDBG(0,"Freeing old Graphics heap");
    LbMemoryFree(heap);
    heap = NULL;
  }
  i = mem_size;
  heap_size = get_best_sound_heap_size(i);
  while ( 1 )
  {
    heap = LbMemoryAlloc(heap_size);
    if (heap != NULL)
      break;
    i = get_smaller_memory_amount(i);
    if (i > 8)
    {
      heap_size = get_best_sound_heap_size(i);
    } else
    {
      if (heap_size < 524288)
      {
        ERRORLOG("Unable to allocate Graphic Heap");
        heap_size = 0;
        return false;
      }
      heap_size -= 16384;
    }
  }
  SYNCMSG("GraphicsHeap Size %d", heap_size);
  return true;
}

void reset_heap_manager(void)
{
    long i;
    SYNCDBG(8,"Starting");
    //_DK_reset_heap_manager();
    if (file_handle != -1)
    {
        //TODO CREATURE_SPRITE Use rewritten file handling when reading is rewritten
        _DK_LbFileClose(file_handle);
        file_handle = -1;
    }
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        keepsprite[i] = NULL;
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        heap_handle[i] = NULL;
}

void reset_heap_memory(void)
{
  SYNCDBG(8,"Starting");
  LbMemoryFree(heap);
  heap = NULL;
}

void init_player_as_single_keeper(struct PlayerInfo *player)
{
  unsigned short idx;
  struct InitLight ilght;
  memset(&ilght, 0, sizeof(struct InitLight));
  player->field_4CD = 0;
  ilght.field_0 = 2560;
  ilght.field_2 = 48;
  ilght.field_3 = 5;
  ilght.is_dynamic = 1;
  idx = light_create_light(&ilght);
  player->field_460 = idx;
  if (idx != 0) {
      light_set_light_never_cache(idx);
  } else {
      WARNLOG("Cannot allocate light to player %d.",(int)player->id_number);
  }
}

TbPixel get_player_path_colour(unsigned short owner)
{
  return player_path_colours[player_colors_map[owner % PLAYERS_EXT_COUNT]];
}

long get_scavenge_effect_element(unsigned short owner)
{
  return scavenge_effect_element[player_colors_map[owner % PLAYERS_EXT_COUNT]];
}

void setup_block_mem(void)
{
    unsigned char **dst;
    unsigned char *src;
    unsigned long i,k,n;
    dst = block_ptrs;
    n = 0;
    for (i=0; i < 68; i++)
    {
        src = block_mem + n;
        for (k=0; k < 8; k++)
        {
            *dst = src;
            src += 32;
            dst++;
        }
        n += 8192;
    }
}

void init_alpha_table(void)
{
  _DK_init_alpha_table();
}

void setup_stuff(void)
{
  char *fname;
  long i;
  fname = prepare_file_path(FGrp_StdData,"tables.dat");
  setup_block_mem();
  if (LbFileLoadAt(fname, &pixmap) != sizeof(struct TbColorTables))
  {
    compute_fade_tables(&pixmap,_DK_palette,_DK_palette);
    LbFileSaveAt(fname, &pixmap, sizeof(struct TbColorTables));
  }
  lbDisplay.FadeTable = pixmap.fade_tables;
  // Update black color
  for (i=0; i < 8192; i++)
  {
    if (pixmap.fade_tables[i] == 0)
      pixmap.fade_tables[i] = 144;
  }
  init_alpha_table();
}

short send_creature_to_room(struct Thing *thing, struct Room *room)
{
    return _DK_send_creature_to_room(thing, room);
}

struct Room *get_room_thing_is_on(struct Thing *thing)
{
    return _DK_get_room_thing_is_on(thing);
}

void destroy_food(struct Thing *thing)
{
    struct Room *room;
    struct Dungeon *dungeon;
    struct Thing *efftng;
    struct Coord3d pos;
    long plyr_idx,i;
    SYNCDBG(8,"Starting");
    plyr_idx = thing->owner;
    dungeon = get_dungeon(plyr_idx);
    if (game.neutral_player_num != plyr_idx) {
        dungeon->lvstats.chickens_wasted++;
    }
    efftng = create_effect(&thing->mappos, TngEff_Unknown49, plyr_idx);
    if (!thing_is_invalid(efftng)) {
        i = UNSYNC_RANDOM(3);
        thing_play_sample(efftng, 112+i, 100, 0, 3, 0, 2, 256);
    }
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val + 256;
    create_effect(&thing->mappos, TngEff_Unknown51, plyr_idx);
    create_effect(&pos, TngEff_Unknown07, plyr_idx);
    if (thing->owner != game.neutral_player_num)
    {
        if (thing->word_13 == -1)
        {
          room = get_room_thing_is_on(thing);
          if (room != NULL)
          {
            if ((room->kind == RoK_GARDEN) && (room->owner == thing->owner))
            {
                if (room->used_capacity > 0)
                  room->used_capacity--;
                thing->word_13 = game.food_life_out_of_hatchery;
            }
          }
        }
    }
    delete_thing_structure(thing, 0);
}

TbBool slap_object(struct Thing *thing)
{
  if (object_is_mature_food(thing)) {
      destroy_food(thing);
      return true;
  }
  return false;
}

TbBool object_is_slappable(const struct Thing *thing, long plyr_idx)
{
    if (thing->owner == plyr_idx) {
        return (object_is_mature_food(thing));
    }
    return false;
}

void external_activate_trap_shot_at_angle(struct Thing *thing, long a2)
{
    _DK_external_activate_trap_shot_at_angle(thing, a2);
}

void process_person_moods_and_needs(struct Thing *thing)
{
    _DK_process_person_moods_and_needs(thing);
}

void process_dungeon_destroy(struct Thing *thing)
{
  _DK_process_dungeon_destroy(thing);
}

struct Thing *create_gold_for_hand_grab(struct Thing *thing, long a2)
{
  return _DK_create_gold_for_hand_grab(thing, a2);
}

long remove_food_from_food_room_if_possible(struct Thing *thing)
{
  return _DK_remove_food_from_food_room_if_possible(thing);
}

unsigned char find_door_of_type(unsigned long a1, unsigned char a2)
{
  return _DK_find_door_of_type(a1, a2);
}

void event_kill_all_players_events(long plyr_idx)
{
  SYNCDBG(8,"Starting");
  _DK_event_kill_all_players_events(plyr_idx);
}

void process_armageddon(void)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct Thing *thing;
  long i;
  SYNCDBG(6,"Starting");
  //_DK_process_armageddon(); return;
  if (game.field_150356 == 0)
    return;
  if (game.armageddon.count_down+game.field_150356 > game.play_gameturn)
  {
    player = get_player(game.field_15035E);
    dungeon = get_dungeon(player->id_number);
    thing = thing_get(dungeon->dnheart_idx);
    if ((player->victory_state == VicS_LostLevel) || thing_is_invalid(thing) || (thing->active_state == CrSt_ImpArrivesAtDigOrMine2))
        game.field_150356 = 0;
  } else
  if (game.armageddon.count_down+game.field_150356 == game.play_gameturn)
  {
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if (player_exists(player))
      {
        if (player->field_2C == 1)
          reveal_whole_map(player);
      }
    }
  } else
  if (game.armageddon.count_down+game.field_150356 < game.play_gameturn)
  {
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if ( (player_exists(player)) && (player->field_2C == 1) )
      {
        dungeon = get_dungeon(player->id_number);
        if ( (player->victory_state == VicS_Undecided) && (dungeon->num_active_creatrs == 0))
        {
          event_kill_all_players_events(i);
          set_player_as_lost_level(player);
          if (is_my_player_number(i))
            LbPaletteSet(_DK_palette);
          thing = thing_get(dungeon->dnheart_idx);
          if (!thing_is_invalid(thing))
          {
            thing->health = -1;
          }
        }
      }
    }
  }
}

long process_obey_leader(struct Thing *thing)
{
  return _DK_process_obey_leader(thing);
}

TbBool player_is_friendly_or_defeated(int plyr_idx, int win_plyr_idx)
{
  struct PlayerInfo *player;
  struct PlayerInfo *win_player;
  struct Dungeon *dungeon;
  // Handle neutral player at first, because we can't get PlayerInfo nor Dungeon for it
  if ((win_plyr_idx == game.neutral_player_num) || (plyr_idx == game.neutral_player_num))
      return true;
  player = get_player(plyr_idx);
  win_player = get_player(win_plyr_idx);
  if (player_exists(player))
  {
      if ( (!player_allied_with(win_player, plyr_idx)) || (!player_allied_with(player, win_plyr_idx)) )
      {
          dungeon = get_dungeon(plyr_idx);
          if (dungeon->dnheart_idx > 0)
            return false;
      }
  }
  return true;
}

TbBool all_dungeons_destroyed(struct PlayerInfo *win_player)
{
  long win_plyr_idx;
  long i;
  win_plyr_idx = win_player->id_number;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    if (i == win_plyr_idx)
      continue;
    if (!player_is_friendly_or_defeated(i,win_plyr_idx))
      return false;
  }
  SYNCDBG(1,"Returning true for player %ld",win_plyr_idx);
  return true;
}

short make_group_member_leader(struct Thing *leadtng)
{
  struct Thing *prvtng;
  prvtng = get_group_leader(leadtng);
  if (thing_is_invalid(prvtng))
    return false;
  if (prvtng != leadtng)
  {
    remove_creature_from_group(leadtng);
    add_creature_to_group_as_leader(leadtng, prvtng);
    return true;
  }
  return false;
}

void reset_player_mode(struct PlayerInfo *player, unsigned short nmode)
{
  //_DK_reset_player_mode(player, nmode);
  player->view_type = nmode;
  switch (nmode)
  {
    case 1:
      player->work_state = player->continue_work_state;
      if (player->field_4B5 == 5)
        set_engine_view(player, 5);
      else
        set_engine_view(player, 2);
      if (is_my_player(player))
        game.numfield_D &= 0xFEu;
      break;
    case 2:
    case 3:
      player->work_state = player->continue_work_state;
      set_engine_view(player, 1);
      if (is_my_player(player))
        game.numfield_D |= 0x01;
      break;
    case 4:
      player->work_state = player->continue_work_state;
      set_engine_view(player, 3);
      if (is_my_player(player))
        game.numfield_D &= 0xFEu;
      break;
    default:
      break;
  }
}

void init_keeper_map_exploration(struct PlayerInfo *player)
{
  _DK_init_keeper_map_exploration(player);
}

void init_player_cameras(struct PlayerInfo *player)
{
  _DK_init_player_cameras(player);
}

void init_dungeons_research(void)
{
  struct Dungeon *dungeon;
  int i;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    dungeon = get_dungeon(i);
    dungeon->field_F78 = get_next_research_item(dungeon);
  }
}

TbBool remove_all_research_from_player(long plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    dungeon->research_num = 0;
    dungeon->research_override = 1;
    return true;
}

TbBool research_overriden_for_player(long plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    return (dungeon->research_override != 0);
}

TbBool clear_research_for_all_players(void)
{
    struct Dungeon *dungeon;
    int plyr_idx;
    for (plyr_idx=0; plyr_idx < DUNGEONS_COUNT; plyr_idx++)
    {
      dungeon = get_dungeon(plyr_idx);
      dungeon->research_num = 0;
      dungeon->research_override = 0;
    }
    return true;
}

TbBool add_research_to_player(long plyr_idx, long rtyp, long rkind, long amount)
{
    struct Dungeon *dungeon;
    struct ResearchVal *resrch;
    long i;
    dungeon = get_dungeon(plyr_idx);
    i = dungeon->research_num;
    if (i >= DUNGEON_RESEARCH_COUNT)
    {
      ERRORLOG("Too much research (%d items) for player %d", i, plyr_idx);
      return false;
    }
    resrch = &dungeon->research[i];
    resrch->rtyp = rtyp;
    resrch->rkind = rkind;
    resrch->req_amount = amount;
    dungeon->research_num++;
    return true;
}

TbBool add_research_to_all_players(long rtyp, long rkind, long amount)
{
  TbBool result;
  long i;
  result = true;
  SYNCDBG(17,"Adding type %d, kind %d, amount %d",rtyp, rkind, amount);
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    result &= add_research_to_player(i, rtyp, rkind, amount);
  }
  return result;
}

TbBool update_players_research_amount(long plyr_idx, long rtyp, long rkind, long amount)
{
  struct Dungeon *dungeon;
  struct ResearchVal *resrch;
  long i;
  dungeon = get_dungeon(plyr_idx);
  for (i = 0; i < dungeon->research_num; i++)
  {
    resrch = &dungeon->research[i];
    if ((resrch->rtyp == rtyp) && (resrch->rkind = rkind))
    {
      resrch->req_amount = amount;
    }
    return true;
  }
  return false;
}

TbBool update_or_add_players_research_amount(long plyr_idx, long rtyp, long rkind, long amount)
{
  if (update_players_research_amount(plyr_idx, rtyp, rkind, amount))
    return true;
  return add_research_to_player(plyr_idx, rtyp, rkind, amount);
}

void init_spiral_steps(void)
{
  struct MapOffset *sstep;
  long x,y;
  long i;
  y = 0;
  x = 0;
    sstep = &spiral_step[0];
    sstep->h = y;
    sstep->v = x;
    sstep->both = (short)y + ((short)x << 8);
  y = -1;
  x = -1;
  for (i=1; i < SPIRAL_STEPS_COUNT; i++)
  {
    sstep = &spiral_step[i];
    sstep->h = y;
    sstep->v = x;
    sstep->both = (short)y + ((short)x << 8);
    if ((y < 0) && (x-y == 1))
    {
      y--;
      x -= 2;
    } else
    if (x == y)
    {
      if (y < 0)
        y++;
      else
        y--;
    } else
    if (y+x == 0)
    {
      if (x >= 0)
        x--;
      else
        x++;
    } else
    if (abs(x) >= abs(y))
    {
      if (x < 0)
        y++;
      else
        y--;
    } else
    {
      if (y >= 0)
        x++;
      else
        x--;
    }
  }
}

void init_creature_scores(void)
{
  struct CreatureStats *crstat;
  long i,k;
  long score,max_score;
  max_score = 0;
  for (i=0; i < CREATURE_TYPES_COUNT; i++)
  {
    crstat = creature_stats_get(i);
    score = compute_creature_max_health(crstat->health,CREATURE_MAX_LEVEL-1)
        + compute_creature_max_defense(crstat->defense,CREATURE_MAX_LEVEL-1)
        + compute_creature_max_dexterity(crstat->dexterity,CREATURE_MAX_LEVEL-1)
        + compute_creature_max_armour(crstat->armour,CREATURE_MAX_LEVEL-1)
        + compute_creature_max_strength(crstat->strength,CREATURE_MAX_LEVEL-1);
    if ((score <= 0) && (i != 0) && (i != CREATURE_TYPES_COUNT-1))
    {
      ERRORLOG("Couldn't get creature %ld score value", i);
      continue;
    }
    if (score > max_score)
    {
      max_score = score;
    }
  }
  if (max_score <= 0)
  {
    ERRORLOG("Creatures have no score");
    return;
  }
  for (i=0; i < CREATURE_TYPES_COUNT; i++)
  {
    crstat = creature_stats_get(i);
    for (k=0; k < CREATURE_MAX_LEVEL; k++)
    {
      score = compute_creature_max_health(crstat->health,k)
          + compute_creature_max_defense(crstat->defense,k)
          + compute_creature_max_dexterity(crstat->dexterity,k)
          + compute_creature_max_armour(crstat->armour,k)
          + compute_creature_max_strength(crstat->strength,k);
      score = saturate_set_unsigned(200*score / max_score, 8);
      if ((score <= 0) && (i != 0) && (i != 31))
      {
        //WARNMSG("Couldn't get creature %d score for lev %d", i, k);
        score = 1;
      }
      game.creature_scores[i].value[k] = score;
    }
  }
}

void init_creature_state(struct Thing *thing)
{
    struct Room *room;
    if (thing->owner == game.neutral_player_num)
    {
        set_start_state(thing);
        return;
    }
    room = get_room_thing_is_on(thing);
    if (!room_is_invalid(room))
    {
        switch (room->kind)
        {
        case RoK_PRISON:
        case RoK_TORTURE:
        case RoK_GUARDPOST:
            if ( send_creature_to_room(thing, room) )
              return;
        default:
            break;
        }
    }
    set_start_state(thing);
}

void clear_creature_pool(void)
{
    memset(&game.pool,0,sizeof(struct CreaturePool));
    game.pool.is_empty = true;
}

void clear_slab_dig(long a1, long a2, char a3)
{
  _DK_clear_slab_dig(a1, a2, a3);
}

void move_thing_in_map(struct Thing *thing, struct Coord3d *pos)
{
  SYNCDBG(18,"Starting");
  if ((thing->mappos.x.stl.num == pos->x.stl.num) && (thing->mappos.y.stl.num == pos->y.stl.num))
  {
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = pos->z.val;
  } else
  {
    remove_thing_from_mapwho(thing);
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = pos->z.val;
    place_thing_in_mapwho(thing);
  }
  thing->field_60 = get_thing_height_at(thing, &thing->mappos);
}

long get_floor_height_under_thing_at(struct Thing *thing, struct Coord3d *pos)
{
  return _DK_get_floor_height_under_thing_at(thing, pos);
}

/**
 * Applies given damage points to a creature and shows health flower.
 * Uses the creature defense value to compute the actual damage.
 * Can be used only to make damage - never to heal creature.
 *
 * @param thing
 * @param dmg
 * @param a3
 */
void apply_damage_to_thing_and_display_health(struct Thing *thing, long dmg, char a3)
{
    //_DK_apply_damage_to_thing_and_display_health(thing, a1, a2);
    if (dmg > 0)
    {
        apply_damage_to_thing(thing, dmg, a3);
        thing->word_17 = 8;
    }
}

void give_shooter_drained_health(struct Thing *shooter, long health_delta)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    long max_health,health;
    if ( !thing_exists(shooter) )
        return;
    crstat = creature_stats_get_from_thing(shooter);
    cctrl = creature_control_get_from_thing(shooter);
    max_health = compute_creature_max_health(crstat->health, cctrl->explevel);
    health = shooter->health + health_delta;
    if (health < max_health) {
        shooter->health = health;
    } else {
        shooter->health = max_health;
    }
}

long get_foot_creature_has_down(struct Thing *thing)
{
    return _DK_get_foot_creature_has_down(thing);
}

void process_disease(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    _DK_process_disease(thing);
}

void process_keeper_spell_effect(struct Thing *thing)
{
  _DK_process_keeper_spell_effect(thing);
}

TbBool add_spell_to_player(long spl_idx, long plyr_idx)
{
    struct Dungeon *dungeon;
    long i;
    if ((spl_idx < 0) || (spl_idx >= KEEPER_SPELLS_COUNT))
    {
        ERRORLOG("Can't add incorrect spell %ld to player %ld",spl_idx, plyr_idx);
        return false;
    }
    dungeon = get_dungeon(plyr_idx);
    i = dungeon->magic_level[spl_idx];
    if (i >= 255)
    {
        ERRORLOG("Spell %ld has bad magic_level=%ld for player %ld",spl_idx, i, plyr_idx);
        return false;
    }
    dungeon->magic_level[spl_idx] = i+1;
    dungeon->magic_resrchable[spl_idx] = 1;
    return true;
}

unsigned char sight_of_evil_expand_check(void)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    player = get_my_player();
    dungeon = get_dungeon(player->id_number);
    return (player->field_4D2 != 0) && (dungeon->keeper_sight_thing_idx == 0);
}

unsigned char call_to_arms_expand_check(void)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    player = get_my_player();
    dungeon = get_dungeon(player->id_number);
    return (player->field_4D2 != 0) && (dungeon->field_884 == 0);
}

unsigned char general_expand_check(void)
{
    struct PlayerInfo *player;
    player = get_my_player();
    return (player->field_4D2 != 0);
}

void leader_find_positions_for_followers(struct Thing *thing)
{
  _DK_leader_find_positions_for_followers(thing);
}

unsigned char external_set_thing_state(struct Thing *thing, long state)
{
    struct CreatureControl *cctrl;
    struct StateInfo *stati;
    CreatureStateFunc1 callback;
    //return _DK_external_set_thing_state(thing, state);
    if ( !can_change_from_state_to(thing, thing->active_state, state) )
    {
        ERRORDBG(4,"State change %s to %s for %s not allowed",creature_state_code_name(thing->active_state), creature_state_code_name(state), thing_model_name(thing));
        return 0;
    }
    SYNCDBG(9,"State change %s to %s for %s index %d",creature_state_code_name(thing->active_state), creature_state_code_name(state), thing_model_name(thing),(int)thing->index);
    stati = get_thing_active_state_info(thing);
    if (stati->state_type == CrStTyp_Value6)
        stati = get_thing_continue_state_info(thing);
    callback = stati->cleanup_state;
    if (callback != NULL) {
        callback(thing);
        thing->field_1 |= 0x10;
    } else {
      clear_creature_instance(thing);
    }
    thing->active_state = state;
    thing->field_1 &= ~0x10;
    thing->continue_state = 0;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    cctrl->field_302 = 0;
    if ((cctrl->flgfield_1 & 0x20) != 0)
    {
        ERRORLOG("External change state %s to %s, but %s in room list even after cleanup",creature_state_code_name(thing->active_state), creature_state_code_name(state), thing_model_name(thing));
        remove_creature_from_work_room(thing);
    }
    return 1;
}

long is_thing_passenger_controlled(struct Thing *thing)
{
    return _DK_is_thing_passenger_controlled(thing);
}

long process_creature_self_spell_casting(struct Thing *thing)
{
    return _DK_process_creature_self_spell_casting(thing);
}

unsigned long lightning_is_close_to_player(struct PlayerInfo *player, struct Coord3d *pos)
{
    return _DK_lightning_is_close_to_player(player, pos);
}

void affect_nearby_enemy_creatures_with_wind(struct Thing *thing)
{
    _DK_affect_nearby_enemy_creatures_with_wind(thing);
}

void god_lightning_choose_next_creature(struct Thing *thing)
{
    _DK_god_lightning_choose_next_creature(thing);
}

void draw_god_lightning(struct Thing *thing)
{
    _DK_draw_god_lightning(thing);
}

void affect_nearby_stuff_with_vortex(struct Thing *thing)
{
  _DK_affect_nearby_stuff_with_vortex(thing);
}

void affect_nearby_friends_with_alarm(struct Thing *thing)
{
    _DK_affect_nearby_friends_with_alarm(thing);
}

long apply_wallhug_force_to_boulder(struct Thing *thing)
{
  return _DK_apply_wallhug_force_to_boulder(thing);
}

void lightning_modify_palette(struct Thing *thing)
{
    struct PlayerInfo *myplyr;
    // _DK_lightning_modify_palette(thing);
    myplyr = get_my_player();

    if (thing->health == 0)
    {
      PaletteSetPlayerPalette(myplyr, _DK_palette);
      myplyr->field_3 &= 0xF7u;
      return;
    }
    if (myplyr->acamera == NULL)
    {
        WARNLOG("No active camera");
        return;
    }
    if (((thing->health % 8) != 7) && (thing->health != 1) && (ACTION_RANDOM(4) != 0))
    {
      if ((myplyr->field_3 & 0x08) != 0)
      {
        if (get_2d_box_distance(&myplyr->acamera->mappos, &thing->mappos) < 11520)
        {
            PaletteSetPlayerPalette(myplyr, _DK_palette);
            myplyr->field_3 &= 0xF7u;
        }
      }
      return;
    }
    if ((myplyr->view_mode != PVM_ParchFadeIn) && (myplyr->view_mode != PVM_ParchFadeOut) && (myplyr->view_mode != PVM_ParchmentView))
    {
      if ((myplyr->field_3 & 0x08) == 0)
      {
        if (get_2d_box_distance(&myplyr->acamera->mappos, &thing->mappos) < 11520)
        {
          PaletteSetPlayerPalette(myplyr, lightning_palette);
          myplyr->field_3 |= 0x08;
        }
      }
    }
}

void update_god_lightning_ball(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *target;
    struct ShotConfigStats *shotst;
    long i;
//    _DK_update_god_lightning_ball(thing);
    if (thing->health <= 0)
    {
        lightning_modify_palette(thing);
        return;
    }
    i = (game.play_gameturn - thing->field_9) % 16;
    switch (i)
    {
    case 0:
        god_lightning_choose_next_creature(thing);
        break;
    case 1:
        target = thing_get(thing->word_17);
        if (thing_is_invalid(target))
            break;
        draw_lightning(&thing->mappos,&target->mappos, 96, 60);
        break;
    case 2:
        target = thing_get(thing->word_17);
        if (thing_is_invalid(target))
            break;
        shotst = get_shot_model_stats(24);
        apply_damage_to_thing_and_display_health(target, shotst->old->damage, thing->owner);
        if (target->health < 0)
        {
            cctrl = creature_control_get_from_thing(target);
            cctrl->shot_model = ShM_GodLightBall;
            kill_creature(target, INVALID_THING, thing->owner, 0, 1, 0);
        }
        thing->word_17 = 0;
        break;
    }
}

unsigned char line_of_sight_3d(const struct Coord3d *pos1, const struct Coord3d *pos2)
{
  return _DK_line_of_sight_3d(pos1, pos2);
}

void draw_flame_breath(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4)
{
  _DK_draw_flame_breath(pos1, pos2, a3, a4);
}

void draw_lightning(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4)
{
  _DK_draw_lightning(pos1, pos2, a3, a4);
}

/**
 * Determines if an explosion of given hit thing type and owner can affect given thing.
 * Explosions can affect a lot more things than shots. If only the thing isn't invalid,
 * it is by default affected by explosions.
 */
TbBool explosion_can_affect_thing(struct Thing *thing, long hit_type, long explode_owner)
{
    if (thing_is_invalid(thing))
    {
        WARNLOG("Invalid thing tries to interact with explosion");
        return false;
    }
    switch (hit_type)
    {
    case 1:
        if ((thing->class_id != TCls_Creature) && (thing->class_id != TCls_Object))
          return false;
        return true;
    case 2:
        if (thing->class_id != TCls_Creature)
            return false;
        return true;
    case 3:
        if ((thing->class_id != TCls_Creature) && (thing->class_id != TCls_Object))
            return false;
        if (thing->owner == explode_owner)
            return false;
        return true;
    case 4:
        if (thing->class_id != TCls_Creature)
            return false;
        if (thing->owner == explode_owner)
            return false;
        return true;
    case 7:
        if (thing->class_id != TCls_Object)
            return false;
        if (!thing_is_dungeon_heart(thing))
            return false;
        return true;
    case 8:
        return true;
    default:
        WARNLOG("Illegal hit thing type %d for explosion",(int)hit_type);
        return true;
    }
}

void explosion_affecting_thing(struct Thing *tngsrc, struct Thing *tngdst, const struct Coord3d *pos, long max_dist, long max_damage, long owner)
{
    long move_dist,move_angle;
    long distance,damage;
    if ( line_of_sight_3d(pos, &tngdst->mappos) )
    {
      move_angle = get_angle_xy_to(pos, &tngdst->mappos);
      distance = get_2d_distance(pos, &tngdst->mappos);
      if ( distance < max_dist )
      {
        move_dist = ((max_dist - distance) << 8) / max_dist;
        if (tngdst->class_id == TCls_Creature)
        {
            // damage = (max_dist - distance) * 300 * max_damage / 256 / max_dist + 1;
            damage = get_radially_decaying_value(max_damage,max_dist/4,max_dist,distance)+1;
            apply_damage_to_thing_and_display_health(tngdst, damage, owner);
        }
        // If the thing isn't dying, move it
        if ((tngdst->class_id != TCls_Creature) || (tngdst->health >= 0))
        {
            tngdst->acceleration.x.val +=   move_dist * LbSinL(move_angle) >> 16;
            tngdst->acceleration.y.val += -(move_dist * LbCosL(move_angle) >> 8) >> 8;
            tngdst->field_1 |= 0x04;
        } else
        {
            kill_creature(tngdst, tngsrc, -1, 0, 1, 0);
        }
      }
    }
}

void explosion_affecting_mapblk(struct Thing *tngsrc, const struct Map *mapblk, const struct Coord3d *pos, long max_dist, long max_damage, unsigned char hit_type)
{
    struct Thing *thing;
    long owner;
    unsigned long k;
    long i;
    if (!thing_is_invalid(tngsrc))
        owner = tngsrc->owner;
    else
        owner = -1;
    k = 0;
    i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->field_2;
        // Should never happen - only existing thing shall be in list
        if (!thing_exists(thing))
        {
            WARNLOG("Jump to nonexisting thing");
            break;
        }
        // Per thing processing block
        if (explosion_can_affect_thing(thing, hit_type, owner))
        {
            explosion_affecting_thing(tngsrc, thing, pos, max_dist, max_damage, owner);
        }
        // Per thing processing block ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

void explosion_affecting_area(struct Thing *tngsrc, const struct Coord3d *pos,
      long range, long max_damage, unsigned char hit_type)
{
    const struct Map *mapblk;
    long start_x,start_y;
    long end_x,end_y;
    long stl_x,stl_y;
    long max_dist;
    //_DK_explosion_affecting_area(tngsrc, pos, range, max_damage, hit_type); return;
    if ((hit_type < 1) || (hit_type >= HIT_TYPES_COUNT))
    {
        ERRORLOG("The %s tries to affect area range %d with invalid hit type %d",thing_model_name(tngsrc),(int)range,(int)hit_type);
        hit_type = 1;
    }
    max_dist = (range << 8);
    if (pos->x.stl.num > range)
      start_x = pos->x.stl.num - range;
    else
      start_x = 0;
    if (pos->y.stl.num > range)
      start_y = pos->y.stl.num - range;
    else
      start_y = 0;
    end_x = range + pos->x.stl.num;
    if (end_x >= map_subtiles_x)
      end_x = map_subtiles_x;
    end_y = range + pos->y.stl.num;
    if (end_y > map_subtiles_y)
      end_y = map_subtiles_y;
#if (BFDEBUG_LEVEL > 0)
    if ((start_params.debug_flags & DFlg_ShotsDamage) != 0)
        create_price_effect(pos, my_player_number, max_damage);
#endif
    for (stl_y = start_y; stl_y <= end_y; stl_y++)
    {
        for (stl_x = start_x; stl_x <= end_x; stl_x++)
        {
            mapblk = get_map_block_at(stl_x, stl_y);
            explosion_affecting_mapblk(tngsrc, mapblk, pos, max_dist, max_damage, hit_type);
        }
    }
}

struct Thing *create_cave_in(struct Coord3d *pos, unsigned short cimodel, unsigned short owner)
{
    struct MagicStats *magstat;
    struct Dungeon *dungeon;
    struct Thing *thing;
    if ( !i_can_allocate_free_thing_structure(TAF_FreeEffectIfNoSlots) )
    {
        ERRORDBG(3,"Cannot create cave in %d for player %d. There are too many things allocated.",(int)cimodel,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing = allocate_free_thing_structure(TAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate cave in %d for player %d, but failed.",(int)cimodel,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->class_id = TCls_CaveIn;
    thing->model = 0;
    thing->parent_thing_idx = thing->index;
    memcpy(&thing->mappos,pos,sizeof(struct Coord3d));
    thing->owner = owner;
    thing->field_9 = game.play_gameturn;
    magstat = &game.magic_stats[PwrK_CAVEIN];
    thing->word_15 = magstat->time;
    thing->byte_13 = pos->x.stl.num;
    thing->byte_14 = pos->y.stl.num;
    thing->byte_17 = cimodel;
    thing->health = magstat->time;
    if (owner != game.neutral_player_num)
    {
        dungeon = get_dungeon(owner);
        dungeon->camera_deviate_quake = thing->word_15;
    }
    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);
    return thing;
}

struct Thing *create_thing(struct Coord3d *pos, unsigned short tngclass, unsigned short model, unsigned short owner, long a4)
{
    struct Thing *thing;
    //return _DK_create_thing(pos, tngclass, model, owner, a4);
    thing = INVALID_THING;
    switch (tngclass)
    {
    case TCls_Object:
        thing = create_object(pos, model, owner, a4);
        break;
    case TCls_Shot:
        thing = create_shot(pos, model, owner);
        break;
    case TCls_EffectElem:
        thing = create_effect_element(pos, model, owner);
        break;
    case TCls_DeadCreature:
        thing = create_dead_creature(pos, model, 1, owner, 0);
        break;
    case TCls_Creature:
        thing = create_creature(pos, model, owner);
        break;
    case TCls_Effect:
        thing = create_effect(pos, model, owner);
        break;
    case TCls_Trap:
        thing = create_trap(pos, model, owner);
        break;
    case TCls_AmbientSnd:
        thing = create_ambient_sound(pos, model, owner);
        break;
    case TCls_CaveIn:
        thing = create_cave_in(pos, model, owner);
        break;
    default:
        break;
    }
    return thing;
}

unsigned long setup_move_off_lava(struct Thing *thing)
{
  return _DK_setup_move_off_lava(thing);
}

TbBool any_player_close_enough_to_see(const struct Coord3d *pos)
{
    struct PlayerInfo *player;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if ( (player_exists(player)) && ((player->field_0 & 0x40) == 0))
      {
        if (player->acamera == NULL)
          continue;
        if (get_2d_box_distance(&player->acamera->mappos, pos) <= 6144)
          return true;
      }
    }
    return false;
}

void do_slab_efficiency_alteration(unsigned char a1, unsigned char a2)
{
  _DK_do_slab_efficiency_alteration(a1, a2);
}

unsigned short torch_flags_for_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap *sslb1,*sslb2;
    unsigned short tflag;
    tflag = 0;
    if ((slb_x % 5) == 0)
    {
        sslb1 = get_slabmap_block(slb_x,slb_y+1);
        sslb2 = get_slabmap_block(slb_x,slb_y-1);
        if ((sslb1->kind == SlbT_CLAIMED) || (sslb2->kind == SlbT_CLAIMED))
            tflag |= 0x01;
    }
    if ((slb_y % 5) == 0)
    {
        sslb1 = get_slabmap_block(slb_x+1,slb_y);
        sslb2 = get_slabmap_block(slb_x-1,slb_y);
        if ((sslb1->kind == SlbT_CLAIMED) || (sslb2->kind == SlbT_CLAIMED))
            tflag |= 0x02;
    }
    return tflag;
}

long delete_all_object_things_from_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, long rmeffect)
{
    struct Thing *thing;
    struct Map *mapblk;
    struct Coord3d pos;
    long removed_num;
    unsigned long k;
    long i,n;
    removed_num = 0;
    for (n=0; n < 9; n++)
    {
      mapblk = get_map_block_at_pos(get_subtile_number(3*slb_x+1,3*slb_y+1)+around_map[n]);
      k = 0;
      i = get_mapwho_thing_index(mapblk);
      while (i != 0)
      {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
          WARNLOG("Jump out of things array");
          break;
        }
        i = thing->field_2;
        // Per thing code
        if (thing->class_id == TCls_Object)
        {
            if (rmeffect > 0)
            {
                set_coords_to_slab_center(&pos,slb_x,slb_y);
                pos.z.val = get_floor_height_at(&pos);
                create_effect(&pos, rmeffect, thing->owner);
            }
            delete_thing_structure(thing, 0);
            removed_num++;
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
      }
    }
    return removed_num;
}

long delete_unwanted_things_from_liquid_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, long rmeffect)
{
    struct Thing *thing;
    struct Map *mapblk;
    struct Objects *objdat;
    struct Coord3d pos;
    long removed_num;
    unsigned long k;
    long i,n;
    removed_num = 0;
    for (n=0; n < 9; n++)
    {
      mapblk = get_map_block_at_pos(get_subtile_number(3*slb_x+1,3*slb_y+1)+around_map[n]);
      k = 0;
      i = get_mapwho_thing_index(mapblk);
      while (i != 0)
      {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->field_2;
        // Per thing code
        if (thing->class_id == TCls_Object)
        {
            objdat = get_objects_data_for_thing(thing);
            if (objdat->field_15)
            {
                if (rmeffect > 0)
                {
                    set_coords_to_slab_center(&pos,slb_x,slb_y);
                    pos.z.val = get_floor_height_at(&pos);
                    create_effect(&pos, rmeffect, thing->owner);
                }
                delete_thing_structure(thing, 0);
                removed_num++;
            }
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
      }
    }
    return removed_num;
}

void place_slab_type_on_map(SlabType nslab, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned char owner, unsigned char a5)
{
    SlabType previous_slab_types_around[8];
    struct SlabMap *slb;
    struct SlabAttr *slbattr;
    MapSlabCoord slb_x,slb_y;
    MapSlabCoord spos_x,spos_y;
    int skind;
    long i;
    SYNCDBG(7,"Starting");
    //_DK_place_slab_type_on_map(nslab, stl_x, stl_y, owner, a5); return;
    if ((stl_x < 0) || (stl_x > map_subtiles_x))
        return;
    if ((stl_y < 0) || (stl_y > map_subtiles_y))
        return;
    slb_x = map_to_slab[stl_x];
    slb_y = map_to_slab[stl_y];
    if (slab_kind_is_animated(nslab))
    {
        ERRORLOG("Placing animating slab %d as standard slab",(int)nslab);
    }
    for (i = 0; i < 8; i++)
    {
        spos_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
        spos_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
        slb = get_slabmap_block(spos_x,spos_y);
        if (slabmap_block_invalid(slb))
        {
            previous_slab_types_around[i] = SlbT_ROCK;
            continue;
        }
        previous_slab_types_around[i] = slb->kind;
    }

    skind = alter_rock_style(nslab, slb_x, slb_y, owner);
    slb = get_slabmap_block(slb_x,slb_y);
    slb->kind = skind;

    set_whole_slab_owner(slb_x, slb_y, owner);
    place_single_slab_type_on_map(skind, slb_x, slb_y, owner);
    shuffle_unattached_things_on_slab(slb_x, slb_y);

    slbattr = get_slab_kind_attrs(skind);
    if ((slbattr->field_F == 4) || (slbattr->field_F == 2))
    {
      for (i = 0; i < 8; i++)
      {
          spos_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
          spos_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
          slb = get_slabmap_block(spos_x,spos_y);
          if (slabmap_block_invalid(slb))
              continue;
          if (slb->kind == SlbT_EARTH)
          {
              if (torch_flags_for_slab(spos_x, spos_y) == 0)
                  slb->kind = SlbT_EARTH;
              else
                  slb->kind = SlbT_TORCHDIRT;
          }
      }
    } else
    {
      for (i = 0; i < 8; i++)
      {
          spos_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
          spos_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
          slb = get_slabmap_block(spos_x,spos_y);
          if (slabmap_block_invalid(slb))
              continue;
          if (!slab_kind_is_animated(slb->kind))
          {
              slb->kind = alter_rock_style(slb->kind, spos_x, spos_y, owner);
          }
      }
    }

    pannel_map_update(3*(MapSubtlCoord)slb_x, 3*(MapSubtlCoord)slb_y, 3, 3);

    for (i = 0; i < 8; i++)
    {
        spos_x = slb_x + (MapSlabCoord)my_around_eight[i].delta_x;
        spos_y = slb_y + (MapSlabCoord)my_around_eight[i].delta_y;
        slb = get_slabmap_block(spos_x,spos_y);
        if (slabmap_block_invalid(slb))
            continue;
        if ((previous_slab_types_around[i] != slb->kind)
          || ((slb->kind != SlbT_GOLD) && (slb->kind != SlbT_ROCK))
          || (game.game_kind == GKind_Unknown1))
        {
            slbattr = get_slab_kind_attrs(slb->kind);
            if (slbattr->field_F != 5)
                place_single_slab_type_on_map(slb->kind, spos_x, spos_y, slabmap_owner(slb));
        }
    }

    if (!a5)
      update_blocks_around_slab(slb_x,slb_y);
    switch (nslab)
    {
    case SlbT_EARTH:
    case SlbT_TORCHDIRT:
    case SlbT_ROCK:
    case SlbT_GOLD:
    case SlbT_GEMS:
    case SlbT_WALLDRAPE:
    case SlbT_WALLTORCH:
    case SlbT_WALLWTWINS:
    case SlbT_WALLWWOMAN:
    case SlbT_WALLPAIRSHR:
        delete_all_object_things_from_slab(slb_x, slb_y, 0);
        break;
    case SlbT_LAVA:
        delete_unwanted_things_from_liquid_slab(slb_x, slb_y, 17);
        break;
    case SlbT_WATER:
        delete_unwanted_things_from_liquid_slab(slb_x, slb_y, 21);
        break;
    }

}

void place_single_slab_type_on_map(long a1, unsigned char a2, unsigned char a3, unsigned char a4)
{
    _DK_place_single_slab_type_on_map(a1, a2, a3, a4);
}

void shuffle_unattached_things_on_slab(long a1, long a2)
{
    _DK_shuffle_unattached_things_on_slab(a1, a2);
}

unsigned char alter_rock_style(unsigned char a1, signed char a2, signed char a3, unsigned char a4)
{
    return _DK_alter_rock_style(a1, a2, a3, a4);
}

void update_thing_animation(struct Thing *thing)
{
    SYNCDBG(18,"Starting for %s",thing_model_name(thing));
    int i;
    struct CreatureControl *cctrl;
    if (thing->class_id == TCls_Creature)
    {
      cctrl = creature_control_get_from_thing(thing);
      if (!creature_control_invalid(cctrl))
        cctrl->field_CE = thing->field_40;
    }
    if ((thing->field_3E != 0) && (thing->field_49 != 0))
    {
        thing->field_40 += thing->field_3E;
        i = (thing->field_49 << 8);
        if (i <= 0) i = 256;
        while (thing->field_40  < 0)
        {
          thing->field_40 += i;
        }
        if (thing->field_40 > i-1)
        {
          if (thing->field_4F & 0x40)
          {
            thing->field_3E = 0;
            thing->field_40 = i-1;
          } else
          {
            thing->field_40 %= i;
          }
        }
        thing->field_48 = (thing->field_40 >> 8) & 0xFF;
    }
    if (thing->field_4A != 0)
    {
      thing->field_46 += thing->field_4A;
      if (thing->field_46 > thing->field_4B)
      {
        if (thing->field_46 >= thing->field_4D)
        {
          thing->field_46 = thing->field_4D;
          if (thing->field_50 & 0x02)
            thing->field_4A = -thing->field_4A;
          else
            thing->field_4A = 0;
        }
      } else
      {
        thing->field_46 = thing->field_4B;
        if ((thing->field_50 & 0x02) != 0)
          thing->field_4A = -thing->field_4A;
        else
          thing->field_4A = 0;
      }
    }
}

/**
 * Plays a smacker animation file and sets frontend state to nstate.
 * @param nstate Frontend state; -1 means no change, -2 means don't even
 *    change screen mode.
 * @return Returns false if fatal error occured and probram execution should end.
 */
short play_smacker_file(char *filename, int nstate)
{
  unsigned int movie_flags = 0;
  if ( SoundDisabled )
    movie_flags |= 0x01;
  short result;

  result = 1;
  if ((result)&&(nstate>-2))
  {
    if ( setup_screen_mode_minimal(get_movies_vidmode()) )
    {
      LbMouseChangeSprite(NULL);
      LbScreenClear(0);
      LbScreenSwap();
    } else
    {
      ERRORLOG("Can't enter movies video mode to play a Smacker file");
      result=0;
    }
  }
  if (result)
  {
    // Fail in playing shouldn't set result=0, because result=0 means fatal error.
    if (play_smk_(filename, 0, movie_flags | 0x100) == 0)
    {
      ERRORLOG("Smacker play error");
      result=0;
    }
  }
  if (nstate>-2)
  {
    if ( !setup_screen_mode_minimal(get_frontend_vidmode()) )
    {
      ERRORLOG("Can't re-enter frontend video mode after playing Smacker file");
      FatalError = 1;
      exit_keeper = 1;
      return 0;
    }
  } else
  {
    memset(frontend_palette, 0, PALETTE_SIZE);
  }
  LbScreenClear(0);
  LbScreenSwap();
  LbPaletteSet(frontend_palette);
  if (nstate >= 0)
    frontend_set_state(nstate);
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
  lbDisplay.MiddleButton = 0;
  if (nstate > -2)
    LbMouseSetPosition(lbDisplay.PhysicalScreenWidth/2, lbDisplay.PhysicalScreenHeight/2);
  return result;
}

void init_censorship(void)
{
  if ( censorship_enabled() )
  {
    // Modification for Dark Mistress
      set_creature_breed_graphics(20, 14, 48);
  }
}

long load_anim_file(void)
{
  SYNCDBG(8,"Starting");
  return _DK_load_anim_file();
}

long load_cube_file(void)
{
  SYNCDBG(8,"Starting");
  return _DK_load_cube_file();
}

void init_colours(void)
{
  SYNCDBG(8,"Starting");
  _DK_init_colours();
}

/**
 * Fills the array of keyboard key names.
 */
void init_key_to_strings(void)
{
    struct KeyToStringInit *ktsi;
    long k;
    memset(key_to_string, 0, sizeof(key_to_string));
    for (ktsi = &key_to_string_init[0]; ktsi->chr != 0; ktsi++)
    {
      k = ktsi->chr;
      key_to_string[k] = ktsi->str_idx;
    }
}

TbBool setup_heaps(void)
{
  TbBool low_memory;
  char snd_fname[2048];
  char *spc_fname;
  long i;
  SYNCDBG(8,"Starting");
  low_memory = false;
  if (!SoundDisabled)
  {
    StopAllSamples();
    close_sound_heap();
    if (sound_heap_memory != NULL)
    {
      LbMemoryFree(sound_heap_memory);
      sound_heap_memory = NULL;
    }
  }
  if (heap != NULL)
  {
    ERRORLOG("Graphics heap already allocated");
    LbMemoryFree(heap);
    heap = NULL;
  }
  // Allocate sound heap
  if (!SoundDisabled)
  {
    i = mem_size;
    while (sound_heap_memory == NULL)
    {
      sound_heap_size = get_best_sound_heap_size(i);
      i = get_smaller_memory_amount(i);
      sound_heap_memory = LbMemoryAlloc(sound_heap_size);
      if ((i <= 8) && (sound_heap_memory == NULL))
      {
        low_memory = true;
        break;
      }
    }
  }
  // Allocate graphics heap
  i = mem_size;
  while (heap == NULL)
  {
    heap_size = get_best_sound_heap_size(i);
    i = get_smaller_memory_amount(i);
    heap = LbMemoryAlloc(heap_size);
    if ((i <= 8) && (heap == NULL))
    {
      low_memory = true;
      break;
    }
  }
  SYNCMSG("GraphicsHeap Size %d", heap_size);

  if (low_memory)
  {
    SYNCDBG(8,"Low memory mode entered on heap allocation.");
    while (heap != NULL)
    {
      if ((!SoundDisabled) && (sound_heap_memory == NULL))
      {
        break;
      }
      if (!SoundDisabled)
      {
        if (sound_heap_size < heap_size)
        {
          heap_size -= 16384;
        } else
        if (sound_heap_size == heap_size)
        {
          heap_size -= 16384;
          sound_heap_size -= 16384;
        } else
        {
          sound_heap_size -= 16384;
        }
        if (sound_heap_size < 524288)
        {
          ERRORLOG("Unable to allocate heaps (small_mem)");
          return false;
        }
      } else
      {
        heap_size -= 16384;
      }
      if (heap_size < 524288)
      {
        if (sound_heap_memory != NULL)
        {
          LbMemoryFree(sound_heap_memory);
          sound_heap_memory = NULL;
        }
        ERRORLOG("Unable to allocate heaps (small_mem)");
        return false;
        }
      }
      if (sound_heap_memory != NULL)
      {
        LbMemoryFree(sound_heap_memory);
        sound_heap_memory = NULL;
      }
      if (heap != NULL)
      {
        LbMemoryFree(heap);
        heap = NULL;
      }
      if (!SoundDisabled)
      {
        sound_heap_memory = LbMemoryAlloc(sound_heap_size);
      }
      heap = LbMemoryAlloc(heap_size);
  }
  if (!SoundDisabled)
  {
    SYNCMSG("SoundHeap Size %d", sound_heap_size);
    // Prepare sound sample bank file names
    prepare_file_path_buf(snd_fname,FGrp_LrgSound,sound_fname);
    // language-specific speech file
    spc_fname = prepare_file_fmtpath(FGrp_LrgSound,"speech_%s.dat",get_language_lwrstr(install_info.lang_id));
    // default speech file
    if (!LbFileExists(spc_fname))
      spc_fname = prepare_file_path(FGrp_LrgSound,speech_fname);
    // speech file for english
    if (!LbFileExists(spc_fname))
      spc_fname = prepare_file_fmtpath(FGrp_LrgSound,"speech_%s.dat",get_language_lwrstr(1));
    // Initialize sample banks
    if (!init_sound_heap_two_banks(sound_heap_memory, sound_heap_size, snd_fname, spc_fname, 1622))
    {
      LbMemoryFree(sound_heap_memory);
      sound_heap_memory = NULL;
      SoundDisabled = true;
      ERRORLOG("Unable to initialise sound heap. Sound disabled.");
    }
  }
  return true;
}

void engine_init(void)
{
    //_DK_engine_init(); return;
    fill_floor_heights_table();
    generate_wibble_table();
    load_ceiling_table();
}

void init_objects(void)
{
    long i;
    game.objects_config[1].ilght.field_0 = 0;
    game.objects_config[1].ilght.field_2 = 0x00;
    game.objects_config[1].ilght.field_3 = 0;
    game.objects_config[1].health = 100;
    game.objects_config[1].field_4 = 20;
    game.objects_config[1].field_5 = 0;
    game.objects_config[2].health = 100;
    game.objects_config[2].field_4 = 0;
    game.objects_config[2].field_5 = 1;
    game.objects_config[2].ilght.is_dynamic = 0;
    game.objects_config[2].field_8 = 1;
    game.objects_config[49].health = 100;
    game.objects_config[49].field_4 = 0;
    game.objects_config[49].field_5 = 1;
    game.objects_config[49].ilght.is_dynamic = 0;
    game.objects_config[49].field_8 = 1;
    game.objects_config[3].health = 100;
    game.objects_config[3].field_4 = 20;
    game.objects_config[4].health = 100;
    game.objects_config[4].field_4 = 20;
    game.objects_config[4].field_5 = 1;
    game.objects_config[4].ilght.is_dynamic = 0;
    game.objects_config[4].field_8 = 1;
    game.objects_config[5].health = 1;
    game.objects_config[2].ilght.field_0 = 0x0600;
    game.objects_config[2].ilght.field_2 = 0x32;
    game.objects_config[2].ilght.field_3 = 5;
    game.objects_config[5].field_4 = 20;
    game.objects_config[5].field_5 = 0;
    game.objects_config[5].ilght.is_dynamic = 1;
    game.objects_config[5].field_6 = 1;
    game.objects_config[5].field_8 = 1;
    game.objects_config[6].field_4 = 8;
    game.objects_config[6].health = 50;
    game.objects_config[7].health = 100;
    game.objects_config[7].field_4 = 0;
    game.objects_config[7].field_5 = 1;
    game.objects_config[7].field_8 = 1;
    game.objects_config[8].health = 100;
    game.objects_config[8].field_4 = 20;
    game.objects_config[8].field_5 = 1;
    game.objects_config[10].health = 1000;
    game.objects_config[10].field_4 = 9;
    game.objects_config[28].health = 100;
    game.objects_config[49].ilght.field_0 = 0x0A00u;
    game.objects_config[49].ilght.field_2 = 0x28;
    game.objects_config[49].ilght.field_3 = 5;
    game.objects_config[4].ilght.field_0 = 0x0700u;
    game.objects_config[4].ilght.field_2 = 0x2F;
    game.objects_config[4].ilght.field_3 = 5;
    game.objects_config[5].ilght.field_0 = 0x0E00u;
    game.objects_config[5].ilght.field_2 = 0x24;
    game.objects_config[5].ilght.field_3 = 5;
    game.objects_config[28].field_4 = 0;
    game.objects_config[28].field_5 = 1;
    game.objects_config[28].ilght.is_dynamic = 0;
    game.objects_config[28].field_8 = 1;
    game.objects_config[11].ilght.field_0 = 0x0400u;
    game.objects_config[11].ilght.field_2 = 0x3E;
    game.objects_config[11].ilght.field_3 = 0;
    game.objects_config[11].field_4 = 10;
    game.objects_config[11].field_5 = 0;
    game.objects_config[11].ilght.is_dynamic = 0;
    game.objects_config[11].field_8 = 1;
    game.objects_config[12].ilght.field_0 = 0x0400u;
    game.objects_config[12].ilght.field_2 = 0x3E;
    game.objects_config[12].ilght.field_3 = 0;
    game.objects_config[12].field_4 = 10;
    game.objects_config[12].field_5 = 0;
    game.objects_config[12].ilght.is_dynamic = 0;
    game.objects_config[12].field_8 = 1;
    game.objects_config[13].field_4 = 10;
    game.objects_config[13].field_5 = 0;
    game.objects_config[13].ilght.field_0 = 0x0400u;
    game.objects_config[13].ilght.field_2 = 0x3E;
    game.objects_config[13].ilght.field_3 = 0;
    game.objects_config[13].ilght.is_dynamic = 0;
    game.objects_config[13].field_8 = 1;
    game.objects_config[14].ilght.field_0 = 0x0400u;
    game.objects_config[14].ilght.field_2 = 0x3E;
    game.objects_config[14].ilght.field_3 = 0;
    game.objects_config[14].field_4 = 10;
    game.objects_config[14].field_5 = 0;
    game.objects_config[14].ilght.is_dynamic = 0;
    game.objects_config[14].field_8 = 1;
    game.objects_config[15].field_4 = 10;
    game.objects_config[15].field_5 = 0;
    game.objects_config[15].ilght.field_0 = 0x0400u;
    game.objects_config[15].ilght.field_2 = 0x3E;
    game.objects_config[15].ilght.field_3 = 0;
    game.objects_config[15].ilght.is_dynamic = 0;
    game.objects_config[15].field_8 = 1;
    game.objects_config[16].ilght.field_0 = 0x0400u;
    game.objects_config[16].ilght.field_2 = 0x3E;
    game.objects_config[16].ilght.field_3 = 0;
    game.objects_config[16].field_4 = 10;
    game.objects_config[16].field_5 = 0;
    game.objects_config[16].ilght.is_dynamic = 0;
    game.objects_config[16].field_8 = 1;
    game.objects_config[17].field_4 = 10;
    game.objects_config[17].field_5 = 0;
    game.objects_config[17].ilght.field_0 = 0x0400u;
    game.objects_config[17].ilght.field_2 = 0x3E;
    game.objects_config[17].ilght.field_3 = 0;
    game.objects_config[17].ilght.is_dynamic = 0;
    game.objects_config[17].field_8 = 1;
    game.objects_config[43].field_4 = 8;
    game.objects_config[43].health = 50;
    game.objects_config[28].ilght.field_0 = 0x0600u;
    game.objects_config[28].ilght.field_2 = 0x2E;
    game.objects_config[28].ilght.field_3 = 5;
    game.objects_config[18].field_4 = 10;
    game.objects_config[18].field_5 = 0;
    game.objects_config[18].ilght.field_0 = 0x0400u;
    game.objects_config[18].ilght.field_2 = 0x3E;
    game.objects_config[18].ilght.field_3 = 0;
    game.objects_config[18].ilght.is_dynamic = 0;
    game.objects_config[19].ilght.field_0 = 0x0400u;
    game.objects_config[19].ilght.field_2 = 0x3E;
    game.objects_config[19].ilght.field_3 = 0;
    game.objects_config[18].field_8 = 1;
    game.objects_config[19].field_4 = 10;
    game.objects_config[19].field_5 = 0;
    game.objects_config[20].ilght.field_0 = 0x0400u;
    game.objects_config[20].ilght.field_2 = 0x3E;
    game.objects_config[20].ilght.field_3 = 0;
    game.objects_config[19].ilght.is_dynamic = 0;
    game.objects_config[19].field_8 = 1;
    game.objects_config[20].field_4 = 10;
    game.objects_config[20].field_5 = 0;
    game.objects_config[20].ilght.is_dynamic = 0;
    game.objects_config[21].ilght.field_0 = 0x0400u;
    game.objects_config[21].ilght.field_2 = 0x3E;
    game.objects_config[21].ilght.field_3 = 0;
    game.objects_config[20].field_8 = 1;
    game.objects_config[21].field_4 = 10;
    game.objects_config[21].field_5 = 0;
    game.objects_config[22].ilght.field_0 = 0x0400u;
    game.objects_config[22].ilght.field_2 = 0x3E;
    game.objects_config[22].ilght.field_3 = 0;
    game.objects_config[21].ilght.is_dynamic = 0;
    game.objects_config[21].field_8 = 1;
    game.objects_config[22].field_4 = 10;
    game.objects_config[22].field_5 = 0;
    game.objects_config[22].ilght.is_dynamic = 0;
    game.objects_config[23].ilght.field_0 = 0x0400u;
    game.objects_config[23].ilght.field_2 = 0x3E;
    game.objects_config[23].ilght.field_3 = 0;
    game.objects_config[22].field_8 = 1;
    game.objects_config[23].field_4 = 10;
    game.objects_config[23].field_5 = 0;
    game.objects_config[45].ilght.field_0 = 0x0400u;
    game.objects_config[45].ilght.field_2 = 0x3E;
    game.objects_config[45].ilght.field_3 = 0;
    game.objects_config[23].ilght.is_dynamic = 0;
    game.objects_config[23].field_8 = 1;
    game.objects_config[45].field_4 = 10;
    game.objects_config[45].field_5 = 0;
    game.objects_config[45].ilght.is_dynamic = 0;
    game.objects_config[46].ilght.field_0 = 0x0400u;
    game.objects_config[46].ilght.field_2 = 0x3E;
    game.objects_config[46].ilght.field_3 = 0;
    game.objects_config[45].field_8 = 1;
    game.objects_config[46].field_4 = 10;
    game.objects_config[46].field_5 = 0;
    game.objects_config[47].ilght.field_0 = 0x0400u;
    game.objects_config[47].ilght.field_2 = 0x3E;
    game.objects_config[47].ilght.field_3 = 0;
    game.objects_config[46].ilght.is_dynamic = 0;
    game.objects_config[46].field_8 = 1;
    game.objects_config[47].field_4 = 10;
    game.objects_config[47].field_5 = 0;
    game.objects_config[47].ilght.is_dynamic = 0;
    game.objects_config[134].ilght.field_0 = 0x0400u;
    game.objects_config[134].ilght.field_2 = 0x3E;
    game.objects_config[134].ilght.field_3 = 0;
    game.objects_config[47].field_8 = 1;
    game.objects_config[134].field_4 = 10;
    game.objects_config[134].field_5 = 0;
    game.objects_config[134].ilght.is_dynamic = 0;
    game.objects_config[87].ilght.field_0 = 0x0400u;
    game.objects_config[87].ilght.field_2 = 0x3E;
    game.objects_config[87].ilght.field_3 = 0;
    game.objects_config[134].field_8 = 1;
    game.objects_config[87].field_4 = 10;
    game.objects_config[87].field_5 = 0;
    game.objects_config[88].ilght.field_0 = 0x0400u;
    game.objects_config[88].ilght.field_2 = 0x3E;
    game.objects_config[88].ilght.field_3 = 0;
    game.objects_config[87].ilght.is_dynamic = 0;
    game.objects_config[88].field_4 = 10;
    game.objects_config[88].field_5 = 0;
    game.objects_config[89].ilght.field_0 = 0x0400u;
    game.objects_config[89].ilght.field_2 = 0x3E;
    game.objects_config[89].ilght.field_3 = 0;
    game.objects_config[88].ilght.is_dynamic = 0;
    game.objects_config[89].field_4 = 10;
    game.objects_config[89].field_5 = 0;
    game.objects_config[90].ilght.field_0 = 0x0400u;
    game.objects_config[90].ilght.field_2 = 0x3E;
    game.objects_config[90].ilght.field_3 = 0;
    game.objects_config[89].ilght.is_dynamic = 0;
    game.objects_config[90].field_4 = 10;
    game.objects_config[90].field_5 = 0;
    game.objects_config[91].ilght.field_0 = 0x0400u;
    game.objects_config[91].ilght.field_2 = 0x3E;
    game.objects_config[91].ilght.field_3 = 0;
    game.objects_config[90].ilght.is_dynamic = 0;
    game.objects_config[91].field_4 = 10;
    game.objects_config[91].field_5 = 0;
    game.objects_config[92].ilght.field_0 = 0x0400u;
    game.objects_config[92].ilght.field_2 = 0x3E;
    game.objects_config[92].ilght.field_3 = 0;
    game.objects_config[91].ilght.is_dynamic = 0;
    game.objects_config[92].field_4 = 10;
    game.objects_config[92].field_5 = 0;
    game.objects_config[93].ilght.field_0 = 0x0400u;
    game.objects_config[93].ilght.field_2 = 0x3E;
    game.objects_config[93].ilght.field_3 = 0;
    game.objects_config[92].ilght.is_dynamic = 0;
    game.objects_config[93].field_4 = 10;
    game.objects_config[93].field_5 = 0;
    game.objects_config[86].ilght.field_0 = 0x0400u;
    game.objects_config[86].ilght.field_2 = 0x3E;
    game.objects_config[86].ilght.field_3 = 0;
    game.objects_config[93].ilght.is_dynamic = 0;
    game.objects_config[86].field_4 = 10;
    game.objects_config[86].field_5 = 0;
    game.objects_config[86].ilght.is_dynamic = 0;
    game.objects_config[109].field_7 = 1;
    game.objects_config[109].field_8 = 1;
    game.objects_config[94].field_8 = 1;
    game.objects_config[95].field_8 = 1;
    game.objects_config[96].field_8 = 1;
    game.objects_config[97].field_8 = 1;
    game.objects_config[98].field_8 = 1;
    game.objects_config[99].field_8 = 1;
    game.objects_config[106].field_8 = 1;
    game.objects_config[107].field_8 = 1;
    game.objects_config[108].field_8 = 1;
    game.objects_config[128].field_4 = 10;
    for (i=57; i <= 85; i++)
    {
      game.objects_config[i].field_8 = 1;
    }
    game.objects_config[126].field_8 = 1;
    game.objects_config[26].field_8 = 1;
    game.objects_config[27].field_8 = 1;
    game.objects_config[31].field_8 = 1;
    game.objects_config[32].field_8 = 1;
    game.objects_config[114].field_8 = 1;
    game.objects_config[115].field_8 = 1;
    game.objects_config[117].field_8 = 1;
    game.objects_config[116].field_8 = 1;
    game.objects_config[118].field_8 = 1;
    game.objects_config[119].field_8 = 1;
    game.objects_config[125].field_8 = 1;
}

void init_keeper(void)
{
  SYNCDBG(8,"Starting");
  engine_init();
  init_iso_3d_conversion_tables();
  init_objects();
  init_colours();
  init_spiral_steps();
  init_key_to_strings();
  load_stats_files();
  check_and_auto_fix_stats();
  init_creature_scores();
  load_cube_file();
  init_top_texture_to_cube_table();
  load_anim_file();
  game.neutral_player_num = neutral_player_number;
  game.field_14EA34 = 4;
  game.field_14EA38 = 200;
  game.field_14EA28 = 256;
  game.field_14EA2A = 256;
  game.field_14EA2C = 256;
  game.field_14EA2E = 256;
  if (game.generate_speed <= 0)
    game.generate_speed = game.default_generate_speed;
  poly_pool_end = &poly_pool[sizeof(poly_pool)-128];
  lbDisplay.GlassMap = pixmap.ghost;
  lbDisplay.DrawColour = colours[15][15][15];
  game.comp_player_aggressive = 0;
  game.comp_player_defensive = 1;
  game.comp_player_construct = 0;
  game.comp_player_creatrsonly = 0;
  game.creatures_tend_1 = 0;
  game.creatures_tend_2 = 0;
  game.numfield_C |= 0x40;
  game.numfield_D |= (0x20 | 0x40);
  init_censorship();
  SYNCDBG(9,"Finished");
}

short ceiling_set_info(long height_max, long height_min, long step)
{
  SYNCDBG(6,"Starting");
  long dist;
  if (step <= 0)
  {
    ERRORLOG("Illegal ceiling step value");
    return 0;
  }
  if (height_max > 15)
  {
    ERRORLOG("Max height is too high");
    return 0;
  }
  if (height_min > height_max)
  {
    ERRORLOG("Ceiling max height is smaller than min height");
    return 0;
  }
  dist = (height_max - height_min) / step;
  if ( dist >= 2500 )
    dist = 2500;
  game.field_14A80C = dist;
  if (dist > 20)
  {
    ERRORLOG("Ceiling search distance too big");
    return 0;
  }
  game.field_14A804 = height_max;
  game.field_14A808 = height_min;
  game.field_14A814 = step;
  game.field_14A810 = (2*game.field_14A80C+1) * (2*game.field_14A80C+1);
  return 1;
}

void setup_default_settings(void)
{
    // CPU status variable
    struct CPU_INFO cpu_info;
    const struct GameSettings default_settings = {
     0, 4, 3, 0, 1, 0, 127, 90, 1, 0, 1,
     {
      {KC_UP, KMod_NONE},       {KC_DOWN, KMod_NONE},
      {KC_LEFT, KMod_NONE},     {KC_RIGHT, KMod_NONE},
      {KC_LCONTROL, KMod_NONE}, {KC_LSHIFT, KMod_NONE},
      {KC_DELETE, KMod_NONE},   {KC_PGDOWN, KMod_NONE},
      {KC_HOME, KMod_NONE},     {KC_END, KMod_NONE},
      {KC_T, KMod_NONE},        {KC_L, KMod_NONE},
      {KC_L, KMod_SHIFT},       {KC_P, KMod_SHIFT},
      {KC_T, KMod_ALT},         {KC_T, KMod_SHIFT},
      {KC_H, KMod_NONE},        {KC_W, KMod_NONE},
      {KC_S, KMod_NONE},        {KC_T, KMod_CONTROL},
      {KC_G, KMod_NONE},        {KC_B, KMod_NONE},
      {KC_H, KMod_SHIFT},       {KC_G, KMod_SHIFT},
      {KC_B, KMod_SHIFT},       {KC_F, KMod_NONE},
      {KC_A, KMod_NONE},        {KC_LSHIFT, KMod_NONE},
      {KC_NUMPAD0, KMod_NONE},  {KC_BACK, KMod_NONE},
      {KC_P, KMod_NONE},        {KC_M, KMod_NONE},
     }, 1, 0, 6};
    LbMemoryCopy(&settings, &default_settings, sizeof(struct GameSettings));
    cpu_detect(&cpu_info);
    settings.video_scrnmode = get_next_vidmode(Lb_SCREEN_MODE_INVALID);
    if ((cpu_get_family(&cpu_info) > CPUID_FAMILY_PENTIUM) && (is_feature_on(Ft_HiResVideo)))
    {
        SYNCDBG(6,"Updating to hires video mode");
        settings.video_scrnmode = get_next_vidmode(settings.video_scrnmode);
    }
}

TbBool load_settings(void)
{
    SYNCDBG(6,"Starting");
    char *fname;
    long len;
    fname = prepare_file_path(FGrp_Save,"settings.dat");
    len = LbFileLengthRnc(fname);
    if (len == sizeof(struct GameSettings))
    {
      if (LbFileLoadAt(fname, &settings) == sizeof(struct GameSettings))
          return true;
    }
    setup_default_settings();
    LbFileSaveAt(fname, &settings, sizeof(struct GameSettings));
    return false;
}

/**
 * Initial video setup - loads only most importand files to show startup screens.
 */
TbBool initial_setup(void)
{
    SYNCDBG(6,"Starting");
    // setting this will force video mode change, even if previous one is same
    MinimalResolutionSetup = true;
    if (LbDataLoadAll(game_load_files))
    {
      ERRORLOG("Unable to load game_load_files");
      return false;
    }
    // was LoadMcgaData, but minimal should be enough at this point.
    if ( !LoadMcgaDataMinimal() )
    {
      ERRORLOG("Loading MCGA files failed");
      return false;
    }
    load_pointer_file(0);
    update_screen_mode_data(320, 200);
    clear_game();
    lbDisplay.DrawFlags |= 0x4000u;
    return true;
}

/**
 * Displays 'legal' screens, intro and initializes basic game data.
 * If true is returned, then all files needed for startup were loaded,
 * and there should be the loading screen visible.
 * @return Returns true on success, false on error which makes the
 *   gameplay impossible (usually files loading failure).
 * @note The current screen resolution at end of this function may vary.
 */
short setup_game(void)
{
  struct CPU_INFO cpu_info; // CPU status variable
  char *fname;
  short result;

  // Do only a very basic setup
  cpu_detect(&cpu_info);
  SYNCMSG("CPU %s type %d family %d model %d stepping %d features %08x",cpu_info.vendor,
      (int)cpu_get_type(&cpu_info),(int)cpu_get_family(&cpu_info),(int)cpu_get_model(&cpu_info),
      (int)cpu_get_stepping(&cpu_info),cpu_info.feature_edx);
  update_memory_constraits();
  // Enable features thar require more resources
  update_features(mem_size);

  // Configuration file
  if ( !load_configuration() )
  {
      ERRORLOG("Configuration load error.");
      return 0;
  }

  LbIKeyboardOpen();

  if (LbDataLoadAll(legal_load_files) != 0)
  {
      ERRORLOG("Error on allocation/loading of legal_load_files.");
      return 0;
  }

  // View the legal screen

  if (!setup_screen_mode_zero(get_frontend_vidmode()))
  {
      ERRORLOG("Unable to set display mode for legal screen");
      return 0;
  }

  result = init_actv_bitmap_screen(RBmp_SplashLegal);
  if ( result )
  {
      result = show_actv_bitmap_screen(3000);
      free_actv_bitmap_screen();
  } else
      SYNCLOG("Legal image skipped");

  // Now do more setup
  // Prepare the Game structure
  clear_complete_game();
  // Moon phase calculation
  calculate_moon_phase(true,true);
  // Start the sound system
  if (!init_sound())
    WARNMSG("Sound system disabled.");
  // Note: for some reason, signal handlers must be installed AFTER
  // init_sound(). This will probably change when we'll move sound
  // to SDL - then we'll put that line earlier, before setup_game().
  LbErrorParachuteInstall();

  // View second splash screen
  result = init_actv_bitmap_screen(RBmp_SplashFx);
  if ( result )
  {
      result = show_actv_bitmap_screen(4000);
      free_actv_bitmap_screen();
  } else
      SYNCLOG("startup_fx image skipped");
  draw_clear_screen();

  // View Bullfrog company logo animation when new moon
  if ( is_new_moon )
    if ( !game.no_intro )
    {
      fname = prepare_file_path(FGrp_FxData,"bullfrog.smk");
      result = play_smacker_file(fname, -2);
      if ( !result )
        ERRORLOG("Unable to play new moon movie");
    }

  result = 1;
  // The 320x200 mode is required only for the intro;
  // loading and no CD screens can run in both 320x2?0 and 640x4?0.
  if ( result && (!game.no_intro) )
  {
      LbPaletteDataFillBlack(_DK_palette);
      int mode_ok = LbScreenSetup(get_movies_vidmode(), 320, 200, _DK_palette, 2, 0);
      if (mode_ok != 1)
      {
        ERRORLOG("Can't enter movies screen mode to play intro");
        result=0;
      }
  }

  if ( result )
  {
      draw_clear_screen();
      result = wait_for_cd_to_be_available();
  }
  if ( result && (!game.no_intro) )
  {
      fname = prepare_file_path(FGrp_LoData,"intromix.smk");
      result = play_smacker_file(fname, -2);
  }
  // Intro problems shouldn't force the game to quit,
  // so we're re-setting the result flag
  result = 1;

  if ( result )
  {
      display_loading_screen();
  }
  LbDataFreeAll(legal_load_files);

  if ( result )
  {
      _DK_IsRunningMark();
      if ( !initial_setup() )
        result = 0;
  }

  if ( result )
  {
    load_settings();
    if ( !setup_gui_strings_data() )
      result = 0;
  }

  if ( result )
  {
    if ( !setup_heaps() )
      result = 0;
  }

  if ( result )
  {
      init_keeper();
      switch (start_params.force_ppro_poly)
      {
      case 1:
          gpoly_enable_pentium_pro(true);
          break;
      case 2:
          gpoly_enable_pentium_pro(false);
          break;
      default:
          if (cpu_info.feature_intl == 0)
          {
              gpoly_enable_pentium_pro(false);
          } else
          if ( ((cpu_info.feature_intl>>8) & 0x0F) < 0x06 ) {
              gpoly_enable_pentium_pro(false);
          } else {
              gpoly_enable_pentium_pro(true);
          }
          break;
      }
      set_gamma(settings.gamma_correction, 0);
      SetRedbookVolume(settings.redbook_volume);
      SetSoundMasterVolume(settings.sound_volume);
      SetMusicMasterVolume(settings.sound_volume);
      setup_3d();
      setup_stuff();
      init_lookups();
      result = 1;
  }

  if (result) {
      KEEPERSPEECH_REASON reason = KeeperSpeechInit();
      if (reason == KSR_NO_LIB_INSTALLED) {
          SYNCLOG("Speech recognition disabled: %s",
              KeeperSpeechErrorMessage(reason));
      } else
      if (reason != KSR_OK) {
          ERRORLOG("Failed to initialize Speech recognition module: %s",
              KeeperSpeechErrorMessage(reason));
      }
  }

  return result;
}

unsigned short input_eastegg_keycodes(unsigned char *counter,short allow,struct KeycodeString const *codes)
{
    TbKeyCode currkey;
    unsigned short result;
    if (!allow)
    {
      (*counter) = 0;
      return 0;
    }
    result = 0;
    if ((*counter) < codes->length)
    {
      currkey = codes->keys[(*counter)];
      if (lbKeyOn[currkey])
      {
        (*counter)++;
        result = 1;
        if ((*counter) > 2)
        {
          clear_key_pressed(currkey);
          result = 2;
        }
      }
    }
    if ((*counter) == codes->length)
    {
      if (result > 0)
        result = 3;
      else
        result = 4;
    }
    return result;
}

void input_eastegg(void)
{
    short allow;
    unsigned short state;
    // Maintain the FECKOFF cheat
    allow = (lbKeyOn[KC_LSHIFT] != 0);
    state = input_eastegg_keycodes(&game.eastegg01_cntr,allow,&eastegg_feckoff_codes);
    if ((state == 2) || (state == 3))
      play_non_3d_sample(60);
    // Maintain the JLW cheat
    if ((game.flags_font & FFlg_AlexCheat) != 0)
    {
      allow = (lbKeyOn[KC_LSHIFT]) && (lbKeyOn[KC_RSHIFT]);
      state = input_eastegg_keycodes(&game.eastegg02_cntr,allow,&eastegg_jlw_codes);
      if ((state == 1) || (state == 2)  || (state == 3))
        play_non_3d_sample(159);
    }
    // Maintain the SKEKSIS cheat
    allow = (lbKeyOn[KC_LSHIFT] != 0);
    state = input_eastegg_keycodes(&eastegg_skeksis_cntr,allow,&eastegg_skeksis_codes);
    if (state == 3)
      output_message(SMsg_PantsTooTight, 0, true);
}

void init_messages(void)
{
    //_DK_init_messages();
    clear_messages();
    // Set end turn
    init_messages_turns(0);
}

void zero_messages(void)
{
    int i;
    game.active_messages_count = 0;
    for (i=0; i<3; i++)
    {
      memset(&game.messages[i], 0, sizeof(struct GuiMessage));
    }
}

void battle_initialise(void)
{
  _DK_battle_initialise();
}

void maintain_my_battle_list(void)
{
  _DK_maintain_my_battle_list();
}

struct Thing *create_ambient_sound(struct Coord3d *pos, unsigned short model, unsigned short owner)
{
    struct Thing *thing;
    if ( !i_can_allocate_free_thing_structure(TAF_FreeEffectIfNoSlots) )
    {
        ERRORDBG(3,"Cannot create ambient sound %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing = allocate_free_thing_structure(TAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate ambient sound %d for player %d, but failed.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->class_id = TCls_AmbientSnd;
    thing->model = model;
    thing->parent_thing_idx = thing->index;
    memcpy(&thing->mappos,pos,sizeof(struct Coord3d));
    thing->owner = owner;
    thing->field_4F |= 0x01;
    add_thing_to_its_class_list(thing);
    return thing;
}

/** Returns if cursor for given player is at top of the dungeon in 3D view.
 *  Cursor placed at top of dungeon is marked by green/red "volume box";
 *   if there's no volume box, cursor should be of the fileld behind it
 *   (the exact field in a line of view through cursor). If cursor is at top
 *   of view, then pointed map field is a bit lower than the line of view
 *   through cursor.
 *
 * @param player
 * @return
 */
TbBool players_cursor_is_at_top_of_view(struct PlayerInfo *player)
{
    int i;
    i = player->work_state;
    if ( (i == PSt_BuildRoom) || (i == PSt_PlaceDoor) || (i == PSt_PlaceTrap) || (i == PSt_SightOfEvil) || (i == PSt_Sell) )
        return true;
    if ( (i == PSt_CtrlDungeon) && (player->field_454 != 0) && (player->thing_under_hand == 0) )
        return true;
    return false;
}

TbBool engine_point_to_map(struct Camera *camera, long screen_x, long screen_y, long *map_x, long *map_y)
{
    struct PlayerInfo *player;
    player = get_my_player();
    *map_x = 0;
    *map_y = 0;
    if ( (pointer_x >= 0) && (pointer_y >= 0)
      && (pointer_x < (player->engine_window_width/pixel_size))
      && (pointer_y < (player->engine_window_height/pixel_size)) )
    {
        if ( players_cursor_is_at_top_of_view(player) )
        {
              *map_x = ( top_pointed_at_x << 8) + top_pointed_at_frac_x;
              *map_y = ( top_pointed_at_y << 8) + top_pointed_at_frac_y;
        } else
        {
              *map_x = (block_pointed_at_x << 8) + pointed_at_frac_x;
              *map_y = (block_pointed_at_y << 8) + pointed_at_frac_y;
        }
        // Clipping coordinates
        if (*map_y < 0)
          *map_y = 0;
        else
        if (*map_y > ((map_subtiles_y<<8)-1))
          *map_y = ((map_subtiles_y<<8)-1);
        if (*map_x < 0)
          *map_x = 0;
        else
        if (*map_x > ((map_subtiles_x<<8)-1))
          *map_x = ((map_subtiles_x<<8)-1);
        return true;
    }
    return false;
}

void restore_computer_player_after_load(void)
{
    struct Computer2 *comp;
    struct PlayerInfo *player;
    struct ComputerProcessTypes *cpt;
    long plyr_idx;
    long i;
    SYNCDBG(7,"Starting");
    //_DK_restore_computer_player_after_load();
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        player = get_player(plyr_idx);
        comp = &game.computer[plyr_idx];
        if (!player_exists(player))
        {
            LbMemorySet(comp, 0, sizeof(struct Computer2));
            comp->dungeon = INVALID_DUNGEON;
            continue;
        }
        if (player->field_2C != 1)
        {
            LbMemorySet(comp, 0, sizeof(struct Computer2));
            comp->dungeon = get_players_dungeon(player);
            continue;
        }
        comp->dungeon = get_players_dungeon(player);
        cpt = get_computer_process_type_template(comp->model);

        for (i=0; i < COMPUTER_PROCESSES_COUNT; i++)
        {
            if (cpt->processes[i] == NULL)
                break;
            //if (cpt->processes[i]->name == NULL)
            //    break;
            SYNCDBG(12,"Player %ld process %ld is \"%s\"",plyr_idx,i,cpt->processes[i]->name);
            comp->processes[i].name = cpt->processes[i]->name;
            comp->processes[i].parent = cpt->processes[i];
            comp->processes[i].func_check = cpt->processes[i]->func_check;
            comp->processes[i].func_setup = cpt->processes[i]->func_setup;
            comp->processes[i].func_task = cpt->processes[i]->func_task;
            comp->processes[i].func_complete = cpt->processes[i]->func_complete;
            comp->processes[i].func_pause = cpt->processes[i]->func_pause;
        }
        for (i=0; i < COMPUTER_CHECKS_COUNT; i++)
        {
            if (cpt->checks[i].name == NULL)
              break;
            SYNCDBG(12,"Player %ld check %ld is \"%s\"",plyr_idx,i,cpt->checks[i].name);
            comp->checks[i].name = cpt->checks[i].name;
            comp->checks[i].func = cpt->checks[i].func;
        }
        for (i=0; i < COMPUTER_EVENTS_COUNT; i++)
        {
            if (cpt->events[i].name == NULL)
              break;
            comp->events[i].name = cpt->events[i].name;
            comp->events[i].func_event = cpt->events[i].func_event;
            comp->events[i].func_test = cpt->events[i].func_test;
            comp->events[i].process = cpt->events[i].process;
        }
    }
}

short point_to_overhead_map(struct Camera *camera, long screen_x, long screen_y, long *map_x, long *map_y)
{
  struct PlayerInfo *player;
  player = get_my_player();
  *map_x = 0;
  *map_y = 0;
  if ((screen_x >= 150) && (screen_x < 490)
    && (screen_y >= 56) && (screen_y < 396))
  {
    *map_x = 3*256 * (screen_x-150) / 4 + 384;
    *map_y = 3*256 * (screen_y-56) / 4 + 384;
    return ((*map_x >= 0) && (*map_x < (map_subtiles_x+1)<<8) && (*map_y >= 0) && (*map_y < (map_subtiles_y+1)<<8));
  }
  return false;
}

TbBool screen_to_map(struct Camera *camera, long screen_x, long screen_y, struct Coord3d *mappos)
{
  TbBool result;
  long x,y;
  //SYNCDBG(19,"Starting");
  result = false;
  if (camera != NULL)
  {
    switch (camera->field_6)
    {
      case 1:
      case 2:
      case 5:
        // 3D view mode
        result = engine_point_to_map(camera,screen_x,screen_y,&x,&y);
        break;
      case 3: //map mode
        result = point_to_overhead_map(camera,screen_x,screen_y,&x,&y);
        break;
      default:
        result = false;
        break;
    }
  }
  if ( result )
  {
    mappos->x.val = x;
    mappos->y.val = y;
  }
  if ( mappos->x.val > ((map_subtiles_x<<8)-1) )
    mappos->x.val = ((map_subtiles_x<<8)-1);
  if ( mappos->y.val > ((map_subtiles_y<<8)-1) )
    mappos->y.val = ((map_subtiles_y<<8)-1);
  //SYNCDBG(19,"Finished");
  return result;
}

void update_breed_activities(void)
{
  _DK_update_breed_activities();
}

short save_settings(void)
{
  char *fname;
  fname=prepare_file_path(FGrp_Save,"settings.dat");
  LbFileSaveAt(fname, &settings, sizeof(struct GameSettings));
  return true;
}

void toggle_hero_health_flowers(void)
{
  const char *statstr;
  toggle_flag_byte(&game.flags_cd,0x80);
  if (game.flags_cd & 0x80)
  {
    statstr = "off";
  } else
  {
    do_sound_menu_click();
    statstr = "on";
  }
  show_onscreen_msg(2*game.num_fps, "Hero health flowers %s", statstr);
}

void zoom_to_map(void)
{
  struct PlayerInfo *player;
  turn_off_all_window_menus();
  if ((game.numfield_C & 0x20) == 0)
    set_flag_byte(&game.numfield_C,0x40,false);
  else
    set_flag_byte(&game.numfield_C,0x40,true);
  player=get_my_player();
  if (((game.system_flags & GSF_NetworkActive) != 0)
      || (lbDisplay.PhysicalScreenWidth > 320))
  {
    if (!toggle_status_menu(false))
      set_flag_byte(&game.numfield_C,0x40,false);
    set_players_packet_action(player, PckA_Unknown119, 4, 0, 0, 0);
    turn_off_roaming_menus();
  } else
  {
    set_players_packet_action(player, PckA_Unknown080, 5, 0, 0, 0);
    turn_off_roaming_menus();
  }
}

void zoom_from_map(void)
{
  struct PlayerInfo *player;
  player=get_my_player();
  if (((game.system_flags & GSF_NetworkActive) != 0)
      || (lbDisplay.PhysicalScreenWidth > 320))
  {
      if ((game.numfield_C & 0x40) != 0)
        toggle_status_menu(true);
      set_players_packet_action(player, PckA_Unknown120,1,0,0,0);
  } else
  {
      set_players_packet_action(player, PckA_Unknown080,6,0,0,0);
  }
}

long can_thing_be_picked_up_by_player(const struct Thing *thing, long plyr_idx)
{
  return _DK_can_thing_be_picked_up_by_player(thing, plyr_idx);
}

long can_thing_be_picked_up2_by_player(const struct Thing *thing, long plyr_idx)
{
  //TODO: rewrite, then give it better name
  return _DK_can_thing_be_picked_up2_by_player(thing, plyr_idx);
}

void remove_events_thing_is_attached_to(struct Thing *thing)
{
  _DK_remove_events_thing_is_attached_to(thing);
}

long take_money_from_dungeon(PlayerNumber plyr_idx, long a2, unsigned char a3)
{
  return _DK_take_money_from_dungeon(plyr_idx, a2, a3);
}

void reset_gui_based_on_player_mode(void)
{
  _DK_reset_gui_based_on_player_mode();
}

TbBool ambient_sound_prepare(void)
{
    struct Thing *thing;
    struct Coord3d pos;
    memset(&pos,0,sizeof(struct Coord3d)); // ambient sound position
    thing = create_ambient_sound(&pos, 1, game.neutral_player_num);
    if (thing_is_invalid(thing))
    {
        game.ambient_sound_thing_idx = 0;
        ERRORLOG("Could not create ambient sound object");
        return false;
    }
    game.ambient_sound_thing_idx = thing->index;
    return true;
}

TbBool ambient_sound_stop(void)
{
    struct Thing *thing;
    thing = thing_get(game.ambient_sound_thing_idx);
    if (thing_is_invalid(thing))
    {
        return false;
    }
    if (thing->snd_emitter_id != 0)
    {
        S3DDestroySoundEmitterAndSamples(thing->snd_emitter_id);
        thing->snd_emitter_id = 0;
    }
    return true;
}

void sound_reinit_after_load(void)
{
    //_DK_sound_reinit_after_load();
    stop_all_things_playing_samples();
    if (SpeechEmitter != 0)
    {
        S3DDestroySoundEmitterAndSamples(SpeechEmitter);
        SpeechEmitter = 0;
    }
    if (Non3DEmitter != 0)
    {
        S3DDestroySoundEmitterAndSamples(Non3DEmitter);
        Non3DEmitter = 0;
    }
    ambient_sound_stop();
    StartMusic(1, 127);
    init_messages();
    randomize_sound_font();
}

void reinit_tagged_blocks_for_player(unsigned char idx)
{
  _DK_reinit_tagged_blocks_for_player(idx);
}

TbBool load_stats_files(void)
{
    int i;
    TbBool result;
    result = true;
    clear_research_for_all_players();
    if (!load_creaturetypes_config(keeper_creaturetp_file,CTLd_KindListOnly))
      result = false;
    if (!load_terrain_config(keeper_terrain_file,0))
      result = false;
    if (!load_trapdoor_config(keeper_trapdoor_file,TDLd_Standard))
      result = false;
    if (!load_lenses_config(keeper_lenses_file,LnLd_Standard))
      result = false;
    if (!load_magic_config(keeper_magic_file,0))
      result = false;
    if (!load_creaturetypes_config(keeper_creaturetp_file,CTLd_Standard))
      result = false;
    if (!load_creaturestates_config(creature_states_file,0))
      result = false;
    // note that rules file requires definitions of magic and creature types
    if (!load_rules_config(keeper_rules_file,0))
      result = false;
    for (i=0; i < crtr_conf.model_count; i++)
    {
      if (!load_creaturemodel_config(i+1,0))
        result = false;
    }
    game.field_149E7B = game.tile_strength;
//  LbFileSaveAt("!stat11", &game, sizeof(struct Game));
//  LbFileSaveAt("!stat12", &shot_stats, sizeof(shot_stats));
//  LbFileSaveAt("!stat13", &instance_info, sizeof(instance_info));
    SYNCDBG(3,"Finished");
    return result;
}


void check_and_auto_fix_stats(void)
{
    struct CreatureStats *crstat;
    long model;
    long i,n;
    SYNCDBG(8,"Starting");
    //_DK_check_and_auto_fix_stats();
    for (model=0; model < CREATURE_TYPES_COUNT; model++)
    {
        crstat = creature_stats_get(model);
        if ( (crstat->lair_size <= 0) && (crstat->heal_requirement != 0) )
        {
            ERRORLOG("Creature model %d No LairSize But Heal Requirment - Fixing", (int)model);
            crstat->heal_threshold = 0;
            crstat->heal_requirement = 0;
        }
        if (crstat->heal_requirement > crstat->heal_threshold)
        {
            ERRORLOG("Creature model %d Heal Requirment > Heal Threshold - Fixing", (int)model);
            crstat->heal_threshold = crstat->heal_requirement;
        }
        if ( (crstat->hunger_rate != 0) && (crstat->hunger_fill == 0) )
        {
            ERRORLOG("Creature model %d HungerRate > 0 & Hunger Fill = 0 - Fixing", (int)model);
            crstat->hunger_fill = 1;
        }
        if ( (crstat->sleep_exp_slab != 0) && (crstat->sleep_experience == 0) )
        {
            ERRORLOG("Creature model %d SleepSlab & SleepExperience = 0 - Fixing", (int)model);
            crstat->sleep_exp_slab = 0;
        }
        if (crstat->grow_up > 30)
        {
            ERRORLOG("Creature model %d Invalid GrowUp - Fixing", (int)model);
            crstat->grow_up = 0;
        }
        if (crstat->grow_up > 0)
        {
          if ( (crstat->grow_up_level < 1) || (crstat->grow_up_level > 10) )
          {
              ERRORLOG("Creature model %d GrowUp & GrowUpLevel invalid - Fixing", (int)model);
              crstat->grow_up_level = 1;
          }
        }
        if (crstat->rebirth > 10)
        {
            ERRORLOG("Creature model %d Rebirth Invalid - Fixing", (int)model);
            crstat->rebirth = 0;
        }
        for (i=0; i < 10; i++)
        {
            n = crstat->instance_level[i];
            if (n != 0)
            {
                if ( (n < 1) || (n > 10) )
                {
                    ERRORLOG("Creature model %d Instance Level For Slot %d Invalid - Fixing", (int)model, (int)(i+1));
                    crstat->instance_level[i] = 1;
                }
            } else
            {
                if ( (n >= 1) && (n <= 10) )
                {
                    ERRORLOG("Creature model %d Instance Level For Not Used Spell %d - Fixing", (int)model, (int)(i+1));
                    crstat->instance_level[i] = 0;
                }
            }
        }
    }
    SYNCDBG(9,"Finished");
}

long update_dungeon_scores(void)
{
  return _DK_update_dungeon_scores();
}

long update_dungeon_generation_speeds(void)
{
  return _DK_update_dungeon_generation_speeds();
}

void calculate_dungeon_area_scores(void)
{
  _DK_calculate_dungeon_area_scores();
}

struct GoldLookup *get_gold_lookup(long idx)
{
    return &game.gold_lookup[idx];
}

/** Finds a gold vein with smaller amount of gold and gem slabs than given values.
 *  Gems slabs count has higher priority than gold slabs count.
 *
 * @param higher_gold_slabs
 * @param higher_gem_slabs
 * @return
 */
long smaller_gold_vein_lookup_idx(long higher_gold_slabs, long higher_gem_slabs)
{
    struct GoldLookup *gldlook;
    long gold_slabs, gem_slabs;
    long gold_idx;
    long i;
    gold_slabs = higher_gold_slabs;
    gem_slabs = higher_gem_slabs;
    gold_idx = -1;
    for (i=0; i < GOLD_LOOKUP_COUNT; i++)
    {
        gldlook = get_gold_lookup(i);
        if (gldlook->field_10 == gem_slabs)
        {
            if (gldlook->field_A < gold_slabs)
            {
              gold_slabs = gldlook->field_A;
              gold_idx = i;
            }
        } else
        if (gldlook->field_10 < gem_slabs)
        {
            gem_slabs = gldlook->field_10;
            gold_slabs = gldlook->field_A;
            gold_idx = i;
        }
    }
    return gold_idx;
}

void check_treasure_map(unsigned char *treasure_map, unsigned short *vein_list, long *gold_next_idx, MapSlabCoord veinslb_x, MapSlabCoord veinslb_y)
{
    struct GoldLookup *gldlook;
    struct SlabMap *slb;
    SlabCodedCoords slb_num,slb_around;
    MapSlabCoord slb_x,slb_y;
    long gold_slabs,gem_slabs;
    long vein_total,vein_idx;
    long gld_v1,gld_v2,gld_v3;
    long gold_idx;
    // First, find a vein
    vein_total = 0;
    slb_x = veinslb_x;
    slb_y = veinslb_y;
    gld_v1 = 0;
    gld_v2 = 0;
    gld_v3 = 0;
    gem_slabs = 0;
    gold_slabs = 0;
    slb_num = get_slab_number(slb_x, slb_y);
    treasure_map[slb_num] |= 0x02;
    for (vein_idx=0; vein_idx <= vein_total; vein_idx++)
    {
        gld_v1 += slb_x;
        gld_v2 += slb_y;
        gld_v3++;
        slb_around = get_slab_number(slb_x, slb_y);
        slb = get_slabmap_direct(slb_around);
        if (slb->kind == SlbT_GEMS)
        {
            gem_slabs++;
        } else
        {
            gold_slabs++;
            slb_around = get_slab_number(slb_x-1, slb_y);
            if ((treasure_map[slb_around] & 0x03) == 0)
            {
                treasure_map[slb_around] |= 0x02;
                vein_list[vein_total] = slb_around;
                vein_total++;
            }
            slb_around = get_slab_number(slb_x+1, slb_y);
            if ((treasure_map[slb_around] & 0x03) == 0)
            {
                treasure_map[slb_around] |= 0x02;
                vein_list[vein_total] = slb_around;
                vein_total++;
            }
            slb_around = get_slab_number(slb_x, slb_y-1);
            if ((treasure_map[slb_around] & 0x03) == 0)
            {
                treasure_map[slb_around] |= 0x02;
                vein_list[vein_total] = slb_around;
                vein_total++;
            }
            slb_around = get_slab_number(slb_x, slb_y+1);
            if ((treasure_map[slb_around] & 0x03) == 0)
            {
                treasure_map[slb_around] |= 0x02;
                vein_list[vein_total] = slb_around;
                vein_total++;
            }
        }
        // Move to next slab in list
        slb_x = slb_num_decode_x(vein_list[vein_idx]);
        slb_y = slb_num_decode_y(vein_list[vein_idx]);
    }
    // Now get a GoldLookup struct to put the vein into
    if (*gold_next_idx < GOLD_LOOKUP_COUNT)
    {
        gold_idx = *gold_next_idx;
        (*gold_next_idx)++;
    } else
    {
        gold_idx = smaller_gold_vein_lookup_idx(gold_slabs, gem_slabs);
    }
    // Write the vein to GoldLookup item
    if (gold_idx != -1)
    {
        gldlook = get_gold_lookup(gold_idx);
        LbMemorySet(gldlook, 0, sizeof(struct GoldLookup));
        gldlook->field_0 |= 0x01;
        gldlook->x_stl_num = 3 * gld_v1 / gld_v3 + 1;
        gldlook->y_stl_num = 3 * gld_v2 / gld_v3 + 1;
        gldlook->field_A = gold_slabs;
        gldlook->field_C = 0;
        gldlook->field_E = gold_slabs;
        gldlook->field_10 = gem_slabs;
    }
}

void check_map_for_gold(void)
{
    MapSlabCoord slb_x,slb_y;
    struct SlabMap *slb;
    SlabCodedCoords slb_num;
    unsigned char *treasure_map;
    unsigned short *vein_list;
    long gold_next_idx;
    long i;
    SYNCDBG(8,"Starting");
    //_DK_check_map_for_gold();
    for (i=0; i < GOLD_LOOKUP_COUNT; i++) {
        LbMemorySet(&game.gold_lookup[i], 0, sizeof(struct GoldLookup));
    }

    treasure_map = (unsigned char *)scratch;
    vein_list = (unsigned short *)&scratch[map_tiles_x*map_tiles_y];
    for (slb_y = 0; slb_y < map_tiles_y; slb_y++) {
        for (slb_x = 0; slb_x < map_tiles_x; slb_x++) {
            slb_num = get_slab_number(slb_x, slb_y);
            slb = get_slabmap_direct(slb_num);
            treasure_map[slb_num] = 0;
            if ( (slb->kind != SlbT_GOLD) && (slb->kind != SlbT_GEMS) ) {
                treasure_map[slb_num] |= 0x01;
            }
        }
    }
    gold_next_idx = 0;
    for (slb_y = 0; slb_y < map_tiles_y; slb_y++) {
        for (slb_x = 0; slb_x < map_tiles_x; slb_x++) {
            slb_num = get_slab_number(slb_x, slb_y);
            if ( ((treasure_map[slb_num] & 0x01) == 0) && ((treasure_map[slb_num] & 0x02) == 0) )
            {
                check_treasure_map(treasure_map, vein_list, &gold_next_idx, slb_x, slb_y);
            }
        }
    }
    SYNCDBG(8,"Found %ld possible digging locations",gold_next_idx);
}

void gui_set_button_flashing(long btn_idx, long gameturns)
{
    game.flash_button_index = btn_idx;
    game.flash_button_gameturns = gameturns;
}

void instant_instance_selected(long a1)
{
    _DK_instant_instance_selected(a1);
}

unsigned char active_battle_exists(unsigned char a1)
{
  return _DK_active_battle_exists(a1);
}

unsigned char step_battles_forward(unsigned char a1)
{
  return _DK_step_battles_forward(a1);
}

short zoom_to_fight(unsigned char a1)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    player = get_my_player();
    if (active_battle_exists(a1))
    {
        dungeon = get_players_num_dungeon(my_player_number);
        set_players_packet_action(player, 104, dungeon->field_1174, 0, 0, 0);
        step_battles_forward(a1);
        return true;
    }
    return false;
}

TbBool create_random_evil_creature(long x, long y, PlayerNumber owner, long max_lv)
{
    struct Thing *thing;
    struct Coord3d pos;
    long i;
    i = ACTION_RANDOM(17) + 14;
    pos.x.val = x;
    pos.y.val = y;
    pos.z.val = 0;
    thing = create_creature(&pos, i, owner);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Cannot create evil creature type %ld at (%ld,%ld)",i,x,y);
        return false;
    }
    pos.z.val = get_thing_height_at(thing, &pos);
    if (thing_in_wall_at(thing, &pos))
    {
        delete_thing_structure(thing, 0);
        ERRORLOG("Evil creature type %ld at (%ld,%ld) deleted because is in wall",i,x,y);
        return false;
    }
    thing->mappos.x.val = pos.x.val;
    thing->mappos.y.val = pos.y.val;
    thing->mappos.z.val = pos.z.val;
    remove_first_creature(thing);
    set_first_creature(thing);
    set_start_state(thing);
    i = ACTION_RANDOM(max_lv);
    set_creature_level(thing, i);
    return true;
}

TbBool create_random_hero_creature(long x, long y, PlayerNumber owner, long max_lv)
{
  struct Thing *thing;
  struct Coord3d pos;
  long i;
  i = ACTION_RANDOM(13) + 1;
  pos.x.val = x;
  pos.y.val = y;
  pos.z.val = 0;
  thing = create_creature(&pos, i, owner);
  if (thing_is_invalid(thing))
  {
      ERRORLOG("Cannot create player %d hero model %ld at (%ld,%ld)",(int)owner,i,x,y);
      return false;
  }
  pos.z.val = get_thing_height_at(thing, &pos);
  if (thing_in_wall_at(thing, &pos))
  {
      delete_thing_structure(thing, 0);
      ERRORLOG("Hero model %ld at (%ld,%ld) deleted because is in wall",i,x,y);
      return false;
  }
  thing->mappos.x.val = pos.x.val;
  thing->mappos.y.val = pos.y.val;
  thing->mappos.z.val = pos.z.val;
  remove_first_creature(thing);
  set_first_creature(thing);
//  set_start_state(thing); - simplified to the following two commands
  game.field_14E498 = game.play_gameturn;
  game.field_14E49C++;
  i = ACTION_RANDOM(max_lv);
  set_creature_level(thing, i);
  return true;
}

TbBool create_hero_special_worker(long x, long y, PlayerNumber owner)
{
    struct Thing *thing;
    struct Coord3d pos;
    long i;
    i = 8;
    pos.x.val = x;
    pos.y.val = y;
    pos.z.val = 0;
    thing = create_creature(&pos, i, owner);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Cannot create hero creature type %ld at (%ld,%ld)",i,x,y);
        return false;
    }
    pos.z.val = get_thing_height_at(thing, &pos);
    if (thing_in_wall_at(thing, &pos))
    {
        delete_thing_structure(thing, 0);
        ERRORLOG("Hero creature type %ld at (%ld,%ld) deleted because is in wall",i,x,y);
        return false;
    }
    thing->mappos.x.val = pos.x.val;
    thing->mappos.y.val = pos.y.val;
    thing->mappos.z.val = pos.z.val;
    remove_first_creature(thing);
    set_first_creature(thing);
    return true;
}

short zoom_to_next_annoyed_creature(void)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct Thing *thing;
  player = get_my_player();
  dungeon = get_players_num_dungeon(my_player_number);
  dungeon->zoom_annoyed_creature_idx = find_next_annoyed_creature(player->id_number,dungeon->zoom_annoyed_creature_idx);
  thing = thing_get(dungeon->zoom_annoyed_creature_idx);
  if (thing_is_invalid(thing))
  {
    return false;
  }
  set_players_packet_action(player, PckA_Unknown087, thing->mappos.x.val, thing->mappos.y.val, 0, 0);
  return true;
}

void go_to_my_next_room_of_type(unsigned long rkind)
{
  _DK_go_to_my_next_room_of_type(rkind);
}

short toggle_computer_player(int idx)
{
  struct Dungeon *dungeon;
  dungeon = get_players_num_dungeon(idx);
  if (dungeon_invalid(dungeon))
    return false;
  toggle_flag_byte(&dungeon->computer_enabled,0x01);
  return true;
}

TbBool load_texture_map_file(unsigned long tmapidx, unsigned char n)
{
    char *fname;
    SYNCDBG(7,"Starting");
    fname = prepare_file_fmtpath(FGrp_StdData,"tmapa%03d.dat",tmapidx);
    if (!wait_for_cd_to_be_available())
        return false;
    if (!LbFileExists(fname))
    {
        WARNMSG("Texture file \"%s\" doesn't exits.",fname);
        return false;
    }
    // The texture file has always over 500kb
    if (LbFileLoadAt(fname, block_mem) < 65536)
    {
        WARNMSG("Texture file \"%s\" can't be loaded or is too small.",fname);
        return false;
    }
    return true;
}

void reinit_level_after_load(void)
{
    struct PlayerInfo *player;
    int i;
    SYNCDBG(6,"Starting");
    player = get_my_player();
    player->field_7 = 0;
    init_lookups();
    init_navigation();
    reinit_packets_after_load();
    parchment_loaded = 0;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if (player_exists(player))
        set_engine_view(player, player->view_mode);
    }
    start_rooms = &game.rooms[1];
    end_rooms = &game.rooms[ROOMS_COUNT];
    load_texture_map_file(game.texture_id, 2);
    init_animating_texture_maps();
    load_computer_player_config();
    init_gui();
    reset_gui_based_on_player_mode();
    erstats_clear();
    player = get_my_player();
    reinit_tagged_blocks_for_player(player->id_number);
    restore_computer_player_after_load();
    sound_reinit_after_load();
}

void create_shadow_limits(long start, long end)
{
    if (start <= end)
    {
        memset(&game.shadow_limits[start], 1, end-start);
    } else
    {
        memset(&game.shadow_limits[start], 1, SHADOW_LIMITS_COUNT-1-start);
        memset(&game.shadow_limits[0], 1, end);
    }
}

void clear_shadow_limits(void)
{
    memset(game.shadow_limits, 0, SHADOW_LIMITS_COUNT);
}

/**
 * Sets to defaults some basic parameters which are
 * later copied into Game structure.
 */
TbBool set_default_startup_parameters(void)
{
    memset(&start_params, 0, sizeof(struct StartupParameters));
    start_params.packet_checksum = 1;
    set_flag_byte(&start_params.flags_font,FFlg_unk01,false);
    // Set levels to 0, as we may not have the campaign loaded yet
    start_params.selected_level_number = 0;
    start_params.num_fps = 20;
    start_params.one_player = 1;
    set_flag_byte(&start_params.flags_cd,MFlg_IsDemoMode,false);
    set_flag_byte(&start_params.flags_cd,MFlg_unk40,true);
    start_params.force_ppro_poly = 0;
    return true;
}

/**
 * Clears the Game structure completely, and copies statrup parameters
 * from start_params structure.
 */
void clear_complete_game(void)
{
    memset(&game, 0, sizeof(struct Game));
    game.turns_packetoff = -1;
    game.numfield_149F46 = 0;
    game.packet_checksum = start_params.packet_checksum;
    game.numfield_1503A2 = -1;
    game.flags_font = start_params.flags_font;
    game.numfield_149F47 = 0;
    // Set levels to 0, as we may not have the campaign loaded yet
    set_continue_level_number(first_singleplayer_level());
    if ((start_params.numfield_C & 0x02) != 0)
      set_selected_level_number(start_params.selected_level_number);
    else
      set_selected_level_number(first_singleplayer_level());
    game.num_fps = start_params.num_fps;
    game.flags_cd = start_params.flags_cd;
    game.no_intro = start_params.no_intro;
    set_flag_byte(&game.system_flags,GSF_AllowOnePlayer,start_params.one_player);
  //  game.one_player = start_params.one_player;
    game.numfield_C = start_params.numfield_C;
    strncpy(game.packet_fname,start_params.packet_fname,150);
    game.packet_save_enable = start_params.packet_save_enable;
    game.packet_load_enable = start_params.packet_load_enable;
    my_player_number = default_loc_player;
}

void clear_slabsets(void)
{
    struct SlabSet *sset;
    struct SlabObj *sobj;
    int i;
    for (i=0; i < SLABSET_COUNT; i++)
    {
        sset = &game.slabset[i];
        memset(sset, 0, sizeof(struct SlabSet));
        game.slabobjs_idx[i] = -1;
    }
    game.slabset_num = SLABSET_COUNT;
    game.slabobjs_num = 0;
    for (i=0; i < SLABOBJS_COUNT; i++)
    {
        sobj = &game.slabobjs[i];
        memset(sobj, 0, sizeof(struct SlabObj));
    }
}

void clear_map(void)
{
    clear_mapmap();
    clear_slabs();
    clear_columns();
    clear_slabsets();
}

void clear_things_and_persons_data(void)
{
    struct Thing *thing;
    long i;
    for (i=0; i < THINGS_COUNT; i++)
    {
        thing = &game.things_data[i];
        memset(thing, 0, sizeof(struct Thing));
        thing->owner = PLAYERS_COUNT;
        thing->mappos.x.val = get_subtile_center_pos(map_subtiles_x/2);
        thing->mappos.y.val = get_subtile_center_pos(map_subtiles_y/2);
    }
    for (i=0; i < CREATURES_COUNT; i++)
    {
      memset(&game.cctrl_data[i], 0, sizeof(struct CreatureControl));
    }
}

void clear_computer(void)
{
    long i;
    SYNCDBG(8,"Starting");
    for (i=0; i < COMPUTER_TASKS_COUNT; i++)
    {
      memset(&game.computer_task[i], 0, sizeof(struct ComputerTask));
    }
    for (i=0; i < GOLD_LOOKUP_COUNT; i++)
    {
      memset(&game.gold_lookup[i], 0, sizeof(struct GoldLookup));
    }
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      memset(&game.computer[i], 0, sizeof(struct Computer2));
    }
}

void clear_rooms(void)
{
  int i;
  for (i=0; i < ROOMS_COUNT; i++)
  {
    memset(&game.rooms[i], 0, sizeof(struct Room));
  }
}

void clear_events(void)
{
    int i;
    for (i=0; i < EVENTS_COUNT; i++)
    {
      memset(&game.event[i], 0, sizeof(struct Event));
    }
    memset(&game.evntbox_scroll_window, 0, sizeof(struct TextScrollWindow));
    memset(&game.evntbox_text_buffer, 0, MESSAGE_TEXT_LEN);
    memset(&game.evntbox_text_objective, 0, MESSAGE_TEXT_LEN);
    for (i=0; i < 5; i++)
    {
      memset(&game.bookmark[i], 0, sizeof(struct Bookmark));
    }
}

void init_keepers_map_exploration(void)
{
    struct PlayerInfo *player;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if (player_exists(player) && (player->field_2C == 1))
      {
          if ((player->field_0 & 0x40) != 0)
            init_keeper_map_exploration(player);
      }
    }
}

void clear_players_for_save(void)
{
    struct PlayerInfo *player;
    unsigned short id_mem,mem2,memflg;
    struct Camera cammem;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      id_mem = player->id_number;
      mem2 = player->field_2C;
      memflg = player->field_0;
      LbMemoryCopy(&cammem,&player->cameras[1],sizeof(struct Camera));
      memset(player, 0, sizeof(struct PlayerInfo));
      player->id_number = id_mem;
      player->field_2C = mem2;
      set_flag_byte(&player->field_0,0x01,((memflg & 0x01) != 0));
      set_flag_byte(&player->field_0,0x40,((memflg & 0x40) != 0));
      LbMemoryCopy(&player->cameras[1],&cammem,sizeof(struct Camera));
      player->acamera = &player->cameras[1];
    }
}

void delete_all_thing_structures(void)
{
    long i;
    struct Thing *thing;
    for (i=1; i < THINGS_COUNT; i++)
    {
      thing = thing_get(i);
      if (thing_exists(thing)) {
          delete_thing_structure(thing, 1);
      }
    }
    for (i=0; i < THINGS_COUNT-1; i++) {
      game.free_things[i] = i+1;
    }
    game.free_things_start_index = 0;
}

void delete_all_structures(void)
{
    SYNCDBG(6,"Starting");
    delete_all_thing_structures();
    delete_all_control_structures();
    delete_all_room_structures();
    delete_all_action_point_structures();
    light_initialise();
    SYNCDBG(16,"Done");
}

void clear_game_for_summary(void)
{
    SYNCDBG(6,"Starting");
    delete_all_structures();
    clear_shadow_limits();
    clear_stat_light_map();
    clear_mapwho();
    game.entrance_room_id = 0;
    game.action_rand_seed = 0;
    set_flag_byte(&game.numfield_C,0x04,false);
    clear_columns();
    clear_action_points();
    clear_players();
    clear_dungeons();
}

void clear_game(void)
{
    SYNCDBG(6,"Starting");
    clear_game_for_summary();
    game.audiotrack = 0;
    clear_map();
    clear_computer();
    clear_script();
    clear_events();
    clear_things_and_persons_data();
    ceiling_set_info(12, 4, 1);
    init_animating_texture_maps();
}

void clear_game_for_save(void)
{
    delete_all_structures();
    light_initialise();
    clear_mapwho();
    game.entrance_room_id = 0;
    game.action_rand_seed = 0;
    set_flag_byte(&game.numfield_C,0x04,false);
    clear_columns();
    clear_players_for_save();
    clear_dungeons();
}

void reset_creature_max_levels(void)
{
    struct Dungeon *dungeon;
    int i,k;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
        for (k=1; k < CREATURE_TYPES_COUNT; k++)
        {
            dungeon = get_dungeon(i);
            dungeon->creature_max_level[k] = CREATURE_MAX_LEVEL+1;
        }
    }
}

void reset_script_timers_and_flags(void)
{
    struct Dungeon *dungeon;
    int i,k;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
      dungeon = get_dungeon(i);
      dungeon->magic_resrchable[18] = 1;
      dungeon->magic_level[18] = 1;
      for (k=0; k<TURN_TIMERS_COUNT; k++)
      {
        memset(&dungeon->turn_timers[k], 0, sizeof(struct TurnTimer));
        dungeon->turn_timers[k].state = 0;
      }
      for (k=0; k<SCRIPT_FLAGS_COUNT; k++)
      {
        dungeon->script_flags[k] = 0;
      }
    }
}

void init_good_player_as(long plr_idx)
{
    struct PlayerInfo *player;
    game.hero_player_num = plr_idx;
    player = get_player(plr_idx);
    player->field_0 |= 0x01;
    player->field_0 |= 0x40;
    player->id_number = game.hero_player_num;
}

void message_add(char c)
{
    _DK_message_add(c);
}

void change_engine_window_relative_size(long w_delta, long h_delta)
{
    struct PlayerInfo *myplyr;
    myplyr=get_my_player();
    setup_engine_window(myplyr->engine_window_x, myplyr->engine_window_y,
        myplyr->engine_window_width+w_delta, myplyr->engine_window_height+h_delta);
}

void PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *pal)
{
    if (pal == blue_palette)
    {
      if ((player->field_3 & 0x04) == 0)
        return;
      player->field_3 |= 0x04;
    } else
    {
      player->field_3 &= 0xFB;
    }
    if ( (player->field_7 == 0) || ((pal != player->palette) && (pal == player->field_7)) )
    {
        player->palette = pal;
        player->field_4C1 = 0;
        player->field_4C5 = 0;
        if (is_my_player(player))
        {
            LbScreenWaitVbi();
            LbPaletteSet(pal);
        }
    }
}

TbBool set_gamma(char corrlvl, TbBool do_set)
{
    char *fname;
    TbBool result = true;
    if (corrlvl < 0)
      corrlvl = 0;
    else
    if (corrlvl > 4)
      corrlvl = 4;
    settings.gamma_correction = corrlvl;
    fname=prepare_file_fmtpath(FGrp_StdData,"pal%05d.dat",settings.gamma_correction);
    if (!LbFileExists(fname))
    {
      WARNMSG("Palette file \"%s\" doesn't exist.", fname);
      result = false;
    }
    if (result)
    {
      result = (LbFileLoadAt(fname, _DK_palette) != -1);
    }
    if ((result) && (do_set))
    {
      struct PlayerInfo *myplyr;
      myplyr=get_my_player();
      PaletteSetPlayerPalette(myplyr, _DK_palette);
    }
    if (!result)
      ERRORLOG("Can't load palette file.");
    return result;
}

void centre_engine_window(void)
{
    long x1,y1;
    struct PlayerInfo *player=get_my_player();
    if ((game.numfield_C & 0x20) != 0)
      x1 = (MyScreenWidth-player->engine_window_width-status_panel_width) / 2 + status_panel_width;
    else
      x1 = (MyScreenWidth-player->engine_window_width) / 2;
    y1 = (MyScreenHeight-player->engine_window_height) / 2;
    setup_engine_window(x1, y1, player->engine_window_width, player->engine_window_height);
}

long dummy_sound_line_of_sight(long a1, long a2, long a3, long a4, long a5, long a6)
{
    return 1;
}

void set_sprite_view_3d(void)
{
    _DK_set_sprite_view_3d();
}

void set_sprite_view_isometric(void)
{
    _DK_set_sprite_view_isometric();
}

void set_engine_view(struct PlayerInfo *player, long val)
{
    //_DK_set_engine_view(player, val);
    switch ( val )
    {
    case 1:
      player->acamera = &player->cameras[1];
      if (!is_my_player(player))
        break;
      lens_mode = 2;
      set_sprite_view_3d();
      S3DSetLineOfSightFunction(dummy_sound_line_of_sight);
      S3DSetDeadzoneRadius(0);
      LbMouseSetPosition((MyScreenWidth/pixel_size) >> 1,(MyScreenHeight/pixel_size) >> 1);
      break;
    case 2:
      player->acamera = &player->cameras[0];
      if (!is_my_player(player))
        break;
      lens_mode = 0;
      set_sprite_view_isometric();
      S3DSetLineOfSightFunction(dummy_sound_line_of_sight);
      S3DSetDeadzoneRadius(1280);
      break;
    case 3:
      player->acamera = &player->cameras[2];
      if (!is_my_player(player))
        break;
      S3DSetLineOfSightFunction(dummy_sound_line_of_sight);
      S3DSetDeadzoneRadius(1280);
      break;
    case 5:
      player->acamera = &player->cameras[3];
      if (!is_my_player(player))
        break;
      lens_mode = 0;
      set_sprite_view_isometric();
      S3DSetLineOfSightFunction(dummy_sound_line_of_sight);
      S3DSetDeadzoneRadius(1280);
      break;
    default:
      break;
    }
    player->view_mode = val;
}

void set_player_state(struct PlayerInfo *player, short nwrk_state, long chosen_kind)
{
  struct Thing *thing;
  struct Coord3d pos;
  //_DK_set_player_state(player, nwrk_state, chosen_kind);
  // Selecting the same state again - update only 2nd parameter
  if (player->work_state == nwrk_state)
  {
    switch ( player->work_state )
    {
    case PSt_BuildRoom:
        player->chosen_room_kind = chosen_kind;
        break;
    case PSt_PlaceTrap:
        player->chosen_trap_kind = chosen_kind;
        break;
    case PSt_PlaceDoor:
        player->chosen_door_kind = chosen_kind;
        break;
    }
    return;
  }
  player->continue_work_state = player->work_state;
  player->work_state = nwrk_state;
  if (is_my_player(player))
    game.field_14E92E = 0;
  if ((player->work_state != PSt_Unknown12) && (player->work_state != PSt_Unknown15)
     && (player->work_state != PSt_CtrlDirect) && (player->work_state != PSt_CtrlPassngr))
  {
    player->controlled_thing_idx = 0;
  }
  switch (player->work_state)
  {
  case PSt_CtrlDungeon:
      player->field_4A4 = 1;
      break;
  case PSt_BuildRoom:
      player->chosen_room_kind = chosen_kind;
      break;
  case PSt_Unknown5:
      create_power_hand(player->id_number);
      break;
  case PSt_Slap:
      pos.x.val = 0;
      pos.y.val = 0;
      pos.z.val = 0;
      thing = create_object(&pos, 37, player->id_number, -1);
      if (thing_is_invalid(thing))
      {
        player->hand_thing_idx = 0;
        break;
      }
      player->hand_thing_idx = thing->index;
      set_power_hand_graphic(player->id_number, 785, 256);
      place_thing_in_limbo(thing);
      break;
  case PSt_PlaceTrap:
      player->chosen_trap_kind = chosen_kind;
      break;
  case PSt_PlaceDoor:
      player->chosen_door_kind = chosen_kind;
      break;
  default:
      break;
  }
}

void set_player_mode(struct PlayerInfo *player, long nview)
{
  long i;
  if (player->view_type == nview)
    return;
  player->view_type = nview;
  player->field_0 &= 0xF7;
  if (is_my_player(player))
  {
    game.numfield_D &= 0xF7;
    game.numfield_D |= 0x01;
    if (is_my_player(player))
      stop_all_things_playing_samples();
  }
  switch (player->view_type)
  {
  case PVT_DungeonTop:
      i = 2;
      if (player->field_4B5 == 5)
      {
        set_engine_view(player, 2);
        i = 5;
      }
      set_engine_view(player, i);
      if (is_my_player(player))
        toggle_status_menu((game.numfield_C & 0x40) != 0);
      if ((game.numfield_C & 0x20) != 0)
        setup_engine_window(status_panel_width, 0, MyScreenWidth, MyScreenHeight);
      else
        setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
      break;
  case PVT_CreatureContrl:
  case PVT_CreaturePasngr:
      set_engine_view(player, 1);
      if (is_my_player(player))
        game.numfield_D &= ~0x01;
      setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
      break;
  case PVT_MapScreen:
      player->continue_work_state = player->work_state;
      set_engine_view(player, 3);
      break;
  case PVT_MapFadeIn:
      set_player_instance(player, PI_MapFadeTo, 0);
      break;
  case PVT_MapFadeOut:
      set_player_instance(player, PI_MapFadeFrom, 0);
      break;
  }
}

void turn_off_query(short a)
{
  _DK_turn_off_query(a);
}

void turn_off_call_to_arms(long a)
{
  _DK_turn_off_call_to_arms(a);
}

long battle_move_player_towards_battle(struct PlayerInfo *player, long var)
{
  return _DK_battle_move_player_towards_battle(player, var);
}

long set_autopilot_type(unsigned int plridx, long aptype)
{
  return _DK_set_autopilot_type(plridx, aptype);
}

unsigned char find_first_battle_of_mine(unsigned char idx)
{
  return _DK_find_first_battle_of_mine(idx);
}

void level_lost_go_first_person(long plyr_idx)
{
  struct CreatureControl *cctrl;
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct Thing *thing;
  long spectator_breed;
  SYNCDBG(6,"Starting for player %ld\n",plyr_idx);
  //_DK_level_lost_go_first_person(plridx);
  player = get_player(plyr_idx);
  dungeon = get_dungeon(player->id_number);
  spectator_breed = get_players_spectator_breed(plyr_idx);
  player->dungeon_camera_zoom = get_camera_zoom(player->acamera);
  thing = create_and_control_creature_as_controller(player, spectator_breed, &dungeon->mappos);
  if (thing_is_invalid(thing))
  {
    ERRORLOG("Unable to create spectator creature");
    return;
  }
  cctrl = creature_control_get_from_thing(thing);
  cctrl->flgfield_1 |= CCFlg_NoCompControl;
  SYNCDBG(8,"Finished");
}

void find_map_location_coords(long location, long *x, long *y, const char *func_name)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct ActionPoint *apt;
  struct Thing *thing;
  long pos_x,pos_y;
  long i;
  SYNCDBG(15,"From %s; Location %ld, pos(%ld,%ld)",func_name, location, *x, *y);
  pos_y = 0;
  pos_x = 0;
  switch (get_map_location_type(location))
  {
  case MLoc_ACTIONPOINT:
      i = get_map_location_longval(location);
      apt = action_point_get_by_number(i);
      if (!action_point_is_invalid(apt))
      {
        pos_y = apt->mappos.y.stl.num;
        pos_x = apt->mappos.x.stl.num;
      } else
        WARNMSG("%s: Action Point %d location for not found",func_name,i);
      break;
  case MLoc_HEROGATE:
      i = get_map_location_longval(location);
      thing = find_hero_gate_of_number(i);
      if (!thing_is_invalid(thing))
      {
        pos_y = thing->mappos.y.stl.num;
        pos_x = thing->mappos.x.stl.num;
      } else
        WARNMSG("%s: Hero Gate %d location not found",func_name,i);
      break;
  case MLoc_PLAYERSHEART:
      i = get_map_location_longval(location);
      if (i < PLAYERS_COUNT)
      {
        player = get_player(i);
        dungeon = get_dungeon(player->id_number);
        thing = thing_get(dungeon->dnheart_idx);
      } else
        thing = NULL;
      if (!thing_is_invalid(thing))
      {
        pos_y = thing->mappos.y.stl.num;
        pos_x = thing->mappos.x.stl.num;
      } else
        WARNMSG("%s: Dungeon Heart location for player %d not found",func_name,i);
      break;
  case MLoc_NONE:
      pos_y = *y;
      pos_x = *x;
      break;
  case MLoc_THING:
      i = get_map_location_longval(location);
      thing = thing_get(i);
      if (!thing_is_invalid(thing))
      {
        pos_y = thing->mappos.y.stl.num;
        pos_x = thing->mappos.x.stl.num;
      } else
        WARNMSG("%s: Thing %d location not found",func_name,i);
      break;
  case MLoc_CREATUREBREED:
  case MLoc_OBJECTKIND:
  case MLoc_ROOMKIND:
  default:
        WARNMSG("%s: Unsupported location, %lu.",func_name,location);
      break;
  }
  *y = pos_y;
  *x = pos_x;
}

void set_general_information(long msg_id, long target, long x, long y)
{
  struct PlayerInfo *player;
  long pos_x,pos_y;
  player = get_my_player();
  find_map_location_coords(target, &x, &y, __func__);
  pos_x = 0;
  pos_y = 0;
  if ((x != 0) || (y != 0))
  {
    pos_y = (y << 8) + 128;
    pos_x = (x << 8) + 128;
  }
  event_create_event(pos_x, pos_y, 21, player->id_number, -msg_id);
}

void set_quick_information(long msg_id, long target, long x, long y)
{
    struct PlayerInfo *player;
    long pos_x,pos_y;
    player = get_my_player();
    find_map_location_coords(target, &x, &y, __func__);
    pos_x = 0;
    pos_y = 0;
    if ((x != 0) || (y != 0))
    {
      pos_y = (y << 8) + 128;
      pos_x = (x << 8) + 128;
    }
    event_create_event(pos_x, pos_y, 27, player->id_number, -msg_id);
}

void set_general_objective(long msg_id, long target, long x, long y)
{
    process_objective(campaign.strings[msg_id%STRINGS_MAX], target, x, y);
}

void process_objective(char *msg_text, long target, long x, long y)
{
    struct PlayerInfo *player;
    long pos_x,pos_y;
    player = get_my_player();
    find_map_location_coords(target, &x, &y, __func__);
    pos_y = y;
    pos_x = x;
    set_level_objective(msg_text);
    display_objectives(player->id_number, pos_x, pos_y);
}

void directly_cast_spell_on_thing(long plridx, unsigned char a2, unsigned short a3, long a4)
{
  _DK_directly_cast_spell_on_thing(plridx, a2, a3, a4);
}

short winning_player_quitting(struct PlayerInfo *player, long *plyr_count)
{
    struct PlayerInfo *swplyr;
    int i,k,n;
    if (player->victory_state == VicS_LostLevel)
    {
      return 0;
    }
    k = 0;
    n = 0;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      swplyr = get_player(i);
      if (player_exists(swplyr))
      {
        if (swplyr->field_2C == 1)
        {
          k++;
          if (swplyr->victory_state == VicS_LostLevel)
            n++;
        }
      }
    }
    *plyr_count = k;
    return ((k - n) == 1);
}

short lose_level(struct PlayerInfo *player)
{
    if (!is_my_player(player))
        return false;
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
        LbNetwork_Stop();
    }
    quit_game = 1;
    return true;
}

short resign_level(struct PlayerInfo *player)
{
    if (!is_my_player(player))
        return false;
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
        LbNetwork_Stop();
    }
    quit_game = 1;
    return true;
}

short player_has_won(long plyr_idx)
{
  struct PlayerInfo *player;
  player = get_player(plyr_idx);
  if (player_invalid(player))
    return false;
  return (player->victory_state == VicS_WonLevel);
}

short player_has_lost(long plyr_idx)
{
  struct PlayerInfo *player;
  player = get_player(plyr_idx);
  if (player_invalid(player))
    return false;
  return (player->victory_state == VicS_LostLevel);
}

long compute_player_final_score(struct PlayerInfo *player,long gameplay_score)
{
  struct PlayerInfo *myplyr;
  long i;
  myplyr = get_my_player();
  if (((game.system_flags & GSF_NetworkActive) != 0)
      || !is_singleplayer_level(game.loaded_level_number))
    i = 2 * gameplay_score;
  else
    i = gameplay_score + 10 * gameplay_score * array_index_for_singleplayer_level(game.loaded_level_number) / 100;
  if (player_has_lost(player->id_number))
    i /= 2;
  return i;
}

void set_player_as_won_level(struct PlayerInfo *player)
{
  struct Dungeon *dungeon;
  if (player->victory_state != VicS_Undecided)
  {
      WARNLOG("Player fate is already decided to %d",(int)player->victory_state);
      return;
  }
  if (is_my_player(player))
    frontstats_initialise();
  player->victory_state = VicS_WonLevel;
  dungeon = get_dungeon(player->id_number);
  // Computing player score
  dungeon->lvstats.player_score = compute_player_final_score(player, dungeon->field_AE9[2]);
  dungeon->lvstats.allow_save_score = 1;
  if ((game.system_flags & GSF_NetworkActive) == 0)
    player->field_4EB = game.play_gameturn + 300;
  if (is_my_player(player))
  {
    if (lord_of_the_land_in_prison_or_tortured())
    {
        SYNCLOG("Lord Of The Land kept captive. Torture tower unlocked.");
        player->field_3 |= 0x10;
    }
    output_message(SMsg_LevelWon, 0, true);
  }
}

void set_player_as_lost_level(struct PlayerInfo *player)
{
  struct Dungeon *dungeon;
  struct Thing *thing;
  if (player->victory_state != VicS_Undecided)
  {
      WARNLOG("Victory state already set to %d",(int)player->victory_state);
      return;
  }
  if (is_my_player(player))
    frontstats_initialise();
  player->victory_state = VicS_LostLevel;
  dungeon = get_dungeon(player->id_number);
  // Computing player score
  dungeon->lvstats.player_score = compute_player_final_score(player, dungeon->field_AE9[2]);
  if (is_my_player(player))
  {
    output_message(SMsg_LevelFailed, 0, true);
    turn_off_all_menus();
    clear_transfered_creature();
  }
  clear_things_in_hand(player);
  dungeon->field_63 = 0;
  if (dungeon->field_884 != 0)
    turn_off_call_to_arms(player->id_number);
  if (dungeon->keeper_sight_thing_idx > 0)
  {
    thing = thing_get(dungeon->keeper_sight_thing_idx);
    delete_thing_structure(thing, 0);
    dungeon->keeper_sight_thing_idx = 0;
  }
  if (is_my_player(player))
    gui_set_button_flashing(0, 0);
  set_player_mode(player, 1);
  set_player_state(player, 1, 0);
  if ((game.system_flags & GSF_NetworkActive) == 0)
    player->field_4EB = game.play_gameturn + 300;
  if ((game.system_flags & GSF_NetworkActive) != 0)
    reveal_whole_map(player);
  if ((dungeon->computer_enabled & 0x01) != 0)
    toggle_computer_player(player->id_number);
}

TbBool move_campaign_to_next_level(void)
{
  long lvnum;
  long curr_lvnum;
  curr_lvnum = get_continue_level_number();
  lvnum = next_singleplayer_level(curr_lvnum);
  if (lvnum != LEVELNUMBER_ERROR)
  {
    set_continue_level_number(lvnum);
    SYNCDBG(8,"Continue level moved to %ld.",lvnum);
    return true;
  } else
  {
    set_continue_level_number(SINGLEPLAYER_NOTSTARTED);
    SYNCDBG(8,"Continue level moved to NOTSTARTED.");
    return false;
  }
}

TbBool move_campaign_to_prev_level(void)
{
    long lvnum;
    long curr_lvnum;
    curr_lvnum = get_continue_level_number();
    lvnum = prev_singleplayer_level(curr_lvnum);
    if (lvnum != LEVELNUMBER_ERROR)
    {
        set_continue_level_number(lvnum);
        SYNCDBG(8,"Continue level moved to %ld.",lvnum);
        return true;
    } else
    {
        set_continue_level_number(SINGLEPLAYER_FINISHED);
        SYNCDBG(8,"Continue level moved to FINISHED.");
        return false;
    }
}

short complete_level(struct PlayerInfo *player)
{
    long lvnum;
    SYNCDBG(6,"Starting");
    if (!is_my_player(player))
      return false;
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
      LbNetwork_Stop();
      quit_game = 1;
      return true;
    }
    lvnum = get_continue_level_number();
    if (get_loaded_level_number() == lvnum)
    {
      move_campaign_to_next_level();
    }
    set_selected_level_number(get_continue_level_number());
    quit_game = 1;
    return true;
}

void place_animating_slab_type_on_map(long a1, char a2, unsigned char a3, unsigned char a4, unsigned char a5)
{
    SYNCDBG(7,"Starting");
    _DK_place_animating_slab_type_on_map(a1,a2,a3,a4,a5);
}

void init_lookups(void)
{
    long i;
    SYNCDBG(8,"Starting");
    for (i=0; i < THINGS_COUNT; i++)
    {
      game.things.lookup[i] = &game.things_data[i];
    }
    game.things.end = &game.things_data[THINGS_COUNT];

    memset(&game.persons, 0, sizeof(struct Persons));
    for (i=0; i < CREATURES_COUNT; i++)
    {
      game.persons.cctrl_lookup[i] = &game.cctrl_data[i];
    }
    game.persons.cctrl_end = &game.cctrl_data[CREATURES_COUNT];

    for (i=0; i < COLUMNS_COUNT; i++)
    {
      game.columns.lookup[i] = &game.columns_data[i];
    }
    game.columns.end = &game.columns_data[COLUMNS_COUNT];
}

void clear_lookups(void)
{
    long i;
    SYNCDBG(8,"Starting");
    for (i=0; i < THINGS_COUNT; i++)
    {
      game.things.lookup[i] = NULL;
    }
    game.things.end = NULL;

    memset(&game.persons, 0, sizeof(struct Persons));

    for (i=0; i < COLUMNS_COUNT; i++)
    {
      game.columns.lookup[i] = NULL;
    }
    game.columns.end = NULL;
}

void set_mouse_light(struct PlayerInfo *player)
{
  SYNCDBG(6,"Starting");
  _DK_set_mouse_light(player);
}

struct Thing *create_room_surrounding_flame(struct Room *room,struct Coord3d *pos,unsigned short eetype, unsigned short owner)
{
  struct Thing *eething;
  eething = create_effect_element(pos, room_effect_elements[eetype & 7], owner);
  if (!thing_is_invalid(eething))
  {
    eething->mappos.z.val = get_thing_height_at(eething, &eething->mappos);
    eething->mappos.z.val += 10;
    // Size of the flame depends on room effifiency
    eething->field_46 = ((eething->field_46 - 80) * ((long)room->efficiency) / 256) + 80;
  }
  return eething;
}

void room_update_surrounding_flames(struct Room *room, struct Coord3d *pos)
{
    MapSlabCoord x,y;
  long i,k;
  i = room->field_43;
  x = pos->x.stl.num + (MapSlabCoord)small_around[i].delta_x;
  y = pos->y.stl.num + (MapSlabCoord)small_around[i].delta_y;
  if (room != subtile_room_get(x,y))
  {
      k = (i + 1) % 4;
      room->field_43 = k;
      return;
  }
  k = (i + 3) % 4;
  x += (MapSlabCoord)small_around[k].delta_x;
  y += (MapSlabCoord)small_around[k].delta_y;
  if (room == subtile_room_get(x,y))
  {
      room->field_41 += slab_around[i] + slab_around[k];
      room->field_43 = k;
      return;
  }
  room->field_41 += slab_around[i];
}

void process_room_surrounding_flames(struct Room *room)
{
  struct Coord3d pos;
  long x,y;
  long i;
  SYNCDBG(19,"Starting");
  x = 3 * slb_num_decode_x(room->field_41);
  y = 3 * slb_num_decode_y(room->field_41);
  i = 3 * room->field_43 + room->field_44;
  pos.x.val = 256 * (x+1) + room_spark_offset[i].delta_x + 128;
  pos.y.val = 256 * (y+1) + room_spark_offset[i].delta_y + 128;
  pos.z.val = 0;
  // Create new element
  if (room->owner == game.neutral_player_num)
  {
    create_room_surrounding_flame(room,&pos,game.play_gameturn & 3,game.neutral_player_num);
  } else
  if (room_effect_elements[room->owner] != 0)
  {
    create_room_surrounding_flame(room,&pos,room->owner,room->owner);
  }
  // Update coords for next element
  if (room->field_44 == 2)
  {
    room_update_surrounding_flames(room,&pos);
  }
  room->field_44 = (room->field_44 + 1) % 3;
}

void recompute_rooms_count_in_dungeons(void)
{
  SYNCDBG(17,"Starting");
  struct Dungeon *dungeon;
  long i,k;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    dungeon = get_dungeon(i);
    dungeon->buildable_rooms_count = 0;
    for (k = 1; k < 17; k++)
    {
      if ((k != RoK_ENTRANCE) && (k != RoK_DUNGHEART))
      {
        dungeon->buildable_rooms_count += get_player_rooms_count(i, k);
      }
    }
  }
}
void process_rooms(void)
{
  SYNCDBG(7,"Starting");
  struct Room *room;
  struct Packet *pckt;
  //_DK_process_rooms(); return;
  for (room = start_rooms; room < end_rooms; room++)
  {
    if ((room->field_0 & 0x01) == 0)
      continue;
    if (room->kind == RoK_GARDEN)
      room_grow_food(room);
    pckt = get_packet(my_player_number);
    pckt->chksum += (room->slabs_count & 0xFF) + room->central_stl_x + room->central_stl_y;
    if (((game.numfield_D & 0x40) == 0) || (room->kind == RoK_DUNGHEART))
      continue;
    process_room_surrounding_flames(room);
  }
  recompute_rooms_count_in_dungeons();
  SYNCDBG(9,"Finished");
}

void check_players_won(void)
{
  SYNCDBG(8,"Starting");
  _DK_check_players_won();
}

void check_players_lost(void)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct Thing *thing;
  long i;
  SYNCDBG(8,"Starting");
  //_DK_check_players_lost();
  for (i=0; i < PLAYERS_COUNT; i++)
  {
      player = get_player(i);
      if (player_exists(player) && (player->field_2C == 1))
      {
          dungeon = get_players_dungeon(player);
          thing = thing_get(dungeon->dnheart_idx);
          if ((thing_is_invalid(thing) || (thing->active_state == CrSt_ImpArrivesAtDigOrMine2)) && (player->victory_state == VicS_Undecided))
          {
            event_kill_all_players_events(i);
            set_player_as_lost_level(player);
            //TODO: make sure we really want to do this; it wasn't here in oroginal code, but it will prevent computer player activities on dead player.
            player->field_2C = 0;
            if (is_my_player_number(i))
              LbPaletteSet(_DK_palette);
          }
      }
  }
}

void process_dungeon_power_magic(void)
{
  SYNCDBG(8,"Starting");
  _DK_process_dungeon_power_magic();
}

void process_dungeon_devastation_effects(void)
{
  SYNCDBG(8,"Starting");
  _DK_process_dungeon_devastation_effects();
}

/** Checks if an entrance shall now generate next creature.
 *
 * @return Gives true if an entrance shall generate, false otherwise.
 */
TbBool generation_due_in_game(void)
{
    if (game.generate_speed == -1)
        return true;
    return ( (game.play_gameturn-game.entrance_last_generate_turn) >= game.generate_speed );
}

TbBool generation_due_for_dungeon(struct Dungeon * dungeon)
{
    SYNCDBG(9,"Starting");
    if ( (game.field_150356 == 0) || (game.armageddon.count_down + game.field_150356 > game.play_gameturn) )
    {
        if ( (dungeon->turns_between_entrance_generation != -1) &&
             (game.play_gameturn - dungeon->last_entrance_generation_gameturn >= dungeon->turns_between_entrance_generation) ) {
            return true;
        }
    }
    return false;
}

TbBool generation_available_to_dungeon(struct Dungeon * dungeon)
{
    SYNCDBG(9,"Starting");
    if (dungeon->room_kind[RoK_ENTRANCE] <= 0)
        return false;
    return ((long)dungeon->num_active_creatrs < (long)dungeon->max_creatures_attracted);
}

long calculate_attractive_room_quantity(RoomKind room_kind, int plyr_idx, int crtr_kind)
{
    struct Dungeon * dungeon;
    long used_fraction;
    long slabs_count;

    dungeon = get_dungeon(plyr_idx);
    slabs_count = get_room_slabs_count(plyr_idx, room_kind);

    switch (room_kind)
    {
    case RoK_NONE:
    case RoK_DUNGHEART:
    case RoK_LAIR:
    case RoK_BRIDGE:
        return -(long)dungeon->owned_creatures_of_model[crtr_kind];
    case RoK_ENTRANCE:
    case RoK_LIBRARY:
    case RoK_PRISON:
    case RoK_TORTURE:
    case RoK_TRAINING:
    case RoK_SCAVENGER:
    case RoK_TEMPLE:
    case RoK_GRAVEYARD:
    case RoK_BARRACKS:
    case RoK_GUARDPOST:
        return slabs_count / 3 - (long)dungeon->owned_creatures_of_model[crtr_kind];
    case RoK_WORKSHOP:
    case RoK_GARDEN:
        return slabs_count / 4 - (long)dungeon->owned_creatures_of_model[crtr_kind];
    case RoK_TREASURE:
        used_fraction = get_room_kind_used_capacity_fraction(plyr_idx, room_kind);
        return (slabs_count * used_fraction) / 256 / 3;
    default:
        return 0;
    }
}

long calculate_excess_attraction_for_creature(int crtr_kind, int plyr_idx)
{
    struct CreatureStats * stats;
    RoomKind room_kind;

    SYNCDBG(11, "Starting");

    stats = creature_stats_get(crtr_kind);
    room_kind = stats->entrance_rooms[0];

    return calculate_attractive_room_quantity(room_kind,
        plyr_idx, crtr_kind);
}

TbBool creature_will_generate_for_dungeon(struct Dungeon * dungeon, int crtr_kind)
{
    RoomKind room_kind;
    struct CreatureStats * stats;
    int i;
    int slabs_count;

    SYNCDBG(11, "Starting for creature kind %s", creature_code_name(crtr_kind));

    if (game.pool.crtr_kind[crtr_kind] <= 0) {
        return false;
    }

    if (!dungeon->creature_enabled[crtr_kind]) {
        return false;
    }

    stats = creature_stats_get(crtr_kind);

    // Check if we've got rooms of enough size for attraction
    for (i = 0; i < 3; ++i) {
        room_kind = stats->entrance_rooms[i];

        if (room_kind != RoK_NONE) {
            slabs_count = get_room_slabs_count(dungeon->owner, room_kind);

            if (slabs_count < stats->entrance_slabs_req[i]) {
                return false;
            }
        }
    }

    return true;
}

int calculate_creature_to_generate_for_dungeon(struct Dungeon * dungeon)
{
    long cum_freq; //cumulative frequency
    long gen_count;
    long crtr_freq[CREATURE_TYPES_COUNT];
    long rnd;
    long score;
    long i;

    SYNCDBG(9,"Starting");

    cum_freq = 0;
    gen_count = 0;
    crtr_freq[0] = 0;
    for (i = 1; i < CREATURE_TYPES_COUNT; ++i) {
        if (creature_will_generate_for_dungeon(dungeon, i)) {
            gen_count += 1;

            score = (long)attract_score[i]
                + calculate_excess_attraction_for_creature(i, dungeon->owner);
            if (score < 1) {
                score = 1;
            }
            cum_freq += score;
            crtr_freq[i] = cum_freq;
        }
        else {
            crtr_freq[i] = 0;
        }
    }

    // Select a creature kind to generate based on score we've got for every kind
    // Scores define a chance of being generated.
    if (gen_count > 0) {
        if (cum_freq > 0) {
            rnd = ACTION_RANDOM(cum_freq);

            i = 1;
            while (rnd >= crtr_freq[i]) {
                ++i;
                if (i >= CREATURE_TYPES_COUNT) {
                    ERRORLOG("Internal problem; got outside of cummulative range.");
                    return 0;
                }
            }

            return i;
        }
        else {
            ERRORLOG("Bad configuration; creature available but no scores for randomization.");
        }
    }

    return 0;
}

int create_creature_at_entrance(struct Room * room, unsigned short crtr_kind)
{
    return _DK_create_creature_at_entrance(room, crtr_kind);
}

TbBool generate_creature_at_random_entrance(struct Dungeon * dungeon, ThingModel crtr_kind)
{
    struct Room * room;

    SYNCDBG(9,"Starting");

    room = pick_random_room(dungeon->owner, RoK_ENTRANCE);
    if (room_is_invalid(room))
    {
        ERRORLOG("Could not get a random entrance for player %d",(int)dungeon->owner);
        return false;
    }
    if (create_creature_at_entrance(room, crtr_kind))
    {
        if (game.pool.crtr_kind[crtr_kind] > 0) {
            game.pool.crtr_kind[crtr_kind] -= 1;
        }
        return true;
    }
    return false;
}

long calculate_free_lair_space(struct Dungeon * dungeon)
{
    SYNCDBG(9,"Starting");
    return _DK_calculate_free_lair_space(dungeon);
}

void generate_creature_for_dungeon(struct Dungeon * dungeon)
{
    ThingModel crkind;
    long lair_space;

    SYNCDBG(9,"Starting");

    crkind = calculate_creature_to_generate_for_dungeon(dungeon);

    if (crkind > 0) {
        lair_space = calculate_free_lair_space(dungeon);
        if ((long)game.creature_stats[crkind].pay > dungeon->total_money_owned)
        {
            if (is_my_player_number(dungeon->owner)) {
                output_message(SMsg_GoldLow, MESSAGE_DELAY_TREASURY, true);
            }
        } else
        if (lair_space > 0)
        {
            generate_creature_at_random_entrance(dungeon, crkind);
        } else
        if (lair_space == 0)
        {
            generate_creature_at_random_entrance(dungeon, crkind);

            if (is_my_player_number(dungeon->owner))
            {
                if (dungeon->room_kind[RoK_LAIR] > 0) {
                    output_message(SMsg_LairTooSmall, 500, true);
                }
                else {
                    output_message(SMsg_RoomLairNeeded, 500, true);
                }
            }

            if (dungeon->room_kind[RoK_LAIR] > 0) {
                event_create_event_or_update_nearby_existing_event(0, 0, 17, dungeon->owner, 0);
            }
        }
    }
}

void process_entrance_generation(void)
{
    struct PlayerInfo *plyr;
    struct Dungeon *dungeon;
    long i;
    SYNCDBG(8,"Starting");
    //_DK_process_entrance_generation();

    if (generation_due_in_game())
    {
        if (game.field_150356 == 0) {
            update_dungeon_scores();
            update_dungeon_generation_speeds();
            game.entrance_last_generate_turn = game.play_gameturn;
        }
    }

    for (i = 0; i < PLAYERS_COUNT; i++)
    {
        plyr = get_player(i);
        if (!player_exists(plyr)) {
            continue;
        }
        if ((plyr->field_2C == 1) && (plyr->victory_state != VicS_LostLevel) )
        {
            dungeon = get_players_dungeon(plyr);
            if (generation_due_for_dungeon(dungeon))
            {
                if (generation_available_to_dungeon(dungeon)) {
                    generate_creature_for_dungeon(dungeon);
                }
                dungeon->last_entrance_generation_gameturn = game.play_gameturn;
            }
            dungeon->field_1485 = 0;
        }
    }
}

void process_payday(void)
{
  _DK_process_payday();
}

void count_dungeon_stuff(void)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  int i;

  game.field_14E4A4 = 0;
  game.field_14E4A0 = 0;
  game.field_14E49E = 0;

  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    dungeon = get_dungeon(i);
    player = get_player(i);
    if (player_exists(player))
    {
      game.field_14E4A0 += dungeon->total_money_owned;
      game.field_14E4A4 += dungeon->num_active_diggers;
      game.field_14E49E += dungeon->num_active_creatrs;
    }
  }
}

void process_dungeons(void)
{
  SYNCDBG(7,"Starting");
  check_players_won();
  check_players_lost();
  process_dungeon_power_magic();
  count_dungeon_stuff();
  process_dungeon_devastation_effects();
  process_entrance_generation();
  process_payday();
  process_things_in_dungeon_hand();
  SYNCDBG(9,"Finished");
}

/**
 * Returns if there is a bonus timer visible on the level.
 */
TbBool bonus_timer_enabled(void)
{
  return ((game.flags_gui & GGUI_CountdownTimer) != 0);
/*  LevelNumber lvnum;
  lvnum = get_loaded_level_number();
  return (is_bonus_level(lvnum) || is_extra_level(lvnum));*/
}

void process_player_research(int plyr_idx)
{
  _DK_process_player_research(plyr_idx);
}

struct Room *player_has_room_of_type(long plyr_idx, long rkind)
{
  return _DK_player_has_room_of_type(plyr_idx, rkind);
}

long count_slabs_of_room_type(long plyr_idx, long rkind)
{
    struct Dungeon *dungeon;
    struct Room *room;
    long nslabs;
    long i;
    unsigned long k;
    nslabs = 0;
    dungeon = get_dungeon(plyr_idx);
    i = dungeon->room_kind[rkind];
    k = 0;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        nslabs += room->slabs_count;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return nslabs;
}

TbBool set_manufacture_level(struct Dungeon *dungeon)
{
    int wrkshp_slabs;
    wrkshp_slabs = count_slabs_of_room_type(dungeon->owner, RoK_WORKSHOP);
    if (wrkshp_slabs <= 9)
    {
        dungeon->manufacture_level = 0;
    } else
    if (wrkshp_slabs <= 16)
    {
        dungeon->manufacture_level = 1;
    } else
    if (wrkshp_slabs <= 25)
    {
        if (wrkshp_slabs == 20) // why there's special code for 20 slabs!?
            dungeon->manufacture_level = 4;
        else
            dungeon->manufacture_level = 2;
    } else
    if (wrkshp_slabs <= 36)
    {
        dungeon->manufacture_level = 3;
    } else
    {
        dungeon->manufacture_level = 4;
    }
    return true;
}

TbBool get_next_manufacture(struct Dungeon *dungeon)
{
    int chosen_class,chosen_kind,chosen_amount,chosen_level;
    struct ManfctrConfig *mconf;
    int tng_kind;
    long amount;
    //return _DK_get_next_manufacture(dungeon);
    set_manufacture_level(dungeon);
    chosen_class = TCls_Empty;
    chosen_kind = 0;
    chosen_amount = LONG_MAX;
    chosen_level = LONG_MAX;
    // Try getting door kind for manufacture
    for (tng_kind = 1; tng_kind < DOOR_TYPES_COUNT; tng_kind++)
    {
        mconf = &game.doors_config[tng_kind];
        if ( (dungeon->door_buildable[tng_kind]) && (dungeon->manufacture_level >= mconf->manufct_level) )
        {
            amount = dungeon->door_amount[tng_kind];
            if (amount < MANUFACTURED_ITEMS_LIMIT)
            {
                if ( (chosen_amount > amount) ||
                    ((chosen_amount == amount) && (chosen_level > mconf->manufct_level)) )
                {
                    chosen_class = TCls_Door;
                    chosen_amount = dungeon->door_amount[tng_kind];
                    chosen_kind = tng_kind;
                    chosen_level = mconf->manufct_level;
                }
            }
        }
    }
    // Try getting trap kind for manufacture
    for (tng_kind = 1; tng_kind < TRAP_TYPES_COUNT; tng_kind++)
    {
        mconf = &game.traps_config[tng_kind];
        if ( (dungeon->trap_buildable[tng_kind]) && (dungeon->manufacture_level >= mconf->manufct_level) )
        {
            amount = dungeon->trap_amount[tng_kind];
            if (amount < MANUFACTURED_ITEMS_LIMIT)
            {
                if ( (chosen_amount > amount) ||
                    ((chosen_amount == amount) && (chosen_level > mconf->manufct_level)) )
                {
                    chosen_class = TCls_Trap;
                    chosen_amount = dungeon->trap_amount[tng_kind];
                    chosen_kind = tng_kind;
                    chosen_level = mconf->manufct_level;
                }
            }
        }
    }
    if (chosen_class != TCls_Empty)
    {
        dungeon->manufacture_class = chosen_class;
        dungeon->manufacture_kind = chosen_kind;
        return true;
    }
    return false;
}

void remove_thing_from_mapwho(struct Thing *thing)
{
  struct Map *map;
  struct Thing *mwtng;
  SYNCDBG(18,"Starting");
  //_DK_remove_thing_from_mapwho(thing);
  if ((thing->field_0 & TF_IsInMapWho) == 0)
    return;
  if (thing->field_4 > 0)
  {
    mwtng = thing_get(thing->field_4);
    mwtng->field_2 = thing->field_2;
  } else
  {
    map = get_map_block_at(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
    set_mapwho_thing_index(map, thing->field_2);
  }
  if (thing->field_2 > 0)
  {
    mwtng = thing_get(thing->field_2);
    mwtng->field_4 = thing->field_4;
  }
  thing->field_2 = 0;
  thing->field_4 = 0;
  thing->field_0 &= ~TF_IsInMapWho;
}

void place_thing_in_mapwho(struct Thing *thing)
{
  SYNCDBG(18,"Starting");
  _DK_place_thing_in_mapwho(thing);
}

long get_thing_height_at(struct Thing *thing, struct Coord3d *pos)
{
  SYNCDBG(18,"Starting");
  return _DK_get_thing_height_at(thing, pos);
}

long manufacture_points_required(long mfcr_type, unsigned long mfcr_kind, const char *func_name)
{
  switch (mfcr_type)
  {
  case TCls_Trap:
      return game.traps_config[mfcr_kind%TRAP_TYPES_COUNT].manufct_required;
  case TCls_Door:
      return game.doors_config[mfcr_kind%DOOR_TYPES_COUNT].manufct_required;
  default:
      ERRORMSG("%s: Invalid type of manufacture",func_name);
      return 0;
  }
}

short process_player_manufacturing(long plyr_idx)
{
  struct Dungeon *dungeon;
  struct PlayerInfo *player;
  struct Room *room;
  int k;
  SYNCDBG(17,"Starting");
//  return _DK_process_player_manufacturing(plr_idx);

  dungeon = get_players_num_dungeon(plyr_idx);
  room = player_has_room_of_type(plyr_idx, RoK_WORKSHOP);
  if (room_is_invalid(room))
      return true;
  if (dungeon->manufacture_class == TCls_Empty)
  {
      get_next_manufacture(dungeon);
      return true;
  }
  k = manufacture_points_required(dungeon->manufacture_class, dungeon->manufacture_kind, __func__);
  // If we don't have enough manufacture points, don't do anything
  if (dungeon->manufacture_progress < (k << 8))
    return true;
  // Try to do the manufacturing
  room = find_room_with_spare_room_item_capacity(plyr_idx, RoK_WORKSHOP);
  if (room_is_invalid(room))
  {
      dungeon->manufacture_class = TCls_Empty;
      return false;
  }
  if (check_workshop_item_limit_reached(plyr_idx, dungeon->manufacture_class, dungeon->manufacture_kind))
  {
      ERRORLOG("Bad choice for manufacturing - limit reached for %s kind %d",thing_class_code_name(dungeon->manufacture_class),(int)dungeon->manufacture_kind);
      get_next_manufacture(dungeon);
      return false;
  }
  if (create_workshop_object_in_workshop_room(plyr_idx, dungeon->manufacture_class, dungeon->manufacture_kind) == 0)
  {
      ERRORLOG("Could not create manufactured %s kind %d",thing_class_code_name(dungeon->manufacture_class),(int)dungeon->manufacture_kind);
      return false;
  }
  add_workshop_item(plyr_idx, dungeon->manufacture_class, dungeon->manufacture_kind);

  switch (dungeon->manufacture_class)
  {
  case TCls_Trap:
      dungeon->lvstats.manufactured_traps++;
      // If that's local player - make a message
      player=get_my_player();
      if (player->id_number == plyr_idx)
        output_message(SMsg_ManufacturedTrap, 0, true);
      break;
  case TCls_Door:
      dungeon->lvstats.manufactured_doors++;
      // If that's local player - make a message
      player=get_my_player();
      if (player->id_number == plyr_idx)
        output_message(SMsg_ManufacturedDoor, 0, true);
      break;
  default:
      ERRORLOG("Invalid type of new manufacture");
      return false;
  }

  dungeon->manufacture_progress -= (k << 8);
  dungeon->field_118B = game.play_gameturn;
  dungeon->lvstats.manufactured_items++;
  get_next_manufacture(dungeon);
  return true;
}

void maintain_my_event_list(struct Dungeon *dungeon)
{
  _DK_maintain_my_event_list(dungeon); return;
}

void kill_oldest_my_event(struct Dungeon *dungeon)
{
  struct Event *event;
  long old_idx;
  long old_birth;
  long i,k;
  old_idx = -1;
  old_birth = 2147483647;
  for (i=12; i > 0; i--)
  {
    k = dungeon->field_13A7[i];
    event = &game.event[k];
    if ((event->birth_turn >= 0) && (event->birth_turn < old_birth))
    {
      old_idx = k;
      old_birth = event->birth_turn;
    }
  }
  if (old_idx >= 0)
    event_delete_event(dungeon->owner, old_idx);
  maintain_my_event_list(dungeon);
}

void maintain_all_players_event_lists(void)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  long i;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if (player_exists(player))
    {
      dungeon = get_players_dungeon(player);
      maintain_my_event_list(dungeon);
    }
  }
}

struct Thing *event_is_attached_to_thing(long ev_idx)
{
    struct Event *event;
    long i;
    event = &game.event[ev_idx];
    switch (event->kind)
    {
    case 3:
    case 6:
    case 10:
    case 14:
    case 16:
    case 17:
    case 24:
    case 25:
    case 26:
        i = event->target;
        break;
    default:
        i = 0;
        break;
    }
    return thing_get(i);
}

void event_process_events(void)
{
  _DK_event_process_events();
}

void update_all_events(void)
{
  struct Thing *thing;
  struct Event *event;
  long i;
//  _DK_update_all_events();
  for (i=EVENT_BUTTONS_COUNT; i > 0; i--)
  {
    thing = event_is_attached_to_thing(i);
    if (!thing_is_invalid(thing))
    {
      event = &game.event[i];
      if ((thing->class_id == TCls_Creature) && ((thing->field_0 & 0x10) || (thing->field_1 & 0x02)))
      {
        event->mappos_x = 0;
        event->mappos_y = 0;
      } else
      {
        event->mappos_x = thing->mappos.x.val;
        event->mappos_y = thing->mappos.y.val;
      }
    }
  }
  maintain_all_players_event_lists();
}

void process_level_script(void)
{
  SYNCDBG(6,"Starting");
  struct PlayerInfo *player;
  player = get_my_player();
  if (((game.system_flags & GSF_NetworkActive) == 0)
      && (player->victory_state != VicS_Undecided))
    return;
  process_conditions();
  process_check_new_creature_partys();
//script_process_messages(); is not here, but it is in beta - check why
  process_check_new_tunneller_partys();
  process_values();
  process_win_and_lose_conditions(my_player_number); //player->id_number may be uninitialized yet
//  show_onscreen_msg(8, "Flags %d %d %d %d %d %d", game.dungeon[0].script_flags[0],game.dungeon[0].script_flags[1],
//    game.dungeon[0].script_flags[2],game.dungeon[0].script_flags[3],game.dungeon[0].script_flags[4],game.dungeon[0].script_flags[5]);
  SYNCDBG(19,"Finished");
}

void message_update(void)
{
  SYNCDBG(6,"Starting");
  _DK_message_update();
}

long wp_check_map_pos_valid(struct Wander *wandr, long a1)
{
    return _DK_wp_check_map_pos_valid(wandr, a1);
}

long wander_point_update(struct Wander *wandr)
{
    long array[20];
    double realidx,delta;
    long tile1,tile2;
    long slb_num,valid_num,idx;
    long i;
    //return _DK_wander_point_update(wandr);
    valid_num = 0;
    slb_num = wandr->field_8;
    tile1 = 85;
    tile2 = slb_num_decode_x(slb_num);
    idx = 0;
    for (i = 0; i < wandr->field_C; i++)
    {
        if (wp_check_map_pos_valid(wandr, tile1))
        {
          if (valid_num >= 20)
              break;
          valid_num++;
          array[idx] = tile1;
          idx++;
          if (((wandr->field_14 & 0xFF) != 0) && (wandr->field_10 == valid_num))
          {
              slb_num = (wandr->field_C + wandr->field_8) % (map_tiles_x*map_tiles_y);
              break;
          }
        }
        slb_num++;
        tile2++;
        if (tile2 < map_tiles_x)
        {
          tile1 += 3;
        } else
        {
          tile2 = 0;
          if (slb_num >= map_tiles_x*map_tiles_y)
            slb_num = 0;
          tile1 = get_subtile_number(3*slb_num_decode_x(slb_num)+1, 3*slb_num_decode_y(slb_num)+1);
        }
    }
    wandr->field_8 = slb_num;
    if (valid_num <= 0)
        return 1;
    if (valid_num > wandr->field_10)
    {
      if (wandr->field_10 <= 0)
          return 1;
      wandr->field_4 %= 200;
      delta = (double)valid_num / (double)wandr->field_10;
      realidx = 0.0;
      for (i = 0; i < wandr->field_10; i++)
      {
          tile1 = array[(unsigned int)realidx];
          tile2 = ((tile1 >> 24) & 0xFF);
          wandr->field_18[2 * wandr->field_4] = (tile2 ^ ((tile2 ^ tile1) - tile2)) - tile2;
          wandr->field_18[2 * wandr->field_4 + 1] = (tile1 >> 8) & 0xFF;
          wandr->field_4 = (wandr->field_4 + 1) % 200;
          if (wandr->field_0 < 200)
            wandr->field_0++;
          realidx += delta;
      }
    } else
    {
        idx = 0;
        while (valid_num > 0)
        {
            tile1 = array[idx];
            wandr->field_18[2 * wandr->field_4] = abs(tile1);
            wandr->field_18[2 * wandr->field_4 + 1] = (tile1 >> 8) & 0xFF;
            wandr->field_4 = (wandr->field_4 + 1) % 200;
            if (wandr->field_0 < 200)
              wandr->field_0++;
            idx++;
            valid_num--;
        }
    }
    return 1;
}

void update_player_camera(struct PlayerInfo *player)
{
  _DK_update_player_camera(player);
}

void update_research(void)
{
  int i;
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player = get_player(i);
      if (player_exists(player) && (player->field_2C == 1))
      {
          process_player_research(i);
      }
  }
}

void update_manufacturing(void)
{
  int i;
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player = get_player(i);
      if (player_exists(player) && (player->field_2C == 1))
      {
          process_player_manufacturing(i);
      }
  }
}

void update_all_players_cameras(void)
{
  int i;
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if (player_exists(player) && ((player->field_0 & 0x40) == 0))
    {
          update_player_camera(player);
    }
  }
}

void update_flames_nearest_camera(struct Camera *camera)
{
  if (camera == NULL)
    return;
  _DK_update_flames_nearest_camera(camera);
}

void update_footsteps_nearest_camera(struct Camera *camera)
{
  SYNCDBG(6,"Starting");
  if (camera == NULL)
    return;
  _DK_update_footsteps_nearest_camera(camera);
}

void process_player_states(void)
{
  SYNCDBG(6,"Starting");
  _DK_process_player_states();
}

void set_level_objective(char *msg_text)
{
    if (msg_text == NULL)
    {
        ERRORLOG("Invalid message pointer");
        return;
    }
    strncpy(game.evntbox_text_objective, msg_text, MESSAGE_TEXT_LEN);
    new_objective = 1;
}

void update_player_objectives(int plridx)
{
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting for player %d",plridx);
  player = get_player(plridx);
  if ((game.system_flags & GSF_NetworkActive) != 0)
  {
    if ((!player->field_4EB) && (player->victory_state != VicS_Undecided))
      player->field_4EB = game.play_gameturn+1;
  }
  if (player->field_4EB == game.play_gameturn)
  {
    switch (player->victory_state)
    {
    case VicS_WonLevel:
        if (plridx == my_player_number)
          set_level_objective(gui_strings[0]); // Success message
        display_objectives(player->id_number, 0, 0);
        break;
    case VicS_LostLevel:
        if (plridx == my_player_number)
          set_level_objective(gui_strings[335]); // Defeated message
        display_objectives(player->id_number, 0, 0);
        break;
    }
  }
}

void process_players(void)
{
  int i;
  struct PlayerInfo *player;
  SYNCDBG(5,"Starting");
  process_player_instances();
  process_player_states();
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player = get_player(i);
      if (player_exists(player) && (player->field_2C == 1))
      {
          SYNCDBG(6,"Doing updates for player %d",i);
          wander_point_update(&player->wandr1);
          wander_point_update(&player->wandr2);
          update_power_sight_explored(player);
          update_player_objectives(i);
      }
  }
  SYNCDBG(17,"Finished");
}

short update_animating_texture_maps(void)
{
  int i;
  SYNCDBG(18,"Starting");
  anim_counter = (anim_counter+1) % 8;
  short result=true;
  for (i=0; i<TEXTURE_BLOCKS_ANIM_COUNT; i++)
  {
        short j = game.texture_animation[8*i+anim_counter];
        if ((j>=0) && (j<TEXTURE_BLOCKS_STAT_COUNT))
        {
          block_ptrs[TEXTURE_BLOCKS_STAT_COUNT+i] = block_ptrs[j];
        } else
        {
          result=false;
        }
  }
  return result;
}

void add_creature_to_pool(long kind, long amount, unsigned long a3)
{
  long prev_amount;
  prev_amount = game.pool.crtr_kind[kind%CREATURE_TYPES_COUNT];
  if ((a3 == 0) || (prev_amount != -1))
  {
    if ((amount != -1) && (amount != 0) && (prev_amount != -1))
      game.pool.crtr_kind[kind%CREATURE_TYPES_COUNT] = prev_amount+amount;
    else
      game.pool.crtr_kind[kind%CREATURE_TYPES_COUNT] = amount;
  }
}

short update_creature_pool_state(void)
{
  int i;
  game.pool.is_empty = true;
  for (i=1; i < CREATURE_TYPES_COUNT; i++)
  {
      if (game.pool.crtr_kind[i] > 0)
      { game.pool.is_empty = false; break; }
  }
  return true;
}

int clear_active_dungeons_stats(void)
{
  struct Dungeon *dungeon;
  int i;
  for (i=0; i <= game.hero_player_num; i++)
  {
      dungeon = get_dungeon(i);
      if (dungeon_invalid(dungeon))
          break;
      memset((char *)dungeon->field_64, 0, 480 * sizeof(short));
      memset((char *)dungeon->job_breeds_count, 0, CREATURE_TYPES_COUNT*3*sizeof(unsigned short));
      memset((char *)dungeon->field_4E4, 0, CREATURE_TYPES_COUNT*3*sizeof(unsigned short));
  }
  return i;
}

long update_cave_in(struct Thing *thing)
{
  return _DK_update_cave_in(thing);
}

void update(void)
{
    struct PlayerInfo *player;
    SYNCDBG(4,"Starting for turn %ld",(long)game.play_gameturn);

    if ((game.numfield_C & 0x01) == 0)
        update_light_render_area();
    process_packets();
    if (quit_game || exit_keeper) {
        return;
    }
    if (game.game_kind == GKind_Unknown1)
    {
        game.field_14EA4B = 0;
        return;
    }

    if ((game.numfield_C & 0x01) == 0)
    {
        player = get_my_player();
        if (player->field_3 & 0x08)
        {
            PaletteSetPlayerPalette(player, _DK_palette);
            set_flag_byte(&player->field_3,0x08,false);
        }
        clear_active_dungeons_stats();
        update_creature_pool_state();
        if ((game.play_gameturn & 0x01) != 0)
            update_animating_texture_maps();
        update_things();
        process_rooms();
        process_dungeons();
        update_research();
        update_manufacturing();
        event_process_events();
        update_all_events();
        process_level_script();
        if ((game.numfield_D & 0x04) != 0)
            process_computer_players2();
        process_players();
        process_action_points();
        player = get_my_player();
        if (player->view_mode == PVM_CreatureView)
            update_flames_nearest_camera(player->acamera);
        update_footsteps_nearest_camera(player->acamera);
        PaletteFadePlayer(player);
        process_armageddon();
#if (BFDEBUG_LEVEL > 9)
        lights_stats_debug_dump();
        things_stats_debug_dump();
        creature_stats_debug_dump();
#endif
    }

    message_update();
    update_all_players_cameras();
    update_player_sounds();
    game.field_14EA4B = 0;
    SYNCDBG(6,"Finished");
}

void draw_spell_cursor(unsigned char wrkstate, unsigned short tng_idx, unsigned char stl_x, unsigned char stl_y)
{
    struct PlayerInfo *player;
    struct Thing *thing;
    struct SpellData *pwrdata;
    struct MagicStats *magstat;
    Expand_Check_Func chkfunc;
    TbBool allow_cast;
    long spl_id;
    long i;
    //_DK_draw_spell_cursor(wrkstate, tng_idx, stl_x, stl_y); return;
    spl_id = -1;
    if (wrkstate < PLAYER_STATES_COUNT)
      spl_id = player_state_to_spell[wrkstate];
    SYNCDBG(5,"Starting for spell %ld",spl_id);
    if (spl_id <= 0)
    {
      set_pointer_graphic(0);
      return;
    }
    player = get_my_player();
    thing = thing_get(tng_idx);
    allow_cast = false;
    pwrdata = get_power_data(spl_id);
    if ((tng_idx == 0) || (thing->owner == player->id_number) || (pwrdata->flag_1A != 0))
    {
      if (can_cast_spell_at_xy(player->id_number, spl_id, stl_x, stl_y, 0))
      {
        if ((tng_idx == 0) || can_cast_spell_on_creature(player->id_number, thing, spl_id))
        {
          allow_cast = true;
        }
      }
    }
    if (!allow_cast)
    {
      set_pointer_graphic(15);
      return;
    }
    chkfunc = pwrdata->field_15;
    if (chkfunc != NULL)
    {
      if (chkfunc())
      {
        i = player->field_4D2/4;
        if (i > 8)
          i = 8;
        set_pointer_graphic(16+i);
        magstat = &game.magic_stats[spl_id];
        draw_spell_cost = magstat->cost[i];
        return;
      }
    }
    i = pwrdata->field_13;
    set_pointer_graphic_spell(i, game.play_gameturn);
}

void draw_bonus_timer(void)
{
  _DK_draw_bonus_timer(); return;
}

void draw_swipe(void)
{
  _DK_draw_swipe();
}

void do_map_rotate_stuff(long a1, long a2, long *a3, long *a4, long a5)
{
  _DK_do_map_rotate_stuff(a1, a2, a3, a4, a5);
}

short do_left_map_drag(long begin_x, long begin_y, long curr_x, long curr_y, long zoom)
{
  SYNCDBG(17,"Starting");
  struct PlayerInfo *player;
  long x,y;
  if (!clicked_on_small_map)
  {
    grabbed_small_map = 0;
    return 0;
  }
  x = (curr_x - (MyScreenWidth >> 1)) / 2;
  y = (curr_y - (MyScreenHeight >> 1)) / 2;
  if ((abs(curr_x - old_mx) < 2) && (abs(curr_y - old_my) < 2))
    return 0;
  if (!grabbed_small_map)
  {
    grabbed_small_map = 1;
    x = 0;
    y = 0;
  }
  do_map_rotate_stuff(x, y, &curr_x, &curr_y, zoom);
  player = get_my_player();
  game.hand_over_subtile_x = curr_x;
  game.hand_over_subtile_y = curr_y;
  if (subtile_has_slab(curr_x, curr_y))
  {
    set_players_packet_action(player, PckA_BookmarkLoad, curr_x, curr_y, 0, 0);
  }
  return 1;
}

short do_left_map_click(long begin_x, long begin_y, long curr_x, long curr_y, long zoom)
{
  SYNCDBG(17,"Starting");
  struct PlayerInfo *player;
  short result;
  result = 0;
  player = get_my_player();
  if ((left_button_released) && (clicked_on_small_map))
  {
      if (grabbed_small_map)
      {
        game.small_map_state = 2;
        LbMouseSetPosition((begin_x+58)/pixel_size, (begin_y+58)/pixel_size);
      } else
      {
        do_map_rotate_stuff(curr_x-begin_x-58, curr_y-begin_y-58, &curr_x, &curr_y, zoom);
        game.hand_over_subtile_x = curr_x;
        game.hand_over_subtile_y = curr_y;
        if (subtile_has_slab(curr_x, curr_y))
        {
          result = 1;
          set_players_packet_action(player, PckA_BookmarkLoad, curr_x, curr_y, 0, 0);
        }
      }
    grabbed_small_map = 0;
    clicked_on_small_map = 0;
    left_button_released = 0;
  }
  return result;
}

struct Thing *get_workshop_box_thing(long owner, long model)
{
    struct Thing *thing;
    int i,k;
    k = 0;
    i = game.thing_lists[2].index;
    while (i > 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if ( ((thing->field_0 & 0x01) != 0) && (thing->model == model) && (thing->owner == owner) )
        {
            if ( ((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0) )
                return thing;
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return INVALID_THING;
}

long remove_workshop_object_from_player(long owner, long model)
{
    struct Thing *thing;
    struct Room *room;
    //return _DK_remove_workshop_object_from_player(a1, a2);
    thing = get_workshop_box_thing(owner, model);
    if (thing_is_invalid(thing))
        return 0;
    room = get_room_thing_is_on(thing);
    if ( room_exists(room) ) {
        remove_workshop_object_from_workshop(room,thing);
    } else {
        WARNLOG("Crate thing index %d isn't placed existing room; removing anyway",(int)thing->index);
    }
    create_effect(&thing->mappos, imp_spangle_effects[thing->owner], thing->owner);
    delete_thing_structure(thing, 0);
    return 1;
}

unsigned char tag_cursor_blocks_place_trap(unsigned char a1, long a2, long a3)
{
  SYNCDBG(7,"Starting");
  return _DK_tag_cursor_blocks_place_trap(a1, a2, a3);
}

void stop_creatures_around_hand(char a1, unsigned short a2, unsigned short a3)
{
  _DK_stop_creatures_around_hand(a1, a2, a3);
}

long near_map_block_thing_filter_queryable_object(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
/* Currently this only makes Dungeon Heart blinking; maybe I'll find a purpose for it later
    long dist_x,dist_y;
    if ((thing->class_id == TCls_Object) && (thing->model == 5))
    {
      if (thing->owner == param->plyr_idx)
      {
          // note that abs() is not required because we're computing square of the values
          dist_x = param->num1-(MapCoord)thing->mappos.x.val;
          dist_y = param->num2-(MapCoord)thing->mappos.y.val;
          // This function should return max value when the distance is minimal, so:
          return LONG_MAX-(dist_x*dist_x + dist_y*dist_y);
      }
    }
*/
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

struct Thing *get_queryable_object_near(MapCoord pos_x, MapCoord pos_y, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundFilterParam param;
    SYNCDBG(19,"Starting");
    //return _DK_get_queryable_object_near(pos_x, pos_y, plyr_idx);
    filter = near_map_block_thing_filter_queryable_object;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

void tag_cursor_blocks_dig(unsigned char a1, long a2, long a3, long a4)
{
  SYNCDBG(7,"Starting");
  _DK_tag_cursor_blocks_dig(a1, a2, a3, a4);
}

void tag_cursor_blocks_thing_in_hand(unsigned char a1, long a2, long a3, int a4, long a5)
{
  SYNCDBG(7,"Starting");
  _DK_tag_cursor_blocks_thing_in_hand(a1, a2, a3, a4, a5);
}

unsigned long can_drop_thing_here(long x, long y, long a3, unsigned long a4)
{
  return _DK_can_drop_thing_here(x, y, a3, a4);
}

/**
 * Returns if a given player (owner) can dig the specified subtile.
 */
short can_dig_here(long stl_x, long stl_y, long plyr_idx)
{
  struct SlabMap *slb;
  struct SlabAttr *slbattr;
  slb = get_slabmap_block(map_to_slab[stl_x],map_to_slab[stl_y]);
  if (slabmap_block_invalid(slb))
    return false;
  if (!subtile_revealed(stl_x, stl_y, plyr_idx))
    return true;
  if (slab_kind_is_nonmagic_door(slb->kind))
  {
      if (slabmap_owner(slb) == plyr_idx)
        return false;
  }
  slbattr = get_slab_attrs(slb);
  if ((slbattr->field_6 & 0x29) != 0)
    return true;
  return false;
}

long thing_in_wall_at(struct Thing *thing, struct Coord3d *pos)
{
    return _DK_thing_in_wall_at(thing, pos);
}

short can_place_thing_here(struct Thing *thing, long x, long y, long dngn_idx)
{
    struct Coord3d pos;
    TbBool is_digger;
    is_digger = thing_is_creature_special_digger(thing);
    if (!can_drop_thing_here(x, y, dngn_idx, is_digger))
      return false;
    pos.x.val = (x << 8) + 128;
    pos.y.val = (y << 8) + 128;
    pos.z.val = get_thing_height_at(thing, &pos);
    return !thing_in_wall_at(thing, &pos);
}

short do_right_map_click(long start_x, long start_y, long curr_mx, long curr_my, long zoom)
{
    long x,y;
    SYNCDBG(17,"Starting");
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Thing *thing;
    do_map_rotate_stuff(curr_mx-start_x-58, curr_my-start_y-58, &x, &y, zoom);
    game.hand_over_subtile_x = x;
    game.hand_over_subtile_y = y;
    player = get_my_player();
    dungeon = get_dungeon(player->id_number);
    thing = get_first_thing_in_power_hand(player);
    if (!thing_is_invalid(thing))
    {
        if (can_place_thing_here(thing, x, y, player->id_number))
          game.small_map_state = 1;
    }
    if (right_button_clicked)
      right_button_clicked = 0;
    if (right_button_released)
    {
        right_button_released = 0;
        if (subtile_has_slab(x, y))
        {
          set_players_packet_action(player, PckA_DumpHeldThings, x, y, 0, 0);
          return 1;
        }
    }
    return 0;
}

void set_player_cameras_position(struct PlayerInfo *player, long pos_x, long pos_y)
{
    player->cameras[2].mappos.x.val = pos_x;
    player->cameras[3].mappos.x.val = pos_x;
    player->cameras[0].mappos.x.val = pos_x;
    player->cameras[2].mappos.y.val = pos_y;
    player->cameras[3].mappos.y.val = pos_y;
    player->cameras[0].mappos.y.val = pos_y;
}

void draw_texture(long a1, long a2, long a3, long a4, long a5, long a6, long a7)
{
  _DK_draw_texture(a1, a2, a3, a4, a5, a6, a7);
}

void update_block_pointed(int i,long x, long x_frac, long y, long y_frac)
{
    struct Map *map;
    struct Column *colmn;
    short visible;
    unsigned int mask;
    long k;

    if (i > 0)
    {
      map = get_map_block_at(x,y);
      visible = map_block_revealed_bit(map, player_bit);
      if ((!visible) || ((map->data & 0x7FF) > 0))
      {
        if (visible)
          k = map->data & 0x7FF;
        else
          k = game.unrevealed_column_idx;
        colmn = get_column(k);
        mask = colmn->solidmask;
        if ((temp_cluedo_mode) && (mask != 0))
        {
          if (visible)
            k = map->data & 0x7FF;
          else
            k = game.unrevealed_column_idx;
          colmn = get_column(k);
          if (colmn->solidmask >= 8)
          {
            if ( (!visible) || (((get_navigation_map(x,y) & 0x80) == 0) && ((map->flags & 0x02) == 0)) )
              mask &= 3;
          }
        }
        if (mask & (1 << (i-1)))
        {
          pointed_at_frac_x = x_frac;
          pointed_at_frac_y = y_frac;
          block_pointed_at_x = x;
          block_pointed_at_y = y;
          me_pointed_at = map;
        }
        if (((!temp_cluedo_mode) && (i == 5)) || ((temp_cluedo_mode) && (i == 2)))
        {
          top_pointed_at_frac_x = x_frac;
          top_pointed_at_frac_y = y_frac;
          top_pointed_at_x = x;
          top_pointed_at_y = y;
        }
      }
    } else
    {
        map = get_map_block_at(x,y);
        floor_pointed_at_x = x;
        floor_pointed_at_y = y;
        block_pointed_at_x = x;
        block_pointed_at_y = y;
        pointed_at_frac_x = x_frac;
        pointed_at_frac_y = y_frac;
        me_pointed_at = map;
    }
}

void update_blocks_pointed(void)
{
    long x,y;
    long x_frac,y_frac;
    long hori_ptr_y,vert_ptr_y;
    long hori_hdelta_y,vert_hdelta_y;
    long hori_ptr_x,vert_ptr_x;
    long hvdiv_x,hvdiv_y;
    long long lltmp;
    long k;
    int i;
    SYNCDBG(19,"Starting");
    if ((!vert_offset[1]) && (!hori_offset[1]))
    {
        block_pointed_at_x = 0;
        block_pointed_at_y = 0;
        me_pointed_at = INVALID_MAP_BLOCK;//get_map_block_at(0,0);
    } else
    {
        hori_ptr_y = (long)hori_offset[0] * (pointer_y - y_init_off);
        vert_ptr_y = (long)vert_offset[0] * (pointer_y - y_init_off);
        hori_hdelta_y = (long)hori_offset[0] * ((long)high_offset[1] >> 8);
        vert_hdelta_y = (long)vert_offset[0] * ((long)high_offset[1] >> 8);
        vert_ptr_x = (long)(vert_offset[1] * (pointer_x - x_init_off)) >> 1;
        hori_ptr_x = (long)(hori_offset[1] * (pointer_x - x_init_off)) >> 1;
        lltmp = hori_offset[0] * (long long)vert_offset[1] - vert_offset[0] * (long long)hori_offset[1];
        hvdiv_x = (lltmp >> 11);
        if (hvdiv_x == 0) hvdiv_x = 1;
        lltmp = vert_offset[0] * (long long)hori_offset[1] - hori_offset[0] * (long long)vert_offset[1];
        hvdiv_y = (lltmp >> 11);
        if (hvdiv_y == 0) hvdiv_y = 1;
        for (i=0; i < 8; i++)
        {
          k = (vert_ptr_x - (vert_ptr_y >> 1)) / hvdiv_x;
          x_frac = (k & 3) << 6;
          x = k >> 2;
          k = (hori_ptr_x - (hori_ptr_y >> 1)) / hvdiv_y;
          y_frac = (k & 3) << 6;
          y = k >> 2;
          if ((x >= 0) && (x < map_subtiles_x) && (y >= 0) && (y < map_subtiles_y))
          {
              update_block_pointed(i,x,x_frac,y,y_frac);
          }
          hori_ptr_y -= hori_hdelta_y;
          vert_ptr_y -= vert_hdelta_y;
        }
    }
    SYNCDBG(19,"Finished");
}

void engine(struct PlayerInfo *player, struct Camera *cam)
{
    TbGraphicsWindow grwnd;
    TbGraphicsWindow ewnd;
    unsigned short flg_mem;

    SYNCDBG(9,"Starting");
    //_DK_engine(cam); return;

    flg_mem = lbDisplay.DrawFlags;
    update_engine_settings(player);
    mx = cam->mappos.x.val;
    my = cam->mappos.y.val;
    mz = cam->mappos.z.val;
    pointer_x = (GetMouseX() - player->engine_window_x) / pixel_size;
    pointer_y = (GetMouseY() - player->engine_window_y) / pixel_size;
    lens = (cam->field_13 * ((long)MyScreenWidth))/pixel_size / 320;
    if (lens_mode == 0)
        update_blocks_pointed();
    LbScreenStoreGraphicsWindow(&grwnd);
    store_engine_window(&ewnd,pixel_size);
    view_height_over_2 = ewnd.height/2;
    view_width_over_2 = ewnd.width/2;
    LbScreenSetGraphicsWindow(ewnd.x, ewnd.y, ewnd.width, ewnd.height);
    setup_vecs(lbDisplay.GraphicsWindowPtr, 0, lbDisplay.GraphicsScreenWidth,
        ewnd.width, ewnd.height);
    camera_zoom = scale_camera_zoom_to_screen(cam->zoom);
    draw_view(cam, 0);
    lbDisplay.DrawFlags = flg_mem;
    thing_being_displayed = 0;
    LbScreenLoadGraphicsWindow(&grwnd);
}

void remove_explored_flags_for_power_sight(struct PlayerInfo *player)
{
    SYNCDBG(9,"Starting");
    _DK_remove_explored_flags_for_power_sight(player);
}

void DrawBigSprite(long x, long y, struct BigSprite *bigspr, struct TbSprite *sprite)
{
  _DK_DrawBigSprite(x, y, bigspr, sprite);
}

void pannel_map_draw(long x, long y, long zoom)
{
  _DK_pannel_map_draw(x, y, zoom);
}

void draw_overlay_things(long zoom)
{
  SYNCDBG(7,"Starting");
  _DK_draw_overlay_things(zoom);
}

void find_frame_rate(void)
{
    static TbClockMSec prev_time2=0;
    static TbClockMSec cntr_time2=0;
    unsigned long curr_time;
    curr_time = LbTimerClock();
    cntr_time2++;
    if (curr_time-prev_time2 >= 1000)
    {
        double time_fdelta = 1000.0*((double)(cntr_time2))/(curr_time-prev_time2);
        prev_time2 = curr_time;
        game.time_delta = (unsigned long)(time_fdelta*256.0);
        cntr_time2 = 0;
    }
}

void packet_load_find_frame_rate(unsigned long incr)
{
    static TbClockMSec start_time=0;
    static TbClockMSec extra_frames=0;
    TbClockMSec curr_time;
    curr_time = LbTimerClock();
    if ((curr_time-start_time) < 5000)
    {
        extra_frames += incr;
    } else
    {
        double time_fdelta = 1000.0*((double)(extra_frames+incr))/(curr_time-start_time);
        start_time = curr_time;
        game.time_delta = (unsigned long)(time_fdelta*256.0);
        extra_frames = 0;
    }
}

/**
 * Checks if the game screen needs redrawing.
 */
short display_should_be_updated_this_turn(void)
{
    if ((game.numfield_C & 0x01) != 0)
      return true;
    if ( (game.turns_fastforward == 0) && (!game.numfield_149F38) )
    {
      find_frame_rate();
      if ( (game.frame_skip == 0) || ((game.play_gameturn % game.frame_skip) == 0))
        return true;
    } else
    if ( ((game.play_gameturn & 0x3F)==0) ||
         ((game.numfield_149F38) && ((game.play_gameturn & 7)==0)) )
    {
      packet_load_find_frame_rate(64);
      return true;
    }
    return false;
}

/**
 * Makes last updates to the video buffer, and swaps buffers to show
 * the new image.
 */
TbBool keeper_screen_swap(void)
{
/*  // For resolution 640x480, move the graphics data 40 lines lower
  if ( lbDisplay.ScreenMode == Lb_SCREEN_MODE_640_480_8 )
    if (LbScreenLock() == Lb_SUCCESS)
    {
      int i;
      int scrmove_x=0;
      int scrmove_y=40;
      int scanline_len=640;
      for (i=400;i>=0;i--)
        memcpy(lbDisplay.WScreen+scanline_len*(i+scrmove_y)+scrmove_x, lbDisplay.WScreen+scanline_len*i, scanline_len-scrmove_x);
      memset(lbDisplay.WScreen, 0, scanline_len*scrmove_y);
      LbScreenUnlock();
    }*/
  LbScreenSwap();
  return true;
}

/**
 * Waits until the next game turn. Delay is usually controlled by
 * num_fps variable.
 */
TbBool keeper_wait_for_next_turn(void)
{
    if ((game.numfield_D & 0x10) != 0)
    {
        // No idea when such situation occurs
        TbClockMSec sleep_end = last_loop_time + 1000;
        LbSleepUntil(sleep_end);
        last_loop_time = LbTimerClock();
        return true;
    }
    if (game.frame_skip == 0)
    {
        // Standard delaying system
        TbClockMSec sleep_end = last_loop_time + 1000/game.num_fps;
        LbSleepUntil(sleep_end);
        last_loop_time = LbTimerClock();
        return true;
    }
    return false;
}

TbBool keeper_wait_for_screen_focus(void)
{
    do {
        if ( !LbWindowsControl() )
        {
          if ((game.system_flags & GSF_NetworkActive) == 0)
          {
            exit_keeper = 1;
            break;
          }
          SYNCLOG("Alex's point reached");
        }
        if (LbIsActive())
          return true;
        if ((game.system_flags & GSF_NetworkActive) != 0)
          return true;
        LbSleepFor(50);
    } while ((!exit_keeper) && (!quit_game));
    return false;
}

void keeper_gameplay_loop(void)
{
    short do_draw;
    struct PlayerInfo *player;
    SYNCDBG(5,"Starting");
    player = get_my_player();
    PaletteSetPlayerPalette(player, _DK_palette);
    if ((game.numfield_C & 0x02) != 0)
        initialise_eye_lenses();
    SYNCDBG(0,"Entering the gameplay loop for level %d",(int)get_loaded_level_number());

    KeeperSpeechClearEvents();
    LbErrorParachuteUpdate(); // For some reasone parachute keeps changing; Remove when won't be needed anymore

    //the main gameplay loop starts
    while ((!quit_game) && (!exit_keeper))
    {
        if ((game.flags_font & FFlg_unk10) != 0)
        {
          if (game.play_gameturn == 4)
              LbNetwork_ChangeExchangeTimeout(0);
        }

        // Check if we should redraw screen in this turn
        do_draw = display_should_be_updated_this_turn() || (!LbIsActive());

        LbWindowsControl();
        update_mouse();
        input_eastegg();
        input();
        update();

        if (quit_game || exit_keeper)
            do_draw = false;

        if ( do_draw )
            keeper_screen_redraw();
        keeper_wait_for_screen_focus();
        // Direct information/error messages
        if (LbScreenLock() == Lb_SUCCESS)
        {
            if ( do_draw )
                perform_any_screen_capturing();
            draw_onscreen_direct_messages();
            LbScreenUnlock();
        }

        // Music and sound control
        if ( !SoundDisabled )
        {
            if ( (game.turns_fastforward == 0) && (!game.numfield_149F38) )
            {
                MonitorStreamedSoundTrack();
                process_sound_heap();
            }
        }

        // Move the graphics window to center of screen buffer and swap screen
        if ( do_draw )
            keeper_screen_swap();

        // Make delay if the machine is too fast
        if ( (!game.packet_load_enable) || (game.turns_fastforward == 0) )
            keeper_wait_for_next_turn();
        if (game.turns_packetoff == game.play_gameturn)
            exit_keeper = 1;
    } // end while
    SYNCDBG(0,"Gameplay loop finished after %lu turns",(unsigned long)game.play_gameturn);
}

void intro(void)
{
    char *fname;
    fname = prepare_file_path(FGrp_LoData, "intromix.smk");
    SYNCDBG(0,"Playing intro movie \"%s\"",fname);
    play_smacker_file(fname, 1);
}

void outro(void)
{
    char *fname;
    fname = prepare_file_path(FGrp_LoData, "outromix.smk");
    SYNCDBG(0,"Playing outro movie \"%s\"",fname);
    play_smacker_file(fname, 17);
}

void set_thing_draw(struct Thing *thing, long anim, long speed, long a4, char a5, char start_frame, unsigned char a7)
{
  unsigned long i;
  //_DK_set_thing_draw(thing, anim, speed, a4, a5, start_frame, a7); return;
  thing->field_44 = convert_td_iso(anim);
  thing->field_50 &= 0x03;
  thing->field_50 |= (a7 << 2);
  thing->field_49 = keepersprite_frames(thing->field_44);
  if (speed != -1)
  {
    thing->field_3E = speed;
  }
  if (a4 != -1)
  {
    thing->field_46 = a4;
  }
  if (a5 != -1)
  {
    set_flag_byte(&thing->field_4F, 0x40, a5);
  }
  if (start_frame == -2)
  {
    i = keepersprite_frames(thing->field_44) - 1;
    thing->field_48 = i;
    thing->field_40 = i << 8;
  } else
  if (start_frame == -1)
  {
    i = ACTION_RANDOM(thing->field_49);
    thing->field_48 = i;
    thing->field_40 = i << 8;
  } else
  {
    i = start_frame;
    thing->field_48 = i;
    thing->field_40 = i << 8;
  }
}

struct Thing *find_base_thing_on_mapwho(unsigned char oclass, unsigned short model, unsigned short x, unsigned short y)
{
  return _DK_find_base_thing_on_mapwho(oclass, model, x, y);
}

void init_dungeons(void)
{
  int i,k;
  struct Dungeon *dungeon;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    dungeon = get_dungeon(game.hero_player_num);
    dungeon->hates_player[i] = game.fight_max_hate;
    dungeon = get_dungeon(i);
    dungeon->hates_player[game.hero_player_num%DUNGEONS_COUNT] = game.fight_max_hate;
    dungeon->num_active_diggers = 0;
    dungeon->num_active_creatrs = 0;
    dungeon->creatr_list_start = 0;
    dungeon->digger_list_start = 0;
    dungeon->owner = i;
    dungeon->max_creatures_attracted = game.default_max_crtrs_gen_entrance;
    dungeon->dead_creatures_count = 0;
    dungeon->dead_creature_idx = 0;
    for (k=0; k < DUNGEONS_COUNT; k++)
    {
      if (k == i)
        dungeon->hates_player[k] = game.fight_max_love;
      else
        dungeon->hates_player[k] = game.fight_max_hate;
    }
    LbMemorySet(dungeon->field_1489, 0, 32);
  }
}

short thing_model_is_gold_hoarde(unsigned short model)
{
  return ((model >= 52) && (model <= 56));
}

long add_gold_to_hoarde(struct Thing *thing, struct Room *room, long amount)
{
  return _DK_add_gold_to_hoarde(thing, room, amount);
}

int can_thing_be_queried(struct Thing *thing, long a2)
{
  return _DK_can_thing_be_queried(thing, a2);
}

int can_thing_be_possessed(struct Thing *thing, long a2)
{
  return _DK_can_thing_be_possessed(thing, a2);
}

long tag_blocks_for_digging_in_rectangle_around(long a1, long a2, char a3)
{
  return _DK_tag_blocks_for_digging_in_rectangle_around(a1, a2, a3);
}

void untag_blocks_for_digging_in_rectangle_around(long a1, long a2, char a3)
{
  _DK_untag_blocks_for_digging_in_rectangle_around(a1, a2, a3);
}

void kill_all_room_slabs_and_contents(struct Room *room)
{
    struct SlabMap *slb;
    long slb_x, slb_y;
    unsigned long k;
    long i;
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        slb = get_slabmap_block(slb_x, slb_y);
        kill_room_slab_and_contents(room->owner, slb_x, slb_y);
        slb->next_in_room = 0;
        slb->room_index = 0;
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    room->slabs_list = 0;
    room->slabs_count = 0;
}

void remove_slab_from_room_tiles_list(struct Room *room, long rslb_num)
{
    struct SlabMap *slb,*rslb;
    unsigned long k;
    long i;
    rslb = get_slabmap_direct(rslb_num);
    if (slabmap_block_invalid(rslb))
    {
        ERRORLOG("Nonexisting slab %d.",(int)rslb_num);
        return;
    }
    // If the slab to remove is first in room slabs list - it's simple
    // In this case we need to re-put a flag on first slab
    if (room->slabs_list == rslb_num)
    {
        delete_room_flag(room);
        room->slabs_list = rslb->next_in_room;
        rslb->next_in_room = 0;
        rslb->room_index = 0;
        room->slabs_count--;
        create_room_flag(room);
        return;
    }
    // If the slab to remove is not first, we have to sweep the list
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        slb = get_slabmap_direct(i);
        if (slabmap_block_invalid(slb))
        {
          ERRORLOG("Jump to invalid item when sweeping Slabs.");
          break;
        }
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        if (slb->next_in_room == rslb_num)
        {
            // When the item was found, replace its reference with next item
            slb->next_in_room = rslb->next_in_room;
            rslb->next_in_room = 0;
            rslb->room_index = 0;
            room->slabs_count--;
            return;
        }
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    WARNLOG("Slab %ld couldn't be found in room tiles list.",rslb_num);
    rslb->next_in_room = 0;
    rslb->room_index = 0;
}

void sell_room_slab_when_no_free_room_structures(struct Room *room, long slb_x, long slb_y, unsigned char gnd_slab)
{
    struct RoomStats *rstat;
    struct Coord3d pos;
    long revenue;
    delete_room_slab_when_no_free_room_structures(slb_x, slb_y, gnd_slab);
    rstat = &game.room_stats[room->kind];
    revenue = compute_value_percentage(rstat->cost, ROOM_SELL_REVENUE_PERCENT);
    if (revenue != 0)
    {
        set_coords_to_slab_center(&pos,slb_x,slb_y);
        create_price_effect(&pos, room->owner, revenue);
        player_add_offmap_gold(room->owner, revenue);
    }
}

void recreate_rooms_from_room_slabs(struct Room *room, unsigned char gnd_slab)
{
    struct Room *nroom;
    struct SlabMap *slb;
    struct RoomData *rdata;
    long slb_x, slb_y;
    unsigned long k;
    long i;
    SYNCDBG(7,"Starting");
    // Clear room index in all slabs
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        slb = get_slabmap_direct(i);
        if (slabmap_block_invalid(slb))
        {
          ERRORLOG("Jump to invalid item when sweeping Slabs.");
          break;
        }
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        slb->room_index = 0;
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    // Create a new room for every slab
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        nroom = create_room(room->owner, room->kind, 3*slb_x+1, 3*slb_y+1);
        if (room_is_invalid(nroom)) // In case of error, sell the whole thing
        {
            sell_room_slab_when_no_free_room_structures(room, slb_x, slb_y, gnd_slab);
        } else
        {
            nroom->efficiency = calculate_room_efficiency(nroom);
            rdata = room_data_get_for_room(nroom);
            nroom->field_C = (long)game.hits_per_slab * (long)nroom->slabs_count;
            if (rdata->ofsfield_3 != NULL)
                rdata->ofsfield_3(nroom);
            if (rdata->ofsfield_7 != NULL)
                rdata->ofsfield_7(nroom);
            if (rdata->offfield_B != NULL)
                rdata->offfield_B(nroom);
            init_room_sparks(nroom);
        }
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    room->slabs_list = 0;
    room->slabs_count = 0;
}

TbBool delete_room_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned char gnd_slab)
{
    struct Room *room;
    SlabCodedCoords slb_num;
    //return _DK_delete_room_slab(slb_x, slb_y, gnd_slab);
    room = slab_room_get(slb_x,slb_y);
    if (room_is_invalid(room))
    {
        ERRORLOG("Slab (%ld,%ld) is not a room",slb_x, slb_y);
        return false;
    }
    SYNCDBG(7,"Room on (%d,%d) had %d slabs",(int)slb_x,(int)slb_y,(int)room->slabs_count);
    decrease_room_area(room->owner, 1);
    kill_room_slab_and_contents(room->owner, slb_x, slb_y);
    if (room->slabs_count <= 1)
    {
        delete_room_flag(room);
        replace_room_slab(room, slb_x, slb_y, room->owner, gnd_slab);
        kill_all_room_slabs_and_contents(room);
        free_room_structure(room);
        do_slab_efficiency_alteration(slb_x, slb_y);
    } else
    {
        slb_num = get_slab_number(slb_x, slb_y);
        // Remove the slab from room tiles list
        remove_slab_from_room_tiles_list(room, slb_num);
        replace_room_slab(room, slb_x, slb_y, room->owner, gnd_slab);
        // Create a new room from slabs left in old one
        recreate_rooms_from_room_slabs(room, gnd_slab);
        reset_creatures_rooms(room);
        free_room_structure(room);
    }
    return true;
}

void tag_cursor_blocks_sell_area(unsigned char a1, long a2, long a3, long a4)
{
    SYNCDBG(7,"Starting");
    _DK_tag_cursor_blocks_sell_area(a1, a2, a3, a4);
}

long packet_place_door(long a1, long a2, long a3, long a4, unsigned char a5)
{
    return _DK_packet_place_door(a1, a2, a3, a4, a5);
}

void delete_room_slabbed_objects(long a1)
{
    SYNCDBG(17,"Starting");
    _DK_delete_room_slabbed_objects(a1);
}

unsigned char tag_cursor_blocks_place_door(unsigned char a1, long a2, long a3)
{
    SYNCDBG(7,"Starting");
    return _DK_tag_cursor_blocks_place_door(a1, a2, a3);
}

long remove_workshop_item(long owner, long tngclass, long tngmodel)
{
    SYNCDBG(8,"Starting");
    return _DK_remove_workshop_item(owner, tngclass, tngmodel);
}

unsigned char tag_cursor_blocks_place_room(unsigned char a1, long a2, long a3, long a4)
{
    SYNCDBG(7,"Starting");
    return _DK_tag_cursor_blocks_place_room(a1, a2, a3, a4);
}

/**
 * Updates thing interaction with rooms. Sometimes deletes the given thing.
 * @param thing Thing to be checked, and assimilatedor deleted.
 * @return True if the thing was asimilated, false if it was deleted.
 */
short check_and_asimilate_thing_by_room(struct Thing *thing)
{
  struct Room *room;
  unsigned long n;
  if (thing_model_is_gold_hoarde(thing->model))
  {
    room = get_room_thing_is_on(thing);
    if (room == NULL)
    {
        delete_thing_structure(thing, 0);
        return false;
    }
    n = (gold_per_hoarde/5)*(((long)thing->model)-51);
    thing->owner = room->owner;
    add_gold_to_hoarde(thing, room, n);
  }
  return true;
}

short thing_create_thing(struct InitThing *itng)
{
  struct Thing *thing;
  if (itng->owner == 7)
  {
    ERRORLOG("Invalid owning player %d, fixing to %d", (int)itng->owner, (int)game.hero_player_num);
    itng->owner = game.hero_player_num;
  } else
  if (itng->owner == 8)
  {
    ERRORLOG("Invalid owning player %d, fixing to %d", (int)itng->owner, (int)game.neutral_player_num);
    itng->owner = game.neutral_player_num;
  }
  if (itng->owner > 5)
  {
    ERRORLOG("Invalid owning player %d, thing discarded", (int)itng->owner);
    return false;
  }
  switch (itng->oclass)
  {
  case TCls_Object:
      thing = create_thing(&itng->mappos, itng->oclass, itng->model, itng->owner, itng->index);
      if (!thing_is_invalid(thing))
      {
          if (itng->model == 49)
              thing->byte_13 = itng->params[1];
          check_and_asimilate_thing_by_room(thing);
          // make sure we don't have invalid pointer
          thing = INVALID_THING;
      } else
      {
          ERRORLOG("Couldn't create object model %d", (int)itng->model);
          return false;
      }
      break;
  case TCls_Creature:
      thing = create_creature(&itng->mappos, itng->model, itng->owner);
      if (thing_is_invalid(thing))
      {
          ERRORLOG("Couldn't create creature model %d", (int)itng->model);
          return false;
      }
      init_creature_level(thing, itng->params[1]);
      break;
  case TCls_EffectGen:
      thing = create_effect_generator(&itng->mappos, itng->model, itng->range, itng->owner, itng->index);
      if (thing_is_invalid(thing))
      {
          ERRORLOG("Couldn't create effect generator model %d", (int)itng->model);
          return false;
      }
      break;
  case TCls_Trap:
      thing = create_thing(&itng->mappos, itng->oclass, itng->model, itng->owner, itng->index);
      if (thing_is_invalid(thing))
      {
          ERRORLOG("Couldn't create trap model %d", (int)itng->model);
          return false;
      }
      break;
  case TCls_Door:
      thing = create_door(&itng->mappos, itng->model, itng->params[0], itng->owner, itng->params[1]);
      if (thing_is_invalid(thing))
      {
          ERRORLOG("Couldn't create door model %d", (int)itng->model);
          return false;
      }
      break;
  case 10:
  case 11:
      thing = create_thing(&itng->mappos, itng->oclass, itng->model, itng->owner, itng->index);
      if (thing_is_invalid(thing))
      {
          ERRORLOG("Couldn't create thing class %d model %d", (int)itng->oclass, (int)itng->model);
          return false;
      }
      break;
  default:
      ERRORLOG("Invalid class %d, thing discarded", (int)itng->oclass);
      return false;
  }
  return true;
}

void initialise_map_collides(void)
{
  SYNCDBG(7,"Starting");
  _DK_initialise_map_collides();
}

void initialise_map_health(void)
{
  SYNCDBG(7,"Starting");
  _DK_initialise_map_health();
}

long slabs_count_near(long tx,long ty,long rad,unsigned short slbkind)
{
  long dx,dy;
  long x,y;
  long count;
  count=0;
  struct SlabMap *slb;
  for (dy=-rad; dy <= rad; dy++)
  {
    y = ty+dy;
    if ((y>=0) && (y<map_tiles_y))
      for (dx=-rad; dx <= rad; dx++)
      {
        x = tx+dx;
        if ((x>=0) && (x<map_tiles_x))
        {
          slb = get_slabmap_block(x, y);
          if (slb->kind == slbkind)
            count++;
        }
      }
  }
  return count;
}

long ceiling_init(unsigned long a1, unsigned long a2)
{
  return _DK_ceiling_init(a1, a2);
}

long process_temple_special(struct Thing *thing)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(thing->owner);
    if (object_is_mature_food(thing))
    {
        dungeon->chickens_sacrificed++;
    } else
    {
        dungeon->field_8D5++;
    }
    return 0;
}

void do_creature_swap(long ncrt_id, long crtr_id)
{
//TODO: rewrite from DD
  WARNMSG("Swaping creatures is only supported in Deeper Dungeons");
}

TbBool swap_creature(long ncrt_id, long crtr_id)
{
  if ((crtr_id < 0) || (crtr_id >= CREATURE_TYPES_COUNT))
  {
      ERRORLOG("Creature index %d is invalid", crtr_id);
      return false;
  }
  if (creature_swap_idx[crtr_id] > 0)
  {
      ERRORLOG("Creature of index %d already swapped", crtr_id);
      return false;
  }
  do_creature_swap(ncrt_id, crtr_id);
  return true;
}

void init_dungeon_owner(unsigned short owner)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    int i,k;
    k = 0;
    i = game.thing_lists[2].index;
    while (i>0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
          break;
        i = thing->next_of_class;
        if ((game.objects_config[thing->model].field_6) && (thing->owner == owner))
        {
          dungeon = get_dungeon(owner);
          dungeon->dnheart_idx = thing->index;
          break;
        }
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
}

void init_level(void)
{
    SYNCDBG(6,"Starting");
    struct CreatureStorage transfer_mem;
    //_DK_init_level(); return;
    LbMemoryCopy(&transfer_mem,&game.transfered_creature,sizeof(struct CreatureStorage));
    game.flags_gui = 0;
    game.action_rand_seed = 1;
    free_swipe_graphic();
    game.field_1516FF = -1;
    game.play_gameturn = 0;
    clear_game();
    reset_heap_manager();
    lens_mode = 0;
    setup_heap_manager();
    load_computer_player_config();
    init_good_player_as(hero_player_number);

    light_set_lights_on(1);
    start_rooms = &game.rooms[1];
    end_rooms = &game.rooms[ROOMS_COUNT];

    erstats_clear();
    init_dungeons();
    preload_script(get_selected_level_number());
    load_map_file(get_selected_level_number());

    init_navigation();
    clear_messages();
    LbStringCopy(game.campaign_fname,campaign.fname,sizeof(game.campaign_fname));
    // Initialize unsynchnonized random seed (the value may be different
    // on computers in MP, as it shouldn't affect game actions)
    game.unsync_rand_seed = (unsigned long)LbTimeSec();
    if (!SoundDisabled)
    {
        game.field_14BB54 = (UNSYNC_RANDOM(67) % 3 + 1);
        game.field_14BB55 = 0;
    }
    light_set_lights_on(1);
    init_dungeon_owner(game.hero_player_num);
    game.numfield_D |= 0x04;
    LbMemoryCopy(&game.transfered_creature,&transfer_mem,sizeof(struct CreatureStorage));
    event_initialise_all();
    battle_initialise();
    ambient_sound_prepare();
    zero_messages();
    game.field_150356 = 0;
    game.field_15035A = 0;
    init_messages();
    game.creatures_tend_1 = 0;
    game.creatures_tend_2 = 0;
    game.field_15033A = 0;
    game.field_151801 = 0;
    game.field_151805 = 0;
    game.field_151809 = 0;
    game.chosen_spell_type = 0;
    game.chosen_spell_look = 0;
    game.chosen_spell_tooltip = 0;
    game.manufactr_element = 0;
    game.numfield_15181D = 0;
    game.manufactr_tooltip = 0;
}

void pannel_map_update(long x, long y, long w, long h)
{
    SYNCDBG(7,"Starting");
    _DK_pannel_map_update(x, y, w, h);
}

void set_chosen_spell(long sptype, long sptooltip)
{
    struct SpellData *pwrdata;
    pwrdata = get_power_data(sptype);
    if (power_data_is_invalid(pwrdata))
      sptype = 0;
    SYNCDBG(6,"Setting to %ld",sptype);
    game.chosen_spell_type = sptype;
    game.chosen_spell_look = pwrdata->field_9;
    game.chosen_spell_tooltip = sptooltip;
}

void set_chosen_spell_none(void)
{
    SYNCDBG(6,"Setting to %d",0);
    game.chosen_spell_type = 0;
    game.chosen_spell_look = 0;
    game.chosen_spell_tooltip = 0;
}

void init_player_music(struct PlayerInfo *player)
{
    LevelNumber lvnum;
    lvnum = get_loaded_level_number();
    game.audiotrack = ((lvnum - 1) % -4) + 3;
    randomize_sound_font();
}

void init_player(struct PlayerInfo *player, short no_explore)
{
    SYNCDBG(5,"Starting");
    //_DK_init_player(player, no_explore); return;
    player->mouse_x = 10;
    player->mouse_y = 12;
    player->minimap_zoom = 256;
    player->field_4D1 = player->id_number;
    setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
    player->continue_work_state = PSt_CtrlDungeon;
    player->work_state = PSt_CtrlDungeon;
    player->field_14 = 2;
    player->palette = _DK_palette;
    if (is_my_player(player))
    {
        set_flag_byte(&game.numfield_C,0x40,true);
        set_gui_visible(true);
        init_gui();
        turn_on_menu(GMnu_MAIN);
        turn_on_menu(GMnu_ROOM);
    }
    switch (game.game_kind)
    {
    case GKind_NetworkGame:
        init_player_as_single_keeper(player);
        init_player_start(player);
        reset_player_mode(player, 1);
        if ( !no_explore )
          init_keeper_map_exploration(player);
        break;
    case GKind_KeeperGame:
        if (player->field_2C != 1)
        {
          ERRORLOG("Non Keeper in Keeper game");
          break;
        }
        init_player_as_single_keeper(player);
        init_player_start(player);
        reset_player_mode(player, 1);
        init_keeper_map_exploration(player);
        break;
    default:
        ERRORLOG("How do I set up this player?");
        break;
    }
    init_player_cameras(player);
    pannel_map_update(0, 0, map_subtiles_x+1, map_subtiles_y+1);
    player->strfield_463[0] = '\0';
    if (is_my_player(player))
    {
        init_player_music(player);
    }
    player->allied_players = (1 << player->id_number);
    player->field_10 = 0;
}

void init_players(void)
{
    struct PlayerInfo *player;
    int i;
    for (i=0;i<PLAYERS_COUNT;i++)
    {
        player = get_player(i);
        player->field_0 ^= (player->field_0 ^ ((game.packet_save_head.field_C & (1 << i)) >> i)) & 1;
        if (player_exists(player))
        {
            player->id_number = i;
            player->field_0 ^= (player->field_0 ^ (((game.packet_save_head.field_D & (1 << i)) >> i) << 6)) & 0x40;
            if ((player->field_0 & 0x40) == 0)
            {
              game.field_14E495++;
              player->field_2C = 1;
              game.game_kind = GKind_KeeperGame;
              init_player(player, 0);
            }
        }
    }
}

TbBool create_transferred_creature_on_level(void)
{
    struct PlayerInfo *player;
    struct Thing *thing;
    struct Dungeon *dungeon;
    struct Coord3d *pos;
    if (game.transfered_creature.model > 0)
    {
        player = get_my_player();
        dungeon = get_dungeon(player->id_number);
        thing = thing_get(dungeon->dnheart_idx);
        pos = &(thing->mappos);
        thing = create_creature(pos, game.transfered_creature.model, 5);
        if (thing_is_invalid(thing))
          return false;
        init_creature_level(thing, game.transfered_creature.explevel);
        clear_transfered_creature();
        return true;
    }
    return false;
}

void post_init_level(void)
{
    SYNCDBG(8,"Starting");
    struct Dungeon *dungeon;
    //_DK_post_init_level(); return;
    if (game.packet_save_enable)
        open_new_packet_file_for_save();
    calculate_dungeon_area_scores();
    init_animating_texture_maps();
    int i,k;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
        dungeon = get_dungeon(i);
        for (k=0; k < CREATURE_TYPES_COUNT; k++)
        {
          dungeon->creature_max_level[k] = 10;
        }
    }
    clear_creature_pool();
    setup_computer_players2();
    load_stats_files();
    check_and_auto_fix_stats();
    load_script(get_loaded_level_number());
    init_dungeons_research();
    create_transferred_creature_on_level();
    update_dungeon_scores();
    update_dungeon_generation_speeds();
    init_traps();
    init_all_creature_states();
    init_keepers_map_exploration();
    SYNCDBG(9,"Finished");
}

void post_init_players(void)
{
    _DK_post_init_players(); return;
}

short init_animating_texture_maps(void)
{
    SYNCDBG(8,"Starting");
    //_DK_init_animating_texture_maps(); return;
    anim_counter = 7;
    return update_animating_texture_maps();
}

void init_players_local_game(void)
{
    struct PlayerInfo *player;
    SYNCDBG(4,"Starting");
    player = get_my_player();
    player->id_number = my_player_number;
    player->field_0 |= 0x01;
    if (settings.field_3 < 1u)
      player->field_4B5 = 2;
    else
      player->field_4B5 = 5;
    init_player(player, 0);
}

void startup_saved_packet_game(void)
{
    struct CatalogueEntry centry;
    //_DK_startup_saved_packet_game(); return;
    clear_packets();
    open_packet_file_for_load(game.packet_fname,&centry);
    if (!change_campaign(centry.campaign_fname))
    {
        ERRORLOG("Unable to load campaign associated with packet file");
    }
    set_selected_level_number(game.packet_save_head.level_num);
    lbDisplay.DrawColour = colours[15][15][15];
    game.pckt_gameturn = 0;
#if (BFDEBUG_LEVEL > 0)
    SYNCDBG(0,"Initialising level %d", (int)get_selected_level_number());
    SYNCMSG("Packet Loading Active (File contains %d turns)", game.turns_stored);
    if ( game.packet_checksum )
      SYNCMSG("Packet Checksum Active");
    SYNCMSG("Fast Forward through %d game turns", game.turns_fastforward);
    if (game.turns_packetoff != -1)
      SYNCMSG("Packet Quit at %d", game.turns_packetoff);
    if (game.packet_load_enable)
    {
      if (game.log_things_end_turn != game.log_things_start_turn)
        SYNCMSG("Logging things, game turns %d -> %d", game.log_things_start_turn, game.log_things_end_turn);
    }
#endif
    game.game_kind = GKind_NetworkGame;
    if (!(game.packet_save_head.field_C & (1 << game.numfield_149F46))
      || (game.packet_save_head.field_D & (1 << game.numfield_149F46)))
      my_player_number = 0;
    else
      my_player_number = game.numfield_149F46;
    init_level();
    init_players();
    if (game.field_14E495 == 1)
      game.game_kind = GKind_NetworkGame;
    if (game.turns_stored < game.turns_fastforward)
      game.turns_fastforward = game.turns_stored;
    post_init_level();
    post_init_players();
    set_selected_level_number(0);
}

void faststartup_saved_packet_game(void)
{
    struct PlayerInfo *player;
    reenter_video_mode();
    startup_saved_packet_game();
    player = get_my_player();
    player->field_6 &= 0xFDu;
    set_gui_visible(false);
    set_flag_byte(&game.numfield_C,0x40,false);
}

void startup_network_game(TbBool local)
{
    SYNCDBG(0,"Starting up network game");
    //_DK_startup_network_game(); return;
    unsigned int flgmem;
    struct PlayerInfo *player;
    setup_count_players();
    player = get_my_player();
    flgmem = player->field_2C;
    init_level();
    player = get_my_player();
    player->field_2C = flgmem;
    //if (game.flagfield_14EA4A == 2) //was wrong because init_level sets this to 2. global variables are evil (though perhaps that's why they were chosen for DK? ;-))
    if (local)
    {
        game.game_kind = GKind_NetworkGame;
        init_players_local_game();
    } else
    {
        game.game_kind = GKind_KeeperGame;
        init_players_network_game();
    }
    if (fe_computer_players)
    {
        SYNCDBG(5,"Setting up uninitialized players as computer players");
        setup_computer_players();
    } else
    {
        SYNCDBG(5,"Setting up uninitialized players as zombie players");
        setup_zombie_players();
    }
    post_init_level();
    post_init_players();
    post_init_packets();
    set_selected_level_number(0);
    //LbNetwork_EnableLag(1);
}

void faststartup_network_game(void)
{
    struct PlayerInfo *player;
    reenter_video_mode();
    my_player_number = default_loc_player;
    game.game_kind = GKind_NetworkGame;
    if (!is_campaign_loaded())
    {
      if (!change_campaign(""))
        ERRORLOG("Unable to load campaign");
    }
    player = get_my_player();
    player->field_2C = 1;
    startup_network_game(true);
    player = get_my_player();
    player->field_6 &= ~0x02;
}

void wait_at_frontend(void)
{
  struct PlayerInfo *player;
  SYNCDBG(0,"Falling into frontend menu.");
  // Moon phase calculation
  calculate_moon_phase(true,false);
  update_extra_levels_visibility();
  // Returning from Demo Mode
  if (game.flags_cd & MFlg_IsDemoMode)
  {
    close_packet_file();
    game.packet_load_enable = 0;
  }
  game.numfield_15 = -1;
  // Make sure campaign is loaded
  if (!load_campaigns_list())
  {
    ERRORLOG("No valid campaign files found");
    exit_keeper = 1;
    return;
  }
  // Init load/save catalogue
  initialise_load_game_slots();
  // Prepare to enter PacketLoad game
  if ((game.packet_load_enable) && (!game.numfield_149F47))
  {
    faststartup_saved_packet_game();
    return;
  }
  // Prepare to enter network/standard game
  if ((game.numfield_C & 0x02) != 0)
  {
    faststartup_network_game();
    return;
  }

  if ( !setup_screen_mode_minimal(get_frontend_vidmode()) )
  {
    FatalError = 1;
    exit_keeper = 1;
    return;
  }
  LbScreenClear(0);
  LbScreenSwap();
  if ( !frontend_load_data() )
  {
    ERRORLOG("Unable to load frontend data");
    exit_keeper = 1;
    return;
  }
  memset(scratch, 0, PALETTE_SIZE);
  LbPaletteSet(scratch);
  frontend_set_state(get_startup_menu_state());

  short finish_menu = 0;
  set_flag_byte(&game.flags_cd,0x40,false);
  // Begin the frontend loop
  long last_loop_time = LbTimerClock();
  do
  {
    if (!LbWindowsControl())
    {
      if ((game.system_flags & GSF_NetworkActive) == 0)
      {
          exit_keeper = 1;
          SYNCDBG(0,"Windows Control exit condition invoked");
          break;
      }
    }
    update_mouse();
    update_key_modifiers();
    old_mouse_over_button = frontend_mouse_over_button;
    frontend_mouse_over_button = 0;

    frontend_input();
    if ( exit_keeper )
    {
      SYNCDBG(0,"Frontend Input exit condition invoked");
      break; // end while
    }

    frontend_update(&finish_menu);
    if ( exit_keeper )
    {
      SYNCDBG(0,"Frontend Update exit condition invoked");
      break; // end while
    }

    if ((!finish_menu) && (LbIsActive()))
    {
      frontend_draw();
      LbScreenSwap();
    }

    if (!SoundDisabled)
    {
      process_3d_sounds();
      process_sound_heap();
      MonitorStreamedSoundTrack();
    }

    if (fade_palette_in)
    {
      fade_in();
      fade_palette_in = 0;
    } else
    {
      LbSleepUntil(last_loop_time + 30);
    }
    last_loop_time = LbTimerClock();
  } while (!finish_menu);

  LbPaletteFade(0, 8, Lb_PALETTE_FADE_CLOSED);
  LbScreenClear(0);
  LbScreenSwap();
  short prev_state;
  prev_state = frontend_menu_state;
  frontend_set_state(0);
  if (exit_keeper)
  {
    player = get_my_player();
    player->field_6 &= ~0x02;
    return;
  }
  reenter_video_mode();

  display_loading_screen();
  short flgmem;
  switch (prev_state)
  {
  case FeSt_UNKNOWN07:
        my_player_number = default_loc_player;
        game.game_kind = GKind_NetworkGame;
        set_flag_byte(&game.system_flags,GSF_NetworkActive,false);
        player = get_my_player();
        player->field_2C = 1;
        startup_network_game(true);
        break;
  case FeSt_UNKNOWN08:
        set_flag_byte(&game.system_flags,GSF_NetworkActive,true);
        game.game_kind = GKind_KeeperGame;
        player = get_my_player();
        player->field_2C = 1;
        startup_network_game(false);
        break;
  case FeSt_LOAD_GAME:
        flgmem = game.numfield_15;
        set_flag_byte(&game.system_flags,GSF_NetworkActive,false);
        LbScreenClear(0);
        LbScreenSwap();
        if (!load_game(game.numfield_15))
        {
            ERRORLOG("Loading game %d failed; quitting.",(int)game.numfield_15);
            quit_game = 1;
        }
        game.numfield_15 = flgmem;
        break;
  case FeSt_PACKET_DEMO:
        game.flags_cd |= MFlg_IsDemoMode;
        startup_saved_packet_game();
        set_gui_visible(false);
        set_flag_byte(&game.numfield_C,0x40,false);
        break;
  }
  player = get_my_player();
  player->field_6 &= ~0x02;
}

void game_loop(void)
{
  //_DK_game_loop(); return;
  unsigned long random_seed;
  unsigned long playtime;
  playtime = 0;
  random_seed = 0;
  SYNCDBG(0,"Entering gameplay loop.");
  while ( !exit_keeper )
  {
    update_mouse();
    wait_at_frontend();
    if ( exit_keeper )
      break;
    struct PlayerInfo *player;
    player = get_my_player();
    if (game.game_kind == GKind_NetworkGame)
    {
      if (game.numfield_15 == -1)
      {
        set_player_instance(player, 11, 0);
      } else
      {
        game.numfield_15 = -1;
        set_flag_byte(&game.numfield_C,0x01,false);
      }
    }
    unsigned long starttime;
    unsigned long endtime;
    struct Dungeon *dungeon;
    // get_my_dungeon() can't be used here because players are not initialized yet
    dungeon = get_dungeon(my_player_number);
    starttime = LbTimerClock();
    dungeon->lvstats.start_time = starttime;
    dungeon->lvstats.end_time = starttime;
    LbScreenClear(0);
    LbScreenSwap();
    keeper_gameplay_loop();
    set_pointer_graphic_none();
    LbScreenClear(0);
    LbScreenSwap();
    StopRedbookTrack();
    StopMusic();
    turn_off_all_menus();
    delete_all_structures();
    clear_mapwho();
    endtime = LbTimerClock();
    quit_game = 0;
    if ((game.numfield_C & 0x02) != 0)
        exit_keeper=true;
    playtime += endtime-starttime;
    SYNCDBG(0,"Play time is %d seconds",playtime>>10);
    random_seed += game.play_gameturn;
    reset_eye_lenses();
    close_packet_file();
    game.packet_load_enable = false;
    game.packet_save_enable = false;
  } // end while
  // Stop the movie recording if it's on
  if ((game.system_flags & GSF_CaptureMovie) != 0)
    movie_record_stop();
}

short reset_game(void)
{
  SYNCDBG(6,"Starting");
  _DK_IsRunningUnmark();

  KeeperSpeechExit();

  LbMouseSuspend();
  LbIKeyboardClose();
  LbScreenReset();
  LbDataFreeAll(game_load_files);
  free_gui_strings_data();
  FreeAudio();
  return LbMemoryReset();
}

short process_command_line(unsigned short argc, char *argv[])
{
  char fullpath[CMDLN_MAXLEN+1];
  strncpy(fullpath, argv[0], CMDLN_MAXLEN);

  sprintf( keeper_runtime_directory, fullpath);
  char *endpos = strrchr( keeper_runtime_directory, '\\');
  if (endpos==NULL)
      endpos=strrchr( keeper_runtime_directory, '/');
  if (endpos!=NULL)
      *endpos='\0';

  SoundDisabled = 0;
  // Note: the working log file is set up in LbBullfrogMain
  _DK_LbErrorLogSetup(0, 0, 1);

  set_default_startup_parameters();

  short bad_param;
  LevelNumber level_num;
  bad_param = 0;
  unsigned short narg;
  level_num = LEVELNUMBER_ERROR;
  narg = 1;
  while ( narg < argc )
  {
      char *par;
      par = argv[narg];
      if ( (par == NULL) || ((par[0] != '-') && (par[0] != '/')) )
          return -1;
      char parstr[CMDLN_MAXLEN+1];
      char pr2str[CMDLN_MAXLEN+1];
      strncpy(parstr, par+1, CMDLN_MAXLEN);
      if (narg+1 < argc)
        strncpy(pr2str,  argv[narg+1], CMDLN_MAXLEN);
      else
        pr2str[0]='\0';

      if (strcasecmp(parstr, "nointro") == 0)
      {
        start_params.no_intro = 1;
      } else
      if (strcasecmp(parstr, "nocd") == 0)
      {
          set_flag_byte(&start_params.flags_cd,MFlg_NoMusic,true);
      } else
      if (strcasecmp(parstr, "1player") == 0)
      {
          start_params.one_player = 1;
      } else
      if ((strcasecmp(parstr, "s") == 0) || (strcasecmp(parstr, "nosound") == 0))
      {
          SoundDisabled = 1;
      } else
      if (strcasecmp(parstr, "fps") == 0)
      {
          narg++;
          start_params.num_fps = atoi(pr2str);
      } else
      if (strcasecmp(parstr, "human") == 0)
      {
          narg++;
          default_loc_player = atoi(pr2str);
      } else
      if (strcasecmp(parstr, "usersfont") == 0)
      {
          set_flag_byte(&start_params.flags_font,FFlg_UsrSndFont,true);
      } else
      if (strcasecmp(parstr, "vidsmooth") == 0)
      {
          smooth_on = 1;
      } else
      if ( strcasecmp(parstr,"level") == 0 )
      {
        set_flag_byte(&start_params.numfield_C,0x02,true);
        level_num = atoi(pr2str);
        narg++;
      } else
      if ( strcasecmp(parstr,"ppropoly") == 0 )
      {
          start_params.force_ppro_poly = atoi(pr2str);
          narg++;
      } else
      if ( strcasecmp(parstr,"altinput") == 0 )
      {
          SYNCLOG("Mouse auto reset disabled");
          lbMouseAutoReset = false;
      } else
      if ( strcasecmp(parstr,"vidriver") == 0 )
      {
          LbScreenHardwareConfig(pr2str,8);
          narg++;
      } else
      if (strcasecmp(parstr,"packetload") == 0)
      {
         if (start_params.packet_save_enable)
            WARNMSG("PacketSave disabled to enable PacketLoad.");
         start_params.packet_load_enable = true;
         start_params.packet_save_enable = false;
         strncpy(start_params.packet_fname,pr2str,149);
         narg++;
      } else
      if (strcasecmp(parstr,"packetsave") == 0)
      {
         if (start_params.packet_load_enable)
            WARNMSG("PacketLoad disabled to enable PacketSave.");
         start_params.packet_load_enable = false;
         start_params.packet_save_enable = true;
         strncpy(start_params.packet_fname,pr2str,149);
         narg++;
      } else
      if (strcasecmp(parstr,"q") == 0)
      {
         set_flag_byte(&start_params.numfield_C,0x02,true);
      } else
      if (strcasecmp(parstr,"columnconvert") == 0)
      {
         set_flag_byte(&start_params.numfield_C,0x08,true);
      } else
      if (strcasecmp(parstr,"lightconvert") == 0)
      {
         set_flag_byte(&start_params.numfield_C,0x10,true);
      } else
      if (strcasecmp(parstr, "dbgshots") == 0)
      {
          set_flag_byte(&start_params.debug_flags,DFlg_ShotsDamage,true);
      } else
      if (strcasecmp(parstr, "sessions") == 0) {
          narg++;
          LbNetwork_InitSessionsFromCmdLine(pr2str);
      } else
      if (strcasecmp(parstr,"alex") == 0)
      {
         set_flag_byte(&start_params.flags_font,FFlg_AlexCheat,true);
      } else
      {
        WARNMSG("Unrecognized command line parameter '%s'.",parstr);
        bad_param=narg;
      }
      narg++;
  }

  if (level_num == LEVELNUMBER_ERROR)
    level_num = first_singleplayer_level();
  start_params.selected_level_number = level_num;
  my_player_number = default_loc_player;
  return (bad_param==0);
}

int LbBullfrogMain(unsigned short argc, char *argv[])
{
  short retval;
  retval=0;
  LbErrorLogSetup("/", log_file_name, 5);
  LbScreenHardwareConfig("directx",8);

  retval=process_command_line(argc,argv);
  if ( retval < 1 )
  {
      static const char *msg_text="Command line parameters analysis failed.\n";
      error_dialog_fatal(__func__, 1, msg_text);
      LbErrorLogClose();
      return 0;
  }

  LbTimerInit();
  LbScreenInitialize();
  LbSetTitle(PROGRAM_NAME);
  LbSetIcon(1);
  LbScreenSetDoubleBuffering(true);
  srand(LbTimerClock());

  retval = setup_game();
  if (retval)
  {
    if ((install_info.lang_id == 16) || (install_info.lang_id == 17) ||
        (install_info.lang_id == 18))
    {
      switch (install_info.lang_id)
      {
      case 16:
          dbc_set_language(1);
          break;
      case 17:
          dbc_set_language(2);
          break;
      case 18:
          dbc_set_language(3);
          break;
      }
      if (dbc_initialize("fxdata"))
      {
        ERRORLOG("DBC fonts Initialization failed.");
      }
    }
  }
  if ( retval )
  {
      game_loop();
  }
  reset_game();
  LbScreenReset();
  if ( !retval )
  {
      static const char *msg_text="Setting up game failed.\n";
      error_dialog_fatal(__func__, 2, msg_text);
  } else
  {
    SYNCDBG(0,"finished properly");
  }
  LbErrorLogClose();
  return 0;
}

void get_cmdln_args(unsigned short &argc, char *argv[])
{
  char *ptr;
  const char *cmndln_orig;
  cmndln_orig = GetCommandLineA();
  strncpy(cmndline, cmndln_orig, CMDLN_MAXLEN);
  ptr = cmndline;
  bf_argc = 0;
  while (*ptr != '\0')
  {
      if ((*ptr == '\t') || (*ptr == ' '))
      {
          ptr++;
          continue;
      }
      if (*ptr == '\"')
      {
          ptr++;
          bf_argv[bf_argc] = ptr;
          bf_argc++;
          while (*ptr != '\0')
          {
            if (*ptr == '\"')
            {
              *ptr++ = '\0';
              break;
            }
            ptr++;
          }
      } else
      {
          bf_argv[bf_argc] = ptr;
          bf_argc++;
          while (*ptr != '\0')
          {
            if ((*ptr == '\t') || (*ptr == ' '))
            {
              *ptr++ = '\0';
              break;
            }
            ptr++;
          }
      }
  }
}

int main(int argc, char *argv[])
{
  char *text;
  _DK_hInstance = GetModuleHandle(NULL);

  get_cmdln_args(bf_argc, bf_argv);

//TODO: delete when won't be needed anymore
  memcpy(_DK_menu_list,menu_list,40*sizeof(struct GuiMenu *));
  memcpy(_DK_player_instance_info,player_instance_info,17*sizeof(struct PlayerInstanceInfo));
  memcpy(_DK_states,states,145*sizeof(struct StateInfo));
  memcpy(_DK_room_data,room_data,17*sizeof(struct RoomData));

#if (BFDEBUG_LEVEL > 1)
/*  {
      struct PlayerInfo *player;
      player = get_player(0);
      text = buf_sprintf("Position of the first Player is %06x, first Camera is %06x bytes.\n",((int)player) - ((int)&_DK_game),((int)&(player->acamera)) - ((int)player));
      error_dialog(__func__, 1, text);
      return 0;
  }
  {
      struct Dungeon *dungeon;
      dungeon = get_dungeon(0);
      text = buf_sprintf("Position of the first Dungeon is %06x, field_ACF is at %06x bytes.\n",
                  ((int)dungeon) - ((int)&game),((int)(&dungeon->field_ACF)) - ((int)dungeon));
      error_dialog(__func__, 1, text);
      return 0;
  }*/
  if (sizeof(struct Game) != SIZEOF_Game)
  {
      long delta1,delta2,delta3;
      if (sizeof(struct PlayerInfo) != SIZEOF_PlayerInfo)
      {
          text = buf_sprintf("Bad compilation - struct PlayerInfo has wrong size!\nThe difference is %d bytes.\n",sizeof(struct PlayerInfo)-SIZEOF_PlayerInfo);
          error_dialog(__func__, 1, text);
          return 1;
      }
      if (sizeof(struct Dungeon) != SIZEOF_Dungeon)
      {
          text = buf_sprintf("Bad compilation - struct Dungeon has wrong size!\nThe difference is %d bytes.\n",sizeof(struct Dungeon)-SIZEOF_Dungeon);
          error_dialog(__func__, 1, text);
          return 1;
      }
      if (sizeof(struct CreatureControl) != SIZEOF_CreatureControl)
      {
          //delta1 =((char *)&game.cctrl_data[0].moveto_pos) - ((char *)&game.cctrl_data);
          text = buf_sprintf("Bad compilation - struct CreatureControl has wrong size!\nThe difference is %d bytes.\n",sizeof(struct CreatureControl)-SIZEOF_CreatureControl);
          error_dialog(__func__, 1, text);
          return 1;
      }
      delta1 =((char *)&game.land_map_start) - ((char *)&game) - 0x1DD40;
      delta2 =((char *)&game.cctrl_data) - ((char *)&game) - 0x66157;
      delta3 =((char *)&game.creature_scores) - ((char *)&game) - 0x14EA4C;
      text = buf_sprintf("Bad compilation - struct Game has wrong size!\nThe difference is %d bytes.\n"
          "Field \"land_map_start\" is moved by %ld bytes.\nField \"cctrl_data\" is moved by %ld bytes.\n"
          "Field \"creature_scores\" is moved by %ld bytes.\n",sizeof(struct Game)-SIZEOF_Game,delta1,delta2,delta3);
      error_dialog(__func__, 1, text);
      return 1;
  }
  if (sizeof(struct S3DSample) != 37)
  {
      text = buf_sprintf("Bad compilation - struct S3DSample has wrong size!\nThe difference is %d bytes.\n",sizeof(struct S3DSample)-37);
      error_dialog(__func__, 1, text);
      return 1;
  }
#endif

  try {
  LbBullfrogMain(bf_argc, bf_argv);
  } catch (...)
  {
      text = buf_sprintf("Exception raised!");
      error_dialog(__func__, 1, text);
      return 1;
  }

//  LbFileSaveAt("!tmp_file", &_DK_game, sizeof(struct Game));

  return 0;
}
