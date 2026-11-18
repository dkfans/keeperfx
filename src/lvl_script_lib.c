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

#include "globals.h"
#include "config_creature.h"
#include "creature_states_pray.h"
#include "custom_sprites.h"
#include "keeperfx.hpp"
#include "sprites.h"
#include "dungeon_data.h"
#include "gui_msgs.h"
#include "lvl_script_lib.h"
#include "lvl_script_conditions.h"
#include "room_util.h"
#include "thing_corpses.h"
#include "thing_factory.h"
#include "thing_navigate.h"
#include "thing_physics.h"

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
        } else
        {
            struct Thing* backup = thing_get(dungeon->backup_heart_idx);
            if (!thing_is_dungeon_heart(backup))
            {
                ERRORLOG("%s had invalid backup heart %s", player_code_name(plyr_idx), thing_model_name(backup));
                dungeon->backup_heart_idx = thing->index;
            }
        }
    }
    // Try to move thing out of the solid wall if it's inside one
    if (thing_in_wall_at(thing, &thing->mappos))
    {
        if (!move_creature_to_nearest_valid_position(thing)) {
            ERRORLOG("The %s was created in wall, removing",thing_model_name(thing));
            destroy_object(thing);
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
        default:
            struct ObjectConfigStats* objst = get_object_model_stats(tngmodel);
            if (objst->genre == OCtg_GoldHoard)
            {
                if (arg > 0)
                {
                    thing->valuable.gold_stored = arg;
                }
            }
            check_and_asimilate_thing_by_room(thing);
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

struct Thing* script_process_new_corpse(ThingModel tngmodel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, CrtrExpLevel exp_level, TbBool dying)
{
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = get_floor_height_at(&pos);

    int16_t crpscondition = DCrSt_LongDead;
    if (dying)
    {
        crpscondition = DCrSt_Dying;
    }

    struct Thing* thing = create_dead_creature(&pos, tngmodel, crpscondition, plyr_idx, exp_level);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Couldn't create %s at location %d, %d", thing_class_and_model_name(TCls_DeadCreature, tngmodel), stl_x, stl_y);
        return INVALID_THING;
    }
    
    // Try to move thing out of the solid wall if it's inside one
    if (thing_in_wall_at(thing, &thing->mappos))
    {
        if (!move_creature_to_nearest_valid_position(thing))
        {
            ERRORLOG("The %s was created in wall, removing", thing_model_name(thing));
            destroy_thing(thing);
            return INVALID_THING;
        }
    }
    return thing;
}

TbBool script_new_creature_type(const char *name)
{
    if (game.conf.crtr_conf.model_count >= CREATURE_TYPES_MAX)
    {
        SCRPTERRLOG("Cannot increase creature type count for creature type '%s', already at maximum %d types.", name, CREATURE_TYPES_MAX);
        return false;
    }
    for (int j = 0; j < (game.conf.crtr_conf.model_count - 1); j++)
    {
        if (strcmp(creature_desc[j].name, name) == 0)
        {
            SCRPTERRLOG("Trying to add creature type that already exists: %s", name);
            return false;
        }
    }
    int i = game.conf.crtr_conf.model_count;
    game.conf.crtr_conf.model_count++;
    snprintf(game.conf.crtr_conf.model[i].name, COMMAND_WORD_LEN, "%s", name);
    creature_desc[i - 1].name = game.conf.crtr_conf.model[i].name;
    creature_desc[i - 1].num = i;
    
    if (load_default_creaturemodel_config(i, 0))
    {
        SCRPTLOG("Adding creature type %s and increasing creature types to %d", creature_code_name(i), game.conf.crtr_conf.model_count - 1);
        return true;
    }
    else
    {
        SCRPTERRLOG("Failed to load config for creature '%s'(%d).", game.conf.crtr_conf.model[i].name, i);
    }
    return false;
}

TbBool script_copy_creature_type(ThingModel source_id, const char* name)
{
    if (game.conf.crtr_conf.model_count >= CREATURE_TYPES_MAX)
    {
        SCRPTERRLOG("Cannot increase creature type count for creature type '%s', already at maximum %d types.", name, CREATURE_TYPES_MAX);
        return false;
    }
    for (int j = 0; j < (game.conf.crtr_conf.model_count - 1); j++)
    {
        if (strcmp(creature_desc[j].name, name) == 0)
        {
            SCRPTERRLOG("Trying to add creature type that already exists: %s", name);
            return false;
        }
    }
    int i = game.conf.crtr_conf.model_count;
    game.conf.crtr_conf.model_count++;
    

//    init_creature_model_stats(i);
    game.conf.crtr_conf.model[i] = game.conf.crtr_conf.model[source_id];
    snprintf(game.conf.crtr_conf.model[i].name, COMMAND_WORD_LEN, "%s", name);
    creature_desc[i - 1].name = game.conf.crtr_conf.model[i].name;
    creature_desc[i - 1].num = i;
    for (int k = 0; k < CREATURE_GRAPHICS_INSTANCES; k++)
    {
        game.conf.crtr_conf.creature_graphics[i][k] = game.conf.crtr_conf.creature_graphics[source_id][k];
    }
    game.conf.crtr_conf.creature_sounds[i] = game.conf.crtr_conf.creature_sounds[source_id];

    return true;
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

void set_creature_property(ThingModel creature, int32_t property, TbBool enable)
{
    struct CreatureModelConfig* crconf = creature_stats_get(creature);
    switch (property)
    {
    case 1: // BLEEDS
        crconf->bleeds = enable;
        break;
    case 2: // UNAFFECTED_BY_WIND
        if (enable)
        {
            set_flag(crconf->immunity_flags, CSAfF_Wind);
        }
        else
        {
            clear_flag(crconf->immunity_flags, CSAfF_Wind);
        }
        break;
    case 3: // IMMUNE_TO_GAS
        if (enable)
        {
            set_flag(crconf->immunity_flags, CSAfF_PoisonCloud);
        }
        else
        {
            clear_flag(crconf->immunity_flags, CSAfF_PoisonCloud);
        }
        break;
    case 4: // HUMANOID_SKELETON
        crconf->humanoid_creature = enable;
        break;
    case 5: // PISS_ON_DEAD
        crconf->piss_on_dead = enable;
        break;
    case 7: // FLYING
        crconf->flying = enable;
        break;
    case 8: // SEE_INVISIBLE
        crconf->can_see_invisible = enable;
        break;
    case 9: // PASS_LOCKED_DOORS
        crconf->can_go_locked_doors = enable;
        break;
    case 10: // SPECIAL_DIGGER
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_IsSpecDigger);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_IsSpecDigger);
        }
        recalculate_all_creature_digger_lists();
        update_creatr_model_activities_list(1);
        break;
    case 11: // ARACHNID
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_IsArachnid);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_IsArachnid);
        }
        break;
    case 12: // DIPTERA
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_IsDiptera);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_IsDiptera);
        }
        break;
    case 13: // LORD
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_IsLordOfLand);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_IsLordOfLand);
        }
        break;
    case 14: // SPECTATOR
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_IsSpectator);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_IsSpectator);
        }
        break;
    case 15: // EVIL
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_IsEvil);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_IsEvil);
        }
        break;
    case 16: // NEVER_CHICKENS
        if (enable)
        {
            set_flag(crconf->immunity_flags, CSAfF_Chicken);
        }
        else
        {
            clear_flag(crconf->immunity_flags, CSAfF_Chicken);
        }
        break;
    case 17: // IMMUNE_TO_BOULDER
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_ImmuneToBoulder);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_ImmuneToBoulder);
        }
        break;
    case 18: // NO_CORPSE_ROTTING
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_NoCorpseRotting);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_NoCorpseRotting);
        }
        break;
    case 19: // NO_ENMHEART_ATTCK
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_NoEnmHeartAttack);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_NoEnmHeartAttack);
        }
        break;
    case 20: // TREMBLING_FAT
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_Trembling);
            set_flag(crconf->model_flags, CMF_Fat);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_Trembling);
            clear_flag(crconf->model_flags, CMF_Fat);
        }
        break;
    case 21: // FEMALE
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_Female);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_Female);
        }
        break;
    case 22: // INSECT
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_Insect);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_Insect);
        }
        break;
    case 23: // ONE_OF_KIND
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_OneOfKind);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_OneOfKind);
        }
        break;
    case 24: // NO_IMPRISONMENT
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_NoImprisonment);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_NoImprisonment);
        }
        break;
    case 25: // NEVER_SICK
        if (enable)
        {
            set_flag(crconf->immunity_flags, CSAfF_Disease);
        }
        else
        {
            clear_flag(crconf->immunity_flags, CSAfF_Disease);
        }
        break;
    case 26: // ILLUMINATED
        crconf->illuminated = enable;
        break;
    case 27: // ALLURING_SCVNGR
        crconf->entrance_force = enable;
        break;
    case 28: // NO_RESURRECT
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_NoResurrect);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_NoResurrect);
        }
        break;
    case 29: // NO_TRANSFER
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_NoTransfer);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_NoTransfer);
        }
        break;
    case 30: // TREMBLING
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_Trembling);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_Trembling);
        }
        break;
    case 31: // FAT
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_Fat);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_Fat);
        }
        break;
    case 32: // NO_STEAL_HERO
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_NoStealHero);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_NoStealHero);
        }
        break;
    case 33: // PREFER_STEAL
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_PreferSteal);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_PreferSteal);
        }
        break;
    case 34: // EVENTFUL_DEATH
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_EventfulDeath);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_EventfulDeath);
        }
        break;
    case 35: // DIGGING_CREATURE
        if (enable)
        {
            set_flag(crconf->model_flags, CMF_IsDiggingCreature);
        }
        else
        {
            clear_flag(crconf->model_flags, CMF_IsDiggingCreature);
        }
        recalculate_all_creature_digger_lists();
        update_creatr_model_activities_list(1);
        break;
    case 36: // NO_HEALTH_FLOWER
        if (enable) {
            set_flag(crconf->model_flags, CMF_NoHealthFlower);
        }
        else {
            clear_flag(crconf->model_flags, CMF_NoHealthFlower);
        }
        break;
    case 37: // CANNOT_PICK_UP
        if (enable) {
            set_flag(crconf->model_flags, CMF_CannotPickUp);
        }
        else {
            clear_flag(crconf->model_flags, CMF_CannotPickUp);
        }
        break;
    case 38: // DROP_ON_PATH
        if (enable) {
            set_flag(crconf->model_flags, CMF_DropOnPath);
        }
        else {
            clear_flag(crconf->model_flags, CMF_DropOnPath);
        }
        break;
    case 39: // CANNOT_POSSESS
        if (enable) {
            set_flag(crconf->model_flags, CMF_CannotPossess);
        }
        else {
            clear_flag(crconf->model_flags, CMF_CannotPossess);
        }
        break;
    default:
        SCRPTERRLOG("Unknown creature property '%d'", property);
        break;
    }
}

