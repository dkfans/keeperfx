/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_traps.c
 *     Traps support functions.
 * @par Purpose:
 *     Functions to support trap things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_traps.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_memory.h"

#include "thing_data.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "creature_states.h"
#include "thing_effects.h"
#include "thing_physics.h"
#include "thing_shots.h"
#include "magic.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "room_util.h"
#include "game_legacy.h"
#include "frontend.h"
#include "engine_render.h"
#include "gui_topmsg.h"

#include "keeperfx.hpp"
#include "creature_senses.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

//field_0; sprite_anim_idx; sprite_size_max; unanimated; anim_speed; field_11; field_12; field_13; size_xy; field_16; trigger_type; activation_type; created_itm_model;  hit_type; etc
struct TrapStats old_trap_stats[7] = {
{0,           0,   0, 0,   0,        0, 0, 0,          0,      0,          0, 0,  0, 0, 0, 0, 0, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, 0, 0, 0},
{128,       861, 384, 1,   0,        0, 0, 1,        640,    512,          1, 1, 15, 9, 0, 0, 0, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, 0, 0, 0}, //Boulder
{1,         844, 256, 0, 256,        0, 0, 1,          0,      0,          2, 3, 19, 2, 0, 0, 0, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, 0, 0, 0}, //Alarm
{1,         845, 256, 0, 256,        0, 0, 1,          0,      0,          2, 2, 13, 4, 0, 0, 0, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, 0, 0, 0}, //Gas
{1,         846, 256, 0, 256,        0, 0, 1,          0,      0,          2, 3, 29, 4, 0, 0, 0, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, 0, 0, 0}, //Lightning
{1,         844, 256, 0, 256,        0, 0, 1,          0,      0,          2, 2, 14, 4, 0, 0, 0, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, 0, 0, 0}, //WoP
{1,         845, 256, 0, 256,        0, 0, 1,          0,      0,          2, 4, 12, 4, 0, 0, 0, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, 0, 0, 0}, //Lava
};

TbBool destroy_trap(struct Thing *traptng)
{
    if ((traptng->trap.num_shots == 0) && !is_neutral_thing(traptng) && !is_hero_thing(traptng)) {
        readd_workshop_item_to_amount_placeable(traptng->owner, traptng->class_id, traptng->model);
    }
    delete_thing_structure(traptng, 0);
    return true;
}

TbBool trap_is_active(const struct Thing *thing)
{
    return ((thing->trap.num_shots > 0) && (thing->trap.rearm_turn <= game.play_gameturn));
}

TbBool trap_is_slappable(const struct Thing *thing, PlayerNumber plyr_idx)
{
    struct TrapConfigStats *trapst;
    if (thing->owner == plyr_idx)
    {
        trapst = &gameadd.trapdoor_conf.trap_cfgstats[thing->model];
        return (trapst->slappable == 1) && trap_is_active(thing);
    }
    return false;
}

struct Thing *get_trap_for_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if (thing->class_id == TCls_Trap) {
            return thing;
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return INVALID_THING;
}

struct Thing *get_trap_for_slab_position(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    MapCoord pos_x = subtile_coord_center(slab_subtile_center(slb_x));
    MapCoord pos_y = subtile_coord_center(slab_subtile_center(slb_y));
    return get_trap_around_of_model_and_owned_by(pos_x, pos_y, -1, -1);
}

TbBool slab_has_trap_on(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct Thing* traptng = get_trap_for_slab_position(slb_x, slb_y);
    return !thing_is_invalid(traptng);
}

TbBool subtile_has_trap_on(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Thing* traptng = get_trap_for_position(stl_x, stl_y);
    return !thing_is_invalid(traptng);
}

TbBool slab_middle_row_has_trap_on(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    int i;
    for (i = 0; i <= 2; i++)
    {
        if (subtile_has_trap_on(slab_subtile(slb_x,i), slab_subtile_center(slb_y)))
        {
            return true;
        }
    }
    return false;
}

TbBool slab_middle_column_has_trap_on(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    int i;
    for (i = 0; i <= 2; i++)
    {
        if (subtile_has_trap_on(slab_subtile_center(slb_x), slab_subtile(slb_y,i)))
        {
            return true;
        }
    }
    return false;
}

TbBool thing_is_deployed_trap(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if (thing->class_id != TCls_Trap)
        return false;
    return true;
}

