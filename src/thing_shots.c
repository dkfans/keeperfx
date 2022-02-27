/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_shots.c
 *     Shots support functions.
 * @par Purpose:
 *     Functions to support shot things.
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
#include "thing_shots.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_sound.h"
#include "creature_states.h"
#include "thing_data.h"
#include "thing_factory.h"
#include "thing_effects.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "thing_objects.h"
#include "front_simple.h"
#include "thing_stats.h"
#include "map_blocks.h"
#include "room_garden.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "power_process.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "creature_states.h"
#include "creature_groups.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
TbBool thing_is_shot(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if (thing->class_id != TCls_Shot)
        return false;
    return true;
}

TbBool shot_is_slappable(const struct Thing *thing, PlayerNumber plyr_idx)
{
    if (thing->owner == plyr_idx)
    {
        // Normally, thing models 15 and 20 are slappable. But this is up to config file.
        struct ShotConfigStats* shotst = get_shot_model_stats(thing->model);
        return ((shotst->model_flags & ShMF_Slappable) != 0);
    }
    return false;
}

TbBool shot_model_is_navigable(long tngmodel)
{
    // Normally, only shot model 6 is navigable
    struct ShotConfigStats* shotst = get_shot_model_stats(tngmodel);
    return ((shotst->model_flags & ShMF_Navigable) != 0);
}

TbBool shot_is_boulder(const struct Thing *shotng)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    return ((shotst->model_flags & ShMF_Boulder) != 0);
}

TbBool detonate_shot(struct Thing *shotng)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    SYNCDBG(8,"Starting for %s index %d owner %d",thing_model_name(shotng),(int)shotng->index,(int)shotng->owner);
    struct Thing* castng = INVALID_THING;
    struct PlayerInfo* myplyr = get_my_player();
    // Identify the creator if the shot
    if (shotng->index != shotng->parent_idx) {
        castng = thing_get(shotng->parent_idx);
        TRACE_THING(castng);
    }
    // If the shot has area_range, then make area damage
    if (shotst->area_range != 0) {
        struct CreatureStats* crstat = creature_stats_get_from_thing(castng);
        //TODO SPELLS Spell level should be taken from within the shot, not from caster creature
        // Caster may have leveled up, or even may be already dead
        // But currently shot do not store its level, so we don't really have a choice
        struct CreatureControl* cctrl = creature_control_get_from_thing(castng);
        long dist = compute_creature_attack_range(shotst->area_range * COORD_PER_STL, crstat->luck, cctrl->explevel);
        long damage = compute_creature_attack_spell_damage(shotst->area_damage, crstat->luck, cctrl->explevel, shotng);
        HitTargetFlags hit_targets = hit_type_to_hit_targets(shotst->area_hit_type);
        explosion_affecting_area(castng, &shotng->mappos, dist, damage, shotst->area_blow, hit_targets, shotst->damage_type);
    }
    //TODO CONFIG shot model dependency, make config option instead
    switch (shotng->model)
    {
    case ShM_Lightning:
    case ShM_GodLightning:
    case ShM_GodLightBall:
        PaletteSetPlayerPalette(myplyr, engine_palette);
        break;
    case ShM_Grenade:
    case ShM_Lizard:
    case ShM_Firebomb:
        create_effect(&shotng->mappos, TngEff_Explosion7, shotng->owner);
        create_effect(&shotng->mappos,  TngEff_Blood4, shotng->owner);
        break;
    case ShM_Boulder:
        create_effect_around_thing(shotng, TngEff_DirtRubble);
        break;
    default:
        break;
    }
    delete_thing_structure(shotng, 0);
    return true;
}

struct Thing *get_shot_collided_with_same_type_on_subtile(struct Thing *shotng, struct Coord3d *nxpos, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    const struct Thing *parentng;
    if (shotng->parent_idx > 0) {
        parentng = thing_get(shotng->parent_idx);
    } else {
        parentng = INVALID_THING;
    }
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return INVALID_THING;
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code
        if ((thing->index != shotng->index) && collide_filter_thing_is_of_type(thing, parentng, shotng->class_id, shotng->model))
        {
            if (things_collide_while_first_moves_to(shotng, nxpos, thing)) {
                return thing;
            }
        }
        // Per thing code ends
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

struct Thing *get_shot_collided_with_same_type(struct Thing *shotng, struct Coord3d *nxpos)
{
    //return _DK_get_shot_collided_with_same_type(thing, nxpos);
    MapSubtlCoord stl_x_beg = coord_subtile(nxpos->x.val - 384);
    if (stl_x_beg < 0)
        stl_x_beg = 0;
    MapSubtlCoord stl_y_beg = coord_subtile(nxpos->y.val - 384);
    if (stl_y_beg < 0)
        stl_y_beg = 0;
    MapSubtlCoord stl_x_end = coord_subtile(nxpos->x.val + 384);
    if (stl_x_end >= map_subtiles_x)
      stl_x_end = map_subtiles_x;
    MapSubtlCoord stl_y_end = coord_subtile(nxpos->y.val + 384);
    if (stl_y_end >= map_subtiles_y)
      stl_y_end = map_subtiles_y;
    for (MapSubtlCoord stl_y = stl_y_beg; stl_y <= stl_y_end; stl_y++)
    {
        for (MapSubtlCoord stl_x = stl_x_beg; stl_x <= stl_x_end; stl_x++)
        {
            struct Thing* thing = get_shot_collided_with_same_type_on_subtile(shotng, nxpos, stl_x, stl_y);
            if (!thing_is_invalid(thing)) {
                return thing;
            }
        }
    }
    return INVALID_THING;
}

TbBool give_gold_to_creature_or_drop_on_map_when_digging(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long damage)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    long gold = calculate_gold_digged_out_of_slab_with_single_hit(damage, creatng->owner, cctrl->explevel, slb);
    creatng->creature.gold_carried += gold;
    if (!dungeon_invalid(dungeon)) {
        dungeon->lvstats.gold_mined += gold;
    }
    if (crstat->gold_hold <= creatng->creature.gold_carried)
    {
        struct Thing* gldtng = drop_gold_pile(creatng->creature.gold_carried, &creatng->mappos);
        creatng->creature.gold_carried = 0;
        struct Room* room;
        room = get_room_thing_is_on(creatng);
        if (!room_is_invalid(room))
        {
            if (room_role_matches(room->kind, RoRoF_GoldStorage))
            {
                if (room->owner == creatng->owner)
                {
                    gold_being_dropped_at_treasury(gldtng, room);
                }
            }
        }
    }
    return true;
}

