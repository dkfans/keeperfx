/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_instances.c
 *     Player instances support and switching code.
 * @par Purpose:
 *     Supports various states of a player, and switching between them.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_instances.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_sound.h"

#include "creature_control.h"
#include "creature_states.h"
#include "config_creature.h"
#include "thing_effects.h"
#include "front_simple.h"
#include "frontend.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
long pinstfs_hand_grab(struct PlayerInfo *player, long *n);
long pinstfm_hand_grab(struct PlayerInfo *player, long *n);
long pinstfe_hand_grab(struct PlayerInfo *player, long *n);
long pinstfs_hand_drop(struct PlayerInfo *player, long *n);
long pinstfe_hand_drop(struct PlayerInfo *player, long *n);
long pinstfs_hand_whip(struct PlayerInfo *player, long *n);
long pinstfe_hand_whip(struct PlayerInfo *player, long *n);
long pinstfm_hand_drop(struct PlayerInfo *player, long *n);
long pinstfs_hand_whip_end(struct PlayerInfo *player, long *n);
long pinstfe_hand_whip_end(struct PlayerInfo *player, long *n);
long pinstfs_control_creature(struct PlayerInfo *player, long *n);
long pinstfm_control_creature(struct PlayerInfo *player, long *n);
long pinstfe_direct_control_creature(struct PlayerInfo *player, long *n);
long pinstfe_passenger_control_creature(struct PlayerInfo *player, long *n);
long pinstfs_direct_leave_creature(struct PlayerInfo *player, long *n);
long pinstfm_leave_creature(struct PlayerInfo *player, long *n);
long pinstfs_passenger_leave_creature(struct PlayerInfo *player, long *n);
long pinstfe_leave_creature(struct PlayerInfo *player, long *n);
long pinstfs_query_creature(struct PlayerInfo *player, long *n);
long pinstfs_unquery_creature(struct PlayerInfo *player, long *n);
long pinstfs_zoom_to_heart(struct PlayerInfo *player, long *n);
long pinstfm_zoom_to_heart(struct PlayerInfo *player, long *n);
long pinstfe_zoom_to_heart(struct PlayerInfo *player, long *n);
long pinstfs_zoom_out_of_heart(struct PlayerInfo *player, long *n);
long pinstfm_zoom_out_of_heart(struct PlayerInfo *player, long *n);
long pinstfe_zoom_out_of_heart(struct PlayerInfo *player, long *n);
long pinstfm_control_creature_fade(struct PlayerInfo *player, long *n);
long pinstfe_control_creature_fade(struct PlayerInfo *player, long *n);
long pinstfs_fade_to_map(struct PlayerInfo *player, long *n);
long pinstfm_fade_to_map(struct PlayerInfo *player, long *n);
long pinstfe_fade_to_map(struct PlayerInfo *player, long *n);
long pinstfs_fade_from_map(struct PlayerInfo *player, long *n);
long pinstfm_fade_from_map(struct PlayerInfo *player, long *n);
long pinstfe_fade_from_map(struct PlayerInfo *player, long *n);
long pinstfs_zoom_to_position(struct PlayerInfo *player, long *n);
long pinstfm_zoom_to_position(struct PlayerInfo *player, long *n);
long pinstfe_zoom_to_position(struct PlayerInfo *player, long *n);

