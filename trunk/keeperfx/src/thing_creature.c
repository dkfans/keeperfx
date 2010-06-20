/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_creature.c
 *     Creatures related functions.
 * @par Purpose:
 *     Functions for support of creatures as things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     17 Mar 2009 - 10 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_creature.h"
#include "globals.h"

#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_filelst.h"
#include "bflib_sprite.h"

#include "engine_lenses.h"
#include "config_creature.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "creature_graphics.h"
#include "config_lenses.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "thing_objects.h"
#include "thing_navigate.h"
#include "lens_api.h"
#include "light_data.h"
#include "keeperfx.hpp"
#include "frontend.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
int creature_swap_idx[CREATURE_TYPES_COUNT];

/******************************************************************************/
DLLIMPORT struct Thing *_DK_find_my_next_creature_of_breed_and_job(long breed_idx, long job_idx, long a3);
DLLIMPORT void _DK_anger_set_creature_anger_all_types(struct Thing *thing, long a2);
DLLIMPORT void _DK_change_creature_owner(struct Thing *thing , char nowner);
DLLIMPORT long _DK_remove_all_traces_of_combat(struct Thing *thing);
DLLIMPORT void _DK_cause_creature_death(struct Thing *thing, unsigned char a2);
DLLIMPORT void _DK_apply_spell_effect_to_thing(struct Thing *thing, long spell_idx, long spell_lev);
DLLIMPORT void _DK_creature_cast_spell_at_thing(struct Thing *caster, struct Thing *target, long a3, long a4);
DLLIMPORT void _DK_creature_cast_spell(struct Thing *caster, long a2, long a3, long a4, long a5);
DLLIMPORT void _DK_set_first_creature(struct Thing *thing);
DLLIMPORT void _DK_remove_first_creature(struct Thing *thing);
DLLIMPORT struct Thing *_DK_get_creature_near(unsigned short pos_x, unsigned short pos_y);
DLLIMPORT struct Thing *_DK_get_creature_near_with_filter(unsigned short pos_x, unsigned short pos_y, Thing_Filter filter, long a4);
DLLIMPORT struct Thing *_DK_get_creature_near_for_controlling(unsigned char a1, long a2, long a3);
DLLIMPORT long _DK_remove_creature_from_group(struct Thing *thing);
DLLIMPORT long _DK_add_creature_to_group_as_leader(struct Thing *thing1, struct Thing *thing2);
DLLIMPORT void _DK_anger_apply_anger_to_creature(struct Thing *thing, long anger, long a2, long a3);
DLLIMPORT long _DK_creature_available_for_combat_this_turn(struct Thing *thing);
DLLIMPORT long _DK_creature_look_for_combat(struct Thing *thing);
DLLIMPORT struct Thing *_DK_get_enemy_dungeon_heart_creature_can_see(struct Thing *thing);
DLLIMPORT long _DK_set_creature_object_combat(struct Thing *crthing, struct Thing *obthing);
DLLIMPORT void _DK_set_creature_door_combat(struct Thing *crthing, struct Thing *obthing);
DLLIMPORT void _DK_food_eaten_by_creature(struct Thing *crthing, struct Thing *obthing);
DLLIMPORT void _DK_creature_fire_shot(struct Thing *firing,struct  Thing *target, unsigned short a1, char a2, unsigned char a3);
DLLIMPORT unsigned long _DK_control_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
DLLIMPORT unsigned long _DK_control_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing);
DLLIMPORT void _DK_load_swipe_graphic_for_creature(struct Thing *thing);
DLLIMPORT unsigned short _DK_find_next_annoyed_creature(unsigned char a1, unsigned short a2);
DLLIMPORT long _DK_creature_instance_has_reset(struct Thing *thing, long a2);
DLLIMPORT long _DK_get_human_controlled_creature_target(struct Thing *thing, long a2);
DLLIMPORT void _DK_set_creature_instance(struct Thing *thing, long a1, long a2, long a3, struct Coord3d *pos);
DLLIMPORT void _DK_draw_creature_view(struct Thing *thing);
DLLIMPORT void _DK_process_creature_standing_on_corpses_at(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT short _DK_kill_creature(struct Thing *thing, struct Thing *tngrp, char a1, unsigned char a2, unsigned char a3, unsigned char a4);
DLLIMPORT void _DK_update_creature_count(struct Thing *thing);
DLLIMPORT long _DK_process_creature_state(struct Thing *thing);
DLLIMPORT long _DK_move_creature(struct Thing *thing);
DLLIMPORT void _DK_init_creature_level(struct Thing *thing, long nlev);
DLLIMPORT long _DK_check_for_first_person_barrack_party(struct Thing *thing);
DLLIMPORT void _DK_terminate_thing_spell_effect(struct Thing *thing, long a2);
DLLIMPORT void _DK_creature_increase_level(struct Thing *thing);
DLLIMPORT struct Thing *_DK_destroy_creature_and_create_corpse(struct Thing *thing, long a1);
DLLIMPORT void _DK_thing_death_flesh_explosion(struct Thing *thing);
DLLIMPORT void _DK_thing_death_gas_and_flesh_explosion(struct Thing *thing);
DLLIMPORT void _DK_thing_death_smoke_explosion(struct Thing *thing);
DLLIMPORT void _DK_thing_death_ice_explosion(struct Thing *thing);
DLLIMPORT struct Thing *_DK_create_dead_creature(struct Coord3d *pos, unsigned short model, unsigned short a1, unsigned short owner, long explevel);
/******************************************************************************/
TbBool thing_can_be_controlled_as_controller(struct Thing *thing)
{
  if (thing->class_id == TCls_Creature)
    return true;
  if (thing->class_id == TCls_DeadCreature)
    return true;
  return false;
}

TbBool thing_can_be_controlled_as_passenger(struct Thing *thing)
{
  if (thing->class_id == TCls_Creature)
    return true;
  if (thing->class_id == TCls_DeadCreature)
    return true;
  if ((thing->class_id == TCls_Object) && (thing->model == 10))
    return true;
  return false;
}

long check_for_first_person_barrack_party(struct Thing *thing)
{
  return _DK_check_for_first_person_barrack_party(thing);
}

TbBool control_creature_as_controller(struct PlayerInfo *player, struct Thing *thing)
{
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  struct InitLight ilght;
  struct Camera *cam;
  //return _DK_control_creature_as_controller(player, thing);
  if ( (thing->owner != player->id_number) || !thing_can_be_controlled_as_controller(thing) )
  {
    if (!control_creature_as_passenger(player, thing))
      return false;
    cam = player->acamera;
    crstat = creature_stats_get(23);
    cam->mappos.z.val += crstat->eye_height;
    return true;
  }
  cctrl = creature_control_get_from_thing(thing);
  cctrl->moveto_pos.x.val = 0;
  cctrl->moveto_pos.y.val = 0;
  cctrl->moveto_pos.z.val = 0;
  if (is_my_player(player))
  {
    toggle_status_menu(0);
    turn_off_roaming_menus();
  }
  cam = player->acamera;
  player->field_2F = thing->index;
  player->field_31 = thing->field_9;
  if (cam != NULL)
    player->field_4B5 = cam->field_6;
  thing->field_0 |= 0x20u;
  thing->field_4F |= 0x01;
  set_start_state(thing);
  set_player_mode(player, 2);
  if (thing->class_id == TCls_Creature)
  {
    cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
    check_for_first_person_barrack_party(thing);
    if ((cctrl->field_7A & 0xFFF) != 0)
      make_group_member_leader(thing);
  }
  memset(&ilght, 0, sizeof(struct InitLight));
  ilght.mappos.x.val = thing->mappos.x.val;
  ilght.mappos.y.val = thing->mappos.y.val;
  ilght.mappos.z.val = thing->mappos.z.val;
  ilght.field_3 = 1;
  ilght.field_2 = 36;
  ilght.field_0 = 2560;
  ilght.field_11 = 1;
  thing->field_62 = light_create_light(&ilght);
  light_set_light_never_cache(thing->field_62);
  if (thing->field_62 == 0)
    ERRORLOG("Cannot allocate light to new controlled thing");
  if (is_my_player_number(thing->owner))
  {
    if (thing->class_id == TCls_Creature)
    {
      crstat = creature_stats_get_from_thing(thing);
      setup_eye_lens(crstat->eye_effect);
    }
  }
  return true;
}

TbBool control_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing)
{
  struct Camera *cam;
  //return _DK_control_creature_as_passenger(player, thing);
  if (thing->owner != player->id_number)
  {
    ERRORLOG("Player %d cannot control as passenger thing owned by player %d",(int)player->id_number,(int)thing->owner);
    return false;
  }
  if (!thing_can_be_controlled_as_passenger(thing))
  {
    ERRORLOG("Thing of class %d and model %d can't be controlled as passenger",
        (int)thing->class_id,(int)thing->model);
    return false;
  }
  if (is_my_player(player))
  {
    toggle_status_menu(0);
    turn_off_roaming_menus();
  }
  cam = player->acamera;
  player->field_2F = thing->index;
  player->field_31 = thing->field_9;
  if (cam != NULL)
    player->field_4B5 = cam->field_6;
  set_player_mode(player, 3);
  thing->field_4F |= 0x01;
  return true;
}

void free_swipe_graphic(void)
{
    SYNCDBG(6,"Starting");
    if (game.field_1516FF != -1)
    {
      LbDataFreeAll(swipe_load_file);
      game.field_1516FF = -1;
    }
    LbSpriteClearAll(swipe_setup_sprites);
}

void load_swipe_graphic_for_creature(struct Thing *thing)
{
    struct TbLoadFiles *t_lfile;
    int swpe_idx;
    int i;
    SYNCDBG(6,"Starting for model %d",(int)thing->model);
    //_DK_load_swipe_graphic_for_creature(thing);

    i = creatures[thing->model%CREATURE_TYPES_COUNT].field_8;
    if ((i == 0) || (game.field_1516FF == i))
        return;
    free_swipe_graphic();
    swpe_idx = 5 * (i-1);
    t_lfile = &swipe_load_file[0];
    for (i=0; i < 5; i++)
    {
        sprintf(t_lfile->FName, "data/swpe%02d.dat", swpe_idx+i);
        t_lfile++;
        sprintf(t_lfile->FName, "data/swpe%02d.tab", swpe_idx+i);
        t_lfile++;
    }
    if ( LbDataLoadAll(swipe_load_file) )
    {
        free_swipe_graphic();
        ERRORLOG("Unable to load swipe graphics for creature model %d",(int)thing->model);
        return;
    }
    LbSpriteSetupAll(swipe_setup_sprites);
    game.field_1516FF = swpe_idx;
}

long creature_available_for_combat_this_turn(struct Thing *thing)
{
  return _DK_creature_available_for_combat_this_turn(thing);
}

long creature_look_for_combat(struct Thing *thing)
{
  return _DK_creature_look_for_combat(thing);
}

