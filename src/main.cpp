/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file main.cpp
 * @author KeeperFX Team
 * @date 01 Aug 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"

#include "platform.h"
#include "keeperfx.hpp"

#include "bflib_coroutine.h"
#include "bflib_math.h"
#include "bflib_keybrd.h"
#include "bflib_inputctrl.h"
#include "bflib_datetm.h"
#include "bflib_sprfnt.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_sndlib.h"
#include "bflib_cpu.h"
#include "bflib_crash.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "bflib_guibtns.h"
#include "bflib_sound.h"
#include "bflib_mouse.h"
#include "bflib_mshandler.hpp"
#include "bflib_filelst.h"
#include "bflib_network.h"
#include "net_resync.h"
#include "bflib_planar.h"

#include "api.h"
#include "custom_sprites.h"
#include "version.h"
#include "front_simple.h"
#include "frontend.h"
#include "front_input.h"
#include "frontmenu_net.h"
#include "gui_parchment.h"
#include "gui_frontmenu.h"
#include "gui_msgs.h"
#include "scrcapt.h"
#include "vidmode.h"
#include "kjm_input.h"
#include "packets.h"
#include "config.h"
#include "config_slabsets.h"
#include "config_strings.h"
#include "config_campaigns.h"
#include "config_terrain.h"
#include "config_objects.h"
#include "config_magic.h"
#include "config_creature.h"
#include "config_compp.h"
#include "config_effects.h"
#include "lua_triggers.h"
#include "lvl_script.h"
#include "lvl_filesdk1.h"
#include "thing_list.h"
#include "player_instances.h"
#include "player_utils.h"
#include "config_players.h"
#include "player_computer.h"
#include "game_heap.h"
#include "game_saves.h"
#include "engine_render.h"
#include "engine_lenses.h"
#include "engine_camera.h"
#include "local_camera.h"
#include "engine_arrays.h"
#include "engine_textures.h"
#include "engine_redraw.h"
#include "front_easter.h"
#include "front_fmvids.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_creature.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_doors.h"
#include "thing_traps.h"
#include "thing_navigate.h"
#include "thing_shots.h"
#include "thing_factory.h"
#include "slab_data.h"
#include "room_data.h"
#include "room_entrance.h"
#include "room_util.h"
#include "map_columns.h"
#include "map_ceiling.h"
#include "map_events.h"
#include "map_utils.h"
#include "map_blocks.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "creature_graphics.h"
#include "creature_states_combt.h"
#include "creature_states_mood.h"
#include "lens_api.h"
#include "light_data.h"
#include "magic_powers.h"
#include "power_process.h"
#include "power_hand.h"
#include "game_merge.h"
#include "gui_topmsg.h"
#include "gui_boxmenu.h"
#include "gui_soundmsgs.h"
#include "gui_frontbtns.h"
#include "frontmenu_ingame_tabs.h"
#include "frontmenu_ingame_evnt.h"
#include "ariadne.h"
#include "sounds.h"
#include "vidfade.h"
#include "KeeperSpeech.h"
#include "config_settings.h"
#include "config_keeperfx.h"
#include "game_legacy.h"
#include "room_list.h"
#include "steam_api.hpp"
#include "game_loop.h"
#include "net_input_lag.h"
#include "moonphase.h"
#include "frontmenu_ingame_map.h"
#include <stdint.h>

#ifdef FUNCTESTING
  #include "ftests/ftest.h"
#endif

#include "post_inc.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

short do_draw;
short default_loc_player = 0;
struct StartupParameters start_params;
char autostart_multiplayer_campaign[80] = "";
int autostart_multiplayer_level = 0;
int32_t game_num_fps;

int32_t game_num_fps_draw_current = 0;
int32_t game_num_fps_draw_main = 0; // -1 if auto
int32_t game_num_fps_draw_secondary = 0;


unsigned char *blue_palette;
unsigned char *red_palette;
unsigned char *dog_palette;
unsigned char *vampire_palette;
unsigned char exit_keeper;
unsigned char quit_game;
unsigned char is_running_under_wine = false;
int continue_game_option_available;
int32_t last_mouse_x;
int32_t last_mouse_y;
int FatalError;
int32_t define_key_scroll_offset;
uint32_t time_last_played_demo;
short drag_menu_x;
short drag_menu_y;
unsigned short tool_tip_time;
unsigned short help_tip_time;
int32_t pointer_x;
int32_t pointer_y;
int32_t block_pointed_at_x;
int32_t block_pointed_at_y;
int32_t pointed_at_frac_x;
int32_t pointed_at_frac_y;
int32_t top_pointed_at_x;
int32_t top_pointed_at_y;
int32_t top_pointed_at_frac_x;
int32_t top_pointed_at_frac_y;
char level_name[88];
char top_of_breed_list;
/** Amount of different creature kinds the local player has. Used for creatures tab in panel menu. */
char no_of_breeds_owned;
int32_t optimised_lights;
int32_t total_lights;
unsigned char do_lights;
struct Thing *thing_pointed_at;
struct Map *me_pointed_at;
int32_t my_mouse_x;
int32_t my_mouse_y;
char *level_names_data;
char *end_level_names_data;
unsigned char *frontend_backup_palette;
unsigned char zoom_to_heart_palette[768];
unsigned char EngineSpriteDrawUsingAlpha;
unsigned char temp_pal[768];
unsigned char *lightning_palette;


#ifdef __cplusplus
extern "C" {
#endif

TbBool force_player_num = false;

/******************************************************************************/
extern void faststartup_network_game(CoroutineLoop *context);
extern void faststartup_saved_packet_game(void);
extern TngUpdateRet damage_creatures_with_physical_force(struct Thing *thing, ModTngFilterParam param);
extern CoroutineLoopState set_not_has_quit(CoroutineLoop *context);
extern void startup_network_game(CoroutineLoop *context, TbBool local);
void first_gameturn_actions(void);
/******************************************************************************/

TbClockMSec timerstarttime = 0;
long double last_draw_completed_time = 0;
struct TimerTime Timer;
TbBool TimerGame = false;
TbBool TimerNoReset = false;
TbBool TimerFreeze = false;
/******************************************************************************/


TbPixel get_player_path_colour(unsigned short owner)
{
  return player_path_colours[get_player_color_idx(owner % PLAYERS_COUNT)];
}

void setup_stuff(void)
{
    setup_texture_block_mem();
    init_fades_table();
    init_alpha_table();
}

TbBool should_use_delta_time_on_menu()
{
    switch (frontend_menu_state) {
        case FeSt_MAIN_MENU:
        case FeSt_FELOAD_GAME:
        case FeSt_NET_SERVICE: /**< Network service selection, where player can select Serial/Modem/IPX/TCP IP/1 player. */
        case FeSt_NET_SESSION: /**< Network session selection screen, where list of games is displayed, with possibility to join or create own game. */
        case FeSt_NET_START: /**< Network game start screen (the menu with chat), when created new session or joined existing session. */
        case FeSt_HIGH_SCORES:
        case FeSt_FEDEFINE_KEYS:
        case FeSt_FEOPTIONS:
        case FeSt_LEVEL_SELECT:
        case FeSt_CAMPAIGN_SELECT:
        case FeSt_MAPPACK_SELECT:
        case FeSt_LAND_VIEW:
        case FeSt_NETLAND_VIEW:
        case FeSt_TORTURE:
            return true;
        default:
            return false;
    }
}

TbBool all_dungeons_destroyed(const struct PlayerInfo *win_player)
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

void clear_creature_pool(void)
{
    memset(&game.pool,0,sizeof(struct CreaturePool));
    game.pool.is_empty = true;
}

void give_shooter_drained_health(struct Thing *shooter, HitPoints health_delta)
{
    struct CreatureControl *cctrl;
    HitPoints max_health;
    HitPoints health;
    if ( !thing_exists(shooter) )
        return;
    cctrl = creature_control_get_from_thing(shooter);
    max_health = cctrl->max_health;
    health = shooter->health + health_delta;
    if (health < max_health) {
        shooter->health = health;
    } else {
        shooter->health = max_health;
    }
}

long get_foot_creature_has_down(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    unsigned short val;
    long i;
    int n;
    cctrl = creature_control_get_from_thing(thing);
    val = thing->current_frame;
    if (val == (cctrl->anim_time >> 8))
        return 0;
    unsigned short frame = (creature_is_dragging_something(thing)) ? CGI_Drag : CGI_Ambulate;
    n = get_creature_model_graphics(thing->model, frame);
    i = convert_td_iso(n);
    if (i != thing->anim_sprite)
        return 0;
    if (val == 1)
        return 1;
    if (val == 4)
        return 2;
    return 0;
}

void process_keeper_spell_aura(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->spell_aura_duration--;
    if (cctrl->spell_aura_duration <= 0)
    {
        cctrl->spell_aura = 0;
        return;
    }
    struct Coord3d pos;
    long amp;
    long direction;
    long delta_x;
    long delta_y;
    amp = 5 * thing->clipbox_size_xy / 8;
    direction = THING_RANDOM(thing, DEGREES_360);
    delta_x = (amp * LbSinL(direction) >> 8);
    delta_y = (amp * LbCosL(direction) >> 8);
    pos.x.val = thing->mappos.x.val + (delta_x >> 8);
    pos.y.val = thing->mappos.y.val - (delta_y >> 8);
    pos.z.val = thing->mappos.z.val;

    create_used_effect_or_element(&pos, cctrl->spell_aura, thing->owner, thing->index);
}

unsigned long lightning_is_close_to_player(struct PlayerInfo *player, struct Coord3d *pos)
{
    return get_chessboard_distance(&player->acamera->mappos, pos) < subtile_coord(45,0);
}

void affect_nearby_stuff_with_vortex(struct Thing *thing)
{
    //TODO implement vortex; it's not implemented in original DK
    WARNLOG("Not implemented");
}

void affect_nearby_friends_with_alarm(struct Thing *traptng)
{
    SYNCDBG(8,"Starting");
    if (is_neutral_thing(traptng)) {
        return;
    }
    struct Dungeon *dungeon;
    unsigned long k;
    int i;
    dungeon = get_players_num_dungeon(traptng->owner);
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct CreatureControl *cctrl;
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!thing_is_picked_up(thing) && !is_thing_directly_controlled(thing) &&
            !creature_is_being_unconscious(thing) && !creature_is_kept_in_custody(thing) &&
            (cctrl->combat_flags == 0) && !creature_is_dragging_something(thing) && !creature_is_dying(thing) && !creature_is_leaving_and_cannot_be_stopped(thing))
        {
            struct CreatureStateConfig *stati;
            stati = get_thing_state_info_num(get_creature_state_besides_interruptions(thing));
            if (stati->react_to_cta && (get_chessboard_distance(&traptng->mappos, &thing->mappos) < 4096))
            {
                creature_mark_if_woken_up(thing);
                if (external_set_thing_state(thing, CrSt_ArriveAtAlarm))
                {
                    if (setup_person_move_to_position(thing, traptng->mappos.x.stl.num, traptng->mappos.y.stl.num, 0))
                    {
                        thing->continue_state = CrSt_ArriveAtAlarm;
                        cctrl->alarm_over_turn = game.play_gameturn + 800;
                        cctrl->alarm_stl_x = traptng->mappos.x.stl.num;
                        cctrl->alarm_stl_y = traptng->mappos.y.stl.num;
                    }
                }
            }
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
}

long apply_wallhug_force_to_boulder(struct Thing *thing)
{
  unsigned short angle;
  long collide;
  unsigned short new_angle;
  struct Coord3d pos2;
  struct Coord3d pos;
  struct ShotConfigStats *shotst = get_shot_model_stats(thing->model);
  short speed = shotst->speed;
  pos.x.val = move_coord_with_angle_x(thing->mappos.x.val,speed,thing->move_angle_xy);
  pos.y.val = move_coord_with_angle_y(thing->mappos.y.val,speed,thing->move_angle_xy);
  pos.z.val = thing->mappos.z.val;
  if ( (GAME_RANDOM(8) == 0) && (!thing->velocity.z.val ) )
  {
    if ( thing_touching_floor(thing) )
    {
      long top_cube = get_top_cube_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num, NULL);
      if ( ((top_cube & 0xFFFFFFFE) != 0x28) && (top_cube != 39) )
      {
        thing->veloc_push_add.z.val += 48;
        thing->state_flags |= TF1_PushAdd;
      }
    }
  }
  if ( thing_in_wall_at(thing, &pos) )
  {
    long blocked_flags = get_thing_blocked_flags_at(thing, &pos);
    if ( blocked_flags & SlbBloF_WalledX )
    {
      angle = thing->move_angle_xy;
      if ( (angle) && (angle <= ANGLE_SOUTH) )
        collide = process_boulder_collision(thing, &pos, 1, 0);
      else
        collide = process_boulder_collision(thing, &pos, -1, 0);
    }
    else if ( blocked_flags & SlbBloF_WalledY )
    {
      angle = thing->move_angle_xy;
      if ( (angle <= ANGLE_EAST) || (angle > ANGLE_WEST) )
        collide = process_boulder_collision(thing, &pos, 0, -1);
      else
        collide = process_boulder_collision(thing, &pos, 0, 1);
    }
    else
    {
      collide = 0;
    }
    if ( collide != 1 )
    {
      if ( (thing->model != ShM_SolidBoulder) && (collide == 0) )
      {
        thing->health -= game.conf.rules[thing->owner].game.boulder_reduce_health_wall;
      }
      slide_thing_against_wall_at(thing, &pos, blocked_flags);
      if ( blocked_flags & SlbBloF_WalledX )
      {
        angle = thing->move_angle_xy;
        if ( (angle) && ( (angle <= ANGLE_EAST) || (angle > ANGLE_WEST) ) )
        {
          unsigned short y = thing->mappos.y.val;
          pos2.x.val = thing->mappos.x.val;
          pos2.z.val = 0;
          pos2.y.val = y - STL_PER_SLB * speed;
          pos2.z.val = get_thing_height_at(thing, &pos2);
          new_angle = (thing_in_wall_at(thing, &pos2) < 1) ? ANGLE_NORTH : ANGLE_SOUTH;
        }
        else
        {
          pos2.x.val = thing->mappos.x.val;
          pos2.z.val = 0;
          pos2.y.val = thing->mappos.y.val + STL_PER_SLB * speed;
          pos2.z.val = get_thing_height_at(thing, &pos2);
          new_angle = (thing_in_wall_at(thing, &pos2) < 1) ? ANGLE_SOUTH : ANGLE_NORTH;
        }
      }
      else if ( blocked_flags & SlbBloF_WalledY )
      {
        angle = thing->move_angle_xy;
        if ( (angle) && (angle <= ANGLE_SOUTH) )
        {
          pos2.z.val = 0;
          pos2.y.val = thing->mappos.y.val;
          pos2.x.val = thing->mappos.x.val + STL_PER_SLB * speed;
          pos2.z.val = get_thing_height_at(thing, &pos2);
          new_angle = (thing_in_wall_at(thing, &pos2) < 1) ? ANGLE_EAST : ANGLE_WEST;
        }
        else
        {
          unsigned short x = thing->mappos.x.val;
          pos2.z.val = 0;
          pos2.y.val = thing->mappos.y.val;
          pos2.x.val = x - STL_PER_SLB * speed;
          pos2.z.val = get_thing_height_at(thing, &pos2);
          new_angle = (thing_in_wall_at(thing, &pos2) < 1) ? ANGLE_WEST : ANGLE_EAST;
        }
      }
      else
      {
        ERRORLOG("Cannot find boulder wall hug angle!");
        new_angle = 0;
      }
      thing->move_angle_xy = new_angle;
    }
  }
  angle = thing->move_angle_xy;
  thing->velocity.x.val = distance_with_angle_to_coord_x(shotst->speed,angle);
  thing->velocity.y.val = distance_with_angle_to_coord_y(shotst->speed,angle);
  return 0;
}

