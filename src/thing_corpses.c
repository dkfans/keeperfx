/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_corpses.c
 *     Dead creature things support functions.
 * @par Purpose:
 *     Functions to create and operate on dead creature corpses.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 02 Mar 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_corpses.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_memory.h"
#include "thing_data.h"
#include "thing_stats.h"
#include "thing_list.h"
#include "thing_physics.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "config_terrain.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_graphics.h"
#include "player_instances.h"
#include "dungeon_data.h"
#include "config_creature.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/

/**
 *  Returns if given corpse can rot in graveyard.
 * @param thing The dead creature thing.
 * @return True if the corpse can rot in a graveyard.
 */
TbBool corpse_is_rottable(const struct Thing *thing)
{
    if (!thing_exists(thing))
        return false;
    if (thing->class_id != TCls_DeadCreature)
        return false;
    if ((get_creature_model_flags(thing) & CMF_NoCorpseRotting) != 0)
        return false;
    struct PlayerInfo* player = get_player_thing_is_controlled_by(thing);
    if (player_invalid(player))
        return true;
    return false;
}

/**
 *  Returns if given corpse placed properly into the graveyard.
 * @param thing The dead creature thing.
 * @return True if the corpse has been placed in graveyard.
 */
TbBool corpse_laid_to_rest(const struct Thing* thing)
{
    if (!thing_exists(thing))
        return false;
    if (thing->class_id != TCls_DeadCreature)
        return false;
    if ((get_creature_model_flags(thing) & CMF_NoCorpseRotting) != 0)
        return false;
    if (thing->corpse.laid_to_rest >= 1)
        return true;
    return false;
}

/**
*  Returns if given thing is corpse that can be picked up for graveyard.
* @param thing The dead creature thing.
* @return True if the corpse can be dragged into graveyard for rotting.
*/
TbBool corpse_ready_for_collection(const struct Thing* thing)
{
    if (!corpse_is_rottable(thing))
        return false;
    if (corpse_laid_to_rest(thing))
        return false;
    if (thing_is_dragged_or_pulled(thing))
        return false;
    if (thing->active_state != DCrSt_RigorMortis)
        return false;
    return true;
}

/**
 * Returns if given dead creature thing is a room inventory.
 * Inventory are the things which can be stored in a room, but are movable and optional.
 * @param thing
 * @return
 */
TbBool dead_creature_is_room_inventory(const struct Thing *thing, RoomKind rkind)
{
    switch (rkind)
    {
    case RoK_GRAVEYARD:
        return corpse_is_rottable(thing);
    default:
        return false;
    }
}

TbBool create_vampire_in_room(struct Room *room)
{
    struct Coord3d pos;
    pos.x.val = 0;
    pos.y.val = 0;
    pos.z.val = 0;
    long crmodel = get_room_create_creature_model(room->kind);
    struct Thing* thing = create_creature(&pos, crmodel, room->owner);
    if (thing_is_invalid(thing)) {
        ERRORLOG("Could not create creature model %ld",crmodel);
        return false;
    }
    if (!find_random_valid_position_for_thing_in_room(thing, room, &pos)) {
        ERRORLOG("Could not find valid position in room");
        delete_thing_structure(thing, 0);
        return false;
    }
    move_thing_in_map(thing, &pos);
    struct Dungeon* dungeon = get_dungeon(room->owner);
    dungeon->lvstats.vamps_created++;
    create_effect(&pos, TngEff_Explosion3, thing->owner);
    if (is_my_player_number(room->owner)) {
        output_message(SMsg_GraveyardMadeVampire, 0, true);
    }
    return true;
}

