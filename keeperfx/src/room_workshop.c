/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_workshop.c
 *     Workshop room maintain functions.
 * @par Purpose:
 *     Functions to create and use workshops.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_workshop.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "config_terrain.h"
#include "power_hand.h"
#include "game_legacy.h"
#include "gui_soundmsgs.h"
#include "player_instances.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_remove_workshop_item(long a1, long a2, long a3);
DLLIMPORT long _DK_remove_workshop_object_from_player(long a1, long a2);
DLLIMPORT long _DK_get_next_manufacture(struct Dungeon *dungeon);
DLLIMPORT long _DK_process_player_manufacturing(int plr_idx);
/******************************************************************************/
/**
 * Stores manufacturable items.
 * Was originally named trap_data.
 */
struct ManufactureData manufacture_data[] = {
    {PSt_None,      TCls_Empty,0,   0,   0},
    {PSt_PlaceTrap, TCls_Trap, 1, 130, 152},
    {PSt_PlaceTrap, TCls_Trap, 2, 132, 154},
    {PSt_PlaceTrap, TCls_Trap, 3, 134, 156},
    {PSt_PlaceTrap, TCls_Trap, 4, 136, 158},
    {PSt_PlaceTrap, TCls_Trap, 5, 138, 160},
    {PSt_PlaceTrap, TCls_Trap, 6, 140, 162},
    {PSt_PlaceDoor, TCls_Door, 1, 144, 166},
    {PSt_PlaceDoor, TCls_Door, 2, 146, 168},
    {PSt_PlaceDoor, TCls_Door, 3, 148, 170},
    {PSt_PlaceDoor, TCls_Door, 4, 150, 172},
};
/******************************************************************************/


#ifdef __cplusplus
}
#endif
/******************************************************************************/
/**
 * Returns manufacture data for a given manufacture index.
 * @param manufctr_idx Manufacture array index.
 * @return Dummy entry pinter if not found, manufacture data pointer otherwise.
 */
struct ManufactureData *get_manufacture_data(int manufctr_idx)
{
    if ((manufctr_idx < 0) || (manufctr_idx >= MANUFCTR_TYPES_COUNT)) {
        return &manufacture_data[0];
    }
    return &manufacture_data[manufctr_idx];
}

/**
 * Finds index into manufactures data array for a given trap/door class and model.
 * @param tngclass Manufacturable thing class.
 * @param tngmodel Manufacturable thing model.
 * @return 0 if not found, otherwise index where 1 <= index < MANUFCTR_TYPES_COUNT
 */
int get_manufacture_data_index_for_thing(ThingClass tngclass, ThingModel tngmodel)
{
    int i;
    for (i=1; i < MANUFCTR_TYPES_COUNT; i++)
    {
        struct ManufactureData *manufctr;
        manufctr = &manufacture_data[i];
        if ((manufctr->tngclass == tngclass) && (manufctr->tngmodel == tngmodel)) {
            return i;
        }
    }
    return 0;
}

/**
 * Returns manufacture data for a given trap/door class and model.
 * @param tngclass Manufacturable thing class.
 * @param tngmodel Manufactureble thing model.
 * @return Dummy entry pinter if not found, manufacture data pointer otherwise.
 */
struct ManufactureData *get_manufacture_data_for_thing(ThingClass tngclass, ThingModel tngmodel)
{
    int i;
    for (i=1; i < MANUFCTR_TYPES_COUNT; i++)
    {
        struct ManufactureData *manufctr;
        manufctr = &manufacture_data[i];
        if ((manufctr->tngclass == tngclass) && (manufctr->tngmodel == tngmodel)) {
            return manufctr;
        }
    }
    return &manufacture_data[0];
}

