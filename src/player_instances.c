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

#include "front_simple.h"
#include "frontend.h"
#include "keeperfx.h"

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
  { 0, 0, NULL,                      NULL,                      NULL,                               0, 0, 0},
  { 3, 1, pinstfs_hand_grab,         pinstfm_hand_grab,         pinstfe_hand_grab,                  0, 0, 0},
  { 3, 1, pinstfs_hand_drop,         pinstfm_hand_drop,         pinstfe_hand_drop,                  0, 0, 0},
  { 4, 0, pinstfs_hand_whip,         NULL,                      pinstfe_hand_whip,                  0, 0, 0},
  { 5, 0, pinstfs_hand_whip_end,     NULL,                      pinstfe_hand_whip_end,              0, 0, 0},
  {12, 1, pinstfs_control_creature,  pinstfm_control_creature,  pinstfe_direct_control_creature,    0, 0, 0},
  {12, 1, pinstfs_control_creature,  pinstfm_control_creature,  pinstfe_passenger_control_creature, 0, 0, 0},
  {12, 1, pinstfs_direct_leave_creature,pinstfm_leave_creature, pinstfe_leave_creature,             0, 0, 0},
  {12, 1, pinstfs_passenger_leave_creature,pinstfm_leave_creature,pinstfe_leave_creature,           0, 0, 0},
  { 0, 1, pinstfs_query_creature,    NULL,                      NULL,                               0, 0, 0},
  { 0, 1, pinstfs_unquery_creature,  NULL,                      NULL,                               0, 0, 0},
  {16, 1, pinstfs_zoom_to_heart,     pinstfm_zoom_to_heart,     pinstfe_zoom_to_heart,              0, 0, 0},
  {16, 1, pinstfs_zoom_out_of_heart, pinstfm_zoom_out_of_heart, pinstfe_zoom_out_of_heart,          0, 0, 0},
  {12, 1, NULL,                      pinstfm_control_creature_fade,pinstfe_control_creature_fade,   0, 0, 0},
  { 8, 1, pinstfs_fade_to_map,       pinstfm_fade_to_map,       pinstfe_fade_to_map,                0, 0, 0},
  { 8, 1, pinstfs_fade_from_map,     pinstfm_fade_from_map,     pinstfe_fade_from_map,              0, 0, 0},
  {-1, 1, pinstfs_zoom_to_position,  pinstfm_zoom_to_position,  pinstfe_zoom_to_position,           0, 0, 0},
  { 0, 0, NULL,                      NULL,                      NULL,                               0, 0, 0},
  { 0, 0, NULL,                      NULL,                      NULL,                               0, 0, 0},
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
DLLIMPORT void __cdecl _DK_process_player_instance(struct PlayerInfo *player);
DLLIMPORT void _DK_process_player_instances(void);
DLLIMPORT void __cdecl _DK_set_player_instance(struct PlayerInfo *playerinf, long, int);

/******************************************************************************/
long pinstfs_hand_grab(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_hand_grab(player, n);
}

long pinstfm_hand_grab(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_hand_grab(player, n);
}

long pinstfe_hand_grab(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_hand_grab(player, n);
  struct Thing *picktng;
  struct Thing *thing2;
  struct CreatureControl *cctrl;
  long i;
  picktng = game.things_lookup[player->field_43E];
  thing2 = game.things_lookup[player->field_43A];
  if (!thing_is_pickable_by_hand(player,picktng))
  {
    player->field_440 = 0;
    player->field_43E = 0;
    return 0;
  }
  set_power_hand_offset(player, picktng);
  switch (picktng->class_id)
  {
  case TCls_Creature:
      if (!external_set_thing_state(picktng, 38))
        return 0;
      cctrl = game.persons.cctrl_lookup[picktng->field_64%CREATURES_COUNT];
      if (cctrl->field_AD & 0x02)
        i = convert_td_iso(122);
      else
        i = get_creature_anim(picktng, 9);
      set_thing_draw(picktng, i, 256, -1, -1, 0, 2);
      break;
  case TCls_Object:
      picktng = process_object_being_picked_up(picktng, thing2->owner);
      if (thing_is_invalid(picktng))
      {
        player->field_440 = 0;
        player->field_43E = 0;
        return 0;
      }
      break;
  }
  if (!thing_is_invalid(thing2))
    set_power_hand_graphic(player->field_2B, 784, 256);
  dump_thing_in_power_hand(picktng, player->field_2B);
  player->field_440 = 0;
  player->field_43E = 0;
  place_thing_in_limbo(picktng);
  return 0;
}

long pinstfs_hand_drop(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_hand_drop(player, n);
}

long pinstfe_hand_drop(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_hand_drop(player, n);
}

long pinstfs_hand_whip(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_hand_whip(player, n);
}

long pinstfe_hand_whip(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_hand_whip(player, n);
}

long pinstfm_hand_drop(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_hand_drop(player, n);
}

long pinstfs_hand_whip_end(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_hand_whip_end(player, n);
}