long process_boulder_collision(struct Thing *boulder, struct Coord3d *pos, int direction_x, int direction_y)
{
    unsigned short boulder_radius = (boulder->clipbox_size_xy >> 1);
    MapSubtlCoord pos_x = (pos->x.val + boulder_radius * direction_x) >> 8;
    MapSubtlCoord pos_y = (pos->y.val + boulder_radius * direction_y) >> 8;
    MapSubtlCoord stl_x = stl_slab_center_subtile(pos_x);
    MapSubtlCoord stl_y = stl_slab_center_subtile(pos_y);

    struct Room *room = subtile_room_get(stl_x, stl_y);
    if (room_exists(room))
    {
        if (room->kind == RoK_GUARDPOST)  // Collide with Guardposts
        {
            if (room->owner != game.neutral_player_num)
            {
                struct Dungeon *dungeon = get_dungeon(room->owner);
                if (!dungeon_invalid(dungeon))
                {
                    dungeon->rooms_destroyed++; // add to player stats
                }
            }
            delete_room_slab(subtile_slab(stl_x), subtile_slab(stl_y), 0); // destroy guardpost
            for (long k = 0; k < AROUND_TILES_COUNT; k++)
            {
                create_dirt_rubble_for_dug_block(stl_x + around[k].delta_x, stl_y + around[k].delta_y, 4, room->owner);
            }
            if (boulder->model != ShM_SolidBoulder) // Solid Boulder (shot20) takes no damage when destroying guardposts
            {
                boulder->health -= game.conf.rules[boulder->owner].game.boulder_reduce_health_room; // decrease boulder health
            }
            return 1; // guardpost destroyed
        }
    }
    else
    {
        if (subtile_has_door_thing_on(stl_x, stl_y)) // Collide with Doors
        {
            struct Thing *doortng = get_door_for_position(stl_x, stl_y);
            HitPoints door_health = doortng->health;
            doortng->health -= boulder->health; // decrease door health
            boulder->health -= door_health; // decrease boulder health
            if (doortng->health <= 0)
            {
                return 2; // door destroyed
            }
        }
    }
    return 0; // Default: No collision OR boulder destroyed on door
}

void draw_flame_breath(struct Coord3d *pos1, struct Coord3d *pos2, long delta_step, long num_per_step, short ef_or_efel_model, ThingIndex parent_idx)
{
  MapCoordDelta dist_x;
  MapCoordDelta dist_y;
  MapCoordDelta dist_z;
  dist_x = pos2->x.val - (MapCoordDelta)pos1->x.val;
  dist_y = pos2->y.val - (MapCoordDelta)pos1->y.val;
  dist_z = pos2->z.val - (MapCoordDelta)pos1->z.val;
  int delta_x;
  int delta_y;
  int delta_z;
  if (delta_step <= 0)
      delta_step = 1;
  if (dist_x >= 0)
  {
      delta_x = delta_step;
    } else {
        dist_x = -dist_x;
        delta_x = -delta_step;
    }
    if (dist_y >= 0) {
        delta_y = delta_step;
    } else {
        dist_y = -dist_y;
        delta_y = -delta_step;
    }
    if (dist_z >= 0) {
        delta_z = delta_step;
    } else {
        dist_z = -dist_z;
        delta_z = -delta_step;
    }
    // Now our dist_x,dist_y,dist_z is always non-negative,
    // and sign is stored in delta_x,delta_y,delta_z.
    if ((dist_x != 0) || (dist_y != 0) || (dist_z != 0))
    {
        int nsteps;
        // Find max dimension, and scale deltas to it
        if ((dist_z > dist_x) && (dist_z > dist_y))
        {
            nsteps = dist_z / delta_step;
            delta_y = dist_y * delta_y / dist_z;
            delta_x = dist_x * delta_x / dist_z;
        } else
        if ((dist_x > dist_y) && (dist_x > dist_z))
        {
            nsteps = dist_x / delta_step;
            delta_y = dist_y * delta_y / dist_x;
            delta_z = dist_z * delta_z / dist_x;
        } else
        if ((dist_y > dist_x) && (dist_y > dist_z))
        {
            nsteps = dist_y / delta_step;
            delta_x = dist_x * delta_x / dist_y;
            delta_z = dist_z * delta_z / dist_y;
        } else
        { // No dominate direction
            nsteps = (dist_x + dist_y + dist_z) / delta_step;
            delta_x = dist_x * delta_x / (dist_x + dist_y + dist_z);
            delta_y = dist_y * delta_y / (dist_x + dist_y + dist_z);
            delta_z = dist_z * delta_z / (dist_x + dist_y + dist_z);
        }

        int sprsize = 0;
        int delta_size = 0;

        struct EffectElementConfigStats *eestat;
        if (ef_or_efel_model < 0)
        {
            eestat = get_effect_element_model_stats(ef_or_efel_model * -1);
            delta_size = ((eestat->sprite_size_max - eestat->sprite_size_min) << 8) / (nsteps+1);
            sprsize = (eestat->sprite_size_min << 8);
        }

        int deviat;
        deviat = 1;
        struct Coord3d curpos;
        curpos.x.val = pos1->x.val;
        curpos.y.val = pos1->y.val;
        curpos.z.val = pos1->z.val;
        int i;
        for (i=nsteps+1; i > 0; i--)
        {
            int devrange;
            devrange = 2 * deviat;
            int k;
            for (k = num_per_step; k > 0; k--)
            {
                struct Coord3d tngpos;
                tngpos.x.val = curpos.x.val + deviat - UNSYNC_RANDOM(devrange);
                tngpos.y.val = curpos.y.val + deviat - UNSYNC_RANDOM(devrange);
                tngpos.z.val = curpos.z.val + deviat - UNSYNC_RANDOM(devrange);
                if ((tngpos.x.val < subtile_coord(game.map_subtiles_x,0)) && (tngpos.y.val < subtile_coord(game.map_subtiles_y,0)))
                {
                    struct Thing *eelemtng;

                    eelemtng = create_used_effect_or_element(&tngpos, ef_or_efel_model, game.neutral_player_num, parent_idx);
                    if (!thing_is_invalid(eelemtng)) {
                        eelemtng->sprite_size = sprsize >> 8;
                    }
                }
            }
            curpos.x.val += delta_x;
            curpos.y.val += delta_y;
            curpos.z.val += delta_z;
            deviat += 16;
            sprsize += delta_size;
        }
    }
}

void draw_lightning(const struct Coord3d *pos1, const struct Coord3d *pos2, long eeinterspace, EffectOrEffElModel ef_or_efel_model)
{
    MapCoordDelta dist_x = pos2->x.val - pos1->x.val;
    MapCoordDelta dist_y = pos2->y.val - pos1->y.val;
    MapCoordDelta dist_z = pos2->z.val - pos1->z.val;
    int delta_x;
    int delta_y;
    int delta_z;
    if (eeinterspace <= 0)
        eeinterspace = 1;
    if (dist_x >= 0) {
        delta_x = eeinterspace;
    } else {
        dist_x = -dist_x;
        delta_x = -eeinterspace;
    }
    if (dist_y >= 0) {
        delta_y = eeinterspace;
    } else {
        dist_y = -dist_y;
        delta_y = -eeinterspace;
    }
    if (dist_z >= 0) {
        delta_z = eeinterspace;
    } else {
        dist_z = -dist_z;
        delta_z = -eeinterspace;
    }
    if ((dist_x != 0) || (dist_y != 0) || (dist_z != 0))
    {
        int nsteps;
        if ((dist_z >= dist_x) && (dist_z >= dist_y))
        {
            nsteps = dist_z / eeinterspace;
            delta_y = delta_y * dist_y / dist_z;
            delta_x = dist_x * delta_x / dist_z;
        } else
        if ((dist_x >= dist_y) && (dist_x >= dist_z))
        {
            nsteps = dist_x / eeinterspace;
            delta_y = delta_y * dist_y / dist_x;
            delta_z = delta_z * dist_z / dist_x;
        } else
        {
            nsteps = dist_y / eeinterspace;
            delta_x = dist_x * delta_x / dist_y;
            delta_z = delta_z * dist_z / dist_y;
        }
        int deviat_x = 0;
        int deviat_y = 0;
        int deviat_z = 0;
        struct Coord3d curpos;
        curpos.x.val = pos1->x.val + UNSYNC_RANDOM(eeinterspace/4);
        curpos.y.val = pos1->y.val + UNSYNC_RANDOM(eeinterspace/4);
        curpos.z.val = pos1->z.val + UNSYNC_RANDOM(eeinterspace/4);
        for (int i=nsteps+1; i > 0; i--)
        {
            struct Coord3d tngpos;
            tngpos.x.val = curpos.x.val + deviat_x;
            tngpos.y.val = curpos.y.val + deviat_y;
            tngpos.z.val = curpos.z.val + deviat_z;
            if ((tngpos.x.val < subtile_coord(game.map_subtiles_x,0)) && (tngpos.y.val < subtile_coord(game.map_subtiles_y,0)))
            {
                create_used_effect_or_element(&tngpos, ef_or_efel_model, game.neutral_player_num, 0);
            }
            if (UNSYNC_RANDOM(6) >= 3) {
                deviat_x -= 32;
            } else {
                deviat_x += 32;
            }
            if (UNSYNC_RANDOM(6) >= 3) {
                deviat_y -= 32;
            } else {
                deviat_y += 32;
            }
            if (UNSYNC_RANDOM(6) >= 3) {
                deviat_z -= 32;
            } else {
                deviat_z += 32;
            }
            MapCoordDelta dist = get_chessboard_3d_distance(&curpos, pos2);
            int deviat_limit = 128;
            if (dist < 1024)
              deviat_limit = (dist * 128) / 1024;
            // Limit deviations
            if (deviat_x < -deviat_limit) {
                deviat_x = -deviat_limit;
            } else
            if (deviat_x > deviat_limit) {
                deviat_x = deviat_limit;
            }
            if (deviat_y < -deviat_limit) {
                deviat_y = -deviat_limit;
            } else
            if (deviat_y > deviat_limit) {
                deviat_y = deviat_limit;
            }
            if (deviat_z < -deviat_limit) {
                deviat_z = -deviat_limit;
            } else
            if (deviat_z > deviat_limit) {
                deviat_z = deviat_limit;
            }
            curpos.x.val += delta_x;
            curpos.y.val += delta_y;
            curpos.z.val += delta_z;
        }
    }
}

TbBool any_player_close_enough_to_see(const struct Coord3d *pos)
{
    struct PlayerInfo *player;
    int i;
    short limit = 24 * COORD_PER_STL;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        player = get_player(i);
        if ( (player_exists(player)) && ((player->allocflags & PlaF_CompCtrl) == 0))
        {
            if (player->acamera == NULL)
                continue;
            if (player->acamera->view_mode != PVM_FrontView)
            {
                if (player->acamera->zoom >= CAMERA_ZOOM_MIN)
                {
                    limit = SHRT_MAX - (2 * player->acamera->zoom);
                }
            }
            else
            {
                if (player->acamera->zoom >= FRONTVIEW_CAMERA_ZOOM_MIN)
                {
                    limit = SHRT_MAX - (player->acamera->zoom / 3);
                }
            }
            if (get_chessboard_distance(&player->acamera->mappos, pos) <= limit)
            {
                return true;
            }
        }
    }
    return false;
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
        cctrl->anim_time = thing->anim_time;
    }
    if ((thing->anim_speed != 0) && (thing->max_frames != 0))
    {
        thing->anim_time += thing->anim_speed;
        i = (thing->max_frames << 8);
        if (i <= 0) i = 256;
        while (thing->anim_time  < 0)
        {
          thing->anim_time += i;
        }
        if (thing->anim_time > i-1)
        {
          if (thing->rendering_flags & TRF_AnimateOnce)
          {
            thing->anim_speed = 0;
            thing->anim_time = i-1;
          } else
          {
            thing->anim_time %= i;
          }
        }
        thing->current_frame = (thing->anim_time >> 8) & 0xFF;
    }
    if (thing->transformation_speed != 0)
    {
      thing->sprite_size += thing->transformation_speed;
      if (thing->sprite_size > thing->sprite_size_min)
      {
        if (thing->sprite_size >= thing->sprite_size_max)
        {
          thing->sprite_size = thing->sprite_size_max;
          if ((thing->size_change & TSC_ChangeSizeContinuously) != 0)
            thing->transformation_speed = -thing->transformation_speed;
          else
            thing->transformation_speed = 0;
        }
      } else
      {
        thing->sprite_size = thing->sprite_size_min;
        if ((thing->size_change & TSC_ChangeSizeContinuously) != 0)
          thing->transformation_speed = -thing->transformation_speed;
        else
          thing->transformation_speed = 0;
      }
    }
}

void init_censorship(void)
{
  if ( censorship_enabled() )
  {
    // Modification for Dark Mistress
      set_creature_model_graphics(20, 14, 48);
  }
}

void engine_init(void)
{
    fill_floor_heights_table();
    generate_wibble_table();
    load_ceiling_table();
}

void init_keeper(void)
{
    SYNCDBG(8,"Starting");
    engine_init();
    init_iso_3d_conversion_tables();
    init_colours();
    init_spiral_steps();
    init_key_to_strings();
    // Load configs which may have per-campaign part, and even be modified within a level
    recheck_all_mod_exist();
    init_custom_sprites(SPRITE_LAST_LEVEL);
    load_stats_files();
    check_and_auto_fix_stats();
    init_creature_scores();
    init_top_texture_to_cube_table();
    game.neutral_player_num = PLAYER_NEUTRAL;
    poly_pool_end = &poly_pool[sizeof(poly_pool)-128];
    lbDisplay.GlassMap = pixmap.ghost;
    lbDisplay.DrawColour = colours[15][15][15];
    game.comp_player_aggressive  = (comp_player_conf.player_assist_default == comp_player_conf.computer_assist_types[0]);
    game.comp_player_defensive   = (comp_player_conf.player_assist_default == comp_player_conf.computer_assist_types[1]);
    game.comp_player_construct   = (comp_player_conf.player_assist_default == comp_player_conf.computer_assist_types[2]);
    game.comp_player_creatrsonly = (comp_player_conf.player_assist_default == comp_player_conf.computer_assist_types[3]);
    game.creatures_tend_imprison = 0;
    game.creatures_tend_flee = 0;
    game.operation_flags |= GOF_ShowPanel;
    game.view_mode_flags |= (GNFldD_StatusPanelDisplay | GNFldD_RoomFlameProcessing);
    init_censorship();
    SYNCDBG(9,"Finished");
}

/**
 * Initial video setup - loads only most important files to show startup screens.
 */
