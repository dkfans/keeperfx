//
// Created by Sim on 7/13/21.
// All this functions are located at main.cpp and should be moved out of there
//
#include <keeperfx.hpp>
#include <thing_data.h>

extern "C" {

int test_variable;

TbClockMSec timerstarttime = 0;
TbBool TimerGame = false;
TbBool TimerFreeze = false;
TbBool TimerNoReset = false;
struct TimerTime Timer;

struct StartupParameters start_params;
TbClockMSec last_loop_time=0;

void affect_nearby_enemy_creatures_with_wind(struct Thing *shotng)
{
}

void draw_lightning(const struct Coord3d *pos1, const struct Coord3d *pos2, long eeinterspace, long eemodel)
{
}

void affect_nearby_friends_with_alarm(struct Thing *traptng)
{
}

long apply_wallhug_force_to_boulder(struct Thing *thing)
{
    return 0;
}

unsigned long lightning_is_close_to_player(struct PlayerInfo *player, struct Coord3d *pos)
{
    return 0;
}

long update_cave_in(struct Thing *thing)
{
    return 0;
}

void update_thing_animation(struct Thing *thing)
{
}

TbBool any_player_close_enough_to_see(const struct Coord3d *pos)
{
    return 1;
}

long get_foot_creature_has_down(struct Thing *thing)
{
}

long packet_place_door(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel, unsigned char a5)
{
    return 0;
}

void turn_off_query(PlayerNumber plyr_idx)
{
}

void clear_computer(void)
{
}

void PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *pal)
{
}

void find_map_location_coords(long location, long *x, long *y, int plyr_idx, const char *func_name)
{
}

void clear_things_and_persons_data(void)
{
}

void draw_texture(long a1, long a2, long a3, long a4, long a5, long a6, long a7)
{
}

void give_shooter_drained_health(struct Thing *shooter, long health_delta)
{
}

void process_keeper_spell_effect(struct Thing *thing)
{
}

void draw_flame_breath(struct Coord3d *pos1, struct Coord3d *pos2, long delta_step, long num_per_step)
{
}

void update_time(void)
{
}

TbBool toggle_computer_player(PlayerNumber plyr_idx)
{
}

void tag_cursor_blocks_dig(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long full_slab)
{
}

long ceiling_init(unsigned long a1, unsigned long a2)
{
    return 0;
}

TbBool swap_creature(long ncrt_id, long crtr_id)
{
    return 1;
}

TbBool tag_cursor_blocks_place_door(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    return 1;
}

void set_mouse_light(struct PlayerInfo *player)
{
}

TbBool screen_to_map(struct Camera *camera, long screen_x, long screen_y, struct Coord3d *mappos)
{
    return 1;
}

__attribute__((regparm(3))) struct GameTime get_game_time(unsigned long turns, unsigned long fps)
{
    struct GameTime GameT = {0};
    return GameT;
}

void instant_instance_selected(CrInstance check_inst_id)
{
}

short complete_level(struct PlayerInfo *player)
{
    return 0;
}

short lose_level(struct PlayerInfo *player)
{
    return 0;
}

short resign_level(struct PlayerInfo *player)
{
    return 0;
}

void clear_creature_pool(void)
{
}

TbBool can_thing_be_queried(struct Thing *thing, PlayerNumber plyr_idx)
{
    return 1;
}

void set_player_cameras_position(struct PlayerInfo *player, long pos_x, long pos_y)
{
}

void clear_game_for_save(void)
{
}

void init_lookups(void)
{
}

TbBool set_gamma(char corrlvl, TbBool do_set)
{
    return 1;
}

TbBool all_dungeons_destroyed(const struct PlayerInfo *win_player)
{
    return 0;
}

void tag_cursor_blocks_thing_in_hand(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, int is_special_digger, long full_slab)
{
}

void reinit_level_after_load(void)
{
}

void centre_engine_window(void)
{
}

void change_engine_window_relative_size(long w_delta, long h_delta)
{
}

void level_lost_go_first_person(PlayerNumber plyr_idx)
{
}

short winning_player_quitting(struct PlayerInfo *player, long *plyr_count)
{
    return 0;
}

struct Thing *get_queryable_object_near(MapCoord pos_x, MapCoord pos_y, long plyr_idx)
{
    return NULL;
}

TbBool tag_cursor_blocks_sell_area(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long full_slab)
{
    return 1;
}

TbBool tag_cursor_blocks_place_room(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long full_slab)
{
    return 1;
}

void reset_script_timers_and_flags(void)
{
}

void reset_creature_max_levels(void)
{
}

void set_quick_information(long msg_id, long target, long x, long y)
{
}

void process_objective(const char *msg_text, long target, long x, long y)
{
}

void set_general_objective(long msg_id, long target, long x, long y)
{
}

void set_general_information(long msg_id, long target, long x, long y)
{
}

short zoom_to_next_annoyed_creature(void)
{
    return 1;
}

void toggle_hero_health_flowers(void)
{
}

void update_creatr_model_activities_list(void)
{
}

void reset_gui_based_on_player_mode(void)
{
}

extern TbPixel player_path_colours[];
TbPixel get_player_path_colour(unsigned short owner)
{
    return player_path_colours[0];
}

void initialise_map_collides(void)
{
}

void clear_map(void)
{
}

void initialise_map_health(void)
{
}

void engine(struct PlayerInfo *player, struct Camera *cam)
{
}

} //extern "C"
