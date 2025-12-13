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
#include "pre_inc.h"
#include "thing_shots.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "bflib_sound.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "thing_data.h"
#include "thing_factory.h"
#include "thing_effects.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "thing_objects.h"
#include "front_simple.h"
#include "thing_stats.h"
#include "map_blocks.h"
#include "magic_powers.h"
#include "room_garden.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "power_process.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "creature_states.h"
#include "creature_groups.h"
#include "game_legacy.h"
#include "engine_lenses.h"

#include "keeperfx.hpp"
#include "post_inc.h"

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

TbBool detonate_shot(struct Thing *shotng, TbBool destroy)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    SYNCDBG(8,"Starting for %s index %d owner %d",thing_model_name(shotng),(int)shotng->index,(int)shotng->owner);
    struct Thing* castng = get_parent_thing(shotng);
    TRACE_THING(castng);
    struct PlayerInfo* myplyr = get_my_player();
    KeepPwrLevel power_level;
    long damage;
    // If the shot has area_range, then make area damage
    if (shotst->area_range != 0) {
        unsigned char luck = 0;
        CrtrExpLevel exp_level = 0;
        if (thing_is_creature(castng))
        {
            struct CreatureModelConfig* crconf = creature_stats_get_from_thing(castng);
            //TODO SPELLS Spell level should be taken from within the shot, not from caster creature
            // Caster may have leveled up, or even may be already dead
            // But currently shot do not store its level, so we don't really have a choice
            struct CreatureControl* cctrl = creature_control_get_from_thing(castng);
            luck = crconf->luck;
            exp_level = cctrl->exp_level;
        }
        long dist = compute_creature_attack_range(shotst->area_range * COORD_PER_STL, luck, exp_level);
        if (flag_is_set(shotst->model_flags, ShMF_StrengthBased))
        {
            if (shotst->area_damage == 0)
            {
                damage = shotng->shot.damage;
            }
            else
            {
                damage = (shotst->area_damage * shotng->shot.damage) / 100;
            }
        }
        else
        {
            damage = compute_creature_attack_spell_damage(shotst->area_damage, luck, exp_level, shotng->owner);
        }
        HitTargetFlags hit_targets = hit_type_to_hit_targets(shotst->area_hit_type);
        explosion_affecting_area(shotng, &shotng->mappos, dist, damage, shotst->area_blow, hit_targets);
    }
    create_used_effect_or_element(&shotng->mappos, shotst->explode.effect1_model, shotng->owner, shotng->parent_idx); //Parent of explosion is set to caster creature/trap
    create_used_effect_or_element(&shotng->mappos, shotst->explode.effect2_model, shotng->owner, shotng->parent_idx);
    if (shotst->explode.around_effect1_model != 0)
    {
        create_effect_around_thing(shotng, shotst->explode.around_effect1_model);
    }
    if (shotst->explode.around_effect2_model != 0)
    {
        create_effect_around_thing(shotng, shotst->explode.around_effect2_model);
    }
    //TODO CONFIG shot model dependency, make config option instead
    switch (shotng->model)
    {
    case ShM_Lightning:
    case ShM_GodLightning:
    case ShM_GodLightBall:
        if (lens_mode != 0) {
            PaletteSetPlayerPalette(myplyr, engine_palette);
        }
        break;
    case ShM_TrapTNT:
        power_level = shotng->shot.damage;
        if (power_level > SPELL_MAX_LEVEL)
        {
            power_level = SPELL_MAX_LEVEL;
        }
        magic_use_power_direct(shotng->owner, PwrK_DESTRWALLS, power_level, shotng->mappos.x.stl.num, shotng->mappos.y.stl.num,INVALID_THING, PwMod_CastForFree);
        break;
    default:
        break;
    }
    if (destroy)
    {
        delete_thing_structure(shotng, 0);
    }
    return true;
}

TbBool give_gold_to_creature_or_drop_on_map_when_digging(struct Thing *creatng, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long damage)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    long gold = calculate_gold_digged_out_of_slab_with_single_hit(damage, slb);
    creatng->creature.gold_carried += gold;
    if (!dungeon_invalid(dungeon)) {
        dungeon->lvstats.gold_mined += gold;
    }
    if (crconf->gold_hold <= creatng->creature.gold_carried)
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