void process_dig_shot_hit_wall(struct Thing *thing, long blocked_flags)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    unsigned long k;
    struct Thing* diggertng = INVALID_THING;
    if (thing->index != thing->parent_idx)
      diggertng = thing_get(thing->parent_idx);
    if (!thing_exists(diggertng))
    {
        ERRORLOG("Digging shot hit wall, but there's no digger creature index %d.",thing->parent_idx);
        return;
    }
    if (blocked_flags & SlbBloF_WalledX)
    {
        k = thing->move_angle_xy & 0xFC00;
        if (k != 0)
        {
          stl_x = slab_subtile_center(coord_slab(thing->mappos.x.val) - 1);
          stl_y = slab_subtile_center(coord_slab(thing->mappos.y.val));
        }
        else
        {
          stl_x = slab_subtile_center(coord_slab(thing->mappos.x.val) + 1);
          stl_y = slab_subtile_center(coord_slab(thing->mappos.y.val));
        }
    } else
    if (blocked_flags & SlbBloF_WalledY)
    {
        k = thing->move_angle_xy & 0xFE00;
        if ((k != 0) && (k != 0x0600))
        {
          stl_x = slab_subtile_center(coord_slab(thing->mappos.x.val));
          stl_y = slab_subtile_center(coord_slab(thing->mappos.y.val) + 1);
        }
        else
        {
          stl_x = slab_subtile_center(coord_slab(thing->mappos.x.val));
          stl_y = slab_subtile_center(coord_slab(thing->mappos.y.val) - 1);
        }
    } else
    {
        stl_x = coord_subtile(thing->mappos.x.val);
        stl_y = coord_subtile(thing->mappos.y.val);
    }

    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);

    // You can only dig your own tiles or non-fortified neutral ground (dirt/gold)
    // If you're not the tile owner, unless the classic bug mode is enabled.
    if (!(gameadd.classic_bugs_flags & ClscBug_BreakNeutralWalls))
    {
        if (slabmap_owner(slb) != diggertng->owner)
        {
            struct SlabAttr* slbattr = get_slab_attrs(slb);
            // and if it's fortified
            if (slbattr->category == SlbAtCtg_FortifiedWall)
            {
                // digging not allowed
                return;
            }
        }
    }
    else
    {
        if ((slabmap_owner(slb) != game.neutral_player_num) && (slabmap_owner(slb) != diggertng->owner))
        {
            return;
        }
    }

    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if ((mapblk->flags & SlbAtFlg_IsRoom) != 0)
    {
        if (diggertng->creature.gold_carried > 0)
        {
            struct Thing* gldtng = drop_gold_pile(diggertng->creature.gold_carried, &diggertng->mappos);
            diggertng->creature.gold_carried = 0;
            struct Room* room;
            room = get_room_xy(stl_x, stl_y);
            if (!room_is_invalid(room))
            {
                if (room_role_matches(room->kind, RoRoF_GoldStorage))
                {
                    if (room->owner == diggertng->owner)
                    {
                        gold_being_dropped_at_treasury(gldtng, room);
                        return;
                    }
                }
            }
        }
    }

    // Doors cannot be dug
    if ((mapblk->flags & SlbAtFlg_IsDoor) != 0)
    {
        return;
    }
    if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
    {
        return;
    }
    int damage = thing->damagepoints;
    if ((damage >= slb->health) && !slab_kind_is_indestructible(slb->kind))
    {
        if ((mapblk->flags & SlbAtFlg_Valuable) != 0)
        { // Valuables require counting gold
            give_gold_to_creature_or_drop_on_map_when_digging(diggertng, stl_x, stl_y, damage);
            mine_out_block(stl_x, stl_y, diggertng->owner);
            thing_play_sample(diggertng, 72+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        } else
        if ((mapblk->flags & SlbAtFlg_IsDoor) == 0)
        { // All non-gold and non-door slabs are just destroyed
            dig_out_block(stl_x, stl_y, diggertng->owner);
            thing_play_sample(diggertng, 72+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        }
        check_map_explored(diggertng, stl_x, stl_y);
    } else
    {
        if (!slab_kind_is_indestructible(slb->kind))
        {
            slb->health -= damage;
        }
        if ((mapblk->flags & SlbAtFlg_Valuable) != 0)
        {
            give_gold_to_creature_or_drop_on_map_when_digging(diggertng, stl_x, stl_y, damage);
        }
    }
}

struct Thing *create_shot_hit_effect(struct Coord3d *effpos, long effowner, long eff_kind, long snd_idx, long snd_range)
{
    struct Thing* efftng = INVALID_THING;
    if (eff_kind > 0) {
        efftng = create_effect(effpos, eff_kind, effowner);
        TRACE_THING(efftng);
    }
    if (snd_idx > 0)
    {
        if (!thing_is_invalid(efftng))
        {
            long i = snd_idx;
            if (snd_range > 1)
                i += UNSYNC_RANDOM(snd_range);
            thing_play_sample(efftng, i, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        }
    }
    return efftng;
}

/**
 * Processes hitting a wall by given shot.
 *
 * @param thing The thing to be moved into wall.
 * @param pos Next position of the thing.
 * @return Gives true if the shot hit a wall and was destroyed.
 *     If the shot wasn't detonated, then the function returns false.
 * @note This function may delete the thing given in parameter.
 */
TbBool shot_hit_wall_at(struct Thing *shotng, struct Coord3d *pos)
{
    struct Thing *doortng;
    long i;
    SYNCDBG(8,"Starting for %s index %d",thing_model_name(shotng),(int)shotng->index);

    struct Thing* efftng = INVALID_THING;
    TbBool destroy_shot = 0;
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    long blocked_flags = get_thing_blocked_flags_at(shotng, pos);
    if (shotst->model_flags & ShMF_Digging)
    {
        process_dig_shot_hit_wall(shotng, blocked_flags);
    }

    // If blocked by a higher wall
    if ((blocked_flags & SlbBloF_WalledZ) != 0)
    {
        long cube_id = get_top_cube_at(pos->x.stl.num, pos->y.stl.num, NULL);
        doortng = get_door_for_position(pos->x.stl.num, pos->y.stl.num);
        if (!thing_is_invalid(doortng))
        {
            efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->hit_door.effect_model, shotst->hit_door.sndsample_idx, shotst->hit_door.sndsample_range);
            if (!shotst->hit_door.withstand)
              destroy_shot = 1;
            i = calculate_shot_real_damage_to_door(doortng, shotng);
            apply_damage_to_thing(doortng, i, shotst->damage_type, -1);
        } else
        if (cube_is_water(cube_id))
        {
            efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->hit_water.effect_model, shotst->hit_water.sndsample_idx, shotst->hit_water.sndsample_range);
            if (!shotst->hit_water.withstand) {
                destroy_shot = 1;
            }
        } else
        if (cube_is_lava(cube_id))
        {
            efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->hit_lava.effect_model, shotst->hit_lava.sndsample_idx, shotst->hit_lava.sndsample_range);
            if (!shotst->hit_lava.withstand) {
                destroy_shot = 1;
            }
        } else
        {
            efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->hit_generic.effect_model, shotst->hit_generic.sndsample_idx, shotst->hit_generic.sndsample_range);
            if (!shotst->hit_generic.withstand) {
                destroy_shot = 1;
            }
        }
    }

    if ( !destroy_shot )
    {
        if ((blocked_flags & (SlbBloF_WalledX|SlbBloF_WalledY)) != 0)
        {
            if (shotng->model == ShM_Lizard)
            {
                if (shotng->shot.dexterity >= CREATURE_RANDOM(shotng, 90))
                {
                    struct Coord3d target_pos;
                    target_pos.x.val = shotng->price.number;
                    target_pos.y.val = shotng->shot.byte_19 * gameadd.crtr_conf.sprite_size;
                    target_pos.z.val = pos->z.val;
                    const MapCoordDelta dist = get_2d_distance(pos, &target_pos);
                    if (dist <= 800) return detonate_shot(shotng);
                }
            }
            doortng = get_door_for_position(pos->x.stl.num, pos->y.stl.num);
            if (!thing_is_invalid(doortng))
            {
                efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->hit_door.effect_model, shotst->hit_door.sndsample_idx, shotst->hit_door.sndsample_range);
                if (!shotst->hit_door.withstand)
                    destroy_shot = 1;
                i = calculate_shot_real_damage_to_door(doortng, shotng);
                apply_damage_to_thing(doortng, i, shotst->damage_type, -1);
            } else
            {
                efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->hit_generic.effect_model, shotst->hit_generic.sndsample_idx, shotst->hit_generic.sndsample_range);
                if (!shotst->hit_generic.withstand)
                {
                    destroy_shot = 1;
                }
            }
        }
    }
    if (!thing_is_invalid(efftng)) {
        efftng->hit_type = shotst->area_hit_type;
    }
    if ( destroy_shot )
    {
        return detonate_shot(shotng);
    }
    if (shotng->bounce_angle <= 0)
    {
        slide_thing_against_wall_at(shotng, pos, blocked_flags);
    }
    else
    {
        bounce_thing_off_wall_at(shotng, pos, blocked_flags);
    }
    return false;
}

