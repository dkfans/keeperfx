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
#include "lvl_script_conditions.h"
#include "lvl_script_commands.h"

#include "globals.h"
#include "thing_factory.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "dungeon_data.h"
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