TbBool update_trap_trigger_line_of_sight_90_on_subtile(struct Thing *traptng, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if (thing_is_creature(thing) && (thing->owner != traptng->owner))
        {
            // Trigger for enemy player, or any player for neutral traps (otherwise neutral traps would be useless)
            if (players_are_enemies(traptng->owner,thing->owner) || is_neutral_thing(traptng))
            {
                if (!creature_is_being_unconscious(thing) && !thing_is_dragged_or_pulled(thing)
                 && !creature_is_kept_in_custody_by_enemy(thing) && !creature_is_dying(thing)
                 && ((get_creature_model_flags(thing) & CMF_IsSpectator) == 0)) {
                    activate_trap(traptng, thing);
                    return true;
                }
            }
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return false;
}

TbBool update_trap_trigger_line_of_sight_90(struct Thing *traptng)
{
    static const MapSubtlDelta line_of_sight_90_range = 20;
    MapSubtlCoord stl_x_beg;
    MapSubtlCoord stl_x_end;
    MapSubtlCoord stl_y_beg;
    MapSubtlCoord stl_y_end;
    {
        MapCoordDelta trap_radius = traptng->clipbox_size_xy / 2;
        MapCoord coord_x = traptng->mappos.x.val;
        stl_x_beg = coord_subtile(coord_x - trap_radius);
        if (stl_x_beg <= 0)
            stl_x_beg = 0;
        stl_x_end = coord_subtile(coord_x + trap_radius);
        if (stl_x_end >= map_subtiles_x)
            stl_x_end = map_subtiles_x;
        MapCoord coord_y = traptng->mappos.y.val;
        stl_y_beg = coord_subtile(coord_y - trap_radius);
        if (stl_y_beg <= 0)
            stl_y_beg = 0;
        stl_y_end = coord_subtile(coord_y + trap_radius);
        if (stl_y_end >= map_subtiles_y)
            stl_y_end = map_subtiles_y;
    }
    MapSubtlCoord stl_x_pre;
    MapSubtlCoord stl_x_aft;
    MapSubtlCoord stl_y_pre;
    MapSubtlCoord stl_y_aft;
    {
        stl_y_pre = stl_y_beg - line_of_sight_90_range;
        if (stl_y_pre <= 0)
            stl_y_pre = 0;
        stl_y_aft = stl_y_end + line_of_sight_90_range;
        if (stl_y_aft >= map_subtiles_y+1)
            stl_y_aft = map_subtiles_y+1;
        stl_x_pre = stl_x_beg - line_of_sight_90_range;
        if (stl_x_pre <= 0)
            stl_x_pre = 0;
        stl_x_aft = stl_x_end + line_of_sight_90_range;
        if (stl_x_aft >= map_subtiles_x+1)
            stl_x_aft = map_subtiles_x+1;
    }
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    // Find a limit of where the trap will fit in negative Y
    for (stl_x=stl_x_beg; stl_x <= stl_x_end; stl_x++)
    {
        for (stl_y = stl_y_beg; stl_y >= stl_y_pre; stl_y--)
        {
            struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
                stl_y_pre = stl_y + 1;
                break;
            }
        }
    }
    // Check the area for activation
    for (stl_x=stl_x_beg; stl_x <= stl_x_end; stl_x++)
    {
        for (stl_y = stl_y_beg; stl_y >= stl_y_pre; stl_y--)
        {
            if (update_trap_trigger_line_of_sight_90_on_subtile(traptng, stl_x, stl_y)) {
                return true;
            }
        }
    }
    // Find a limit of where the trap will fit in positive Y
    for (stl_x=stl_x_beg; stl_x <= stl_x_end; stl_x++)
    {
        for (stl_y = stl_y_end; stl_y <= stl_y_aft; stl_y++)
        {
            struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
                stl_y_aft = stl_y - 1;
                break;
            }
        }
    }
    // Check the area for activation
    for (stl_x=stl_x_beg; stl_x <= stl_x_end; stl_x++)
    {
        for (stl_y = stl_y_end; stl_y <= stl_y_aft; stl_y++)
        {
            if (update_trap_trigger_line_of_sight_90_on_subtile(traptng, stl_x, stl_y)) {
                return true;
            }
        }
    }
    // Find a limit of where the trap will fit in positive X
    for (stl_y=stl_y_beg; stl_y <= stl_y_end; stl_y++)
    {
        for (stl_x=stl_x_end; stl_x <= stl_x_aft; stl_x++)
        {
            struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
                stl_x_aft = stl_x - 1;
                break;
            }
        }
    }
    // Check the area for activation
    for (stl_y=stl_y_beg; stl_y <= stl_y_end; stl_y++)
    {
        for (stl_x=stl_x_end; stl_x <= stl_x_aft; stl_x++)
        {
            if (update_trap_trigger_line_of_sight_90_on_subtile(traptng, stl_x, stl_y)) {
                return true;
            }
        }
    }
    // Find a limit of where the trap will fit in negative X
    for (stl_y=stl_y_beg; stl_y <= stl_y_end; stl_y++)
    {
        for (stl_x=stl_x_beg; stl_x >= stl_x_pre; stl_x--)
        {
            struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
                stl_x_pre = stl_x + 1;
                break;
            }
        }
    }
    // Check the area for activation
    for (stl_y=stl_y_beg; stl_y <= stl_y_end; stl_y++)
    {
        for (stl_x=stl_x_beg; stl_x >= stl_x_pre; stl_x--)
        {
            if (update_trap_trigger_line_of_sight_90_on_subtile(traptng, stl_x, stl_y)) {
                return true;
            }
        }
    }
    return false;
}