struct Thing *get_enemy_dungeon_heart_creature_can_see(struct Thing *thing)
{
  return _DK_get_enemy_dungeon_heart_creature_can_see(thing);
}

long set_creature_object_combat(struct Thing *crthing, struct Thing *obthing)
{
  return _DK_set_creature_object_combat(crthing, obthing);
}

void set_creature_door_combat(struct Thing *crthing, struct Thing *obthing)
{
  _DK_set_creature_door_combat(crthing, obthing);
}

void food_eaten_by_creature(struct Thing *crthing, struct Thing *obthing)
{
  _DK_food_eaten_by_creature(crthing, obthing);
}

void anger_apply_anger_to_creature(struct Thing *thing, long anger, long a2, long a3)
{
  _DK_anger_apply_anger_to_creature(thing, anger, a2, a3);
}

void terminate_thing_spell_effect(struct Thing *thing, long a2)
{
    _DK_terminate_thing_spell_effect(thing, a2);
}

long get_free_spell_slot(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long ci,cval;
    long i,k;
    cctrl = creature_control_get_from_thing(thing);
    cval = LONG_MAX;
    ci = -1;
    for (i=0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        // If there's unused slot, return it immediately
        if (cctrl->field_1D4[i].field_0 == 0)
        {
            return i;
        }
        // Otherwise, select the one making minimum damage
        k = abs(cctrl->field_1D4[i].field_1);
        if (k < cval)
        {
            cval = k;
            ci = i;
        }
    }
    // Terminate the min damage effect and return its slot index
    terminate_thing_spell_effect(thing, cctrl->field_1D4[ci].field_0);
    for (i=0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        if (cctrl->field_1D4[i].field_0 == 0)
        {
            return i;
        }
    }
    ERRORLOG("Spell effect has been terminated, but still its slot (%ld) isn't empty!",ci);
    return ci;
}

void first_apply_spell_effect_to_thing(struct Thing *thing, long spell_idx, long spell_lev)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct SpellConfig *splconf;
    struct ComponentVector cvect;
    struct Coord3d pos;
    struct Thing *ntng;
    long i,k,n;
    cctrl = creature_control_get_from_thing(thing);
    if (spell_lev > SPELL_MAX_LEVEL)
        spell_lev = SPELL_MAX_LEVEL;
    // This pointer may be invalid if spell_idx is incorrect. But we're using it only when correct.
    splconf = &game.spells_config[spell_idx];
    switch ( spell_idx )
    {
    case SplK_Freeze:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->field_1D4[i].field_0 = spell_idx;
            cctrl->field_1D4[i].field_1 = splconf->duration;
            cctrl->field_AB |= 0x02;
            if ((thing->field_25 & 0x20) != 0)
            {
                cctrl->field_AD |= 0x80;
                thing->field_25 &= 0xDF;
            }
            creature_set_speed(thing, 0);
        }
        break;
    case SplK_Armour:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
          cctrl->field_1D4[i].field_0 = spell_idx;
          cctrl->field_1D4[i].field_1 = game.magic_stats[12].power[spell_lev];
          n = 0;
          cctrl->spell_flags |= CSF_Armour;
          for (k=0; k < 3; k++)
          {
            pos.x.val = thing->mappos.x.val;
            pos.y.val = thing->mappos.y.val;
            pos.z.val = thing->mappos.z.val;
            pos.x.val += (32 * LbSinL(n) >> 16);
            pos.y.val -= (32 * LbCosL(n) >> 16);
            pos.z.val += k * (long)(thing->field_58 >> 1);
            ntng = create_object(&pos, 51, thing->owner, -1);
            if (!thing_is_invalid(ntng))
            {
              cctrl->field_2B3[k] = ntng->index;
              ntng->health = game.magic_stats[12].power[spell_lev] + 1;
              ntng->word_13 = thing->index;
              ntng->byte_15 = k;
              ntng->field_52 = thing->field_52;
              ntng->field_54 = thing->field_54;
              angles_to_vector(ntng->field_52, ntng->field_54, 32, &cvect);
              ntng->pos_32.x.val += cvect.x;
              ntng->pos_32.y.val += cvect.y;
              ntng->pos_32.z.val += cvect.z;
              ntng->field_1 |= 0x04;
            }
            n += ANGLE_TRIGL_PERIOD/3;
          }
        }
        break;
    case SplK_Rebound:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->field_1D4[i].field_0 = spell_idx;
            cctrl->field_1D4[i].field_1 = splconf->duration;
            cctrl->spell_flags |= CSF_Rebound;
        }
        break;
    case SplK_Heal:
        crstat = creature_stats_get_from_thing(thing);
        i = saturate_set_signed(thing->health + game.magic_stats[8].power[spell_lev],16);
        if (i < 0)
        {
          thing->health = 0;
        } else
        {
          k = compute_creature_max_health(crstat->health,cctrl->explevel);
          thing->health = min(i,k);
        }
        cctrl->field_2B0 = 7;
        cctrl->field_2AE = game.magic_stats[8].time;
        break;
    case SplK_Invisibility:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->field_1D4[i].field_0 = spell_idx;
            cctrl->field_1D4[i].field_1 = game.magic_stats[13].power[spell_lev];
            cctrl->spell_flags |= CSF_Conceal;
            cctrl->field_AF = 0;
        }
        break;
    case SplK_Teleport:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->field_1D4[i].field_0 = spell_idx;
            cctrl->field_1D4[i].field_1 = splconf->duration;
            cctrl->field_AB |= 0x04;
        }
        break;
    case SplK_Speed:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->field_1D4[i].field_0 = spell_idx;
            cctrl->field_1D4[i].field_1 = game.magic_stats[11].power[spell_lev];
            cctrl->spell_flags |= CSF_Speed;
            cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        }
        break;
    case SplK_Slow:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->field_1D4[i].field_0 = spell_idx;
            cctrl->field_1D4[i].field_1 = splconf->duration;
            cctrl->spell_flags |= CSF_Slow;
            cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        }
        break;
    case SplK_Fly:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->field_1D4[i].field_0 = spell_idx;
            cctrl->field_1D4[i].field_1 = splconf->duration;
            cctrl->spell_flags |= CSF_Fly;
            thing->field_25 |= 0x20;
        }
        break;
    case SplK_Sight:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->field_1D4[i].field_0 = spell_idx;
            cctrl->field_1D4[i].field_1 = splconf->duration;
            cctrl->spell_flags |= CSF_Sight;
        }
        break;
    case SplK_Disease:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
          cctrl->field_1D4[i].field_0 = spell_idx;
          cctrl->field_1D4[i].field_1 = game.magic_stats[14].power[spell_lev];
          n = 0;
          cctrl->field_AD |= 0x01;
          cctrl->field_B6 = thing->owner;
          cctrl->field_2EB = game.play_gameturn;
          for (k=0; k < 3; k++)
          {
            pos.x.val = thing->mappos.x.val;
            pos.y.val = thing->mappos.y.val;
            pos.z.val = thing->mappos.z.val;
            pos.x.val += (32 * LbSinL(n) >> 16);
            pos.y.val -= (32 * LbCosL(n) >> 16);
            pos.z.val += k * (long)(thing->field_58 >> 1);
            ntng = create_object(&pos, 112, thing->owner, -1);
            if (!thing_is_invalid(ntng))
            {
              cctrl->field_2B9[k] = ntng->index;
              ntng->health = game.magic_stats[14].power[spell_lev] + 1;
              ntng->word_13 = thing->index;
              ntng->byte_15 = k;
              ntng->field_52 = thing->field_52;
              ntng->field_54 = thing->field_54;
              angles_to_vector(ntng->field_52, ntng->field_54, 32, &cvect);
              ntng->pos_32.x.val += cvect.x;
              ntng->pos_32.y.val += cvect.y;
              ntng->pos_32.z.val += cvect.z;
              ntng->field_1 |= 0x04;
            }
            n += ANGLE_TRIGL_PERIOD/3;
          }
        }
        break;
    case SplK_Chicken:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            external_set_thing_state(thing, 120);
            cctrl->field_282 = 10;
            cctrl->field_AD |= 0x02;
            cctrl->field_1D4[i].field_0 = spell_idx;
            cctrl->field_1D4[i].field_1 = game.magic_stats[15].power[spell_lev];
        }
        break;
    default:
        WARNLOG("No action for spell %ld at level %ld",spell_idx,spell_lev);
        break;
    }
}

void reapply_spell_effect_to_thing(struct Thing *thing, long spell_idx, long spell_lev, long idx)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct SpellConfig *splconf;
    long i,k;
    cctrl = creature_control_get_from_thing(thing);
    if (spell_lev > SPELL_MAX_LEVEL)
        spell_lev = SPELL_MAX_LEVEL;
    // This pointer may be invalid if spell_idx is incorrect. But we're using it only when correct.
    splconf = &game.spells_config[spell_idx];
    switch (spell_idx)
    {
    case SplK_Freeze:
        cctrl->field_1D4[idx].field_1 = splconf->duration;
        creature_set_speed(thing, 0);
        break;
    case SplK_Armour:
        cctrl->field_1D4[idx].field_1 = game.magic_stats[12].power[spell_lev];
        break;
    case SplK_Rebound:
        cctrl->field_1D4[idx].field_1 = splconf->duration;
        break;
    case SplK_Heal:
        crstat = creature_stats_get_from_thing(thing);
        i = saturate_set_signed(thing->health + game.magic_stats[8].power[spell_lev],16);
        if (i < 0)
        {
          thing->health = 0;
        } else
        {
          k = compute_creature_max_health(crstat->health,cctrl->explevel);
          thing->health = min(i,k);
        }
        cctrl->field_2B0 = 7;
        cctrl->field_2AE = game.magic_stats[8].time;
        break;
    case SplK_Invisibility:
        cctrl->field_1D4[idx].field_1 = game.magic_stats[13].power[spell_lev];
        break;
    case SplK_Teleport:
        cctrl->field_1D4[idx].field_1 = splconf->duration;
        break;
    case SplK_Speed:
        cctrl->field_1D4[idx].field_1 = game.magic_stats[11].power[spell_lev];
        break;
    case SplK_Slow:
        cctrl->field_1D4[idx].field_1 = splconf->duration;
        break;
    case SplK_Light:
        cctrl->field_1D4[idx].field_1 = splconf->duration;
        break;
    case SplK_Fly:
        cctrl->field_1D4[idx].field_1 = splconf->duration;
        break;
    case SplK_Sight:
        cctrl->field_1D4[idx].field_1 = splconf->duration;
        break;
    case SplK_Disease:
        cctrl->field_1D4[idx].field_1 = game.magic_stats[14].power[spell_lev];
        cctrl->field_B2[4] = thing->owner;
        break;
    case SplK_Chicken:
        external_set_thing_state(thing, 120);
        cctrl->field_282 = 10;
        cctrl->field_1D4[idx].field_1 = game.magic_stats[15].power[spell_lev];
        break;
    default:
        WARNLOG("No action for spell %ld at level %ld",spell_idx,spell_lev);
        break;
    }
}

