/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_factory.c
 *     Things creation unified functions.
 * @par Purpose:
 *     Functions to create various things using single interface.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 22 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "thing_factory.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_memory.h"

#include "thing_data.h"
#include "thing_doors.h"
#include "thing_list.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "thing_creature.h"
#include "thing_objects.h"
#include "thing_shots.h"
#include "thing_traps.h"
#include "thing_corpses.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "room_util.h"
#include "sounds.h"
#include "dungeon_data.h"
#include "gui_topmsg.h"
#include "config_magic.h"
#include "game_legacy.h"

#include "keeperfx.hpp"
#include "value_util.h"

#include "post_inc.h"

/******************************************************************************/
struct Thing *create_cave_in(struct Coord3d *pos, ThingModel cimodel, unsigned short owner)
{
    if ( !i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots) )
    {
        ERRORDBG(3,"Cannot create cave in %d for player %d. There are too many things allocated.",(int)cimodel,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    struct Thing* thing = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate cave in %d for player %d, but failed.",(int)cimodel,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->class_id = TCls_CaveIn;
    thing->model = 0;
    thing->parent_idx = thing->index;
    memcpy(&thing->mappos,pos,sizeof(struct Coord3d));
    thing->owner = owner;
    thing->creation_turn = game.play_gameturn;
    struct MagicStats* pwrdynst = get_power_dynamic_stats(PwrK_CAVEIN);
    thing->cave_in.time = pwrdynst->duration;
    thing->cave_in.x = pos->x.stl.num;
    thing->cave_in.y = pos->y.stl.num;
    thing->cave_in.model = cimodel;
    thing->health = pwrdynst->duration;
    if (owner != game.neutral_player_num)
    {
        struct Dungeon* dungeon = get_dungeon(owner);
        dungeon->camera_deviate_quake = thing->cave_in.time;
    }
    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);
    return thing;
}

struct Thing *create_thing(struct Coord3d *pos, unsigned short tngclass, ThingModel tngmodel, unsigned short owner, long parent_idx)
{
    struct Thing* thing = INVALID_THING;
    switch (tngclass)
    {
    case TCls_Object:
        thing = create_object(pos, tngmodel, owner, parent_idx);
        break;
    case TCls_Shot:
        thing = create_shot(pos, tngmodel, owner);
        break;
    case TCls_EffectElem:
        thing = create_effect_element(pos, tngmodel, owner);
        break;
    case TCls_DeadCreature:
        thing = create_dead_creature(pos, tngmodel, 1, owner, 0);
        break;
    case TCls_Creature:
        thing = create_creature(pos, tngmodel, owner);
        break;
    case TCls_Effect:
        thing = create_effect(pos, tngmodel, owner);
        break;
    case TCls_Trap:
        thing = create_trap(pos, tngmodel, owner);
        break;
    case TCls_AmbientSnd:
        thing = create_ambient_sound(pos, tngmodel, owner);
        break;
    case TCls_CaveIn:
        // for cave in, model is really a level
        thing = create_cave_in(pos, tngmodel, owner);
        break;
    case TCls_Door:
        thing = create_door(pos, tngmodel, find_door_angle(pos->x.stl.num, pos->y.stl.num, owner), owner, false);
        break;
    case TCls_EffectGen:
        thing = create_effect_generator(pos, tngmodel, 1, owner, parent_idx);
        break;
    default:
        break;
    }
    return thing;
}

