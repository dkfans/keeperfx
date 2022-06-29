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

static int filter_criteria_type(long desc_type)
{
    return desc_type & 0x0F;
}

static long filter_criteria_loc(long desc_type)
{
    return desc_type >> 4;
}

struct Thing* script_get_creature_by_criteria(PlayerNumber plyr_idx, long crmodel, long criteria)
{
    switch (filter_criteria_type(criteria))
    {
    case CSelCrit_Any:
        return get_random_players_creature_of_model(plyr_idx, crmodel);
    case CSelCrit_MostExperienced:
        return find_players_highest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Any, plyr_idx, 0);
    case CSelCrit_MostExpWandering:
        return find_players_highest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Wandering, plyr_idx, 0);
    case CSelCrit_MostExpWorking:
        return find_players_highest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Working, plyr_idx, 0);
    case CSelCrit_MostExpFighting:
        return find_players_highest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Fighting, plyr_idx, 0);
    case CSelCrit_LeastExperienced:
        return find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Any, plyr_idx, 0);
    case CSelCrit_LeastExpWandering:
        return find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Wandering, plyr_idx, 0);
    case CSelCrit_LeastExpWorking:
        return find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Working, plyr_idx, 0);
    case CSelCrit_LeastExpFighting:
        return find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Fighting, plyr_idx, 0);
    case CSelCrit_NearOwnHeart:
    {
        const struct Coord3d* pos = dungeon_get_essential_pos(plyr_idx);
        return get_creature_near_and_owned_by(pos->x.val, pos->y.val, plyr_idx, crmodel);
    }
    case CSelCrit_NearEnemyHeart:
        return get_creature_in_range_around_any_of_enemy_heart(plyr_idx, crmodel, 11);
    case CSelCrit_OnEnemyGround:
        return get_random_players_creature_of_model_on_territory(plyr_idx, crmodel, 0);
    case CSelCrit_OnFriendlyGround:
        return get_random_players_creature_of_model_on_territory(plyr_idx, crmodel, 1);
    case CSelCrit_OnNeutralGround:
        return get_random_players_creature_of_model_on_territory(plyr_idx, crmodel, 2);
    case CSelCrit_NearAP:
    {
        int loc = filter_criteria_loc(criteria);
        struct ActionPoint* apt = action_point_get(loc);
        if (!action_point_exists(apt))
        {
            WARNLOG("Action point is invalid:%d", apt->num);
            return INVALID_THING;
        }
        if (apt->range == 0)
        {
            WARNLOG("Action point with zero range:%d", apt->num);
            return INVALID_THING;
        }
        // Action point range should be inside spiral in subtiles
        int dist = 2 * coord_subtile(apt->range + COORD_PER_STL - 1) + 1;
        dist = dist * dist;

        Thing_Maximizer_Filter filter = near_map_block_creature_filter_diagonal_random;
        struct CompoundTngFilterParam param;
        param.model_id = crmodel;
        param.plyr_idx = (unsigned char)plyr_idx;
        param.num1 = apt->mappos.x.val;
        param.num2 = apt->mappos.y.val;
        param.num3 = apt->range;
        return get_thing_spiral_near_map_block_with_filter(apt->mappos.x.val, apt->mappos.y.val,
            dist,
            filter, &param);
    }
    default:
        ERRORLOG("Invalid level up criteria %d", (int)criteria);
        return INVALID_THING;
    }
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

#define get_player_id(plrname, plr_range_id) get_player_id_f(plrname, plr_range_id, __func__, text_line_number)
TbBool get_player_id_f(const char *plrname, long *plr_range_id, const char *func_name, long ln_num)
{
    *plr_range_id = get_rid(player_desc, plrname);
    if (*plr_range_id == -1)
    {
      *plr_range_id = get_rid(cmpgn_human_player_options, plrname);
      if (*plr_range_id == -1)
      {
        ERRORMSG("%s(line %lu): Invalid player name, '%s'",func_name,ln_num, plrname);
        return false;
      }
    }
    return true;
}

#ifdef __cplusplus
}
#endif