void apply_spell_effect_to_thing(struct Thing *thing, long spell_idx, long spell_lev)
{
    struct CreatureControl *cctrl;
    long i;
    // Make sure the creature level isn't larger than max spell level
    if (spell_lev > SPELL_MAX_LEVEL)
        spell_lev = SPELL_MAX_LEVEL;
    //_DK_apply_spell_effect_to_thing(thing, spell_idx, spell_lev); return;
    cctrl = creature_control_get_from_thing(thing);
    for (i=0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        if (cctrl->field_1D4[i].field_0 == spell_idx)
        {
            reapply_spell_effect_to_thing(thing, spell_idx, spell_lev, i);
            return;
        }
    }
    first_apply_spell_effect_to_thing(thing, spell_idx, spell_lev);
}

short creature_take_wage_from_gold_pile(struct Thing *crthing,struct Thing *obthing)
{
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  long i;
  crstat = creature_stats_get_from_thing(crthing);
  cctrl = creature_control_get_from_thing(crthing);
  if (obthing->long_13 <= 0)
  {
    ERRORLOG("GoldPile had no gold so was deleted.");
    delete_thing_structure(obthing, 0);
    return false;
  }
  if (crthing->long_13 < crstat->gold_hold)
  {
    if (obthing->long_13+crthing->long_13 > crstat->gold_hold)
    {
      i = crstat->gold_hold-crthing->long_13;
      crthing->long_13 += i;
      obthing->long_13 -= i;
    } else
    {
      crthing->long_13 += obthing->long_13;
      delete_thing_structure(obthing, 0);
    }
  }
  anger_apply_anger_to_creature(crthing, crstat->annoy_got_wage, 1, 1);
  return true;
}

void creature_cast_spell_at_thing(struct Thing *caster, struct Thing *target, long spl_idx, long a4)
{
  const struct SpellInfo *spinfo;
  long i;
  if ((caster->field_0 & 0x20) != 0)
  {
    if (target->class_id == TCls_Object)
      i = 1;
    else
      i = 2;
  } else
  {
    if (target->class_id == TCls_Object)
      i = 3;
    else
    if (target->owner == caster->owner)
      i = 2;
    else
      i = 4;
  }
  spinfo = get_magic_info(spl_idx);
  if (magic_info_is_invalid(spinfo))
  {
    ERRORLOG("Thing owned by player %d tried to cast invalid spell %ld",(int)caster->owner,spl_idx);
    return;
  }
  creature_fire_shot(caster, target, spinfo->field_1, a4, i);
}

void creature_cast_spell(struct Thing *caster, long spl_idx, long a3, long trg_x, long trg_y)
{
  const struct SpellInfo *spinfo;
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  struct Thing *efthing;
  long i,k;
  //_DK_creature_cast_spell(caster, spl_idx, a3, trg_x, trg_y);
  spinfo = get_magic_info(spl_idx);
  cctrl = creature_control_get_from_thing(caster);
  crstat = creature_stats_get_from_thing(caster);
  if (creature_control_invalid(cctrl))
  {
    ERRORLOG("Invalid creature tried to cast spell %ld",spl_idx);
    return;
  }
  if (spl_idx == 10) // Teleport
  {
    cctrl->teleport_x = trg_x;
    cctrl->teleport_y = trg_y;
  }
  // Check if the spell can be fired as a shot
  if (spinfo->field_1)
  {
    if ((caster->field_0 & 0x20) != 0)
      i = 1;
    else
      i = 4;
    creature_fire_shot(caster, 0, spinfo->field_1, a3, i);
  } else
  // Check if the spell can be self-casted
  if (spinfo->field_2)
  {
    i = (long)spinfo->field_6;
    if (i > 0)
      thing_play_sample(caster, i, 100, 0, 3, 0, 4, 256);
    apply_spell_effect_to_thing(caster, spl_idx, cctrl->explevel);
  }
  // Check if the spell has an effect associated
  if (spinfo->cast_effect != 0)
  {
    efthing = create_effect(&caster->mappos, spinfo->cast_effect, caster->owner);
    if (!thing_is_invalid(efthing))
    {
      if (spinfo->cast_effect == 14)
        efthing->byte_16 = 3;
    }
  }
  // If the spell has area_range, then make area damage
  if (spinfo->area_range > 0)
  {
    // This damage is computed directly, not selected from array, so it
    // don't have to be limited as others... but let's limit it anyway
    k = compute_creature_attack_range(spinfo->area_range, crstat->luck, cctrl->explevel);
    i = compute_creature_attack_damage(spinfo->area_damage, crstat->luck, cctrl->explevel);
    explosion_affecting_area(caster, &caster->mappos, k, i, spinfo->area_hit_type);
  }
}

void update_creature_count(struct Thing *thing)
{
  _DK_update_creature_count(thing);
}

struct Thing *find_gold_pile_or_chicken_laying_on_mapblk(struct Map *mapblk)
{
  struct Thing *thing;
  struct Room *room;
  unsigned long k;
  long i;
  k = 0;
  i = get_mapwho_thing_index(mapblk);
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      WARNLOG("Jump out of things array");
      break;
    }
    i = thing->field_2;
    if (thing->class_id == TCls_Object)
    {
      if ((thing->model == 43) && thing_touching_floor(thing))
        return thing;
      if (thing->model == 10)
      {
        room = get_room_thing_is_on(thing);
        if (room_is_invalid(room))
          return thing;
        if ((room->kind != RoK_GARDEN) && (room->kind != RoK_TORTURE) && (room->kind != RoK_PRISON))
          return thing;
      }
    }
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return INVALID_THING;
}

struct Thing *find_interesting_object_laying_around_thing(struct Thing *crthing)
{
  struct Thing *thing;
  struct Map *mapblk;
  long stl_x,stl_y;
  long k;
  for (k=0; k < AROUND_TILES_COUNT; k++)
  {
    stl_x = crthing->mappos.x.stl.num + around[k].delta_x;
    stl_y = crthing->mappos.y.stl.num + around[k].delta_y;
    mapblk = get_map_block_at(stl_x,stl_y);
    if (!map_block_invalid(mapblk))
    {
      if ((mapblk->flags & 0x10) == 0)
      {
        thing = find_gold_pile_or_chicken_laying_on_mapblk(mapblk);
        if (!thing_is_invalid(thing))
          return thing;
      }
    }
  }
  return INVALID_THING;
}

long process_creature_state(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  struct StateInfo *stati;
  struct Thing *tgthing;
  long x,y;
  long k;
  SYNCDBG(18,"Starting");
//TODO: rework! (causes hang if out of things)
  //return _DK_process_creature_state(thing);

  cctrl = creature_control_get_from_thing(thing);
  process_person_moods_and_needs(thing);
  if (creature_available_for_combat_this_turn(thing))
  {
    if (!creature_look_for_combat(thing))
    {
      if ((!cctrl->field_3) && (thing->model != 23))
      {
        tgthing = get_enemy_dungeon_heart_creature_can_see(thing);
        if (!thing_is_invalid(tgthing))
          set_creature_object_combat(thing, tgthing);
      }
    }
  }
  if ((cctrl->field_3 & 0x10) == 0)
  {
    if ((cctrl->field_1D0) && ((cctrl->flgfield_1 & 0x02) == 0))
    {
        if ( can_change_from_state_to(thing, thing->field_7, CrSt_CreatureDoorCombat) )
        {
          x = stl_num_decode_x(cctrl->field_1D0);
          y = stl_num_decode_y(cctrl->field_1D0);
          tgthing = get_door_for_position(x,y);
          if (!thing_is_invalid(tgthing))
          {
            if (thing->owner != tgthing->owner)
              set_creature_door_combat(thing, tgthing);
          }
        }
    }
  }
  cctrl->field_1D0 = 0;
  if ((cctrl->field_7A & 0xFFF) != 0)
  {
    if (!creature_is_group_leader(thing))
      process_obey_leader(thing);
  }
  if ((thing->field_7 < 1) || (thing->field_7 >= CREATURE_STATES_COUNT))
  {
    ERRORLOG("Creature has illegal state[1], T=%d, M=%d, S=%d, TCS=%d, reset", (int)thing->index, (int)thing->model, (int)thing->field_7, (int)thing->field_8);
    set_start_state(thing);
  }
  if (((thing->field_25 & 0x20) == 0) && (thing->model != 23))
  {
    tgthing = find_interesting_object_laying_around_thing(thing);
    if (!thing_is_invalid(tgthing))
    {
      if (tgthing->model == 43)
      {
        crstat = creature_stats_get_from_thing(thing);
        if (tgthing->long_13 > 0)
        {
          if (thing->long_13 < crstat->gold_hold)
          {
            if (crstat->gold_hold < tgthing->long_13 + thing->long_13)
            {
              k = crstat->gold_hold - thing->long_13;
              thing->long_13 += k;
              tgthing->long_13 -= k;
            } else
            {
              thing->long_13 += tgthing->long_13;
              delete_thing_structure(tgthing, 0);
            }
          }
        } else
        {
          ERRORLOG("GoldPile with no gold!");
          delete_thing_structure(tgthing, 0);
        }
        anger_apply_anger_to_creature(thing, crstat->annoy_got_wage, 1, 1);
      } else
      if (tgthing->model == 10)
      {
        if (!is_thing_passenger_controlled(tgthing))
          food_eaten_by_creature(tgthing, thing);
      }
    }
  }
  // Enable this to know which function hangs on update_creature.
  //TODO: rewrite state subfunctions so they won't hang
  SYNCDBG(18,"Executing state %d",(int)thing->field_7);
  stati = get_thing_state7_info(thing);
  if (stati->ofsfield_0 == NULL)
    return false;
  if (stati->ofsfield_0(thing) != -1)
    return false;
  SYNCDBG(18,"Finished");
  return true;
}