void activate_trap_shot_head_for_target90(struct Thing *traptng, struct Thing *creatng)
{
    struct TrapStats* trapstat = &gameadd.trap_stats[traptng->model];
    if (trapstat->created_itm_model <= 0)
    {
        ERRORLOG("Trap activation of bad shot kind %d",(int)trapstat->created_itm_model);
        return;
    }
    struct Thing* shotng = create_shot(&traptng->mappos, trapstat->created_itm_model, traptng->owner);
    if (!thing_is_invalid(shotng))
    {
        {
            MapCoord trpos_x = traptng->mappos.x.val;
            MapCoord trpos_y = traptng->mappos.y.val;
            MapCoord crpos_x = creatng->mappos.x.val;
            MapCoord crpos_y = creatng->mappos.y.val;
            if (abs(trpos_x - crpos_x) <= abs(trpos_y - crpos_y))
            {
                if (crpos_y >= trpos_y)
                    shotng->move_angle_xy = LbFPMath_PI;
                else
                    shotng->move_angle_xy = 0;
            } else
            {
                if (crpos_x >= trpos_x)
                    shotng->move_angle_xy = LbFPMath_PI/2;
                else
                    shotng->move_angle_xy = 3*LbFPMath_PI/2;
            }
        }
        shotng->move_angle_z = 0;
        struct ShotConfigStats* shotst = get_shot_model_stats(trapstat->created_itm_model);
        struct ComponentVector cvect;
        angles_to_vector(shotng->move_angle_xy, 0, shotst->speed, &cvect);
        shotng->veloc_push_add.x.val += cvect.x;
        shotng->veloc_push_add.y.val += cvect.y;
        shotng->veloc_push_add.z.val += cvect.z;
        shotng->state_flags |= TF1_PushAdd;
        shotng->hit_type = trapstat->hit_type;
        if (shotst->firing_sound > 0) {
            thing_play_sample(traptng, shotst->firing_sound+UNSYNC_RANDOM(shotst->firing_sound_variants),
                NORMAL_PITCH, 0, 3, 0, 6, FULL_LOUDNESS);
        }
        if (shotst->shot_sound > 0) {
            thing_play_sample(shotng, shotst->shot_sound, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        }
    }
}

void activate_trap_effect_on_trap(struct Thing *traptng, struct Thing *creatng)
{
    struct TrapStats* trapstat = &gameadd.trap_stats[traptng->model];
    if (trapstat->created_itm_model <= 0)
    {
        ERRORLOG("Trap activation of bad effect kind %d",(int)trapstat->created_itm_model);
        return;
    }
    struct Thing* efftng = create_effect(&traptng->mappos, trapstat->created_itm_model, traptng->owner);
    if (!thing_is_invalid(efftng)) 
    {
        efftng->hit_type = trapstat->hit_type;
        SYNCDBG(18,"Created %s",thing_model_name(efftng));
    }
    if(trapstat->created_itm_model == 14) //Word of Power trap
    { 
        struct ShotConfigStats* shotst;
        shotst = get_shot_model_stats(31); //SHOT_TRAP_WORD_OF_POWER
        if (shotst->firing_sound > 0) 
        {
            thing_play_sample(traptng, shotst->firing_sound+UNSYNC_RANDOM(shotst->firing_sound_variants),
                NORMAL_PITCH, 0, 3, 0, 6, FULL_LOUDNESS);
        }
        if (shotst->shot_sound > 0) 
        {
            thing_play_sample(efftng, shotst->shot_sound, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        }
    }
}

void activate_trap_shot_on_trap(struct Thing *traptng, struct Thing *creatng)
{
    struct TrapStats* trapstat = &gameadd.trap_stats[traptng->model];
    if (trapstat->created_itm_model <= 0)
    {
        ERRORLOG("Trap activation of bad shot kind %d",(int)trapstat->created_itm_model);
        return;
    }
    struct Thing* shotng = create_shot(&traptng->mappos, trapstat->created_itm_model, traptng->owner);
    if (!thing_is_invalid(shotng)) {
        shotng->hit_type = trapstat->hit_type;
        shotng->parent_idx = 0;
        shotng->veloc_push_add.x.val += trapstat->field_30;
        shotng->veloc_push_add.y.val += trapstat->field_32;
        shotng->veloc_push_add.z.val += trapstat->field_34;
        shotng->state_flags |= TF1_PushAdd;
    }
}

void activate_trap_slab_change(struct Thing *traptng, struct Thing *creatng)
{
    MapSubtlCoord stl_x = traptng->mappos.x.stl.num;
    MapSubtlCoord stl_y = traptng->mappos.y.stl.num;
    if (subtile_is_room(stl_x, stl_y)) {
        delete_room_slab(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y), true);
    }
    place_slab_type_on_map(gameadd.trap_stats[traptng->model].created_itm_model, stl_x, stl_y, game.neutral_player_num, 0);
    // TODO
    //remove_traps_around_subtile(slab_subtile_center(slb_x), slab_subtile_center(slb_y), NULL);
    //update_navigation_triangulation(stl_x-1,  stl_y-1, stl_x+1,stl_y+1);
    do_slab_efficiency_alteration(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y));
}

