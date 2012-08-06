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
#include "spdigger_stack.h"
#include "thing_corpses.h"
#include "thing_objects.h"
#include "frontend.h"
#include "gui_frontmenu.h"
#include "gui_soundmsgs.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_activate_dungeon_special(struct Thing *boxtng, struct PlayerInfo *player);
DLLIMPORT void _DK_resurrect_creature(struct Thing *boxtng, unsigned char a2, unsigned char crmodel, unsigned char crlevel);
DLLIMPORT void _DK_transfer_creature(struct Thing *boxtng, struct Thing *transftng, unsigned char crmodel);
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
    i = cctrl->players_next_creature_idx;
    // Thing list loop body
    tncopy = create_creature(&thing->mappos, thing->model, player->id_number);
    if (thing_is_invalid(tncopy))
    {
      WARNLOG("Can't create a copy of creature");
      break;
    }
    set_creature_level(tncopy, cctrl->explevel);
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
  i = dungeon->digger_list_start;
  while (i != 0)
  {
    thing = thing_get(i);
    cctrl = creature_control_get_from_thing(thing);
    if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
    {
      ERRORLOG("Jump to invalid creature detected");
      break;
    }
    i = cctrl->players_next_creature_idx;
    // Thing list loop body
    tncopy = create_creature(&thing->mappos, thing->model, player->id_number);
    if (thing_is_invalid(tncopy))
    {
      WARNLOG("Can't create a copy of creature");
      break;
    }
    set_creature_level(tncopy, cctrl->explevel);
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
        i = cctrl->players_next_creature_idx;
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
    i = dungeon->digger_list_start;
    while (i != 0)
    {
        thing = thing_get(i);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
          ERRORLOG("Jump to invalid creature detected");
          break;
        }
        i = cctrl->players_next_creature_idx;
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

void activate_dungeon_special(struct Thing *boxtng, struct PlayerInfo *player)
{
  SYNCDBG(6,"Starting");
  short used;
  struct Coord3d pos;
  int spkindidx;

  // Gathering data which we'll need if the special is used and disposed.
  memcpy(&pos,&boxtng->mappos,sizeof(struct Coord3d));
  spkindidx = boxtng->model - 86;
  used = 0;
  if (thing_exists(boxtng) && is_dungeon_special(boxtng))
  {
    switch (boxtng->model)
    {
        case 86:
          reveal_whole_map(player);
          remove_events_thing_is_attached_to(boxtng);
          used = 1;
          delete_thing_structure(boxtng, 0);
          break;
        case 87:
          start_resurrect_creature(player, boxtng);
          break;
        case 88:
          start_transfer_creature(player, boxtng);
          break;
        case 89:
          if (steal_hero(player, &boxtng->mappos))
          {
            remove_events_thing_is_attached_to(boxtng);
            used = 1;
            delete_thing_structure(boxtng, 0);
          }
          break;
        case 90:
          multiply_creatures(player);
          remove_events_thing_is_attached_to(boxtng);
          used = 1;
          delete_thing_structure(boxtng, 0);
          break;
        case 91:
          increase_level(player);
          remove_events_thing_is_attached_to(boxtng);
          used = 1;
          delete_thing_structure(boxtng, 0);
          break;
        case 92:
          make_safe(player);
          remove_events_thing_is_attached_to(boxtng);
          used = 1;
          delete_thing_structure(boxtng, 0);
          break;
        case 93:
          activate_bonus_level(player);
          remove_events_thing_is_attached_to(boxtng);
          used = 1;
          delete_thing_structure(boxtng, 0);
          break;
        default:
          ERRORLOG("Invalid dungeon special (Model %d)", (int)boxtng->model);
          break;
      }
      if ( used )
      {
        if (is_my_player(player))
          output_message(special_desc[spkindidx].field_8, 0, true);
        create_special_used_effect(&pos, player->id_number);
      }
  }
}

void resurrect_creature(struct Thing *boxtng, PlayerNumber owner, ThingModel crmodel, unsigned char crlevel)
{
    struct Thing *creatng;
    //_DK_resurrect_creature(thing, owner, crmodel, crlevel);
    if (!thing_exists(boxtng) || (box_thing_to_special(boxtng) != SpcKind_Resurrect) ) {
        ERRORMSG("Invalid resurrect box object!");
        return;
    }
    creatng = create_creature(&boxtng->mappos, crmodel, owner);
    if (!thing_is_invalid(creatng))
    {
        init_creature_level(creatng, crlevel);
        if (is_my_player_number(owner))
          output_message(SMsg_CommonAcknowledge, 0, true);
    }
    create_special_used_effect(&boxtng->mappos, owner);
    remove_events_thing_is_attached_to(boxtng);
    force_any_creature_dragging_owned_thing_to_drop_it(boxtng);
    remove_item_from_dead_creature_list(get_players_num_dungeon(owner), crmodel, crlevel);
    delete_thing_structure(boxtng, 0);
}

void transfer_creature(struct Thing *boxtng, struct Thing *transftng, unsigned char plyr_idx)
{
  SYNCDBG(7,"Starting");
  struct Dungeon *dungeon;
  struct CreatureControl *cctrl;
  long i;
  if (!thing_exists(boxtng) || (box_thing_to_special(boxtng) != SpcKind_TrnsfrCrtr) ) {
      ERRORMSG("Invalid transfer box object!");
      return;
  }
  dungeon = get_players_num_dungeon(plyr_idx);
  // Check if 'things' are correct
  if (!thing_exists(transftng) || !thing_is_creature(transftng) || (transftng->owner != plyr_idx)) {
      ERRORMSG("Invalid transfer creature thing!");
      return;
  }

  cctrl = creature_control_get_from_thing(transftng);
  set_transfered_creature(plyr_idx, transftng->model, cctrl->explevel);
  // Remove the creature from power hand
  for (i = 0; i < dungeon->field_63; i++)
  {
    if (dungeon->things_in_hand[i] == transftng->index)
    {
      for ( ; i < dungeon->field_63-1; i++)
      {
        dungeon->things_in_hand[i] = dungeon->things_in_hand[i+1];
      }
      dungeon->field_63--;
      dungeon->things_in_hand[dungeon->field_63] = 0;
    }
  }
  kill_creature(transftng, NULL, 0, 1, 0, 0);
  create_special_used_effect(&boxtng->mappos, plyr_idx);
  remove_events_thing_is_attached_to(boxtng);
  force_any_creature_dragging_owned_thing_to_drop_it(boxtng);
  delete_thing_structure(boxtng, 0);
  if (is_my_player_number(plyr_idx))
    output_message(SMsg_CommonAcknowledge, 0, true);
}

void start_transfer_creature(struct PlayerInfo *player, struct Thing *thing)
{
  struct Dungeon *dungeon;
  dungeon = get_dungeon(player->id_number);
  if (dungeon->num_active_creatrs != 0)
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