struct PlayerInstanceInfo player_instance_info[] = {
  { 0, 0, NULL,                      NULL,                      NULL,                               {0}, {0}, 0, 0},
  { 3, 1, pinstfs_hand_grab,         pinstfm_hand_grab,         pinstfe_hand_grab,                  {0}, {0}, 0, 0},
  { 3, 1, pinstfs_hand_drop,         pinstfm_hand_drop,         pinstfe_hand_drop,                  {0}, {0}, 0, 0},
  { 4, 0, pinstfs_hand_whip,         NULL,                      pinstfe_hand_whip,                  {0}, {0}, 0, 0},
  { 5, 0, pinstfs_hand_whip_end,     NULL,                      pinstfe_hand_whip_end,              {0}, {0}, 0, 0},
  {12, 1, pinstfs_control_creature,  pinstfm_control_creature,  pinstfe_direct_control_creature,    {0}, {0}, 0, 0},
  {12, 1, pinstfs_control_creature,  pinstfm_control_creature,  pinstfe_passenger_control_creature, {0}, {0}, 0, 0},
  {12, 1, pinstfs_direct_leave_creature,pinstfm_leave_creature, pinstfe_leave_creature,             {0}, {0}, 0, 0},
  {12, 1, pinstfs_passenger_leave_creature,pinstfm_leave_creature,pinstfe_leave_creature,           {0}, {0}, 0, 0},
  { 0, 1, pinstfs_query_creature,    NULL,                      NULL,                               {0}, {0}, 0, 0},
  { 0, 1, pinstfs_unquery_creature,  NULL,                      NULL,                               {0}, {0}, 0, 0},
  {16, 1, pinstfs_zoom_to_heart,     pinstfm_zoom_to_heart,     pinstfe_zoom_to_heart,              {0}, {0}, 0, 0},
  {16, 1, pinstfs_zoom_out_of_heart, pinstfm_zoom_out_of_heart, pinstfe_zoom_out_of_heart,          {0}, {0}, 0, 0},
  {12, 1, NULL,                      pinstfm_control_creature_fade,pinstfe_control_creature_fade,   {0}, {0}, 0, 0},
  { 8, 1, pinstfs_fade_to_map,       pinstfm_fade_to_map,       pinstfe_fade_to_map,                {0}, {0}, 0, 0},
  { 8, 1, pinstfs_fade_from_map,     pinstfm_fade_from_map,     pinstfe_fade_from_map,              {0}, {0}, 0, 0},
  {-1, 1, pinstfs_zoom_to_position,  pinstfm_zoom_to_position,  pinstfe_zoom_to_position,           {0}, {0}, 0, 0},
  { 0, 0, NULL,                      NULL,                      NULL,                               {0}, {0}, 0, 0},
  { 0, 0, NULL,                      NULL,                      NULL,                               {0}, {0}, 0, 0},
};

/******************************************************************************/

