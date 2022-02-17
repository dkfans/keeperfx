/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_locations.c
 *     Map location functions.
 * @par Purpose:
 *     Functions related to locations of points of intrest on the map
 * @par Comment:
 *     None.
 * @author   KeeperFx Team
 * @date     7 Feb 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "map_locations.h"
#include "globals.h"
#include "game_merge.h"
#include "game_legacy.h"
#include "bflib_math.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/


TbBool get_coords_at_location(struct Coord3d *pos, TbMapLocation location)
{

    long i = get_map_location_longval(location);

    switch (get_map_location_type(location))
    {
    case MLoc_ACTIONPOINT:
        return get_coords_at_action_point(pos, i, 1);

    case MLoc_HEROGATE:
        return get_coords_at_hero_door(pos, i, 1);

    case MLoc_PLAYERSHEART:
        return get_coords_at_dungeon_heart(pos, i);
        
    case MLoc_METALOCATION:
        return get_coords_at_meta_action(pos, 0, i);
        
    case MLoc_CREATUREKIND:
    case MLoc_OBJECTKIND:
    case MLoc_ROOMKIND:
    case MLoc_THING:
    case MLoc_PLAYERSDUNGEON:
    case MLoc_APPROPRTDUNGEON:
    case MLoc_DOORKIND:
    case MLoc_TRAPKIND:
    case MLoc_NONE:
    default:
        return false;
    }
    
}


TbBool get_coords_at_meta_action(struct Coord3d *pos, PlayerNumber target_plyr_idx, long i)
{
    
    SYNCDBG(7,"Starting with loc:%ld", i);
    struct Coord3d *src;
    PlayerNumber loc_player = i & 0xF;
    if (loc_player == 15) // CURRENT_PLAYER
        loc_player = gameadd.script_current_player;

    struct DungeonAdd* dungeonadd = get_dungeonadd(loc_player);

    switch (i >> 8)
    {
    case MML_LAST_EVENT:
        src = &gameadd.triggered_object_location;
        break;
    case MML_RECENT_COMBAT:
        src = &dungeonadd->last_combat_location;
        break;
    default:
        return false;
    }

    pos->x.val = src->x.val + PLAYER_RANDOM(target_plyr_idx, 33) - 16;
    pos->y.val = src->y.val + PLAYER_RANDOM(target_plyr_idx, 33) - 16;
    pos->z.val = src->z.val;

    return true;
    
}

TbBool get_coords_at_hero_door(struct Coord3d *pos, long gate_num, unsigned char random_factor)
{
    SYNCDBG(7,"Starting at HG%d", (int)gate_num);
    if (gate_num <= 0)
    {
        ERRORLOG("Script error - invalid hero gate index %d",(int)gate_num);
        return false;
    }
    struct Thing* gatetng = find_hero_gate_of_number(gate_num);
    if (thing_is_invalid(gatetng))
    {
        ERRORLOG("Script error - attempt to create thing at non-existing hero gate index %d",(int)gate_num);
        return false;
    }
    pos->x.val = gatetng->mappos.x.val;
    pos->y.val = gatetng->mappos.y.val;
    pos->z.val = gatetng->mappos.z.val + 384;
    return true;
}


/**
 * Creates a thing on given players dungeon heart.
 * Originally was script_support_create_creature_at_dungeon_heart().
 * @param plyr_idx
 */
TbBool get_coords_at_dungeon_heart(struct Coord3d *pos, PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Starting at player %d", (int)plyr_idx);
    struct Thing* heartng = get_player_soul_container(plyr_idx);
    TRACE_THING(heartng);
    if (thing_is_invalid(heartng))
    {
        ERRORLOG("Script error - attempt to create thing in player %d dungeon with no heart",(int)plyr_idx);
        return false;
    }
    pos->x.val = heartng->mappos.x.val + PLAYER_RANDOM(plyr_idx, 65) - 32;
    pos->y.val = heartng->mappos.y.val + PLAYER_RANDOM(plyr_idx, 65) - 32;
    pos->z.val = heartng->mappos.z.val;
    return true;
}

TbBool get_coords_at_action_point(struct Coord3d *pos, long apt_idx, unsigned char random_factor)
{
    SYNCDBG(7,"Starting at action point %d", (int)apt_idx);

    struct ActionPoint* apt = action_point_get(apt_idx);
    if (!action_point_exists(apt))
    {
        ERRORLOG("Script error - attempt to create thing at non-existing action point %d",(int)apt_idx);
        return false;
    }

    if ( (random_factor == 0) || (apt->range == 0) )
    {
        pos->x.val = apt->mappos.x.val;
        pos->y.val = apt->mappos.y.val;
    } else
    {
        long direction = GAME_RANDOM(2 * LbFPMath_PI);
        long delta_x = (apt->range * LbSinL(direction) >> 8);
        long delta_y = (apt->range * LbCosL(direction) >> 8);
        pos->x.val = apt->mappos.x.val + (delta_x >> 8);
        pos->y.val = apt->mappos.y.val - (delta_y >> 8);
    }
    return true;
}