/**
 * Processes hitting a door by given shot.
 *
 * @param thing The thing to be moved.
 * @param pos Next position of the thing.
 * @return Gives true if the shot hit a door and was destroyed.
 *     If the shot wasn't detonated, then the function returns false.
 * @note This function may delete the thing given in parameter.
 */
long shot_hit_door_at(struct Thing *shotng, struct Coord3d *pos)
{
    SYNCDBG(18,"Starting for %s index %d",thing_model_name(shotng),(int)shotng->index);
    TbBool shot_explodes = false;
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    struct Thing* efftng = INVALID_THING;
    long blocked_flags = get_thing_blocked_flags_at(shotng, pos);
    if (blocked_flags != 0)
    {
        struct Thing* doortng = get_door_for_position(pos->x.stl.num, pos->y.stl.num);
        // If we did found a door to hit
        if (!thing_is_invalid(doortng))
        {
            // If the shot hit is supposed to create effect thing
            int n = shotst->hit_door.effect_model;
            if (n > 0)
            {
                efftng = create_effect(&shotng->mappos, n, shotng->owner);
            }
            // If the shot hit is supposed to create sound
            n = shotst->hit_door.sndsample_idx;
            int i;
            if (n > 0)
            {
                if (!thing_is_invalid(efftng))
                {
                    i = shotst->hit_door.sndsample_range;
                    thing_play_sample(efftng, n + UNSYNC_RANDOM(i), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
                }
            }
            // Shall the shot be destroyed on impact
            if (!shotst->hit_door.withstand)
            {
                shot_explodes = true;
            }
            // Apply damage to the door
            i = calculate_shot_real_damage_to_door(doortng, shotng);
            apply_damage_to_thing(doortng, i, shotst->damage_type, -1);
      }
    }
    if (!thing_is_invalid(efftng)) {
        efftng->hit_type = shotst->area_hit_type;
    }
    if ( shot_explodes )
    {
        return detonate_shot(shotng);
    }
    if (shotng->bounce_angle <= 0)
    {
        slide_thing_against_wall_at(shotng, pos, blocked_flags);
    }
    else
    {
        bounce_thing_off_wall_at(shotng, pos, blocked_flags);
    }
    return false;
}

TbBool apply_shot_experience(struct Thing *shooter, long exp_factor, long exp_increase, long shot_model)
{
    if (!creature_can_gain_experience(shooter))
        return false;
    struct CreatureControl* shcctrl = creature_control_get_from_thing(shooter);
    struct ShotConfigStats* shotst = get_shot_model_stats(shot_model);
    long exp_mag = shotst->old->experience_given_to_shooter;
    long exp_gained = (exp_mag * (exp_factor + 12 * exp_factor * exp_increase / 100) << 8) / 256;
    shcctrl->prev_exp_points = shcctrl->exp_points;
    shcctrl->exp_points += exp_gained;
    if ( check_experience_upgrade(shooter) ) {
        external_set_thing_state(shooter, CrSt_CreatureBeHappy);
    }
    return true;
}

// originally was apply_shot_experience()
TbBool apply_shot_experience_from_hitting_creature(struct Thing *shooter, struct Thing *target, long shot_model)
{
    struct CreatureControl* tgcctrl = creature_control_get_from_thing(target);
    struct CreatureStats* tgcrstat = creature_stats_get_from_thing(target);
    return apply_shot_experience(shooter, tgcrstat->exp_for_hitting, tgcctrl->explevel, shot_model);
}

long shot_kill_object(struct Thing *shotng, struct Thing *target)
{
    if (thing_is_dungeon_heart(target))
    {
        target->active_state = 3;
        target->health = -1;
        if (is_my_player_number(shotng->owner))
        {
            struct PlayerInfo* player = get_player(target->owner);
            if (player_exists(player) && (player->is_active == 1) && (shotng->owner != target->owner))
            {
                output_message(SMsg_DefeatedKeeper, 0, true);
            }
        }
        struct Dungeon* dungeon = get_players_num_dungeon(shotng->owner);
        if (!dungeon_invalid(dungeon)) {
            dungeon->lvstats.keepers_destroyed++;
        }
        return 1;
    }
    else
    if (object_is_mature_food(target) || object_is_growing_food(target))
    {
        destroy_food(target);
    } else
    {
        WARNLOG("Killing %s by %s is not supported",thing_model_name(target),thing_model_name(shotng));
    }
    return 0;
}

long shot_hit_object_at(struct Thing *shotng, struct Thing *target, struct Coord3d *pos)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    if (!thing_is_object(target)) {
        return 0;
    }
    if (shotst->model_flags & ShMF_NoHit) {
        return 0;
    }
    if (target->health < 0) {
        return 0;
    }
    struct ObjectConfig* objconf = get_object_model_stats2(target->model);
    if (objconf->resistant_to_nonmagic && !(shotst->damage_type == DmgT_Magical)) {
        return 0;
    }
    struct Thing* shootertng = INVALID_THING;
    if (shotng->parent_idx != shotng->index) {
        shootertng = thing_get(shotng->parent_idx);
    }
    if (thing_is_dungeon_heart(target))
    {
        if (shotng->model == 21) //TODO CONFIG shot model dependency, make config option instead
        {
            thing_play_sample(target, 134+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
        } else
        if (shotng->model == 22) //TODO CONFIG shot model dependency, make config option instead
        {
            thing_play_sample(target, 144+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
        }
        event_create_event_or_update_nearby_existing_event(
            shootertng->mappos.x.val, shootertng->mappos.y.val,
          EvKind_HeartAttacked, target->owner, shootertng->index);
        if (is_my_player_number(target->owner)) {
            output_message(SMsg_HeartUnderAttack, 400, true);
        }
    } else
    {
        int i = shotst->hit_generic.sndsample_idx;
        if (i > 0) {
            thing_play_sample(target, i, NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
        }
    }
    HitPoints damage = 0;
    if (shotng->damagepoints)
    {
        if (object_can_be_damaged(target)) // do not damage objects that cannot be destroyed
        {
            damage = apply_damage_to_thing(target, shotng->damagepoints, shotst->damage_type, -1);
            // Drain allows caster to regain half of damage, even against objects
            if ((shotst->model_flags & ShMF_LifeDrain) && thing_is_creature(shootertng))
            {
                apply_health_to_thing(shootertng, shotng->damagepoints / 2);
            }
        }
        target->byte_13 = 20; //todo figure out what this is, and if it needs to be within this if statement above/below
    }
    create_relevant_effect_for_shot_hitting_thing(shotng, target);
    if (target->health < 0) {
        shot_kill_object(shotng, target);
    }
    if (shotst->old->destroy_on_first_hit) {
        delete_thing_structure(shotng, 0);
    }
    return damage;
}

long get_damage_of_melee_shot(struct Thing *shotng, const struct Thing *target)
{
    const struct CreatureStats* tgcrstat = creature_stats_get_from_thing(target);
    const struct CreatureControl* tgcctrl = creature_control_get_from_thing(target);
    long crdefense = compute_creature_max_defense(tgcrstat->defense, tgcctrl->explevel);
    long hitchance = ((long)shotng->shot.dexterity - crdefense) / 2;
    if (hitchance < -96)
    {
        hitchance = -96;
    } else
    if (hitchance > 96)
    {
        hitchance = 96;
    }
    if (CREATURE_RANDOM(shotng, 256) < (128 + hitchance))
    {
        return shotng->shot.damage;
    }
    return -1;
}

long project_damage_of_melee_shot(long shot_dexterity, long shot_damage, const struct Thing *target)
{
    const struct CreatureStats* tgcrstat = creature_stats_get_from_thing(target);
    const struct CreatureControl* tgcctrl = creature_control_get_from_thing(target);
    long crdefense = compute_creature_max_defense(tgcrstat->defense, tgcctrl->explevel);
    long hitchance = (shot_dexterity - crdefense) / 2;
    if (hitchance < -96) {
        hitchance = -96;
    } else
    if (hitchance > 96) {
        hitchance = 96;
    }
    // If we'd return something which rounds to 0, change it to 1. Otherwise, just use hit chance in computations.
    if (abs(shot_damage) > 256/(128+hitchance))
        return shot_damage * (128+hitchance)/256;
    else if (shot_damage > 0)
        return 1;
    else if (shot_damage < 0)
        return -1;
    return 0;
}

void create_relevant_effect_for_shot_hitting_thing(struct Thing *shotng, struct Thing *target)
{
    struct Thing* efftng = INVALID_THING;
    if (target->class_id == TCls_Creature)
    {
        switch (shotng->model)
        {
        case ShM_Fireball:
        case ShM_Firebomb:
        case ShM_Lightning:
            efftng = create_effect(&shotng->mappos, TngEff_Explosion1, shotng->owner);
            break;
        case ShM_PoisonCloud:
            efftng = create_effect(&shotng->mappos, TngEff_Gas3, shotng->owner);
            if ( !thing_is_invalid(efftng) ) {
                efftng->hit_type = THit_CrtrsOnly;
            }
            break;
        case ShM_NaviMissile:
        case ShM_Missile:
            efftng = create_effect(&shotng->mappos, TngEff_Blood3, shotng->owner);
            break;
        case ShM_Arrow:
        case ShM_SwingSword:
        case ShM_SwingFist:
            if (creature_affected_by_spell(target, SplK_Freeze)) {
                efftng = create_effect(&shotng->mappos, TngEff_HitFrozenUnit, shotng->owner);
            } else
            if (creature_model_bleeds(target->model)) {
                efftng = create_effect(&shotng->mappos, TngEff_HitBleedingUnit, shotng->owner);
            }
            break;
        }
    }
    TRACE_THING(efftng);
}

long check_hit_when_attacking_door(struct Thing *thing)
{
    if (!thing_is_creature(thing))
    {
        ERRORLOG("The %s in invalid for this check", thing_model_name(thing));
        return 0;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->combat_flags & CmbtF_DoorFight) != 0)
    {
        CrtrStateId crstate = get_creature_state_besides_move(thing);
        if (crstate != CrSt_CreatureCombatFlee)
        {
            set_start_state(thing);
            return 1;
        }
    }
    return 1;
}

/**
 * Kills a creature with given shot.
 * @param shotng The shot which is killing the victim creature.
 * @param creatng The victim creature thing.
 * @return True if the creature is being killed, false if something have failed.
 * @note sometimes named shot_kills_creature().
 */
TbBool shot_kill_creature(struct Thing *shotng, struct Thing *creatng)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    creatng->health = -1;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->shot_model = shotng->model;
    struct Thing *killertng;
    CrDeathFlags dieflags;
    if (shotng->index == shotng->parent_idx) {
        killertng = INVALID_THING;
        dieflags = CrDed_DiedInBattle;
    } else {
        killertng = thing_get(shotng->parent_idx);
        dieflags = CrDed_DiedInBattle | ((shotst->model_flags & ShMF_NoStun)?CrDed_NoUnconscious:0);
    }
    // Friendly fire should kill the creature, not knock out
    if ((shotng->owner == creatng->owner) &! (gameadd.classic_bugs_flags & ClscBug_FriendlyFaint))
    {
        dieflags |= CrDed_NoUnconscious;
    }
    return kill_creature(creatng, killertng, shotng->owner, dieflags);
}

long melee_shot_hit_creature_at(struct Thing *shotng, struct Thing *trgtng, struct Coord3d *pos)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    //throw_strength = shotng->fall_acceleration; //this seems to be always 0, this is why it didn't work;
    long throw_strength = shotst->push_on_hit;
    if (trgtng->health < 0)
        return 0;
    struct Thing* shooter = INVALID_THING;
    if (shotng->parent_idx != shotng->index)
        shooter = thing_get(shotng->parent_idx);
    struct CreatureControl* tgcctrl = creature_control_get_from_thing(trgtng);
    long damage = get_damage_of_melee_shot(shotng, trgtng);
    if (damage > 0)
    {
      if (shotst->hit_creature.sndsample_idx > 0)
      {
          thing_play_sample(trgtng, shotst->hit_creature.sndsample_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
          play_creature_sound(trgtng, CrSnd_Hurt, 3, 0);
      }
      if (!thing_is_invalid(shooter)) {
          apply_damage_to_thing_and_display_health(trgtng, shotng->shot.damage, shotst->damage_type, shooter->owner);
      } else {
          apply_damage_to_thing_and_display_health(trgtng, shotng->shot.damage, shotst->damage_type, -1);
      }
      if (shotst->old->field_24 != 0) {
          tgcctrl->field_B1 = shotst->old->field_24;
      }
      if ( shotst->push_on_hit || creature_is_being_unconscious(trgtng))
      {
          if (creature_is_being_unconscious(trgtng)) {
              throw_strength++;
              throw_strength *= 10;
          }
          trgtng->veloc_push_add.x.val += (throw_strength * (long)shotng->velocity.x.val) / 16;
          trgtng->veloc_push_add.y.val += (throw_strength * (long)shotng->velocity.y.val) / 16;
          trgtng->state_flags |= TF1_PushAdd;
      }
      create_relevant_effect_for_shot_hitting_thing(shotng, trgtng);
      if (trgtng->health >= 0)
      {
          if (trgtng->owner != shotng->owner) {
              check_hit_when_attacking_door(trgtng);
          }
      } else
      {
          shot_kill_creature(shotng,trgtng);
      }
    }
    if (shotst->old->destroy_on_first_hit) {
        delete_thing_structure(shotng, 0);
    }
    return 1;
}

void clear_thing_acceleration(struct Thing *thing)
{
    thing->veloc_push_add.x.val = 0;
    thing->veloc_push_add.y.val = 0;
    thing->veloc_push_add.z.val = 0;
}

void set_thing_acceleration_angles(struct Thing *thing, long angle_xy, long angle_yz)
{
    thing->move_angle_xy = angle_xy;
    thing->move_angle_z = angle_yz;
    struct ComponentVector cvect;
    angles_to_vector(thing->move_angle_xy, thing->move_angle_z, 256, &cvect);
    thing->veloc_base.x.val = cvect.x;
    thing->veloc_base.y.val = cvect.y;
    thing->veloc_base.z.val = cvect.z;
}

TbBool shot_model_makes_flesh_explosion(long shot_model)
{
    if ((shot_model == ShM_Firebomb) || (shot_model == ShM_GodLightBall))
        return true;
    return false;
}

long shot_hit_creature_at(struct Thing *shotng, struct Thing *trgtng, struct Coord3d *pos)
{
    long i;
    long n;
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    //amp = shotng->fall_acceleration;
    long amp = shotst->push_on_hit;
    struct Thing* shooter = INVALID_THING;
    if (shotng->parent_idx != shotng->index) {
        shooter = thing_get(shotng->parent_idx);
    }
    // Two fighting creatures gives experience
    if (thing_is_creature(shooter) && thing_is_creature(trgtng))
    {
        apply_shot_experience_from_hitting_creature(shooter, trgtng, shotng->model);
    }
    if (((shotst->model_flags & ShMF_NoHit) != 0) || (trgtng->health < 0)) {
        return 0;
    }
    if (creature_affected_by_spell(trgtng, SplK_Rebound) && !(shotst->model_flags & ShMF_ReboundImmune))
    {
        struct Thing* killertng = INVALID_THING;
        if (shotng->index != shotng->parent_idx) {
            killertng = thing_get(shotng->parent_idx);
        }
        if (!thing_is_invalid(killertng))
        {
            if (shot_model_is_navigable(shotng->model))
            {
                shotng->shot.target_idx = 0;
            }
            struct CreatureStats* crstat = creature_stats_get_from_thing(killertng);
            struct Coord3d pos2;
            pos2.x.val = killertng->mappos.x.val;
            pos2.y.val = killertng->mappos.y.val;
            pos2.z.val = crstat->eye_height + killertng->mappos.z.val;
            clear_thing_acceleration(shotng);
            set_thing_acceleration_angles(shotng, get_angle_xy_to(&shotng->mappos, &pos2), get_angle_yz_to(&shotng->mappos, &pos2));
            shotng->parent_idx = trgtng->parent_idx;
            shotng->owner = trgtng->owner;
        } else
        {
            clear_thing_acceleration(shotng);
            i = (shotng->move_angle_xy + LbFPMath_PI) & LbFPMath_AngleMask;
            n = (shotng->move_angle_z + LbFPMath_PI) & LbFPMath_AngleMask;
            set_thing_acceleration_angles(shotng, i, n);
            if (trgtng->class_id == TCls_Creature)
            {
                shotng->parent_idx = trgtng->parent_idx;
            }
        }
        return 1;
    }
    if ((shotst->model_flags & ShMF_StrengthBased) != 0)
    {
        return melee_shot_hit_creature_at(shotng, trgtng, pos);
    }
    // Immunity to boulders
    if (shot_is_boulder(shotng))
    {
        if ((get_creature_model_flags(trgtng) & CMF_ImmuneToBoulder) != 0)
        {
            struct Thing* efftng = create_effect(&trgtng->mappos, TngEff_WoPExplosion, trgtng->owner);
            if (!thing_is_invalid(efftng)) {
                efftng->hit_type = THit_HeartOnlyNotOwn;
            }
            shotng->health = -1;
            return 1;
        }
    }
    if (shotng->shot.damage != 0)
    {
        if (shotst->model_flags & ShMF_LifeDrain)
        {
            give_shooter_drained_health(shooter, shotng->shot.damage / 2);
        }
        if (!thing_is_invalid(shooter)) {
            apply_damage_to_thing_and_display_health(trgtng, shotng->shot.damage, shotst->damage_type, shooter->owner);
        } else {
            apply_damage_to_thing_and_display_health(trgtng, shotng->shot.damage, shotst->damage_type, -1);
        }
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(trgtng);
    if (shotst->old->field_24 != 0)
    {
        if (cctrl->field_B1 == 0) {
            cctrl->field_B1 = shotst->old->field_24;
        }
    }
    if (shotst->cast_spell_kind != 0)
    {
        struct CreatureControl* scctrl = creature_control_get_from_thing(shooter);
        if (!creature_control_invalid(scctrl)) {
            n = scctrl->explevel;
        } else {
            n = 0;
        }
        if (shotst->cast_spell_kind == SplK_Disease)
        {
            cctrl->disease_caster_plyridx = shotng->owner;
        }
        apply_spell_effect_to_thing(trgtng, shotst->cast_spell_kind, n);
    }
    if (shotst->model_flags & ShMF_GroupUp)
    {
        if (thing_is_creature(shooter))
        {
            if (get_no_creatures_in_group(shooter) < GROUP_MEMBERS_COUNT) {
                add_creature_to_group(trgtng, shooter);
            }
        } else
        {
            WARNDBG(8,"The %s index %d owner %d cannot group; invalid parent",thing_model_name(shotng),(int)shotng->index,(int)shotng->owner);
        }
    }
    if (shotst->push_on_hit != 0 )
    {
        i = amp * (long)shotng->velocity.x.val;
        trgtng->veloc_push_add.x.val += i / 16;
        i = amp * (long)shotng->velocity.y.val;
        trgtng->veloc_push_add.y.val += i / 16;
        trgtng->state_flags |= TF1_PushAdd;
    }
    if (creature_is_being_unconscious(trgtng))
    {
        amp ++;
        if (gameadd.classic_bugs_flags & ClscBug_FaintedImmuneToBoulder)
        {
            amp *= 5;
            i = amp * (long)shotng->velocity.x.val;
            trgtng->veloc_push_add.x.val += i / 16;
            i = amp * (long)shotng->velocity.y.val;
            trgtng->veloc_push_add.y.val += i / 16;
            trgtng->state_flags |= TF1_PushAdd;
            if (shotst->hit_creature.sndsample_idx != 0)
            {
                play_creature_sound(trgtng, CrSnd_Hurt, 1, 0);
                thing_play_sample(trgtng, shotst->hit_creature.sndsample_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
            }
        }
        else
        {
            if (shotst->model_flags & ShMF_Boulder) //Boulders move units slightly but without purpose
            {
                if (abs(shotng->velocity.x.val) >= abs(shotng->velocity.y.val))
                {
                    i = amp * (long)shotng->velocity.x.val;
                    trgtng->veloc_push_add.x.val += i / 64;
                    i = amp * (long)shotng->velocity.x.val * (CREATURE_RANDOM(shotng, 3) - 1);
                    trgtng->veloc_push_add.y.val += i / 64;
                }
                else
                {
                    i = amp * (long)shotng->velocity.y.val;
                    trgtng->veloc_push_add.y.val += i / 64;
                    i = amp * (long)shotng->velocity.y.val * (CREATURE_RANDOM(shotng, 3) - 1);
                    trgtng->veloc_push_add.x.val += i / 64;
                }
                trgtng->state_flags |= TF1_PushAdd;
            }
            else // Normal shots blast unconscious units out of the way
            {
                amp *= 5;
                i = amp * (long)shotng->velocity.x.val;
                trgtng->veloc_push_add.x.val += i / 16;
                i = amp * (long)shotng->velocity.y.val;
                trgtng->veloc_push_add.y.val += i / 16;
                trgtng->state_flags |= TF1_PushAdd;
            }
        }
    }
    else // not for unconscious units
    {
        if (shotst->hit_creature.sndsample_idx != 0)
        {
            play_creature_sound(trgtng, CrSnd_Hurt, 1, 0);
            thing_play_sample(trgtng, shotst->hit_creature.sndsample_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        }
    }

    create_relevant_effect_for_shot_hitting_thing(shotng, trgtng);
    if (shotst->model_flags & ShMF_Boulder)
    {
        if (creature_is_being_unconscious(trgtng)  && !(gameadd.classic_bugs_flags & ClscBug_FaintedImmuneToBoulder)) //We're not actually hitting the unconscious units with a boulder
        {
            return 0;
        } 
        else
        {
            struct CreatureStats* crstat = creature_stats_get_from_thing(trgtng);
            shotng->health -= crstat->damage_to_boulder;
        }

    }
    if (trgtng->health < 0)
    {
        shot_kill_creature(shotng, trgtng);
    } else
    {
        if (trgtng->owner != shotng->owner) 
        {
            check_hit_when_attacking_door(trgtng);
        }
    }

    if (shotst->area_range != 0)
    {
        detonate_shot(shotng);
    }


    if (shotst->old->destroy_on_first_hit != 0) {
        delete_thing_structure(shotng, 0);
    }
    return 1;
}

TbBool shot_hit_shootable_thing_at(struct Thing *shotng, struct Thing *target, struct Coord3d *pos)
{
    if (!thing_exists(target))
        return false;
    if (target->class_id == TCls_Object) {
        return shot_hit_object_at(shotng, target, pos);
    }
    if (target->class_id == TCls_Creature) {
        return shot_hit_creature_at(shotng, target, pos);
    }
    if (target->class_id == TCls_DeadCreature) {
        //TODO implement shooting dead bodies
    }
    if (target->class_id == TCls_Shot) {
        // On a shot for collision, both shots are destroyed
        //TODO maybe make both shots explode instead?
        shotng->health = -1;
        target->health = -1;
        return true;
    }
    return false;
}

long collide_filter_thing_is_shootable(const struct Thing *thing, const struct Thing *parntng, long hit_targets, long a4)
{
    PlayerNumber shot_owner = -1;
    if (thing_exists(parntng))
        shot_owner = parntng->owner;
    return thing_is_shootable(thing, shot_owner, hit_targets);
}

struct Thing *get_thing_collided_with_at_satisfying_filter_for_subtile(struct Thing *shotng, struct Coord3d *pos, Thing_Collide_Func filter, long param1, long param2, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Thing* parntng = INVALID_THING;
    if (shotng->parent_idx > 0) {
        parntng = thing_get(shotng->parent_idx);
    }
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
        if (thing->index != shotng->index)
        {
            if (filter(thing, parntng, param1, param2))
            {
                if (things_collide_while_first_moves_to(shotng, pos, thing)) {
                    return thing;
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

struct Thing *get_thing_collided_with_at_satisfying_filter(struct Thing *shotng, struct Coord3d *pos, Thing_Collide_Func filter, long hit_targets, long a5)
{
    MapSubtlCoord stl_x_min;
    MapSubtlCoord stl_y_min;
    MapSubtlCoord stl_x_max;
    MapSubtlCoord stl_y_max;
    {
        int radius = 384;
        stl_x_min = coord_subtile(pos->x.val - radius);
        if (stl_x_min < 0)
            stl_x_min = 0;
        stl_y_min = coord_subtile(pos->y.val - radius);
        if (stl_y_min < 0)
            stl_y_min = 0;
        stl_x_max = coord_subtile(pos->x.val + radius);
        if (stl_x_max > map_subtiles_x)
            stl_x_max = map_subtiles_x;
        stl_y_max = coord_subtile(pos->y.val + radius);
        if (stl_y_max > map_subtiles_y)
            stl_y_max = map_subtiles_y;
    }
    for (MapSubtlCoord stl_y = stl_y_min; stl_y <= stl_y_max; stl_y++)
    {
        for (MapSubtlCoord stl_x = stl_x_min; stl_x <= stl_x_max; stl_x++)
        {
            struct Thing* coltng = get_thing_collided_with_at_satisfying_filter_for_subtile(shotng, pos, filter, hit_targets, a5, stl_x, stl_y);
            if (!thing_is_invalid(coltng)) {
                return coltng;
            }
        }
    }
    return INVALID_THING;
}

/**
 * Processes hitting another thing.
 *
 * @param shotng The thing to be moved.
 * @param nxpos Next position of the thing.
 * @return Gives true if the shot hit something and was destroyed.
 *     If the shot wasn't detonated, then the function returns false.
 * @note This function may delete the thing given in parameter.
 */
TbBool shot_hit_something_while_moving(struct Thing *shotng, struct Coord3d *nxpos)
{
    SYNCDBG(18,"Starting for %s index %d, hit type %d",thing_model_name(shotng),(int)shotng->index, (int)shotng->shot.hit_type);
    struct Thing* targetng = INVALID_THING;
    HitTargetFlags hit_targets = hit_type_to_hit_targets(shotng->shot.hit_type);
    targetng = get_thing_collided_with_at_satisfying_filter(shotng, nxpos, collide_filter_thing_is_shootable, hit_targets, 0);
    if (thing_is_invalid(targetng)) {
        return false;
    }
    SYNCDBG(18,"The %s index %d, collided with %s index %d",thing_model_name(shotng),(int)shotng->index,thing_model_name(targetng),(int)targetng->index);
    if (shot_hit_shootable_thing_at(shotng, targetng, nxpos)) {
        return true;
    }
    return false;
}

TngUpdateRet move_shot(struct Thing *shotng)
{
    SYNCDBG(18,"Starting for %s index %d",thing_model_name(shotng),(int)shotng->index);
    TRACE_THING(shotng);

    struct Coord3d pos;
    TbBool move_allowed = get_thing_next_position(&pos, shotng);
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    if (!(shotst->model_flags & ShMF_NoHit))
    {
        if (shot_hit_something_while_moving(shotng, &pos)) {
            return TUFRet_Deleted;
        }
    }
    if ((shotng->movement_flags & TMvF_Unknown10) != 0)
    {
      if ((shotst->model_flags & ShMF_StrengthBased) && thing_in_wall_at(shotng, &pos)) {
          if (shot_hit_door_at(shotng, &pos)) {
              return TUFRet_Deleted;
          }
      }
    } else
    {
      if ((!move_allowed) || thing_in_wall_at(shotng, &pos)) {
          if (shot_hit_wall_at(shotng, &pos)) {
              return TUFRet_Deleted;
          }
      }
    }
    move_thing_in_map(shotng, &pos);
    return TUFRet_Modified;
}

TngUpdateRet update_shot(struct Thing *thing)
{
    struct Thing *target;
    struct Coord3d pos1;
    struct Coord3d pos2;
    struct CoordDelta3d dtpos;
    SYNCDBG(18,"Starting for index %d, model %d",(int)thing->index,(int)thing->model);
    TRACE_THING(thing);
    TbBool hit = false;
    struct ShotConfigStats* shotst = get_shot_model_stats(thing->model);
    struct PlayerInfo* myplyr = get_my_player();
    if (shotst->shot_sound != 0)
    {
        if (!S3DEmitterIsPlayingSample(thing->snd_emitter_id, shotst->shot_sound, 0))
            thing_play_sample(thing, shotst->shot_sound, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    }
    if (!shotst->no_air_damage)
    {
        thing->health--;
    }
    if (thing->health < 0)
    {
        hit = true;
    } else
    {
        long i;
        if (shotst->model_flags & ShMF_Navigable) //Navigable shot property combines with other shots.
        {
            target = thing_get(thing->shot.target_idx);
            struct ComponentVector cvect;
            if ((thing_exists(target)) && (target->class_id == TCls_Creature))
            {
                pos2.x.val = target->mappos.x.val;
                pos2.y.val = target->mappos.y.val;
                pos2.z.val = target->mappos.z.val;
                pos2.z.val += (target->clipbox_size_yz >> 1);
                thing->move_angle_xy = get_angle_xy_to(&thing->mappos, &pos2);
                thing->move_angle_z = get_angle_yz_to(&thing->mappos, &pos2);
                angles_to_vector(thing->move_angle_xy, thing->move_angle_z, shotst->speed, &cvect);
                dtpos.x.val = cvect.x - thing->veloc_base.x.val;
                dtpos.y.val = cvect.y - thing->veloc_base.y.val;
                dtpos.z.val = cvect.z - thing->veloc_base.z.val;
                cvect.x = dtpos.x.val;
                cvect.y = dtpos.y.val;
                cvect.z = dtpos.z.val;
                i = LbSqrL(dtpos.x.val*(long)dtpos.x.val + dtpos.y.val*(long)dtpos.y.val + dtpos.z.val*(long)dtpos.z.val);
                if (i > 128)
                {
                  dtpos.x.val = ((long)cvect.x << 7) / i;
                  dtpos.y.val = ((long)cvect.y << 7) / i;
                  dtpos.z.val = ((long)cvect.z << 7) / i;
                  cvect.x = dtpos.x.val;
                  cvect.y = dtpos.y.val;
                  cvect.z = dtpos.z.val;
                }
                thing->veloc_push_add.x.val += cvect.x;
                thing->veloc_push_add.y.val += cvect.y;
                thing->veloc_push_add.z.val += cvect.z;
                thing->state_flags |= TF1_PushAdd;
            }
        }
        if (shotst->model_flags & ShMF_AlarmsUnits)
        {
            affect_nearby_friends_with_alarm(thing);
        }
        switch (thing->model)
        {
        case ShM_Firebomb:
            for (i = 2; i > 0; i--)
            {
              pos1.x.val = thing->mappos.x.val - UNSYNC_RANDOM(127) + 63;
              pos1.y.val = thing->mappos.y.val - UNSYNC_RANDOM(127) + 63;
              pos1.z.val = thing->mappos.z.val - UNSYNC_RANDOM(127) + 63;
              create_thing(&pos1, TCls_EffectElem, TngEffElm_Blast1, thing->owner, -1);
            }
            break;
        case ShM_Lightning:
        {
            struct PlayerInfo* player;
            if (lightning_is_close_to_player(myplyr, &thing->mappos))
            {
              if (is_my_player_number(thing->owner))
              {
                  player = get_player(thing->owner);
                  if ((thing->parent_idx != 0) && (myplyr->controlled_thing_idx == thing->parent_idx))
                  {
                      PaletteSetPlayerPalette(player, lightning_palette);
                      myplyr->additional_flags |= PlaAF_LightningPaletteIsActive;
                  }
              }
            }
            break;
        }
        case ShM_Wind:
            for (i = 10; i > 0; i--)
            {
              pos1.x.val = thing->mappos.x.val - UNSYNC_RANDOM(1023) + 511;
              pos1.y.val = thing->mappos.y.val - UNSYNC_RANDOM(1023) + 511;
              pos1.z.val = thing->mappos.z.val - UNSYNC_RANDOM(1023) + 511;
              create_thing(&pos1, TCls_EffectElem, TngEffElm_Leaves1, thing->owner, -1);
            }
            affect_nearby_enemy_creatures_with_wind(thing);
            break;
        case ShM_Grenade:
            thing->move_angle_xy = (thing->move_angle_xy + LbFPMath_PI/9) & LbFPMath_AngleMask;
            break;
        case ShM_Boulder:
        case ShM_SolidBoulder:
            if (apply_wallhug_force_to_boulder(thing))
              hit = true;
            break;
        case ShM_GodLightning:
            draw_god_lightning(thing);
            lightning_modify_palette(thing);
            break;
        /**case ShM_Vortex:
            //Not implemented, due to limited amount of shots, replaced by Lizard
            affect_nearby_stuff_with_vortex(thing);
            break;
            **/
        case ShM_Lizard:
            thing->move_angle_xy = (thing->move_angle_xy + LbFPMath_PI/9) & LbFPMath_AngleMask;
            int skill = thing->shot.dexterity;
            target = thing_get(thing->shot.target_idx);
            if (thing_is_invalid(target)) break;
            MapCoordDelta dist;
            if (skill <= 35)
            {
                dist = get_2d_distance(&thing->mappos, &target->mappos);
                if (dist <= 260) hit = true;
            }
            else
            {
                struct Coord3d target_pos;
                target_pos.x.val = thing->price.number;
                target_pos.y.val = thing->shot.byte_19 * gameadd.crtr_conf.sprite_size;
                target_pos.z.val = target->mappos.z.val;
                dist = get_2d_distance(&thing->mappos, &target_pos);
                if (dist <= 260) hit = true;
            }
            break;
        case ShM_GodLightBall:
            update_god_lightning_ball(thing);
            break;
        case ShM_TrapLightning:
            if (((game.play_gameturn - thing->creation_turn) % 16) == 0)
            {
              thing->shot.byte_19 = 5;
              god_lightning_choose_next_creature(thing);
              target = thing_get(thing->shot.target_idx);
              if (thing_exists(target))
              {
                  shotst = get_shot_model_stats(ShM_GodLightBall);
                  draw_lightning(&thing->mappos,&target->mappos, 96, TngEffElm_ElectricBall3);
                  apply_damage_to_thing_and_display_health(target, shotst->damage, shotst->damage_type, thing->owner);
              }
            }
            break;
        case ShM_Disease:
            for (i = 1; i > 0; i--)
            {
              pos1.x.val = thing->mappos.x.val - UNSYNC_RANDOM(511) + 255;
              pos1.y.val = thing->mappos.y.val - UNSYNC_RANDOM(511) + 255;
              pos1.z.val = thing->mappos.z.val - UNSYNC_RANDOM(511) + 255;
              create_thing(&pos1, TCls_EffectElem, TngEffElm_DiseaseFly, thing->owner, -1);
            }
            break;
        default:
            // All shots that do not require special processing
            break;
        }
    }
    if (!thing_exists(thing)) {
        WARNLOG("Thing disappeared during update");
        return TUFRet_Deleted;
    }
    if (hit) {
        detonate_shot(thing);
        return TUFRet_Deleted;
    }
    return move_shot(thing);
}

struct Thing *create_shot(struct Coord3d *pos, unsigned short model, unsigned short owner)
{
    if ( !i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots) )
    {
        ERRORDBG(3,"Cannot create shot %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    struct ShotConfigStats* shotst = get_shot_model_stats(model);
    struct Thing* thing = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate shot %d for player %d, but failed.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->creation_turn = game.play_gameturn;
    thing->class_id = TCls_Shot;
    thing->model = model;
    memcpy(&thing->mappos,pos,sizeof(struct Coord3d));
    thing->parent_idx = thing->index;
    thing->owner = owner;
    thing->bounce_angle = shotst->bounce_angle;
    thing->fall_acceleration = shotst->fall_acceleration;
    thing->field_21 = shotst->old->field_10;
    thing->field_23 = shotst->old->field_11;
    thing->field_24 = shotst->old->field_12;
    thing->movement_flags ^= (thing->movement_flags ^ TMvF_Unknown08 * shotst->old->field_13) & TMvF_Unknown08;
    set_thing_draw(thing, shotst->sprite_anim_idx, 256, shotst->sprite_size_max, 0, 0, 2);
    thing->field_4F ^= (thing->field_4F ^ 0x02 * shotst->old->field_6) & TF4F_Unknown02;
    thing->field_4F ^= thing->field_4F ^ ((thing->field_4F ^ TF4F_Transpar_8 * shotst->animation_transparency) & (TF4F_Transpar_Flags));
    thing->field_4F ^= (thing->field_4F ^ shotst->old->field_7) & TF4F_Unknown01;
    thing->clipbox_size_xy = shotst->size_xy;
    thing->clipbox_size_yz = shotst->size_yz;
    thing->solid_size_xy = shotst->size_xy;
    thing->solid_size_yz = shotst->size_yz;
    thing->shot.damage = shotst->damage;
    thing->shot.dexterity = 255;
    thing->health = shotst->health;
    if (shotst->old->lightf_50)
    {
        struct InitLight ilght;
        LbMemorySet(&ilght, 0, sizeof(struct InitLight));
        memcpy(&ilght.mappos,&thing->mappos,sizeof(struct Coord3d));
        ilght.radius = shotst->old->lightf_50;
        ilght.intensity = shotst->old->lightf_52;
        ilght.is_dynamic = 1;
        ilght.field_3 = shotst->old->lightf_53;
        thing->light_id = light_create_light(&ilght);
        if (thing->light_id == 0) {
            // Being out of free lights is quite common - so info instead of warning here
            SYNCDBG(8,"Cannot allocate dynamic light to %s.",thing_model_name(thing));
        }
    }
    place_thing_in_mapwho(thing);
    add_thing_to_its_class_list(thing);
    return thing;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