void activate_trap_spawn_creature(struct Thing *traptng, const struct TrapStats *trapstat)
{
    struct Thing *thing = create_creature(&traptng->mappos, trapstat->created_itm_model, traptng->owner);
    struct CreatureControl* cctrl;

    if (thing_is_invalid(thing))
    {
        return;
    }
    thing->mappos.z.val = get_thing_height_at(thing, &thing->mappos);
// Try to move thing out of the solid wall if it's inside one
    if (thing_in_wall_at(thing, &thing->mappos))
    {
        ERRORLOG("Trap has to create creature, but creation failed");
        delete_thing_structure(thing, 0);
        return;
    }
    cctrl = creature_control_get_from_thing(thing);
    thing->veloc_push_add.x.val += CREATURE_RANDOM(thing, 161) - 80;
    thing->veloc_push_add.y.val += CREATURE_RANDOM(thing, 161) - 80;
    thing->veloc_push_add.z.val += 0;
    thing->state_flags |= TF1_PushAdd;
    cctrl->spell_flags |= CSAfF_MagicFall;
    thing->move_angle_xy = 0;
    // EVM_CREATURE_EVENT("joined.trap", thing->owner, thing);
}


void activate_trap_god_spell(struct Thing *traptng, struct Thing *creatng, PowerKind pwkind)
{
    struct PowerConfigStats *powerst = get_power_model_stats(pwkind);

    if (powerst->can_cast_flags & PwCast_AllThings)
    {
        magic_use_power_on_thing(traptng->owner, pwkind, SPELL_MAX_LEVEL, creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, creatng, PwMod_CastForFree);
    }
    else if (powerst->can_cast_flags & PwCast_AllGround)
    {
        magic_use_power_on_subtile(traptng->owner, pwkind, SPELL_MAX_LEVEL, creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, PwMod_CastForFree);
    }
    else if (powerst->can_cast_flags & PwCast_Anywhere)
    {
        magic_use_power_on_level(traptng->owner, pwkind, SPELL_MAX_LEVEL, PwMod_CastForFree);
    }
    else
    {
        ERRORLOG("Illegal trap Power %d/%s (idx=%d)", pwkind, get_string(powerst->name_stridx), traptng->index);
    }
}

