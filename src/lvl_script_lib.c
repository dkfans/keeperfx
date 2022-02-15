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





struct Thing *script_process_new_object(long tngmodel, TbMapLocation location, long arg)
{
    
    int tngowner = 5; // Neutral
    struct Coord3d pos;

    const unsigned char tngclass = TCls_Object;

    if(!get_coords_at_location(&pos,location))
    {
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


char get_player_number_from_value(const char* txt)
{
    char id;
    if (strcasecmp(txt, "None") == 0)
    {
        id = 127;
    }
    else if (strcasecmp(txt, "Kills") == 0)
    {
        id = -114;
    }
    else if (strcasecmp(txt, "Strength") == 0)
    {
        id = -115;
    }
    else if (strcasecmp(txt, "Gold") == 0)
    {
        id = -116;
    }
    else if (strcasecmp(txt, "Wage") == 0)
    {
        id = -117;
    }
    else if (strcasecmp(txt, "Armour") == 0)
    {
        id = -118;
    }
    else if (strcasecmp(txt, "Time") == 0)
    {
        id = -119;
    }
    else if (strcasecmp(txt, "Dexterity") == 0)
    {
        id = -120;
    }
    else if (strcasecmp(txt, "Defence") == 0)
    {
        id = -121;
    }
    else if (strcasecmp(txt, "Luck") == 0)
    {
        id = -122;
    }
    else if (strcasecmp(txt, "Blood") == 0)
    {
        id = -123;
    }
    else
    {
        id = get_rid(player_desc, txt);
    }
    if (id == -1)
    {
        id = get_rid(cmpgn_human_player_options, txt);
        if (id == -1)
        {
            id = get_rid(creature_desc, txt);
            if (id != -1)
            {
                id = (~id) + 1;
            }
            else
            {
                id = get_rid(spell_desc, txt);
                if (id != -1)
                {
                    id = -35 - id;
                }
                else
                {
                    id = get_rid(room_desc, txt);
                    if (id != -1)
                    {
                        id = -78 - id;
                    }
                    else
                    {
                        id = get_rid(power_desc, txt);
                        if (id != -1)
                        {
                            id = -94 - id;
                        }
                        else
                        {
                            id = atoi(txt);
                        }
                    }
                }
            }
        }
    }
    return id;
}

#ifdef __cplusplus
}
#endif