TbBool update_dead_creatures_list(struct Dungeon *dungeon, struct Thing *thing)
{
  struct CreatureStorage *cstore;
  struct CreatureControl *cctrl;
  long i;
  SYNCDBG(18,"Starting");
  cctrl = creature_control_get_from_thing(thing);
  if ((dungeon == NULL) || creature_control_invalid(cctrl))
  {
    WARNLOG("Invalid victim or dungeon");
    return false;
  }
  // Check if the creature of same type is in list
  i = dungeon->dead_creatures_count-1;
  while (i >= 0)
  {
    cstore = &dungeon->dead_creatures[i];
    if ((cstore->model == thing->model) && (cstore->explevel == cctrl->explevel))
    {
      // This creature is already in list
      SYNCDBG(18,"Already in list");
      return false;
    }
    i--;
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
  cstore->model = thing->model;
  cstore->explevel = cctrl->explevel;
  SYNCDBG(19,"Finished");
  return true;
}

/**
 * Increases proper kills counter for given player's dungeon.
 */
TbBool inc_player_kills_counter(long killer_idx, struct Thing *victim)
{
  struct Dungeon *killer_dungeon;
  killer_dungeon = get_players_num_dungeon(killer_idx);
  if (victim->owner == killer_idx)
    killer_dungeon->lvstats.friendly_kills++;
  else
    killer_dungeon->battles_won++;
  return true;
}

/**
 * Increases kills counters when victim is being killed by killer.
 * Note that killer may be invalid - in this case def_plyr_idx identifies the killer.
 */
TbBool update_kills_counters(struct Thing *victim, struct Thing *killer, char def_plyr_idx, unsigned char died_in_battle)
{
  struct CreatureControl *cctrl;
  cctrl = creature_control_get_from_thing(victim);
  if (died_in_battle)
  {
    if (!thing_is_invalid(killer))
    {
      return inc_player_kills_counter(killer->owner, victim);
    }
    if ((def_plyr_idx != -1) && (game.neutral_player_num != def_plyr_idx))
    {
      return inc_player_kills_counter(def_plyr_idx, victim);
    }
  }
  if ((cctrl->field_1D2 != -1) && (game.neutral_player_num != cctrl->field_1D2))
  {
    return inc_player_kills_counter(cctrl->field_1D2, victim);
  }
  return false;
}

long move_creature(struct Thing *thing)
{
  return _DK_move_creature(thing);
}

struct Thing *create_dead_creature(struct Coord3d *pos, unsigned short model, unsigned short a1, unsigned short owner, long explevel)
{
    struct Thing *thing;
    unsigned long k;
    //return _DK_create_dead_creature(pos, model, a1, owner, explevel);
    if (!i_can_allocate_free_thing_structure(1))
    {
        ERRORLOG("Cannot create dead creature because there are too many things allocated.");
        return INVALID_THING;
    }
    thing = allocate_free_thing_structure(1);
    thing->class_id = 4;
    thing->model = model;
    thing->field_1D = thing->index;
    thing->owner = owner;
    thing->byte_13 = explevel;
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = 0;
    thing->mappos.z.val = get_thing_height_at(thing, &thing->mappos);
    thing->field_56 = 0;
    thing->field_58 = 0;
    thing->field_5A = 0;
    thing->field_5C = 0;
    thing->field_20 = 16;
    thing->field_23 = 204;
    thing->field_24 = 51;
    thing->field_22 = 0;
    thing->field_25 |= 0x08;
    thing->field_9 = game.play_gameturn;
    if (creatures[model].field_7)
      thing->field_4F |= 0x30;
    add_thing_to_list(thing, &game.thing_lists[4]);
    place_thing_in_mapwho(thing);
    switch (a1)
    {
    case 2:
        thing->field_7 = 2;
        k = get_creature_anim(thing, 17);
        set_thing_draw(thing, k, 256, 300, 0, 0, 2);
        break;
    default:
        thing->field_7 = 1;
        k = get_creature_anim(thing, 15);
        set_thing_draw(thing, k, 128, 300, 0, 0, 2);
        thing->health = 3 * get_lifespan_of_animation(thing->field_44, thing->field_3E);
        play_creature_sound(thing, 9, 3, 0);
        break;
    }
    thing->field_46 = (300 * (long)thing->byte_13) / 20 + 300;
    return thing;
}

struct Thing *destroy_creature_and_create_corpse(struct Thing *thing, long a1)
{
    struct CreatureControl *cctrl;
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Thing *deadtng;
    struct Coord3d pos;
    TbBool memf1;
    long owner;
    long crmodel;
    long explevel;
    long prev_idx;

    //return _DK_destroy_creature_and_create_corpse(thing, a1);
    crmodel = thing->model;
    memf1 = ((thing->field_0 & 0x20) != 0);
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val;
    owner = thing->owner;
    prev_idx = thing->index;
    cctrl = creature_control_get_from_thing(thing);
    explevel = cctrl->explevel;
    player = NULL;
    if (owner != game.neutral_player_num)
    {
        player = get_player(owner);
        dungeon = get_players_dungeon(player);
        dungeon->score -= (long)game.creature_scores[crmodel].value[explevel];
    }
    delete_thing_structure(thing, 0);
    deadtng = create_dead_creature(&pos, crmodel, a1, owner, explevel);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Could not create dead thing.");
        return INVALID_THING;
    }
    set_flag_byte(&deadtng->field_0, 0x20, memf1);
    if (owner != game.neutral_player_num)
    {
        // Update thing index inside player struct
        if (player->field_2F == prev_idx)
        {
            player->field_2F = deadtng->index;
            player->field_31 = deadtng->field_9;
        }
    }
    return deadtng;
}

/**
 * Causes creature rebirth at its lair.
 * If lair isn't available, creature is reborn at dungeon heart.
 *
 * @param thing The creature to be reborn.
 */
void creature_rebirth_at_lair(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *lairtng;
    struct Dungeon *dungeon;
    cctrl = creature_control_get_from_thing(thing);
    lairtng = thing_get(cctrl->lairtng_idx);
    if (thing_is_invalid(lairtng))
    {
        dungeon = get_dungeon(thing->owner);
        lairtng = thing_get(dungeon->dnheart_idx);
    }
    if (cctrl->explevel > 0)
        set_creature_level(thing, cctrl->explevel-1);
    thing->health = cctrl->max_health;
    if (thing_is_invalid(lairtng))
        return;
    create_effect(&thing->mappos, 17, thing->owner);
    move_thing_in_map(thing, &lairtng->mappos);
    create_effect(&lairtng->mappos, 17, thing->owner);
}

void throw_out_gold(struct Thing *thing)
{
  struct Thing *gldtng;
  long angle,radius,delta;
  long x,y;
  long i;
  for (i = thing->long_13; i > 0; i -= delta)
  {
    gldtng = create_object(&thing->mappos, 6, game.neutral_player_num, -1);
    if (thing_is_invalid(gldtng))
        break;
    angle = ACTION_RANDOM(ANGLE_TRIGL_PERIOD);
    radius = ACTION_RANDOM(128);
    x = (radius * LbSinL(angle)) / 256;
    y = (radius * LbCosL(angle)) / 256;
    gldtng->pos_32.x.val += x/256;
    gldtng->pos_32.y.val -= y/256;
    gldtng->pos_32.z.val += ACTION_RANDOM(64) + 96;
    gldtng->field_1 |= 0x04;
    if (i < 400)
        delta = i;
    else
        delta = 400;
    gldtng->long_13 = delta;
  }
}

void thing_death_normal(struct Thing *thing)
{
    struct Thing *deadtng;
    struct Coord3d memaccl;
    long memp1;
    memp1 = thing->field_52;
    memaccl.x.val = thing->pos_2C.x.val;
    memaccl.y.val = thing->pos_2C.y.val;
    memaccl.z.val = thing->pos_2C.z.val;
    deadtng = destroy_creature_and_create_corpse(thing, 1);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Cannot create dead thing");
        return;
    }
    deadtng->field_52 = memp1;
    deadtng->pos_2C.x.val = memaccl.x.val;
    deadtng->pos_2C.y.val = memaccl.y.val;
    deadtng->pos_2C.z.val = memaccl.z.val;
}

void thing_death_flesh_explosion(struct Thing *thing)
{
    struct Thing *deadtng;
    struct Coord3d pos;
    struct Coord3d memaccl;
    long memp1;
    long i;
    //_DK_thing_death_flesh_explosion(thing);return;
    memp1 = thing->field_52;
    memaccl.x.val = thing->pos_2C.x.val;
    memaccl.y.val = thing->pos_2C.y.val;
    memaccl.z.val = thing->pos_2C.z.val;
    for (i = 0; i <= thing->field_58; i+=64)
    {
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = thing->mappos.z.val+i;
        create_effect(&pos, 9, thing->owner);
    }
    deadtng = destroy_creature_and_create_corpse(thing, 2);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Cannot create dead thing");
        return;
    }
    deadtng->field_52 = memp1;
    deadtng->pos_2C.x.val = memaccl.x.val;
    deadtng->pos_2C.y.val = memaccl.y.val;
    deadtng->pos_2C.z.val = memaccl.z.val;
    thing_play_sample(deadtng, 47, 100, 0, 3, 0, 4, 256);
}

void thing_death_gas_and_flesh_explosion(struct Thing *thing)
{
    struct Thing *deadtng;
    struct Coord3d pos;
    struct Coord3d memaccl;
    long memp1;
    long i;
    //_DK_thing_death_gas_and_flesh_explosion(thing);return;
    memp1 = thing->field_52;
    memaccl.x.val = thing->pos_2C.x.val;
    memaccl.y.val = thing->pos_2C.y.val;
    memaccl.z.val = thing->pos_2C.z.val;
    for (i = 0; i <= thing->field_58; i+=64)
    {
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = thing->mappos.z.val+i;
        create_effect(&pos, 9, thing->owner);
    }
    i = (thing->field_58 >> 1);
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val+i;
    create_effect(&pos, 13, thing->owner);
    deadtng = destroy_creature_and_create_corpse(thing, 2);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Cannot create dead thing");
        return;
    }
    deadtng->field_52 = memp1;
    deadtng->pos_2C.x.val = memaccl.x.val;
    deadtng->pos_2C.y.val = memaccl.y.val;
    deadtng->pos_2C.z.val = memaccl.z.val;
    thing_play_sample(deadtng, 47, 100, 0, 3, 0, 4, 256);
}

void thing_death_smoke_explosion(struct Thing *thing)
{
    struct Thing *deadtng;
    struct Coord3d pos;
    struct Coord3d memaccl;
    long memp1;
    long i;
    //_DK_thing_death_smoke_explosion(thing);return;
    memp1 = thing->field_52;
    memaccl.x.val = thing->pos_2C.x.val;
    memaccl.y.val = thing->pos_2C.y.val;
    memaccl.z.val = thing->pos_2C.z.val;
    i = (thing->field_58 >> 1);
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val+i;
    create_effect(&pos, 16, thing->owner);
    deadtng = destroy_creature_and_create_corpse(thing, 2);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Cannot create dead thing");
        return;
    }
    deadtng->field_52 = memp1;
    deadtng->pos_2C.x.val = memaccl.x.val;
    deadtng->pos_2C.y.val = memaccl.y.val;
    deadtng->pos_2C.z.val = memaccl.z.val;
    thing_play_sample(deadtng, 47, 100, 0, 3, 0, 4, 256);
}

void thing_death_ice_explosion(struct Thing *thing)
{
    struct Thing *deadtng;
    struct Coord3d pos;
    struct Coord3d memaccl;
    long memp1;
    long i;
    //_DK_thing_death_ice_explosion(thing);return;
    memp1 = thing->field_52;
    memaccl.x.val = thing->pos_2C.x.val;
    memaccl.y.val = thing->pos_2C.y.val;
    memaccl.z.val = thing->pos_2C.z.val;
    for (i = 0; i <= thing->field_58; i+=64)
    {
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = thing->mappos.z.val+i;
        create_effect(&pos, 24, thing->owner);
    }
    deadtng = destroy_creature_and_create_corpse(thing, 2);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Cannot create dead thing");
        return;
    }
    deadtng->field_52 = memp1;
    deadtng->pos_2C.x.val = memaccl.x.val;
    deadtng->pos_2C.y.val = memaccl.y.val;
    deadtng->pos_2C.z.val = memaccl.z.val;
    thing_play_sample(deadtng, 47, 100, 0, 3, 0, 4, 256);
}

