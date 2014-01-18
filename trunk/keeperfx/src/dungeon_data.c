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
DLLIMPORT struct Room *_DK_player_has_room_of_type(long plr_idx, long roomkind);
/******************************************************************************/
struct Dungeon bad_dungeon;
/******************************************************************************/
struct Dungeon *get_players_num_dungeon_f(long plyr_idx,const char *func_name)
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
    return &(game.dungeon[(int)plyr_num]);
}

struct Dungeon *get_players_dungeon_f(const struct PlayerInfo *player,const char *func_name)
{
    PlayerNumber plyr_num;
    plyr_num = player->id_number;
    if (player_invalid(player) || (plyr_num < 0) || (plyr_num >= DUNGEONS_COUNT))
    {
        ERRORLOG("%s: Tried to get non-existing dungeon %ld!",func_name,(long)plyr_num);
        return INVALID_DUNGEON;
    }
    return &(game.dungeon[(int)plyr_num]);
}

struct Dungeon *get_dungeon_f(PlayerNumber plyr_num,const char *func_name)
{
    if ((plyr_num < 0) || (plyr_num >= DUNGEONS_COUNT))
    {
        ERRORLOG("%s: Tried to get non-existing dungeon %ld!",func_name,(long)plyr_num);
        return INVALID_DUNGEON;
    }
    return &(game.dungeon[(int)plyr_num]);
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

void decrease_dungeon_area(PlayerNumber plyr_idx, long value)
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

void increase_room_area(PlayerNumber plyr_idx, long value)
{
    struct Dungeon *dungeon;
    if (plyr_idx == game.neutral_player_num)
        return;
    dungeon = get_dungeon(plyr_idx);
    dungeon->field_949 += value;
    dungeon->total_area += value;
}

void decrease_room_area(PlayerNumber plyr_idx, long value)
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

void increase_dungeon_area(PlayerNumber plyr_idx, long value)
{
    struct Dungeon *dungeon;
    if (plyr_idx == game.neutral_player_num)
        return;
    dungeon = get_dungeon(plyr_idx);
    dungeon->total_area += value;
}

void player_add_offmap_gold(PlayerNumber plyr_idx, long value)
{
    struct Dungeon *dungeon;
    if (plyr_idx == game.neutral_player_num)
    {
        WARNLOG("Cannot give gold to neutral player %d",(int)plyr_idx);
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
TbBool player_has_room(PlayerNumber plyr_idx, RoomKind rkind)
{
    struct Dungeon *dungeon;
    if (plyr_idx == game.neutral_player_num)
        return false;
    dungeon = get_players_num_dungeon(plyr_idx);
    return (dungeon->room_kind[rkind] > 0);
}

struct Thing *get_player_soul_container(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon->dnheart_idx > 0) {
        return thing_get(dungeon->dnheart_idx);
    }
    return INVALID_THING;
}

TbBool player_has_heart(PlayerNumber plyr_idx)
{
    return thing_exists(get_player_soul_container(plyr_idx));
}

/** Returns if given dungeon contains a room of given kind.
 *
 * @param dungeon Target dungeon.
 * @param rkind Room kind being checked.
 * @return
 */
TbBool dungeon_has_room(const struct Dungeon *dungeon, RoomKind rkind)
{
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    if ((rkind < 1) || (rkind >= ROOM_TYPES_COUNT)) {
        return false;
    }
    return (dungeon->room_kind[rkind] > 0);
}

struct Room *player_has_room_of_type(PlayerNumber plyr_idx, RoomKind rkind)
{
    //return _DK_player_has_room_of_type(plyr_idx, rkind);
    struct Dungeon *dungeon;
    if (plyr_idx == game.neutral_player_num)
        return false;
    dungeon = get_players_num_dungeon(plyr_idx);
    return room_get(dungeon->room_kind[rkind]);
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

TbBool set_trap_buildable_and_add_to_amount(PlayerNumber plyr_idx, ThingModel tngmodel, long buildable, long amount)
{
    struct Dungeon *dungeon;
    if ( (tngmodel <= 0) || (tngmodel >= TRAP_TYPES_COUNT) ) {
        ERRORDBG(1,"Can't set trap availability; invalid trap kind %d.",(int)tngmodel);
        return false;
    }
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Can't set trap availability; player %d has no dungeon.",(int)plyr_idx);
        return false;
    }
    if (buildable)
        dungeon->trap_build_flags[tngmodel] |= MnfBldF_Manufacturable;
    //TODO SCRIPT This is wrong. We need a list of "instantly buildable" (off-map) traps
    //dungeon->trap_amount_stored[trap_kind] += amount;
    dungeon->trap_amount_placeable[tngmodel] += amount;
    if (amount > 0)
        dungeon->trap_build_flags[tngmodel] |= MnfBldF_Built;
    return true;
}

TbBool set_door_buildable_and_add_to_amount(PlayerNumber plyr_idx, ThingModel door_kind, long buildable, long amount)
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
    if (buildable)
        dungeon->door_build_flags[door_kind] |= MnfBldF_Manufacturable;
    dungeon->door_amount_placeable[door_kind] += amount;
    if (amount > 0)
      dungeon->door_build_flags[door_kind] |= MnfBldF_Built;
    return true;
}

TbBool restart_script_timer(PlayerNumber plyr_idx, long timer_id)
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

TbBool set_script_flag(PlayerNumber plyr_idx, long flag_id, long value)
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

void init_dungeon_essential_position(struct Dungeon *dungeon)
{
    struct Room *room;
    room = room_get(dungeon->room_kind[RoK_DUNGHEART]);
    RoomKind rkind;
    for (rkind = 1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        if (!room_is_invalid(room))
            break;
        room = room_get(dungeon->room_kind[rkind]);
    }
    if (room_is_invalid(room)) {
        dungeon->essential_pos.x.val = subtile_coord_center(map_subtiles_x/2);
        dungeon->essential_pos.y.val = subtile_coord_center(map_subtiles_y/2);
        dungeon->essential_pos.z.val = subtile_coord(0,1);
        return;
    }
    dungeon->essential_pos.x.val = subtile_coord_center(room->central_stl_x);
    dungeon->essential_pos.y.val = subtile_coord_center(room->central_stl_y);
    dungeon->essential_pos.z.val = subtile_coord(0,1);
}

void init_dungeons_essential_position(void)
{
    int i;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
        struct Dungeon *dungeon;
        dungeon = get_dungeon(i);
        init_dungeon_essential_position(dungeon);
    }
}

