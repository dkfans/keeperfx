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
#include "bflib_math.h"
#include "bflib_sound.h"
#include "thing_data.h"
#include "thing_effects.h"
#include "thing_physics.h"
#include "thing_navigate.h"
#include "front_simple.h"
#include "thing_stats.h"
#include "config_creature.h"
#include "gui_topmsg.h"
#include "creature_states.h"
#include "creature_groups.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_move_shot(struct Thing *thing);
DLLIMPORT long _DK_update_shot(struct Thing *thing);

DLLIMPORT struct Thing *_DK_get_thing_collided_with_at_satisfying_filter(struct Thing *thing, struct Coord3d *pos, Thing_Collide_Func filter, long a4, long a5);
DLLIMPORT struct Thing *_DK_get_shot_collided_with_same_type(struct Thing *thing, struct Coord3d *nxpos);
DLLIMPORT long _DK_shot_hit_wall_at(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_shot_hit_door_at(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_shot_hit_shootable_thing_at(struct Thing *shotng, struct Thing *target, struct Coord3d *pos);
DLLIMPORT long _DK_shot_hit_object_at(struct Thing *shotng, struct Thing *target, struct Coord3d *pos);
DLLIMPORT void _DK_create_relevant_effect_for_shot_hitting_thing(struct Thing *shotng, struct Thing *target);
DLLIMPORT long _DK_check_hit_when_attacking_door(struct Thing *thing);
DLLIMPORT void _DK_process_dig_shot_hit_wall(struct Thing *thing, long a2);
/******************************************************************************/
TbBool shot_is_slappable(const struct Thing *thing, long plyr_idx)
{
    struct ShotConfigStats *shotst;
    if (thing->owner == plyr_idx)
    {
        // Normally, thing models 15 and 20 are slappable. But this is up to config file.
        shotst = get_shot_model_stats(thing->model);
        return ((shotst->model_flags & ShMF_Slappable) != 0);
    }
    return false;
}

TbBool shot_model_is_navigable(long tngmodel)
{
    // Normally, only shot model 6 is navigable
    struct ShotConfigStats *shotst;
    shotst = get_shot_model_stats(tngmodel);
    return ((shotst->model_flags & ShMF_Navigable) != 0);
}

TbBool shot_is_boulder(const struct Thing *thing)
{
    return (thing->model == 15);
}

TbBool detonate_shot(struct Thing *thing)
{
    struct PlayerInfo *myplyr;
    struct Thing *shooter;
    SYNCDBG(18,"Starting for model %d",(int)thing->model);
    shooter = INVALID_THING;
    myplyr = get_my_player();
    if (thing->index != thing->parent_idx)
        shooter = thing_get(thing->parent_idx);
    switch ( thing->model )
    {
    case 4:
    case 16:
    case 24:
        PaletteSetPlayerPalette(myplyr, _DK_palette);
        break;
    case 11:
        create_effect(&thing->mappos, TngEff_Unknown50, thing->owner);
        create_effect(&thing->mappos,  TngEff_Unknown09, thing->owner);
        explosion_affecting_area(shooter, &thing->mappos, 8, 256, 256, thing->byte_16);
        break;
    case 15:
        create_effect_around_thing(thing, TngEff_Unknown26);
        break;
    default:
        break;
    }
    delete_thing_structure(thing, 0);
    return true;
}

struct Thing *get_shot_collided_with_same_type(struct Thing *thing, struct Coord3d *nxpos)
{
    return _DK_get_shot_collided_with_same_type(thing, nxpos);
}

void process_dig_shot_hit_wall(struct Thing *thing, long a2)
{
    _DK_process_dig_shot_hit_wall(thing, a2); return;
}

struct Thing *create_shot_hit_effect(struct Coord3d *effpos, long effowner, long eff_kind, long snd_idx, long snd_range)
{
    struct Thing *efftng;
    long i;
    efftng = INVALID_THING;
    if (eff_kind > 0) {
        efftng = create_effect(effpos, eff_kind, effowner);
        TRACE_THING(efftng);
    }
    if (snd_idx > 0)
    {
        if (!thing_is_invalid(efftng))
        {
            i = snd_idx;
            if (snd_range > 1)
                i += UNSYNC_RANDOM(snd_range);
            thing_play_sample(efftng, i, 100, 0, 3, 0, 2, 256);
        }
    }
    return efftng;
}

/**
 * Processes hitting a wall by given shot.
 *
 * @param thing The thing to be moved into wall.
 * @param pos Next position of the thing.
 * @return Gives true if the shot hit a wall wand was destroyed.
 *     If the shot wasn't detonated, then the function returns false.
 * @note This function may delete the thing given in parameter.
 */
TbBool shot_hit_wall_at(struct Thing *thing, struct Coord3d *pos)
{
    struct ShotConfigStats *shotst;
    struct Thing *shooter;
    struct Thing *doortng;
    struct Thing *efftng;
    unsigned long blocked_flags;
    long cube_id;
    TbBool shot_explodes;
    long i;
    //return _DK_shot_hit_wall_at(thing, pos);

    efftng = INVALID_THING;
    shooter = INVALID_THING;
    shot_explodes = 0;
    shotst = get_shot_model_stats(thing->model);
    if (thing->index != thing->parent_idx) {
        shooter = thing_get(thing->parent_idx);
    }
    blocked_flags = get_thing_blocked_flags_at(thing, pos);
    if ( shotst->old->field_49 ) {
        process_dig_shot_hit_wall(thing, blocked_flags);
    }

    if ((blocked_flags & 0x04) != 0)
    {
        cube_id = get_top_cube_at(pos->x.stl.num, pos->y.stl.num);
        doortng = get_door_for_position(pos->x.stl.num, pos->y.stl.num);
        if (!thing_is_invalid(doortng))
        {
            efftng = create_shot_hit_effect(&thing->mappos, thing->owner, shotst->old->field_31, shotst->old->field_33, shotst->old->field_35);
            if ( shotst->old->field_36 )
              shot_explodes = 1;
            if ( !game.objects_config[door_to_object[doortng->model]].field_7 || shotst->old->field_4C )
            {
              apply_damage_to_thing(doortng, thing->word_14, -1);
            }
            else
            {
              i = 32 * thing->word_14 / 256;
              if (i <= 1)
                  i = 1;
              apply_damage_to_thing(doortng, i, -1);
            }
        } else
        if (cube_is_water(cube_id))
        {
            efftng = create_shot_hit_effect(&thing->mappos, thing->owner, shotst->old->field_37, shotst->old->field_39, 1);
            if ( shotst->old->field_3B ) {
                shot_explodes = 1;
            }
        } else
        if (cube_is_lava(cube_id))
        {
            efftng = create_shot_hit_effect(&thing->mappos, thing->owner, shotst->old->field_3C, shotst->old->field_3E, 1);
            if ( shotst->old->field_40 ) {
                shot_explodes = 1;
            }
        } else
        {
            efftng = create_shot_hit_effect(&thing->mappos, thing->owner, shotst->old->field_2B, shotst->old->field_2D, shotst->old->field_2F);
            if ( shotst->old->field_30 ) {
                shot_explodes = 1;
            }
        }
    }

    if ( !shot_explodes )
    {
        if ((blocked_flags & 0x03) != 0)
        {
            doortng = get_door_for_position(pos->x.stl.num, pos->y.stl.num);
            if (!thing_is_invalid(doortng))
            {
                efftng = create_shot_hit_effect(&thing->mappos, thing->owner, shotst->old->field_31, shotst->old->field_33, shotst->old->field_35);
                if (shotst->old->field_36)
                    shot_explodes = 1;
                if ( !game.objects_config[door_to_object[doortng->model]].field_7 || shotst->old->field_4C )
                {
                    apply_damage_to_thing(doortng, thing->word_14, -1);
                } else
                {
                    i = 32 * thing->word_14 / 256;
                    if (i <= 1)
                      i = 1;
                    apply_damage_to_thing(doortng, i, -1);
                }
            } else
            {
                efftng = create_shot_hit_effect(&thing->mappos, thing->owner, shotst->old->field_2B, shotst->old->field_2D, shotst->old->field_2F);
                if ( shotst->old->field_30 )
                  shot_explodes = 1;
            }

        }
    }
    if (!thing_is_invalid(efftng)) {
        efftng->byte_16 = shotst->old->field_4A;
    }
    if ( shot_explodes )
    {
        if ( shotst->old->field_41 ) {
            explosion_affecting_area(shooter, pos, shotst->old->field_41, shotst->old->field_43, 256, shotst->old->field_4A);
        }
        delete_thing_structure(thing, 0);
        return true;
    }
    if (shotst->old->field_D <= 0)
    {
        slide_thing_against_wall_at(thing, pos, blocked_flags);
    } else
    {
        bounce_thing_off_wall_at(thing, pos, blocked_flags);
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
long shot_hit_door_at(struct Thing *thing, struct Coord3d *pos)
{
    return _DK_shot_hit_door_at(thing, pos);
}

TbBool apply_shot_experience(struct Thing *shooter, struct Thing *target, long shot_model)
{
    struct CreatureStats *tgcrstat;
    struct ShotConfigStats *shotst;
    struct CreatureControl *tgcctrl;
    struct CreatureControl *shcctrl;
    long exp_mag,exp_factor,exp_gained;
    if (!creature_can_gain_experience(shooter))
        return false;
    shcctrl = creature_control_get_from_thing(shooter);
    tgcctrl = creature_control_get_from_thing(target);
    tgcrstat = creature_stats_get_from_thing(target);
    exp_factor = tgcrstat->exp_for_hitting;
    shotst = get_shot_model_stats(shot_model);
    exp_mag = shotst->old->experience_given_to_shooter;
    exp_gained = (exp_mag * (exp_factor + 12 * exp_factor * (long)tgcctrl->explevel / 100) << 8) / 256;
    shcctrl->prev_exp_points = shcctrl->exp_points;
    shcctrl->exp_points += exp_gained;
    if ( check_experience_upgrade(shooter) ) {
        external_set_thing_state(shooter, CrSt_CreatureBeHappy);
    }
    return true;
}

long shot_hit_object_at(struct Thing *shotng, struct Thing *target, struct Coord3d *pos)
{
    return _DK_shot_hit_object_at(shotng, target, pos);
}

long get_damage_of_melee_shot(struct Thing *shotng, struct Thing *target)
{
    struct CreatureStats *tgcrstat;
    struct CreatureControl *tgcctrl;
    long crdefense,hitchance;
    tgcrstat = creature_stats_get_from_thing(target);
    tgcctrl = creature_control_get_from_thing(target);
    crdefense = compute_creature_max_defense(tgcrstat->defense,tgcctrl->explevel);
    hitchance = ((long)shotng->byte_13 - crdefense) / 2;
    if (hitchance < -96) {
        hitchance = -96;
    } else
    if (hitchance > 96) {
        hitchance = 96;
    }
    if (ACTION_RANDOM(256) < (128+hitchance))
      return shotng->word_14;
    return 0;
}

void create_relevant_effect_for_shot_hitting_thing(struct Thing *shotng, struct Thing *target)
{
    struct CreatureControl *cctrl;
    struct Thing *efftng;
    //_DK_create_relevant_effect_for_shot_hitting_thing(shotng, target); return;
    efftng = INVALID_THING;
    if (target->class_id == TCls_Creature)
    {
        switch (shotng->model)
        {
        case 1:
        case 2:
        case 4:
            efftng = create_effect(&shotng->mappos, TngEff_Unknown01, shotng->owner);
            break;
        case 5:
            efftng = create_effect(&shotng->mappos, TngEff_Unknown13, shotng->owner);
            if ( !thing_is_invalid(efftng) ) {
                efftng->byte_16 = 2;
            }
            break;
        case 6:
        case 9:
            efftng = create_effect(&shotng->mappos, TngEff_Unknown08, shotng->owner);
            break;
        case 14:
        case 21:
        case 22:
            cctrl = creature_control_get_from_thing(target);
            if ((cctrl->affected_by_spells & 0x02) != 0) {
                efftng = create_effect(&shotng->mappos, TngEff_Unknown22, shotng->owner);
            } else
            if (creature_model_bleeds(target->model)) {
                efftng = create_effect(&shotng->mappos, TngEff_Unknown06, shotng->owner);
            }
            break;
        }
    }
    TRACE_THING(efftng);
}

long check_hit_when_attacking_door(struct Thing *thing)
{
    return _DK_check_hit_when_attacking_door(thing);
}

TbBool shot_kill_creature(struct Thing *shotng, struct Thing *target)
{
    struct CreatureControl *cctrl;
    struct ShotConfigStats *shotst;
    cctrl = creature_control_get_from_thing(target);
    target->health = -1;
    cctrl->shot_model = shotng->model;
    if (shotng->parent_idx == shotng->index) {
        kill_creature(target, INVALID_THING, shotng->owner, 0, 1, 0);
    } else {
        shotst = get_shot_model_stats(shotng->model);
        kill_creature(target, thing_get(shotng->parent_idx), shotng->owner, 0, 1, shotst->old->cannot_make_target_unconscious);
    }
    return true;
}

long melee_shot_hit_creature_at(struct Thing *shotng, struct Thing *target, struct Coord3d *pos)
{
    struct Thing *shooter;
    struct ShotConfigStats *shotst;
    struct CreatureStats *tgcrstat;
    struct CreatureControl *tgcctrl;
    long damage,throw_strength;
    shotst = get_shot_model_stats(shotng->model);
    throw_strength = shotng->field_20;
    if (target->health < 0)
        return 0;
    shooter = INVALID_THING;
    if (shotng->parent_idx != shotng->index)
        shooter = thing_get(shotng->parent_idx);
    tgcrstat = creature_stats_get_from_thing(target);
    tgcctrl = creature_control_get_from_thing(target);
    damage = get_damage_of_melee_shot(shotng, target);
    if (damage != 0)
    {
      if (shotst->old->field_22 > 0)
      {
          thing_play_sample(target, shotst->old->field_22, 100, 0, 3, 0, 2, 256);
          play_creature_sound(target, 1, 3, 0);
      }
      if (!thing_is_invalid(shooter)) {
          apply_damage_to_thing_and_display_health(target, shotng->word_14, shooter->owner);
      } else {
          apply_damage_to_thing_and_display_health(target, shotng->word_14, -1);
      }
      if (shotst->old->field_24 != 0) {
          tgcctrl->field_B1 = shotst->old->field_24;
      }
      if ( shotst->old->field_2A )
      {
          target->acceleration.x.val += (throw_strength * (long)shotng->velocity.x.val) / 16;
          target->acceleration.y.val += (throw_strength * (long)shotng->velocity.y.val) / 16;
          target->field_1 |= 0x04;
      }
      create_relevant_effect_for_shot_hitting_thing(shotng, target);
      if (target->health >= 0)
      {
          if (target->owner != shotng->owner) {
              check_hit_when_attacking_door(target);
          }
      } else
      {
          shot_kill_creature(shotng,target);
      }
    }
    if (shotst->old->destroy_on_first_hit) {
        delete_thing_structure(shotng, 0);
    }
    return 1;
}

void clear_thing_acceleration(struct Thing *thing)
{
    thing->acceleration.x.val = 0;
    thing->acceleration.y.val = 0;
    thing->acceleration.z.val = 0;
}

void set_thing_acceleration_angles(struct Thing *thing, long angle_xy, long angle_yz)
{
    struct ComponentVector cvect;
    thing->field_52 = angle_xy;
    thing->field_54 = angle_yz;
    angles_to_vector(thing->field_52, thing->field_54, 256, &cvect);
    thing->pos_2C.x.val = cvect.x;
    thing->pos_2C.y.val = cvect.y;
    thing->pos_2C.z.val = cvect.z;
}

TbBool shot_model_makes_flesh_explosion(long shot_model)
{
    if ((shot_model == ShM_Firebomb) || (shot_model == ShM_GodLightBall))
        return true;
    return false;
}

void shot_kills_creature(struct Thing *shotng, struct Thing *target)
{
    struct ShotConfigStats *shotst;
    struct CreatureControl *tgcctrl;
    struct Thing *killertng;
    shotst = get_shot_model_stats(shotng->model);
    target->health = -1;
    tgcctrl = creature_control_get_from_thing(target);
    tgcctrl->shot_model = shotng->model;
    if (shotng->index == shotng->parent_idx) {
        kill_creature(target, INVALID_THING, shotng->owner, 0, 1, 0);
    } else {
        killertng = thing_get(shotng->parent_idx);
        kill_creature(target, killertng, shotng->owner, 0, 1, shotst->old->cannot_make_target_unconscious);
    }
}

long shot_hit_creature_at(struct Thing *shotng, struct Thing *target, struct Coord3d *pos)
{
    struct Thing *shooter;
    struct Thing *efftng;
    struct ShotConfigStats *shotst;
    struct CreatureControl *tgcctrl;
    struct Coord3d pos2;
    long i,n,amp;
    //return _DK_shot_hit_shootable_thing_at(shotng, target, pos);
    amp = shotng->field_20;
    shotst = get_shot_model_stats(shotng->model);
    shooter = INVALID_THING;
    if (shotng->parent_idx != shotng->index) {
        shooter = thing_get(shotng->parent_idx);
    }
    if (!thing_is_invalid(shooter))
    {
        if ( (shooter->class_id == TCls_Creature) && (target->class_id == TCls_Creature) )
        {
            apply_shot_experience(shooter, target, shotng->model);
        }
    }
    if (shotst->old->field_48 != 0)
    {
        return melee_shot_hit_creature_at(shotng, target, pos);
    }
    if ( (shotst->old->field_28 != 0) || (target->health < 0) )
        return 0;
    tgcctrl = creature_control_get_from_thing(target);
    if (!creature_control_invalid(tgcctrl))
    {
        if ( ((tgcctrl->spell_flags & CSAfF_Rebound) != 0) && (shotst->old->field_29 == 0) )
        {
            struct Thing *killertng;
            killertng = INVALID_THING;
            if (shotng->index != shotng->parent_idx) {
                killertng = thing_get(shotng->parent_idx);
            }
            if ( !thing_is_invalid(killertng) )
            {
                struct CreatureStats *crstat;
                crstat = creature_stats_get_from_thing(killertng);
                pos2.x.val = killertng->mappos.x.val;
                pos2.y.val = killertng->mappos.y.val;
                pos2.z.val = crstat->eye_height + killertng->mappos.z.val;
                clear_thing_acceleration(shotng);
                set_thing_acceleration_angles(shotng, get_angle_xy_to(&shotng->mappos, &pos2), get_angle_yz_to(&shotng->mappos, &pos2));
                shotng->parent_idx = target->parent_idx;
                shotng->owner = target->owner;
            } else
            {
                clear_thing_acceleration(shotng);
                i = (shotng->field_52 + 1024) & 0x7FF;
                n = (shotng->field_54 + 1024) & 0x7FF;
                set_thing_acceleration_angles(shotng, i, n);
                if (target->class_id == TCls_Creature)
                {
                    shotng->parent_idx = target->parent_idx;
                }
            }
            return 1;
        }
    }
    // Immunity to boulders
    if (shot_is_boulder(shotng))
    {
        if ((get_creature_model_flags(target) & MF_ImmuneToBoulder) != 0)
        {
            efftng = create_effect(&target->mappos, TngEff_Unknown14, target->owner);
            if ( !thing_is_invalid(efftng) ) {
                efftng->byte_16 = 8;
            }
            shotng->health = -1;
            return 1;
        }
    }
    if (shotst->old->field_22 != 0)
    {
        play_creature_sound(target, 1, 1, 0);
        thing_play_sample(target, shotst->old->field_22, 100, 0, 3, 0, 2, 256);
    }
    if (shotng->word_14 != 0)
    {
        if (shotst->old->health_drain) {
            give_shooter_drained_health(shooter, shotng->word_14 / 2);
        }
        if ( !thing_is_invalid(shooter) ) {
            apply_damage_to_thing_and_display_health(target, shotng->word_14, shooter->owner);
        } else {
            apply_damage_to_thing_and_display_health(target, shotng->word_14, -1);
        }
    }
    if (shotst->old->field_24 != 0)
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(target);
        if (cctrl->field_B1 == 0) {
            cctrl->field_B1 = shotst->old->field_24;
        }
    }
    if (shotst->old->cast_spell_kind != 0)
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(shooter);
        if (!creature_control_invalid(cctrl)) {
            n = cctrl->explevel;
        } else {
            n = 0;
        }
        apply_spell_effect_to_thing(target, shotst->old->cast_spell_kind, n);
    }
    if (shotst->old->field_4B != 0)
    {
        if ( !thing_is_invalid(shooter) )
        {
            if (get_no_creatures_in_group(shooter) < 8) {
                add_creature_to_group(target, shooter);
            }
        }
    }
    if (shotst->old->field_2A != 0)
    {
        i = amp * (long)shotng->velocity.x.val;
        target->acceleration.x.val += i / 16;
        i = amp * (long)shotng->velocity.y.val;
        target->acceleration.y.val += i / 16;
        target->field_1 |= 0x04;
    }
    create_relevant_effect_for_shot_hitting_thing(shotng, target);
    if (shotst->old->field_45 != 0)
    {
        struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(target);
        shotng->health -= crstat->damage_to_boulder;
    }
    if (target->health < 0)
    {
        shot_kills_creature(shotng, target);
    } else
    {
        if (target->owner != shotng->owner) {
            check_hit_when_attacking_door(target);
        }
    }
    if (shotst->old->destroy_on_first_hit != 0) {
        delete_thing_structure(shotng, 0);
    }
    return 1;
}

long shot_hit_shootable_thing_at(struct Thing *shotng, struct Thing *target, struct Coord3d *pos)
{
    //return _DK_shot_hit_shootable_thing_at(shotng, target, pos);
    if (!thing_exists(target))
        return 0;
    if (target->class_id == TCls_Object) {
        return shot_hit_object_at(shotng, target, pos);
    }
    if (target->class_id == TCls_Creature) {
        return shot_hit_creature_at(shotng, target, pos);
    }
    return 0;
}

long collide_filter_thing_is_shootable_by_any_player_including_objects(struct Thing *thing, struct Thing *coltng, long a3, long a4)
{
    return thing_is_shootable_by_any_player_including_objects(thing);
}

long collide_filter_thing_is_shootable_by_any_player_excluding_objects(struct Thing *thing, struct Thing *coltng, long a3, long a4)
{
    return thing_is_shootable_by_any_player_excluding_objects(thing);
}

long collide_filter_thing_is_shootable_by_any_player_except_own_including_objects(struct Thing *thing, struct Thing *coltng, long a3, long a4)
{
    return thing_is_shootable_by_any_player_except_own_including_objects(coltng, thing);
}

long collide_filter_thing_is_shootable_by_any_player_except_own_excluding_objects(struct Thing *thing, struct Thing *coltng, long a3, long a4)
{
    return thing_is_shootable_by_any_player_except_own_excluding_objects(coltng, thing);
}

long collide_filter_thing_is_shootable_dungeon_heart_only(struct Thing *thing, struct Thing *coltng, long a3, long a4)
{
    return (thing->class_id == TCls_Object) && (thing->model == 5) && (coltng->owner != thing->owner);
}

struct Thing *get_thing_collided_with_at_satisfying_filter(struct Thing *thing, struct Coord3d *pos, Thing_Collide_Func filter, long a4, long a5)
{
    return _DK_get_thing_collided_with_at_satisfying_filter(thing, pos, filter, a4, a5);
}

/**
 * Processes hitting another thing.
 *
 * @param thing The thing to be moved.
 * @param pos Next position of the thing.
 * @return Gives true if the shot hit something and was destroyed.
 *     If the shot wasn't detonated, then the function returns false.
 * @note This function may delete the thing given in parameter.
 */
TbBool shot_hit_something_while_moving(struct Thing *thing, struct Coord3d *nxpos)
{
    struct ShotConfigStats *shotst;
    struct Thing *target;
    target = INVALID_THING;
    shotst = get_shot_model_stats(thing->model);
    switch ( thing->byte_16 )
    {
    case 8:
        if ( shot_hit_shootable_thing_at(thing, target, nxpos) )
            return true;
        break;
    case 1:
        target = get_thing_collided_with_at_satisfying_filter(thing, nxpos, collide_filter_thing_is_shootable_by_any_player_including_objects, 0, 0);
        if ( thing_is_invalid(target) )
            break;
        if ( shot_hit_shootable_thing_at(thing, target, nxpos) )
            return true;
        break;
    case 2:
        target = get_thing_collided_with_at_satisfying_filter(thing, nxpos, collide_filter_thing_is_shootable_by_any_player_excluding_objects, 0, 0);
        if ( thing_is_invalid(target) )
            break;
        if ( shot_hit_shootable_thing_at(thing, target, nxpos) )
            return true;
        break;
    case 3:
        target = get_thing_collided_with_at_satisfying_filter(thing, nxpos, collide_filter_thing_is_shootable_by_any_player_except_own_including_objects, 0, 0);
        if ( thing_is_invalid(target) )
            break;
        if ( shot_hit_shootable_thing_at(thing, target, nxpos) )
            return true;
        break;
    case 4:
        target = get_thing_collided_with_at_satisfying_filter(thing, nxpos, collide_filter_thing_is_shootable_by_any_player_except_own_excluding_objects, 0, 0);
        if ( thing_is_invalid(target) )
            break;
        if ( shot_hit_shootable_thing_at(thing, target, nxpos) )
            return true;
        break;
    case 7:
        target = get_thing_collided_with_at_satisfying_filter(thing, nxpos, collide_filter_thing_is_shootable_dungeon_heart_only, 0, 0);
        if ( thing_is_invalid(target) )
            break;
        if ( shot_hit_shootable_thing_at(thing, target, nxpos) )
            return true;
        break;
    case 9:
        target = get_thing_collided_with_at_satisfying_filter(thing, nxpos, collide_filter_thing_is_shootable_by_any_player_including_objects, 0, 0);
        if ( !thing_is_invalid(target) )
        {
            if ( shot_hit_shootable_thing_at(thing, target, nxpos) )
              return true;
            break;
        }
        target = get_shot_collided_with_same_type(thing, nxpos);
        if ( !thing_is_invalid(target) )
        {
            thing->health = -1;
            return true;
        }
        if ( shot_hit_shootable_thing_at(thing, target, nxpos) )
            return true;
        break;
    default:
        ERRORLOG("Shot has no hit thing type");
        if ( shot_hit_shootable_thing_at(thing, target, nxpos) )
            return true;
        break;
    }
    return false;
}

TngUpdateRet move_shot(struct Thing *thing)
{
    //return _DK_move_shot(thing);
    struct ShotConfigStats *shotst;
    struct Coord3d pos;
    SYNCDBG(18,"Starting for thing index %d, model %d",(int)thing->index,(int)thing->model);

    get_thing_next_position(&pos, thing);
    shotst = get_shot_model_stats(thing->model);
    if ( !shotst->old->field_28 )
    {
        if ( shot_hit_something_while_moving(thing, &pos) ) {
            return TUFRet_Deleted;
        }
    }
    if ((thing->movement_flags & TMvF_Unknown10) != 0)
    {
      if ( (shotst->old->field_48) && thing_in_wall_at(thing, &pos) ) {
          if ( shot_hit_door_at(thing, &pos) ) {
              return TUFRet_Deleted;
          }
      }
    } else
    {
      if ( thing_in_wall_at(thing, &pos) ) {
          if ( shot_hit_wall_at(thing, &pos) ) {
              return TUFRet_Deleted;
          }
      }
    }
    move_thing_in_map(thing, &pos);
    return TUFRet_Modified;
}

TngUpdateRet update_shot(struct Thing *thing)
{
    struct ShotConfigStats *shotst;
    struct PlayerInfo *myplyr;
    struct PlayerInfo *player;
    struct Thing *target;
    struct Coord3d pos1;
    struct Coord3d pos2;
    struct CoordDelta3d dtpos;
    struct ComponentVector cvect;
    long i;
    TbBool hit;
    SYNCDBG(18,"Starting for index %d, model %d",(int)thing->index,(int)thing->model);
    TRACE_THING(thing);
    //return _DK_update_shot(thing);
    hit = false;
    shotst = get_shot_model_stats(thing->model);
    myplyr = get_my_player();
    if (shotst->old->shot_sound != 0)
    {
        if (!S3DEmitterIsPlayingSample(thing->snd_emitter_id, shotst->old->shot_sound, 0))
            thing_play_sample(thing, shotst->old->shot_sound, 100, 0, 3, 0, 2, 256);
    }
    if (shotst->old->field_47)
        thing->health--;
    if (thing->health < 0)
    {
        hit = true;
    } else
    {
        switch ( thing->model )
        {
        case ShM_Firebomb:
            for (i = 2; i > 0; i--)
            {
              pos1.x.val = thing->mappos.x.val - ACTION_RANDOM(127) + 63;
              pos1.y.val = thing->mappos.y.val - ACTION_RANDOM(127) + 63;
              pos1.z.val = thing->mappos.z.val - ACTION_RANDOM(127) + 63;
              create_thing(&pos1, TCls_EffectElem, 1, thing->owner, -1);
            }
            break;
        case ShM_Lightning:
            if ( lightning_is_close_to_player(myplyr, &thing->mappos) )
            {
              if (is_my_player_number(thing->owner))
              {
                  player = get_player(thing->owner);
                  if ((thing->parent_idx != 0) && (myplyr->controlled_thing_idx == thing->parent_idx))
                  {
                      PaletteSetPlayerPalette(player, lightning_palette);
                      myplyr->field_3 |= 0x08;
                  }
              }
            }
            break;
        case ShM_NaviMissile:
            target = thing_get(thing->word_17);
            if ((thing_exists(target)) && (target->class_id == TCls_Creature))
            {
                pos2.x.val = target->mappos.x.val;
                pos2.y.val = target->mappos.y.val;
                pos2.z.val = target->mappos.z.val;
                pos2.z.val += (target->field_58 >> 1);
                thing->field_52 = get_angle_xy_to(&thing->mappos, &pos2);
                thing->field_54 = get_angle_yz_to(&thing->mappos, &pos2);
                angles_to_vector(thing->field_52, thing->field_54, shotst->old->speed, &cvect);
                dtpos.x.val = cvect.x - thing->pos_2C.x.val;
                dtpos.y.val = cvect.y - thing->pos_2C.y.val;
                dtpos.z.val = cvect.z - thing->pos_2C.z.val;
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
                thing->acceleration.x.val += cvect.x;
                thing->acceleration.y.val += cvect.y;
                thing->acceleration.z.val += cvect.z;
                thing->field_1 |= 0x04;
            }
            break;
        case ShM_Wind:
            for (i = 10; i > 0; i--)
            {
              pos1.x.val = thing->mappos.x.val - ACTION_RANDOM(1023) + 511;
              pos1.y.val = thing->mappos.y.val - ACTION_RANDOM(1023) + 511;
              pos1.z.val = thing->mappos.z.val - ACTION_RANDOM(1023) + 511;
              create_thing(&pos1, TCls_EffectElem, 12, thing->owner, -1);
            }
            affect_nearby_enemy_creatures_with_wind(thing);
            break;
        case ShM_Grenade:
            thing->field_52 = (thing->field_52 + 113) & 0x7FF;
            break;
        case ShM_Boulder:
        case ShM_SolidBoulder:
            if ( apply_wallhug_force_to_boulder(thing) )
              hit = true;
            break;
        case ShM_GodLightning:
            draw_god_lightning(thing);
            lightning_modify_palette(thing);
            break;
        case ShM_Vortex:
            affect_nearby_stuff_with_vortex(thing);
            break;
        case ShM_Alarm:
            affect_nearby_friends_with_alarm(thing);
            break;
        case ShM_GodLightBall:
            update_god_lightning_ball(thing);
            break;
        case ShM_TrapLightning:
            if (((game.play_gameturn - thing->field_9) % 16) == 0)
            {
              thing->field_19 = 5;
              god_lightning_choose_next_creature(thing);
              target = thing_get(thing->word_17);
              if (thing_exists(target))
              {
                  shotst = get_shot_model_stats(24);
                  draw_lightning(&thing->mappos,&target->mappos, 96, 60);
                  apply_damage_to_thing_and_display_health(target, shotst->old->damage, thing->owner);
              }
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
    struct ShotConfigStats *shotst;
    struct InitLight ilght;
    struct Thing *thing;
    if ( !i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots) )
    {
        ERRORDBG(3,"Cannot create shot %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    shotst = get_shot_model_stats(model);
    thing = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate shot %d for player %d, but failed.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->field_9 = game.play_gameturn;
    thing->class_id = TCls_Shot;
    thing->model = model;
    memcpy(&thing->mappos,pos,sizeof(struct Coord3d));
    thing->parent_idx = thing->index;
    thing->owner = owner;
    thing->field_22 = shotst->old->field_D;
    thing->field_20 = shotst->old->field_F;
    thing->field_21 = shotst->old->field_10;
    thing->field_23 = shotst->old->field_11;
    thing->field_24 = shotst->old->field_12;
    thing->movement_flags ^= (thing->movement_flags ^ TMvF_Unknown08 * shotst->old->field_13) & TMvF_Unknown08;
    set_thing_draw(thing, shotst->old->numfield_0, 256, shotst->old->numfield_2, 0, 0, 2);
    thing->field_4F ^= (thing->field_4F ^ 0x02 * shotst->old->field_6) & 0x02;
    thing->field_4F ^= thing->field_4F ^ ((thing->field_4F ^ 0x10 * shotst->old->field_8) & 0x30);
    thing->field_4F ^= (thing->field_4F ^ shotst->old->field_7) & 0x01;
    thing->sizexy = shotst->old->field_9;
    thing->field_58 = shotst->old->field_B;
    thing->field_5A = shotst->old->field_9;
    thing->field_5C = shotst->old->field_B;
    thing->word_13 = shotst->old->damage;
    thing->health = shotst->old->health;
    if (shotst->old->field_50)
    {
        memset(&ilght, 0, sizeof(struct InitLight));
        memcpy(&ilght.mappos,&thing->mappos,sizeof(struct Coord3d));
        ilght.field_0 = shotst->old->field_50;
        ilght.field_2 = shotst->old->field_52;
        ilght.is_dynamic = 1;
        ilght.field_3 = shotst->old->field_53;
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
