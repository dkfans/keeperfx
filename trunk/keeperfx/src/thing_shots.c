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
#include "front_simple.h"
#include "thing_stats.h"
#include "config_creature.h"
#include "gui_topmsg.h"
#include "creature_states.h"

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

TbBool detonate_shot(struct Thing *thing)
{
    struct PlayerInfo *myplyr;
    struct Thing *shooter;
    SYNCDBG(18,"Starting for model %d",(int)thing->model);
    shooter = INVALID_THING;
    myplyr = get_my_player();
    if (thing->index != thing->field_1D)
        shooter = thing_get(thing->field_1D);
    switch ( thing->model )
    {
    case 4:
    case 16:
    case 24:
        PaletteSetPlayerPalette(myplyr, _DK_palette);
        break;
    case 11:
        create_effect(&thing->mappos, 50, thing->owner);
        create_effect(&thing->mappos,  9, thing->owner);
        explosion_affecting_area(shooter, &thing->mappos, 8, 256, thing->byte_16);
        break;
    case 15:
        create_effect_around_thing(thing, 26);
        break;
    default:
        break;
    }
    delete_thing_structure(thing, 0);
    return true;
}

/** Retrieves planned next position for given thing, without collision detection.
 *  Just adds thing velocity to current position. Nothing fancy.
 * @param pos
 * @param thing
 */
void get_thing_next_position(struct Coord3d *pos, const struct Thing *thing)
{
    long delta_x,delta_y,delta_z;
    pos->x.val = thing->mappos.x.val;
    pos->y.val = thing->mappos.y.val;
    pos->z.val = thing->mappos.z.val;
    delta_z = thing->velocity.z.val;
    delta_x = thing->velocity.x.val;
    delta_y = thing->velocity.y.val;
    if (delta_x > 256)
      delta_x = 256;
    if (delta_x < -256)
      delta_x = -256;
    if (delta_y > 256)
        delta_y = 256;
    if (delta_y < -256)
      delta_y = -256;
    if (delta_z > 256)
        delta_z = 256;
    if (delta_z < -256)
        delta_z = -256;
    pos->x.val += delta_x;
    pos->y.val += delta_y;
    pos->z.val += delta_z;
}

struct Thing *get_shot_collided_with_same_type(struct Thing *thing, struct Coord3d *nxpos)
{
    return _DK_get_shot_collided_with_same_type(thing, nxpos);
}

long shot_hit_wall_at(struct Thing *thing, struct Coord3d *pos)
{
    return _DK_shot_hit_wall_at(thing, pos);
}

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
    _DK_create_relevant_effect_for_shot_hitting_thing(shotng, target); return;
}

long check_hit_when_attacking_door(struct Thing *thing)
{
    return _DK_check_hit_when_attacking_door(thing);
}

long get_no_creatures_in_group(struct Thing *grptng)
{
    struct CreatureControl *cctrl;
    struct Thing *ctng;
    long i;
    unsigned long k;
    cctrl = creature_control_get_from_thing(grptng);
    i = cctrl->field_7A & 0xFFF;
    if (i == 0) {
        // No group - just one creature
        return 1;
    }
    k = 0;
    while (i > 0)
    {
        ctng = thing_get(i);
        cctrl = creature_control_get_from_thing(ctng);
        if (creature_control_invalid(cctrl))
            break;
        i = cctrl->next_in_group;
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures group");
            break;
        }
    }
    return k;
}