TbBool initial_setup(void)
{
    SYNCDBG(6,"Starting");
    // setting this will force video mode change, even if previous one is same
    MinimalResolutionSetup = true;
    // Set size of static textures buffer
    game_load_files[1].SLength = max((ulong)TEXTURE_BLOCKS_STAT_COUNT_A*block_dimension*block_dimension,(ulong)LANDVIEW_MAP_WIDTH*LANDVIEW_MAP_HEIGHT);
    if (LbDataLoadAllV2(game_load_files))
    {
        ERRORLOG("Unable to load game_load_files");
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
  short result;
  // Do only a very basic setup
  cpu_detect(&cpu_info);
  SYNCMSG("CPU %s type %d family %d model %d stepping %d features %08lx",cpu_info.vendor,
      (int)cpu_get_type(&cpu_info),(int)cpu_get_family(&cpu_info),(int)cpu_get_model(&cpu_info),
      (int)cpu_get_stepping(&cpu_info),cpu_info.feature_edx);
  if (cpu_info.BrandString)
  {
      SYNCMSG("%s", &cpu_info.brand[0]);
  }
  SYNCMSG("Build image base: %p", get_image_base());
  SYNCMSG("Operating System: %s", get_os_version());

  const auto wine_version = get_wine_version();
  if (wine_version) {
        SYNCMSG("Running on Wine v%s", wine_version);
        is_running_under_wine = true;
        const auto wine_host = get_wine_host();
        SYNCMSG("Wine Host: %s", wine_host);
  }

  // Enable features that require more than 32 megs of memory
  features_enabled |= Ft_HiResCreatr;
  // Enable features that require more than 16 megs of memory
  features_enabled |= Ft_EyeLens;
  features_enabled |= Ft_HiResVideo;
  features_enabled |= Ft_BigPointer;
  features_enabled |= Ft_AdvAmbSound;

  // Default feature settings (in case the options are absent from keeperfx.cfg)
  features_enabled &= ~Ft_FreezeOnLoseFocus; // don't freeze the game, if the game window loses focus
  features_enabled &= ~Ft_UnlockCursorOnPause; // don't unlock the mouse cursor from the window, if the user pauses the game
  features_enabled |= Ft_LockCursorInPossession; // lock the mouse cursor to the window, when the user enters possession mode (when the cursor is already unlocked)
  features_enabled &= ~Ft_PauseMusicOnGamePause; // don't pause the music, if the user pauses the game
  features_enabled &= ~Ft_MuteAudioOnLoseFocus; // don't mute the audio, if the game window loses focus
  features_enabled &= ~Ft_SkipHeartZoom; // don't skip the dungeon heart zoom in
  features_enabled &= ~Ft_DisableCursorCameraPanning; // don't disable cursor camera panning
  features_enabled |= Ft_DeltaTime; // enable delta time
  features_enabled |= Ft_NoCdMusic; // use music files (OGG) rather than CD music

  // Configuration file
  if ( !load_configuration() )
  {
      ERRORLOG("Configuration load error.");
      return 0;
  }

  #ifdef FUNCTESTING
    start_params.startup_flags &= ~SFlg_Legal;
    start_params.startup_flags &= ~SFlg_FX;
    features_enabled |= Ft_SkipHeartZoom;
  #endif

  // Process CmdLine overrides
  process_cmdline_overrides();

  LbIKeyboardOpen();

  if (LbDataLoadAll(legal_load_files) != 0)
  {
      ERRORLOG("Error on allocation/loading of legal_load_files.");
      return 0;
  }

  // Setup polyscans
  setup_bflib_render();

  // View the legal screen
  if (!setup_screen_mode_zero(get_frontend_vidmode()))
  {
      ERRORLOG("Unable to set display mode for legal screen");
      return 0;
  }

  if (flag_is_set(start_params.startup_flags, SFlg_Legal))
  {
      if (is_ar_wider_than_original(LbGraphicsScreenWidth(), LbGraphicsScreenHeight()))
      {
        result = init_actv_bitmap_screen(RBmp_SplashLegalWide);
      } else {
        result = init_actv_bitmap_screen(RBmp_SplashLegal);
      }
       if ( result )
      {
          result = show_actv_bitmap_screen(3000);
          free_actv_bitmap_screen();
      } else
          SYNCLOG("Legal image skipped");
  }
  else
  {
      // Make the white screen into a black screen faster
      draw_clear_screen();
  }

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
  if (flag_is_set(start_params.startup_flags, SFlg_FX))
  {
      result = init_actv_bitmap_screen(RBmp_SplashFx);
      if ( result == 1 )
      {
          result = show_actv_bitmap_screen(4000);
          free_actv_bitmap_screen();
      } else
          SYNCLOG("startup_fx image skipped");
  }

  draw_clear_screen();
  // View Bullfrog company logo animation when new moon
  if ( ( is_new_moon ) || (flag_is_set(start_params.startup_flags, SFlg_Bullfrog)) )
    if (!start_params.no_intro)
    {
        result = moon_video();
        if ( !result ) {
            ERRORLOG("Unable to play new moon movie");
        }
    }

  result = 1;
  // Setup the intro video mode
  if (result && (!start_params.no_intro) )
  {
      if (!setup_screen_mode_zero(get_movies_vidmode()))
      {
        ERRORLOG("Can't enter movies screen mode to play intro");
        result=0;
      }
  }

  if (result == 1)
  {
      draw_clear_screen();
      if (wait_for_installation_files())
      {
          //result = -1; // Helps with better warning message later
      }
      if (!start_params.no_intro)
      {
         if (flag_is_set(start_params.startup_flags, SFlg_EA))
         {
             ea_video();
         }
         if (flag_is_set(start_params.startup_flags, SFlg_Intro))
         {
            result = intro_replay();
         }
      }
  }

  game.frame_skip = start_params.frame_skip;
  redetect_screen_refresh_rate_for_draw();

  // Intro problems shouldn't force the game to quit,
  // so we're re-setting the result flag
  if (result == 0)
      result = 1;

  if (result == 1)
  {
      display_loading_screen();
  }
  LbDataFreeAll(legal_load_files);

  if (result == 1)
  {
      if ( !initial_setup() )
        result = 0;
  }

  if (result == 1)
  {
    load_settings();
    if ( !setup_gui_strings_data() )
      result = 0;
  }

  if (result == 1)
  {
      init_keeper();
      set_gamma(settings.gamma_correction, 0);
      set_music_volume(settings.music_volume);
      SetSoundMasterVolume(settings.sound_volume);
      setup_mesh_randomizers();
      setup_stuff();
      init_lookups();
  }

  if (result == 1)
  {
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

/** Returns if cursor for given player is at top of the dungeon in 3D view.
 *  Cursor placed at top of dungeon is marked by green/red "volume box";
 *   if there's no volume box, cursor should be of the field behind it
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
    if ( (i == PSt_BuildRoom) || (i == PSt_PlaceDoor) || (i == PSt_PlaceTrap) || (i == PSt_SightOfEvil) || (i == PSt_Sell) || (i == PSt_PlaceTerrain) || (i == PSt_MkDigger)
        || (i == PSt_MkGoodCreatr) || (i == PSt_MkBadCreatr) )
        return true;
    if ( (i == PSt_OrderCreatr) && (player->controlled_thing_idx > 0) )
        return true;
    if ( (i == PSt_CtrlDungeon) && (player->primary_cursor_state != CSt_DefaultArrow) && (player->thing_under_hand == 0) )
        return true;
    return false;
}

TbBool engine_point_to_map(struct Camera *camera, long screen_x, long screen_y, int32_t *map_x, int32_t *map_y)
{
    struct PlayerInfo *player = get_my_player();
    *map_x = 0;
    *map_y = 0;
    if ( (pointer_x >= 0) && (pointer_y >= 0)
      && (pointer_x < (player->engine_window_width/pixel_size))
      && (pointer_y < (player->engine_window_height/pixel_size)) )
    {
        if ( players_cursor_is_at_top_of_view(player) )
        {
              *map_x = subtile_coord(top_pointed_at_x,top_pointed_at_frac_x);
              *map_y = subtile_coord(top_pointed_at_y,top_pointed_at_frac_y);
        } else
        {
              *map_x = subtile_coord(block_pointed_at_x,pointed_at_frac_x);
              *map_y = subtile_coord(block_pointed_at_y,pointed_at_frac_y);
        }
        // Clipping coordinates
        if (*map_y < 0)
          *map_y = 0;
        else
        if (*map_y > subtile_coord(game.map_subtiles_y,-1))
          *map_y = subtile_coord(game.map_subtiles_y,-1);
        if (*map_x < 0)
          *map_x = 0;
        else
        if (*map_x > subtile_coord(game.map_subtiles_x,-1))
          *map_x = subtile_coord(game.map_subtiles_x,-1);
        return true;
    }
    return false;
}

TbBool screen_to_map(struct Camera *camera, int32_t screen_x, int32_t screen_y, struct Coord3d *mappos)
{
    TbBool result;
    int32_t x;
    int32_t y;
    SYNCDBG(19,"Starting");
    result = false;
    if (camera != NULL)
    {
      switch (camera->view_mode)
      {
        case PVM_CreatureView:
        case PVM_IsoWibbleView:
        case PVM_FrontView:
        case PVM_IsoStraightView:
          // 3D view mode
          result = engine_point_to_map(camera,screen_x,screen_y,&x,&y);
          break;
        case PVM_ParchmentView: //map mode
          result = point_to_overhead_map(camera,screen_x/pixel_size,screen_y/pixel_size,&x,&y);
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
    if ( mappos->x.val > ((game.map_subtiles_x<<8)-1) )
      mappos->x.val = ((game.map_subtiles_x<<8)-1);
    if ( mappos->y.val > ((game.map_subtiles_y<<8)-1) )
      mappos->y.val = ((game.map_subtiles_y<<8)-1);
    SYNCDBG(19,"Finished");
    return result;
}

void update_creatr_model_activities_list(TbBool forced)
{
    struct Dungeon *dungeon = get_my_dungeon();
    ThingModel crmodel;
    int num_breeds = no_of_breeds_owned;
    TbBool changed = false;

    // Add to breed activities
    for (crmodel = 1; crmodel < game.conf.crtr_conf.model_count; crmodel++)
    {
        if ((dungeon->owned_creatures_of_model[crmodel] > 0)
            && (crmodel != get_players_spectator_model(my_player_number)))
        {
            TbBool found = false;
            for (int i = 0; i < num_breeds; i++)
            {
                if (breed_activities[i] == crmodel)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                changed = true;
                breed_activities[num_breeds] = crmodel;
                num_breeds++;
            }
        }
    }

    // Remove from breed activities
    for (crmodel = 1; crmodel < game.conf.crtr_conf.model_count; crmodel++)
    {
        if ((dungeon->owned_creatures_of_model[crmodel] <= 0)
          && (crmodel != get_players_special_digger_model(my_player_number)))
        {
            for (int i = 0; i < num_breeds; i++)
            {
                if (breed_activities[i] == crmodel)
                {
                    for (; i < num_breeds-1;  i++) {
                        breed_activities[i] = breed_activities[i+1];
                    }
                    changed = true;
                    num_breeds--;
                    breed_activities[i] = 0;
                    break;
                }
            }
        }
        no_of_breeds_owned = num_breeds;
    }

    // Reorder breed activities to ensure diggers are correctly positioned
    if (changed || forced)
    {
        struct CreatureModelConfig* crconf;
        ThingModel temp;
        int write_idx = 1;
        for (int i = 1; i < num_breeds; i++)
        {
            crconf = &game.conf.crtr_conf.model[breed_activities[i]];
            if (any_flag_is_set(crconf->model_flags, (CMF_IsDiggingCreature | CMF_IsSpecDigger)))
            {
                temp = breed_activities[i];
                memmove(&breed_activities[write_idx + 1], &breed_activities[write_idx], (i - write_idx) * sizeof(ThingModel));
                breed_activities[write_idx] = temp;
                write_idx++;
            }
        }
    }
}

void toggle_hero_health_flowers(void)
{
    const char *statstr;
    toggle_flag(game.mode_flags, MFlg_NoHeroHealthFlower);
    if (game.mode_flags & MFlg_NoHeroHealthFlower)
    {
      statstr = "off";
    } else
    {
      do_sound_menu_click();
      statstr = "on";
    }
    show_onscreen_msg(2*game_num_fps, "Hero health flowers %s", statstr);
}

void reset_gui_based_on_player_mode(void)
{
    struct PlayerInfo *player = get_my_player();
    if (player->view_type == PVT_CreatureContrl)
    {
        turn_on_menu(vid_change_query_menu);
        if (player->victory_state == VicS_LostLevel)
        {
            turn_off_query_menus();
        }
    }
    else if (player->view_type == PVT_CreaturePasngr)
    {
        turn_on_menu(vid_change_query_menu);
        turn_off_query_menus();
    }
    else
    {
        turn_on_menu(GMnu_MAIN);
        if (game.active_panel_mnu_idx > 0)
        {
            initialise_tab_tags(game.active_panel_mnu_idx);
            if ( (player->work_state == PSt_CreatrInfo) || (player->work_state == PSt_CreatrInfoAll) )
            {
                turn_on_menu(vid_change_query_menu);
            }
            else
            {
                turn_on_menu(game.active_panel_mnu_idx);
            }
            MenuNumber mnuidx = menu_id_to_number(GMnu_MAIN);
            if (mnuidx != MENU_INVALID_ID) {
                setup_radio_buttons(&active_menus[mnuidx]);
            }
        }
        else
        {
            turn_on_menu(GMnu_ROOM);
        }
    }
    set_gui_visible(true);
}

void reinit_tagged_blocks_for_player(PlayerNumber plyr_idx)
{
    // Clear tagged blocks
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    for (stl_y=0; stl_y < game.map_subtiles_y; stl_y++)
    {
        for (stl_x=0; stl_x < game.map_subtiles_x; stl_x++)
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x, stl_y);
            mapblk->flags &= ~SlbAtFlg_Unexplored;
            mapblk->flags &= ~SlbAtFlg_TaggedValuable;
        }
    }
    // Reinit with data from current players dungeon
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    int task_idx;
    for (task_idx = 0; task_idx < dungeon->highest_task_number; task_idx++)
    {
        struct MapTask  *mtask;
        mtask = &dungeon->task_list[task_idx];
        MapSubtlCoord taskstl_x;
        MapSubtlCoord taskstl_y;
        taskstl_x = stl_num_decode_x(mtask->coords);
        taskstl_y = stl_num_decode_y(mtask->coords);
        switch (mtask->kind)
        {
        case 2:
            for (stl_y = taskstl_y - 1; stl_y <= taskstl_y + 1; stl_y++)
            {
                for (stl_x = taskstl_x - 1; stl_x <= taskstl_x + 1; stl_x++)
                {
                    struct Map *mapblk;
                    mapblk = get_map_block_at(stl_x, stl_y);
                    mapblk->flags |= SlbAtFlg_TaggedValuable;
                }
            }
            break;
        case 1:
        case 3:
            for (stl_y = taskstl_y - 1; stl_y <= taskstl_y + 1; stl_y++)
            {
                for (stl_x = taskstl_x - 1; stl_x <= taskstl_x + 1; stl_x++)
                {
                    struct Map *mapblk;
                    mapblk = get_map_block_at(stl_x, stl_y);
                    mapblk->flags |= SlbAtFlg_Unexplored;
                }
            }
            break;
        default:
            break;
        }
    }
}

void instant_instance_selected(CrInstance check_inst_id)
{
    struct PlayerInfo *player;
    player = get_player(my_player_number);
    struct Thing *ctrltng;
    ctrltng = thing_get(player->controlled_thing_idx);
    struct CreatureModelConfig *crconf;
    crconf = creature_stats_get_from_thing(ctrltng);
    long i;
    long k;
    int avail_pos;
    int match_avail_pos;
    avail_pos = 0;
    match_avail_pos = 0;
    for (i=0; i < CREATURE_MAX_LEVEL; i++)
    {
        k = crconf->learned_instance_id[i];
        if (creature_instance_is_available(ctrltng, k))
        {
            if (k == check_inst_id) {
                match_avail_pos = avail_pos;
                break;
            }
            avail_pos++;
        }
    }
    first_person_instance_top_half_selected = match_avail_pos < 6 && (first_person_instance_top_half_selected || match_avail_pos < 4);
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
    if (!thing_exists(thing))
    {
      return false;
    }
    set_players_packet_action(player, PckA_ZoomToPosition, thing->mappos.x.val, thing->mappos.y.val, 0, 0);
    return true;
}

TbBool toggle_computer_player(PlayerNumber plyr_idx)
{
    struct PlayerInfo *player = get_player(plyr_idx);
    struct Dungeon *dungeon = get_players_dungeon(player);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    if ((dungeon->computer_enabled & 0x01) == 0)
    {
        dungeon->computer_enabled |= 0x01;
    } else
    {
        dungeon->computer_enabled &= ~0x01;
    }
    struct Computer2 *comp;
    comp = get_computer_player(player->id_number);
    computer_force_dump_held_things_on_map(comp, &dungeon->essential_pos);
    return true;
}

void reinit_level_after_load(void)
{
    struct PlayerInfo *player;
    int i;
    SYNCDBG(6,"Starting");
    // Reinit structures from within the game
    player = get_my_player();
    player->lens_palette = 0;
    player->main_palette = engine_palette;
    init_lookups();
    init_navigation();
    reinit_packets_after_load();
    game.easter_eggs_enabled = start_params.easter_egg;
    parchment_loaded = 0;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        player = get_player(i);
        if (player_exists(player))
        {
            set_engine_view(player, player->view_mode);
            update_panel_color_player_color(player->id_number, get_dungeon(i)->color_idx);
        }
    }
    start_rooms = &game.rooms[1];
    end_rooms = &game.rooms[ROOMS_COUNT];
    update_room_tab_to_config();
    update_powers_tab_to_config();
    update_trap_tab_to_config();
    load_texture_map_file(game.texture_id, get_loaded_level_number(), get_level_fgroup(get_loaded_level_number()));
    init_animating_texture_maps();
    init_gui();
    reset_gui_based_on_player_mode();
    erstats_clear();
    player = get_my_player();
    reinit_tagged_blocks_for_player(player->id_number);
    restore_computer_player_after_load();
    sound_reinit_after_load();
    update_panel_colors();
    reset_postal_instance_cache();
}

/**
 * Sets to defaults some basic parameters which are
 * later copied into Game structure.
 */
TbBool set_default_startup_parameters(void)
{
    memset(&start_params, 0, sizeof(struct StartupParameters));
    start_params.startup_flags = (SFlg_Legal|SFlg_FX|SFlg_Intro);
    start_params.packet_checksum_verify = 1;
    // Set levels to 0, as we may not have the campaign loaded yet
    start_params.selected_level_number = 0;
    start_params.num_fps = 20;
    start_params.one_player = 1;
    start_params.computer_chat_flags = CChat_None;
    clear_flag(start_params.mode_flags, MFlg_IsDemoMode);
    set_flag(start_params.mode_flags, MFlg_DemoMode);
    return true;
}

void clear_map(void)
{
    clear_mapmap();
    clear_slabs();
    clear_columns();
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
        thing->mappos.x.val = subtile_coord_center(game.map_subtiles_x/2);
        thing->mappos.y.val = subtile_coord_center(game.map_subtiles_y/2);

        // Create the list of free indices (skip index 0 since that's INVALID_THING
        if (i > 0) {
            if (i < SYNCED_THINGS_COUNT) {
                game.synced_free_things[SYNCED_THINGS_COUNT-1-i] = i;
            } else if (i < THINGS_COUNT) {
                game.unsynced_free_things[THINGS_COUNT-1-i] = i;
            }
        }
    }
    game.synced_free_things_count = SYNCED_THINGS_COUNT-1; // 1 to 8191. Note: COUNT macros aren't real representations of how many things there should be, all of them are off by 1.
    game.unsynced_free_things_count = UNSYNCED_THINGS_COUNT-1; // 8192 to 12287

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

void init_keepers_map_exploration(void)
{
    struct PlayerInfo *player;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if ((player_exists(player) && (player->is_active == 1)) || player_is_roaming(i))
      {
          // Additional init - the main one is in init_player()
          if ((player->allocflags & PlaF_CompCtrl) != 0) {
              init_keeper_map_exploration_by_terrain(player);
              init_keeper_map_exploration_by_creatures(player);
          }
      }
    }
}

void clear_players_for_save(void)
{
    struct PlayerInfo *player;
    unsigned short saved_player_id;
    unsigned short saved_is_active;
    unsigned short saved_allocation_flags;
    struct Camera cammem;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      saved_player_id = player->id_number;
      saved_is_active = player->is_active;
      saved_allocation_flags = player->allocflags;
      memcpy(&cammem,&player->cameras[CamIV_FirstPerson],sizeof(struct Camera));
      memset(player, 0, sizeof(struct PlayerInfo));
      player->id_number = saved_player_id;
      player->is_active = saved_is_active;
      set_flag_value(player->allocflags, PlaF_Allocated, ((saved_allocation_flags & PlaF_Allocated) != 0));
      set_flag_value(player->allocflags, PlaF_CompCtrl, ((saved_allocation_flags & PlaF_CompCtrl) != 0));
      memcpy(&player->cameras[CamIV_FirstPerson],&cammem,sizeof(struct Camera));
      player->acamera = &player->cameras[CamIV_FirstPerson];
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
        if (i < SYNCED_THINGS_COUNT) {
            game.synced_free_things[SYNCED_THINGS_COUNT-1-i] = i;
        } else if (i < THINGS_COUNT) {
            game.unsynced_free_things[THINGS_COUNT-1-i] = i;
        }
    }
    game.synced_free_things_count = SYNCED_THINGS_COUNT-1;
    game.unsynced_free_things_count = UNSYNCED_THINGS_COUNT-1;
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

/**
 * Clears game structures at end of level.
 * Also used as part of clearing before new level is loaded.
 */
void clear_game_for_summary(void)
{
    SYNCDBG(6,"Starting");
    delete_all_structures();
    clear_shadow_limits(&game.lish);
    clear_stat_light_map();
    clear_mapwho();
    game.entrance_room_id = 0;
    game.action_random_seed = 0;
    game.ai_random_seed = 0;
    game.player_random_seed = 0;
    game.operation_flags &= ~GOF_Paused;
    clear_columns();
    clear_action_points();
    clear_players();
    clear_dungeons();
}

void clear_game(void)
{
    SYNCDBG(6,"Starting");
    clear_game_for_summary();
    game.music_track = 0;
    clear_map();
    clear_computer();
    clear_script();
    clear_events();
    clear_things_and_persons_data();
    ceiling_set_info(12, 4, 1);
    init_animating_texture_maps();
    clear_slabsets();
    game.skip_initial_input_turns = 0;
    clear_input_lag_queue();
}

void clear_game_for_save(void)
{
    SYNCDBG(6,"Starting");
    delete_all_structures();
    light_initialise();
    clear_mapwho();
    game.entrance_room_id = 0;
    game.action_random_seed = 0;
    game.ai_random_seed = 0;
    game.player_random_seed = 0;
    clear_columns();
    clear_players_for_save();
    clear_dungeons();
}

void reset_creature_max_levels(void)
{
    int i;
    int k;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
        struct Dungeon *dungeon;
        dungeon = get_dungeon(i);
        for (k=1; k < game.conf.crtr_conf.model_count; k++)
        {
            dungeon->creature_max_level[k] = CREATURE_MAX_LEVEL+1;
        }
    }
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
    if (pal == blue_palette) // if the requested palette is the Freeze palette
    {
      if ((player->additional_flags & PlaAF_FreezePaletteIsActive) != 0)
        return; // Freeze palette is already on
      player->additional_flags |= PlaAF_FreezePaletteIsActive; // flag Freeze palette is active
    } else
    {
      player->additional_flags &= ~PlaAF_FreezePaletteIsActive; // flag Freeze palette is not active
    }
    if ( (player->lens_palette == 0) || ((pal != player->main_palette) && (pal == player->lens_palette)) )
    {
        player->main_palette = pal;
        player->palette_fade_step_pain = 0;
        player->palette_fade_step_possession = 0;
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
      result = (LbFileLoadAt(fname, engine_palette) != -1);
    }
    if ((result) && (do_set))
    {
      struct PlayerInfo *myplyr;
      myplyr=get_my_player();
      PaletteSetPlayerPalette(myplyr, engine_palette);
    }
    if (!result)
      ERRORLOG("Can't load palette file.");
    return result;
}

void centre_engine_window(void)
{
    long window_center_x;
    long window_center_y;
    struct PlayerInfo *player=get_my_player();
    if ((game.operation_flags & GOF_ShowGui) != 0)
      window_center_x = (MyScreenWidth-player->engine_window_width-status_panel_width) / 2 + status_panel_width;
    else
      window_center_x = (MyScreenWidth-player->engine_window_width) / 2;
    window_center_y = (MyScreenHeight-player->engine_window_height) / 2;
    setup_engine_window(window_center_x, window_center_y, player->engine_window_width, player->engine_window_height);
}

void turn_off_query(PlayerNumber plyr_idx)
{
    struct PlayerInfo *player;
    player = get_player(plyr_idx);
    set_player_instance(player, PI_UnqueryCrtr, 0);
}

void level_lost_go_first_person(PlayerNumber plyr_idx)
{
    struct CreatureControl *cctrl;
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Thing *thing;
    ThingModel spectator_breed;
    SYNCDBG(6,"Starting for player %d",(int)plyr_idx);
    player = get_player(plyr_idx);
    dungeon = get_dungeon(player->id_number);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Unable to get player %d dungeon",(int)plyr_idx);
        return;
    }
    spectator_breed = get_players_spectator_model(plyr_idx);
    player->dungeon_camera_zoom = get_camera_zoom(player->acamera);
    thing = create_and_control_creature_as_controller(player, spectator_breed, &dungeon->mappos);
    if (thing_is_invalid(thing)) {
        ERRORLOG("Unable to create spectator creature");
        return;
    }
    cctrl = creature_control_get_from_thing(thing);
    cctrl->creature_control_flags |= CCFlg_NoCompControl;
    SYNCDBG(8,"Finished");
}