unsigned short get_map_location_type(TbMapLocation location)
{
  return location & 0x0F;
}

unsigned long get_map_location_longval(TbMapLocation location)
{
  return (location >> 4);
}

unsigned long get_map_location_plyrval(TbMapLocation location)
{
  return (location >> 12);
}

unsigned short get_map_location_plyridx(TbMapLocation location)
{
  return (location >> 4) & 0xFF;
}

/**
 * Writes Code Name (name to use in script file) of given map location to buffer.
 * @ name Output buffer. It should be COMMAND_WORD_LEN long.
 */
TbBool get_map_location_code_name(TbMapLocation location, char *name)
{
    long i;
    switch (get_map_location_type(location))
    {
    case MLoc_ACTIONPOINT:{
        i = get_map_location_longval(location);
        struct ActionPoint* apt = action_point_get(i);
        if (apt->num <= 0) {
            break;
        }
        itoa(apt->num, name, 10);
        };return true;
    case MLoc_HEROGATE:{
        i = get_map_location_longval(location);
        if (i <= 0) {
            break;
        }
        itoa(-i, name, 10);
        };return true;
    case MLoc_PLAYERSHEART:{
        i = get_map_location_longval(location);
        const char* cnstname = get_conf_parameter_text(player_desc, i);
        if (cnstname[0] == '\0') {
            break;
        }
        strcpy(name, cnstname);
        };return true;
    case MLoc_CREATUREKIND:{
        i = get_map_location_plyrval(location);
        const char* cnstname = get_conf_parameter_text(creature_desc, i);
        if (cnstname[0] == '\0') {
            break;
        }
        strcpy(name, cnstname);
        };return true;
    case MLoc_ROOMKIND:{
        i = get_map_location_plyrval(location);
        const char* cnstname = get_conf_parameter_text(room_desc, i);
        if (cnstname[0] == '\0') {
            break;
        }
        strcpy(name, cnstname);
        };return true;
    case MLoc_OBJECTKIND:
    case MLoc_THING:
    case MLoc_PLAYERSDUNGEON:
    case MLoc_APPROPRTDUNGEON:
    case MLoc_DOORKIND:
    case MLoc_TRAPKIND:
    case MLoc_NONE:
    default:
        break;
    }
    strcpy(name, "INVALID");
    return false;
}


// TODO: z location
void find_location_pos(long location, PlayerNumber plyr_idx, struct Coord3d *pos, const char *func_name)
{
  struct ActionPoint *apt;
  struct Thing *thing;
  unsigned long i = get_map_location_longval(location);
  memset(pos, 0, sizeof(*pos));

  switch (get_map_location_type(location))
  {
    case MLoc_ACTIONPOINT:
      // Location stores action point index
      apt = action_point_get(i);
      if (!action_point_is_invalid(apt))
      {
        pos->x.val = apt->mappos.x.val;
        pos->y.val = apt->mappos.y.val;
      } else
        WARNMSG("%s: Action Point %d location not found",func_name,i);
      break;
    case MLoc_HEROGATE:
      thing = find_hero_gate_of_number(i);
      if (!thing_is_invalid(thing))
      {
        *pos = thing->mappos;
      } else
        WARNMSG("%s: Hero Gate %d location not found",func_name,i);
      break;
    case MLoc_PLAYERSHEART:
      if (i < PLAYERS_COUNT)
      {
        thing = get_player_soul_container(i);
      } else
        thing = INVALID_THING;
      if (!thing_is_invalid(thing))
      {
        *pos = thing->mappos;
      } else
        WARNMSG("%s: Dungeon Heart location for player %d not found",func_name,i);
      break;
    case MLoc_NONE:
      pos->x.val = 0;
      pos->y.val = 0;
      pos->z.val = 0;
      break;
    case MLoc_THING:
      thing = thing_get(i);
      if (!thing_is_invalid(thing))
      {
        *pos = thing->mappos;
      } else
        WARNMSG("%s: Thing %d location not found",func_name,i);
      break;
    case MLoc_METALOCATION:
      if (!get_coords_at_meta_action(pos, plyr_idx, i))
        WARNMSG("%s: Metalocation not found %d",func_name,i);
      break;
    case MLoc_CREATUREKIND:
    case MLoc_OBJECTKIND:
    case MLoc_ROOMKIND:
    case MLoc_PLAYERSDUNGEON:
    case MLoc_APPROPRTDUNGEON:
    case MLoc_DOORKIND:
    case MLoc_TRAPKIND:
    default:
      WARNMSG("%s: Unsupported location, %lu.",func_name,location);
      break;
  }
  SYNCDBG(15,"From %s; Location %ld, pos(%ld,%ld)",func_name, location, pos->x.stl.num, pos->y.stl.num);
}



/******************************************************************************/
#ifdef __cplusplus
}
#endif