void creature_death_as_nature_intended(struct Thing *thing)
{
    long i;
    i = creatures[thing->model%CREATURE_TYPES_COUNT].field_4[0];
    switch (i)
    {
    case 1:
        thing_death_normal(thing);
        break;
    case 2:
        thing_death_flesh_explosion(thing);
        break;
    case 3:
        thing_death_gas_and_flesh_explosion(thing);
        break;
    case 4:
        thing_death_smoke_explosion(thing);
        break;
    case 5:
        thing_death_ice_explosion(thing);
        break;
    default:
        WARNLOG("Unexpected creature death cause %ld",i);
        break;
    }
}

/**
 * Removes given index in things from given StructureList.
 * Returns amount of items updated.
 * TODO figure out what this index is, then rename and move this function.
 */
unsigned long remove_thing_from_field1D_in_list(struct StructureList *list,long remove_idx)
{
  struct Thing *thing;
  unsigned long n;
  unsigned long k;
  int i;
  SYNCDBG(18,"Starting");
  n = 0;
  k = 0;
  i = list->index;
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->next_of_class;
    // Per-thing code
    if (thing->field_1D == remove_idx)
    {
        thing->field_1D = thing->index;
        n++;
    }
    // Per-thing code ends
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
  return n;
}

void cause_creature_death(struct Thing *thing, unsigned char a2)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    long crmodel;
    TbBool simple_death;

    //_DK_cause_creature_death(thing, a2); return;

    cctrl = creature_control_get_from_thing(thing);
    anger_set_creature_anger_all_types(thing, 0);
    throw_out_gold(thing);
    remove_thing_from_field1D_in_list(&game.thing_lists[1],thing->index);

    crmodel = thing->model;
    crstat = creature_stats_get_from_thing(thing);
    if ( a2 )
    {
        if ((game.flags_cd & 0x08) != 0)
            add_creature_to_pool(crmodel, 1, 1);
        delete_thing_structure(thing, 0);
        return;
    }
    if ((crstat->rebirth) && (cctrl->lairtng_idx > 0)
     && (crstat->rebirth-1 <= cctrl->explevel) )
    {
        creature_rebirth_at_lair(thing);
        return;
    }

    //TODO check if this condition is right
    if (censorship_enabled())
        simple_death = crstat->bleeds;
    else
        simple_death = (crstat->bleeds) && ((get_creature_model_flags(thing) & MF_IsEvil) != 0);

    if (simple_death)
    {
        if ((game.flags_cd & 0x08) != 0)
            add_creature_to_pool(crmodel, 1, 1);
        creature_death_as_nature_intended(thing);
    } else
    if ((cctrl->field_AB & 0x02) != 0)
    {
        if ((game.flags_cd & 0x08) != 0)
            add_creature_to_pool(crmodel, 1, 1);
        thing_death_ice_explosion(thing);
    } else
    if ((cctrl->field_1D3 == 2) || (cctrl->field_1D3 == 24))
    {
        if ((game.flags_cd & 0x08) != 0)
            add_creature_to_pool(crmodel, 1, 1);
        thing_death_flesh_explosion(thing);
    } else
    {
        if ((game.flags_cd & 0x08) != 0)
            add_creature_to_pool(crmodel, 1, 1);
        creature_death_as_nature_intended(thing);
    }
}

long remove_all_traces_of_combat(struct Thing *thing)
{
  return _DK_remove_all_traces_of_combat(thing);
}

void prepare_to_controlled_creature_death(struct Thing *thing)
{
  struct PlayerInfo *player;
  player = get_player(thing->owner);
  leave_creature_as_controller(player, thing);
  player->field_43E = 0;
  if (player->id_number == thing->owner)
    setup_eye_lens(0);
  set_camera_zoom(player->acamera, player->field_4B6);
  if (player->id_number == thing->owner)
  {
    turn_off_all_window_menus();
    turn_off_menu(31);
    turn_off_menu(35);
    turn_off_menu(32);
    turn_on_main_panel_menu();
    set_flag_byte(&game.numfield_C,0x40,(game.numfield_C & 0x20) != 0);
  }
  light_turn_light_on(player->field_460);
}

TbBool kill_creature(struct Thing *thing, struct Thing *killertng, char killer_plyr_idx,
      unsigned char a4, TbBool died_in_battle, unsigned char a6)
{
  struct CreatureControl *cctrl;
  struct CreatureControl *cctrlgrp;
  struct CreatureStats *crstat;
  struct Dungeon *dungeon;
  struct Dungeon *killerdngn;
  long i,k;
  SYNCDBG(18,"Starting");
  //TODO check if invalid dead body can happen with original function
  //return _DK_kill_creature(thing, killertng, killer_plyr_idx, a4, died_in_battle, a6);
  dungeon = NULL;
  cctrl = creature_control_get_from_thing(thing);
  cleanup_current_thing_state(thing);
  remove_all_traces_of_combat(thing);
  if ((cctrl->field_7A & 0xFFF) != 0)
    remove_creature_from_group(thing);
  if (thing->owner != game.neutral_player_num)
    dungeon = get_players_num_dungeon(thing->owner);
  if (!thing_is_invalid(killertng))
  {
      if (killertng->owner == game.neutral_player_num)
          died_in_battle = 0;
  }
  if (killer_plyr_idx == game.neutral_player_num)
    died_in_battle = 0;
  remove_events_thing_is_attached_to(thing);
  if (dungeon != NULL)
  {
    update_dead_creatures_list(dungeon, thing);
    if (died_in_battle)
    {
      dungeon->battles_lost++;
    }
  }

  if (!creature_control_invalid(cctrl))
  {
    if ((cctrl->spell_flags & CSF_Armour) != 0)
    {
      set_flag_byte(&cctrl->spell_flags, CSF_Armour, false);
      for (i=0; i < 3; i++)
      {
        k = cctrl->field_2B3[i];
        if (k != 0)
        {
          thing = thing_get(k);
          delete_thing_structure(thing, 0);
          cctrl->field_2B3[i] = 0;
        }
      }
    }
    if ((cctrl->field_AD & 0x01) != 0)
    {
      cctrl->field_AD &= 0xFE;
      for (i=0; i < 3; i++)
      {
        k = cctrl->field_2B9[i];
        if (k != 0)
        {
          thing = thing_get(k);
          delete_thing_structure(thing, 0);
          cctrl->field_2B9[i] = 0;
        }
      }
    }
  }
  update_kills_counters(thing, killertng, killer_plyr_idx, died_in_battle);
  if (thing_is_invalid(killertng) || (killertng->owner == game.neutral_player_num) || (killer_plyr_idx == game.neutral_player_num) || (dungeon == NULL))
  {
    if ((a4) && ((thing->field_0 & 0x20) != 0))
    {
      prepare_to_controlled_creature_death(thing);
    }
    cause_creature_death(thing, a4);
    return true;
  }
  // Now we are sure that killertng and dungeon pointers are correct
  if (thing->owner == killertng->owner)
  {
    if ((get_creature_model_flags(thing) & MF_IsDiptera) && (get_creature_model_flags(killertng) & MF_IsArachnid))
    {
      dungeon->lvstats.flies_killed_by_spiders++;
    }
  }
  cctrlgrp = creature_control_get_from_thing(killertng);
  if (!creature_control_invalid(cctrlgrp))
    cctrlgrp->field_C2++;
  if (is_my_player_number(thing->owner))
  {
    output_message(11, 40, 1);
  } else
  if (is_my_player_number(killertng->owner))
  {
    output_message(12, 40, 1);
  }
  if (game.field_14E496 == killertng->owner)
  {
    killerdngn = get_players_num_dungeon(killertng->owner);
    if ((killerdngn->creature_tendencies & 0x01) != 0)
      ERRORLOG("How can hero have tend to imprison");
  }
  crstat = creature_stats_get_from_thing(killertng);
  anger_apply_anger_to_creature(killertng, crstat->annoy_win_battle, 4, 1);
  if (!creature_control_invalid(cctrlgrp) && died_in_battle)
    cctrlgrp->byte_9A++;
  if (dungeon != NULL)
    dungeon->hates_player[killertng->owner] += game.fight_hate_kill_value;
  SYNCDBG(18,"Almost finished");
  killerdngn = get_players_num_dungeon(killertng->owner);
  if ((a6) || (killerdngn->room_kind[4] == 0)
    || (killerdngn->creature_tendencies & 0x01) == 0)
  {
    if (a4 == 0)
    {
      cause_creature_death(thing, a4);
      return true;
    }
  }
  if (a4)
  {
    if ((thing->field_0 & 0x20) != 0)
      prepare_to_controlled_creature_death(thing);
    cause_creature_death(thing, a4);
    return true;
  }
  clear_creature_instance(thing);
  thing->field_7 = CrSt_CreatureUnconscious;
  cctrl = creature_control_get_from_thing(thing);
  cctrl->flgfield_1 |= 0x04;
  cctrl->flgfield_1 |= 0x02;
  cctrl->field_280 = 2000;
  thing->health = 1;
  return true;
}

void process_creature_standing_on_corpses_at(struct Thing *thing, struct Coord3d *pos)
{
  _DK_process_creature_standing_on_corpses_at(thing, pos);
}

/**
 * Calculates damage made by a creature by hand (using strength).
 */
long calculate_melee_damage(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  cctrl = creature_control_get_from_thing(thing);
  crstat = creature_stats_get_from_thing(thing);
  return compute_creature_attack_damage(crstat->strength, crstat->luck, cctrl->explevel);
}

/**
 * Calculates damage made by a creature using specific shot type.
 */
long calculate_shot_damage(struct Thing *thing,long shot_kind)
{
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  struct ShotStats *shotstat;
  shotstat = &shot_stats[shot_kind];
  cctrl = creature_control_get_from_thing(thing);
  crstat = creature_stats_get_from_thing(thing);
  return compute_creature_attack_damage(shotstat->damage, crstat->luck, cctrl->explevel);
}