void set_general_information(long msg_id, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y)
{
    struct PlayerInfo *player;
    long pos_x;
    long pos_y;
    player = get_my_player();
    find_map_location_coords(target, &x, &y, my_player_number, __func__);
    pos_x = 0;
    pos_y = 0;
    if ((x != 0) || (y != 0))
    {
        pos_y = subtile_coord_center(y);
        pos_x = subtile_coord_center(x);
    }
    event_create_event(pos_x, pos_y, EvKind_Information, player->id_number, -msg_id);
}

void set_quick_information(long msg_id, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y)
{
    struct PlayerInfo *player;
    long pos_x;
    long pos_y;
    player = get_my_player();
    find_map_location_coords(target, &x, &y, my_player_number, __func__);
    pos_x = 0;
    pos_y = 0;
    if ((x != 0) || (y != 0))
    {
        pos_y = subtile_coord_center(y);
        pos_x = subtile_coord_center(x);
    }
    event_create_event(pos_x, pos_y, EvKind_QuickInformation, player->id_number, -msg_id);
}

void set_general_objective(long msg_id, TbMapLocation target, long x, long y)
{
    process_objective(get_string(msg_id), target, x, y);
}

void process_objective(const char *msg_text, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y)
{
    struct PlayerInfo *player;
    MapSubtlCoord pos_x;
    MapSubtlCoord pos_y;
    player = get_my_player();
    find_map_location_coords(target, &x, &y, my_player_number, __func__);
    pos_y = y;
    pos_x = x;
    set_level_objective(msg_text);
    display_objectives(player->id_number, pos_x, pos_y);
}

short winning_player_quitting(struct PlayerInfo *player, int32_t *plyr_count)
{
    struct PlayerInfo *swplyr;
    int i;
    int k;
    int n;
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
        if (swplyr->is_active == 1)
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

short complete_level(struct PlayerInfo *player)
{
    SYNCDBG(6,"Starting");
    if (!is_my_player(player))
        return false;
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
        LbNetwork_Stop();
        quit_game = 1;
        return true;
    }
    LevelNumber lvnum;
    lvnum = get_continue_level_number();
    if (get_loaded_level_number() == lvnum)
    {
        SYNCDBG(7,"Progressing the campaign");
        move_campaign_to_next_level();
    }
    quit_game = 1;
    return true;
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

void interp_fix_mouse_light_off_map(struct PlayerInfo *player)
{
    // This fixes the interpolation issue of moving the mouse off map in one position then back onto the map far elsewhere.
    struct Light* light = &game.lish.lights[player->cursor_light_idx];

    if (player->mouse_on_map == false) {
        light->disable_interp_for_turns = 2;
    }
    if (light->disable_interp_for_turns > 0) {
        light->disable_interp_for_turns -= 1;
        light->last_turn_drawn = 0;
    }
}

void set_mouse_light(struct PlayerInfo *player)
{
    SYNCDBG(6,"Starting");
    struct Packet *pckt;
    if (is_my_player(player)) {
        pckt = get_local_input_lag_packet_for_turn(game.play_gameturn);
    } else {
        pckt = get_packet_direct(player->packet_num);
    }
    if (player->cursor_light_idx != 0)
    {
        if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
        {
            struct Coord3d pos;
            pos.x.val = pckt->pos_x;
            pos.y.val = pckt->pos_y;
            pos.z.val = get_floor_height_at(&pos);
            if (is_my_player(player)) {
                game.mouse_light_pos = pos;
            }
            light_turn_light_on(player->cursor_light_idx);
            light_set_light_position(player->cursor_light_idx, &pos);
        }
        else
        {
            light_turn_light_off(player->cursor_light_idx);
        }
        interp_fix_mouse_light_off_map(player);
    }
}

void check_players_won(void)
{
  SYNCDBG(8,"Starting");

    if (!flag_is_set(game.system_flags,GSF_NetworkActive))
        return;

    struct PlayerInfo* curPlayer;
    for (PlayerNumber playerIdx = 0; playerIdx < PLAYERS_COUNT; ++playerIdx)
    {
        curPlayer = get_player(playerIdx);
        if (!player_exists(curPlayer) || (curPlayer->is_active != 1) || (curPlayer->victory_state != VicS_Undecided))
            continue;

        // check if any other player is still alive
        TbBool LivingOpponent = false;
        for (PlayerNumber secondPlayerIdx = 0; secondPlayerIdx < PLAYERS_COUNT; ++secondPlayerIdx)
        {
            if (secondPlayerIdx == playerIdx)
                continue;

            struct PlayerInfo* otherPlayer = get_player(secondPlayerIdx);
            if (player_exists(otherPlayer) && otherPlayer->victory_state == VicS_Undecided)
            {
                struct Thing* heartng = get_player_soul_container(secondPlayerIdx);
                if (heartng->active_state != ObSt_BeingDestroyed)
                {
                    LivingOpponent = true;
                    break;
                }
            }
        }
        if (LivingOpponent == false)
        {
            set_player_as_won_level(curPlayer);
            return;
        }
    }
}

void check_players_lost(void)
{
  long i;
  SYNCDBG(8,"Starting");
  struct PlayerInfo* player;
  struct Dungeon* dungeon;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
      player = get_player(i);
      dungeon = get_players_dungeon(player);
      if (player_exists(player) && (player->is_active == 1))
      {
          struct Thing *heartng;
          heartng = get_player_soul_container(i);
          if (heartng->owner != i)
          {
              init_player_start(player, true);
              if (dungeon->dnheart_idx == 0)
              {
                  initialise_devastate_dungeon_from_heart(player->id_number);
              }
          }
          if ((!thing_exists(heartng) || ((heartng->active_state == ObSt_BeingDestroyed) && !(dungeon->backup_heart_idx > 0))) && (player->victory_state == VicS_Undecided))
          {
            event_kill_all_players_events(i);
            set_player_as_lost_level(player);
            //this would easily prevent computer player activities on dead player, but it also makes dead player unable to use
            //floating spirit, so it can't be done this way: player->is_active = 0;
            if (is_my_player_number(i)) {
                LbPaletteSet(engine_palette);
            }
          }
      }
  }
}

void blast_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx)
{
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    if (slabmap_block_invalid(slb)) {
        return;
    }
    if (slabmap_owner(slb) != plyr_idx) {
        return;
    }
    struct Thing *doortng;
    doortng = get_door_for_position(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    if (!thing_is_invalid(doortng)) {
        destroy_door(doortng);
    }
    struct SlabConfigStats *slabst;
    slabst = get_slab_stats(slb);
    if (slabst->category == SlbAtCtg_FortifiedGround)
    {
      place_slab_type_on_map(SlbT_PATH, slab_subtile_center(slb_x), slab_subtile_center(slb_y), game.neutral_player_num, 1);
      decrease_dungeon_area(plyr_idx, 1);
      do_unprettying(game.neutral_player_num, slb_x, slb_y);
      do_slab_efficiency_alteration(slb_x, slb_y);
      struct Coord3d pos;
      pos.x.val = subtile_coord_center(slab_subtile_center(slb_x));
      pos.y.val = subtile_coord_center(slab_subtile_center(slb_y));
      pos.z.val = get_floor_height_at(&pos);
      create_effect_element(&pos, TngEffElm_RedFlameBig, plyr_idx);
    }
}

static void process_dungeon_devastation_effects(void)
{
    SYNCDBG(8,"Starting");
    int plyr_idx;
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct Dungeon *dungeon;
        dungeon = get_players_num_dungeon(plyr_idx);
        if (dungeon->devastation_turn == 0)
            continue;
        if ((game.play_gameturn & 1) != 0)
            continue;
        dungeon->devastation_turn++;
        if (dungeon->devastation_turn >= max(game.map_tiles_x,game.map_tiles_y))
            continue;
        MapSlabCoord slb_x;
        MapSlabCoord slb_y;
        int i;
        int range;
        slb_x = subtile_slab(dungeon->devastation_centr_x) - dungeon->devastation_turn;
        slb_y = subtile_slab(dungeon->devastation_centr_y) - dungeon->devastation_turn;
        range = 2*dungeon->devastation_turn;
        for (i = 0; i <= range; i++)
        {
            blast_slab(slb_x + i, slb_y,         dungeon->owner);
            blast_slab(slb_x + i, slb_y + range, dungeon->owner);
        }
        for (i = 0; i <= range; i++)
        {
            blast_slab(slb_x,         slb_y + i, dungeon->owner);
            blast_slab(slb_x + range, slb_y + i, dungeon->owner);
        }
    }
}