SubtlCodedCoords process_dig_shot_hit_wall(struct Thing *thing, long blocked_flags, HitPoints *health)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    unsigned short k;
    struct Thing* diggertng = get_parent_thing(thing);
    if (!thing_exists(diggertng))
    {
        ERRORLOG("Digging shot hit wall, but there's no digger creature index %d.",thing->parent_idx);
        return 0;
    }
    TbBool can_dig;
    switch ( blocked_flags )
    {
        case SlbBloF_WalledX:
        {
            k = thing->move_angle_xy & 0xFC00;
            if (k != 0)
            {
              stl_x = thing->mappos.x.stl.num - 1;
              stl_y = thing->mappos.y.stl.num;
            }
            else
            {
              stl_x = thing->mappos.x.stl.num + 1;
              stl_y = thing->mappos.y.stl.num;
            }
            can_dig = true;
            break;
        }
        case SlbBloF_WalledY:
        {
            k = thing->move_angle_xy & 0xFE00;
            if ((k != ANGLE_NORTH) && (k != ANGLE_WEST))
            {
              stl_x = thing->mappos.x.stl.num;
              stl_y = thing->mappos.y.stl.num + 1;
            }
            else
            {
              stl_x = thing->mappos.x.stl.num;
              stl_y = thing->mappos.y.stl.num - 1;
            }
            can_dig = true;
            break;
        }
        case SlbBloF_WalledX|SlbBloF_WalledY:
        case SlbBloF_WalledX|SlbBloF_WalledY|SlbBloF_WalledZ:
        {
            k = (thing->move_angle_xy & DEGREES_315) | DEGREES_45;
            switch(k)
            {
                case ANGLE_NORTHEAST:
                {
                    stl_x = thing->mappos.x.stl.num + 1;
                    stl_y = thing->mappos.y.stl.num - 1;
                    break;
                }
                case ANGLE_SOUTHEAST:
                {
                    stl_x = thing->mappos.x.stl.num + 1;
                    stl_y = thing->mappos.y.stl.num + 1;
                    break;
                }
                case ANGLE_SOUTHWEST:
                {
                    stl_x = thing->mappos.x.stl.num - 1;
                    stl_y = thing->mappos.y.stl.num + 1;
                    break;
                }
                case ANGLE_NORTHWEST:
                {
                    stl_x = thing->mappos.x.stl.num - 1;
                    stl_y = thing->mappos.y.stl.num - 1;
                    break;
                }
                default:
                {
                    ERRORLOG("Tried to dig from subtile (%d, %d) diagonally, but angle was not diagonal: thing move angle was %d, and got a digging angle of %d.", thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->move_angle_xy, k);
                    stl_x = thing->mappos.x.stl.num;
                    stl_y = thing->mappos.y.stl.num;
                    break;
                }
            }
            can_dig = subtile_is_diggable_at_diagonal_angle(thing, k, stl_x, stl_y);
            break;
        }
        default:
        {
            stl_x = thing->mappos.x.stl.num;
            stl_y = thing->mappos.y.stl.num;
            can_dig = true;
            break;
        }
    }
    SubtlCodedCoords result = get_subtile_number(stl_x, stl_y);
    struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
    *health = slb->health;
    // You can only dig your own tiles or non-fortified neutral ground (dirt/gold)
    // If you're not the tile owner, unless the classic bug mode is enabled.
    if (!(game.conf.rules[diggertng->owner].game.classic_bugs_flags & ClscBug_BreakNeutralWalls))
    {
        if (slabmap_owner(slb) != diggertng->owner)
        {
            struct SlabConfigStats* slabst = get_slab_stats(slb);
            // and if it's fortified
            if (slabst->category == SlbAtCtg_FortifiedWall)
            {
                // digging not allowed
                return result;
            }
        }
    }
    else
    {
        if ((slabmap_owner(slb) != game.neutral_player_num) && (slabmap_owner(slb) != diggertng->owner))
        {
            return result;
        }
    }

    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if ((mapblk->flags & SlbAtFlg_IsRoom) != 0)
    {
        if (diggertng->creature.gold_carried > 0)
        {
            struct Thing* gldtng;
            struct Room* room;
            room = get_room_xy(stl_x, stl_y);
            if (!room_is_invalid(room))
            {
                if (room_role_matches(room->kind, RoRoF_GoldStorage))
                {
                    if (room->owner == diggertng->owner)
                    {
                        gldtng = drop_gold_pile(diggertng->creature.gold_carried, &diggertng->mappos);
                        diggertng->creature.gold_carried = 0;
                        gold_being_dropped_at_treasury(gldtng, room);
                    }
                }
            }
        }
        // Room pillars cannot be dug
        return result;
    }

    // Doors cannot be dug
    if ((mapblk->flags & SlbAtFlg_IsDoor) != 0)
    {
        return result;
    }
    if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
    {
        return result;
    }
    if (can_dig)
    {
        int damage = thing->shot.damage;
        if ((damage >= slb->health) && !slab_kind_is_indestructible(slb->kind))
        {
            if ((mapblk->flags & SlbAtFlg_Valuable) != 0)
            { // Valuables require counting gold
                if (!slab_kind_is_indestructible(slb->kind))
                {
                    slb->health -= damage; // otherwise, we won't get the final lot of gold
                }
                give_gold_to_creature_or_drop_on_map_when_digging(diggertng, stl_x, stl_y, damage);
                mine_out_block(stl_x, stl_y, diggertng->owner);
                thing_play_sample(diggertng, 72+SOUND_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
            } else
            if ((mapblk->flags & SlbAtFlg_IsDoor) == 0)
            { // All non-gold and non-door slabs are just destroyed
                dig_out_block(stl_x, stl_y, diggertng->owner);
                thing_play_sample(diggertng, 72+SOUND_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
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
    return result;
}

struct Thing *create_shot_hit_effect(struct Coord3d *effpos, long effowner, EffectOrEffElModel eff_kind, long snd_idx, long snd_range, ThingIndex parent_idx)
{
    struct Thing* efftng = INVALID_THING;
    if (eff_kind != 0) {
        efftng = create_used_effect_or_element(effpos, eff_kind, effowner, parent_idx);
        TRACE_THING(efftng);
    }
    if (snd_idx > 0)
    {
        if (!thing_is_invalid(efftng))
        {
            long i = snd_idx;
            if (snd_range > 1)
                i += SOUND_RANDOM(snd_range);
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
    TbBool digging = (shotst->model_flags & ShMF_Digging);
    HitPoints old_health = 0;
    EffectOrEffElModel eff_kind;
    short smpl_idx;
    unsigned char range;
    struct SlabMap* slb;
    MapSubtlCoord hit_stl_x, hit_stl_y;
    if (digging)
    {
        SubtlCodedCoords hit_stl_num = process_dig_shot_hit_wall(shotng, blocked_flags, &old_health);
        hit_stl_x = stl_num_decode_x(hit_stl_num);
        hit_stl_y = stl_num_decode_y(hit_stl_num);
    }
    else
    {
        unsigned short angle;
        switch ( blocked_flags )
        {
            case SlbBloF_WalledX:
            {
                angle = shotng->move_angle_xy & 0xFC00;
                if (angle != 0)
                {
                  hit_stl_x = pos->x.stl.num - 1;
                  hit_stl_y = pos->y.stl.num;
                }
                else
                {
                  hit_stl_x = pos->x.stl.num + 1;
                  hit_stl_y = pos->y.stl.num;
                }
                break;
            }
            case SlbBloF_WalledY:
            {
                angle = shotng->move_angle_xy & 0xFE00;
                if ((angle != ANGLE_NORTH) && (angle != ANGLE_WEST))
                {
                  hit_stl_x = pos->x.stl.num;
                  hit_stl_y = pos->y.stl.num + 1;
                }
                else
                {
                  hit_stl_x = pos->x.stl.num;
                  hit_stl_y = pos->y.stl.num - 1;
                }
                break;
            }
            case SlbBloF_WalledX|SlbBloF_WalledY:
            case SlbBloF_WalledX|SlbBloF_WalledY|SlbBloF_WalledZ:
            {
                angle = (shotng->move_angle_xy & DEGREES_315) | DEGREES_45;
                switch(angle)
                {
                    case ANGLE_NORTHEAST:
                    {
                        hit_stl_x = pos->x.stl.num + 1;
                        hit_stl_y = pos->y.stl.num - 1;
                        break;
                    }
                    case ANGLE_SOUTHEAST:
                    {
                        hit_stl_x = pos->x.stl.num + 1;
                        hit_stl_y = pos->y.stl.num + 1;
                        break;
                    }
                    case ANGLE_SOUTHWEST:
                    {
                        hit_stl_x = pos->x.stl.num - 1;
                        hit_stl_y = pos->y.stl.num + 1;
                        break;
                    }
                    case ANGLE_NORTHWEST:
                    {
                        hit_stl_x = pos->x.stl.num - 1;
                        hit_stl_y = pos->y.stl.num - 1;
                        break;
                    }
                    default:
                    {
                        ERRORLOG("Hit from subtile (%u, %u) diagonally, but angle was not diagonal: thing move angle was %d, and got a digging angle of %u.",
                            shotng->mappos.x.stl.num, shotng->mappos.y.stl.num, shotng->move_angle_xy, angle);
                        hit_stl_x = shotng->mappos.x.stl.num;
                        hit_stl_y = shotng->mappos.y.stl.num;
                        break;
                    }
                }
                break;
            }
            default:
            {
                hit_stl_x = shotng->mappos.x.stl.num;
                hit_stl_y = shotng->mappos.y.stl.num;
                break;
            }
        }
    }

    // If blocked by a higher wall
    if ((blocked_flags & SlbBloF_WalledZ) != 0)
    {
        long cube_id = get_top_cube_at(pos->x.stl.num, pos->y.stl.num, NULL);
        doortng = get_door_for_position(hit_stl_x, hit_stl_y);
        if (!thing_is_invalid(doortng))
        {
            efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->hit_door.effect_model, shotst->hit_door.sndsample_idx, shotst->hit_door.sndsample_range, shotng->index);
            if (!shotst->hit_door.withstand)
              destroy_shot = 1;
            i = calculate_shot_real_damage_to_door(doortng, shotng);
            apply_damage_to_thing(doortng, i, -1);
            reveal_secret_door_to_player(doortng,shotng->owner);
        } else
        if (cube_is_water(cube_id))
        {
            efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->hit_water.effect_model, shotst->hit_water.sndsample_idx, shotst->hit_water.sndsample_range, shotng->index);
            if (!shotst->hit_water.withstand) {
                destroy_shot = 1;
            }
        } else
        if (cube_is_lava(cube_id))
        {
            efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->hit_lava.effect_model, shotst->hit_lava.sndsample_idx, shotst->hit_lava.sndsample_range, shotng->index);
            if (!shotst->hit_lava.withstand) {
                destroy_shot = 1;
            }
        } else
        {
            eff_kind = shotst->hit_generic.effect_model;
            smpl_idx = shotst->hit_generic.sndsample_idx;
            range = shotst->hit_generic.sndsample_range;
            if (digging)
            {
                slb = get_slabmap_for_subtile(hit_stl_x, hit_stl_y);
                if ((old_health > slb->health) || (slb->kind == SlbT_GEMS))
                {
                    smpl_idx = shotst->dig.sndsample_idx;
                    range = shotst->dig.sndsample_range;
                    eff_kind = shotst->dig.effect_model;
                }
            }
            efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, eff_kind, smpl_idx, range, shotng->index);
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
                if (shotng->shot_lizard2.range >= THING_RANDOM(shotng, 90))
                {
                    struct Coord3d target_pos;
                    target_pos.x.val = shotng->shot_lizard.x;
                    target_pos.y.val = shotng->shot_lizard.posint * game.conf.crtr_conf.sprite_size;
                    target_pos.z.val = pos->z.val;
                    const MapCoordDelta dist = get_2d_distance(pos, &target_pos);
                    if (dist <= 800) return detonate_shot(shotng, true);
                }
            }
            doortng = get_door_for_position(hit_stl_x, hit_stl_y);
            if (!thing_is_invalid(doortng))
            {
                efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, shotst->hit_door.effect_model, shotst->hit_door.sndsample_idx, shotst->hit_door.sndsample_range, shotng->index);
                if (!shotst->hit_door.withstand)
                    destroy_shot = 1;
                i = calculate_shot_real_damage_to_door(doortng, shotng);
                apply_damage_to_thing(doortng, i, -1);
                reveal_secret_door_to_player(doortng,shotng->owner);
            } else
            {
                eff_kind = shotst->hit_generic.effect_model;
                smpl_idx = shotst->hit_generic.sndsample_idx;
                range = shotst->hit_generic.sndsample_range;
                if (digging)
                {
                    slb = get_slabmap_for_subtile(hit_stl_x, hit_stl_y);
                    if ((old_health > slb->health) || (slb->kind == SlbT_GEMS))
                    {
                        smpl_idx = shotst->dig.sndsample_idx;
                        range = shotst->dig.sndsample_range;
                        eff_kind = shotst->dig.effect_model;
                    }
                }
                efftng = create_shot_hit_effect(&shotng->mappos, shotng->owner, eff_kind, smpl_idx, range, shotng->index);

                if (!shotst->hit_generic.withstand) {
                    destroy_shot = 1;
                }
            }
        }
    }
    if (!thing_is_invalid(efftng)) {
        efftng->shot_effect.hit_type = shotst->area_hit_type;
        efftng->shot_effect.parent_class_id = TCls_Shot;
        efftng->shot_effect.parent_model = shotng->model;
    }
    if ( destroy_shot )
    {
        return detonate_shot(shotng, true);
    }
    if (!flag_is_set(shotst->model_flags,ShMF_WallPierce))
    {
        if (shotng->bounce_angle <= 0)
        {
            slide_thing_against_wall_at(shotng, pos, blocked_flags);
        }
        else
        {
            bounce_thing_off_wall_at(shotng, pos, blocked_flags);
        }
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
            if (shotst->hit_door.effect_model != 0)
            {
                efftng = create_used_effect_or_element(&shotng->mappos, shotst->hit_door.effect_model, shotng->owner, shotng->index);
            }
            // If the shot hit is supposed to create sound
            int n = shotst->hit_door.sndsample_idx;
            int i;
            if (n > 0)
            {
                if (!thing_is_invalid(efftng))
                {
                    i = shotst->hit_door.sndsample_range;
                    thing_play_sample(efftng, n + SOUND_RANDOM(i), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
                }
            }
            // Shall the shot be destroyed on impact
            if (!shotst->hit_door.withstand)
            {
                shot_explodes = true;
            }
            // Apply damage to the door
            i = calculate_shot_real_damage_to_door(doortng, shotng);
            apply_damage_to_thing(doortng, i, -1);
            reveal_secret_door_to_player(doortng,shotng->owner);
      }
    }
    if (!thing_is_invalid(efftng)) {
        efftng->shot_effect.hit_type = shotst->area_hit_type;
        efftng->shot_effect.parent_class_id = TCls_Shot;
        efftng->shot_effect.parent_model = shotng->model;
    }
    if ( shot_explodes )
    {
        return detonate_shot(shotng, true);
    }
    if (!(shotst->model_flags & ShMF_Penetrating))
    {
        if (shotng->bounce_angle <= 0)
        {
            slide_thing_against_wall_at(shotng, pos, blocked_flags);
        }
        else
        {
            bounce_thing_off_wall_at(shotng, pos, blocked_flags);
        }
    }
    return false;
}