void remove_body_from_graveyard(struct Thing *thing)
{
    struct Room* room = get_room_thing_is_on(thing);
    if (room_is_invalid(room)) {
        ERRORLOG("The %s is not in room",thing_model_name(thing));
        return;
    }
    if (room->kind != RoK_GRAVEYARD) {
        ERRORLOG("The %s is in %s instead of graveyard",thing_model_name(thing),room_code_name(room->kind));
        return;
    }
    if (room->used_capacity <= 0) {
        ERRORLOG("Graveyard had no allocated capacity to remove body from");
        return;
    }
    if (!corpse_laid_to_rest(thing)) {
        ERRORLOG("The %s is not in graveyard",thing_model_name(thing));
        return;
    }
    room->used_capacity--;
    thing->byte_14 = 0; //Laid to rest
    struct Dungeon* dungeon = get_dungeon(room->owner);
    dungeon->bodies_rotten_for_vampire++;
    dungeon->lvstats.graveyard_bodys++;
    if (dungeon->bodies_rotten_for_vampire >= game.bodies_for_vampire) {
        dungeon->bodies_rotten_for_vampire -= game.bodies_for_vampire;
        create_vampire_in_room(room);
    }
}

long move_dead_creature(struct Thing *thing)
{
    if ( (thing->velocity.x.val != 0) || (thing->velocity.y.val != 0) || (thing->velocity.z.val != 0) )
    {
        long i = (long)thing->mappos.x.val + (long)thing->velocity.x.val;
        if (i >= subtile_coord(map_subtiles_x,0)) i = subtile_coord(map_subtiles_x,0)-1;
        if (i < 0) i = 0;
        struct Coord3d pos;
        pos.x.val = i;
        i = (long)thing->mappos.y.val + (long)thing->velocity.y.val;
        if (i >= subtile_coord(map_subtiles_y,0)) i = subtile_coord(map_subtiles_y,0)-1;
        if (i < 0) i = 0;
        pos.y.val = i;
        i = (long)thing->mappos.z.val + (long)thing->velocity.z.val;
        if (i < 0) i = 0;
        pos.z.val = i;
        if ( !positions_equivalent(&thing->mappos, &pos) )
        {
          if ( thing_in_wall_at(thing, &pos) )
          {
              i = get_creature_blocked_flags_at(thing, &pos);
              slide_thing_against_wall_at(thing, &pos, i);
              remove_relevant_forces_from_thing_after_slide(thing, &pos, i);
          }
        }
        move_thing_in_map(thing, &pos);
    } else
    {
        // Even if no velocity, update field_60
        thing->field_60 = get_thing_height_at(thing, &thing->mappos);
    }
    return 1;
}

TngUpdateRet update_dead_creature(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);
    if ((thing->alloc_flags & TAlF_IsDragged) == 0)
    {
        if (thing->active_state == DCrSt_DramaticDying)
        {
            struct Coord3d pos;
            pos.x.val = thing->mappos.x.val;
            pos.y.val = thing->mappos.y.val;
            pos.z.val = thing->mappos.z.val;
            pos.z.val += 3 * (int)thing->clipbox_size_yz / 4;
            if (creature_model_bleeds(thing->model)) {
                create_effect(&pos, TngEff_BloodyFootstep, thing->owner);
            }
            if (thing->health > 0)
                thing->health--;
            if (thing->health <= 0) {
                thing->active_state = DCrSt_RigorMortis;
                long i = get_creature_anim(thing, 16);
                set_thing_draw(thing, i, 64, -1, 1, 0, 2);
            }
        } else
        if (corpse_laid_to_rest(thing))
        {
            if ( corpse_is_rottable(thing) )
            {
                if (thing->health > 0)
                    thing->health--;
                if (thing->health <= 0) {
                    remove_body_from_graveyard(thing);
                    EVM_CREATURE_EVENT("remove.decomposed", thing->owner, thing);
                    delete_thing_structure(thing, 0);
                    return TUFRet_Deleted;
                }
            } else
            {
                if (game.play_gameturn - thing->creation_turn > game.body_remains_for) {
                    EVM_CREATURE_EVENT("remove", thing->owner, thing);
                    delete_thing_structure(thing, 0);
                    return TUFRet_Deleted;
                }
            }
        } else
        {
            if (game.play_gameturn - thing->creation_turn > game.body_remains_for) {
                EVM_CREATURE_EVENT("remove", thing->owner, thing);
                delete_thing_structure(thing, 0);
                return TUFRet_Deleted;
            }
        }
    }
    if (subtile_has_water_on_top(thing->mappos.x.stl.num, thing->mappos.y.stl.num)) {
        thing->movement_flags |= TMvF_IsOnWater;
    }
    if ((thing->alloc_flags & TAlF_IsControlled) != 0)
    {
        move_dead_creature(thing);
        return TUFRet_Modified;
    }
    if ( map_pos_is_lava(thing->mappos.x.stl.num, thing->mappos.y.stl.num)
      && !thing_is_dragged_or_pulled(thing) )
    {
        EVM_CREATURE_EVENT("remove.melted", thing->owner, thing);
        delete_thing_structure(thing, 0);
        return TUFRet_Deleted;
    }
    if (subtile_is_door(thing->mappos.x.stl.num, thing->mappos.y.stl.num))
    {
        EVM_CREATURE_EVENT("remove.squished", thing->owner, thing);
        delete_thing_structure(thing, 0);
        create_dead_creature(&thing->mappos, thing->model, 2, thing->owner, thing->corpse.exp_level);
        return TUFRet_Deleted;
    }
    move_dead_creature(thing);
    return TUFRet_Modified;
}

