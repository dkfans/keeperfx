/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_library.c
 *     Library room maintain functions.
 * @par Purpose:
 *     Functions to create and use libraries.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 05 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_library.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_objects.h"
#include "thing_stats.h"
#include "config_terrain.h"
#include "creature_states.h"
#include "creature_states_rsrch.h"
#include "magic.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_process_player_research(int plr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct Thing *create_spell_in_library(struct Room *room, ThingModel spkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Coord3d pos;
    struct Thing *spelltng;
    if (room->kind != RoK_LIBRARY) {
        SYNCDBG(4,"Cannot add spell to %s owned by player %d",room_code_name(room->kind),(int)room->owner);
        return INVALID_THING;
    }
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = 0;
    spelltng = create_object(&pos, spkind, room->owner, -1);
    if (thing_is_invalid(spelltng))
    {
        return INVALID_THING;
    }
    // Neutral thing do not need any more processing
    if (is_neutral_thing(spelltng))
    {
        return spelltng;
    }
    if (!add_item_to_room_capacity(room))
    {
        destroy_object(spelltng);
        return INVALID_THING;
    }
    if (!add_spell_to_player(book_thing_to_magic(spelltng), room->owner))
    {
        remove_item_from_room_capacity(room);
        destroy_object(spelltng);
        return INVALID_THING;
    }
    return spelltng;
}

TbBool remove_spell_from_library(struct Room *room, struct Thing *spelltng, long new_owner)
{
    if ( (room->kind != RoK_LIBRARY) || (spelltng->owner != room->owner) ) {
        SYNCDBG(4,"Spell %s owned by player %d found in a %s owned by player %d, instead of proper library",thing_model_name(spelltng),(int)spelltng->owner,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    if (!remove_item_from_room_capacity(room))
        return false;
    remove_spell_from_player(book_thing_to_magic(spelltng), room->owner);
    if (is_my_player_number(room->owner))
    {
        output_message(SMsg_SpellbookStolen, 0, true);
    } else
    if (is_my_player_number(new_owner))
    {
        output_message(SMsg_SpellbookTaken, 0, true);
    }
    return true;
}

void init_dungeons_research(void)
{
    struct Dungeon *dungeon;
    int i;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
      dungeon = get_dungeon(i);
      dungeon->current_research_idx = get_next_research_item(dungeon);
    }
}

TbBool remove_all_research_from_player(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    dungeon->research_num = 0;
    dungeon->research_override = 1;
    return true;
}

TbBool research_overriden_for_player(PlayerNumber plyr_idx)
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

TbBool add_research_to_player(PlayerNumber plyr_idx, long rtyp, long rkind, long amount)
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

TbBool update_players_research_amount(PlayerNumber plyr_idx, long rtyp, long rkind, long amount)
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

TbBool update_or_add_players_research_amount(PlayerNumber plyr_idx, long rtyp, long rkind, long amount)
{
  if (update_players_research_amount(plyr_idx, rtyp, rkind, amount))
    return true;
  return add_research_to_player(plyr_idx, rtyp, rkind, amount);
}

void process_player_research(PlayerNumber plyr_idx)
{
  _DK_process_player_research(plyr_idx);
}
/******************************************************************************/