TbBool apply_shot_experience(struct Thing *shooter, long exp_factor, CrtrExpLevel exp_level, long shot_model)
{
    if (!creature_can_gain_experience(shooter))
        return false;
    struct CreatureControl* shcctrl = creature_control_get_from_thing(shooter);
    struct ShotConfigStats* shotst = get_shot_model_stats(shot_model);
    long exp_mag = shotst->experience_given_to_shooter;
    long exp_gained = (exp_mag * (exp_factor + game.conf.crtr_conf.exp.exp_on_hitting_increase_on_exp * exp_factor * (long)exp_level / 100) << 8) / 256;
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
    struct CreatureModelConfig* tgcrconf = creature_stats_get_from_thing(target);
    return apply_shot_experience(shooter, tgcrconf->exp_for_hitting, tgcctrl->exp_level, shot_model);
}

long shot_kill_object(struct Thing *shotng, struct Thing *target)
{
    if (thing_is_dungeon_heart(target))
    {
        target->active_state = ObSt_BeingDestroyed;
        target->health = -1;
        if (is_my_player_number(shotng->owner))
        {
            struct PlayerInfo* player = get_player(target->owner);
            if (player_exists(player) && (player->is_active == 1) && (shotng->owner != target->owner))
            {
                output_message(SMsg_DefeatedKeeper, 0);
            }
        }
        struct Dungeon* dungeon = get_players_num_dungeon(shotng->owner);
        if (!dungeon_invalid(dungeon)) {
            dungeon->lvstats.keepers_destroyed++;
            dungeon->lvstats.keeper_destroyed[target->owner]++;
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

static TbBool shot_hit_trap_at(struct Thing* shotng, struct Thing* target, struct Coord3d* pos)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    if (target->class_id != TCls_Trap) {
        return false;
    }
    if (shotst->model_flags & ShMF_NoHit) {
        return false;
    }
    if (target->health < 0) {
        return false;
    }
    struct Thing* shootertng = INVALID_THING;
    if (shotng->parent_idx != shotng->index) {
        shootertng = thing_get(shotng->parent_idx);
    }
    int i = shotst->hit_generic.sndsample_idx;
    if (i > 0) {
        thing_play_sample(target, i, NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
    }

    HitPoints damage_done = 0;
    if (shotng->shot.damage)
    {

        if ((thing_is_destructible_trap(target) > 0) || ((thing_is_destructible_trap(target) > -1) && (shotst->model_flags & ShMF_Disarming)))
        {
            damage_done = apply_damage_to_thing(target, shotng->shot.damage, -1);

            // Drain allows caster to regain half of damage
            if ((shotst->model_flags & ShMF_LifeDrain) && thing_is_creature(shootertng))
            {
                give_shooter_drained_health(shootertng, damage_done / 2);
            }
        }
    }
    create_relevant_effect_for_shot_hitting_thing(shotng, target);
    if (target->health < 0)
    {
        struct TrapConfigStats* trapst = get_trap_model_stats(target->model);
        if (trapst->destroyed_effect != 0)
        {
            create_used_effect_or_element(&target->mappos, trapst->destroyed_effect, target->owner, shotng->index);
        }
        if (((trapst->unstable == 1) && !(shotst->model_flags & ShMF_Disarming)) || trapst->unstable == 2)
        {
            activate_trap(target, target);
            struct Dungeon* dungeon = get_dungeon(target->owner);
            if (!dungeon_invalid(dungeon))
            {
                dungeon->trap_info.activated[target->trap.flag_number]++;
                if (target->trap.flag_number > 0)
                {
                    memcpy(&dungeon->last_trap_event_location, &target->mappos, sizeof(struct Coord3d));
                }
            }
            process_trap_charge(target);
        }
    }
    if (shotst->destroy_on_first_hit) {
        delete_thing_structure(shotng, 0);
        // If thing was deleted something was hit
        // To test this use zero damage shots
        return true;
    }
    return damage_done > 0;
}

static TbBool shot_hit_object_at(struct Thing *shotng, struct Thing *target, struct Coord3d *pos)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    if (!thing_is_object(target)) {
        return false;
    }
    if (shotst->model_flags & ShMF_NoHit) {
        return false;
    }
    if (target->health < 0) {
        return false;
    }
    struct Thing* shootertng = get_parent_thing(shotng);
    if (thing_is_dungeon_heart(target))
    {
        if (shotst->hit_heart.effect_model != 0)
        {
            create_used_effect_or_element(&shotng->mappos, shotst->hit_heart.effect_model, shotng->owner, shotng->index);
        }
        if (shotst->hit_heart.sndsample_idx > 0)
        {
            thing_play_sample(target, shotst->hit_heart.sndsample_idx + SOUND_RANDOM(shotst->hit_heart.sndsample_range), NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
        }
        if (shotng->owner != target->owner)
        {
            event_create_event_or_update_nearby_existing_event(shootertng->mappos.x.val, shootertng->mappos.y.val, EvKind_HeartAttacked, target->owner, shootertng->index);
            if (is_my_player_number(target->owner)) {
                output_message(SMsg_HeartUnderAttack, 400);
            }
        }
    } else
    {
        int i = shotst->hit_generic.sndsample_idx;
        if (i > 0) {
            thing_play_sample(target, i, NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
        }
    }

    HitPoints damage_done = 0;
    if (shotng->shot.damage)
    {
        if (object_can_be_damaged(target)) // do not damage objects that cannot be destroyed
        {
            damage_done = apply_damage_to_thing(target, shotng->shot.damage, -1);

            // Drain allows caster to regain half of damage
            if ((shotst->model_flags & ShMF_LifeDrain) && thing_is_creature(shootertng))
            {
                give_shooter_drained_health(shootertng, damage_done / 2);
            }
        }
    }
    create_relevant_effect_for_shot_hitting_thing(shotng, target);
    if (target->health < 0) {
        shot_kill_object(shotng, target);
    }
    if (shotst->area_range != 0)
    {
        return detonate_shot(shotng, shotst->destroy_on_first_hit);
    }
    if (shotst->destroy_on_first_hit) {
        delete_thing_structure(shotng, 0);
        // If thing was deleted something was hit
        // To test this use zero damage shots
        return true;
    }
    return damage_done > 0;
}

long get_damage_of_melee_shot(struct Thing *shotng, const struct Thing *target, TbBool NeverBlock)
{
    if (NeverBlock)
        return shotng->shot.damage;
    long crdefense = calculate_correct_creature_defense(target);
    long hitchance = ((long)shotng->shot.dexterity - crdefense) / 2;
    if (hitchance < -96)
    {
        hitchance = -96;
    } else
    if (hitchance > 96)
    {
        hitchance = 96;
    }
    if (THING_RANDOM(shotng, 256) < (128 + hitchance))
    {
        return shotng->shot.damage;
    }
    return -1;
}

void create_relevant_effect_for_shot_hitting_thing(struct Thing *shotng, struct Thing *target)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    if (target->class_id == TCls_Creature)
    {
        thing_play_sample(target, shotst->hit_creature.sndsample_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        if (shotst->hit_creature.effect_model != 0) {
            create_used_effect_or_element(&shotng->mappos, shotst->hit_creature.effect_model, shotng->owner, shotng->index);
        }
        if (creature_under_spell_effect(target, CSAfF_Freeze))
        {
            if (shotst->effect_frozen != 0) {
                create_used_effect_or_element(&shotng->mappos, shotst->effect_frozen, shotng->owner, shotng->index);
            }
        } else
        if (creature_model_bleeds(target->model))
        {
            if (shotst->effect_bleeding != 0) {
                create_used_effect_or_element(&shotng->mappos, shotst->effect_bleeding, shotng->owner, shotng->index);
            }
        }
    }
    if (target->class_id == TCls_Trap)
    {
        // TODO for a later PR: introduces trap/object hit, for now it uses the on hit creature sound and effect.
        thing_play_sample(target, shotst->hit_creature.sndsample_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        if (shotst->hit_creature.effect_model != 0) {
            create_used_effect_or_element(&shotng->mappos, shotst->hit_creature.effect_model, shotng->owner, shotng->index);
        }
    }
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
 * @note sometimes named shot_kills_creature().
 */
void shot_kill_creature(struct Thing *shotng, struct Thing *creatng)
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
        dieflags = CrDed_DiedInBattle | ((shotst->model_flags & ShMF_NoStun)?CrDed_NoUnconscious:0) | ((shotst->model_flags & ShMF_BlocksRebirth)? CrDed_NoRebirth : 0);
    }
    // Friendly fire should kill the creature, not knock out
    if (players_creatures_tolerate_each_other(shotng->owner,creatng->owner) &! (game.conf.rules[shotng->owner].game.classic_bugs_flags & ClscBug_FriendlyFaint))
    {
        dieflags |= CrDed_NoUnconscious;
    }
    kill_creature(creatng, killertng, shotng->owner, dieflags);
}

/*
 * Calculate the adjusted push strength based on the weight of the creature.
 * @param weight The weight of the creature.
 * @param push_strength The original push strength.
 * @return The adjusted push strength.
 */
int weight_calculated_push_strenght(int weight, int push_strength)
{
    const int min_weight = 6; // Minimum weight threshold for the creature.
    const int max_weight = game.conf.rules[0].magic.weight_calculate_push; // Maximum weight threshold for the creature.
    const int percent_factor = 1000; // Factor used to scale the weight factor to a percentage.

    // Ensure that the weight is within the valid range of min_weight to max_weight.
    if (weight < min_weight) {
        weight = min_weight;
    } else if (weight > max_weight) {
        weight = max_weight;
    }

    // Calculate the weight factor based on the creature's weight.
    int weight_factor = percent_factor - ((weight - min_weight) * percent_factor / (max_weight - min_weight));

    // Ensure the weight factor is within the valid range of 0 to @percent_factor.
    if (weight_factor < 0) {
        weight_factor = 0;
    } else if (weight_factor > percent_factor) {
        weight_factor = percent_factor;
    }

    // Calculate the adjusted push strength based on the weight factor.
    int adjusted_push_strength = (push_strength * weight_factor) / percent_factor;

    return adjusted_push_strength;
}

long melee_shot_hit_creature_at(struct Thing *shotng, struct Thing *trgtng, struct Coord3d *pos)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    long throw_strength = shotst->push_on_hit;
    int adjusted_throw_strength;
    if (trgtng->health < 0)
        return 0;
    struct Thing* shooter = get_parent_thing(shotng);
    struct CreatureControl* tgcctrl = creature_control_get_from_thing(trgtng);
    long damage = get_damage_of_melee_shot(shotng, trgtng, flag_is_set(shotst->model_flags, ShMF_NeverBlock));
    if (damage > 0)
    {
        if (shotst->hit_creature.sndsample_idx > 0)
        {
            play_creature_sound(trgtng, CrSnd_Hurt, 3, 0);
        }
        create_relevant_effect_for_shot_hitting_thing(shotng, trgtng);
        if (!thing_is_invalid(shooter)) {
            damage = apply_damage_to_thing_and_display_health(trgtng, damage, shooter->owner);
        } else {
            damage = apply_damage_to_thing_and_display_health(trgtng, damage, -1);
        }
        if (shotst->model_flags & ShMF_LifeDrain)
        {
            give_shooter_drained_health(shooter, damage / 2);
        }
        if (shotst->cast_spell_kind != 0)
        {
            struct CreatureControl* scctrl = creature_control_get_from_thing(shooter);
            CrtrExpLevel spell_level = 0;
            if (!creature_control_invalid(scctrl))
            {
                spell_level = scctrl->exp_level;
            }
            apply_spell_effect_to_thing(trgtng, shotst->cast_spell_kind, spell_level, shotng->owner);
            struct SpellConfig *spconf = get_spell_config(shotst->cast_spell_kind);
            if (flag_is_set(spconf->spell_flags, CSAfF_Disease))
            {
                tgcctrl->disease_caster_plyridx = shotng->owner;
            }
        }
        if (shotst->model_flags & ShMF_GroupUp)
        {
            if (thing_is_creature(shooter))
            {
                if (get_no_creatures_in_group(shooter) < GROUP_MEMBERS_COUNT)
                {
                    add_creature_to_group(trgtng, shooter);
                }
            }
            else
            {
                WARNDBG(8, "The %s index %d owner %d cannot group; invalid parent", thing_model_name(shotng), (int)shotng->index, (int)shotng->owner);
            }
        }
        if (shotst->target_hitstop_turns != 0)
        {
            tgcctrl->frozen_on_hit = shotst->target_hitstop_turns;
        }

        adjusted_throw_strength = throw_strength;


        if (game.conf.rules[trgtng->owner].magic.weight_calculate_push > 0)
        {
            int weight = compute_creature_weight(trgtng);
            adjusted_throw_strength = weight_calculated_push_strenght(weight, throw_strength);
        }
        if (shotst->push_on_hit || creature_is_being_unconscious(trgtng))
        {
            if (creature_is_being_unconscious(trgtng))
            {
                if (adjusted_throw_strength == 0)
                {
                    adjusted_throw_strength++;
                }
                adjusted_throw_strength *= 10;
            }
            trgtng->veloc_push_add.x.val += (adjusted_throw_strength * (long)shotng->velocity.x.val) / 16;
            trgtng->veloc_push_add.y.val += (adjusted_throw_strength * (long)shotng->velocity.y.val) / 16;
            trgtng->state_flags |= TF1_PushAdd;
        }
        if (trgtng->health >= 0)
        {
            if (trgtng->owner != shotng->owner) {
                check_hit_when_attacking_door(trgtng);
            }
        } else
        {
            shot_kill_creature(shotng,trgtng);
        }
        if (shotst->area_range != 0)
        {
            detonate_shot(shotng, shotst->destroy_on_first_hit);
        }
    }
    if (shotst->destroy_on_first_hit)
    {
        delete_thing_structure(shotng, 0);
    }
    return 1;
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
    struct ShotConfigStats* shotst = get_shot_model_stats(shot_model);
    return ((shotst->model_flags & ShMF_Exploding) != 0);
}

long shot_hit_creature_at(struct Thing *shotng, struct Thing *trgtng, struct Coord3d *pos)
{
    long i;
    long n;
    int adjusted_push_strength;
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    long push_strength = shotst->push_on_hit;
    struct Thing* shooter = get_parent_thing(shotng);
    // Two fighting creatures gives experience
    if (thing_is_creature(shooter) && thing_is_creature(trgtng))
    {
        apply_shot_experience_from_hitting_creature(shooter, trgtng, shotng->model);
    }
    if (thing_is_deployed_trap(shooter) && thing_is_creature(trgtng))
    {
        creature_start_combat_with_trap_if_available(trgtng, shooter);
    }
    if (((shotst->model_flags & ShMF_NoHit) != 0) || (trgtng->health < 0)) {
        return 0;
    }
    if (creature_under_spell_effect(trgtng, CSAfF_Rebound) && !flag_is_set(shotst->model_flags, ShMF_ReboundImmune))
    {
        struct Thing* killertng = get_parent_thing(shotng);
        if (!thing_is_invalid(killertng))
        {
            if (shot_model_is_navigable(shotng->model))
            {
                shotng->shot.target_idx = 0;
            }
            struct Coord3d pos2;
            pos2.x.val = killertng->mappos.x.val;
            pos2.y.val = killertng->mappos.y.val;
            if(thing_is_deployed_trap(killertng))
            {
                pos2.z.val = killertng->mappos.z.val;
                pos2.z.val += (killertng->clipbox_size_z >> 1);
                if (thing_is_destructible_trap(killertng))
                {
                    shotng->shot.hit_type = THit_CrtrsNObjctsNotOwn;
                }
            }
            else
            {
                struct CreatureControl* cctrl = creature_control_get_from_thing(killertng);
                short target_center = (killertng->solid_size_z + ((killertng->solid_size_z * game.conf.crtr_conf.exp.size_increase_on_exp * cctrl->exp_level) / 100)) / 2;
                pos2.z.val = target_center + killertng->mappos.z.val;
            }
            clear_thing_acceleration(shotng);
            set_thing_acceleration_angles(shotng, get_angle_xy_to(&shotng->mappos, &pos2), get_angle_yz_to(&shotng->mappos, &pos2));
            shotng->parent_idx = trgtng->parent_idx;
            shotng->owner = trgtng->owner;
        } else
        {
            clear_thing_acceleration(shotng);
            i = (shotng->move_angle_xy + DEGREES_180) & ANGLE_MASK;
            n = (shotng->move_angle_z + DEGREES_180) & ANGLE_MASK;
            set_thing_acceleration_angles(shotng, i, n);
            if (trgtng->class_id == TCls_Creature)
            {
                shotng->parent_idx = trgtng->parent_idx;
            }
        }
        return 1;
    }
    if (flag_is_set(shotst->model_flags,ShMF_StrengthBased))
    {
        return melee_shot_hit_creature_at(shotng, trgtng, pos);
    }
    // Immunity to boulders
    if (shot_is_boulder(shotng))
    {
        if ((get_creature_model_flags(trgtng) & CMF_ImmuneToBoulder) != 0)
        {
            create_effect(&trgtng->mappos, shotst->hit_lava.effect_model, trgtng->owner);
            shotng->health = -1;
            return 1;
        }
    }
    if (shotng->shot.damage != 0)
    {
        HitPoints damage_done;
        if (thing_exists(shooter)) {
            damage_done = apply_damage_to_thing_and_display_health(trgtng, shotng->shot.damage, shooter->owner);
        } else {
            damage_done = apply_damage_to_thing_and_display_health(trgtng, shotng->shot.damage, -1);
        }
        if (shotst->model_flags & ShMF_LifeDrain)
        {
            give_shooter_drained_health(shooter, damage_done / 2);
        }
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(trgtng);
    if (shotst->target_hitstop_turns != 0)
    {
        if (cctrl->frozen_on_hit == 0) {
            cctrl->frozen_on_hit = shotst->target_hitstop_turns;
        }
    }
    if (shotst->cast_spell_kind != 0)
    {
        struct CreatureControl* scctrl = creature_control_get_from_thing(shooter);
        CrtrExpLevel spell_level = 0;
        if (!creature_control_invalid(scctrl))
        {
            spell_level = scctrl->exp_level;
        }
        apply_spell_effect_to_thing(trgtng, shotst->cast_spell_kind, spell_level, shotng->owner);
        struct SpellConfig *spconf = get_spell_config(shotst->cast_spell_kind);
        if (flag_is_set(spconf->spell_flags, CSAfF_Disease))
        {
            cctrl->disease_caster_plyridx = shotng->owner;
        }
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

    adjusted_push_strength = push_strength;
    if (game.conf.rules[trgtng->owner].magic.weight_calculate_push > 0)
    {
        int weight = compute_creature_weight(trgtng);
        adjusted_push_strength = weight_calculated_push_strenght(weight, push_strength);
    }

    if (push_strength != 0 )
    {
        i = adjusted_push_strength * shotng->velocity.x.val;
        trgtng->veloc_push_add.x.val += i / 16;
        i = adjusted_push_strength * shotng->velocity.y.val;
        trgtng->veloc_push_add.y.val += i / 16;
        trgtng->state_flags |= TF1_PushAdd;
    }

    if (creature_is_being_unconscious(trgtng))
    {
        if (push_strength == 0)
            push_strength++;
        if (game.conf.rules[trgtng->owner].game.classic_bugs_flags & ClscBug_FaintedImmuneToBoulder)
        {
        push_strength *= 5;
        int move_x = push_strength * shotng->velocity.x.val / 16.0;
        int move_y = push_strength * shotng->velocity.y.val / 16.0;

            trgtng->veloc_push_add.x.val += move_x;
            trgtng->veloc_push_add.y.val += move_y;
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
                    i = push_strength * shotng->velocity.x.val;
                    trgtng->veloc_push_add.x.val += i / 64;
                    i = push_strength * shotng->velocity.x.val * (THING_RANDOM(shotng, 3) - 1);
                    trgtng->veloc_push_add.y.val += i / 64;
                }
                else
                {
                    i = push_strength * shotng->velocity.y.val;
                    trgtng->veloc_push_add.y.val += i / 64;
                    i = push_strength * shotng->velocity.y.val * (THING_RANDOM(shotng, 3) - 1);
                    trgtng->veloc_push_add.x.val += i / 64;
                }
                trgtng->state_flags |= TF1_PushAdd;
            }
            else // Normal shots blast unconscious units out of the way
            {
                push_strength *= 5;
                i = push_strength * shotng->velocity.x.val;
                trgtng->veloc_push_add.x.val += i / 16;
                i = push_strength * shotng->velocity.y.val;
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
        if (creature_is_being_unconscious(trgtng)  && !(game.conf.rules[trgtng->owner].game.classic_bugs_flags & ClscBug_FaintedImmuneToBoulder)) //We're not actually hitting the unconscious units with a boulder
        {
            return 0;
        }
        else
        {
            struct CreatureModelConfig* crconf = creature_stats_get_from_thing(trgtng);
            shotng->health -= crconf->damage_to_boulder;
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
        detonate_shot(shotng, shotst->destroy_on_first_hit);
    }


    if (shotst->destroy_on_first_hit != 0) {
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
    if (target->class_id == TCls_Trap) {
        return shot_hit_trap_at(shotng, target, pos);
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

HitTargetFlags collide_filter_thing_is_shootable(const struct Thing *thing, const struct Thing *parntng, HitTargetFlags hit_targets, long a4)
{
    PlayerNumber shot_owner = -1;
    if (thing_exists(parntng))
        shot_owner = parntng->owner;
    return thing_is_shootable(thing, shot_owner, hit_targets);
}

struct Thing *get_thing_collided_with_at_satisfying_filter_for_subtile(struct Thing *shotng, struct Coord3d *pos, Thing_Collide_Func filter, HitTargetFlags param1, long param2, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Thing* parntng = get_parent_thing(shotng);
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

struct Thing *get_thing_collided_with_at_satisfying_filter(struct Thing *shotng, struct Coord3d *pos, Thing_Collide_Func filter, HitTargetFlags hit_targets, long a5)
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
        if (stl_x_max > game.map_subtiles_x)
            stl_x_max = game.map_subtiles_x;
        stl_y_max = coord_subtile(pos->y.val + radius);
        if (stl_y_max > game.map_subtiles_y)
            stl_y_max = game.map_subtiles_y;
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
        if (shot_hit_something_while_moving(shotng, &pos))
        {
            if ( (!flag_is_set(shotst->model_flags,ShMF_Penetrating)) || (!thing_exists(shotng)) ) // Shot may have been destroyed when it hit something
            {
                return TUFRet_Deleted;
            }
        }
    }
    if (flag_is_set(shotng->movement_flags,TMvF_GoThroughWalls))
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
            if ((thing_exists(target)) && (target->class_id == TCls_Creature) && !thing_is_picked_up(target) && !creature_is_being_unconscious(target))
            {
                pos2.x.val = target->mappos.x.val;
                pos2.y.val = target->mappos.y.val;
                pos2.z.val = target->mappos.z.val;
                pos2.z.val += (target->clipbox_size_z >> 1);
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
        if (shotst->model_flags & ShMF_Boulder)
        {
            if (apply_wallhug_force_to_boulder(thing))
                hit = true;
        }
        if (shotst->visual.effect_model != 0)
        {
            if ((shotst->visual.shot_health == 0) ||
                ((shotst->visual.shot_health > 0) && ((shotst->health - thing->health) <= shotst->visual.shot_health)) ||
                ((shotst->visual.shot_health < 0) && ((thing->health - shotst->health) <= shotst->visual.shot_health)))
            {
                for (i = shotst->visual.amount; i > 0; i--)
                {
                    pos1.x.val = thing->mappos.x.val - THING_RANDOM(thing, shotst->visual.random_range) + (shotst->visual.random_range / 2);
                    pos1.y.val = thing->mappos.y.val - THING_RANDOM(thing, shotst->visual.random_range) + (shotst->visual.random_range / 2);
                    pos1.z.val = thing->mappos.z.val - THING_RANDOM(thing, shotst->visual.random_range) + (shotst->visual.random_range / 2);
                    if (shotst->visual.effect_model != 0)
                    {
                        create_used_effect_or_element(&pos1, shotst->visual.effect_model, thing->owner, thing->index);
                    }
                }
            }
        }
        if (shotst->periodical > 0) {
            unsigned short frequency = shotst->periodical;
            if (((game.play_gameturn + thing->index) % frequency) == 0) {
                detonate_shot(thing, false);
            }
        }
        switch (shotst->update_logic)
        {
            case ShUL_Lightning:
            {
                struct PlayerInfo* player;
                if (lightning_is_close_to_player(myplyr, &thing->mappos))
                {
                  if (is_my_player_number(thing->owner))
                  {
                      player = get_player(thing->owner);
                      if ((thing->parent_idx > 0) && (myplyr->controlled_thing_idx == thing->parent_idx))
                      {
                          PaletteSetPlayerPalette(player, lightning_palette);
                          myplyr->additional_flags |= PlaAF_LightningPaletteIsActive;
                      }
                  }
                }
                break;
            }
            case ShUL_Wind:
                affect_nearby_enemy_creatures_with_wind(thing);
                break;
            case ShUL_Grenade:
                thing->move_angle_xy = (thing->move_angle_xy + DEGREES_20) & ANGLE_MASK;
                break;
            case ShUL_GodLightning:
                draw_god_lightning(thing);
                lightning_modify_palette(thing);
                break;
            /**case ShUL_Vortex:
                //Not implemented, due to limited amount of shots, replaced by Lizard
                affect_nearby_stuff_with_vortex(thing);
                break;
                **/
            case ShUL_Lizard:
                thing->move_angle_xy = (thing->move_angle_xy + DEGREES_20) & ANGLE_MASK;
                int skill = thing->shot_lizard2.range;
                target = thing_get(thing->shot_lizard.target_idx);
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
                    target_pos.x.val = thing->shot_lizard.x;
                    target_pos.y.val = thing->shot_lizard.posint * game.conf.crtr_conf.sprite_size;
                    target_pos.z.val = target->mappos.z.val;
                    dist = get_2d_distance(&thing->mappos, &target_pos);
                    if (dist <= 260) hit = true;
                }
                break;
            case ShUL_GodLightBall:
                update_god_lightning_ball(thing);
                break;
            case ShUL_TrapTNT:
                thing->mappos.z.val = 0;
                break;
            case ShUL_TrapLightning:
                if (((game.play_gameturn - thing->creation_turn) % 16) == 0)
                {
                  god_lightning_choose_next_creature(thing);
                  target = thing_get(thing->shot.target_idx);
                  if (thing_exists(target))
                  {
                      shotst = get_shot_model_stats(ShM_GodLightBall);
                      draw_lightning(&thing->mappos,&target->mappos, shotst->effect_spacing, shotst->effect_id);
                      apply_damage_to_thing_and_display_health(target, shotst->damage, thing->owner);
                  }
                }
                break;
        default:
            if (shotst->update_logic < 0)
                luafunc_thing_update_func(shotst->update_logic, thing);

            // All shots that do not require special processing
            break;
        }
    }
    if (!thing_exists(thing)) {
        WARNLOG("Thing disappeared during update");
        return TUFRet_Deleted;
    }
    if (hit) {
        detonate_shot(thing, true);
        return TUFRet_Deleted;
    }
    return move_shot(thing);
}

struct Thing *create_shot(struct Coord3d *pos, ThingModel model, unsigned short owner)
{
    if ( !i_can_allocate_free_thing_structure(TCls_Shot) )
    {
        ERRORDBG(3,"Cannot create shot %d (%s) for player %d. There are too many things allocated.",(int)model,shot_code_name(model),(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    struct ShotConfigStats* shotst = get_shot_model_stats(model);
    struct Thing* thing = allocate_free_thing_structure(TCls_Shot);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate shot %d (%s) for player %d, but failed.",(int)model,shot_code_name(model),(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->creation_turn = game.play_gameturn;
    thing->class_id = TCls_Shot;
    thing->model = model;
    memcpy(&thing->mappos,pos,sizeof(struct Coord3d));
    // save the origin of the shot
    memcpy(&thing->shot.originpos,pos,sizeof(struct Coord3d));
    thing->parent_idx = thing->index;
    thing->owner = owner;
    thing->bounce_angle = shotst->bounce_angle;
    thing->fall_acceleration = shotst->fall_acceleration;
    thing->inertia_floor = shotst->inertia_floor;
    thing->inertia_air = shotst->inertia_air;
    thing->movement_flags ^= (thing->movement_flags ^ TMvF_ZeroVerticalVelocity * shotst->soft_landing) & TMvF_ZeroVerticalVelocity;
    set_thing_draw(thing, shotst->sprite_anim_idx, 256, shotst->sprite_size_max, 0, 0, ODC_Default);
    thing->rendering_flags ^= (thing->rendering_flags ^ TRF_Unshaded * shotst->unshaded) & TRF_Unshaded;
    thing->rendering_flags ^= thing->rendering_flags ^ ((thing->rendering_flags ^ TRF_Transpar_8 * shotst->animation_transparency) & (TRF_Transpar_Flags));
    thing->rendering_flags ^= (thing->rendering_flags ^ shotst->hidden_projectile) & TRF_Invisible;
    thing->clipbox_size_xy = shotst->size_xy;
    thing->clipbox_size_z = shotst->size_z;
    thing->solid_size_xy = shotst->size_xy;
    thing->solid_size_z = shotst->size_z;
    thing->shot.damage = shotst->damage;
    thing->shot.dexterity = 255;
    thing->health = shotst->health;
    if (shotst->light_radius)
    {
        struct InitLight ilght;
        memset(&ilght, 0, sizeof(struct InitLight));
        memcpy(&ilght.mappos,&thing->mappos,sizeof(struct Coord3d));
        ilght.radius = shotst->light_radius;
        ilght.intensity = shotst->light_intensity;
        ilght.is_dynamic = 1;
        ilght.flags = shotst->light_flags;
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

static TngUpdateRet affect_thing_by_wind(struct Thing *thing, ModTngFilterParam param)
{
    SYNCDBG(18,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    if (thing->index == param->secondary_number) {
        return TUFRet_Unchanged;
    }
    struct Thing *shotng = (struct Thing *)param->tertiary_pointer;
    struct ShotConfigStats *shotst = get_shot_model_stats(shotng->model);
    if ((thing->index == shotng->index) || (thing->index == shotng->parent_idx)) {
        return TUFRet_Unchanged;
    }
    // param->primary_number = 2048 from affect_nearby_enemy_creatures_with_wind
    long blow_distance = param->primary_number;
    // calculate max distance
    int maxdistance = shotst->health * shotst->speed;
    MapCoordDelta creature_distance = INT32_MAX;
    TbBool apply_velocity = false;
    switch (thing->class_id)
    {
        case TCls_Creature:
        {
            if (!thing_is_picked_up(thing) && !creature_is_being_unconscious(thing))
            {
                struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
                TbBool creatureAlreadyAffected = false;

                // distance between creature and actual position of the projectile
                creature_distance = get_chessboard_distance(&shotng->mappos, &thing->mappos) + 1;

                // if weight-affect-push-rule is on
                if (game.conf.rules[thing->owner].magic.weight_calculate_push > 0)
                {
                    long weight = compute_creature_weight(thing);
                    //max push distance
                    blow_distance = maxdistance - (maxdistance - weight_calculated_push_strenght(weight, maxdistance));
                    // distance between startposition and actual position of the projectile
                    int origin_distance = get_chessboard_distance(&shotng->shot.originpos, &thing->mappos) + 1;
                    creature_distance = origin_distance;

                    // Check the the spell instance for already affected creatures
                    for (int i = 0; i < shotng->shot.num_wind_affected; i++)
                    {
                        if (shotng->shot.wind_affected_creature[i] == cctrl->index)
                        {
                            creatureAlreadyAffected = true;
                            set_flag(cctrl->spell_flags, CSAfF_Wind);
                            break;
                        }
                    }
                }
                if ((creature_distance < blow_distance) && !creature_is_immune_to_spell_effect(thing, CSAfF_Wind) && !creatureAlreadyAffected)
                {
                    set_start_state(thing);
                    cctrl->idle.start_gameturn = game.play_gameturn;
                    apply_velocity = true;
                    set_flag(cctrl->spell_flags, CSAfF_Wind);
                } // If weight_affect_push_rule is on.
                else if (game.conf.rules[thing->owner].magic.weight_calculate_push > 0 && creature_distance >= blow_distance && !creatureAlreadyAffected)
                {
                    // Add creature index to wind_affected_creature array.
                    shotng->shot.wind_affected_creature[shotng->shot.num_wind_affected++] = cctrl->index;
                }
            }
            break;
        }
        case TCls_EffectElem:
        {
            if (!thing_is_picked_up(thing))
            {
                struct EffectElementConfigStats *eestat = get_effect_element_model_stats(thing->model);
                creature_distance = get_chessboard_distance(&shotng->mappos, &thing->mappos) + 1;
                if ((creature_distance < blow_distance) && eestat->affected_by_wind)
                {
                    apply_velocity = true;
                }
            }
            break;
        }
        case TCls_Shot:
        {
            if (!thing_is_picked_up(thing))
            {
                struct ShotConfigStats *thingshotst = get_shot_model_stats(shotng->model);
                creature_distance = get_chessboard_distance(&shotng->mappos, &thing->mappos) + 1;
                if ((creature_distance < blow_distance) && !thingshotst->wind_immune)
                {
                    apply_velocity = true;
                }
            }
            break;
        }
        case TCls_Effect:
        {
            if (!thing_is_picked_up(thing))
            {
                struct EffectConfigStats *effcst = get_effect_model_stats(thing->model);
                creature_distance = get_chessboard_distance(&shotng->mappos, &thing->mappos) + 1;
                if ((creature_distance < blow_distance) && effcst->affected_by_wind)
                {
                    apply_velocity = true;
                }
            }
            break;
        }
    }
    if (apply_velocity)
    {
        struct ComponentVector wind_push;
        wind_push.x = (shotng->veloc_base.x.val * blow_distance) / creature_distance;
        wind_push.y = (shotng->veloc_base.y.val * blow_distance) / creature_distance;
        wind_push.z = (shotng->veloc_base.z.val * blow_distance) / creature_distance;
        SYNCDBG(8,"Applying (%d,%d,%d) to %s index %d",(int)wind_push.x,(int)wind_push.y,(int)wind_push.z,thing_model_name(thing),(int)thing->index);
        apply_transitive_velocity_to_thing(thing, &wind_push);
        return TUFRet_Modified;
    }
    return TUFRet_Unchanged;
}

void affect_nearby_enemy_creatures_with_wind(struct Thing *shotng)
{
    Thing_Modifier_Func do_cb;
    struct CompoundTngFilterParam param;
    param.plyr_idx = -1;
    param.class_id = 0;
    param.model_id = 0;
    param.primary_number = 2048;
    param.secondary_number = shotng->parent_idx;
    param.tertiary_pointer = shotng;
    do_cb = affect_thing_by_wind;
    do_to_things_with_param_spiral_near_map_block(&shotng->mappos, param.primary_number-COORD_PER_STL, do_cb, &param);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