TbBool shot_kill_creature(struct Thing *shotng, struct Thing *target)
{
    struct CreatureControl *cctrl;
    struct ShotConfigStats *shotst;
    cctrl = creature_control_get_from_thing(target);
    target->health = -1;
    cctrl->field_1D3 = shotng->model;
    if (shotng->field_1D == shotng->index) {
        kill_creature(target, INVALID_THING, shotng->owner, 0, 1, 0);
    } else {
        shotst = get_shot_model_stats(shotng->model);
        kill_creature(target, thing_get(shotng->field_1D), shotng->owner, 0, 1, shotst->old->numfield_4D);
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
    if (shotng->field_1D != shotng->index)
        shooter = thing_get(shotng->field_1D);
    tgcrstat = creature_stats_get_from_thing(target);
    tgcctrl = creature_control_get_from_thing(target);
    damage = get_damage_of_melee_shot(shotng, target);
    if (damage != 0)
    {
      if (shotst->old->field_22 > 0)
      {
          thing_play_sample(target, shotst->old->field_22, 0x64u, 0, 3, 0, 2, 256);
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
        target->pos_32.x.val += (throw_strength * (long)shotng->velocity.x.val) / 16;
        target->pos_32.y.val += (throw_strength * (long)shotng->velocity.y.val) / 16;
        target->field_1 |= 0x04;
      }
      create_relevant_effect_for_shot_hitting_thing(shotng, target);
      if ( target->health >= 0 )
      {
        if (target->owner != shotng->owner) {
            check_hit_when_attacking_door(target);
        }
      } else
      {
          shot_kill_creature(shotng,target);
      }
    }
    if (shotst->old->field_18) {
        delete_thing_structure(shotng, 0);
    }
    return 1;
}

long shot_hit_shootable_thing_at(struct Thing *shotng, struct Thing *target, struct Coord3d *pos)
{
    long i;
    /*if ((game.play_gameturn > 11310)) {
        SYNCLOG("index %d, model %d hit%d A",(int)shotng->index,(int)shotng->model,(int)shotng->byte_16);
        things_stats_debug_dump();//!!!!
    }*/
    i = _DK_shot_hit_shootable_thing_at(shotng, target, pos);
    /*if ((game.play_gameturn > 11310)) {
        SYNCLOG("index %d, model %d hit%d B",(int)shotng->index,(int)shotng->model,(int)shotng->byte_16);
        things_stats_debug_dump();//!!!!
    }*/
    return i;
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
    return (thing->class_id == 1) && (thing->model == 5) && (coltng->owner != thing->owner);
}

struct Thing *get_thing_collided_with_at_satisfying_filter(struct Thing *thing, struct Coord3d *pos, Thing_Collide_Func filter, long a4, long a5)
{
    return _DK_get_thing_collided_with_at_satisfying_filter(thing, pos, filter, a4, a5);
}

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

long move_shot(struct Thing *thing)
{
    //return _DK_move_shot(thing);
    struct ShotConfigStats *shotst;
    struct Coord3d filpos;

    get_thing_next_position(&filpos, thing);
    shotst = get_shot_model_stats(thing->model);
    if ( !shotst->old->field_28 )
    {
        if ( shot_hit_something_while_moving(thing, &filpos) ) {
            return 0;
        }
    }
    if ((thing->field_25 & 0x10) != 0)
    {
      if ( (shotst->old->field_48) && thing_in_wall_at(thing, &filpos) && shot_hit_door_at(thing, &filpos) ) {
          return 0;
      }
    } else
    {
      if ( thing_in_wall_at(thing, &filpos) && shot_hit_wall_at(thing, &filpos) ) {
          return 0;
      }
    }
    if ( (thing->mappos.x.stl.num != filpos.x.stl.num) || (thing->mappos.y.stl.num != filpos.y.stl.num) )
    {
        remove_thing_from_mapwho(thing);
        thing->mappos.x.val = filpos.x.val;
        thing->mappos.y.val = filpos.y.val;
        thing->mappos.z.val = filpos.z.val;
        place_thing_in_mapwho(thing);
    } else
    {
        thing->mappos.x.val = filpos.x.val;
        thing->mappos.y.val = filpos.y.val;
        thing->mappos.z.val = filpos.z.val;
    }
    thing->field_60 = get_thing_height_at(thing, &thing->mappos);
    return 1;
}

long update_shot(struct Thing *thing)
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
    SYNCDBG(18,"Starting for model %d",(int)thing->model);
    //return _DK_update_shot(thing);
    hit = false;
    shotst = get_shot_model_stats(thing->model);
    myplyr = get_my_player();
    if (shotst->old->shot_sound != 0)
    {
        if (!S3DEmitterIsPlayingSample(thing->field_66, shotst->old->shot_sound, 0))
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
        case 2:
            for (i = 2; i > 0; i--)
            {
              pos1.x.val = thing->mappos.x.val - ACTION_RANDOM(127) + 63;
              pos1.y.val = thing->mappos.y.val - ACTION_RANDOM(127) + 63;
              pos1.z.val = thing->mappos.z.val - ACTION_RANDOM(127) + 63;
              create_thing(&pos1, TCls_EffectElem, 1, thing->owner, -1);
            }
            break;
        case 4:
            if ( lightning_is_close_to_player(myplyr, &thing->mappos) )
            {
              if (is_my_player_number(thing->owner))
              {
                  player = get_player(thing->owner);
                  if ((thing->field_1D != 0) && (myplyr->field_2F == thing->field_1D))
                  {
                      PaletteSetPlayerPalette(player, lightning_palette);
                      myplyr->field_3 |= 0x08;
                  }
              }
            }
            break;
        case 6:
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
                thing->pos_32.x.val += cvect.x;
                thing->pos_32.y.val += cvect.y;
                thing->pos_32.z.val += cvect.z;
                thing->field_1 |= 0x04;
            }
            break;
        case 8:
            for (i = 10; i > 0; i--)
            {
              pos1.x.val = thing->mappos.x.val - ACTION_RANDOM(1023) + 511;
              pos1.y.val = thing->mappos.y.val - ACTION_RANDOM(1023) + 511;
              pos1.z.val = thing->mappos.z.val - ACTION_RANDOM(1023) + 511;
              create_thing(&pos1, TCls_EffectElem, 12, thing->owner, -1);
            }
            affect_nearby_enemy_creatures_with_wind(thing);
            break;
        case 11:
            thing->field_52 = (thing->field_52 + 113) & 0x7FF;
            break;
        case 15:
        case 20:
            if ( apply_wallhug_force_to_boulder(thing) )
              hit = true;
            break;
        case 16:
            draw_god_lightning(thing);
            lightning_modify_palette(thing);
            break;
        case 18:
            affect_nearby_stuff_with_vortex(thing);
            break;
        case 19:
            affect_nearby_friends_with_alarm(thing);
            break;
        case 24:
            update_god_lightning_ball(thing);
            break;
        case 29:
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
        return 0;
    }
    if (hit) {
        detonate_shot(thing);
        return 0;
    }
    return move_shot(thing);
}

