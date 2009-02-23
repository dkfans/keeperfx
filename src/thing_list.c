/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_list.c
 *     Things list support.
 * @par Purpose:
 *     Create and maintain list of things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Feb 2009 - 24 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_list.h"

#include "bflib_basics.h"
#include "globals.h"
#include "packets.h"
#include "lvl_script.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
DLLIMPORT void _DK_update_creatures_not_in_list(void);
DLLIMPORT long _DK_update_things_in_list(struct StructureList *list);
DLLIMPORT void _DK_update_things(void);

/******************************************************************************/
unsigned long update_things_in_list(struct StructureList *list)
{
  static const char *func_name="update_things_in_list";
  struct Thing *thing;
  unsigned long k;
  unsigned long sum;
  int i;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //return _DK_update_things_in_list(list);
  sum = 0;
  i = list->field_4;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->next_of_class;
    if ((thing->field_0 & 0x40) == 0)
    {
        if ((thing->field_0 & 0x10) != 0)
          update_thing_animation(thing);
        else
          update_thing(thing);
    }
    sum += get_thing_checksum(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
#if (BFDEBUG_LEVEL > 19)
    LbSyncLog("%s: Finished\n",func_name);
#endif
  return sum;
}

/*
 * Updates cave in things, using proper StructureList.
 * Returns amount of items in the list.
 */
unsigned long update_cave_in_things(void)
{
  static const char *func_name="update_cave_in_things";
  struct Thing *thing;
  unsigned long k;
  int i;
  i = game.thing_lists[10].field_4;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4576,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->next_of_class;
    update_cave_in(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4577,"Infinite loop detected when sweeping things list");
      break;
    }
  }
  return k;
}

/*
 * Updates sounds of things from given StructureList.
 * Returns amount of items in the list.
 */
unsigned long update_things_sounds_in_list(struct StructureList *list)
{
  static const char *func_name="update_things_sounds_in_list";
  struct Thing *thing;
  unsigned long k;
  int i;
  i = list->field_4;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->next_of_class;
    update_thing_sound(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
  return k;
}

void update_creatures_not_in_list(void)
{
  static const char *func_name="update_creatures_not_in_list";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_update_creatures_not_in_list();
}

void update_things(void)
{
  static const char *func_name="update_things";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_update_things(); return;
  unsigned long sum;
  struct PlayerInfo *player;
  optimised_lights = 0;
  total_lights = 0;
  do_lights = game.field_4614D;
  sum = 0;
  sum += update_things_in_list(&game.thing_lists[0]);
  update_creatures_not_in_list();
  sum += update_things_in_list(&game.thing_lists[7]);
  sum += update_things_in_list(&game.thing_lists[1]);
  sum += update_things_in_list(&game.thing_lists[2]);
  sum += update_things_in_list(&game.thing_lists[5]);
  sum += update_things_in_list(&game.thing_lists[3]);
  sum += update_things_in_list(&game.thing_lists[4]);
  sum += update_things_in_list(&game.thing_lists[6]);
  sum += update_things_in_list(&game.thing_lists[8]);
  update_things_sounds_in_list(&game.thing_lists[9]);
  update_cave_in_things();
  sum += compute_players_checksum();
  sum += game.field_14BB4A;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  game.packets[player->field_B%PACKETS_COUNT].field_4 = sum;
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void init_player_start(struct PlayerInfo *player)
{
  static const char *func_name="init_player_start";
  struct Dungeon *dungeon;
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[2].field_4;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->next_of_class;
    if ((game.objects_config[thing->model].field_6) && (thing->owner == player->field_2B))
    {
      dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
      dungeon->field_0 = thing->field_1B;
      memcpy(&dungeon->mappos,&thing->mappos,sizeof(struct Coord3d));
      break;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
}

void init_traps(void)
{
  static const char *func_name="init_traps";
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[7].field_4;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->next_of_class;
    if (thing->field_13 == 0)
    {
      thing->field_13 = game.traps_config[thing->model].field_8;
      thing->field_4F ^= (thing->field_4F ^ (trap_stats[thing->model].field_12 << 4)) & 0x30;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
}

void setup_computer_players(void)
{
  static const char *func_name="setup_computer_players";
  struct PlayerInfo *player;
  struct Thing *thing;
  int plr_idx;
  int i,k;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  for (plr_idx=0; plr_idx<PLAYERS_COUNT; plr_idx++)
  {
      player=&(game.players[plr_idx]);
      if ((player->field_0 & 0x01) == 0)
      {
        k = 0;
        i = game.thing_lists[2].field_4;
        while (i>0)
        {
          if (i >= THINGS_COUNT)
          {
            error(func_name,4578,"Jump out of things array bounds deteted");
            break;
          }
          thing = game.things_lookup[i];
          if ((thing == game.things_lookup[0]) || (thing == NULL))
            break;
          i = thing->next_of_class;
          if ((game.objects_config[thing->model].field_6) && (thing->owner == plr_idx))
          {
            script_support_setup_player_as_computer_keeper(plr_idx, 0);
            break;
          }
          k++;
          if (k > THINGS_COUNT)
          {
            error(func_name,4579,"Infinite loop detected when sweeping things list");
            break;
          }
        }
      }
  }
}

void init_all_creature_states(void)
{
  static const char *func_name="init_all_creature_states";
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[0].field_4;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->next_of_class;
    init_creature_state(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