TbBool add_workshop_object_to_workshop(struct Room *room,struct Thing *cratetng)
{
    if (room->kind != RoK_WORKSHOP) {
        SYNCDBG(4,"Crate %s owned by player %d can't be placed in a %s owned by player %d, expected proper workshop",thing_model_name(cratetng),(int)cratetng->owner,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    return add_item_to_room_capacity(room);
}

TbBool remove_workshop_object_from_workshop(struct Room *room,struct Thing *cratetng)
{
    if ( (room->kind != RoK_WORKSHOP) || (cratetng->owner != room->owner) ) {
        SYNCDBG(4,"Crate %s owned by player %d found in a %s owned by player %d, instead of proper workshop",thing_model_name(cratetng),(int)cratetng->owner,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    return remove_item_from_room_capacity(room);
}

TbBool add_workshop_item(long plyr_idx, long wrkitm_class, long wrkitm_kind)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Can't add item; player %d has no dungeon.",(int)plyr_idx);
        return false;
    }
    switch (wrkitm_class)
    {
    case TCls_Trap:
        dungeon->trap_amount[wrkitm_kind]++;
        dungeon->trap_placeable[wrkitm_kind] = true;
        break;
    case TCls_Door:
        dungeon->door_amount[wrkitm_kind]++;
        dungeon->door_placeable[wrkitm_kind] = true;
        break;
    default:
        ERRORLOG("Can't add item; illegal item class %d",(int)wrkitm_class);
        return false;
    }
    return true;
}

TbBool check_workshop_item_limit_reached(long plyr_idx, long wrkitm_class, ThingModel tngmodel)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
        return true;
    switch (wrkitm_class)
    {
    case TCls_Trap:
        return (dungeon->trap_amount[tngmodel] >= MANUFACTURED_ITEMS_LIMIT);
    case TCls_Door:
        return (dungeon->door_amount[tngmodel] >= MANUFACTURED_ITEMS_LIMIT);
    }
    return true;
}

TbBool set_manufacture_level(struct Dungeon *dungeon)
{
    int wrkshp_slabs;
    wrkshp_slabs = count_slabs_of_room_type(dungeon->owner, RoK_WORKSHOP);
    if (wrkshp_slabs <= 9)
    {
        dungeon->manufacture_level = 0;
    } else
    if (wrkshp_slabs <= 16)
    {
        dungeon->manufacture_level = 1;
    } else
    if (wrkshp_slabs <= 25)
    {
        if (wrkshp_slabs == 20) // why there's special code for 20 slabs!?
            dungeon->manufacture_level = 4;
        else
            dungeon->manufacture_level = 2;
    } else
    if (wrkshp_slabs <= 36)
    {
        dungeon->manufacture_level = 3;
    } else
    {
        dungeon->manufacture_level = 4;
    }
    return true;
}

struct Thing *get_workshop_box_thing(long owner, long model)
{
    struct Thing *thing;
    int i,k;
    k = 0;
    i = game.thing_lists[TngList_Objects].index;
    while (i > 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if ( ((thing->alloc_flags & TAlF_Exists) != 0) && (thing->model == model) && (thing->owner == owner) )
        {
            if (!thing_is_picked_up(thing))
                return thing;
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return INVALID_THING;
}

long remove_workshop_item(long owner, long tngclass, long tngmodel)
{
    SYNCDBG(8,"Starting");
    return _DK_remove_workshop_item(owner, tngclass, tngmodel);
}

long remove_workshop_object_from_player(PlayerNumber owner, ThingModel objmodel)
{
    struct Thing *thing;
    struct Room *room;
    //return _DK_remove_workshop_object_from_player(a1, a2);
    thing = get_workshop_box_thing(owner, objmodel);
    if (thing_is_invalid(thing))
        return 0;
    room = get_room_thing_is_on(thing);
    if (room_exists(room)) {
        remove_workshop_object_from_workshop(room,thing);
    } else {
        WARNLOG("Crate thing index %d isn't placed existing room; removing anyway",(int)thing->index);
    }
    create_effect(&thing->mappos, imp_spangle_effects[thing->owner], thing->owner);
    delete_thing_structure(thing, 0);
    return 1;
}

/**
 * Finds a doable manufacture which has minimal amoint of items ready to use.
 * Returns that minimal amount of items, and manufacture kind related to it.
 * @param dungeon The dungeon which is to be checked.
 * @param mnfctr_class Class of the manufacture with minimal items available.
 * @param mnfctr_kind Kind of the manufacture with minimal items available.
 * @return Gives minimal amount of items available, or LONG_MAX if no doable manufacture was found.
 */
long get_doable_manufacture_with_minimal_amount_available(const struct Dungeon *dungeon, int * mnfctr_class, int * mnfctr_kind)
{
    int chosen_class,chosen_kind,chosen_amount,chosen_level;
    struct ManfctrConfig *mconf;
    int tng_kind;
    long amount;
    chosen_class = TCls_Empty;
    chosen_kind = 0;
    chosen_amount = LONG_MAX;
    chosen_level = LONG_MAX;
    // Try getting door kind for manufacture
    for (tng_kind = 1; tng_kind < DOOR_TYPES_COUNT; tng_kind++)
    {
        mconf = &game.doors_config[tng_kind];
        if ( (dungeon->door_buildable[tng_kind]) && (dungeon->manufacture_level >= mconf->manufct_level) )
        {
            amount = dungeon->door_amount[tng_kind];
            if ( (chosen_amount > amount) ||
                ((chosen_amount == amount) && (chosen_level > mconf->manufct_level)) )
            {
                chosen_class = TCls_Door;
                chosen_amount = dungeon->door_amount[tng_kind];
                chosen_kind = tng_kind;
                chosen_level = mconf->manufct_level;
            }
        }
    }
    // Try getting trap kind for manufacture
    for (tng_kind = 1; tng_kind < TRAP_TYPES_COUNT; tng_kind++)
    {
        mconf = &game.traps_config[tng_kind];
        if ( (dungeon->trap_buildable[tng_kind]) && (dungeon->manufacture_level >= mconf->manufct_level) )
        {
            amount = dungeon->trap_amount[tng_kind];
            if ( (chosen_amount > amount) ||
                ((chosen_amount == amount) && (chosen_level > mconf->manufct_level)) )
            {
                chosen_class = TCls_Trap;
                chosen_amount = dungeon->trap_amount[tng_kind];
                chosen_kind = tng_kind;
                chosen_level = mconf->manufct_level;
            }
        }
    }
    if (chosen_class != TCls_Empty)
    {
        if (mnfctr_class != NULL)
            *mnfctr_class = chosen_class;
        if (mnfctr_kind != NULL)
            *mnfctr_kind = chosen_kind;
    }
    return chosen_amount;
}

TbBool get_next_manufacture(struct Dungeon *dungeon)
{
    int chosen_class,chosen_kind,chosen_amount;
    //return _DK_get_next_manufacture(dungeon);
    set_manufacture_level(dungeon);
    chosen_class = TCls_Empty;
    chosen_kind = 0;
    chosen_amount = get_doable_manufacture_with_minimal_amount_available(dungeon, &chosen_class, &chosen_kind);
    if (chosen_amount >= MANUFACTURED_ITEMS_LIMIT)
    {
        WARNDBG(6,"Player %d reached manufacture limit for all items",(int)dungeon->owner);
        return false;
    }
    if (chosen_class != TCls_Empty)
    {
        SYNCDBG(8,"Player %d manufacturing class %d kind %d",(int)dungeon->owner,(int)chosen_class,(int)chosen_kind);
        dungeon->manufacture_class = chosen_class;
        dungeon->manufacture_kind = chosen_kind;
        return true;
    }
    WARNDBG(6,"Player %d has nothing to manufacture",(int)dungeon->owner);
    return false;
}

long manufacture_points_required(long mfcr_type, unsigned long mfcr_kind, const char *func_name)
{
  switch (mfcr_type)
  {
  case TCls_Trap:
      return game.traps_config[mfcr_kind%TRAP_TYPES_COUNT].manufct_required;
  case TCls_Door:
      return game.doors_config[mfcr_kind%DOOR_TYPES_COUNT].manufct_required;
  default:
      ERRORMSG("%s: Invalid type of manufacture",func_name);
      return 0;
  }
}

short process_player_manufacturing(PlayerNumber plyr_idx)
{
  struct Dungeon *dungeon;
  struct PlayerInfo *player;
  struct Room *room;
  int k;
  SYNCDBG(17,"Starting");
//  return _DK_process_player_manufacturing(plr_idx);

  dungeon = get_players_num_dungeon(plyr_idx);
  room = player_has_room_of_type(plyr_idx, RoK_WORKSHOP);
  if (room_is_invalid(room))
  {
      return true;
  }
  if (dungeon->manufacture_class == TCls_Empty)
  {
      get_next_manufacture(dungeon);
      return true;
  }
  k = manufacture_points_required(dungeon->manufacture_class, dungeon->manufacture_kind, __func__);
  // If we don't have enough manufacture points, don't do anything
  if (dungeon->manufacture_progress < (k << 8))
    return true;
  // Try to do the manufacturing
  room = find_room_with_spare_room_item_capacity(plyr_idx, RoK_WORKSHOP);
  if (room_is_invalid(room))
  {
      dungeon->manufacture_class = TCls_Empty;
      return false;
  }
  if (check_workshop_item_limit_reached(plyr_idx, dungeon->manufacture_class, dungeon->manufacture_kind))
  {
      ERRORLOG("Bad choice for manufacturing - limit reached for %s kind %d",thing_class_code_name(dungeon->manufacture_class),(int)dungeon->manufacture_kind);
      get_next_manufacture(dungeon);
      return false;
  }
  if (create_workshop_object_in_workshop_room(plyr_idx, dungeon->manufacture_class, dungeon->manufacture_kind) == 0)
  {
      ERRORLOG("Could not create manufactured %s kind %d",thing_class_code_name(dungeon->manufacture_class),(int)dungeon->manufacture_kind);
      return false;
  }
  add_workshop_item(plyr_idx, dungeon->manufacture_class, dungeon->manufacture_kind);

  switch (dungeon->manufacture_class)
  {
  case TCls_Trap:
      dungeon->lvstats.manufactured_traps++;
      // If that's local player - make a message
      player=get_my_player();
      if (player->id_number == plyr_idx)
        output_message(SMsg_ManufacturedTrap, 0, true);
      break;
  case TCls_Door:
      dungeon->lvstats.manufactured_doors++;
      // If that's local player - make a message
      player=get_my_player();
      if (player->id_number == plyr_idx)
        output_message(SMsg_ManufacturedDoor, 0, true);
      break;
  default:
      ERRORLOG("Invalid type of new manufacture, %d",(int)dungeon->manufacture_class);
      return false;
  }

  dungeon->manufacture_progress -= (k << 8);
  dungeon->field_118B = game.play_gameturn;
  dungeon->lvstats.manufactured_items++;
  get_next_manufacture(dungeon);
  return true;
}
/******************************************************************************/