void activate_trap(struct Thing *traptng, struct Thing *creatng)
{
    traptng->trap.revealed = 1;
    const struct TrapStats *trapstat = &gameadd.trap_stats[traptng->model];
    struct TrapConfigStats *trapst = &gameadd.trapdoor_conf.trap_cfgstats[traptng->model];
    // EVM_TRAP_EVENT("trap.actiated", traptng->owner, thing)
    if (trapst->notify == 1)
    {
        event_create_event(traptng->mappos.x.val, traptng->mappos.y.val, EvKind_AlarmTriggered, traptng->owner, 0);
    }
    thing_play_sample(traptng, 176, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    switch (trapstat->activation_type)
    {
    case TrpAcT_HeadforTarget90:
        activate_trap_shot_head_for_target90(traptng, creatng);
        break;
    case TrpAcT_EffectonTrap:
        activate_trap_effect_on_trap(traptng, creatng);
        break;
    case TrpAcT_ShotonTrap:
        activate_trap_shot_on_trap(traptng, creatng);
        break;
    case TrpAcT_SlabChange:
        activate_trap_slab_change(traptng, creatng);
        break;
    case TrpAcT_CreatureShot:
        creature_fire_shot(traptng, creatng, trapstat->created_itm_model, THit_CrtrsNObjcts, 1);
        break;
    case TrpAcT_CreatureSpawn:
        activate_trap_spawn_creature(traptng, trapstat);
        break;
    case TrpAcT_Power:
        activate_trap_god_spell(traptng, creatng, trapstat->created_itm_model);
        break;
    default:
        ERRORLOG("Illegal trap activation type %d (idx=%d)",(int)trapstat->activation_type, traptng->index);
        break;
    }
}

TbBool find_pressure_trigger_trap_target_passing_by_subtile(const struct Thing *traptng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing **found_thing)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if (thing_is_creature(thing) && (thing->owner != traptng->owner))
        {
            if (!creature_is_being_unconscious(thing) && !thing_is_dragged_or_pulled(thing)
             && !creature_is_kept_in_custody_by_enemy(thing) && !creature_is_dying(thing)
             && ((get_creature_model_flags(thing) & CMF_IsSpectator) == 0))
            {
                if (!is_neutral_thing(thing) && !players_are_mutual_allies(traptng->owner,thing->owner))
                {
                    *found_thing = thing;
                    return true;
                }
            }
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return false;
}

TbBool update_trap_trigger_pressure(struct Thing *traptng)
{
    MapSlabCoord slb_x = subtile_slab_fast(traptng->mappos.x.stl.num);
    MapSlabCoord slb_y = subtile_slab_fast(traptng->mappos.y.stl.num);
    MapSubtlCoord end_stl_x = slab_subtile(slb_x, 2);
    MapSubtlCoord end_stl_y = slab_subtile(slb_y, 2);
    for (MapSubtlCoord stl_y = slab_subtile(slb_y, 0); stl_y <= end_stl_y; stl_y++)
    {
        for (MapSubtlCoord stl_x = slab_subtile(slb_x, 0); stl_x <= end_stl_x; stl_x++)
        {
            struct Thing* creatng = INVALID_THING;
            if (find_pressure_trigger_trap_target_passing_by_subtile(traptng, stl_x, stl_y, &creatng))
            {
                activate_trap(traptng, creatng);
                return true;
            }

        }
    }
    return false;
}

TbBool update_trap_trigger_line_of_sight(struct Thing* traptng)
{
    struct Thing* trgtng = get_nearest_enemy_creature_possible_to_attack_by(traptng);

    if (line_of_sight_2d(&traptng->mappos, &trgtng->mappos))
    {
        activate_trap(traptng, trgtng);
        return true;
    }
    return false;
}

TngUpdateRet update_trap_trigger(struct Thing *traptng)
{
    if (traptng->trap.num_shots <= 0) {
        return TUFRet_Unchanged;
    }
    TbBool do_trig;
    switch (gameadd.trap_stats[traptng->model].trigger_type)
    {
    case TrpTrg_LineOfSight90:
        do_trig = update_trap_trigger_line_of_sight_90(traptng);
        break;
    case TrpTrg_Pressure:
        do_trig = update_trap_trigger_pressure(traptng);
        break;
    case TrpTrg_LineOfSight:
        do_trig = update_trap_trigger_line_of_sight(traptng);
        break;
    case TrpTrg_None: // for manually activated traps
        do_trig = false;
        break;
    default:
        ERRORLOG("Illegal trap trigger type %d",(int)gameadd.trap_stats[traptng->model].trigger_type);
        do_trig = false;
        break;
    }
    if (do_trig)
    {
        const struct ManfctrConfig* mconf = &gameadd.traps_config[traptng->model];
        traptng->trap.rearm_turn = game.play_gameturn + mconf->shots_delay;
        int n = traptng->trap.num_shots;
        if ((n > 0) && (n != 255))
        {
            traptng->trap.num_shots = n - 1;
            if (traptng->trap.num_shots == 0)
            {
                // If the trap is in strange location, destroy it after it's depleted
                struct SlabMap* slb = get_slabmap_thing_is_on(traptng);
                if ((slb->kind != SlbT_CLAIMED) && (slb->kind != SlbT_PATH)) {
                    traptng->health = -1;
                }
                traptng->field_4F &= TF4F_Transpar_Flags;
                traptng->field_4F |= TF4F_Transpar_4;
                if (!is_neutral_thing(traptng) && !is_hero_thing(traptng)) 
                {
                    if (placing_offmap_workshop_item(traptng->owner, TCls_Trap, traptng->model))
                    {
                        //When there's only offmap traps, destroy the disarmed one so the player can place a new one.
                        delete_thing_structure(traptng, 0);
                    }
                    else
                    {
                        //Trap is available to be rearmed, so earmark a workshop crate for it.
                        remove_workshop_item_from_amount_placeable(traptng->owner, traptng->class_id, traptng->model);
                    }
                }
            }
        }
        return TUFRet_Modified;
    }
    return TUFRet_Unchanged;
}

TbBool rearm_trap(struct Thing *traptng)
{
    struct ManfctrConfig* mconf = &gameadd.traps_config[traptng->model];
    struct TrapStats* trapstat = &gameadd.trap_stats[traptng->model];
    traptng->trap.num_shots = mconf->shots;
    traptng->field_4F ^= (traptng->field_4F ^ (trapstat->field_12 << 4)) & (TF4F_Transpar_Flags);
    return true;
}



TngUpdateRet update_trap(struct Thing *traptng)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(traptng);
    if (traptng->health < 0)
    {
        destroy_trap(traptng);
        return TUFRet_Deleted;
    }
    if (trap_is_active(traptng))
    {
        update_trap_trigger(traptng);
    }
    if (map_pos_is_lava(traptng->mappos.x.stl.num, traptng->mappos.y.stl.num) && !thing_is_dragged_or_pulled(traptng))
    {
        traptng->health = -1;
    }
    return TUFRet_Modified;
}


