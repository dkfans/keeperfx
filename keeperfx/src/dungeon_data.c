/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file dungeon_data.c
 *     Dungeon data structures definitions.
 * @par Purpose:
 *     Defines functions for dungeon-related structures support.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Nov 2009 - 20 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "dungeon_data.h"

#include "globals.h"
#include "bflib_basics.h"
#include "keeperfx.hpp"

/******************************************************************************/
/******************************************************************************/
struct Dungeon bad_dungeon;
/******************************************************************************/
struct Dungeon *get_players_num_dungeon_ptr(long plyr_idx,const char *func_name)
{
    struct PlayerInfo *player;
    PlayerNumber plyr_num;
    player = get_player(plyr_idx);
    plyr_num = player->id_number;
    if (player_invalid(player) || (plyr_num < 0) || (plyr_num >= DUNGEONS_COUNT))
    {
        ERRORMSG("%s: Tried to get nonexisting dungeon %ld!",func_name,(long)plyr_num);
        return INVALID_DUNGEON;
    }
    if (plyr_num != player->id_number)
    {
        WARNDBG(7,"%s: Player number(%ld) differ from index(%ld)!",func_name,(long)plyr_num,plyr_idx);
    }
    return &(game.dungeon[plyr_num]);
}

struct Dungeon *get_players_dungeon_ptr(struct PlayerInfo *player,const char *func_name)
{
    PlayerNumber plyr_num;
    plyr_num = player->id_number;
    if (player_invalid(player) || (plyr_num < 0) || (plyr_num >= DUNGEONS_COUNT))
    {
        ERRORLOG("%s: Tried to get nonexisting dungeon %ld!",func_name,(long)plyr_num);
        return INVALID_DUNGEON;
    }
    return &(game.dungeon[plyr_num]);
}

struct Dungeon *get_dungeon_ptr(PlayerNumber plyr_num,const char *func_name)
{
    if ((plyr_num < 0) || (plyr_num >= DUNGEONS_COUNT))
    {
        ERRORLOG("%s: Tried to get nonexisting dungeon %ld!",func_name,(long)plyr_num);
        return INVALID_DUNGEON;
    }
    return &(game.dungeon[plyr_num]);
}

TbBool dungeon_invalid(struct Dungeon *dungeon)
{
    if (dungeon == INVALID_DUNGEON)
        return true;
    return (dungeon < &game.dungeon[0]);
}

void clear_dungeons(void)
{
  SYNCDBG(6,"Starting");
  int i;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    memset(&game.dungeon[i], 0, sizeof(struct Dungeon));
  }
  memset(&bad_dungeon, 0, sizeof(struct Dungeon));
  game.field_14E4A4 = 0;
  game.field_14E4A0 = 0;
  game.field_14E49E = 0;
}

void decrease_dungeon_area(unsigned char plyr_idx, long value)
{
    struct Dungeon *dungeon;
    if (plyr_idx == game.neutral_player_num)
        return;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon->total_area < value)
      dungeon->total_area = 0;
    else
      dungeon->total_area -= value;
}

void increase_room_area(unsigned char plyr_idx, long value)
{
    struct Dungeon *dungeon;
    if (plyr_idx == game.neutral_player_num)
        return;
    dungeon = get_dungeon(plyr_idx);
    dungeon->field_949 += value;
    dungeon->total_area += value;
}

void decrease_room_area(unsigned char plyr_idx, long value)
{
    struct Dungeon *dungeon;
    if (plyr_idx == game.neutral_player_num)
        return;
    dungeon = get_dungeon(plyr_idx);

    if (dungeon->field_949 < value)
      dungeon->field_949 = 0;
    else
      dungeon->field_949 -= value;

    if (dungeon->total_area < value)
      dungeon->total_area = 0;
    else
      dungeon->total_area -= value;
}

/******************************************************************************/