short get_chat_icon_sprite_idx_from_id(short id, char type)
{
    switch (type)
    {
        case MsgType_Player:
            if (player_is_roaming(id))
                return GPS_plyrsym_symbol_player_red_std_b;

            if (id == game.neutral_player_num)
                return ((get_gameturn() >> 1) & 3)
                    + GPS_plyrsym_symbol_player_red_std_b;

            return player_has_heart(id)
                ? GPS_plyrsym_symbol_player_red_std_b
                : GPS_plyrsym_symbol_player_red_dead;

        case MsgType_Creature:
            return get_creature_model_graphics(id, CGI_HandSymbol);

        case MsgType_CreatureSpell:
            return get_spell_config(id)->medsym_sprite_idx;

        case MsgType_Room:
            return get_room_kind_stats(id)->medsym_sprite_idx;

        case MsgType_KeeperSpell:
            return get_power_model_stats(id)->medsym_sprite_idx;

        case MsgType_Query:
            return id + GPS_plyrsym_symbol_room_yellow_std_a;

        case MsgType_CreatureInstance:
            return creature_instance_info_get(id)->symbol_spridx;

        case MsgType_Custom:
            return id;

        case MsgType_Blank:
        default:
            return -1;
    }
}

short get_chat_icon_sprite_idx(const char* txt)
{
    short id = 0;
    char type = 0;

    get_chat_icon_from_value(txt, &id, &type);

    return get_chat_icon_sprite_idx_from_id(id, type);
}


void get_chat_icon_from_value(const char* txt, short* id, char* type)
{
    short idx;
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
                                *id = get_icon_id(txt);
                                *type = MsgType_Custom;
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