TbBool thing_create_thing(struct InitThing *itng)
{
    if (itng->owner == 7)
    {
        ERRORLOG("Invalid owning player %d, fixing to %d", (int)itng->owner, (int)game.hero_player_num);
        itng->owner = game.hero_player_num;
    } else
    if (itng->owner == 8)
    {
        ERRORLOG("Invalid owning player %d, fixing to %d", (int)itng->owner, (int)game.neutral_player_num);
        itng->owner = game.neutral_player_num;
    }
    if (itng->owner > 5)
    {
        ERRORLOG("Invalid owning player %d, thing discarded", (int)itng->owner);
        return false;
    }
    struct Thing* thing;
    switch (itng->oclass)
    {
    case TCls_Object:
        thing = create_thing(&itng->mappos, itng->oclass, itng->model, itng->owner, itng->index);
        if (!thing_is_invalid(thing))
        {
            if (object_is_hero_gate(thing))
            {
                thing->hero_gate.number = itng->params[1];
            }
            else if (thing_is_custom_special_box(thing))
            {
                thing->custom_box.box_kind = itng->params[1];
                if (itng->params[1] > gameadd.max_custom_box_kind)
                {
                    gameadd.max_custom_box_kind = itng->params[1];
                }
            }
            check_and_asimilate_thing_by_room(thing);
            // make sure we don't have invalid pointer
            thing = INVALID_THING;
        } else
        {
            ERRORLOG("Couldn't create object model %d (%s)", (int)itng->model, object_code_name(itng->model));
            return false;
        }
        break;
    case TCls_Creature:
        thing = create_creature(&itng->mappos, itng->model, itng->owner);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Couldn't create creature model %d (%s)", (int)itng->model, creature_code_name(itng->model));
            return false;
        }
        init_creature_level(thing, itng->params[1]);
        break;
    case TCls_EffectGen:
        thing = create_effect_generator(&itng->mappos, itng->model, itng->range, itng->owner, itng->index);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Couldn't create effect generator model %d", (int)itng->model);
            return false;
        }
        break;
    case TCls_Trap:
        thing = create_thing(&itng->mappos, itng->oclass, itng->model, itng->owner, itng->index);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Couldn't create trap model %d (%s)", (int)itng->model, trap_code_name(itng->model));
            return false;
        }
        break;
    case TCls_Door:
        thing = create_door(&itng->mappos, itng->model, itng->params[0], itng->owner, itng->params[1]);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Couldn't create door model %d (%s)", (int)itng->model, door_code_name(itng->model));
            return false;
        }
        break;
    case TCls_Unkn10:
    case TCls_Unkn11:
        thing = create_thing(&itng->mappos, itng->oclass, itng->model, itng->owner, itng->index);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Couldn't create thing class %d model %d", (int)itng->oclass, (int)itng->model);
            return false;
        }
        break;
    default:
        ERRORLOG("Invalid class %d, thing discarded", (int)itng->oclass);
        return false;
    }
    return true;
}

TbBool thing_create_thing_adv(VALUE *init_data)
{
    int owner = value_int32(value_dict_get(init_data, "Ownership"));
    int oclass = value_parse_class(value_dict_get(init_data, "ThingType"));
    ThingModel model = value_parse_model(oclass, value_dict_get(init_data, "Subtype"));
    struct Coord3d mappos;
    mappos.x.val = value_read_stl_coord(value_dict_get(init_data, "SubtileX"));
    mappos.y.val = value_read_stl_coord(value_dict_get(init_data, "SubtileY"));
    mappos.z.val = value_read_stl_coord(value_dict_get(init_data, "SubtileZ"));
    if (oclass == -1)
    {
        ERRORLOG("Thing ThingType is not set");
        return false;
    }
    if (model == -1)
    {
        ERRORLOG("Thing Subtype is not set");
        return false;
    }
    if (owner == -1)
    {
        ERRORLOG("Thing Ownership is not set");
        return false;
    }

    if (owner > PLAYERS_COUNT)
    {
        ERRORLOG("Invalid owning player %d, thing discarded", owner);
        return false;
    }
    struct Thing* thing;
    switch (oclass)
    {
        case TCls_Object:
            thing = create_thing(&mappos, oclass, model, owner, (unsigned short)value_int32(value_dict_get(init_data, "ParentTile")));
            if (!thing_is_invalid(thing))
            {
                if (object_is_hero_gate(thing))
                {
                    VALUE* gate = value_dict_get(init_data, "HerogateNumber");
                    if (gate != NULL)
                    {
                        thing->hero_gate.number = value_int32(gate);
                    }
                }
                else if (thing_is_custom_special_box(thing))
                {
                    int box_kind = value_int32(value_dict_get(init_data, "CustomBox"));
                    if (box_kind == -1)
                        box_kind = 0;
                    thing->custom_box.box_kind = box_kind;
                    if (box_kind > gameadd.max_custom_box_kind)
                    {
                        gameadd.max_custom_box_kind = box_kind;
                    }
                }
                else if (object_is_gold_pile(thing))
                {
                    VALUE* value = value_dict_get(init_data, "GoldValue");
                    if (value != NULL)
                    {
                        thing->valuable.gold_stored = 0;
                        add_gold_to_pile(thing, value_int32(value));
                    }
                }
                check_and_asimilate_thing_by_room(thing);
                VALUE* rotation = value_dict_get(init_data, "Orientation");
                if (rotation != NULL)
                {
                    thing->move_angle_xy = value_int32(rotation);
                }
                // make sure we don't have invalid pointer
                thing = INVALID_THING;
            } else
            {
                ERRORLOG("Couldn't create object model %d (%s)", (int)model, object_code_name(model));
                return false;
            }
            break;
        case TCls_Creature:
            thing = create_creature(&mappos, model, owner);
            struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
            if (thing_is_invalid(thing))
            {
                ERRORLOG("Couldn't create creature model %d (%s)", (int)model, creature_code_name(model));
                return false;
            }
            {
                int level = value_int32(value_dict_get(init_data, "CreatureLevel"));
                if (level < 1 || level > 10)
                {
                    level = 0; // Default
                    WARNLOG("invalid level in tngfx file %d", level);
                }
                else
                {
                    level --; //levels are in readable format in file, gamecode always has them 1 lower
                }
                init_creature_level(thing, level);
                VALUE* creature_rotation = value_dict_get(init_data, "Orientation");
                if (creature_rotation != NULL)
                {
                    thing->move_angle_xy = value_int32(creature_rotation);
                }
                VALUE* gold_held = value_dict_get(init_data, "CreatureGold");
                if (gold_held != NULL)
                {
                    thing->creature.gold_carried = value_int32(gold_held);
                }
                VALUE *HealthPercentage = value_dict_get(init_data, "CreatureInitialHealth");
                if (HealthPercentage != NULL)
                {
                    thing->health = value_int32(HealthPercentage) * cctrl->max_health / 100;
                }
                const char* creatureName = value_string(value_dict_get(init_data, "CreatureName"));
                if(creatureName != NULL)
                {
                    if(strlen(creatureName) >= CREATURE_NAME_MAX)
                    {
                        ERRORLOG("init creature name (%s) too long max %d chars", creatureName, CREATURE_NAME_MAX-1);
                        break;
                    }
                    strcpy(cctrl->creature_name,creatureName);
                }

            }
            break;
        case TCls_EffectGen:
            {
                unsigned short range = value_read_stl_coord(value_dict_get(init_data, "EffectRange"));
                thing = create_effect_generator(&mappos, model, range, owner, (unsigned short)value_int32(value_dict_get(init_data, "ParentTile")));
            }
            if (thing_is_invalid(thing))
            {
                ERRORLOG("Couldn't create effect generator model %d", (int)model);
                return false;
            }
            break;
        case TCls_Trap:
            thing = create_thing(&mappos, oclass, model, owner, (unsigned short)value_int32(value_dict_get(init_data, "ParentTile")));
            if (thing_is_invalid(thing))
            {
                ERRORLOG("Couldn't create trap model %d (%s)", (int)model, trap_code_name(model));
                return false;
            }
            VALUE* trap_rotation = value_dict_get(init_data, "Orientation");
            if (trap_rotation != NULL)
            {
                thing->move_angle_xy = value_int32(trap_rotation);
            }
            break;
        case TCls_Door:
            {
                int orientation = value_int32(value_dict_get(init_data, "DoorOrientation"));
                int is_locked = value_int32(value_dict_get(init_data, "DoorLocked"));
                if (orientation == -1)
                    orientation = 0;
                if (is_locked == -1)
                    is_locked = 0;
                thing = create_door(&mappos, model, orientation, owner, is_locked);
            }
            if (thing_is_invalid(thing))
            {
                ERRORLOG("Couldn't create door model %d (%s)", (int)model, door_code_name(model));
                return false;
            }
            break;
        case TCls_Unkn10:
        case TCls_Unkn11:
            thing = create_thing(&mappos, oclass, model, owner, (unsigned short)value_int32(value_dict_get(init_data, "ParentTile")));
            if (thing_is_invalid(thing))
            {
                ERRORLOG("Couldn't create thing class %d model %d", (int)oclass, (int)model);
                return false;
            }
            break;
        default:
            ERRORLOG("Invalid class %d, thing discarded", (int)oclass);
            return false;
    }
    return true;
}

