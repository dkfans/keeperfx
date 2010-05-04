/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file magic.c
 *     magic support functions.
 * @par Purpose:
 *     Functions to magic.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "magic.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_sound.h"

#include "player_data.h"
#include "dungeon_data.h"
#include "thing_list.h"
#include "game_merge.h"
#include "power_specials.h"
#include "thing_stats.h"
#include "creature_control.h"
#include "creature_states.h"
#include "config_creature.h"
#include "config_magic.hpp"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const long power_sight_close_instance_time[] = {4, 4, 5, 5, 6, 6, 7, 7, 8};

/******************************************************************************/
DLLIMPORT void _DK_magic_use_power_chicken(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
DLLIMPORT void _DK_magic_use_power_disease(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
DLLIMPORT void _DK_magic_use_power_destroy_walls(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT short _DK_magic_use_power_imp(unsigned short a1, unsigned short a2, unsigned short a3);
DLLIMPORT void _DK_magic_use_power_heal(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
DLLIMPORT void _DK_magic_use_power_conceal(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
DLLIMPORT void _DK_magic_use_power_armour(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
DLLIMPORT void _DK_magic_use_power_speed(unsigned char a1, struct Thing *thing, long a3, long a4, long a5);
DLLIMPORT void _DK_magic_use_power_lightning(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT long _DK_magic_use_power_sight(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT void _DK_magic_use_power_cave_in(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT long _DK_magic_use_power_call_to_arms(unsigned char a1, long a2, long a3, long a4, long a5);
DLLIMPORT short _DK_magic_use_power_hand(unsigned short a1, unsigned short a2, unsigned short a3, unsigned short a4);
DLLIMPORT short _DK_magic_use_power_slap(unsigned short a1, unsigned short a2, unsigned short a3);
DLLIMPORT short _DK_magic_use_power_obey(unsigned short plridx);
DLLIMPORT long _DK_magic_use_power_armageddon(unsigned char val);
DLLIMPORT void _DK_magic_use_power_hold_audience(unsigned char idx);

DLLIMPORT long _DK_power_sight_explored(long stl_x, long stl_y, unsigned char plyr_idx);
DLLIMPORT void _DK_update_power_sight_explored(struct PlayerInfo *player);
DLLIMPORT unsigned char _DK_can_cast_spell_at_xy(unsigned char a1, unsigned char a2, unsigned char a3, unsigned char a4, long a5);
/******************************************************************************/
void update_power_sight_explored(struct PlayerInfo *player)
{
  SYNCDBG(6,"Starting");
  _DK_update_power_sight_explored(player);
}

long power_sight_explored(long stl_x, long stl_y, unsigned char plyr_idx)
{
  return _DK_power_sight_explored(stl_x, stl_y, plyr_idx);
}

void slap_creature(struct PlayerInfo *player, struct Thing *thing)
{
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  long i;
  crstat = creature_stats_get_from_thing(thing);
  cctrl = creature_control_get_from_thing(thing);

  anger_apply_anger_to_creature(thing, crstat->annoy_slapped, 4, 1);
  if (crstat->slaps_to_kill > 0)
  {
    i = compute_creature_max_health(crstat->health,cctrl->explevel) / crstat->slaps_to_kill;
    if (i > 0)
    {
      apply_damage_to_thing(thing, i, player->id_number);
      thing->word_17 = 8;
    }
  }
  i = cctrl->field_21;
  cctrl->field_21 = game.magic_stats[4].time;
  if (i == 0)
    cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
  if (thing->field_7 != 66)
  {
    clear_creature_instance(thing);
    cctrl->field_27D = thing->field_7;
    cctrl->field_27E = thing->field_8;
    if (thing->field_7 == 26)
      anger_apply_anger_to_creature(thing, crstat->annoy_woken_up, 4, 1);
    external_set_thing_state(thing, 66);
  }
  cctrl->field_B1 = 6;
  cctrl->field_27F = 18;
  play_creature_sound(thing, CrSnd_SlappedOuch, 3, 0);
}

TbBool can_cast_spell_at_xy(unsigned char plyr_idx, unsigned char spl_id, unsigned char stl_x, unsigned char stl_y, long a5)
{
  struct PlayerInfo *player;
  struct Map *mapblk;
  struct SlabMap *slb;
  TbBool can_cast;
  mapblk = get_map_block_at(stl_x, stl_y);
  slb = get_slabmap_for_subtile(stl_x, stl_y);
  can_cast = false;
  switch (spl_id)
  {
  default:
      if ((mapblk->flags & 0x10) == 0)
      {
        can_cast = true;
      }
      break;
  case 2:
      if ((mapblk->flags & 0x10) == 0)
      {
        if (slabmap_owner(slb) == plyr_idx)
        {
          can_cast = true;
        }
      }
      break;
  case 5:
      can_cast = true;
      break;
  case 6:
      if ((mapblk->flags & 0x10) == 0)
      {
        if (map_block_revealed(mapblk, plyr_idx) || (a5 == 1))
        {
          can_cast = true;
        }
      }
      break;
  case 7:
      if ((mapblk->flags & 0x10) == 0)
      {
        if (power_sight_explored(stl_x, stl_y, plyr_idx) || map_block_revealed(mapblk, plyr_idx))
        {
          can_cast = true;
        }
      }
      break;
  case 10:
      if ((mapblk->flags & 0x10) == 0)
      {
        if (power_sight_explored(stl_x, stl_y, plyr_idx) || map_block_revealed(mapblk, plyr_idx))
        {
          player = get_player(plyr_idx);
          if (player->field_4E3+20 < game.play_gameturn)
          {
            can_cast = true;
          }
        }
      }
      break;
  case 14:
  case 15:
      if (slabmap_owner(slb) == plyr_idx)
      {
        can_cast = true;
      }
      break;
  case 16:
      if (power_sight_explored(stl_x, stl_y, plyr_idx) || map_block_revealed(mapblk, plyr_idx))
      {
        if ((mapblk->flags & 0x10) != 0)
        {
          if ((mapblk->flags & 0x43) == 0)
          {
            if (slb->slab != 0)
              can_cast = true;
          }
        }
      }
      break;
  }
  return can_cast;
}

void magic_use_power_armageddon(unsigned int plridx)
{
  SYNCDBG(6,"Starting");
  _DK_magic_use_power_armageddon(plridx);
}

short magic_use_power_obey(unsigned short plridx)
{
  return _DK_magic_use_power_obey(plridx);
}

void turn_off_sight_of_evil(long plyr_idx)
{
    struct Dungeon *dungeon;
  struct MagicStats *mgstat;
  long spl_lev,cit;
  long i,imax,k,n;
  //_DK_turn_off_sight_of_evil(plyr_idx);
  dungeon = get_players_num_dungeon(plyr_idx);
  mgstat = &(game.magic_stats[5]);
  spl_lev = dungeon->field_5DA;
  if (spl_lev > SPELL_MAX_LEVEL)
      spl_lev = SPELL_MAX_LEVEL;
  i = game.play_gameturn - dungeon->field_5D4;
  imax = abs(mgstat->power[spl_lev]/4) >> 2;
  if (i > imax)
      i = imax;
  if (i < 0)
      i = 0;
  n = game.play_gameturn - mgstat->power[spl_lev];
  cit = power_sight_close_instance_time[spl_lev];
  k = imax / cit;
  if (k < 1) k = 1;
  dungeon->field_5D4 = n + i/k - cit;
}

void magic_use_power_hold_audience(unsigned char idx)
{
  _DK_magic_use_power_hold_audience(idx);
}

void magic_use_power_chicken(unsigned char a1, struct Thing *thing, long a3, long a4, long a5)
{
  _DK_magic_use_power_chicken(a1, thing, a3, a4, a5);
}

void magic_use_power_disease(unsigned char a1, struct Thing *thing, long a3, long a4, long a5)
{
  _DK_magic_use_power_disease(a1, thing, a3, a4, a5);
}

void magic_use_power_destroy_walls(unsigned char a1, long a2, long a3, long a4)
{
  _DK_magic_use_power_destroy_walls(a1, a2, a3, a4);
}

short magic_use_power_imp(unsigned short plyr_idx, unsigned short stl_x, unsigned short stl_y)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    struct Thing *dnheart;
    struct Coord3d pos;
    long cost;
    long i;
    //return _DK_magic_use_power_imp(plyr_idx, x, y);
    if (!can_cast_spell_at_xy(plyr_idx, 2, stl_x, stl_y, 0)
     || !i_can_allocate_free_control_structure()
     || !i_can_allocate_free_thing_structure(1))
    {
      if (is_my_player_number(plyr_idx))
          play_non_3d_sample(119);
      return 1;
    }
    dungeon = get_players_num_dungeon(plyr_idx);
    i = dungeon->field_918 - dungeon->creature_sacrifice[23] + 1;
    if (i < 1)
      i = 1;
    cost = game.magic_stats[2].cost[0]*i/2;
    if (take_money_from_dungeon(plyr_idx, cost, 1) < 0)
    {
        if (is_my_player_number(plyr_idx))
          output_message(87, 0, 1);
        return -1;
    }
    dnheart = thing_get(dungeon->dnheart_idx);
    pos.x.val = get_subtile_center_pos(stl_x);
    pos.y.val = get_subtile_center_pos(stl_y);
    pos.z.val = get_floor_height_at(&pos) + (dnheart->field_58 >> 1);
    thing = create_creature(&pos, get_players_special_digger_breed(plyr_idx), plyr_idx);
    if (!thing_is_invalid(thing))
    {
        thing->pos_32.x.val += ACTION_RANDOM(161) - 80;
        thing->pos_32.y.val += ACTION_RANDOM(161) - 80;
        thing->pos_32.z.val += 160;
        thing->field_1 |= 0x04;
        thing->field_52 = 0;
        initialise_thing_state(thing, CrSt_ImpBirth);
        play_creature_sound(thing, 3, 2, 0);
    }
    return 1;
}

void magic_use_power_heal(unsigned char a1, struct Thing *thing, long a3, long a4, long a5)
{
  _DK_magic_use_power_heal(a1, thing, a3, a4, a5);
}

void magic_use_power_conceal(unsigned char a1, struct Thing *thing, long a3, long a4, long a5)
{
  _DK_magic_use_power_conceal(a1, thing, a3, a4, a5);
}

void magic_use_power_armour(unsigned char a1, struct Thing *thing, long a3, long a4, long a5)
{
  _DK_magic_use_power_armour(a1, thing, a3, a4, a5);
}

void magic_use_power_speed(unsigned char a1, struct Thing *thing, long a3, long a4, long a5)
{
  _DK_magic_use_power_speed(a1, thing, a3, a4, a5);
}

void magic_use_power_lightning(unsigned char a1, long a2, long a3, long a4)
{
  _DK_magic_use_power_lightning(a1, a2, a3, a4);
}

long magic_use_power_sight(unsigned char a1, long a2, long a3, long a4)
{
  return _DK_magic_use_power_sight(a1, a2, a3, a4);
}

void magic_use_power_cave_in(unsigned char a1, long a2, long a3, long a4)
{
  _DK_magic_use_power_cave_in(a1, a2, a3, a4);
}

long magic_use_power_call_to_arms(unsigned char a1, long a2, long a3, long a4, long a5)
{
  return _DK_magic_use_power_call_to_arms(a1, a2, a3, a4, a5);
}

short magic_use_power_slap(unsigned short plyr_idx, unsigned short stl_x, unsigned short stl_y)
{
  return _DK_magic_use_power_slap(plyr_idx, stl_x, stl_y);
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
