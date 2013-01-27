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
#include "bflib_memory.h"
#include "game_legacy.h"

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
        ERRORMSG("%s: Tried to get players %d non-existing dungeon %d!",func_name,(int)plyr_idx,(int)plyr_num);
        return INVALID_DUNGEON;
    }
    if (plyr_num != player->id_number)
    {
        WARNDBG(7,"%s: Player number(%d) differ from index(%d)!",func_name,(int)plyr_num,(int)plyr_idx);
    }
    return &(game.dungeon[plyr_num]);
}

struct Dungeon *get_players_dungeon_ptr(const struct PlayerInfo *player,const char *func_name)
{
    PlayerNumber plyr_num;
    plyr_num = player->id_number;
    if (player_invalid(player) || (plyr_num < 0) || (plyr_num >= DUNGEONS_COUNT))
    {
        ERRORLOG("%s: Tried to get non-existing dungeon %ld!",func_name,(long)plyr_num);
        return INVALID_DUNGEON;
    }
    return &(game.dungeon[plyr_num]);
}

struct Dungeon *get_dungeon_ptr(PlayerNumber plyr_num,const char *func_name)
{
    if ((plyr_num < 0) || (plyr_num >= DUNGEONS_COUNT))
    {
        ERRORLOG("%s: Tried to get non-existing dungeon %ld!",func_name,(long)plyr_num);
        return INVALID_DUNGEON;
    }
    return &(game.dungeon[plyr_num]);
}

TbBool dungeon_invalid(const struct Dungeon *dungeon)
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
      LbMemorySet(&game.dungeon[i], 0, sizeof(struct Dungeon));
  }
  LbMemorySet(&bad_dungeon, 0, sizeof(struct Dungeon));
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

void increase_dungeon_area(unsigned char plyr_idx, long value)
{
    struct Dungeon *dungeon;
    if (plyr_idx == game.neutral_player_num)
        return;
    dungeon = get_dungeon(plyr_idx);
    dungeon->total_area += value;
}

void player_add_offmap_gold(long plyr_idx, long value)
{
    struct Dungeon *dungeon;
    if (plyr_idx == game.neutral_player_num)
    {
        WARNLOG("Cannot give gold to neutral player %ld",plyr_idx);
        return;
    }
    dungeon = get_dungeon(plyr_idx);
    dungeon->offmap_money_owned += value;
    dungeon->total_money_owned += value;
}

/** Returns if given player owns a room of given kind.
 *
 * @param plyr_idx
 * @param rkind Room kind being checked.
 * @return
 */
TbBool player_has_room(long plyr_idx, RoomKind rkind)
{
    struct Dungeon *dungeon;
    if (plyr_idx == game.neutral_player_num)
        return false;
    dungeon = get_players_num_dungeon(plyr_idx);
    return (dungeon->room_kind[rkind] > 0);
}

/** Returns if given dungeon contains a room of given kind.
 *
 * @param dungeon Target dungeon.
 * @param rkind Room kind being checked.
 * @return
 */
TbBool dungeon_has_room(const struct Dungeon *dungeon, RoomKind rkind)
{
    if (dungeon_invalid(dungeon))
        return false;
    return (dungeon->room_kind[rkind] > 0);
}

TbBool player_creature_tends_to(PlayerNumber plyr_idx, unsigned short tend_type)
{
    const struct Dungeon *dungeon;
    if (plyr_idx == game.neutral_player_num)
        return false;
    dungeon = get_players_num_dungeon(plyr_idx);
    switch (tend_type)
    {
    case CrTend_Imprison:
        return ((dungeon->creature_tendencies & 0x01) != 0);
    case CrTend_Flee:
        return ((dungeon->creature_tendencies & 0x02) != 0);
    default:
        ERRORLOG("Bad tendency type %d",(int)tend_type);
        return false;
    }
}

TbBool toggle_creature_tendencies(struct PlayerInfo *player, unsigned short tend_type)
{
  struct Dungeon *dungeon;
  dungeon = get_dungeon(player->id_number);
  switch (tend_type)
  {
  case CrTend_Imprison:
      dungeon->creature_tendencies ^= 0x01;
      return true;
  case CrTend_Flee:
      dungeon->creature_tendencies ^= 0x02;
      return true;
  default:
      ERRORLOG("Can't toggle tendency; bad tendency type %d",(int)tend_type);
      return false;
  }
}

TbBool set_creature_tendencies(struct PlayerInfo *player, unsigned short tend_type, TbBool val)
{
  struct Dungeon *dungeon;
  dungeon = get_dungeon(player->id_number);
  if (dungeon_invalid(dungeon)) {
      ERRORLOG("Can't set tendency; player %d has no dungeon.",(int)player->id_number);
      return false;
  }
  switch (tend_type)
  {
  case CrTend_Imprison:
      set_flag_byte(&dungeon->creature_tendencies, 0x01, val);
      return true;
  case CrTend_Flee:
      set_flag_byte(&dungeon->creature_tendencies, 0x02, val);
      return true;
  default:
      ERRORLOG("Can't set tendency; bad tendency type %d",(int)tend_type);
      return false;
  }
}

TbBool set_trap_buildable_and_add_to_amount(long plyr_idx, long trap_kind, long buildable, long amount)
{
    struct Dungeon *dungeon;
    if ( (trap_kind <= 0) || (trap_kind >= TRAP_TYPES_COUNT) ) {
        ERRORDBG(1,"Can't set trap availability; invalid trap kind %d.",(int)trap_kind);
        return false;
    }
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Can't set trap availability; player %d has no dungeon.",(int)plyr_idx);
        return false;
    }
    dungeon->trap_buildable[trap_kind] = buildable;
    dungeon->trap_amount[trap_kind] += amount;
    if (amount > 0)
        dungeon->trap_placeable[trap_kind] = true;
    return true;
}

TbBool set_door_buildable_and_add_to_amount(long plyr_idx, long door_kind, long buildable, long amount)
{
    struct Dungeon *dungeon;
    if ( (door_kind <= 0) || (door_kind >= DOOR_TYPES_COUNT) ) {
        ERRORDBG(1,"Can't set door availability; invalid door kind %d.",(int)door_kind);
        return false;
    }
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Can't set door availability; player %d has no dungeon.",(int)plyr_idx);
        return false;
    }
    dungeon->door_buildable[door_kind] = buildable;
    dungeon->door_amount[door_kind] += amount;
    if (amount > 0)
      dungeon->door_placeable[door_kind] = true;
    return true;
}

TbBool restart_script_timer(long plyr_idx, long timer_id)
{
    struct Dungeon *dungeon;
    if ( (timer_id < 0) || (timer_id >= TURN_TIMERS_COUNT) ) {
        ERRORLOG("Can't restart timer; invalid timer id %d.",(int)timer_id);
        return false;
    }
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Can't restart timer; player %d has no dungeon.",(int)plyr_idx);
        return false;
    }
    dungeon->turn_timers[timer_id].state = 1;
    dungeon->turn_timers[timer_id].count = game.play_gameturn;
    return true;
}

TbBool set_script_flag(long plyr_idx, long flag_id, long value)
{
    struct Dungeon *dungeon;
    if ( (flag_id < 0) || (flag_id >= SCRIPT_FLAGS_COUNT) ) {
        ERRORLOG("Can't set flag; invalid flag id %d.",(int)flag_id);
        return false;
    }
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Can't set flag; player %d has no dungeon.",(int)plyr_idx);
        return false;
    }
    dungeon->script_flags[flag_id] = value;
    return true;
}
/******************************************************************************/
