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
#include "pre_inc.h"
#include "lvl_script_lib.h"
#include "lvl_script_conditions.h"
#include "lvl_script_commands.h"

#include "globals.h"
#include "thing_factory.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "dungeon_data.h"
#include "lvl_filesdk1.h"
#include "creature_states_pray.h"
#include "magic_powers.h"
#include "config_creature.h"
#include "gui_msgs.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
struct ScriptValue *allocate_script_value(void)
{
    if (game.script.values_num >= SCRIPT_VALUES_COUNT)
        return NULL;
    struct ScriptValue* value = &game.script.values[game.script.values_num];
    game.script.values_num++;
    return value;
}

void command_init_value(struct ScriptValue* value, unsigned long var_index, unsigned long plr_range_id)
{
    set_flag_value(value->flags, TrgF_REUSABLE, next_command_reusable);
    clear_flag(value->flags, TrgF_DISABLED);
    value->valtype = var_index;
    value->plyr_range = plr_range_id;
    value->condit_idx = get_script_current_condition();
}

// For dynamic strings
long script_strdup(const char *src)
{
    // TODO: add string deduplication to save space

    const long offset = game.script.next_string_offset;
    const long remaining_size = sizeof(game.script.strings) - offset;
    const long string_size = strlen(src) + 1;
    if (string_size >= remaining_size)
    {
        return -1;
    }
    memcpy(&game.script.strings[offset], src, string_size);
    game.script.next_string_offset += string_size;
    return offset;
}

const char * script_strval(long offset)
{
    if (offset >= sizeof(game.script.strings))
    {
        return NULL;
    }
    return &game.script.strings[offset];
}

struct Thing *script_process_new_object(ThingModel tngmodel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long arg, PlayerNumber plyr_idx, short move_angle)
{
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = get_floor_height_at(&pos);
    struct Thing* thing = create_object(&pos, tngmodel, plyr_idx, -1);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Couldn't create %s at location %d, %d",thing_class_and_model_name(TCls_Object, tngmodel),stl_x, stl_y);
        return INVALID_THING;
    }
    thing->move_angle_xy = move_angle;
    if (thing_is_dungeon_heart(thing))
    {
        struct Dungeon* dungeon = get_dungeon(thing->owner);
        if (dungeon->backup_heart_idx == 0)
        {
            dungeon->backup_heart_idx = thing->index;
        }
    }
    // Try to move thing out of the solid wall if it's inside one
    if (thing_in_wall_at(thing, &thing->mappos))
    {
        if (!move_creature_to_nearest_valid_position(thing)) {
            ERRORLOG("The %s was created in wall, removing",thing_model_name(thing));
            delete_thing_structure(thing, 0);
            return INVALID_THING;
        }
    }
    if (thing_is_special_box(thing) && !thing_is_hardcoded_special_box(thing))
    {
        thing->custom_box.box_kind = (unsigned char)arg;
    }
    switch (tngmodel)
    {
        case ObjMdl_GoldChest:
        case ObjMdl_GoldPot:
        case ObjMdl_Goldl:
        case ObjMdl_GoldBag:
            thing->valuable.gold_stored = arg;
            break;
    }
    return thing;
}

struct Thing* script_process_new_effectgen(ThingModel tngmodel, TbMapLocation location, long range)
{
    struct Coord3d pos;
    const unsigned char tngclass = TCls_EffectGen;
    if (!get_coords_at_location(&pos, location, false))
    {
        ERRORLOG("Couldn't find location %d to create %s", (int)location, thing_class_and_model_name(tngclass, tngmodel));
        return INVALID_THING;
    }
    SlabCodedCoords place_slbnum = get_slab_number(subtile_slab(pos.x.stl.num), subtile_slab(pos.y.stl.num));
    struct Thing* thing = create_thing(&pos, tngclass, tngmodel, game.neutral_player_num, place_slbnum);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Couldn't create %s at location %d", thing_class_and_model_name(tngclass, tngmodel), (int)location);
        return INVALID_THING;
    }
    thing->effect_generator.range = range;
    thing->mappos.z.val = get_thing_height_at(thing, &thing->mappos);

    // Try to move thing out of the solid wall if it's inside one
    if (thing_in_wall_at(thing, &thing->mappos))
    {
        if (!move_creature_to_nearest_valid_position(thing)) {
            ERRORLOG("The %s was created in wall, removing", thing_model_name(thing));
            delete_thing_structure(thing, 0);
            return INVALID_THING;
        }
    }
    return thing;
}

void set_variable(int player_idx, long var_type, long var_idx, long new_val)
{
    struct Dungeon *dungeon = get_dungeon(player_idx);
    struct Coord3d pos = {0};

    switch (var_type)
    {
    case SVar_FLAG:
        set_script_flag(player_idx, var_idx, new_val);
        break;
    case SVar_CAMPAIGN_FLAG:
        intralvl.campaign_flags[player_idx][var_idx] = new_val;
        break;
    case SVar_BOX_ACTIVATED:
        dungeon->box_info.activated[var_idx] = saturate_set_unsigned(new_val, 16);
        break;
    case SVar_TRAP_ACTIVATED:
        dungeon->trap_info.activated[var_idx] = saturate_set_unsigned(new_val, 16);
        break;
    case SVar_SACRIFICED:
        dungeon->creature_sacrifice[var_idx] = saturate_set_unsigned(new_val, 8);
        if (find_temple_pool(player_idx, &pos))
        {
            process_sacrifice_creature(&pos, var_idx, player_idx, false);
        }
        break;
    case SVar_REWARDED:
        dungeon->creature_awarded[var_idx] = new_val;
        break;
    default:
        WARNLOG("Unexpected type:%d",(int)var_type);
    }
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
    if (plr_range_id < PLAYERS_COUNT)
    {
        return plr_range_id;
    }
    return -2;
}