/**
 * Increments paydays_owed for all players creatures
 * returns amount of creatures needing payday for player
 */
int set_players_creatures_to_get_paid(PlayerNumber plyr_idx)
{
    unsigned long k;
    long i;
    int count = 0;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Creature);
    i = slist->index;
    k = 0;
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (thing->owner == plyr_idx)
        {
            struct CreatureModelConfig *crconf;
            crconf = creature_stats_get_from_thing(thing);
            if (crconf->pay != 0)
            {
                struct CreatureControl *cctrl;
                cctrl = creature_control_get_from_thing(thing);
                if (cctrl->paydays_advanced > 0)
                {
                    cctrl->paydays_advanced--;
                } else
                {
                    if (!creature_is_kept_in_custody_by_enemy(thing))
                    {
                        cctrl->paydays_owed++;
                        count++;
                    }
                    else
                    {
                        cctrl->paydays_advanced--;
                    }
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return count;
}

void process_payday(void)
{
    PlayerNumber plyr_idx;
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        game.pay_day_progress[plyr_idx] = game.pay_day_progress[plyr_idx] + (game.conf.rules[plyr_idx].game.pay_day_speed / 100);
        if (player_is_roaming(plyr_idx) || (plyr_idx == game.neutral_player_num)) {
            continue;
        }
        struct PlayerInfo *player;
        player = get_player(plyr_idx);
        if (player_exists(player) && (player->is_active == 1))
        {
            compute_and_update_player_payday_total(plyr_idx);
            compute_and_update_player_backpay_total(plyr_idx);
        }
    }
    int player_paid_creatures_count;
    for (plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if (game.conf.rules[plyr_idx].game.pay_day_gap <= game.pay_day_progress[plyr_idx])
        {
            if (is_my_player_number(plyr_idx))
                output_message(SMsg_Payday, 0);
            game.pay_day_progress[plyr_idx] = 0;
            player_paid_creatures_count = set_players_creatures_to_get_paid(plyr_idx);
            if (player_paid_creatures_count > 0)
            {
                struct Dungeon *dungeon = get_players_num_dungeon(plyr_idx);
                event_create_event_or_update_nearby_existing_event(0, 0, EvKind_CreaturePayday, plyr_idx, dungeon->creatures_total_pay);
            }
        }
    }
}

void process_dungeons(void)
{
  SYNCDBG(7,"Starting");
  check_players_won();
  check_players_lost();
  process_dungeon_power_magic();
  process_dungeon_devastation_effects();
  process_entrance_generation();
  process_payday();
  process_things_in_dungeon_hand();
  SYNCDBG(9,"Finished");
}

void update_near_creatures_for_footsteps(int32_t *near_creatures, const struct Coord3d *srcpos)
{
    long near_distance[3];
    // Don't allow creatures which are far by over 20 subtiles
    near_distance[0] = subtile_coord(20,0);
    near_distance[1] = subtile_coord(20,0);
    near_distance[2] = subtile_coord(20,0);
    near_creatures[0] = 0;
    near_creatures[1] = 0;
    near_creatures[2] = 0;
    // Find the closest thing for footsteps
    struct Thing *thing;
    unsigned long k;
    long i;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Creature);
    i = slist->index;
    k = 0;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        thing->state_flags &= ~TF1_DoFootsteps;
        if ( (!thing_is_picked_up(thing)) && (!thing_is_dragged_or_pulled(thing)) )
        {
            struct CreatureSound *crsound;
            crsound = get_creature_sound(thing, CrSnd_Foot);
            if (crsound->index > 0)
            {
                struct CreatureControl *cctrl;
                cctrl = creature_control_get_from_thing(thing);
                long ndist;
                ndist = get_chessboard_distance(srcpos, &thing->mappos);
                if (ndist < near_distance[0])
                {
                    if (((cctrl->distance_to_destination != 0) && ((int)thing->floor_height >= (int)thing->mappos.z.val))
                      || ((thing->movement_flags & TMvF_Flying) != 0))
                    {
                        // Insert the new item to our list
                        int n;
                        for (n = 2; n>0; n--)
                        {
                            near_creatures[n] = near_creatures[n-1];
                            near_distance[n] = near_distance[n-1];
                        }
                        near_distance[0] = ndist;
                        near_creatures[0] = thing->index;
                    }
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

long stop_playing_flight_sample_in_all_flying_creatures(void)
{
    struct Thing *thing;
    unsigned long k;
    long i;
    long naffected;
    naffected = 0;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Creature);
    i = slist->index;
    k = 0;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if ((get_creature_model_flags(thing) & CMF_IsDiptera) && ((thing->state_flags & TF1_DoFootsteps) == 0))
        {
            if ( S3DEmitterIsPlayingSample(thing->snd_emitter_id, 25, 0) ) {
                S3DDeleteSampleFromEmitter(thing->snd_emitter_id, 25, 0);
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    return naffected;
}

void update_footsteps_nearest_camera(struct Camera *cam)
{
    static long timeslice = 0;
    static int32_t near_creatures[3];
    struct Coord3d srcpos;
    SYNCDBG(6,"Starting");
    if (cam == NULL)
        return;
    srcpos.x.val = cam->mappos.x.val;
    srcpos.y.val = cam->mappos.y.val;
    srcpos.z.val = cam->mappos.z.val;
    if (timeslice == 0) {
        update_near_creatures_for_footsteps(near_creatures, &srcpos);
    }
    long i;
    for (i=0; i < 3; i++)
    {
        struct Thing *thing;
        if (near_creatures[i] == 0)
            break;
        thing = thing_get(near_creatures[i]);
        if (thing_is_creature(thing)) {
            thing->state_flags |= TF1_DoFootsteps;
            play_thing_walking(thing);
        }
    }
    if (timeslice == 0)
    {
        stop_playing_flight_sample_in_all_flying_creatures();
    }
    timeslice = (timeslice + 1) % 4;
}

int clear_active_dungeons_stats(void)
{
  struct Dungeon *dungeon;
  int i;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
      dungeon = get_dungeon(i);
      if (dungeon_invalid(dungeon))
          break;
      memset((char *)dungeon->crmodel_state_type_count, 0, game.conf.crtr_conf.model_count * STATE_TYPES_COUNT * sizeof(uint16_t));
      memset((char *)dungeon->guijob_all_creatrs_count, 0, game.conf.crtr_conf.model_count *3*sizeof(uint16_t));
      memset((char *)dungeon->guijob_angry_creatrs_count, 0, game.conf.crtr_conf.model_count *3*sizeof(uint16_t));
  }
  return i;
}

TngUpdateRet damage_creatures_with_physical_force(struct Thing *thing, ModTngFilterParam param)
{
    SYNCDBG(18,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    if (thing_is_picked_up(thing) || thing_is_dragged_or_pulled(thing))
    {
        return TUFRet_Unchanged;
    }
    if (thing_is_creature(thing))
    {
        apply_damage_to_thing_and_display_health(thing, param->secondary_number, param->primary_number);
        if ((thing->health >= 0) && !creature_is_leaving_and_cannot_be_stopped(thing))
        {
            if (((thing->alloc_flags & TAlF_IsControlled) == 0) && !creature_is_kept_in_custody(thing))
            {
                if (get_creature_state_besides_interruptions(thing) != CrSt_CreatureEscapingDeath)
                {
                    if (cleanup_current_thing_state(thing) && setup_move_out_of_cave_in(thing))
                        thing->continue_state = CrSt_CreatureEscapingDeath;
                }
            }
            return TUFRet_Modified;
        } else
        {
            kill_creature(thing, INVALID_THING, param->primary_number, CrDed_NoEffects|CrDed_DiedInBattle);
            return TUFRet_Deleted;
        }
    }
    else if (thing_is_destructible_trap(thing) > 0)
    {
        apply_damage_to_thing(thing, param->secondary_number, param->primary_number);
        return TUFRet_Modified;
    }
    return TUFRet_Unchanged;
}

TbBool valid_cave_in_position(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x,stl_y);
    if ((mapblk->flags & SlbAtFlg_Blocking) != 0)
        return false;
    struct SlabMap *slb;
    slb = get_slabmap_for_subtile(stl_x,stl_y);
    return (plyr_idx == game.neutral_player_num) || (slabmap_owner(slb) == game.neutral_player_num) || (slabmap_owner(slb) == plyr_idx);
}

long update_cave_in(struct Thing *thing)
{
    thing->health--;
    thing->rendering_flags |= TRF_Invisible;
    if (thing->health < 1)
    {
        delete_thing_structure(thing, 0);
        return 1;
    }

    const struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(PwrK_CAVEIN);
    struct Thing *efftng;
    struct Coord3d pos;
    PlayerNumber owner;
    owner = thing->owner;
    if ((game.play_gameturn % 3) == 0)
    {
        int n;
        n = GAME_RANDOM(AROUND_TILES_COUNT);
        pos.x.val = thing->mappos.x.val + GAME_RANDOM(704) * around[n].delta_x;
        pos.y.val = thing->mappos.y.val + GAME_RANDOM(704) * around[n].delta_y;
        if (subtile_has_slab(coord_subtile(pos.x.val),coord_subtile(pos.y.val)))
        {
            pos.z.val = get_ceiling_height(&pos) - 128;
            efftng = create_effect_element(&pos, TngEff_Flash, owner);
            if (!thing_is_invalid(efftng)) {
                efftng->health = powerst->duration;
            }
        }
    }

    GameTurnDelta turns_between;
    GameTurnDelta turns_alive;
    turns_between = powerst->duration / 5;
    turns_alive = game.play_gameturn - thing->creation_turn;
    if ((turns_alive != 0) && ((turns_between < 1) || (3 * turns_between / 4 == turns_alive % turns_between)))
    {
        pos.x.val = thing->mappos.x.val + THING_RANDOM(thing, 128);
        pos.y.val = thing->mappos.y.val + THING_RANDOM(thing, 128);
        pos.z.val = get_floor_height_at(&pos) + 384;
        create_effect(&pos, TngEff_HarmlessGas4, owner);
    }

    if ((turns_alive % game.conf.rules[owner].magic.turns_per_collapse_dngn_dmg) == 0)
    {
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = subtile_coord(1,0);
        Thing_Modifier_Func do_cb;
        struct CompoundTngFilterParam param;
        param.plyr_idx = -1;
        param.class_id = 0;
        param.model_id = 0;
        param.primary_number = thing->owner;
        param.secondary_number = game.conf.rules[thing->owner].magic.collapse_dungeon_damage;
        param.tertiary_pointer = 0;
        do_cb = damage_creatures_with_physical_force;
        do_to_things_with_param_around_map_block(&pos, do_cb, &param);
    }

    if ((8 * powerst->duration / 10 >= thing->health) && (2 * powerst->duration / 10 <= thing->health))
    {
        if ((powerst->duration < 10) || ((thing->health % (powerst->duration / 10)) == 0))
        {
            int round_idx;
            round_idx = THING_RANDOM(thing, AROUND_TILES_COUNT);
            set_coords_to_slab_center(&pos, subtile_slab(thing->mappos.x.val + 3 * around[round_idx].delta_x), subtile_slab(thing->mappos.y.val + 3 * around[round_idx].delta_y));
            if (subtile_has_slab(coord_subtile(pos.x.val), coord_subtile(pos.y.val)) && valid_cave_in_position(thing->owner, coord_subtile(pos.x.val), coord_subtile(pos.y.val)))
            {
                struct Thing *ncavitng;
                ncavitng = get_cavein_at_subtile_owned_by(coord_subtile(pos.x.val), coord_subtile(pos.y.val), -1);
                if (thing_is_invalid(ncavitng))
                {
                    long dist;
                    struct Coord3d pos2;
                    pos2.x.val = subtile_coord(thing->cave_in.x,0);
                    pos2.y.val = subtile_coord(thing->cave_in.y,0);
                    pos2.z.val = subtile_coord(1,0);
                    dist = get_chessboard_distance(&pos, &pos2);
                    if (powerst->strength[thing->cave_in.model] >= coord_subtile(dist))
                    {
                        ncavitng = create_thing(&pos, TCls_CaveIn, thing->cave_in.model, owner, -1);
                        if (!thing_is_invalid(ncavitng))
                        {
                            thing->health += 5;
                            if (thing->health > 0)
                            {
                                ncavitng->cave_in.x = thing->cave_in.x;
                                ncavitng->cave_in.y = thing->cave_in.y;
                            }
                        }
                    }
                }
            }
        }
    }
    return 1;
}

/**
 * Checks if a gamerule for lighting has changed and updates the lights if they are.
 * This function also refreshes the light status of the map.
*/
void update_global_lighting()
{
    // Check if any values have changed
    if (
        game.conf.rules[0].game.global_ambient_light != game.lish.global_ambient_light ||
        game.conf.rules[0].game.light_enabled != game.lish.light_enabled
    ){

        // GlobalAmbientLight
        if (game.conf.rules[0].game.global_ambient_light != game.lish.global_ambient_light)
        {
            game.lish.global_ambient_light = game.conf.rules[0].game.global_ambient_light;
        }

        // LightEnabled
        if (game.conf.rules[0].game.light_enabled != game.lish.light_enabled)
        {
            game.lish.light_enabled = game.conf.rules[0].game.light_enabled;
        }

        // Refresh the lights
        light_stat_refresh();
    }
}

void update(void)
{
    struct PlayerInfo *player;
    SYNCDBG(4,"Starting for turn %ld",(long)game.play_gameturn);

    process_packets();
    api_update_server();

    if (quit_game || exit_keeper) {
        return;
    }
    if (game.game_kind == GKind_NonInteractiveState)
    {
        game.map_changed_for_nagivation = 0;
        return;
    }
    player = get_my_player();

    if (!flag_is_set(game.operation_flags,GOF_Paused))
    {
        if (flag_is_set(player->additional_flags,PlaAF_LightningPaletteIsActive))
        {
            PaletteSetPlayerPalette(player, engine_palette);
            clear_flag(player->additional_flags, PlaAF_LightningPaletteIsActive);
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
        process_fx_lines();
        lua_on_game_tick();
        if ((game.view_mode_flags & GNFldD_ComputerPlayerProcessing) != 0)
            process_computer_players2();
        process_players();
        process_action_points();
        player = get_my_player();
        if (player->view_mode == PVM_CreatureView)
        {
            struct Thing *thing = thing_get(player->controlled_thing_idx);
            update_first_person_object_ambience(thing);
        }
        update_footsteps_nearest_camera(player->acamera);
        PaletteFadePlayer(player);
        process_armageddon();
        update_global_lighting();
#if (BFDEBUG_LEVEL > 9)
        lights_stats_debug_dump();
        things_stats_debug_dump();
        creature_stats_debug_dump();
#endif
        game.play_gameturn++;
    }

    message_update();
    update_all_players_cameras();
    update_player_sounds();
    first_gameturn_actions();
    SYNCDBG(6,"Finished");
}

void intentional_desync() {
    if (game.play_gameturn != 50 || !is_my_player_number(0)) {
        return;
    }
    for (struct Room* room = start_rooms; room < end_rooms; room += 1) {
        if (room_exists(room)) {
            room->slabs_count += 1;
            break;
        }
    }
    int i = game.thing_lists[TngList_Creatures].index;
    if (i != 0) {
        struct Thing* thing = thing_get(i);
        if (!thing_is_invalid(thing)) {
            thing->health += 1;
        }
    }
    get_player(0)->instance_remain_turns += 1;
}

void first_gameturn_actions() {
    if (game.play_gameturn == 1) {
        apply_default_flee_and_imprison_setting();
        send_sprite_zip_count_to_other_players();
    }
    //intentional_desync();
}

long near_map_block_thing_filter_queryable_object(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
/* Currently this only makes Dungeon Heart blinking; maybe I'll find a purpose for it later
    long dist_x,dist_y;
    if ((thing->class_id == TCls_Object) && (thing->model == 5))
    {
      if (thing->owner == param->plyr_idx)
      {
          // note that abs() is not required because we're computing square of the values
          dist_x = param->primary_number-(MapCoord)thing->mappos.x.val;
          dist_y = param->secondary_number-(MapCoord)thing->mappos.y.val;
          // This function should return max value when the distance is minimal, so:
          return INT32_MAX-(dist_x*dist_x + dist_y*dist_y);
      }
    }
*/
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

struct Thing *get_queryable_object_near(MapCoord pos_x, MapCoord pos_y, PlayerNumber plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    filter = near_map_block_thing_filter_queryable_object;
    param.plyr_idx = plyr_idx;
    param.primary_number = pos_x;
    param.secondary_number = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

void set_player_cameras_position(struct PlayerInfo *player, int32_t pos_x, int32_t pos_y)
{
    player->cameras[CamIV_Parchment].mappos.x.val = pos_x;
    player->cameras[CamIV_FrontView].mappos.x.val = pos_x;
    player->cameras[CamIV_Isometric].mappos.x.val = pos_x;
    player->cameras[CamIV_Parchment].mappos.y.val = pos_y;
    player->cameras[CamIV_FrontView].mappos.y.val = pos_y;
    player->cameras[CamIV_Isometric].mappos.y.val = pos_y;
}

void scale_tmap2(long texture_block_index, long flags, long fade_level, long screen_x, long screen_y, long scaled_width, long scaled_height)
{
    if ((scaled_width == 0) || (scaled_height == 0)) {
        return;
    }
    long xstart;
    long ystart;
    long xend;
    long yend;
    char orient;
    switch (flags)
    {
    case 0:
        xstart = 0;
        ystart = 0;
        xend = 2097151 / scaled_width;
        yend = 2097151 / scaled_height;
        orient = 0;
        break;
    case 0x10:
        xstart = 2097151;
        ystart = 0;
        xend = -2097151 / scaled_width;
        yend = 2097151 / scaled_height;
        orient = 0;
        break;
    case 0x20:
        xstart = 0;
        ystart = 2097151;
        xend = 2097151 / scaled_width;
        yend = -2097151 / scaled_height;
        orient = 0;
        break;
    case 0x30:
        xstart = 2097151;
        ystart = 2097151;
        xend = -2097151 / scaled_width;
        yend = -2097151 / scaled_height;
        orient = 0;
        break;
    case 0x40:
        ystart = 0;
        xstart = 0;
        yend = 2097151 / scaled_height;
        xend = 2097151 / scaled_width;
        orient = 1;
        break;
    case 0x50:
        ystart = 0;
        xstart = 2097151;
        yend = 2097151 / scaled_height;
        xend = -2097151 / scaled_width;
        orient = 1;
        break;
    case 0x60:
        ystart = 2097151;
        xstart = 0;
        yend = -2097151 / scaled_height;
        xend = 2097151 / scaled_width;
        orient = 1;
        break;
    case 0x70:
        xstart = 2097151;
        ystart = 2097151;
        yend = -2097151 / scaled_height;
        xend = -2097151 / scaled_width;
        orient = 1;
        break;
    default:
          return;
    }
    long local_screen_x;
    long local_screen_y;
    local_screen_x = screen_x;
    if (local_screen_x < 0)
    {
        scaled_width += local_screen_x;
        if (scaled_width < 0) {
            return;
        }
        xstart -= xend * local_screen_x;
        local_screen_x = 0;
    }
    if (local_screen_x + scaled_width > vec_window_width)
    {
        scaled_width = vec_window_width - local_screen_x;
        if (scaled_width < 0) {
            return;
        }
    }
    local_screen_y = screen_y;
    if (local_screen_y < 0)
    {
        scaled_height += local_screen_y;
        if (scaled_height < 0) {
            return;
        }
        ystart -= local_screen_y * yend;
        local_screen_y = 0;
    }
    if (local_screen_y + scaled_height > vec_window_height)
    {
        scaled_height = vec_window_height - local_screen_y;
        if (scaled_height < 0) {
            return;
        }
    }
    int i;
    int32_t hlimits[480];
    int32_t wlimits[640];
    int32_t *xlim;
    int32_t *ylim;
    unsigned char *dbuf;
    unsigned char *block;
    if (!orient)
    {
        xlim = wlimits;
        for (i = scaled_width; i > 0; i--)
        {
            *xlim = xstart;
            xlim++;
            xstart += xend;
        }
        ylim = hlimits;
        for (i = scaled_height; i > 0; i--)
        {
            *ylim = ystart;
            ylim++;
            ystart += yend;
        }
        dbuf = &vec_screen[local_screen_x + local_screen_y * vec_screen_width];
        block = block_ptrs[texture_block_index];
        ylim = hlimits;
        long px;
        long py;
        int srcx;
        int srcy;
        unsigned char *d;
        if ( fade_level >= 0 )
        {
          for (py = scaled_height; py > 0; py--)
          {
              xlim = wlimits;
              d = dbuf;
              srcy = (((*ylim) & 0xFF0000u) >> 16);
              for (px = scaled_width; px > 0; px--)
              {
                srcx = (((*xlim) & 0xFF0000u) >> 16);
                xlim++;
                *d = pixmap.fade_tables[256 * fade_level + block[(srcy << 8) + srcx]];
                ++d;
              }
              dbuf += vec_screen_width;
              ylim++;
          }
        } else
        {
          for (py = scaled_height; py > 0; py--)
          {
            xlim = wlimits;
            d = dbuf;
            srcy = (((*ylim) & 0xFF0000u) >> 16);
            for (px = scaled_width; px > 0; px--)
            {
              srcx = (((*xlim) & 0xFF0000u) >> 16);
              xlim++;
              *d = block[(srcy << 8) + srcx];
              ++d;
            }
            dbuf += vec_screen_width;
            ylim++;
          }
        }
    } else
    {
        ylim = wlimits;
        for (i = scaled_height; i > 0; i--)
        {
          *ylim = ystart;
          ylim++;
          ystart += yend;
        }
        xlim = hlimits;
        for (i = scaled_width; i > 0; i--)
        {
          *xlim = xstart;
          xlim++;
          xstart += xend;
        }
        dbuf = &vec_screen[local_screen_x + local_screen_y * vec_screen_width];
        block = block_ptrs[texture_block_index];
        ylim = wlimits;
        long px;
        long py;
        int srcx;
        int srcy;
        unsigned char *d;
        if ( fade_level >= 0 )
        {
          for (py = scaled_height; py > 0; py--)
          {
              xlim = hlimits;
              d = dbuf;
              srcy = (((*ylim) & 0xFF0000u) >> 16);
              for (px = scaled_width; px > 0; px--)
              {
                srcx = (((*xlim) & 0xFF0000u) >> 16);
                xlim++;
                *d = pixmap.fade_tables[256 * fade_level + block[(srcx << 8) + srcy]];
                ++d;
              }
              dbuf += vec_screen_width;
              ylim++;
          }
        } else
        {
          for (py = scaled_height; py > 0; py--)
          {
            xlim = hlimits;
            d = dbuf;
            srcy = (((*ylim) & 0xFF0000u) >> 16);
            for (px = scaled_width; px > 0; px--)
            {
              srcx = (((*xlim) & 0xFF0000u) >> 16);
              xlim++;
              *d = block[(srcx << 8) + srcy];
              ++d;
            }
            dbuf += vec_screen_width;
            ylim++;
          }
        }
    }
}

void draw_texture(int32_t texture_x, int32_t texture_y, int32_t texture_width, int32_t texture_height, int32_t texture_block_index, int32_t flags, int32_t fade_level)
{
    scale_tmap2(texture_block_index, flags, fade_level, texture_x / pixel_size, texture_y / pixel_size, texture_width / pixel_size, texture_height / pixel_size);
}

void update_block_pointed(int i,long x, long x_frac, long y, long y_frac)
{
    struct Map *mapblk;
    struct Column *colmn;
    short visible;
    unsigned int smask;
    long k;

    if (i > 0)
    {
      mapblk = get_map_block_at(x,y);
      visible = map_block_revealed(mapblk, my_player_number);
      if ((!visible) || (get_mapblk_column_index(mapblk) > 0))
      {
        if (visible)
          k = get_mapblk_column_index(mapblk);
        else
          k = game.unrevealed_column_idx;
        colmn = get_column(k);
        smask = colmn->solidmask;
        if ((temp_cluedo_mode) && (smask != 0))
        {
          if (visible)
            k = get_mapblk_column_index(mapblk);
          else
            k = game.unrevealed_column_idx;
          colmn = get_column(k);
          if (colmn->solidmask >= 8)
          {
            if ( (!visible) || (((get_navigation_map(x,y) & 0x80) == 0) && ((mapblk->flags & SlbAtFlg_IsRoom) == 0)) )
              smask &= 3;
          }
        }
        if (smask & (1 << (i-1)))
        {
          pointed_at_frac_x = x_frac;
          pointed_at_frac_y = y_frac;
          block_pointed_at_x = x;
          block_pointed_at_y = y;
          me_pointed_at = mapblk;
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
        mapblk = get_map_block_at(x,y);
        floor_pointed_at_x = x;
        floor_pointed_at_y = y;
        block_pointed_at_x = x;
        block_pointed_at_y = y;
        pointed_at_frac_x = x_frac;
        pointed_at_frac_y = y_frac;
        me_pointed_at = mapblk;
    }
}

void update_blocks_pointed(void)
{
    long x;
    long y;
    long x_frac;
    long y_frac;
    long hori_ptr_y;
    long vert_ptr_y;
    long hori_hdelta_y;
    long vert_hdelta_y;
    long hori_ptr_x;
    long vert_ptr_x;
    long hvdiv_x;
    long hvdiv_y;
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
          if ((x >= 0) && (x < game.map_subtiles_x) && (y >= 0) && (y < game.map_subtiles_y))
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

    flg_mem = lbDisplay.DrawFlags;
    update_engine_settings(player);
    mx = cam->mappos.x.val;
    my = cam->mappos.y.val;
    mz = cam->mappos.z.val;
    pointer_x = (GetMouseX() - player->engine_window_x) / pixel_size;
    pointer_y = (GetMouseY() - player->engine_window_y) / pixel_size;
    lens = cam->horizontal_fov * scale_value_by_horizontal_resolution(4) / pixel_size;
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
    if ((game.operation_flags & GOF_Paused) != 0)
      return true;
    if ( (game.turns_fastforward == 0) && (!game.packet_loading_in_progress) )
    {
      find_frame_rate();
      if ( (game.frame_skip == 0) || ((game.play_gameturn % game.frame_skip) == 0))
        return true;
    } else
    if ( ((game.play_gameturn & 0x3F)==0) ||
         ((game.packet_loading_in_progress) && ((game.play_gameturn & 7)==0)) )
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
    const long double tick_ns_one_sec = 1000000000.0;
    long double tick_ns_one_frame = -1;
    if ((game.view_mode_flags & GNFldD_WaitSleepMode) != 0)
    {
        // No idea when such situation occurs
        tick_ns_one_frame = tick_ns_one_sec;
    }
    if (game.frame_skip >= 0)
    {
        // Standard delaying system
        long num_fps = game_num_fps;
        if (game.frame_skip > 0)
            num_fps *= game.frame_skip;

        tick_ns_one_frame = tick_ns_one_sec/num_fps;
    }

    if (tick_ns_one_frame >= 0) {
        static long double tick_ns_last_turn = 0;

        long double tick_ns_cur = get_time_tick_ns();
        long double tick_ns_used = tick_ns_cur - tick_ns_last_turn;
        long double tick_ns_delay = tick_ns_one_frame - tick_ns_used;

        long double tick_ns_end = tick_ns_cur;
        // tick_ns_used: every level, initialized_time_point will be reset, so tick_ns_used may be less than 0 when enter level for the non-first time, Skip it directly to solve the problem.
        if (tick_ns_delay > 0 && tick_ns_used >= 0) {
            tick_ns_end = tick_ns_cur + tick_ns_delay;
            LbSleepUntilExt(tick_ns_end);
        }
        tick_ns_last_turn = tick_ns_end;
        return true;
    }

    return false;
}


TbBool keeper_wait_for_next_draw(void)
{
    // fps.draw is currently unable to work properly with frame_skip
    if (game_num_fps_draw_current > 0 && is_feature_on(Ft_DeltaTime) == true && game.frame_skip == 0)
    {
        const long double tick_ns_one_sec = 1000000000.0;
        const long double tick_ns_one_frame = tick_ns_one_sec/game_num_fps_draw_current;

        static long double tick_ns_last_draw = 0;
        long double tick_ns_cur = get_time_tick_ns();
        long double tick_ns_used = tick_ns_cur - tick_ns_last_draw;
        long double tick_ns_delay = tick_ns_one_frame - tick_ns_used;

        long double tick_ns_end = tick_ns_cur;
        // tick_ns_used: every level, initialized_time_point will be reset, so tick_ns_used may be less than 0 when enter level for the non-first time, Skip it directly to solve the problem.
        if (tick_ns_delay > 0 && tick_ns_used >= 0) {
            tick_ns_end = tick_ns_cur + tick_ns_delay;
            LbSleepUntilExt(tick_ns_end);
        }
        tick_ns_last_draw = tick_ns_end;

        return true;
    }
    return false;
}

void redetect_screen_refresh_rate_for_draw()
{
    game_num_fps_draw_current = 0;

    if (game_num_fps_draw_main == -1) {
        if (game_num_fps_draw_secondary > 0)
            game_num_fps_draw_current = game_num_fps_draw_secondary;

        if (lbWindow != NULL) {
            int display_index = SDL_GetWindowDisplayIndex(lbWindow);
            if (display_index >= 0) {
                SDL_DisplayMode mode;
                if (SDL_GetCurrentDisplayMode(display_index, &mode) == 0 && mode.refresh_rate > 0) {
                    game_num_fps_draw_current = mode.refresh_rate;
                }
            }
        }

    } else if (game_num_fps_draw_main > 0) {
        game_num_fps_draw_current = game_num_fps_draw_main;
    }
}

TbBool keeper_wait_for_screen_focus(void)
{
    do {
        if ( !LbWindowsControl() )
        {
          force_application_close();
          break;
        }
        if (LbIsActive())
          return true;
        if ((game.system_flags & GSF_NetworkActive) != 0)
          return true;
        if (!freeze_game_on_focus_lost())
          return true;
        LbSleepFor(50);
    } while ((!exit_keeper) && (!quit_game));
    return false;
}

void gameplay_loop_logic()
{
    if(flag_is_set(start_params.debug_flags, DFlg_PauseAtGameTurn))
    {
        static GameTurn previous_gameturn = 0;
        if(game.play_gameturn >= start_params.pause_at_gameturn && game.play_gameturn != previous_gameturn)
        {
            if(!game.paused_at_gameturn)
            {
                game.paused_at_gameturn = true;

                game.frame_skip = 0;
                if(game.packet_load_enable)
                {
                    disable_packet_mode();
                }
                set_packet_pause_toggle();
            }
        }
        previous_gameturn = game.play_gameturn;
    }

    if (is_feature_on(Ft_DeltaTime) == true) {
        if (game.process_turn_time < 1.0) {
            return;
        }
        game.process_turn_time -= 1.0;
    }

    frametime_start_measurement(Frametime_Logic);
    if (frametime_enabled())
        framerate_measurement_capture(Framerate_Logic);

#ifdef FUNCTESTING
    if(flag_is_set(start_params.functest_flags, FTF_Enabled))
    {
        FTestFrameworkState ftstate = ftest_update(NULL);
        if(ftstate == FTSt_InvalidState || ftstate == FTSt_TestsCompletedSuccessfully)
        {
            quit_game = true;
            exit_keeper = true;
            return;
        }
    }
#endif // FUNCTESTING
    do_draw = display_should_be_updated_this_turn() || (!LbIsActive());
    LbWindowsControl();
    input_eastegg();
    input();
    update();
    frametime_end_measurement(Frametime_Logic);

    if(game.frame_step)
    {
        game.frame_step = false;
        set_packet_pause_toggle();
    }
}

void gameplay_loop_draw()
{
    // Floats are used a lot in the drawing related functions. But keep in mind integers are typically preferred for logic related functions.
    frametime_start_measurement(Frametime_Draw);

    // Update lights
    if ((game.operation_flags & GOF_Paused) == 0) {
        update_light_render_area();
    }

    if (quit_game || exit_keeper) {
        do_draw = false;
    }
    if ( do_draw ) {
        if (frametime_enabled())
            framerate_measurement_capture(Framerate_Draw);
        keeper_screen_redraw();
    }
    keeper_wait_for_screen_focus();
    // Direct information/error messages
    if (LbScreenLock() == Lb_SUCCESS) {
        if ( do_draw ) {
            perform_any_screen_capturing();
        }
        draw_onscreen_direct_messages();
        LbScreenUnlock();
    }
    // Move the graphics window to center of screen buffer and swap screen
    if ( do_draw ) {
        keeper_screen_swap();
    }
    frametime_end_measurement(Frametime_Draw);
    last_draw_completed_time = get_time_tick_ns();
}

extern "C" void network_yield_draw_gameplay()
{
    game.delta_time = get_delta_time();
    game.process_turn_time += game.delta_time;
    gameplay_loop_draw();
}

extern "C" void update_velocity(void);
extern "C" void check_mouse_scroll(void);
extern "C" void fronttorture_update(void);

extern "C" void network_yield_draw_frontend()
{
    game.delta_time = get_delta_time();
    if (frontend_menu_state == FeSt_NETLAND_VIEW) {
        check_mouse_scroll();
        update_velocity();
    }
    if (frontend_menu_state == FeSt_TORTURE) {
        fronttorture_update();
    }
    if (frontend_menu_state == FeSt_NET_START) {
        LbWindowsControl();
        frontnet_start_input();
    }
    frontend_draw();
    LbScreenSwap();
}

void gameplay_loop_timestep()
{
    frametime_start_measurement(Frametime_Sleep);
    if (is_feature_on(Ft_DeltaTime) == true) {
        game.delta_time = get_delta_time();
        game.process_turn_time += game.delta_time;
    } else {
        // Set to 1 so that these variables don't affect anything. (if something is multiplied by 1 it doesn't change)
        game.delta_time = 1;
        game.process_turn_time = 1;
        // Make delay if the machine is too fast
        if ( (!game.packet_load_enable) || (game.turns_fastforward == 0) ) {
            keeper_wait_for_next_turn();
        }
    }

    if (game.turns_packetoff == game.play_gameturn) {
        exit_keeper = 1;
    }

    if (game_num_fps_draw_current > 0 && is_feature_on(Ft_DeltaTime) == true) {
        keeper_wait_for_next_draw();

        if (game.turns_packetoff == game.play_gameturn) {
            exit_keeper = 1;
        }
    }
    
    frametime_end_measurement(Frametime_Sleep);
}

void keeper_gameplay_loop(void)
{
    struct PlayerInfo *player;
    SYNCDBG(5,"Starting");
    player = get_my_player();
    PaletteSetPlayerPalette(player, engine_palette);
    if ((game.operation_flags & GOF_SingleLevel) != 0) {
        initialise_eye_lenses();
    }
    SYNCDBG(0,"Entering the gameplay loop for level %d",(int)get_loaded_level_number());
    KeeperSpeechClearEvents();
    LbErrorParachuteUpdate(); // For some reasone parachute keeps changing; Remove when won't be needed anymore

    initial_time_point();
    LbSleepExtInit();
    LbNetwork_TimesyncBarrier();

    //the main gameplay loop starts
    while ((!quit_game) && (!exit_keeper))
    {
        frametime_start_measurement(Frametime_FullFrame);
        if (frametime_enabled())
            framerate_measurement_capture(Framerate_FullFrame);
        gameplay_loop_logic();
        gameplay_loop_draw();
        gameplay_loop_timestep();

        frametime_end_measurement(Frametime_FullFrame);
    } // end while
    SYNCDBG(0,"Gameplay loop finished after %lu turns",(unsigned long)game.play_gameturn);

    // Reset the game kind because we are not in a game anymore at this point
    game.game_kind = GKind_Unset;

    api_event("GAME_ENDED");
}

TbBool can_thing_be_queried(struct Thing *thing, PlayerNumber plyr_idx)
{
    if ( (!thing_is_creature(thing)) || !( (thing->owner == plyr_idx) || (creature_is_kept_in_custody_by_player(thing, plyr_idx)) ) || (thing->alloc_flags & TAlF_IsInLimbo) || (thing->state_flags & TF1_InCtrldLimbo) || (thing->active_state == CrSt_CreatureUnconscious) )
    {
        return false;
    }
    unsigned char state = (thing->active_state == CrSt_MoveToPosition) ? thing->continue_state : thing->active_state;
    if ( (state == CrSt_CreatureSacrifice) || (state == CrSt_CreatureBeingSacrificed) || (state == CrSt_CreatureBeingSummoned) )
    {
        return false;
    }
    else
    {
        return true;
    }
}

long packet_place_door(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel, TbBool allowed)
{
    if (!allowed) {
        if (is_my_player_number(plyr_idx))
            play_non_3d_sample(119);
        return 0;
    }
    if (!player_place_door_at(stl_x, stl_y, plyr_idx, tngmodel)) {
        return 0;
    }
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    delete_room_slabbed_objects(get_slab_number(slb_x, slb_y));
    remove_dead_creatures_from_slab(slb_x, slb_y);
    return 1;
}

void initialise_map_collides(void)
{
    SYNCDBG(7,"Starting");
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    for (slb_y=0; slb_y < game.map_tiles_y; slb_y++)
    {
        for (slb_x=0; slb_x < game.map_tiles_x; slb_x++)
        {
            struct SlabMap *slb;
            slb = get_slabmap_block(slb_x, slb_y);
            int ssub_x;
            int ssub_y;
            for (ssub_y=0; ssub_y < STL_PER_SLB; ssub_y++)
            {
                for (ssub_x=0; ssub_x < STL_PER_SLB; ssub_x++)
                {
                    MapSubtlCoord stl_x;
                    MapSubtlCoord stl_y;
                    stl_x = slab_subtile(slb_x,ssub_x);
                    stl_y = slab_subtile(slb_y,ssub_y);
                    struct Map *mapblk;
                    mapblk = get_map_block_at(stl_x, stl_y);
                    mapblk->flags = 0;
                    update_map_collide(slb->kind, stl_x, stl_y);
                }
            }
        }
    }
}

void initialise_map_health(void)
{
    SYNCDBG(7,"Starting");
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    for (slb_y=0; slb_y < game.map_tiles_y; slb_y++)
    {
        for (slb_x=0; slb_x < game.map_tiles_x; slb_x++)
        {
            struct SlabMap *slb;
            slb = get_slabmap_block(slb_x, slb_y);
            struct SlabConfigStats *slabst;
            slabst = get_slab_stats(slb);
            slb->health = game.block_health[slabst->block_health_index];
        }
    }
}

static TbBool wait_at_frontend(void)
{
    struct PlayerInfo *player;
    // This is an improvised coroutine-like stuff
    CoroutineLoop loop;
    memset(&loop, 0, sizeof(loop));

    SYNCDBG(0,"Falling into frontend menu.");
    // Moon phase calculation
    calculate_moon_phase(true,false);
    update_extra_levels_visibility();
    // Returning from Demo Mode
    if (game.mode_flags & MFlg_IsDemoMode)
    {
      close_packet_file();
      game.packet_load_enable = 0;
    }
    game.save_game_slot = -1;
    // Make sure campaigns are loaded
    if (!load_campaigns_list())
    {
      ERRORLOG("No valid campaign files found");
      exit_keeper = 1;
      return true;
    }
    // Make sure mappacks are loaded
    if (!load_mappacks_list())
    {
      WARNMSG("No valid mappack files found");
    }
    //Set level number and campaign (for single level mode: GOF_SingleLevel)
    if ((start_params.operation_flags & GOF_SingleLevel) != 0)
    {
        TbBool result = false;
        if (start_params.selected_campaign[0] != '\0')
        {
            str_append(start_params.selected_campaign, sizeof(start_params.selected_campaign), ".cfg");
            result = change_campaign(start_params.selected_campaign);
        }
        if (!result) {
            if (!change_campaign("")) {
                WARNMSG("Unable to load default campaign for the specified level CMD Line parameter");
            }
            else if (start_params.selected_campaign[0] != '\0') { // only show this log message if the user actually specified a campaign
                WARNMSG("Unable to load campaign associated with the specified level CMD Line parameter, default loaded.");
            }
            else {
                JUSTLOG("No campaign specified. Default campaign loaded for selected level (%u).", start_params.selected_level_number);
            }
        }
        set_selected_level_number(start_params.selected_level_number);
        //game.selected_level_number = start_params.selected_level_number;
    }
    else
    {
        set_selected_level_number(first_singleplayer_level());
    }
    // Init load/save catalogue
    initialise_load_game_slots();

    #ifdef FUNCTESTING
    if(flag_is_set(start_params.functest_flags, FTF_Enabled)) //override for functional tests
    {
        FTestFrameworkState ft_prev_state = FTSt_InvalidState;
        FTestFrameworkState ft_current_state = ftest_update(&ft_prev_state);

        TbBool user_aborted_tests = ft_prev_state == FTSt_TestIsProcessingActions && ft_current_state == FTSt_TestIsProcessingActions;
        if(user_aborted_tests)
        {
            FTEST_FAIL_TEST("User aborted tests");
        }

        if(ft_current_state == FTSt_InvalidState || ft_current_state == FTSt_TestsCompletedSuccessfully || user_aborted_tests)
        {
            quit_game = true;
            exit_keeper = true;
            return true;
        }
        faststartup_network_game(&loop);
        coroutine_process(&loop);
        return true;
    }
    #endif

    // Prepare to enter PacketLoad game
    if ((game.packet_load_enable) && (!game.packet_load_initialized))
    {
      faststartup_saved_packet_game();
      return true;
    }
    // Load single-player level directly from command line arguments (-server and -connect bypass this, autoloading a multiplayer map is handled elsewhere)
    if ((game.operation_flags & GOF_SingleLevel) != 0 && !(game_flags2 & (GF2_Connect | GF2_Server)))
    {
      faststartup_network_game(&loop);
      coroutine_process(&loop);
      return true;
    }

    if ( !setup_screen_mode_minimal(get_frontend_vidmode()) )
    {
      FatalError = 1;
      exit_keeper = 1;
      return true;
    }
    LbScreenClear(0);
    LbScreenSwap();
    if (frontend_load_data() != Lb_SUCCESS)
    {
      ERRORLOG("Unable to load frontend data");
      exit_keeper = 1;
      return true;
    }
    memset(scratch, 0, PALETTE_SIZE);
    LbPaletteSet(scratch);
    frontend_set_state(get_startup_menu_state());
    try_restore_frontend_error_box();

    LbWindowsControl();
    clear_mouse_pressed_lrbutton();

    short finish_menu = 0;
    clear_flag(game.mode_flags, MFlg_DemoMode);
    // TODO move to separate function
    // Begin the frontend loop
    long fe_last_loop_time = LbTimerClock();
    do
    {
      if (!LbWindowsControl())
      {
        force_application_close();
        SYNCDBG(0,"Windows Control exit condition invoked");
        break;
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
        MonitorStreamedSoundTrack();
      }

      if (fade_palette_in)
      {
        fade_in();
        fade_palette_in = 0;
      } else {
        if (is_feature_on(Ft_DeltaTime) == true && should_use_delta_time_on_menu()) {
          game.delta_time = get_delta_time();
        } else {
          game.delta_time = 1;
          LbSleepUntil(fe_last_loop_time + 30);
        }
      }
      fe_last_loop_time = LbTimerClock();

      api_update_server();

    } while (!finish_menu);

    LbPaletteFade(0, 8, Lb_PALETTE_FADE_CLOSED);
    LbScreenClear(0);
    LbScreenSwap();
    FrontendMenuState prev_state;
    prev_state = frontend_menu_state;
    frontend_set_state(FeSt_INITIAL);
    if (exit_keeper)
    {
      player = get_my_player();
      player->display_flags &= ~PlaF6_PlyrHasQuit;
      return true;
    }
    reenter_video_mode();

    display_loading_screen();

    short flgmem;
    switch (prev_state)
    {
    case FeSt_START_KPRLEVEL:
          my_player_number = default_loc_player;
          game.game_kind = GKind_LocalGame;
          clear_flag(game.system_flags, GSF_NetworkActive);
          player = get_my_player();
          player->is_active = 1;
          startup_network_game(&loop, true);
          break;
    case FeSt_START_MPLEVEL:
          set_flag(game.system_flags, GSF_NetworkActive);
          game.game_kind = GKind_MultiGame;
          player = get_my_player();
          player->is_active = 1;
          startup_network_game(&loop, false);
          break;
    case FeSt_LOAD_GAME:
          flgmem = game.save_game_slot;
          clear_flag(game.system_flags, GSF_NetworkActive);
          LbScreenClear(0);
          LbScreenSwap();
          if (!load_game(game.save_game_slot))
          {
              ERRORLOG("Loading game %d failed; quitting.",(int)game.save_game_slot);
              quit_game = 1;
          }
          game.save_game_slot = flgmem;
          break;
    case FeSt_PACKET_DEMO:
          game.mode_flags |= MFlg_IsDemoMode;
          startup_saved_packet_game();
          set_gui_visible(false);
          clear_flag(game.operation_flags, GOF_ShowPanel);
          break;
    }

    coroutine_add(&loop, &set_not_has_quit);
    coroutine_process(&loop);
    if (loop.error)
    {
        frontend_set_state(FeSt_INITIAL);
        return false;
    }
    return true;
}

void game_loop(void)
{
    unsigned long total_play_turns;
    unsigned long playtime;
    playtime = 0;
    total_play_turns = 0;
    SYNCDBG(0,"Entering gameplay loop.");

    while ( !exit_keeper )
    {
      update_mouse();
      while (!wait_at_frontend())
      {
          if (exit_keeper)
              break;
      }
      if ( exit_keeper )
        break;
      if (game.game_kind == GKind_LocalGame)
      {
        if (game.save_game_slot == -1)
        {
            if (is_feature_on(Ft_SkipHeartZoom) == false) {
                for (int i = 0; i < PLAYERS_COUNT; i++) {
                    struct PlayerInfo *player = get_player(i);
                    if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0)) {
                        set_player_instance(player, PI_HeartZoom, 0);
                    }
                }
            } else {
                if (!game.packet_load_enable) {
                    toggle_status_menu(1); // Required when skipping PI_HeartZoom
                }
            }
        } else
        {
          game.save_game_slot = -1;
          clear_flag(game.operation_flags, GOF_Paused);
        }
      } else {
          for (int i = 0; i < PLAYERS_COUNT; i++) {
              struct PlayerInfo *player = get_player(i);
              if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0)) {
                  set_player_instance(player, PI_HeartZoom, 0);
              }
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
      if (!TimerNoReset)
      {
        TimerFreeze = true;
        memset(&Timer, 0, sizeof(Timer));
      }
      LbScreenClear(0);
      LbScreenSwap();
      keeper_gameplay_loop();
      set_pointer_graphic_none();
      LbScreenClear(0);
      LbScreenSwap();
      stop_music();
      stop_streamed_samples();
      free_level_strings_data();
      turn_off_all_menus();
      delete_all_structures();
      clear_mapwho();
      endtime = LbTimerClock();
      quit_game = 0;
      if ((game.operation_flags & GOF_SingleLevel) != 0)
          exit_keeper=true;
      playtime += endtime-starttime;
      SYNCDBG(0,"Play time is %lu seconds",playtime>>10);
      total_play_turns += game.play_gameturn;
      reset_eye_lenses();
      close_packet_file();
      game.packet_load_enable = false;
      game.packet_save_enable = false;
    } // end while

    // Stop the movie recording if it's on
    if ((game.system_flags & GSF_CaptureMovie) != 0) {
        movie_record_stop();
    }
    ShutDownSDLAudio();
    SYNCDBG(7,"Done");
}

short reset_game(void)
{
    SYNCDBG(6,"Starting");

    KeeperSpeechExit();

    LbMouseSuspend();
    LbIKeyboardClose();
    LbScreenReset(false);
    LbDataFreeAllV2(game_load_files);
    free_gui_strings_data();
    free_level_strings_data();
    FreeAudio();
    return 1;
}

short process_command_line(unsigned short argc, char *argv[])
{
  char fullpath[CMDLN_MAXLEN+1];
  snprintf(fullpath, CMDLN_MAXLEN, "%s", argv[0]);
  snprintf(keeper_runtime_directory, sizeof(keeper_runtime_directory), "%s", fullpath);
  char *endpos = strrchr( keeper_runtime_directory, '\\');
  if (endpos==NULL)
      endpos=strrchr( keeper_runtime_directory, '/');
  if (endpos!=NULL)
      *endpos='\0';
  else
      strcpy(keeper_runtime_directory, ".");

  AssignCpuKeepers = 0;
  SoundDisabled = 0;
  // Note: the working log file is set up in LbBullfrogMain

  set_default_startup_parameters();

  short bad_param;
  LevelNumber level_num;
  bad_param = 0;
  unsigned short narg;
  level_num = LEVELNUMBER_ERROR;
  TbBool one_player_mode = 0;
  narg = 1;
  char bad_params[TEXT_BUFFER_LENGTH] = "\0";
  while ( narg < argc )
  {
      char *par;
      par = argv[narg];
      if ( (par == NULL) || ((par[0] != '-') && (par[0] != '/')) )
          return -1;
      char parstr[CMDLN_MAXLEN+1];
      char pr2str[CMDLN_MAXLEN+1];
      char pr3str[CMDLN_MAXLEN+1];
      snprintf(parstr, CMDLN_MAXLEN, "%s", par + 1);
      if (narg + 1 < argc)
      {
          snprintf(pr2str, CMDLN_MAXLEN, "%s", argv[narg + 1]);
          if (narg + 2 < argc)
              snprintf(pr3str, CMDLN_MAXLEN, "%s", argv[narg + 2]);
          else
              pr3str[0]='\0';
      }
      else
      {
          pr2str[0]='\0';
          pr3str[0]='\0';
      }

      if (strcasecmp(parstr, "nointro") == 0)
      {
        start_params.no_intro = true;
      } else
      if (strcasecmp(parstr, "nocd") == 0) // kept for legacy reasons
      {
          WARNLOG("The -nocd commandline parameter is no longer functional. Game music from CD is a setting in keeperfx.cfg instead.");
      } else
      if (strcasecmp(parstr, "columnconvert") == 0) //todo remove once it's no longer in the launcher
      {
          WARNLOG("The -%s commandline parameter is no longer functional.", parstr);
      }
      else
      if (strcasecmp(parstr, "cd") == 0)
      {
          start_params.overrides[Clo_CDMusic] = true;
      } else
      if (strcasecmp(parstr, "1player") == 0)
      {
          start_params.one_player = true;
          one_player_mode = true;
      } else
      if ((strcasecmp(parstr, "s") == 0) || (strcasecmp(parstr, "nosound") == 0))
      {
          SoundDisabled = true;
      } else
      if (strcasecmp(parstr, "fps") == 0)
      {
          narg++;
          start_params.num_fps = atoi(pr2str);
          start_params.overrides[Clo_GameTurns] = true;
      } else
      if (strcasecmp(parstr, "fps_draw") == 0)
      {
          narg++;
	  if (parse_draw_fps_config_val(pr2str, &start_params.num_fps_draw_main, &start_params.num_fps_draw_secondary) > 0)
            start_params.overrides[Clo_FramesPerSecond] = true;
      } else
      if (strcasecmp(parstr, "human") == 0)
      {
          narg++;
          default_loc_player = atoi(pr2str);
          force_player_num = true;
      } else
      if (strcasecmp(parstr, "vidsmooth") == 0)
      {
          smooth_on = true;
      } else
      if ( strcasecmp(parstr,"level") == 0 )
      {
        set_flag(start_params.operation_flags, GOF_SingleLevel);
        level_num = atoi(pr2str);
        autostart_multiplayer_level = atoi(pr2str);
        narg++;
      } else
      if ( strcasecmp(parstr,"campaign") == 0 )
      {
        strcpy(start_params.selected_campaign, pr2str);
        strcpy(autostart_multiplayer_campaign, pr2str);
        narg++;
      } else
      if ( strcasecmp(parstr,"altinput") == 0 )
      {
          SYNCLOG("Mouse auto reset disabled");
          lbMouseGrab = false;
      }
      else if (strcasecmp(parstr,"packetload") == 0)
      {
         if (start_params.packet_save_enable)
            WARNMSG("PacketSave disabled to enable PacketLoad.");
         start_params.packet_load_enable = true;
         start_params.packet_save_enable = false;
         snprintf(start_params.packet_fname, sizeof(start_params.packet_fname), "%s", pr2str);
         set_flag(start_params.debug_flags, DFlg_ShowGameTurns | DFlg_FrameStep);
         narg++;
      } else
      if (strcasecmp(parstr,"packetsave") == 0)
      {
         if (start_params.packet_load_enable)
            WARNMSG("PacketLoad disabled to enable PacketSave.");
         start_params.packet_load_enable = false;
         start_params.packet_save_enable = true;
         snprintf(start_params.packet_fname, sizeof(start_params.packet_fname), "%s", pr2str);
         narg++;
      } else
      if (strcasecmp(parstr,"pause_at_gameturn") == 0)
      {
         set_flag(start_params.debug_flags, DFlg_ShowGameTurns | DFlg_FrameStep | DFlg_PauseAtGameTurn);
         start_params.pause_at_gameturn = atoi(pr2str);
         narg++;
      } else
      if (strcasecmp(parstr,"q") == 0)
      {
         set_flag(start_params.operation_flags, GOF_SingleLevel);
      } else
      if (strcasecmp(parstr,"lightconvert") == 0)
      {
         WARNLOG("The -%s commandline parameter is no longer functional.", parstr); //todo remove once it's no longer in the launcher
      } else
      if (strcasecmp(parstr, "dbgshots") == 0)
      {
          set_flag(start_params.debug_flags, DFlg_ShotsDamage);
      } else
      if (strcasecmp(parstr, "dbgpathfind") == 0)
      {
          set_flag(start_params.debug_flags, DFlg_CreatrPaths);
      } else
      if (strcasecmp(parstr, "show_game_turns") == 0)
      {
          set_flag(start_params.debug_flags, DFlg_ShowGameTurns);
      } else
      if (strcasecmp(parstr, "mplog") == 0)
      {
          detailed_multiplayer_logging = true;
      } else
      if (strcasecmp(parstr, "compuchat") == 0)
      {
          if (strcasecmp(pr2str,"scarce") == 0) {
              start_params.computer_chat_flags = CChat_TasksScarce;
          } else
          if (strcasecmp(pr2str,"frequent") == 0) {
              start_params.computer_chat_flags = CChat_TasksScarce|CChat_TasksFrequent;
          } else {
              start_params.computer_chat_flags = CChat_None;
          }
          narg++;
      } else
      if (strcasecmp(parstr, "sessions") == 0) {
          narg++;
          LbNetwork_InitSessionsFromCmdLine(pr2str);
      } else
      if (strcasecmp(parstr, "nomods") == 0) {
          start_params.ignore_mods = true;
      } else
      if (strcasecmp(parstr,"alex") == 0)
      {
         start_params.easter_egg = true;
      }
      else if (strcasecmp(parstr,"connect") == 0)
      {
          narg++;
          LbNetwork_InitSessionsFromCmdLine(pr2str);
          game_flags2 |= GF2_Connect;
      }
      else if (strcasecmp(parstr,"server") == 0)
      {
          game_flags2 |= GF2_Server;
          int port = atoi(pr2str);
          if (port > 0)
          {
              LbNetwork_SetServerPort(port);
              narg++;
          }
      }
      else if (strcasecmp(parstr,"frameskip") == 0)
      {
         start_params.frame_skip = atoi(pr2str);
         narg++;
      } else
      if (strcasecmp(parstr,"framestep") == 0)
      {
         set_flag(start_params.debug_flags, DFlg_ShowGameTurns | DFlg_FrameStep);
      }
      else if (strcasecmp(parstr, "timer") == 0)
      {
          game_flags2 |= GF2_Timer;
          if (strcasecmp(pr2str, "game") == 0)
          {
              TimerGame = true;
          }
          else if (strcasecmp(pr2str, "continuous") == 0)
          {
              TimerNoReset = true;
          }
          narg++;
      }
      else if ( strcasecmp(parstr,"config") == 0 )
      {
        strcpy(start_params.config_file, pr2str);
        start_params.overrides[Clo_ConfigFile] = true;
        narg++;
      }
      else if ( strcasecmp(parstr,"Bullfrog") == 0 ) // force playing the Bullfrog video
      {
        set_flag(start_params.startup_flags, SFlg_Bullfrog);
      }
      else if ( strcasecmp(parstr,"ea") == 0 ) // force playing the EA video
      {
        set_flag(start_params.startup_flags, SFlg_EA);
      }
      else if (strcasecmp(parstr, "ftests") == 0)
      {
#ifdef FUNCTESTING
        if(ftest_parse_arg(pr2str)) // handle arg on ftest build
#else
        if(strlen(pr2str) > 0 && pr2str[0] != '-') // ignore arg on regular build
#endif // FUNCTESTING
        {
            ++narg;
        }

#ifdef FUNCTESTING
        set_flag(start_params.functest_flags, FTF_Enabled);
#else
        WARNLOG("Flag '%s' disabled for release builds.", parstr);
#endif // FUNCTESTING
      }
      else if (strcasecmp(parstr, "log") == 0)
      {
          narg++;
      }
      else if(strcasecmp(parstr, "exitonfailedtest") == 0)
      {
#ifdef FUNCTESTING
        set_flag(start_params.functest_flags, FTF_ExitOnTestFailure);
#else
       WARNLOG("Flag '%s' disabled for release builds.", parstr);
#endif // FUNCTESTING
      }
      else if(strcasecmp(parstr, "includelongtests") == 0)
      {
#ifdef FUNCTESTING
        set_flag(start_params.functest_flags, FTF_IncludeLongTests);
#else
       WARNLOG("Flag '%s' disabled for release builds.", parstr);
#endif // FUNCTESTING
      }
      else
      {
        // append bad parstr to bad_params string
        char param_buffer[128] = "";
        snprintf(param_buffer, sizeof(param_buffer), "%s%s", strnlen(bad_params, TEXT_BUFFER_LENGTH) > 0 ? ", " : "" , parstr);
        str_append(bad_params, sizeof(bad_params), param_buffer);
        bad_param=narg;
      }
      narg++;
  }

  if (level_num == LEVELNUMBER_ERROR)
  {
      if (first_singleplayer_level() > 0)
      {
          level_num = first_singleplayer_level();
      }
      else
      {
          level_num = 1;
      }
  }
  else {
      if (one_player_mode) {
          AssignCpuKeepers = 1;
      }
  }
  start_params.selected_level_number = level_num;
  my_player_number = default_loc_player;

#ifdef FUNCTESTING
  ftest_init(); // initialise test framework on ftest build
#endif

  if(bad_param != 0)
  {
    char message[TEXT_BUFFER_LENGTH];
    snprintf(message, sizeof(message), "Incorrect command line parameters: '%s'.\nPlease correct your Run options.", bad_params);
    warning_dialog(__func__, 0, message);
  }

  return (bad_param==0);
}

const char* determine_log_filename(unsigned short argument_count, char *argument_values[])
{
    for (int argument_index = 1; argument_index < argument_count; argument_index++) {
        if (argument_values[argument_index] && (argument_values[argument_index][0] == '-' || argument_values[argument_index][0] == '/')) {
            char* argument_name = argument_values[argument_index] + 1;
            if (strcasecmp(argument_name, "log") == 0 && argument_index + 1 < argument_count) {
                remove("keeperfx.log");
                return argument_values[argument_index + 1];
            }
        }
    }
    return log_file_name;
}

int LbBullfrogMain(unsigned short argc, char *argv[])
{
    short retval;
    retval=0;

    // Determine correct log file based on command line flags
    const char* selected_log_file_name = determine_log_filename(argc, argv);
    LbErrorLogSetup("/", selected_log_file_name, 5);

    retval = process_command_line(argc,argv);
    if (retval < 1)
    {
        LbErrorLogClose();
        return 0;
    }

    retval = true;
    retval &= (LbTimerInit() != Lb_FAIL);
    retval &= (LbScreenInitialize() != Lb_FAIL);
    LbSetTitle(PROGRAM_NAME);
    LbSetIcon(1);
    LbScreenSetDoubleBuffering(true);
    srand(LbTimerClock());

#ifdef FUNCTESTING
    ftest_srand();
#endif // FUNCTESTING

    if (!retval)
    {
        static const char *msg_text="Basic engine initialization failed.\n";
        error_dialog_fatal(__func__, 1, msg_text);
        LbErrorLogClose();
        return 0;
    }

    retval = setup_game();
    if (retval == 1)
    {
        steam_api_init();
    }
    if (retval == 1)
    {
      if ((install_info.lang_id == Lang_Japanese) ||
          (install_info.lang_id == Lang_ChineseInt) ||
          (install_info.lang_id == Lang_ChineseTra) ||
          (install_info.lang_id == Lang_Korean))
      {
        switch (install_info.lang_id)
        {
        case Lang_Japanese:
            dbc_set_language(1);
            break;
        case Lang_ChineseInt:
            dbc_set_language(2);
            break;
        case Lang_ChineseTra:
            dbc_set_language(3);
            break;
        case Lang_Korean:
            dbc_set_language(4);
            break;
        }
        if (dbc_initialize("fxdata"))
        {
          ERRORLOG("DBC fonts Initialization failed.");
        }
      }
    }
    if ( retval == 1 )
    {
        api_init_server();
        game_loop();
    }
    reset_game();
    LbScreenReset(true);
    if ( retval == 0 )
    {
        static const char *msg_text="Setting up game failed.\n";
        error_dialog_fatal(__func__, 2, msg_text);
    } else
    if (retval == -1)
    {
        static const char* msg_text = " Game files which have to be copied from original DK are not present.\n\n";
        error_dialog_fatal(__func__, 2, msg_text);
    }
    else
    {
        SYNCDBG(0,"finished properly");
    }

    LbErrorLogClose();
    steam_api_shutdown();
    return 0;
}

int kfxmain(int argc, char *argv[])
{
  try {
  LbBullfrogMain(argc, argv);
  } catch (...)
  {
      error_dialog(__func__, 1, "Exception raised!");
      return 1;
  }

#ifdef FUNCTESTING
  TbBool should_report_failure = flag_is_set(start_params.functest_flags, FTF_TestFailed) && flag_is_set(start_params.functest_flags, FTF_ExitOnTestFailure);
  if(flag_is_set(start_params.functest_flags, FTF_Enabled) && (flag_is_set(start_params.functest_flags, FTF_Abort) || should_report_failure))
  {
      return -1;
  }
#endif

  return 0;
}

void update_time(void)
{
    unsigned long time = ((unsigned long)LbTimerClock()) - timerstarttime;
    Timer.MSeconds = time % 1000;
    time /= 1000;
    Timer.Seconds = time % 60;
    time /= 60;
    Timer.Minutes = time % 60;
    Timer.Hours = time / 60;
}

struct GameTime get_game_time(unsigned long turns, unsigned long fps)
{
    struct GameTime GameT;
    unsigned long time = turns / fps;
    GameT.Seconds = time % 60;
    time /= 60;
    GameT.Minutes = time % 60;
    GameT.Hours = time / 60;
    return GameT;
}

#ifdef __cplusplus
}
#endif