DLLIMPORT long _DK_pinstfs_hand_grab(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_hand_grab(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_hand_grab(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_hand_drop(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_hand_drop(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_hand_whip(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_hand_whip(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_hand_drop(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_hand_whip_end(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_hand_whip_end(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_control_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_control_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_direct_control_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_passenger_control_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_direct_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_passenger_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_query_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_unquery_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_zoom_to_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_zoom_to_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_zoom_to_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_zoom_out_of_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_zoom_out_of_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_zoom_out_of_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_control_creature_fade(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_control_creature_fade(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_fade_to_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_fade_to_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_fade_to_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_fade_from_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_fade_from_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_fade_from_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_zoom_to_position(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_zoom_to_position(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_zoom_to_position(struct PlayerInfo *player, long *n);
DLLIMPORT void _DK_process_player_instance(struct PlayerInfo *player);
DLLIMPORT void _DK_process_player_instances(void);
DLLIMPORT void _DK_set_player_instance(struct PlayerInfo *playerinf, long, int);
DLLIMPORT void _DK_leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);

/******************************************************************************/
long pinstfs_hand_grab(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  struct Dungeon *dungeon;
  //return _DK_pinstfs_hand_grab(player, n);
  dungeon = get_players_dungeon(player);
  thing = thing_get(player->field_43A);
  if (dungeon->field_63)
  {
    dungeon->field_43 = 60;
    dungeon->field_53 = 40;
  }
  if (!thing_is_invalid(thing))
    set_power_hand_graphic(player->id_number, 783, 256);
  return 0;

}

long pinstfm_hand_grab(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_hand_grab(player, n);
  struct Thing *thing;
  struct Dungeon *dungeon;
  //return _DK_pinstfs_hand_grab(player, n);
  dungeon = get_players_dungeon(player);
  thing = thing_get(player->field_43E);
  if (thing->class_id == TCls_Creature)
  {
    dungeon->field_43 += (creature_picked_up_offset[thing->model].field_4 - 60) / 4;
    dungeon->field_53 += (creature_picked_up_offset[thing->model].field_6 - 40) / 4;
    return 0;
  }
  else
  {
    dungeon->field_43 = 60;
    dungeon->field_53 = 40;
    return 0;
  }
}

long pinstfe_hand_grab(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_hand_grab(player, n);
  struct Thing *dsttng;
  struct Thing *grabtng;
  struct CreatureControl *cctrl;
  long i;
  SYNCDBG(8,"Starting");
  dsttng = thing_get(player->field_43E);
  grabtng = thing_get(player->field_43A);
  if (!thing_is_pickable_by_hand(player,dsttng))
  {
    player->field_440 = 0;
    player->field_43E = 0;
    return 0;
  }
  set_power_hand_offset(player, dsttng);
  switch (dsttng->class_id)
  {
  case TCls_Creature:
      if (!external_set_thing_state(dsttng, 38))
        return 0;
      cctrl = creature_control_get_from_thing(dsttng);
      if (cctrl->field_AD & 0x02)
        i = convert_td_iso(122);
      else
        i = get_creature_anim(dsttng, 9);
      set_thing_draw(dsttng, i, 256, -1, -1, 0, 2);
      break;
  case TCls_Object:
      dsttng = process_object_being_picked_up(dsttng, grabtng->owner);
      if (thing_is_invalid(dsttng))
      {
        player->field_440 = 0;
        player->field_43E = 0;
        return 0;
      }
      break;
  }
  if (!thing_is_invalid(grabtng))
    set_power_hand_graphic(player->id_number, 784, 256);
  dump_thing_in_power_hand(dsttng, player->id_number);
  player->field_440 = 0;
  player->field_43E = 0;
  place_thing_in_limbo(dsttng);
  return 0;
}

long pinstfs_hand_drop(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  struct Dungeon *dungeon;
  //return _DK_pinstfs_hand_drop(player, n);
  dungeon = get_players_dungeon(player);
  thing = thing_get(player->field_43A);
  player->field_43E = dungeon->things_in_hand[0];
  if (!thing_is_invalid(thing))
    set_power_hand_graphic(player->id_number, 783, -256);
  return 0;

}

long pinstfe_hand_drop(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  struct Dungeon *dungeon;
  //return _DK_pinstfe_hand_drop(player, n);
  dungeon = get_players_dungeon(player);
  thing = thing_get(player->field_43A);
  dungeon->field_43 = 60;
  dungeon->field_53 = 40;
  if (!thing_is_invalid(thing))
    set_power_hand_graphic(player->id_number, 782, 256);
  player->field_43E = 0;
  return 0;

}

long pinstfs_hand_whip(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  //return _DK_pinstfs_hand_whip(player, n);
  thing = thing_get(player->field_43A);
  if (!thing_is_invalid(thing))
    set_power_hand_graphic(player->id_number, 786, 256);
  return 0;
}

long pinstfe_hand_whip(struct PlayerInfo *player, long *n)
{
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  struct Thing *efftng;
  struct Thing *thing;
  struct Camera *cam;
  struct Coord3d pos;
  //return _DK_pinstfe_hand_whip(player, n);

  thing = thing_get(player->field_43E);
  if (((thing->field_0 & 0x01) == 0) || (thing->field_9 != player->field_440) || (!thing_slappable(thing, player->id_number)))
  {
    player->field_440 = 0;
    player->field_43E = 0;
    return 0;
  }
  switch (thing->class_id)
  {
  case TCls_Creature:
      cctrl = creature_control_get_from_thing(thing);
      if ((cctrl->field_AB & 0x02) != 0)
      {
        kill_creature(thing, 0, thing->owner, 0, 0, 0);
      } else
      {
          slap_creature(player, thing);
          pos.x.val = thing->mappos.x.val;
          pos.y.val = thing->mappos.y.val;
          pos.z.val = thing->mappos.z.val + (thing->field_58 >> 1);
          crstat = creature_stats_get_from_thing(thing);
          if ( creature_model_bleeds(thing->model) )
            create_effect(&pos, 6, thing->owner);
          thing_play_sample(thing, 75, 100, 0, 3, 0, 3, 256);
          cam = player->acamera;
          if (cam != NULL)
          {
            thing->pos_2C.x.val += LbSinL(cam->orient_a) << 6 >> 16;
            thing->pos_2C.y.val += -(LbCosL(cam->orient_a) << 6 >> 8) >> 8;
          }
      }
      break;
  case 2:
      if (thing->model == 15)
      {
        thing->field_52 = player->acamera->orient_a;
        thing->health -= game.boulder_reduce_health_slap;
      } else
      if (thing->model == 20)
      {
        thing->field_52 = player->acamera->orient_a;
      }
      break;
  case 8:
      if (thing->model == 1)
        external_activate_trap_shot_at_angle(thing, player->acamera->orient_a);
      break;
  case TCls_Object:
      if (object_is_slappable(thing, player->id_number))
      {
        efftng = create_effect(&thing->mappos, 49, thing->owner);
        if (!thing_is_invalid(efftng))
          thing_play_sample(efftng, 75, 100, 0, 3, 0, 3, 256);
        slap_object(thing);
      }
      break;
  }
  set_player_instance(player, 4, false);
  return 0;
}

long pinstfm_hand_drop(struct PlayerInfo *player, long *n)
{
  struct Dungeon *dungeon;
  long i;
  //return _DK_pinstfm_hand_drop(player, n);
  dungeon = get_players_dungeon(player);
  i = player->field_4B1+1;
  if (i < 1) i = 1;
  dungeon->field_43 += (60 - dungeon->field_43) / i;
  dungeon->field_53 += (40 - dungeon->field_53) / i;
  return 0;
}

long pinstfs_hand_whip_end(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  //return _DK_pinstfs_hand_whip_end(player, n);
  thing = thing_get(player->field_43A);
  if (!thing_is_invalid(thing))
    set_power_hand_graphic(player->id_number, 787, 256);
  return 0;
}

long pinstfe_hand_whip_end(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  //return _DK_pinstfe_hand_whip_end(player, n);
  thing = thing_get(player->field_43A);
  if (!thing_is_invalid(thing))
    set_power_hand_graphic(player->id_number, 785, 256);
  return 0;
}

long pinstfs_control_creature(struct PlayerInfo *player, long *n)
{
  struct Camera *cam;
  //return _DK_pinstfs_control_creature(player, n);
  player->field_0 |= 0x80;
  if (is_my_player(player))
  {
    player->field_4C5 = 1;
    turn_off_all_window_menus();
    turn_off_menu(31);
    turn_off_menu(35);
    game.field_15038E = 0;
    game.flags_font |= FFlg_unk04;
  }
  cam = player->acamera;
  player->field_0 |= 0x10;
  player->field_4B6 = get_camera_zoom(cam);
  if (is_my_player(player))
    play_non_3d_sample(39);
  return 0;
}

long pinstfm_control_creature(struct PlayerInfo *player, long *n)
{
    struct CreatureStats *crstat;
    struct Thing *thing;
    struct Camera *cam;
    long mv_x,mv_y,mv_a;
//    return _DK_pinstfm_control_creature(player, n);
    cam = player->acamera;
    if (cam == NULL)
        return 0;
    thing = thing_get(player->field_43E);
    if (thing_is_invalid(thing) || (thing->class_id == 4) || (thing->health < 0))
    {
        set_camera_zoom(cam, player->field_4B6);
        if (is_my_player(player))
            PaletteSetPlayerPalette(player, _DK_palette);
        player->field_43E = 0;
        player->field_0 &= 0xEF;
        player->field_0 &= 0x7F;
        set_player_instance(player, 0, true);
        return 0;
    }
    if (player->field_37 != 5)
    {
        view_zoom_camera_in(cam, 30000, 6000);
        // Compute new camera angle
        mv_a = (thing->field_52 - cam->orient_a) % ANGLE_TRIGL_PERIOD;
        if (mv_a > ANGLE_TRIGL_PERIOD/2)
          mv_a -= ANGLE_TRIGL_PERIOD;
        if (mv_a < -170)
        {
            mv_a = -170;
        } else
        if (mv_a > 170)
        {
            mv_a = 170;
        }
        cam->orient_a += mv_a;
        cam->orient_a %= ANGLE_TRIGL_PERIOD;
        thing = thing_get(player->field_43E);
        crstat = creature_stats_get_from_thing(thing);
        // Now mv_a becomes a circle radius
        mv_a = crstat->eye_height + thing->mappos.z.val;
        mv_x = thing->mappos.x.val + (mv_a * LbSinL(cam->orient_a) >> 16) - cam->mappos.x.val;
        mv_y = thing->mappos.y.val - (mv_a * LbCosL(cam->orient_a) >> 16) - cam->mappos.y.val;
        if (mv_x < -128)
        {
            mv_x = -128;
        } else
        if (mv_x > 128)
        {
            mv_x = 128;
        }
        if (mv_y < -128)
        {
            mv_y = -128;
        } else
        if (mv_y > 128)
        {
            mv_y = 128;
        }
        cam->mappos.x.val += mv_x;
        cam->mappos.y.val += mv_y;
        if (cam->orient_a < 0)
        {
          cam->orient_a += ANGLE_TRIGL_PERIOD;
        }
        if (cam->orient_a >= ANGLE_TRIGL_PERIOD)
        {
          cam->orient_a -= ANGLE_TRIGL_PERIOD;
        }
    }
    return 0;
}

long pinstfe_direct_control_creature(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_direct_control_creature(player, n);
  struct Thing *thing;
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  long i,k;
  thing = thing_get(player->field_43E);
  if (thing_is_invalid(thing))
    thing = NULL;
  if (thing != NULL)
  {
    if (!control_creature_as_controller(player, thing))
      thing = NULL;
  }
  if (thing == NULL)
  {
    set_camera_zoom(player->acamera, player->field_4B6);
    if (is_my_player(player))
      PaletteSetPlayerPalette(player, _DK_palette);
    player->field_0 &= 0xEF;
    player->field_0 &= 0x7F;
    return 0;
  }
  set_player_instance(player, 13, false);
  if (thing->class_id == TCls_Creature)
  {
    load_swipe_graphic_for_creature(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (is_my_player(player))
    {
      if (cctrl->field_AB & 0x02)
        PaletteSetPlayerPalette(player, blue_palette);
    }
    crstat = creature_stats_get_from_thing(thing);
    for (i=0; i < 10; i++)
    {
      k = crstat->instance_spell[i];
      if (cctrl->instances[k])
      {
        cctrl->field_1E8 = k;
        break;
      }
    }
  }
  if (is_my_player(player))
    turn_on_menu(31);
  return 0;
}

long pinstfe_passenger_control_creature(struct PlayerInfo *player, long *n)
{
//  return _DK_pinstfe_passenger_control_creature(player, n);
  struct Thing *thing;
  thing = thing_get(player->field_43E);
  if (!thing_is_invalid(thing))
    control_creature_as_passenger(player, thing);
  set_player_instance(player, 13, false);
  return 0;

}

long pinstfs_direct_leave_creature(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  //return _DK_pinstfs_direct_leave_creature(player, n);
  if (player->field_43E == 0)
  {
    set_player_instance(player, 0, true);
    return 0;
  }
  player->field_0 |= 0x80;
  thing = thing_get(player->field_43E);
  reset_creature_eye_lens(thing);
  if (is_my_player(player))
  {
      PaletteSetPlayerPalette(player, _DK_palette);
      player->field_4C5 = 11;
      turn_off_all_window_menus();
      turn_off_menu(31);
      turn_off_menu(35);
      turn_off_menu(32);
      turn_on_main_panel_menu();
      set_flag_byte(&game.numfield_C, 0x40, (game.numfield_C & 0x20) != 0);
  }
  thing = thing_get(player->field_43E);
  leave_creature_as_controller(player, thing);
  player->field_0 |= 0x10;
  player->field_43E = 0;
  light_turn_light_on(player->field_460);
  play_non_3d_sample(177);
  return 0;
}

long pinstfm_leave_creature(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfm_leave_creature(player, n);
  if (player->field_37 != 5)
  {
    view_zoom_camera_out(player->acamera, 30000, 6000);
    if (get_camera_zoom(player->acamera) < player->field_4B6)
      set_camera_zoom(player->acamera, player->field_4B6);
  }
  return 0;
}

long pinstfs_passenger_leave_creature(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  //return _DK_pinstfs_passenger_leave_creature(player, n);
  if (player->field_43E == 0)
  {
      set_player_instance(player, 0, true);
      return 0;
  }
  player->field_0 |= 0x80;
  thing = thing_get(player->field_43E);
  reset_creature_eye_lens(thing);
  if (is_my_player(player))
  {
    PaletteSetPlayerPalette(player, _DK_palette);
    player->field_4C5 = 11;
    turn_off_all_window_menus();
    turn_off_menu(31);
    turn_off_menu(35);
    turn_off_menu(32);
    turn_on_main_panel_menu();
    set_flag_byte(&game.numfield_C, 0x40, (game.numfield_C & 0x20) != 0);
  }
  leave_creature_as_passenger(player, thing);
  player->field_0 |= 0x10;
  player->field_43E = 0;
  light_turn_light_on(player->field_460);
  play_non_3d_sample(177);
  return 0;
}

long pinstfe_leave_creature(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_leave_creature(player, n);
  set_camera_zoom(player->acamera, player->field_4B6);
  if (is_my_player(player))
    PaletteSetPlayerPalette(player, _DK_palette);
  player->field_0 &= 0xEF;
  player->field_0 &= 0x7F;
  return 0;
}

long pinstfs_query_creature(struct PlayerInfo *player, long *n)
{
  struct Thing *thing;
  //return _DK_pinstfs_query_creature(player, n);
  thing = thing_get(player->field_43E);
  player->field_4B6 = get_camera_zoom(player->acamera);
  set_selected_creature(player, thing);
  set_player_state(player, 15, 0);
  return 0;
}

long pinstfs_unquery_creature(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfs_unquery_creature(player, n);
  set_player_state(player, 1, 0);
  player->field_31 = 0;
  player->field_2F = 0;
  return 0;
}

long pinstfs_zoom_to_heart(struct PlayerInfo *player, long *n)
{
  struct CreatureControl *cctrl;
  struct Dungeon *dungeon;
  struct Thing *thing;
  struct Coord3d mappos;
  //return _DK_pinstfs_zoom_to_heart(player, n);
  memset(zoom_to_heart_palette, 0x3F, sizeof(zoom_to_heart_palette));
  light_turn_light_off(player->field_460);
  dungeon = get_players_dungeon(player);
  thing = thing_get(dungeon->dnheart_idx);
  mappos.x.val = thing->mappos.x.val;
  mappos.y.val = thing->mappos.y.val + 1792;
  mappos.z.val = thing->mappos.z.val + 256;
  thing = create_and_control_creature_as_controller(player, 31, &mappos);
  if (!thing_is_invalid(thing))
  {
    cctrl = creature_control_get_from_thing(thing);
    cctrl->flgfield_1 |= 0x02;
    player->field_0 |= 0x10;
    player->field_0 |= 0x80;
    game.numfield_D |= 0x08;
  }
  return 0;
}

long pinstfm_zoom_to_heart(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfm_zoom_to_heart(player, n);
  struct Thing *thing;
  struct Coord3d pos;
  thing = thing_get(player->field_2F);
  if (!thing_is_invalid(thing))
  {
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val - 112;
    pos.z.val = thing->mappos.z.val;
    move_thing_in_map(thing, &pos);
  }
  if (player->field_4B1 <= 8)
    LbPaletteFade(zoom_to_heart_palette, 8, Lb_PALETTE_FADE_OPEN);
  return 0;
}


long pinstfe_zoom_to_heart(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_zoom_to_heart(player, n);
  set_player_instance(player, 12, false);
  LbPaletteStopOpenFade();
  return 0;
}

long pinstfs_zoom_out_of_heart(struct PlayerInfo *player, long *n)
{
  struct Dungeon *dungeon;
  struct Thing *thing;
  struct Camera *cam;
  //return _DK_pinstfs_zoom_out_of_heart(player, n);
  thing = thing_get(player->field_2F);
  if (!thing_is_invalid(thing))
    leave_creature_as_controller(player, thing);
  set_player_mode(player, 1);
  cam = player->acamera;
  if (cam == NULL) return 0;
  dungeon = get_players_dungeon(player);
  thing = thing_get(dungeon->dnheart_idx);
  if (thing_is_invalid(thing))
  {
    cam->mappos.x.val = (map_subtiles_x << 8)/2;
    cam->mappos.y.val = (map_subtiles_y << 8)/2;
    cam->field_17 = 24000;
    cam->orient_a = 0;
    return 0;
  }
  cam->mappos.x.val = thing->mappos.x.val;
  if (player->field_37 == 5)
  {
    cam->mappos.y.val = thing->mappos.y.val;
    cam->field_17 = 65536;
  } else
  {
    cam->mappos.y.val = thing->mappos.y.val - (thing->field_58 >> 1) -  thing->mappos.z.val;
    cam->field_17 = 24000;
  }
  cam->orient_a = 0;
  return 0;
}

long pinstfm_zoom_out_of_heart(struct PlayerInfo *player, long *n)
{
  struct Dungeon *dungeon;
  struct Thing *thing;
  struct Camera *dstcam;
  struct Camera *cam;
  unsigned long deltax,deltay;
  unsigned long addval;
  //return _DK_pinstfm_zoom_out_of_heart(player, n);
  if (player->field_37 != 5)
  {
    cam = player->acamera;
    dungeon = get_players_dungeon(player);
    thing = thing_get(dungeon->dnheart_idx);
    if (cam != NULL)
    {
      cam->field_17 -= 988;
      cam->orient_a += 16;
      addval = (thing->field_58 >> 1);
      deltax = (LbSinL(cam->orient_a) * (thing->mappos.z.val+addval) >> 16);
      deltay = (LbCosL(cam->orient_a) * (thing->mappos.z.val+addval) >> 16);
    } else
    {
      addval = (thing->field_58 >> 1);
      deltax = thing->mappos.z.val+addval;
      deltay = thing->mappos.z.val+addval;
    }
    dstcam = &player->cameras[0];
    dstcam->mappos.x.val = thing->mappos.x.val + deltax;
    dstcam->mappos.y.val = thing->mappos.y.val - deltay;
    dstcam = &player->cameras[3];
    dstcam->mappos.x.val = thing->mappos.x.val + deltax;
    dstcam->mappos.y.val = thing->mappos.y.val - deltay;
  }
  if (player->field_4B1 >= 8)
    LbPaletteFade(_DK_palette, 8, Lb_PALETTE_FADE_OPEN);
  return 0;

}

long pinstfe_zoom_out_of_heart(struct PlayerInfo *player, long *n)
{
  struct Camera *cam;
  //return _DK_pinstfe_zoom_out_of_heart(player, n);
  LbPaletteStopOpenFade();
  cam = player->acamera;
  if ((player->field_37 != 5) && (cam != NULL))
  {
    cam->field_17 = 8192;
    cam->orient_a = 256;
  }
  light_turn_light_on(player->field_460);
  player->field_0 &= 0xEF;
  player->field_0 &= 0x7F;
  game.numfield_D &= 0xF7;
  if (is_my_player(player))
    PaletteSetPlayerPalette(player, _DK_palette);
  return 0;
}

long pinstfm_control_creature_fade(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfm_control_creature_fade(player, n);
  player->field_0 |= 0x80;
  return 0;
}

long pinstfe_control_creature_fade(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_control_creature_fade(player, n);
  if (is_my_player(player))
  {
    if ((player->field_3 & 0x04) == 0)
      PaletteSetPlayerPalette(player, _DK_blue_palette);
    else
      PaletteSetPlayerPalette(player, _DK_palette);
  }
  player->field_0 &= 0xEF;
  light_turn_light_off(player->field_460);
  player->field_0 &= 0x7F;
  return 0;
}

long pinstfs_fade_to_map(struct PlayerInfo *player, long *n)
{
  struct Camera *cam;
  //return _DK_pinstfs_fade_to_map(player, n);
  cam = player->acamera;
  player->field_4BD = 0;
  player->field_0 |= 0x80;
  player->field_4B5 = cam->field_6;
  if (is_my_player(player))
  {
    set_flag_byte(&player->field_1, 0x02, settings.tooltips_on);
    settings.tooltips_on = 0;
    set_flag_byte(&player->field_1, 0x01, toggle_status_menu(0));
  }
  set_engine_view(player, 6);
  return 0;

}

long pinstfm_fade_to_map(struct PlayerInfo *player, long *n)
{
  return 0;
}

long pinstfe_fade_to_map(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_fade_to_map(player, n);
  set_player_mode(player, 4);
  if (is_my_player(player))
    settings.tooltips_on = ((player->field_1 & 0x02) != 0);
  player->field_0 &= 0x7Fu;
  return 0;
}

long pinstfs_fade_from_map(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfs_fade_from_map(player, n);
  player->field_0 |= 0x80u;
  if (is_my_player(player))
  {
    set_flag_byte(&player->field_1, 0x02, settings.tooltips_on);
    settings.tooltips_on = 0;
    game.numfield_C &= 0xFFBF;
  }
  player->field_4BD = 32;
  set_player_mode(player, 1);
  set_engine_view(player, 7);
  return 0;
}

long pinstfm_fade_from_map(struct PlayerInfo *player, long *n)
{
  return 0;
}

long pinstfe_fade_from_map(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_fade_from_map(player, n);
}

long pinstfs_zoom_to_position(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_zoom_to_position(player, n);
}

long pinstfm_zoom_to_position(struct PlayerInfo *player, long *n)
{
  struct Camera *cam;
  long x,y;
  //return _DK_pinstfm_zoom_to_position(player, n);
  cam = player->acamera;
  if (abs(cam->mappos.x.val - player->field_E4) >= abs(player->field_4DB))
    x = player->field_4DB + cam->mappos.x.val;
  else
    x = player->field_E4;
  if (abs(cam->mappos.y.val - player->field_E6) >= abs(player->field_4DF))
    y = player->field_4DF + cam->mappos.y.val;
  else
    y = player->field_E6;
  if ((player->field_E4 == x) && (player->field_E6 == y))
      player->field_4B1 = 0;
  cam->mappos.x.val = x;
  cam->mappos.y.val = y;
  return 0;
}

long pinstfe_zoom_to_position(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_zoom_to_position(player, n);
  player->field_0 &= 0x7F;
  player->field_0 &= 0xEF;
  return 0;
}

void set_player_instance(struct PlayerInfo *player, long ninum, TbBool force)
{
  struct PlayerInstanceInfo *inst_info;
  InstncInfo_Func callback;
  long inum;
  inum = player->instance_num;
  if (inum >= PLAYER_INSTANCES_COUNT)
    inum = 0;
  if ((inum == 0) || (player_instance_info[inum].field_4 != 1) || (force))
  {
    player->instance_num = ninum%PLAYER_INSTANCES_COUNT;
    inst_info = &player_instance_info[player->instance_num];
    player->field_4B1 = inst_info->field_0;
    callback = inst_info->start_cb;
    if (callback != NULL)
      callback(player, &inst_info->field_14[0]);
  }
}

void process_player_instance(struct PlayerInfo *player)
{
  struct PlayerInstanceInfo *inst_info;
  InstncInfo_Func callback;
  SYNCDBG(6,"Starting for instance %d",player->instance_num);
  //_DK_process_player_instance(player); return;
  if (player->instance_num > 0)
  {
    if (player->field_4B1 > 0)
    {
      player->field_4B1--;
      inst_info = &player_instance_info[player->instance_num%PLAYER_INSTANCES_COUNT];
      callback = inst_info->maintain_cb;
      if (callback != NULL)
        callback(player, &inst_info->field_24);
    }
    if (player->field_4B1 == 0)
    {
      inst_info = &player_instance_info[player->instance_num%PLAYER_INSTANCES_COUNT];
      player->instance_num = 0;
      callback = inst_info->end_cb;
      if (callback != NULL)
        callback(player, &inst_info->field_24);
    }
  }
}

void process_player_instances(void)
{
  //_DK_process_player_instances();return;
  int i;
  struct PlayerInfo *player;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if (player->field_0 & 0x01)
      process_player_instance(player);
  }
  SYNCDBG(9,"Finished");
}

void leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  long i,k;
  SYNCDBG(7,"Starting");
  //_DK_leave_creature_as_controller(player, thing);
  if ((thing->owner != player->id_number) || (thing->index != player->field_2F))
  {
    set_player_instance(player, 0, 1);
    player->field_2F = 0;
    player->field_31 = 0;
    set_player_mode(player, 1);
    player->field_0 &= 0xF7;
    set_engine_view(player, player->field_4B5);
    player->cameras[0].mappos.x.val = 0;
    player->cameras[0].mappos.y.val = 0;
    player->cameras[3].mappos.x.val = 0;
    player->cameras[3].mappos.y.val = 0;
    return;
  }
  player->field_2F = 0;
  player->field_31 = 0;
  set_player_mode(player, 1);
  thing->field_0 &= 0xDF;
  thing->field_4F &= 0xFE;
  player->field_0 &= 0xF7;
  set_engine_view(player, player->field_4B5);
  i = player->acamera->orient_a;
  crstat = creature_stats_get_from_thing(thing);
  k = thing->mappos.z.val + crstat->eye_height;
  player->cameras[0].mappos.x.val = thing->mappos.x.val + ((LbSinL(i) * k) >> 16);
  player->cameras[0].mappos.y.val = thing->mappos.y.val - ((LbCosL(i) * k) >> 16);
  player->cameras[3].mappos.x.val = thing->mappos.x.val + ((LbSinL(i) * k) >> 16);
  player->cameras[3].mappos.y.val = thing->mappos.y.val - ((LbCosL(i) * k) >> 16);
  if (thing->class_id == TCls_Creature)
  {
    set_start_state(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
    if ((cctrl->field_2 & 0x02) != 0)
      delete_thing_structure(thing, 0);
    else
      disband_creatures_group(thing);
  }
  if (thing->field_62 != 0)
  {
    light_delete_light(thing->field_62);
    thing->field_62 = 0;
  }
}

void leave_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing)
{
  struct CreatureStats *crstat;
  long i,k;
  SYNCDBG(7,"Starting");
  if ((thing->owner != player->id_number) || (thing->index != player->field_2F))
  {
    set_player_instance(player, 0, 1);
    player->field_2F = 0;
    player->field_31 = 0;
    set_player_mode(player, 1);
    player->field_0 &= 0xF7;
    set_engine_view(player, player->field_4B5);
    player->cameras[0].mappos.x.val = 0;
    player->cameras[0].mappos.y.val = 0;
    player->cameras[3].mappos.x.val = 0;
    player->cameras[3].mappos.y.val = 0;
    return;
  }
  set_player_mode(player, 1);
  thing->field_4F &= 0xFE;
  player->field_0 &= 0xF7;
  set_engine_view(player, player->field_4B5);
  i = player->acamera->orient_a;
  crstat = creature_stats_get_from_thing(thing);
  k = thing->mappos.z.val + crstat->eye_height;
  player->cameras[0].mappos.x.val = thing->mappos.x.val + ((LbSinL(i) * k) >> 16);
  player->cameras[0].mappos.y.val = thing->mappos.y.val - ((LbCosL(i) * k) >> 16);
  player->cameras[3].mappos.x.val = thing->mappos.x.val + ((LbSinL(i) * k) >> 16);
  player->cameras[3].mappos.y.val = thing->mappos.y.val - ((LbCosL(i) * k) >> 16);
  player->field_2F = 0;
  player->field_31 = 0;
}

TbBool set_selected_creature(struct PlayerInfo *player, struct Thing *thing)
{
  if (thing->class_id == TCls_Creature)
  {
    player->field_2F = thing->index;
    return true;
  }
  ERRORLOG("Cannot select thing for information");
  return false;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
