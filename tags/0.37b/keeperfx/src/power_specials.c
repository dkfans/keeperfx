/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file power_specials.c
 *     power_specials support functions.
 * @par Purpose:
 *     Functions to power_specials.
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
#include "power_specials.h"

#include "globals.h"
#include "bflib_basics.h"

#include "thing_creature.h"
#include "thing_effects.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "creature_control.h"
#include "creature_states.h"
#include "game_saves.h"
#include "game_merge.h"
#include "slab_data.h"
#include "thing_objects.h"
#include "frontend.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_activate_dungeon_special(struct Thing *thing, struct PlayerInfo *player);
DLLIMPORT void _DK_resurrect_creature(struct Thing *thing, unsigned char a2, unsigned char a3, unsigned char a4);
DLLIMPORT void _DK_transfer_creature(struct Thing *tng1, struct Thing *tng2, unsigned char a3);
DLLIMPORT void _DK_make_safe(struct PlayerInfo *player);
DLLIMPORT unsigned long _DK_steal_hero(struct PlayerInfo *player, struct Coord3d *pos);

/******************************************************************************/
/**
 * Makes a bonus level for current SP level visible on the land map screen.
 */
TbBool activate_bonus_level(struct PlayerInfo *player)
{
  TbBool result;
  LevelNumber sp_lvnum;
  SYNCDBG(5,"Starting");
  set_flag_byte(&game.flags_font,FFlg_unk02,true);
  sp_lvnum = get_loaded_level_number();
  result = set_bonus_level_visibility_for_singleplayer_level(player, sp_lvnum, true);
  if (!result)
    ERRORLOG("No Bonus level assigned to level %d",(int)sp_lvnum);
  set_flag_byte(&game.numfield_C,0x02,false);
  return result;
}

void multiply_creatures(struct PlayerInfo *player)
{
  struct Dungeon *dungeon;
  struct Thing *thing;
  struct Thing *tncopy;
  struct CreatureControl *cctrl;
  unsigned long k;
  int i;
  dungeon = get_dungeon(player->id_number);
  // Copy 'normal' creatures
  k = 0;
  i = dungeon->creatr_list_start;
  while (i != 0)
  {
    thing = thing_get(i);
    cctrl = creature_control_get_from_thing(thing);
    if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
    {
      ERRORLOG("Jump to invalid creature detected");
      break;
    }
    i = cctrl->thing_idx;
    // Thing list loop body
    tncopy = create_creature(&thing->mappos, thing->model, player->id_number);
    if (thing_is_invalid(tncopy))
    {
      WARNLOG("Can't create a copy of creature");
      break;
    }
    set_creature_level(tncopy, thing->field_23);
    tncopy->health = thing->health;
    // Thing list loop body ends
    k++;
    if (k > CREATURES_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping creatures list");
      break;
    }
  }
  // Copy 'special worker' creatures
  k = 0;
  i = dungeon->worker_list_start;
  while (i != 0)
  {
    thing = thing_get(i);
    cctrl = creature_control_get_from_thing(thing);
    if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
    {
      ERRORLOG("Jump to invalid creature detected");
      break;
    }
    i = cctrl->thing_idx;
    // Thing list loop body
    tncopy = create_creature(&thing->mappos, thing->model, player->id_number);
    if (thing_is_invalid(tncopy))
    {
      WARNLOG("Can't create a copy of creature");
      break;
    }
    set_creature_level(tncopy, thing->field_23);
    tncopy->health = thing->health;
    // Thing list loop body ends
    k++;
    if (k > CREATURES_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping creatures list");
      break;
    }
  }
}

void increase_level(struct PlayerInfo *player)
{
  struct Dungeon *dungeon;
  struct CreatureControl *cctrl;
  struct Thing *thing;
  unsigned long k;
  int i;
  dungeon = get_dungeon(player->id_number);
  // Increase level of normal creatures
  k = 0;
  i = dungeon->creatr_list_start;
  while (i != 0)
  {
    thing = thing_get(i);
    cctrl = creature_control_get_from_thing(thing);
    if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
    {
      ERRORLOG("Jump to invalid creature detected");
      break;
    }
    i = cctrl->thing_idx;
    // Thing list loop body
    creature_increase_level(thing);
    // Thing list loop body ends
    k++;
    if (k > CREATURES_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping creatures list");
      break;
    }
  }
  // Increase level of special workers
  k = 0;
  i = dungeon->worker_list_start;
  while (i != 0)
  {
    thing = thing_get(i);
    cctrl = creature_control_get_from_thing(thing);
    if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
    {
      ERRORLOG("Jump to invalid creature detected");
      break;
    }
    i = cctrl->thing_idx;
    // Thing list loop body
    creature_increase_level(thing);
    // Thing list loop body ends
    k++;
    if (k > CREATURES_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping creatures list");
      break;
    }
  }
}

unsigned long steal_hero(struct PlayerInfo *player, struct Coord3d *pos)
{
  return _DK_steal_hero(player, pos);
}