struct Thing *create_trap(struct Coord3d *pos, ThingModel trpkind, PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Starting for %s owner %d",trap_code_name(trpkind),(int)plyr_idx);
    struct TrapStats* trapstat = &gameadd.trap_stats[trpkind];
    if (!i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots)) {
        ERRORDBG(3,"Cannot create trap %s for player %d. There are too many things allocated.",trap_code_name(trpkind),(int)plyr_idx);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    struct InitLight ilght;
    LbMemorySet(&ilght, 0, sizeof(struct InitLight));
    struct Thing* thing = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate trap %s for player %d, but failed.",trap_code_name(trpkind),(int)plyr_idx);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->class_id = TCls_Trap;
    thing->model = trpkind;
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = pos->z.val;
    thing->next_on_mapblk = 0;
    thing->parent_idx = thing->index;
    thing->owner = plyr_idx;
    char start_frame;
    if (trapstat->field_13) {
        start_frame = -1;
    } else {
        start_frame = 0;
    }
    set_thing_draw(thing, trapstat->sprite_anim_idx, trapstat->anim_speed, trapstat->sprite_size_max, trapstat->unanimated, start_frame, 2);
    if (trapstat->field_11) {
        thing->field_4F |= TF4F_Unknown02;
    } else {
        thing->field_4F &= ~TF4F_Unknown02;
    }
    if (trapstat->unanimated) {
        thing->field_4F |= TF4F_Unknown40;
    } else {
        thing->field_4F &= ~TF4F_Unknown40;
    }
    thing->clipbox_size_xy = trapstat->size_xy;
    thing->clipbox_size_yz = trapstat->field_16;
    thing->solid_size_xy = trapstat->size_xy;
    thing->solid_size_yz = trapstat->field_16;
    thing->creation_turn = game.play_gameturn;
    thing->health = trapstat->field_0;
    thing->field_4F &= ~TF4F_Transpar_Flags;
    thing->field_4F |= TF4F_Transpar_4;
    thing->byte_13 = 0;
    thing->long_14 = game.play_gameturn;
    if (trapstat->light_1C != 0)
    {
        ilght.mappos.x.val = thing->mappos.x.val;
        ilght.mappos.y.val = thing->mappos.y.val;
        ilght.mappos.z.val = thing->mappos.z.val;
        ilght.radius = trapstat->light_1C;
        ilght.intensity = trapstat->light_1E;
        ilght.is_dynamic = 1;
        ilght.field_3 = trapstat->light_1F;
        thing->light_id = light_create_light(&ilght);
        if (thing->light_id <= 0) {
            SYNCDBG(8,"Cannot allocate dynamic light to %s.",thing_model_name(thing));
        }
    }
    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);
    return thing;
}