struct Thing *create_thing_at_position_then_move_to_valid_and_add_light(struct Coord3d *pos, unsigned char tngclass, ThingModel tngmodel, unsigned char tngowner)
{
    struct Thing* thing = create_thing(pos, tngclass, tngmodel, tngowner, -1);
    if (thing_is_invalid(thing))
    {
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

    if (thing_is_creature(thing))
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        cctrl->flee_pos.x.val = thing->mappos.x.val;
        cctrl->flee_pos.y.val = thing->mappos.y.val;
        cctrl->flee_pos.z.val = thing->mappos.z.val;
        cctrl->flee_pos.z.val = get_thing_height_at(thing, &thing->mappos);
        cctrl->party.target_plyr_idx = -1;
    }

    long light_rand = GAME_RANDOM(8); // this may be unsynced random
    if (light_rand < 2)
    {
        struct InitLight ilght;
        LbMemorySet(&ilght, 0, sizeof(struct InitLight));
        ilght.mappos.x.val = thing->mappos.x.val;
        ilght.mappos.y.val = thing->mappos.y.val;
        ilght.mappos.z.val = thing->mappos.z.val;
        if (light_rand == 1)
        {
            ilght.intensity = 48;
            ilght.flags = 5;
        } else
        {
            ilght.intensity = 36;
            ilght.flags = 1;
        }
        ilght.is_dynamic = 1;
        ilght.radius = 2560;
        thing->light_id = light_create_light(&ilght);
        if (thing->light_id != 0) {
            light_set_light_never_cache(thing->light_id);
        } else {
            ERRORLOG("Cannot allocate light to new hero");
        }
    }
    return thing;
}
/******************************************************************************/
