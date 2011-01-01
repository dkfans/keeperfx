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
#include "gui_topmsg.h"
#include "creature_states.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_move_shot(struct Thing *thing);
DLLIMPORT long _DK_update_shot(struct Thing *thing);

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

long move_shot(struct Thing *thing)
{
  return _DK_move_shot(thing);
}

long update_shot(struct Thing *thing)
{
    struct ShotConfigStats *shotst;
    struct PlayerInfo *myplyr;
    struct PlayerInfo *player;
    struct Thing *target;
    struct Thing *shooter;
    struct Coord3d pos1;
    struct Coord3d pos2;
    struct CoordDelta3d dtpos;
    struct ComponentVector cvect;
    long i;
    TbBool hit;
    SYNCDBG(18,"Starting");
    //return _DK_update_shot(thing);
    shooter = NULL;
    hit = false;
    shotst = get_shot_model_stats(thing->model);
    myplyr = get_my_player();
    if (thing->index != thing->field_1D)
        shooter = thing_get(thing->field_1D);
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
    if (!hit)
      return move_shot(thing);
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
    return 0;
}

struct Thing *create_shot(struct Coord3d *pos, unsigned short model, unsigned short owner)
{
    struct ShotConfigStats *shotst;
    struct InitLight ilght;
    struct Thing *thing;
    if ( !i_can_allocate_free_thing_structure(1) )
    {
        ERRORDBG(3,"Cannot create shot %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return NULL;
    }
    memset(&ilght, 0, sizeof(struct InitLight));
    shotst = get_shot_model_stats(model);
    thing = allocate_free_thing_structure(1);
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
          WARNLOG("Cannot allocate dynamic light to %s.",thing_model_name(thing));
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