long pinstfe_hand_whip_end(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_hand_whip_end(player, n);
}

long pinstfs_control_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_control_creature(player, n);
}

long pinstfm_control_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_control_creature(player, n);
}

long pinstfe_direct_control_creature(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfe_direct_control_creature(player, n);
  struct Thing *thing;
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  long i,k;
  thing = NULL;
  i = player->field_43E;
  if ((i > 0) && (i < THINGS_COUNT))
    thing = game.things_lookup[i];
  if (thing == game.things_lookup[0])
    thing = NULL;
  if (thing != NULL)
  {
    if (!control_creature_as_controller(player, thing))
      thing = NULL;
  }
  if (thing == NULL)
  {
    set_camera_zoom(player->acamera, player->field_4B6);
    if (player == &game.players[my_player_number%PLAYERS_COUNT])
      PaletteSetPlayerPalette(player, _DK_palette);
    player->field_0 &= 0xEF;
    player->field_0 &= 0x7F;
    return 0;
  }
  set_player_instance(player, 13, 0);
  if (thing->class_id == TCls_Creature)
  {
    load_swipe_graphic_for_creature(thing);
    cctrl = game.persons.cctrl_lookup[thing->field_64%CREATURES_COUNT];
    if (player == &game.players[my_player_number%PLAYERS_COUNT])
    {
      if (cctrl->field_AB & 0x02)
        PaletteSetPlayerPalette(player, blue_palette);
    }
    crstat = &game.creature_stats[thing->model%CREATURE_TYPES_COUNT];
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
  if (player == &game.players[my_player_number%PLAYERS_COUNT])
    turn_on_menu(31);
  return 0;
}

long pinstfe_passenger_control_creature(struct PlayerInfo *player, long *n)
{
//  return _DK_pinstfe_passenger_control_creature(player, n);
  struct Thing *thing;
  long i;
  thing = NULL;
  i = player->field_43E;
  if ((i >= 0) && (i < THINGS_COUNT))
    thing = game.things_lookup[i];
  if ((thing != NULL) && (thing != game.things_lookup[0]))
    control_creature_as_passenger(player, thing);
  set_player_instance(player, 13, false);
  return 0;

}

long pinstfs_direct_leave_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_direct_leave_creature(player, n);
}

long pinstfm_leave_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_leave_creature(player, n);
}

long pinstfs_passenger_leave_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_passenger_leave_creature(player, n);
}

long pinstfe_leave_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_leave_creature(player, n);
}

long pinstfs_query_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_query_creature(player, n);
}

long pinstfs_unquery_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_unquery_creature(player, n);
}

long pinstfs_zoom_to_heart(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_zoom_to_heart(player, n);
}

long pinstfm_zoom_to_heart(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfm_zoom_to_heart(player, n);
  struct Thing *thing;
  struct Coord3d pos;
  long i;
  thing = NULL;
  i = player->field_2F;
  if ((i > 0) && (i < THINGS_COUNT))
    thing = game.things_lookup[i];
  if ((thing != NULL) && (thing != game.things_lookup[0]))
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
  thing = game.things_lookup[player->field_2F%THINGS_COUNT];
  if ((thing != NULL) && (thing != game.things_lookup[0]))
    leave_creature_as_controller(player, thing);
  set_player_mode(player, 1);
  cam = player->acamera;
  if (cam == NULL) return 0;
  dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
  thing = game.things_lookup[dungeon->field_0%THINGS_COUNT];
  if ((thing == NULL) || (thing == game.things_lookup[0]))
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
  return _DK_pinstfm_zoom_out_of_heart(player, n);
}

long pinstfe_zoom_out_of_heart(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_zoom_out_of_heart(player, n);
}

long pinstfm_control_creature_fade(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfm_control_creature_fade(player, n);
  player->field_0 |= 0x80;
  return 0;
}

long pinstfe_control_creature_fade(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_control_creature_fade(player, n);
}

long pinstfs_fade_to_map(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_fade_to_map(player, n);
}

long pinstfm_fade_to_map(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfm_fade_to_map(player, n);
  return 0;
}

long pinstfe_fade_to_map(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_fade_to_map(player, n);
}

long pinstfs_fade_from_map(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_fade_from_map(player, n);
}

long pinstfm_fade_from_map(struct PlayerInfo *player, long *n)
{
  //return _DK_pinstfm_fade_from_map(player, n);
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
  return _DK_pinstfm_zoom_to_position(player, n);
}

long pinstfe_zoom_to_position(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_zoom_to_position(player, n);
}

void set_player_instance(struct PlayerInfo *player, long ninum, short force)
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
  static const char *func_name="process_player_instance";
  struct PlayerInstanceInfo *inst_info;
  InstncInfo_Func callback;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting for instance %d\n",func_name,player->instance_num);
#endif
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
  static const char *func_name="process_player_instances";
  //_DK_process_player_instances();return;
  int i;
  struct PlayerInfo *player;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player=&(game.players[i]);
    if (player->field_0 & 0x01)
      process_player_instance(player);
  }
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