void creature_fire_shot(struct Thing *firing,struct  Thing *target, unsigned short shot_kind, char a2, unsigned char a3)
{
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  struct ShotStats *shotstat;
  struct Coord3d pos1;
  struct Coord3d pos2;
  struct ComponentVector cvect;
  struct Thing *shot;
  struct Thing *tmptng;
  short angle_xy,angle_yz;
  long damage;
  long target_idx,i;
  TbBool flag1;
  //_DK_creature_fire_shot(firing,target, a1, a2, a3); return;
  cctrl = creature_control_get_from_thing(firing);
  crstat = creature_stats_get_from_thing(firing);
  shotstat = &shot_stats[shot_kind];
  flag1 = false;
  // Prepare source position
  pos1.x.val = firing->mappos.x.val;
  pos1.y.val = firing->mappos.y.val;
  pos1.z.val = firing->mappos.z.val;
  pos1.x.val += (cctrl->field_2C1 * LbSinL(firing->field_52+512) >> 16);
  pos1.y.val -= (cctrl->field_2C1 * LbCosL(firing->field_52+512) >> 8) >> 8;
  pos1.x.val += (cctrl->field_2C3 * LbSinL(firing->field_52) >> 16);
  pos1.y.val -= (cctrl->field_2C3 * LbCosL(firing->field_52) >> 8) >> 8;
  pos1.z.val += (cctrl->field_2C5);
  // Compute launch angles
  if (thing_is_invalid(target))
  {
    angle_xy = firing->field_52;
    angle_yz = firing->field_54;
  } else
  {
    pos2.x.val = target->mappos.x.val;
    pos2.y.val = target->mappos.y.val;
    pos2.z.val = target->mappos.z.val;
    pos2.z.val += (target->field_58 >> 1);
    if (( shotstat->field_48 ) && (target->class_id != 9))
    {
      flag1 = true;
      pos1.z.val = pos2.z.val;
    }
    angle_xy = get_angle_xy_to(&pos1, &pos2);
    angle_yz = get_angle_yz_to(&pos1, &pos2);
  }
  // Compute shot damage
  if ( shotstat->field_48 )
  {
    damage = calculate_melee_damage(firing);
  } else
  {
    damage = calculate_shot_damage(firing,shot_kind);
  }
  shot = NULL;
  target_idx = 0;
  // Set target index for navigating shots
  if (shot_kind == 6)
  {
    if (!thing_is_invalid(target))
      target_idx = target->index;
  }
  switch ( shot_kind )
  {
    case 4:
    case 12:
      if ((thing_is_invalid(target)) || (get_2d_distance(&firing->mappos, &pos2) > 5120))
      {
          project_point_to_wall_on_angle(&pos1, &pos2, firing->field_52, firing->field_54, 256, 20);
      }
      shot = create_thing(&pos2, 2, shot_kind, firing->owner, -1);
      if (thing_is_invalid(shot))
        return;
      if (shot_kind == 12)
        draw_lightning(&pos1, &pos2, 96, 93);
      else
        draw_lightning(&pos1, &pos2, 96, 60);
      shot->health = shotstat->health;
      shot->word_14 = shotstat->damage;
      shot->field_1D = firing->index;
      break;
    case 7:
      if ((thing_is_invalid(target)) || (get_2d_distance(&firing->mappos, &pos2) > 768))
        project_point_to_wall_on_angle(&pos1, &pos2, firing->field_52, firing->field_54, 256, 4);
      shot = create_thing(&pos2, 2, shot_kind, firing->owner, -1);
      if (thing_is_invalid(shot))
        return;
      draw_flame_breath(&pos1, &pos2, 96, 2);
      shot->health = shotstat->health;
      shot->word_14 = shotstat->damage;
      shot->field_1D = firing->index;
      break;
    case 13:
      for (i=0; i < 32; i++)
      {
        tmptng = create_thing(&pos1, 2, shot_kind, firing->owner, -1);
        if (thing_is_invalid(tmptng))
          break;
        shot = tmptng;
        shot->byte_16 = a3;
        shot->field_52 = (angle_xy + ACTION_RANDOM(101) - 50) & 0x7FF;
        shot->field_54 = (angle_yz + ACTION_RANDOM(101) - 50) & 0x7FF;
        angles_to_vector(shot->field_52, shot->field_54, shotstat->speed, &cvect);
        shot->pos_32.x.val += cvect.x;
        shot->pos_32.y.val += cvect.y;
        shot->pos_32.z.val += cvect.z;
        shot->field_1 |= 0x04;
        shot->word_14 = damage;
        shot->health = shotstat->health;
        shot->field_1D = firing->index;
      }
      break;
    default:
      shot = create_thing(&pos1, 2, shot_kind, firing->owner, -1);
      if (thing_is_invalid(shot))
        return;
      shot->field_52 = angle_xy;
      shot->field_54 = angle_yz;
      angles_to_vector(shot->field_52, shot->field_54, shotstat->speed, &cvect);
      shot->pos_32.x.val += cvect.x;
      shot->pos_32.y.val += cvect.y;
      shot->pos_32.z.val += cvect.z;
      shot->field_1 |= 0x04;
      shot->word_14 = damage;
      shot->health = shotstat->health;
      shot->field_1D = firing->index;
      shot->word_17 = target_idx;
      shot->byte_13 = compute_creature_max_dexterity(crstat->dexterity,cctrl->explevel);
      break;
  }
  if (!thing_is_invalid(shot))
  {
#if (BFDEBUG_LEVEL > 0)
    damage = shot->word_14;
    // Special debug code that shows amount of damage the shot will make
    if ((start_params.debug_flags & DFlg_ShotsDamage) != 0)
        create_price_effect(&pos1, my_player_number, damage);
    if ((damage < 0) || (damage > 2000))
    {
      WARNLOG("Shot of type %d carries %d damage",(int)shot_kind,(int)damage);
    }
#endif
    shot->byte_16 = a3;
    if (shotstat->firing_sound > 0)
    {
      thing_play_sample(firing, shotstat->firing_sound + UNSYNC_RANDOM(shotstat->firing_sound_variants),
          100, 0, 3, 0, 3, 256);
    }
    if (shotstat->shot_sound > 0)
    {
      thing_play_sample(shot, shotstat->shot_sound, 100, 0, 3, 0, shotstat->field_20, 256);
    }
    set_flag_byte(&shot->field_25,0x10,flag1);
  }
}

void set_creature_level(struct Thing *thing, long nlvl)
{
  //_DK_set_creature_level(thing, nlvl); return;
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  struct Dungeon *dungeon;
  long old_max_health,max_health;
  int i,k;
  crstat = creature_stats_get_from_thing(thing);
  cctrl = creature_control_get_from_thing(thing);
  if (creature_control_invalid(cctrl))
  {
      ERRORLOG("Creature has no control");
      return;
  }
  if (nlvl > CREATURE_MAX_LEVEL-1)
    nlvl = CREATURE_MAX_LEVEL-1;
  if (nlvl < 0)
    nlvl = 0;
  old_max_health = compute_creature_max_health(crstat->health,cctrl->explevel);
  cctrl->explevel = nlvl;
  max_health = compute_creature_max_health(crstat->health,cctrl->explevel);
  cctrl->max_health = max_health;
  if ((cctrl->field_AD & 0x02) != 0)
    thing->field_46 = 300;
  else
    thing->field_46 = saturate_set_signed( 300 + (300*(unsigned long)(cctrl->explevel)) / 20, 16);
  if (old_max_health > 0)
      thing->health = saturate_set_signed( (thing->health*max_health)/old_max_health, 16);
  else
      thing->health = -1;
  for (i=0; i < 10; i++)
  {
    k = crstat->instance_spell[i];
    if (k > 0)
    {
      if (crstat->instance_level[i] <= cctrl->explevel+1)
        cctrl->instances[k] = 1;
    }
  }
  if (thing->owner != game.neutral_player_num)
  {
    dungeon = get_dungeon(thing->owner);
    dungeon->score += game.creature_scores[thing->model%CREATURE_TYPES_COUNT].value[cctrl->explevel%CREATURE_MAX_LEVEL];
  }
}

void init_creature_level(struct Thing *thing, long nlev)
{
    struct CreatureControl *cctrl;
    //_DK_init_creature_level(thing,nlev);
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Creature has no control");
        return;
    }
    set_creature_level(thing, nlev);
    thing->health = cctrl->max_health;
}

long creature_instance_has_reset(struct Thing *thing, long a2)
{
  return _DK_creature_instance_has_reset(thing, a2);
}

long get_human_controlled_creature_target(struct Thing *thing, long a2)
{
  return _DK_get_human_controlled_creature_target(thing, a2);
}

void get_creature_instance_times(struct Thing *thing, long inst_idx, long *ritime, long *raitime)
{
    struct InstanceInfo *inst_inf;
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    long itime,aitime;
    cctrl = creature_control_get_from_thing(thing);
    inst_inf = creature_instance_info_get(inst_idx);
    if ((thing->field_0 & 0x20) != 0)
    {
        itime = inst_inf->fp_time;
        aitime = inst_inf->fp_action_time;
    } else
    {
        itime = inst_inf->time;
        aitime = inst_inf->action_time;
    }
    if ((cctrl->spell_flags & 0x01) != 0)
    {
        aitime *= 2;
        itime *= 2;
    }
    if ((cctrl->spell_flags & 0x02) != 0)
    {
        aitime /= 2;
        itime /= 2;
    } else
    if (cctrl->field_21)
    {
        aitime = 3 * aitime / 4;
        itime = 3 * itime / 4;
    } else
    if (game.neutral_player_num != thing->owner)
    {
        dungeon = get_dungeon(thing->owner);
        if (dungeon->field_888)
        {
            aitime -= aitime / 4;
            itime -= itime / 4;
        }
    }
    if (aitime <= 1)
        aitime = 1;
    if (itime <= 1)
        itime = 1;
    *ritime = itime;
    *raitime = aitime;
}

void set_creature_instance(struct Thing *thing, long inst_idx, long a2, long a3, struct Coord3d *pos)
{
    struct InstanceInfo *inst_inf;
    struct CreatureControl *cctrl;
    long i;
    long itime,aitime;
    if (inst_idx == 0)
        return;
    cctrl = creature_control_get_from_thing(thing);
    inst_inf = creature_instance_info_get(inst_idx);
    if (creature_instance_info_invalid(inst_inf) || (inst_inf->time == -1))
    {
        ERRORLOG("Negative instance");
        return;
    }
    //_DK_set_creature_instance(thing, inst_idx, a2, a3, pos);
    if (inst_inf->force_visibility)
    {
        i = cctrl->field_AF;
        if (i <= inst_inf->force_visibility)
          i = inst_inf->force_visibility;
        cctrl->field_AF = i;
    }
    get_creature_instance_times(thing, inst_idx, &itime, &aitime);
    if ((cctrl->field_D2 > 0) && (cctrl->field_D2 == inst_idx))
    {
        if (inst_inf->field_1A)
        {
            cctrl->field_D3 = 1;
            return;
        }
    }
    cctrl->field_D2 = inst_idx;
    cctrl->field_DA = a3;
    cctrl->field_D4 = 0;
    cctrl->field_D8 = itime;
    cctrl->field_D6 = aitime;
    i = get_creature_breed_graphics(thing->model,inst_inf->graphics_idx);
    cctrl->field_1CE = get_lifespan_of_animation(i, 1) / itime;
    if (pos != NULL)
    {
      cctrl->target_x = (pos->x.val >> 8);
      cctrl->target_y = (pos->y.val >> 8);
    } else
    {
      cctrl->target_x = 0;
      cctrl->target_y = 0;
    }
}