struct Thing *create_shot(struct Coord3d *pos, unsigned short model, unsigned short owner)
{
    struct ShotConfigStats *shotst;
    struct InitLight ilght;
    struct Thing *thing;
    if ( !i_can_allocate_free_thing_structure(TAF_FreeEffectIfNoSlots) )
    {
        ERRORDBG(3,"Cannot create shot %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        //things_stats_debug_dump();
        return NULL;
    }
    memset(&ilght, 0, sizeof(struct InitLight));
    shotst = get_shot_model_stats(model);
    thing = allocate_free_thing_structure(TAF_FreeEffectIfNoSlots);
    thing->field_9 = game.play_gameturn;
    thing->class_id = TCls_Shot;
    thing->model = model;
    memcpy(&thing->mappos,pos,sizeof(struct Coord3d));
    thing->field_1D = thing->index;
    thing->owner = owner;
    thing->field_22 = shotst->old->field_D;
    thing->field_20 = shotst->old->field_F;
    thing->field_21 = shotst->old->field_10;
    thing->field_23 = shotst->old->field_11;
    thing->field_24 = shotst->old->field_12;
    thing->field_25 ^= (thing->field_25 ^ 8 * shotst->old->field_13) & 8;
    set_thing_draw(thing, shotst->old->numfield_0, 256, shotst->old->numfield_2, 0, 0, 2);
    thing->field_4F ^= (thing->field_4F ^ 0x02 * shotst->old->field_6) & 0x02;
    thing->field_4F ^= thing->field_4F ^ ((thing->field_4F ^ 0x10 * shotst->old->field_8) & 0x30);
    thing->field_4F ^= (thing->field_4F ^ shotst->old->field_7) & 0x01;
    thing->field_56 = shotst->old->field_9;
    thing->field_58 = shotst->old->field_B;
    thing->field_5A = shotst->old->field_9;
    thing->field_5C = shotst->old->field_B;
    thing->word_13 = shotst->old->damage;
    thing->health = shotst->old->health;
    if (shotst->old->field_50)
    {
        memcpy(&ilght.mappos,&thing->mappos,sizeof(struct Coord3d));
        ilght.field_0 = shotst->old->field_50;
        ilght.field_2 = shotst->old->field_52;
        ilght.field_11 = 1;
        ilght.field_3 = shotst->old->field_53;
        thing->field_62 = light_create_light(&ilght);
        if (thing->field_62 == 0) {
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