void get_chat_icon_from_value(const char* txt, char* id, char* type)
{
    char idx;
    if (strcasecmp(txt, "None") == 0)
    {
        *id = 0;
        *type = MsgType_Blank;
        return;
    }
    else if (strcasecmp(txt, "Kills") == 0)
    {
        *id = 1;
        *type = MsgType_Query;
        return;
    }
    else if (strcasecmp(txt, "Strength") == 0)
    {
        *id = 2;
        *type = MsgType_Query;
        return;
    }
    else if (strcasecmp(txt, "Gold") == 0)
    {
        *id = 3;
        *type = MsgType_Query;
        return;
    }
    else if (strcasecmp(txt, "Wage") == 0)
    {
        *id = 4;
        *type = MsgType_Query;
        return;
    }
    else if (strcasecmp(txt, "Armour") == 0)
    {
        *id = 5;
        *type = MsgType_Query;
        return;
    }
    else if (strcasecmp(txt, "Time") == 0)
    {
        *id = 6;
        *type = MsgType_Query;
        return;
    }
    else if (strcasecmp(txt, "Dexterity") == 0)
    {
        *id = 7;
        *type = MsgType_Query;
        return;
    }
    else if (strcasecmp(txt, "Defence") == 0)
    {
        *id = 8;
        *type = MsgType_Query;
        return;
    }
    else if (strcasecmp(txt, "Luck") == 0)
    {
        *id = 9;
        *type = MsgType_Query;
        return;
    }
    else if (strcasecmp(txt, "Blood") == 0)
    {
        *id = 10;
        *type = MsgType_Query;
        return;
    }
    else
    {
        idx = get_id(player_desc, txt);
    }
    if (idx == -1)
    {
        idx = get_id(cmpgn_human_player_options, txt);
        if (idx == -1)
        {
            idx = get_id(creature_desc, txt);
            if (idx != -1)
            {
                *id = idx;
                *type = MsgType_Creature;
            }
            else
            {
                idx = get_id(spell_desc, txt);
                if (idx != -1)
                {
                    *id = idx;
                    *type = MsgType_CreatureSpell;
                }
                else
                {
                    idx = get_id(room_desc, txt);
                    if (idx != -1)
                    {
                        *id = idx;
                        *type = MsgType_Room;
                    }
                    else
                    {
                        idx = get_id(power_desc, txt);
                        if (idx != -1)
                        {
                            *id = idx;
                            *type = MsgType_KeeperSpell;
                        }
                        else
                        {
                            idx = get_id(instance_desc, txt);
                            if (idx != -1)
                            {
                                *id = idx;
                                *type = MsgType_CreatureInstance;
                            }
                            else
                            {
                                *id = atoi(txt);
                                *type = MsgType_Player;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        *id = idx;
        *type = MsgType_Player;
    }
}

#define get_player_id(plrname, plr_range_id) get_player_id_f(plrname, plr_range_id, __func__, text_line_number)
TbBool get_player_id_f(const char *plrname, int32_t *plr_range_id, const char *func_name, long ln_num)
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

/**
 * Returns hero objective, and also optionally checks the player name between brackets.
 * @param target gets filled with player number, or -1.
 * @return Hero Objective ID
 */
PlayerNumber get_objective_id_with_potential_target(const char* locname, PlayerNumber* target)
{
    char before_bracket[COMMAND_WORD_LEN];
    char player_string[COMMAND_WORD_LEN];
    const char* bracket = strchr(locname, '[');

    if (bracket == NULL) {
        strncpy(before_bracket, locname, sizeof(before_bracket) - 1);
        before_bracket[sizeof(before_bracket) - 1] = '\0';
        return get_rid(hero_objective_desc, before_bracket);
    }

    // Extract text before '['
    size_t len = min((size_t)(bracket - locname), sizeof(before_bracket) - 1);
    strncpy(before_bracket, locname, len);
    before_bracket[len] = '\0';

    // Extract text inside the brackets
    const char* start = bracket + 1;
    const char* end = strchr(start, ']');

    if (end != NULL) {
        size_t string_length = min((size_t)(end - start), sizeof(player_string) - 1);
        strncpy(player_string, start, string_length);
        player_string[string_length] = '\0';

        PlayerNumber plyr_idx = get_rid(player_desc, player_string);
        if (plyr_idx < 0)
            plyr_idx = get_rid(cmpgn_human_player_options, player_string);
        *target = plyr_idx;
    }
    return get_rid(hero_objective_desc, before_bracket);
}

#ifdef __cplusplus
}
#endif
