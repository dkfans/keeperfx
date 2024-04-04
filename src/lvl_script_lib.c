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
#include "magic.h"
#include "post_inc.h"

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
    set_flag_value(value->flags, TrgF_REUSABLE, next_command_reusable);
    clear_flag(value->flags, TrgF_DISABLED);
    value->valtype = var_index;
    value->plyr_range = plr_range_id;
    value->condit_idx = get_script_current_condition();
}

struct Thing *script_process_new_object(ThingModel tngmodel, TbMapLocation location, long arg, unsigned long plr_range_id)
{
    
    int tngowner = plr_range_id;
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
    if (thing_is_dungeon_heart(thing))
    {
        struct Dungeon* dungeon = get_dungeon(tngowner);
        if (dungeon->backup_heart_idx == 0)
        {
            dungeon->backup_heart_idx = thing->index;
        }
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
    if (thing_is_special_box(thing) && !thing_is_hardcoded_special_box(thing))
    {
        thing->custom_box.box_kind = (unsigned char)arg;
    }
    switch (tngmodel)
    {
        case ObjMdl_GoldChest:
        case ObjMdl_GoldPot:
        case ObjMdl_Goldl:
            thing->valuable.gold_stored = arg;
            break;
    }
    return thing;
}

struct Thing* script_process_new_effectgen(ThingModel tngmodel, TbMapLocation location, long range)
{
    struct Coord3d pos;
    const unsigned char tngclass = TCls_EffectGen;
    if (!get_coords_at_location(&pos, location))
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

/**
 * Casts keeper power on a specific creature, or position of the creature depending on the power.
 * @param thing The creature to target.
 * @param pwkind The ID of the Keeper Power.
 * @param splevel The overcharge level of the keeperpower. Is ignored when not applicable.
 * @param caster The player number of the player who is made to cast the spell.
 * @param is_free If gold is used when casting the spell. It will fail to cast if it is not free and money is not available.
 * @return TbResult whether the spell was successfully cast
 */
TbResult script_use_power_on_creature(struct Thing* thing, short pwkind, short splevel, PlayerNumber caster, TbBool is_free)
{
    if (thing_is_in_power_hand_list(thing, thing->owner))
    {
        char block = pwkind == PwrK_SLAP;
        block |= pwkind == PwrK_CALL2ARMS;
        block |= pwkind == PwrK_CAVEIN;
        block |= pwkind == PwrK_LIGHTNING;
        block |= pwkind == PwrK_MKDIGGER;
        block |= pwkind == PwrK_SIGHT;
        if (block)
        {
            SYNCDBG(5, "Found creature to use power on but it is being held.");
            return Lb_FAIL;
        }
    }

    MapSubtlCoord stl_x = thing->mappos.x.stl.num;
    MapSubtlCoord stl_y = thing->mappos.y.stl.num;
    unsigned long spell_flags = is_free ? PwMod_CastForFree : 0;

    switch (pwkind)
    {
    case PwrK_HEALCRTR:
        return magic_use_power_heal(caster, thing, 0, 0, splevel, spell_flags);
    case PwrK_SPEEDCRTR:
        return magic_use_power_speed(caster, thing, 0, 0, splevel, spell_flags);
    case PwrK_PROTECT:
        return magic_use_power_armour(caster, thing, 0, 0, splevel, spell_flags);
    case PwrK_REBOUND:
        return magic_use_power_rebound(caster, thing, 0, 0, splevel, spell_flags);
    case PwrK_CONCEAL:
        return magic_use_power_conceal(caster, thing, 0, 0, splevel, spell_flags);
    case PwrK_DISEASE:
        return magic_use_power_disease(caster, thing, 0, 0, splevel, spell_flags);
    case PwrK_CHICKEN:
        return magic_use_power_chicken(caster, thing, 0, 0, splevel, spell_flags);
    case PwrK_FREEZE:
        return magic_use_power_freeze(caster, thing, 0, 0, splevel, spell_flags);
    case PwrK_SLOW:
        return magic_use_power_slow(caster, thing, 0, 0, splevel, spell_flags);
    case PwrK_FLIGHT:
        return magic_use_power_flight(caster, thing, 0, 0, splevel, spell_flags);
    case PwrK_VISION:
        return magic_use_power_vision(caster, thing, 0, 0, splevel, spell_flags);
    case PwrK_SLAP:
        return magic_use_power_slap_thing(caster, thing, spell_flags);
    case PwrK_CALL2ARMS:
        return magic_use_power_call_to_arms(caster, stl_x, stl_y, splevel, spell_flags);
    case PwrK_LIGHTNING:
        return magic_use_power_lightning(caster, stl_x, stl_y, splevel, spell_flags);
    case PwrK_CAVEIN:
        return magic_use_power_cave_in(caster, stl_x, stl_y, splevel, spell_flags);
    case PwrK_MKDIGGER:
        return magic_use_power_imp(caster, stl_x, stl_y, spell_flags);
    case PwrK_SIGHT:
        return magic_use_power_sight(caster, stl_x, stl_y, splevel, spell_flags);
    case PwrK_TIMEBOMB:
        return magic_use_power_time_bomb(caster, thing, splevel, spell_flags);
    default:
        SCRPTERRLOG("Power not supported for this command: %s", power_code_name(pwkind));
        return Lb_FAIL;
    }
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
        dungeon->box_info.activated[var_idx] = saturate_set_unsigned(new_val, 8);
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

struct Thing* script_get_creature_by_criteria(PlayerNumber plyr_idx, ThingModel crmodel, long criteria)
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
        return get_player_creature_in_range_around_own_heart(plyr_idx, crmodel, 11);
    case CSelCrit_NearEnemyHeart:
        return get_player_creature_in_range_around_any_enemy_heart(plyr_idx, crmodel, 11);
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