unsigned short find_next_annoyed_creature(unsigned char a1, unsigned short a2)
{
  return _DK_find_next_annoyed_creature(a1, a2);
}

void draw_creature_view(struct Thing *thing)
{
  struct TbGraphicsWindow grwnd;
  struct PlayerInfo *player;
  long grscr_w,grscr_h;
  unsigned char *wscr_cp;
  unsigned char *scrmem;
  //_DK_draw_creature_view(thing); return;

  // If no eye lens required - just draw on the screen, directly
  player = get_my_player();
  if (((game.flags_cd & MFlg_EyeLensReady) == 0) || (eye_lens_memory == NULL) || (game.numfield_1B == 0))
  {
    engine(&player->cameras[1]);
    return;
  }
  // So there is an eye lens - we have to put a buffer in place of screen,
  // draw on that buffer, an then copy it to screen applying lens effect.
  scrmem = eye_lens_spare_screen_memory;
  // Store previous graphics settings
  wscr_cp = lbDisplay.WScreen;
  grscr_w = lbDisplay.GraphicsScreenWidth;
  grscr_h = lbDisplay.GraphicsScreenHeight;
  LbScreenStoreGraphicsWindow(&grwnd);
  // Prepare new settings
  LbMemorySet(scrmem, 0, eye_lens_width*eye_lens_height*sizeof(TbPixel));
  lbDisplay.WScreen = scrmem;
  lbDisplay.GraphicsScreenHeight = eye_lens_height;
  lbDisplay.GraphicsScreenWidth = eye_lens_width;
  LbScreenSetGraphicsWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  // Draw on our buffer
  setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
  engine(&player->cameras[1]);
  // Restore original graphics settings
  lbDisplay.WScreen = wscr_cp;
  lbDisplay.GraphicsScreenWidth = grscr_w;
  lbDisplay.GraphicsScreenHeight = grscr_h;
  LbScreenLoadGraphicsWindow(&grwnd);
  // Draw the buffer on real screen
  setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
  draw_lens_effect(lbDisplay.WScreen, lbDisplay.GraphicsScreenWidth, scrmem, MyScreenWidth/pixel_size,
      MyScreenWidth/pixel_size, MyScreenHeight/pixel_size, game.numfield_1B);
}

struct Thing *get_creature_near(unsigned short pos_x, unsigned short pos_y)
{
  return _DK_get_creature_near(pos_x, pos_y);
}

struct Thing *get_creature_near_with_filter(unsigned short pos_x, unsigned short pos_y, Thing_Filter filter, FilterParam param)
{
  return _DK_get_creature_near_with_filter(pos_x, pos_y, filter, param);
}

struct Thing *get_creature_near_for_controlling(unsigned char a1, long a2, long a3)
{
  return _DK_get_creature_near_for_controlling(a1, a2, a3);
}

long remove_creature_from_group(struct Thing *thing)
{
  return _DK_remove_creature_from_group(thing);
}

long add_creature_to_group_as_leader(struct Thing *thing1, struct Thing *thing2)
{
  return _DK_add_creature_to_group_as_leader(thing1, thing2);
}

void set_first_creature(struct Thing *thing)
{
  _DK_set_first_creature(thing);
}

void remove_first_creature(struct Thing *thing)
{
  _DK_remove_first_creature(thing);
}

TbBool thing_is_creature(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return false;
  if (thing->class_id != TCls_Creature)
    return false;
  return true;
}

/** Returns if a thing is special digger creature.
 *
 * @param thing The thing to be checked.
 * @return True if the thing is creature and special digger, false otherwise.
 */
TbBool thing_is_creature_special_digger(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return false;
  if (thing->class_id != TCls_Creature)
    return false;
  return ((get_creature_model_flags(thing) & MF_IsSpecDigger) != 0);
}

void anger_set_creature_anger_all_types(struct Thing *thing, long a2)
{
    _DK_anger_set_creature_anger_all_types(thing, a2);
}
void change_creature_owner(struct Thing *thing, long nowner)
{
    _DK_change_creature_owner(thing, nowner);
}

struct Thing *create_creature(struct Coord3d *pos, unsigned short model, unsigned short owner)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct CreatureData *crdata;
    struct Thing *crtng;
    long i;
    crstat = creature_stats_get(model);
    if (!i_can_allocate_free_control_structure() || !i_can_allocate_free_thing_structure(1))
    {
        ERRORDBG(3,"Cannot create creature breed %d for player %d. There are too many creatures allocated.",(int)model,(int)owner);
        thing_create_errors++;
        return NULL;
    }
    crtng = allocate_free_thing_structure(1);
    cctrl = allocate_free_control_structure();
    crtng->ccontrol_idx = cctrl->index;
    crtng->class_id = 5;
    crtng->model = model;
    crtng->field_1D = crtng->index;
    crtng->mappos.x.val = pos->x.val;
    crtng->mappos.y.val = pos->y.val;
    crtng->mappos.z.val = pos->z.val;
    crtng->field_56 = crstat->size_xy;
    crtng->field_58 = crstat->size_yz;
    crtng->field_5A = crstat->thing_size_xy;
    crtng->field_5C = crstat->thing_size_yz;
    crtng->field_20 = 32;
    crtng->field_22 = 0;
    crtng->field_23 = 32;
    crtng->field_24 = 8;
    crtng->field_25 |= 0x08;
    crtng->owner = owner;
    crtng->field_52 = 0;
    crtng->field_54 = 0;
    cctrl->max_speed = calculate_correct_creature_maxspeed(crtng);
    cctrl->field_2C1 = creatures[model].field_9;
    cctrl->field_2C3 = creatures[model].field_B;
    cctrl->field_2C5 = creatures[model].field_D;
    i = get_creature_anim(crtng, 0);
    set_thing_draw(crtng, i, 256, 300, 0, 0, 2);
    cctrl->explevel = 1;
    crtng->health = crstat->health;
    cctrl->max_health = compute_creature_max_health(crstat->health,cctrl->explevel);
    crtng->owner = owner;
    crtng->mappos.x.val = pos->x.val;
    crtng->mappos.y.val = pos->y.val;
    crtng->mappos.z.val = pos->z.val;
    crtng->field_9 = game.play_gameturn;
    cctrl->field_286 = 17+ACTION_RANDOM(13);
    cctrl->field_287 = ACTION_RANDOM(7);
    if (game.field_14E496 == owner)
    {
      cctrl->sbyte_89 = -1;
      cctrl->byte_8C = 1;
    }
    cctrl->pos_288.x.val = crtng->mappos.x.val;
    cctrl->pos_288.y.val = crtng->mappos.y.val;
    cctrl->pos_288.z.val = crtng->mappos.z.val;
    cctrl->pos_288.z.val = get_thing_height_at(crtng, pos);
    cctrl->field_1D2 = -1;
    if (crstat->flying)
      crtng->field_25 |= 0x20;
    set_creature_level(crtng, 0);
    crtng->health = cctrl->max_health;
    add_thing_to_list(crtng, &game.thing_lists[0]);
    place_thing_in_mapwho(crtng);
    if (owner <= PLAYERS_COUNT)
      set_first_creature(crtng);
    set_start_state(crtng);
    if (crtng->owner != game.neutral_player_num)
    {
        struct Dungeon *dungeon;
        dungeon = get_dungeon(crtng->owner);
        if (!dungeon_invalid(dungeon))
        {
            dungeon->score += game.creature_scores[crtng->model].value[cctrl->explevel];
        }
    }
    crdata = creature_data_get(crtng->model);
    cctrl->field_1E8 = crdata->flags;
    return crtng;
}

TbBool creature_increase_level(struct Thing *thing)
{
  struct Dungeon *dungeon;
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  //_DK_creature_increase_level(thing);
  cctrl = creature_control_get_from_thing(thing);
  if (creature_control_invalid(cctrl))
  {
      ERRORLOG("Invalid creature control; no action");
      return false;
  }
  dungeon = get_dungeon(thing->owner);
  if (dungeon->creature_max_level[thing->model] > cctrl->explevel)
  {
    crstat = creature_stats_get_from_thing(thing);
    if ((cctrl->explevel < CREATURE_MAX_LEVEL-1) || (crstat->grow_up != 0))
    {
      cctrl->field_AD |= 0x40;
      return true;
    }
  }
  return false;
}