void init_traps(void)
{
    int k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Trap);
    int i = slist->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per thing code
        if (thing->trap.num_shots == 0)
        {
            rearm_trap(thing);
        }
        // Per thing code ends
        k++;
        if (k > slist->index)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

/**
 * Removes traps on the subtile and all sibling subtiles.
 * Either treats the action as selling and removes corresponding crate from workshop,
 * or treats it as destroying and updates workshop counts to skip re-arming the destroyed trap.
 *
 * @param stl_x Central subtile X coordinate.
 * @param stl_y Central subtile Y coordinate.
 * @param sell_value Value to be added to treasury if selling traps; if not selling but just removing, should be null.
 * @return Amount of traps removed.
 */

long remove_trap(struct Thing *traptng, long *sell_value)
{
    long total = 0;
    if (!thing_is_invalid(traptng))
    {
        if (sell_value != NULL)
        {
            // Do the refund only if we were able to sell armed trap
            long i = compute_value_percentage(gameadd.traps_config[traptng->model].selling_value, gameadd.trap_sale_percent);
            if (traptng->trap.num_shots == 0)
            {
                // Trap not armed - try selling crate from workshop
                if (remove_workshop_item_from_amount_stored(traptng->owner, traptng->class_id, traptng->model, WrkCrtF_NoOffmap) > WrkCrtS_None) {
                    remove_workshop_object_from_player(traptng->owner, trap_crate_object_model(traptng->model));
                    (*sell_value) += i;
                }
            } else
            {
                // Trap armed - we can get refund
                (*sell_value) += i;
            }
            // We don't want to increase trap_amount_placeable, so we'll not use destroy_trap() there
            delete_thing_structure(traptng, 0);
        } else {
            destroy_trap(traptng);
        }
        total++;
    } 
    return total;        
}
 
long remove_trap_on_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long *sell_value)
{
    struct Thing* traptng = get_trap_for_position(stl_x, stl_y);
    return remove_trap(traptng, sell_value);
}
 
long remove_traps_around_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long *sell_value)
{
    long total;
    for (long k = 0; k < AROUND_TILES_COUNT; k++)
    {
        struct Thing* traptng = get_trap_for_position(stl_x + around[k].delta_x, stl_y + around[k].delta_y);
        total = remove_trap(traptng, sell_value);
    }
    return total;
}

void external_activate_trap_shot_at_angle(struct Thing *thing, long a2, struct Thing *hand)
{
    struct TrapStats* trapstat = &gameadd.trap_stats[thing->model];
    if (trapstat->created_itm_model <= 0) {
        ERRORLOG("Cannot activate trap with shot model %d",(int)trapstat->created_itm_model);
        return;
    }
    if ((trapstat->activation_type != TrpAcT_CreatureShot)
        && (trapstat->activation_type != TrpAcT_HeadforTarget90))
    {
        activate_trap(thing, hand);
        if (thing->byte_13 != 255)
        {
            if (thing->byte_13 > 0) {
                thing->byte_13--;
            }
            if (thing->byte_13 <= 0) {
                thing->health = -1;
            }
        }
        return;
    }
    struct Thing* shotng = create_shot(&thing->mappos, trapstat->created_itm_model, thing->owner);
    if (thing_is_invalid(shotng)) {
        return;
    }
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    shotng->move_angle_xy = a2;
    shotng->move_angle_z = 0;
    struct ComponentVector cvect;
    angles_to_vector(shotng->move_angle_xy, 0, shotst->speed, &cvect);
    shotng->veloc_push_add.x.val += cvect.x;
    shotng->veloc_push_add.y.val += cvect.y;
    shotng->veloc_push_add.z.val += cvect.z;
    shotng->state_flags |= TF1_PushAdd;
    shotng->hit_type = trapstat->hit_type;
    const struct ManfctrConfig* mconf = &gameadd.traps_config[thing->model];
    thing->long_14 = game.play_gameturn + mconf->shots_delay;
    if (thing->byte_13 != 255)
    {
        if (thing->byte_13 > 0) {
            thing->byte_13--;
        }
        if (thing->byte_13 <= 0) {
            thing->health = -1;
        }
    }
}