long find_item_in_dead_creature_list(struct Dungeon *dungeon, ThingModel crmodel, long crlevel)
{
    if (dungeon_invalid(dungeon))
        return -1;
    long i = dungeon->dead_creatures_count - 1;
    while (i >= 0)
    {
        struct CreatureStorage* cstore = &dungeon->dead_creatures[i];
        if ((cstore->model == crmodel) && (cstore->explevel == crlevel))
        {
          return i;
        }
        i--;
    }
    return -1;
}

TbBool add_item_to_dead_creature_list(struct Dungeon *dungeon, ThingModel crmodel, long crlevel)
{
    SYNCDBG(18,"Starting");
    if (dungeon_invalid(dungeon))
    {
        WARNLOG("Invalid dungeon");
        return false;
    }
    // Check if the creature of same type is in list
    long i = find_item_in_dead_creature_list(dungeon, crmodel, crlevel);
    struct CreatureStorage* cstore;
    if (i >= 0)
    {
        cstore = &dungeon->dead_creatures[i];
        cstore->count++;
        SYNCDBG(18,"Already in list");
        return true;
    }
    // Find a slot for the new creature
    if (dungeon->dead_creatures_count < DEAD_CREATURES_MAX_COUNT)
    {
        i = dungeon->dead_creatures_count;
        dungeon->dead_creatures_count++;
    } else
    {
        i = dungeon->dead_creature_idx;
        dungeon->dead_creature_idx++;
        if (dungeon->dead_creature_idx >= DEAD_CREATURES_MAX_COUNT)
          dungeon->dead_creature_idx = 0;
    }
    cstore = &dungeon->dead_creatures[i];
    cstore->model = crmodel;
    cstore->explevel = crlevel;
    cstore->count = 0;
    SYNCDBG(19,"Finished");
    return true;
}

TbBool remove_item_from_dead_creature_list(struct Dungeon *dungeon, ThingModel crmodel, long crlevel)
{
    SYNCDBG(18,"Starting");
    if (dungeon_invalid(dungeon))
    {
        WARNLOG("Invalid dungeon");
        return false;
    }
    struct CreatureStorage* cstore;
    long rmpos = find_item_in_dead_creature_list(dungeon, crmodel, crlevel);
    if (rmpos < 0)
    {
        return false;
    }
    cstore = &dungeon->dead_creatures[rmpos];
    if (cstore->count >= 1)
    {
        cstore->count--;
    }
    else
    {
        for (long i = rmpos; i < DEAD_CREATURES_MAX_COUNT - 1; i++)
        {
            LbMemoryCopy(&dungeon->dead_creatures[i], &dungeon->dead_creatures[i + 1], sizeof(struct CreatureStorage));
        }
        cstore = &dungeon->dead_creatures[DEAD_CREATURES_MAX_COUNT - 1];
        cstore->model = 0;
        cstore->explevel = 0;
        if (dungeon->dead_creature_idx > 0)
        {
            dungeon->dead_creature_idx--;
        }
        if (dungeon->dead_creatures_count > 0)
        {
            dungeon->dead_creatures_count--;
        }
    }
    SYNCDBG(19,"Finished");
    return true;
}

