/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_lib.c
 *     collection of functions used by multiple files under lvl_script_*
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * @author   KeeperFX Team
 */
/******************************************************************************/

#include "lvl_script_lib.h"
#include "globals.h"
#include "thing_factory.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "dungeon_data.h"
#include "lvl_script_conditions.h"
#include "lvl_filesdk1.h"

#ifdef __cplusplus
extern "C" {
#endif
struct ScriptValue *allocate_script_value(void)
{
    if (gameadd.script.values_num >= SCRIPT_VALUES_COUNT)
        return NULL;
    struct ScriptValue* value = &gameadd.script.values[gameadd.script.values_num];
    gameadd.script.values_num++;
    return value;
}

void command_init_value(struct ScriptValue* value, unsigned long var_index, unsigned long plr_range_id)
{
    set_flag_byte(&value->flags, TrgF_REUSABLE, next_command_reusable);
    set_flag_byte(&value->flags, TrgF_DISABLED, false);
    value->valtype = var_index;
    value->plyr_range = plr_range_id;
    value->condit_idx = get_script_current_condition();
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
    if (locname == NULL)
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
    else if (strcmp(locname, "COMBAT") == 0)
    {
        *location = (((unsigned long)MML_RECENT_COMBAT) << 12)
            | ((unsigned long)my_player_number << 4)
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

void command_add_value(unsigned long var_index, unsigned long plr_range_id, long val2, long val3, long val4)
{
    ALLOCATE_SCRIPT_VALUE(var_index, plr_range_id);

    value->arg0 = val2;
    value->arg1 = val3;
    value->arg2 = val4;

    if ((get_script_current_condition() == CONDITION_ALWAYS) && (next_command_reusable == 0))
    {
        script_process_value(var_index, plr_range_id, val2, val3, val4, value);
        return;
    }
}

long parse_creature_name(const char *creature_name)
{
    long ret = get_rid(creature_desc, creature_name);
    if (ret == -1)
    {
        if (0 == strcasecmp(creature_name, "ANY_CREATURE"))
        {
            return CREATURE_ANY;
        }
    }
    return ret;
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

struct Thing *script_process_new_object(long tngmodel, TbMapLocation location, long arg)
{
    long i = get_map_location_longval(location);
    int tngowner = 5; // Neutral
    struct Coord3d pos;

    const unsigned char tngclass = TCls_Object;

    // TODO: move into a function
    switch (get_map_location_type(location))
    {
    case MLoc_ACTIONPOINT:
        if (!get_coords_at_action_point(&pos, i, 1))
        {
            return INVALID_THING;
        }
        break;
    case MLoc_HEROGATE:
        if (!get_coords_at_hero_door(&pos, i, 1))
        {
            return INVALID_THING;
        }
        break;
    case MLoc_PLAYERSHEART:
        if (!get_coords_at_dungeon_heart(&pos, i))
        {
            return INVALID_THING;
        }
        break;
    case MLoc_METALOCATION:
        if (!get_coords_at_meta_action(&pos, 0, i))
        {
            return INVALID_THING;
        }
        break;
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
        return INVALID_THING;
    }
    struct Thing* thing = create_thing(&pos, tngclass, tngmodel, tngowner, -1);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Couldn't create %s at location %d",thing_class_and_model_name(tngclass, tngmodel),(int)location);
        return INVALID_THING;
    }
    thing->mappos.z.val = get_thing_height_at(thing, &thing->mappos);
    // Try to move thing out of the solid wall if it's inside one
    if (thing_in_wall_at(thing, &thing->mappos))
    {
        if (!move_creature_to_nearest_valid_position(thing)) {
            ERRORLOG("The %s was created in wall, removing",thing_model_name(thing));
            delete_thing_structure(thing, 0);
            return INVALID_THING;
        }
    }
    switch (tngmodel)
    {
        case OBJECT_TYPE_SPECBOX_CUSTOM: // Custom box from SPECBOX_HIDNWRL
            thing->custom_box.box_kind = (unsigned char)arg;
            break;
        case 3:
        case 6: //GOLD
        case 43:
            thing->valuable.gold_stored = arg;
            break;
    }
    return thing;
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

long parse_criteria(const char *criteria)
{
    char c;
    int arg;

    long ret = get_id(creature_select_criteria_desc, criteria);
    if (ret == -1)
    {
        if (2 == sscanf(criteria, "AT_ACTION_POINT[%d%c", &arg, &c) && (c == ']'))
        {
            ActionPointId loc = action_point_number_to_index(arg);
            if (loc == -1)
            {
                SCRPTERRLOG("Unknown action point at criteria, '%s'", criteria);
                return -1;
            }
            ret = (CSelCrit_NearAP) | (loc << 4);
        }
    }
    return ret;
}

#define get_players_range_single(plr_range_id) get_players_range_single_f(plr_range_id, __func__, text_line_number)
long get_players_range_single_f(long plr_range_id, const char *func_name, long ln_num)
{
    if (plr_range_id < 0) {
        return -1;
    }
    if (plr_range_id == ALL_PLAYERS) {
        return -3;
    }
    if (plr_range_id == PLAYER_GOOD) {
        return game.hero_player_num;
    }
    if (plr_range_id == PLAYER_NEUTRAL) {
        return game.neutral_player_num;
    }
    if (plr_range_id < PLAYERS_COUNT)
    {
        return plr_range_id;
    }
    return -2;
}

// Variables that could be set
TbBool parse_set_varib(const char *varib_name, long *varib_id, long *varib_type)
{
    char c;
    int len = 0;
    char arg[MAX_TEXT_LENGTH];

    *varib_id = -1;
    if (*varib_id == -1)
    {
      *varib_id = get_id(flag_desc, varib_name);
      *varib_type = SVar_FLAG;
    }
    if (*varib_id == -1)
    {
      *varib_id = get_id(campaign_flag_desc, varib_name);
      *varib_type = SVar_CAMPAIGN_FLAG;
    }
    if (*varib_id == -1)
    {
        if (2 == sscanf(varib_name, "BOX%ld_ACTIVATE%c", varib_id, &c) && (c == 'D'))
        {
            // activateD
            *varib_type = SVar_BOX_ACTIVATED;
        }
        else
        {
            *varib_id = -1;
        }
        if (2 == sscanf(varib_name, "SACRIFICED[%n%[^]]%c", &len, arg, &c) && (c == ']'))
        {
            *varib_id = get_id(creature_desc, arg);
            *varib_type = SVar_SACRIFICED;
        }
        if (2 == sscanf(varib_name, "REWARDED[%n%[^]]%c", &len, arg, &c) && (c == ']'))
        {
            *varib_id = get_id(creature_desc, arg);
            *varib_type = SVar_REWARDED;
        }
    }
    if (*varib_id == -1)
    {
      SCRPTERRLOG("Unknown variable name, '%s'", varib_name);
      return false;
    }
    return true;
}

TbBool parse_get_varib(const char *varib_name, long *varib_id, long *varib_type)
{
    char c;
    int len = 0;
    char arg[MAX_TEXT_LENGTH];

    if (level_file_version > 0)
    {
        *varib_type = get_id(variable_desc, varib_name);
    } else
    {
        *varib_type = get_id(dk1_variable_desc, varib_name);
    }
    if (*varib_type == -1)
      *varib_id = -1;
    else
      *varib_id = 0;
    if (*varib_id == -1)
    {
      *varib_id = get_id(creature_desc, varib_name);
      *varib_type = SVar_CREATURE_NUM;
    }
    //TODO: list of lambdas
    if (*varib_id == -1)
    {
      *varib_id = get_id(room_desc, varib_name);
      *varib_type = SVar_ROOM_SLABS;
    }
    if (*varib_id == -1)
    {
      *varib_id = get_id(timer_desc, varib_name);
      *varib_type = SVar_TIMER;
    }
    if (*varib_id == -1)
    {
      *varib_id = get_id(flag_desc, varib_name);
      *varib_type = SVar_FLAG;
    }
    if (*varib_id == -1)
    {
      *varib_id = get_id(door_desc, varib_name);
      *varib_type = SVar_DOOR_NUM;
    }
    if (*varib_id == -1)
    {
        *varib_id = get_id(trap_desc, varib_name);
        *varib_type = SVar_TRAP_NUM;
    }
    if (*varib_id == -1)
    {
      *varib_id = get_id(campaign_flag_desc, varib_name);
      *varib_type = SVar_CAMPAIGN_FLAG;
    }
    if (*varib_id == -1)
    {
        if (2 == sscanf(varib_name, "BOX%ld_ACTIVATE%c", varib_id, &c) && (c == 'D'))
        {
            // activateD
            *varib_type = SVar_BOX_ACTIVATED;
        }
        else
        {
            *varib_id = -1;
        }
        if (2 == sscanf(varib_name, "SACRIFICED[%n%[^]]%c", &len, arg, &c) && (c == ']'))
        {
            *varib_id = get_id(creature_desc, arg);
            *varib_type = SVar_SACRIFICED;
        }
        if (2 == sscanf(varib_name, "REWARDED[%n%[^]]%c", &len, arg, &c) && (c == ']'))
        {
            *varib_id = get_id(creature_desc, arg);
            *varib_type = SVar_REWARDED;
        }
    }
    if (*varib_id == -1)
    {
      SCRPTERRLOG("Unknown variable name, '%s'", varib_name);
      return false;
    }
    return true;
}

#ifdef __cplusplus
}
#endif