TbBool can_place_trap_on(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    MapSlabCoord slb_x = subtile_slab_fast(stl_x);
    MapSlabCoord slb_y = subtile_slab_fast(stl_y);
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    struct SlabAttr* slbattr = get_slab_attrs(slb);
    struct PlayerInfo* player = get_player(plyr_idx);
    TbBool HasTrap = true;
    TbBool HasDoor = true;
    if (!subtile_revealed(stl_x, stl_y, plyr_idx)) {
        return false;
    }
    if (((slbattr->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)) {
        return false;
    }
    if (slab_kind_is_liquid(slb->kind)) {
        return false;
    }
    if ((slabmap_owner(slb) == plyr_idx) && ((slb->kind == SlbT_CLAIMED) || (slab_is_door(slb_x, slb_y))))
    {
        if ((!gameadd.place_traps_on_subtiles))
        {
                HasTrap = slab_has_trap_on(slb_x, slb_y);
                HasDoor = slab_is_door(slb_x, slb_y);
        }
        else if ( (gameadd.place_traps_on_subtiles) && (player->chosen_trap_kind == TngTrp_Boulder) ) 
        {
                HasTrap = subtile_has_trap_on(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
                HasDoor = slab_is_door(slb_x, slb_y);
        }
        else
        {
            HasTrap = subtile_has_trap_on(stl_x, stl_y);
            switch(get_door_orientation(slb_x, slb_y))
            {
                case -1:
                {
                    HasDoor = false;
                    break;
                }
                case 0:
                {
                    HasDoor = slab_row_has_door_thing_on(slb_x, stl_y);
                    break;
                }
                case 1:
                {
                    HasDoor = slab_column_has_door_thing_on(stl_x, slb_y);
                    break;
                }
            }
            // HasDoor = ((subtile_has_door_thing_on(stl_x, stl_y)) || (subtile_is_door(stl_x, stl_y)) );
        }
        if (!HasTrap && !HasDoor)
        {
            return true;
        }
    }
    return false;
}

TbBool tag_cursor_blocks_place_trap(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long full_slab)
{
    SYNCDBG(7,"Starting");
    MapSlabCoord slb_x = subtile_slab_fast(stl_x);
    MapSlabCoord slb_y = subtile_slab_fast(stl_y);
    TbBool can_place = can_place_trap_on(plyr_idx, stl_x, stl_y);
    int floor_height = floor_height_for_volume_box(plyr_idx, slb_x, slb_y);
    if (is_my_player_number(plyr_idx))
    {
        if (!game_is_busy_doing_gui() && (game.small_map_state != 2))
        {
            render_roomspace.is_roomspace_a_box = true;
            render_roomspace.render_roomspace_as_box = true;
            if (full_slab)
            {
                // Move to first subtile on a slab
                stl_x = slab_subtile(slb_x,0);
                stl_y = slab_subtile(slb_y,0);
                render_roomspace.is_roomspace_a_single_subtile = false;
                draw_map_volume_box(subtile_coord(stl_x,0), subtile_coord(stl_y,0),
                subtile_coord(stl_x+STL_PER_SLB,0), subtile_coord(stl_y+STL_PER_SLB,0), floor_height, can_place);
            }
            else
            {
                render_roomspace.is_roomspace_a_single_subtile = true;
                draw_map_volume_box(subtile_coord(stl_x,0), subtile_coord(stl_y,0), subtile_coord(stl_x+1,0), subtile_coord(stl_y+1,0), floor_height, can_place);
            }
        }
    }
    return can_place;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