TbBool update_dead_creatures_list(struct Dungeon *dungeon, const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        WARNLOG("Invalid victim creature control");
        return false;
    }
    return add_item_to_dead_creature_list(dungeon, thing->model, cctrl->explevel);
}

TbBool update_dead_creatures_list_for_owner(const struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    struct Dungeon* dungeon = INVALID_DUNGEON;
    if (!is_neutral_thing(thing)) {
        dungeon = get_players_num_dungeon(thing->owner);
    }
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    return update_dead_creatures_list(dungeon, thing);
}

struct Thing *create_dead_creature(const struct Coord3d *pos, ThingModel model, unsigned short crpscondition, unsigned short owner, long explevel)
{
    if (!i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots))
    {
        ERRORDBG(3,"Cannot create dead creature model %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    struct Thing* thing = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate dead creature %d for player %d, but failed.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->class_id = TCls_DeadCreature;
    thing->model = model;
    thing->parent_idx = thing->index;
    thing->owner = owner;
    thing->corpse.exp_level = explevel;
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = 0;
    thing->mappos.z.val = get_thing_height_at(thing, &thing->mappos);
    thing->clipbox_size_xy = 0;
    thing->clipbox_size_yz = 0;
    thing->solid_size_xy = 0;
    thing->solid_size_yz = 0;
    thing->fall_acceleration = 16;
    thing->field_23 = 204;
    thing->field_24 = 51;
    thing->bounce_angle = 0;
    thing->movement_flags |= TMvF_Unknown08;
    thing->creation_turn = game.play_gameturn;
    if (creatures[model].field_7) {
        thing->field_4F |= (TF4F_Transpar_Alpha);
    }
    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);
    unsigned long k;
    switch (crpscondition)
    {
    case DCrSt_RigorMortis:
        thing->active_state = DCrSt_RigorMortis;
        k = get_creature_anim(thing, 17);
        set_thing_draw(thing, k, 256, gameadd.crtr_conf.sprite_size, 0, 0, 2);
        break;
    default:
        thing->active_state = DCrSt_DramaticDying;
        k = get_creature_anim(thing, 15);
        set_thing_draw(thing, k, 128, gameadd.crtr_conf.sprite_size, 0, 0, 2);
        thing->health = 3 * get_lifespan_of_animation(thing->anim_sprite, thing->anim_speed);
        play_creature_sound(thing, CrSnd_Die, 3, 0);
        break;
    }
    thing->sprite_size = (gameadd.crtr_conf.sprite_size * (long)thing->corpse.exp_level) / 20 + gameadd.crtr_conf.sprite_size;
    return thing;
}

/**
 * Kills a creature and creates a proper corpse on its place.
 * @param thing The creature to be killed.
 * @param a1
 * @return The corpse thing, on invalid thing on error.
 */
struct Thing *destroy_creature_and_create_corpse(struct Thing *thing, long crpscondition)
{
    long crmodel = thing->model;
    TbBool memf1 = ((thing->alloc_flags & TAlF_IsControlled) != 0);
    struct Coord3d pos;
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val;
    long owner = thing->owner;
    long prev_idx = thing->index;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    long explevel = cctrl->explevel;
    struct PlayerInfo* player = NULL;
    remove_creature_score_from_owner(thing);
    delete_thing_structure(thing, 0);
    struct Thing* deadtng = create_dead_creature(&pos, crmodel, crpscondition, owner, explevel);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Could not create dead thing while killing %s index %d owned by player %d.",creature_code_name(crmodel),(int)prev_idx,(int)owner);
        return INVALID_THING;
    }
    set_flag_byte(&deadtng->alloc_flags, TAlF_IsControlled, memf1);
    if (owner != game.neutral_player_num)
    {
        // Update thing index inside player struct
        player = get_player(owner);
        if (player->controlled_thing_idx == prev_idx)
        {
            // we can't use set_selected_creature() as we're setting the target to dead body
            set_selected_thing(player, deadtng);
        }
    }
    return deadtng;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