/**
 * Filter function for selecting most experienced and pickable creature.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with creature model and owner to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_most_experienced_and_pickable1(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    long nmaxim;
    cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    nmaxim = cctrl->explevel+1;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((param->model_id == -1) || (thing->model == param->model_id))
      && ((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0)
      && (thing->field_7 != CrSt_CreatureUnconscious) && (nmaxim > maximizer) )
    {
      if (can_thing_be_picked_up_by_player(thing, param->plyr_idx))
      {
        return nmaxim;
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function for selecting most experienced and "pickable2" creature.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with creature model and owner to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_most_experienced_and_pickable2(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    long nmaxim;
    cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    nmaxim = cctrl->explevel+1;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((param->model_id == -1) || (thing->model == param->model_id))
      && ((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0)
      && (thing->field_7 != CrSt_CreatureUnconscious) && (nmaxim > maximizer) )
    {
      if (can_thing_be_picked_up2_by_player(thing, param->plyr_idx))
      {
        return nmaxim;
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function for selecting least experienced and pickable creature.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with creature model and owner to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_least_experienced_and_pickable1(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    long nmaxim;
    cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    nmaxim = CREATURE_MAX_LEVEL-cctrl->explevel;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((param->model_id == -1) || (thing->model == param->model_id))
      && ((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0)
      && (thing->field_7 != CrSt_CreatureUnconscious) && (nmaxim > maximizer) )
    {
      if (can_thing_be_picked_up_by_player(thing, param->plyr_idx))
      {
        return nmaxim;
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function for selecting least experienced and "pickable2" creature.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with creature model and owner to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_least_experienced_and_pickable2(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    long nmaxim;
    cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    nmaxim = CREATURE_MAX_LEVEL-cctrl->explevel;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((param->model_id == -1) || (thing->model == param->model_id))
      && ((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0)
      && (thing->field_7 != CrSt_CreatureUnconscious) && (nmaxim > maximizer) )
    {
      if (can_thing_be_picked_up2_by_player(thing, param->plyr_idx))
      {
        return nmaxim;
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function for selecting first pickable creature with given GUI Job.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with creature model, owner and GUI Job to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_of_gui_job_and_pickable1(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((param->model_id == -1) || (thing->model == param->model_id))
      && ((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0)
      && ((param->num1 == -1) || (get_creature_gui_job(thing) == param->num1)) // job_idx
      && (thing->field_7 != CrSt_CreatureUnconscious) )
    {
      if (can_thing_be_picked_up_by_player(thing, param->plyr_idx))
      {
          // New 'maximizer' equal to MAX_LONG will stop the sweeping
          // and return this thing immediately.
          return LONG_MAX;
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function for selecting first 'pickable2' creature with given GUI state.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with creature model, owner and GUI state to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_of_gui_job_and_pickable2(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((param->model_id == -1) || (thing->model == param->model_id))
      && ((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0)
      && ((param->num1 == -1) || (get_creature_gui_job(thing) == param->num1))
      && (thing->field_7 != CrSt_CreatureUnconscious) )
    {
      if (can_thing_be_picked_up2_by_player(thing, param->plyr_idx))
      {
          // New 'maximizer' equal to MAX_LONG will stop the sweeping
          // and return this thing immediately.
          return LONG_MAX;
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

struct Thing *find_my_highest_level_creature_of_breed(long breed_idx, TbBool pick_check)
{
    Thing_Maximizer_Filter filter;
    struct CompoundFilterParam param;
    struct Dungeon *dungeon;
    struct Thing *thing;
    dungeon = get_players_num_dungeon(my_player_number);
    param.plyr_idx = my_player_number;
    param.class_id = TCls_Creature;
    param.model_id = breed_idx;
    if (pick_check)
    {
        filter = player_list_creature_filter_most_experienced_and_pickable1;
    } else
    {
        filter = player_list_creature_filter_most_experienced_and_pickable2;
    }
    if ((breed_idx == get_players_special_digger_breed(my_player_number)) || (breed_idx == -1))
    {
        thing = get_player_list_creature_with_filter(dungeon->worker_list_start, filter, &param);
    } else
    {
        thing = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    return thing;
}

struct Thing *find_my_lowest_level_creature_of_breed(long breed_idx, TbBool pick_check)
{
    Thing_Maximizer_Filter filter;
    struct CompoundFilterParam param;
    struct Dungeon *dungeon;
    struct Thing *thing;
    dungeon = get_players_num_dungeon(my_player_number);
    param.plyr_idx = my_player_number;
    param.num1 = breed_idx;
    if (pick_check)
    {
        filter = player_list_creature_filter_least_experienced_and_pickable1;
    } else
    {
        filter = player_list_creature_filter_least_experienced_and_pickable2;
    }
    if ((breed_idx == get_players_special_digger_breed(my_player_number)) || (breed_idx == -1))
    {
        thing = get_player_list_creature_with_filter(dungeon->worker_list_start, filter, &param);
    } else
    {
        thing = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    return thing;
}

struct Thing *find_my_first_creature_of_breed_and_gui_job(long breed_idx, long job_idx, TbBool pick_check)
{
    Thing_Maximizer_Filter filter;
    struct CompoundFilterParam param;
    struct Dungeon *dungeon;
    struct Thing *thing;
    SYNCDBG(5,"Searching for breed %ld, GUI job %ld",breed_idx,job_idx);
    dungeon = get_players_num_dungeon(my_player_number);
    param.plyr_idx = my_player_number;
    param.class_id = TCls_Creature;
    param.model_id = breed_idx;
    param.num1 = job_idx;
    if (pick_check)
    {
        filter = player_list_creature_filter_of_gui_job_and_pickable1;
    } else
    {
        filter = player_list_creature_filter_of_gui_job_and_pickable2;
    }
    if ((breed_idx == get_players_special_digger_breed(my_player_number)) || (breed_idx == -1))
    {
        thing = get_player_list_creature_with_filter(dungeon->worker_list_start, filter, &param);
    } else
    {
        thing = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    return thing;
}

struct Thing *find_my_next_creature_of_breed_and_gui_job(long breed_idx, long job_idx, unsigned char pick_flags)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Thing *thing;
    Thing_Maximizer_Filter filter;
    struct CompoundFilterParam param;
    long i;
    SYNCDBG(5,"Searching for breed %ld, GUI job %ld",breed_idx,job_idx);
    //return _DK_find_my_next_creature_of_breed_and_job(breed_idx, job_idx, (pick_flags & TPF_PickableCheck) != 0);
    thing = NULL;
    dungeon = get_my_dungeon();
    if (breed_idx != -1)
    {
      i = dungeon->selected_creatures_of_model[breed_idx];
      thing = thing_get(i);
      if (!thing_is_invalid(thing))
      {
        if ( ((thing->field_0 & 0x01) != 0) && (thing->class_id == TCls_Creature)
          && ((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0)
          && (thing->field_7 != CrSt_CreatureUnconscious) && is_my_player_number(thing->owner) )
        {
          dungeon->selected_creatures_of_model[breed_idx] = 0;
          thing = NULL;
        } else
        {
          cctrl = creature_control_get_from_thing(thing);
          thing = thing_get(cctrl->thing_idx);
        }
      }
    } else
    if (job_idx != -1)
    {
      i = dungeon->selected_creatures_of_gui_job[job_idx];
      thing = thing_get(i);
      if (!thing_is_invalid(thing))
      {
        if ( ((thing->field_0 & 0x01) != 0) && (thing->class_id == TCls_Creature)
          && ((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0)
          && (thing->field_7 != CrSt_CreatureUnconscious) && is_my_player_number(thing->owner)
          && (get_creature_gui_job(thing) == job_idx) )
        {
            cctrl = creature_control_get_from_thing(thing);
            thing = thing_get(cctrl->thing_idx);
        } else
        {
            dungeon->selected_creatures_of_gui_job[job_idx] = 0;
            thing = NULL;
        }
      }
    }
    if ((breed_idx != -1) && (job_idx == -1))
    {
        if ((pick_flags & TPF_ReverseOrder) != 0)
        {
            thing = find_my_lowest_level_creature_of_breed(breed_idx, (pick_flags & TPF_PickableCheck) != 0);
        } else
        {
            thing = find_my_highest_level_creature_of_breed(breed_idx, (pick_flags & TPF_PickableCheck) != 0);
        }
    } else
    if (!thing_is_invalid(thing))
    {
        param.plyr_idx = my_player_number;
        param.class_id = TCls_Creature;
        param.model_id = breed_idx;
        param.num1 = job_idx;
        if ((pick_flags & TPF_PickableCheck) != 0)
        {
            filter = player_list_creature_filter_of_gui_job_and_pickable1;
        } else
        {
            filter = player_list_creature_filter_of_gui_job_and_pickable2;
        }
        thing = get_player_list_creature_with_filter(thing->index, filter, &param);
    }
    if (thing_is_invalid(thing))
    {
        thing = find_my_first_creature_of_breed_and_gui_job(breed_idx, job_idx, (pick_flags & TPF_PickableCheck) != 0);
    }
    if (thing_is_invalid(thing))
    {
        return NULL;
    }
    if ((breed_idx != -1) && (thing->model != breed_idx))
    {
        ERRORLOG("Searched for breed %ld, but found %d.",breed_idx,(int)thing->model);
    }
    dungeon->selected_creatures_of_model[thing->model] = thing->index;
    dungeon->selected_creatures_of_gui_job[get_creature_gui_job(thing)] = thing->index;
    return thing;
}

struct Thing *pick_up_creature_of_breed_and_gui_job(long breed, long job_idx, long owner, unsigned char pick_flags)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    thing = find_my_next_creature_of_breed_and_gui_job(breed, job_idx, pick_flags);
    if (thing_is_invalid(thing))
    {
        SYNCDBG(2,"Can't find creature of breed %ld and GUI job %ld.",breed,job_idx);
        return INVALID_THING;
    }
    dungeon = get_dungeon(owner);
    if ((breed > 0) && (breed < CREATURE_TYPES_COUNT))
    {
        if ((job_idx == -1) || (dungeon->job_breeds_count[breed][job_idx & 0x03]))
        {
            set_players_packet_action(get_my_player(), 90, thing->index, 0, 0, 0);
        }
    } else
    {
        ERRORLOG("Creature breed %ld out of range.",breed);
    }
    return thing;
}

long player_list_creature_filter_needs_to_be_placed_in_room(const struct Thing *thing, MaxFilterParam param, long maximizer)
{
    struct Room *room;
    struct Computer2 *comp;
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    long i,k;
    SYNCDBG(19,"Starting");
    comp = (struct Computer2 *)(param->ptr1);
    dungeon = comp->dungeon;
    if (!can_thing_be_picked_up_by_player(thing, dungeon->field_E9F))
        return -1;
    if (creature_is_being_dropped(thing))
        return -1;
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);

    // If the creature is too angry to help it, then let it go
    if (dungeon->room_kind[RoK_ENTRANCE] > 0)
    {
        if (creature_is_doing_dungeon_improvements(thing) || anger_is_creature_livid(thing))
        {
          param->num2 = RoK_ENTRANCE;
          return LONG_MAX;
        }
    }

    // If it's angry but not furious, then should be placed in temple
    if ( anger_is_creature_angry(thing) )
    {
        // If already at temple, then don't do anything
        if (creature_is_doing_temple_activity(thing))
           return -1;
        if (dungeon->room_kind[RoK_TEMPLE] > 0)
        {
            param->num2 = RoK_TEMPLE;
            return LONG_MAX;
        }
    }

    // If the creature require healing, then drop it to lair
    i = compute_creature_max_health(crstat->health,cctrl->explevel);
    k = compute_value_8bpercentage(i,crstat->heal_threshold);
    if (cctrl->field_3)
    {
        if (thing->health >= k)
            return -1;
        // If already at lair, then don't do anything
        if (creature_is_doing_lair_activity(thing))
            return -1;
        if (dungeon->room_kind[RoK_LAIR] > 0)
        {
            param->num2 = RoK_LAIR;
            return LONG_MAX;
        }
        return -1;
    } else
    if (thing->health < k)
    {
        // If already at lair, then don't do anything
        if (creature_is_doing_lair_activity(thing))
            return -1;
        // don't force it to lair if it wants to eat or take salary
        if (creature_is_doing_garden_activity(thing) || creature_is_taking_salary_activity(thing))
            return -1;
        if (dungeon->room_kind[RoK_LAIR] > 0)
        {
            param->num2 = RoK_LAIR;
            return LONG_MAX;
        }
    }

    // If creature is hungry, place it at garden
    if ((crstat->hunger_rate != 0) && (cctrl->field_39 > crstat->hunger_rate))
    {
        // If already at garden, then don't do anything
        if (creature_is_doing_garden_activity(thing))
            return -1;
        if (dungeon->room_kind[RoK_GARDEN] > 0)
        {
            param->num2 = RoK_GARDEN;
            return LONG_MAX;
        }
    }

    // If creature wants salary, let it go get the gold
    if ( cctrl->field_3D[11] )
    {
        // If already taking salary, then don't do anything
        if (creature_is_taking_salary_activity(thing))
            return -1;
        if (dungeon->room_kind[RoK_TREASURE] > 0)
        {
            param->num2 = RoK_TREASURE;
            return LONG_MAX;
        }
    }

    // Get other rooms the creature may work in
    if (creature_state_is_unset(thing))
    {
        room = get_room_to_place_creature(comp, thing);
        if (!room_is_invalid(room))
        {
            param->num2 = room->kind;
            return LONG_MAX;
        }
    }
    return -1;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
