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
#include "pre_inc.h"
#include "map_locations.h"
#include "globals.h"
#include "game_merge.h"
#include "game_legacy.h"
#include "bflib_math.h"
#include "player_instances.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

const struct NamedCommand head_for_desc[] = {
  {"ACTION_POINT",         MLoc_ACTIONPOINT},
  {"DUNGEON",              MLoc_PLAYERSDUNGEON},
  {"DUNGEON_HEART",        MLoc_PLAYERSHEART},
  {"APPROPIATE_DUNGEON",   MLoc_APPROPRTDUNGEON}, //bullfrog spelling, kept until 2025. Keep for legacy.
  {"APPROPRIATE_DUNGEON",  MLoc_APPROPRTDUNGEON},
  {NULL,                   0},
};

TbMapLocation get_coord_encoded_location(MapSubtlCoord stl_x,MapSubtlCoord stl_y)
{
    return ((stl_x & 0x0FFF) << 20) + ((stl_y & 0x0FFF) << 8) + MLoc_COORDS;
}

TbBool get_coords_at_location(struct Coord3d *pos, TbMapLocation location, TbBool random_factor)
{

    long i = get_map_location_longval(location);

    switch (get_map_location_type(location))
    {
    case MLoc_ACTIONPOINT:
        return get_coords_at_action_point(pos, i, random_factor);

    case MLoc_HEROGATE:
        return get_coords_at_hero_door(pos, i, random_factor);

    case MLoc_PLAYERSHEART:
        return get_coords_at_dungeon_heart(pos, i);

    case MLoc_METALOCATION:
        return get_coords_at_meta_action(pos, 0, i);

    case MLoc_COORDS:
        pos->x.val = subtile_coord_center(location >> 20);
        pos->y.val = subtile_coord_center(((location >> 8) & 0xFFF));
        pos->z.val = get_floor_height_at(pos);
      return true;

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
    struct Coord3d targetpos = {0};
    PlayerNumber loc_player = i & 0xF;
    if (loc_player == 15) // CURRENT_PLAYER
        loc_player = game.script_current_player;

    struct Dungeon* dungeon = get_dungeon(loc_player);

    switch (i >> 8)
    {
    case MML_LAST_EVENT:
        src = &game.triggered_object_location;
        break;
    case MML_RECENT_COMBAT:
        src = &dungeon->last_combat_location;
        break;
    case MML_LAST_DEATH_EVENT:
        src = &dungeon->last_eventful_death_location;
        break;
    case MML_LAST_TRAP_EVENT:
        src = &dungeon->last_trap_event_location;
        break;
    case MML_ACTIVE_CTA:
        if ((dungeon->cta_stl_x == 0) && (dungeon->cta_stl_y == 0))
            return false;
        targetpos.x.val = subtile_coord_center(dungeon->cta_stl_x);
        targetpos.y.val = subtile_coord_center(dungeon->cta_stl_y);
        targetpos.z.val = get_floor_height_at(pos);
        src = &targetpos;
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
    if (!thing_exists(heartng))
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
        long distance = GAME_RANDOM(apt->range);
        long direction = GAME_RANDOM(DEGREES_360);
        long delta_x = (distance * LbSinL(direction) >> 8);
        long delta_y = (distance * LbCosL(direction) >> 8);
        pos->x.val = apt->mappos.x.val + (delta_x >> 8);
        pos->y.val = apt->mappos.y.val - (delta_y >> 8);
    }
    pos->z.val = get_floor_height_at(pos);
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
        snprintf(name, MAX_TEXT_LENGTH, "%d", apt->num);
        };return true;
    case MLoc_HEROGATE:{
        i = get_map_location_longval(location);
        if (i <= 0) {
            break;
        }
        snprintf(name, MAX_TEXT_LENGTH, "%ld", -i);
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
        WARNMSG("%s: Action Point %lu location not found",func_name,i);
      break;
    case MLoc_HEROGATE:
      thing = find_hero_gate_of_number(i);
      if (!thing_is_invalid(thing))
      {
        *pos = thing->mappos;
      } else
        WARNMSG("%s: Hero Gate %lu location not found",func_name,i);
      break;
    case MLoc_PLAYERSHEART:
      if (i < PLAYERS_COUNT)
      {
        thing = get_player_soul_container(i);
      } else
        thing = INVALID_THING;
      if (thing_exists(thing))
      {
        *pos = thing->mappos;
      } else
        WARNMSG("%s: Dungeon Heart location for player %lu not found",func_name,i);
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
        WARNMSG("%s: Thing %lu location not found",func_name,i);
      break;
    case MLoc_METALOCATION:
      if (!get_coords_at_meta_action(pos, plyr_idx, i))
        WARNMSG("%s: Metalocation not found %lu",func_name,i);
      break;
    case MLoc_COORDS:
        pos->x.val = subtile_coord_center(location >> 20);
        pos->y.val = subtile_coord_center((location >> 8) & 0xFFF);
        pos->z.val = 0;
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
  SYNCDBG(15,"From %s; Location %ld, pos(%u,%u)",func_name, location, pos->x.stl.num, pos->y.stl.num);
}

/**
 * Returns playernumber included withing brackets from location string from script.
 * @param locname
 * @return Playernumber, or -1 on error.
 */
PlayerNumber get_player_name_from_location_string(const char* locname)
{
    char player_string[COMMAND_WORD_LEN];
    const char* start = strchr(locname, '[');
    if (start == NULL) {
        // Square bracket not found
        return -1;
    }

    start++; // Move past '['
    const char* end = strchr(start, ']');
    if (end == NULL) {
        // Closing square bracket not found
        return -1;
    }

    // Extract the player number string
    strncpy(player_string, start, min(end - start, sizeof(player_string) - 1));
    player_string[end - start] = '\0';
    return get_rid(player_desc, player_string);
}

/**
 * Returns location id for 1-param location from script.
 * @param locname
 * @param location
 * @return
 * @see get_map_heading_id()
 */
#define get_map_location_id(locname, location) get_map_location_id_f(locname, location, __func__, text_line_number)
TbBool get_map_location_id_f(const char *locname, TbMapLocation *location, const char *func_name, long ln_num)
{
    // If there's no locname, then coordinates are set directly as (x,y)
    if (locname == NULL || *locname == '\0')
    {
      *location = MLoc_NONE;
      return true;
    }
    // Player name means the location of player's Dungeon Heart
    long i = get_rid(player_desc, locname);
    if (i != -1)
    {
      if ((i != ALL_PLAYERS) && (i != PLAYER_NEUTRAL)) {
          if (!player_has_heart(i)) {
              WARNMSG("%s(line %lu): Target player %d has no heart",func_name,ln_num, (int)i);
          }
          *location = ((unsigned long)i << 4) | MLoc_PLAYERSHEART;
      } else {
          *location = MLoc_NONE;
      }
      return true;
    }
    // Creature name means location of such creature belonging to player0
    i = get_rid(creature_desc, locname);
    if (i != -1)
    {
        *location = ((unsigned long)i << 12) | ((unsigned long)my_player_number << 4) | MLoc_CREATUREKIND;
        return true;
    }
    // Room name means location of such room belonging to player0
    i = get_rid(room_desc, locname);
    if (i != -1)
    {
        *location = ((unsigned long)i << 12) | ((unsigned long)my_player_number << 4) | MLoc_ROOMKIND;
        return true;
    }
    // Todo list of functions
    if (strcmp(locname, "LAST_EVENT") == 0)
    {
        *location = (((unsigned long)MML_LAST_EVENT) << 12)
            | (((unsigned long)CurrentPlayer) << 4) //TODO: other players
            | MLoc_METALOCATION;
        return true;
    }
    else if (strncmp(locname, "COMBAT", strlen("COMBAT")) == 0)
    {
        if (strcmp(locname, "COMBAT") == 0)
        {
            if (game.game_kind == GKind_MultiGame)
            {
                WARNLOG(" %s (line %lu) : LOCATION = '%s' cannot be used on Multiplayer maps", func_name, ln_num, locname);
                i = PLAYER0;
            }
            else
            {
                i = my_player_number;
            }
        }
        else
        {
            i = get_player_name_from_location_string(locname);
            if (i == -1)
            {
                ERRORMSG("%s(line %lu): Invalid LOCATION = '%s'", func_name, ln_num, locname);
                *location = MLoc_NONE;
                return false;
            }
        }
        *location = (((unsigned long)MML_RECENT_COMBAT) << 12)
            | ((unsigned long)i << 4)
            | MLoc_METALOCATION;
        return true;
    }
    else if (strncmp(locname, "LAST_DEATH_EVENT", strlen("LAST_DEATH_EVENT")) == 0)
    {
        if (strcmp(locname, "LAST_DEATH_EVENT") == 0)
        {
            if (game.game_kind == GKind_MultiGame)
            {
                WARNLOG(" %s (line %lu) : LOCATION = '%s' cannot be used on Multiplayer maps", func_name, ln_num, locname);
                i = PLAYER0;
            }
            else
            {
                i = my_player_number;
            }
        }
        else
        {
            i = get_player_name_from_location_string(locname);
            if (i == -1)
            {
                ERRORMSG("%s(line %lu): Invalid LOCATION = '%s'", func_name, ln_num, locname);
                *location = MLoc_NONE;
                return false;
            }
        }
        *location = (((unsigned long)MML_LAST_DEATH_EVENT) << 12)
            | ((unsigned long)i << 4)
            | MLoc_METALOCATION;
        return true;
    }
    else if (strncmp(locname, "LAST_TRAP_EVENT", strlen("LAST_TRAP_EVENT")) == 0)
    {
        if (strcmp(locname, "LAST_TRAP_EVENT") == 0)
        {
            if (game.game_kind == GKind_MultiGame)
            {
                WARNLOG(" %s (line %lu) : LOCATION = '%s' cannot be used on Multiplayer maps", func_name, ln_num, locname);
                i = PLAYER0;
            }
            else
            {
                i = my_player_number;
            }
        }
        else
        {
            i = get_player_name_from_location_string(locname);
            if (i == -1)
            {
                ERRORMSG("%s(line %lu): Invalid LOCATION = '%s'", func_name, ln_num, locname);
                *location = MLoc_NONE;
                return false;
            }
        }
        *location = (((unsigned long)MML_LAST_TRAP_EVENT) << 12)
            | ((unsigned long)i << 4)
            | MLoc_METALOCATION;
        return true;
    }
    else if (strncmp(locname, "CTA", strlen("CTA")) == 0)
    {
        if (strcmp(locname, "CTA") == 0)
        {
            if (game.game_kind == GKind_MultiGame)
            {
                WARNLOG(" %s (line %lu) : LOCATION = '%s' cannot be used on Multiplayer maps", func_name, ln_num, locname);
                i = PLAYER0;
            }
            else
            {
                i = my_player_number;
            }
        }
        else
        {
            i = get_player_name_from_location_string(locname);
            if (i == -1)
            {
                ERRORMSG("%s(line %lu): Invalid LOCATION = '%s'", func_name, ln_num, locname);
                *location = MLoc_NONE;
                return false;
            }
        }
        *location = (((unsigned long)MML_ACTIVE_CTA) << 12)
            | ((unsigned long)i << 4)
            | MLoc_METALOCATION;
        return true;
    }
    i = atol(locname);
    // Negative number means Hero Gate
    if (i < 0)
    {
        long n = -i;
        struct Thing* thing = find_hero_gate_of_number(n);
        if (thing_is_invalid(thing))
        {
            ERRORMSG("%s(line %lu): Non-existing Hero Door, no %d",func_name,ln_num,(int)-i);
            *location = MLoc_NONE;
            return false;
        }
        *location = (((unsigned long)n) << 4) | MLoc_HEROGATE;
    } else
    // Positive number means Action Point
    if (i > 0)
    {
        long n = action_point_number_to_index(i);
        if (!action_point_exists_idx(n))
        {
            ERRORMSG("%s(line %lu): Non-existing Action Point, no %d",func_name,ln_num,(int)i);
            *location = MLoc_NONE;
            return false;
        }
        // Set to action point number
        *location = (((unsigned long)n) << 4) | MLoc_ACTIONPOINT;
    } else
    // Zero is an error; reset to no location
    {
      ERRORMSG("%s(line %lu): Invalid LOCATION = '%s'",func_name,ln_num, locname);
      *location = MLoc_NONE;
    }
    return true;
}

/**
 * Returns location id for 2-param tunneler heading from script.
 * @param headname
 * @param target
 * @param location
 * @return
 * @see get_map_location_id()
 */
#define get_map_heading_id(headname, target, location) get_map_heading_id_f(headname, target, location, __func__, text_line_number)
TbBool get_map_heading_id_f(const char *headname, long target, TbMapLocation *location, const char *func_name, long ln_num)
{
    // If there's no headname, then there's an error
    if (headname == NULL)
    {
        SCRPTERRLOG("No heading objective");
        *location = MLoc_NONE;
        return false;
    }
    long head_id = get_rid(head_for_desc, headname);
    if (head_id == -1)
    {
        SCRPTERRLOG("Unhandled heading objective, '%s'", headname);
        *location = MLoc_NONE;
        return false;
    }
    // Check if the target place exists, and set 'location'
    // Note that we only need to support enum items which are in head_for_desc[].
    switch (head_id)
    {
    case MLoc_ACTIONPOINT:
    {
        long n = action_point_number_to_index(target);
        *location = ((unsigned long)n << 4) | head_id;
        if (!action_point_exists_idx(n)) {
            SCRPTWRNLOG("Target action point no %d doesn't exist", (int)target);
        }
        return true;
    }
    case MLoc_PLAYERSDUNGEON:
    case MLoc_PLAYERSHEART:
        *location = ((unsigned long)target << 4) | head_id;
        if (!player_has_heart(target)) {
            SCRPTWRNLOG("Target player %d has no heart", (int)target);
        }
        return true;
    case MLoc_APPROPRTDUNGEON:
        *location = (0) | head_id; // This option has no 'target' value
        return true;
    default:
        *location = MLoc_NONE;
        SCRPTWRNLOG("Unsupported Heading objective %d", (int)head_id);
        break;
    }
    return false;
}

// TODO: replace this function by find_location_pos
void find_map_location_coords(TbMapLocation location, int32_t *x, int32_t *y, int plyr_idx, const char *func_name)
{
    struct ActionPoint *apt;
    struct Thing *thing;
    struct Coord3d pos;

    long pos_x;
    long pos_y;
    long i;
    SYNCDBG(15,"From %s; Location %ld, pos(%d,%d)",func_name, location, *x, *y);
    pos_y = 0;
    pos_x = 0;
    i = get_map_location_longval(location);
    switch (get_map_location_type(location))
    {
    case MLoc_ACTIONPOINT:
        // Location stores action point index
        apt = action_point_get(i);
        if (!action_point_is_invalid(apt))
        {
          pos_y = apt->mappos.y.stl.num;
          pos_x = apt->mappos.x.stl.num;
        } else
          WARNMSG("%s: Action Point %ld location not found",func_name,i);
        break;
    case MLoc_HEROGATE:
        thing = find_hero_gate_of_number(i);
        if (!thing_is_invalid(thing))
        {
          pos_y = thing->mappos.y.stl.num;
          pos_x = thing->mappos.x.stl.num;
        } else
          WARNMSG("%s: Hero Gate %ld location not found",func_name,i);
        break;
    case MLoc_PLAYERSHEART:
        if (i < PLAYERS_COUNT)
        {
            thing = get_player_soul_container(i);
        } else
          thing = INVALID_THING;
        if (thing_exists(thing))
        {
          pos_y = thing->mappos.y.stl.num;
          pos_x = thing->mappos.x.stl.num;
        } else
          WARNMSG("%s: Dungeon Heart location for player %ld not found",func_name,i);
        break;
    case MLoc_NONE:
        pos_y = *y;
        pos_x = *x;
        break;
    case MLoc_THING:
        thing = thing_get(i);
        if (!thing_is_invalid(thing))
        {
          pos_y = thing->mappos.y.stl.num;
          pos_x = thing->mappos.x.stl.num;
        } else
          WARNMSG("%s: Thing %ld location not found",func_name,i);
        break;
    case MLoc_METALOCATION:
        if (get_coords_at_meta_action(&pos, plyr_idx, i))
        {
            pos_x = pos.x.stl.num;
            pos_y = pos.y.stl.num;
        }
        else
          WARNMSG("%s: Metalocation not found %ld",func_name,i);
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
    *y = pos_y;
    *x = pos_x;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