void make_safe(struct PlayerInfo *player)
{
  _DK_make_safe(player);
}

void activate_dungeon_special(struct Thing *thing, struct PlayerInfo *player)
{
  SYNCDBG(6,"Starting");
  short used;
  struct Coord3d pos;
  int spkindidx;

  // Gathering data which we'll need if the special is used and disposed.
  memcpy(&pos,&thing->mappos,sizeof(struct Coord3d));
  spkindidx = thing->model - 86;
  used = 0;
  if (thing_exists(thing) && is_dungeon_special(thing))
  {
    switch (thing->model)
    {
        case 86:
          reveal_whole_map(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        case 87:
          start_resurrect_creature(player, thing);
          break;
        case 88:
          start_transfer_creature(player, thing);
          break;
        case 89:
          if (steal_hero(player, &thing->mappos))
          {
            remove_events_thing_is_attached_to(thing);
            used = 1;
            delete_thing_structure(thing, 0);
          }
          break;
        case 90:
          multiply_creatures(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        case 91:
          increase_level(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        case 92:
          make_safe(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        case 93:
          activate_bonus_level(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        default:
          ERRORLOG("Invalid dungeon special (Model %d)", (int)thing->model);
          break;
      }
      if ( used )
      {
        if (is_my_player(player))
          output_message(special_desc[spkindidx].field_8, 0, 1);
        create_special_used_effect(&pos, player->id_number);
      }
  }
}

void resurrect_creature(struct Thing *thing, unsigned char a2, unsigned char a3, unsigned char a4)
{
  _DK_resurrect_creature(thing, a2, a3, a4);
}

void transfer_creature(struct Thing *tng1, struct Thing *tng2, unsigned char plyr_idx)
{
  SYNCDBG(7,"Starting");
  struct Dungeon *dungeon;
  struct CreatureControl *cctrl;
  struct Thing *thing;
  long i,k;
  dungeon = get_players_num_dungeon(plyr_idx);
  // Check if 'things' are correct
  if ((tng1->field_0 & 0x01) == 0)
    return;
  if ((tng1->class_id != TCls_Object) || (tng1->model != 88))
    return;

  if ((tng2->field_0 & 0x01) == 0)
    return;
  if ((tng2->class_id != TCls_Creature) || (tng2->owner != plyr_idx))
    return;

  cctrl = creature_control_get_from_thing(tng2);
  set_transfered_creature(plyr_idx, tng2->model, cctrl->explevel);
  // Remove the creature from power hand
  for (i = 0; i < dungeon->field_63; i++)
  {
    if (dungeon->things_in_hand[i] == tng2->index)
    {
      for ( ; i < dungeon->field_63-1; i++)
      {
        dungeon->things_in_hand[i] = dungeon->things_in_hand[i+1];
      }
      dungeon->field_63--;
      dungeon->things_in_hand[dungeon->field_63] = 0;
    }
  }
  kill_creature(tng2, 0, 0, 1, 0, 0);
  create_special_used_effect(&tng1->mappos, plyr_idx);
  remove_events_thing_is_attached_to(tng1);
  if ((tng1->field_1 & 0x01) || (tng1->field_0 & 0x80))
  {
    k = 0;
    i = dungeon->worker_list_start;
    while (i > 0)
    {
      thing = thing_get(i);
      if (thing_is_invalid(thing))
        break;
      cctrl = creature_control_get_from_thing(thing);
      if (creature_control_invalid(cctrl))
        break;
      if (cctrl->field_6E == tng1->index)
      {
        set_start_state(thing);
        break;
      }
      i = cctrl->thing_idx;
      k++;
      if (k > CREATURES_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping creatures list");
        break;
      }
    }
  }
  delete_thing_structure(tng1, 0);
  if (is_my_player_number(plyr_idx))
    output_message(80, 0, 1);
}

void start_transfer_creature(struct PlayerInfo *player, struct Thing *thing)
{
  struct Dungeon *dungeon;
  dungeon = get_dungeon(player->id_number);
  if (dungeon->field_919 != 0)
  {
    if (is_my_player(player))
    {
      dungeon_special_selected = thing->index;
      transfer_creature_scroll_offset = 0;
      turn_off_menu(GMnu_DUNGEON_SPECIAL);
      turn_on_menu(GMnu_TRANSFER_CREATURE);
    }
  }
}

void start_resurrect_creature(struct PlayerInfo *player, struct Thing *thing)
{
  struct Dungeon *dungeon;
  dungeon = get_dungeon(player->id_number);
  if (dungeon->dead_creatures_count != 0)
  {
    if (is_my_player(player))
    {
      dungeon_special_selected = thing->index;
      resurrect_creature_scroll_offset = 0;
      turn_off_menu(GMnu_DUNGEON_SPECIAL);
      turn_on_menu(GMnu_RESURRECT_CREATURE);
    }
  }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