const struct Coord3d *dungeon_get_essential_pos(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon->dnheart_idx > 0) {
        struct Thing *heartng;
        heartng = thing_get(dungeon->dnheart_idx);
        if (thing_exists(heartng)) {
            return &heartng->mappos;
        }
    }
    return &dungeon->essential_pos;
}

void init_dungeons(void)
{
    int i,k;
    struct Dungeon *dungeon;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
        dungeon = get_dungeon(game.hero_player_num);
        dungeon->hates_player[i] = game.fight_max_hate;
        dungeon = get_dungeon(i);
        dungeon->hates_player[game.hero_player_num%DUNGEONS_COUNT] = game.fight_max_hate;
        dungeon->num_active_diggers = 0;
        dungeon->num_active_creatrs = 0;
        dungeon->creatr_list_start = 0;
        dungeon->digger_list_start = 0;
        dungeon->owner = i;
        dungeon->max_creatures_attracted = game.default_max_crtrs_gen_entrance;
        dungeon->dead_creatures_count = 0;
        dungeon->dead_creature_idx = 0;
        for (k=0; k < DUNGEONS_COUNT; k++)
        {
          if (k == i)
            dungeon->hates_player[k] = game.fight_max_love;
          else
            dungeon->hates_player[k] = game.fight_max_hate;
        }
        LbMemorySet(dungeon->creature_models_joined, 0, CREATURE_TYPES_COUNT);
    }
}

/******************************************************************************/
