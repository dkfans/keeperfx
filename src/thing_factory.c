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

/******************************************************************************/
struct Thing *create_cave_in(struct Coord3d *pos, unsigned short cimodel, unsigned short owner)
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
    thing->word_15 = pwrdynst->time;
    thing->byte_13 = pos->x.stl.num;
    thing->byte_14 = pos->y.stl.num;
    thing->byte_17 = cimodel;
    thing->health = pwrdynst->time;
    if (owner != game.neutral_player_num)
    {
        struct Dungeon* dungeon = get_dungeon(owner);
        dungeon->camera_deviate_quake = thing->word_15;
    }
    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);
    return thing;
}

struct Thing *create_thing(struct Coord3d *pos, unsigned short tngclass, unsigned short tngmodel, unsigned short owner, long parent_idx)
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
    default:
        break;
    }
    return thing;
}

short thing_create_thing(struct InitThing *itng)
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
                thing->byte_13 = itng->params[1];
            }
            else if (thing->model == OBJECT_TYPE_SPECBOX_CUSTOM)
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
            ERRORLOG("Couldn't create object model %d", (int)itng->model);
            return false;
        }
        break;
    case TCls_Creature:
        thing = create_creature(&itng->mappos, itng->model, itng->owner);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Couldn't create creature model %d", (int)itng->model);
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
            ERRORLOG("Couldn't create trap model %d", (int)itng->model);
            return false;
        }
        break;
    case TCls_Door:
        thing = create_door(&itng->mappos, itng->model, itng->params[0], itng->owner, itng->params[1]);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Couldn't create door model %d", (int)itng->model);
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

struct Thing *create_thing_at_position_then_move_to_valid_and_add_light(struct Coord3d *pos, unsigned char tngclass, unsigned char tngmodel, unsigned char tngowner)
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
            ilght.field_3 = 5;
        } else
        {
            ilght.intensity = 36;
            ilght.field_3 = 1;
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
