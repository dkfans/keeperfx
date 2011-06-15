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

#include <assert.h>

#include "thing_creature.h"
#include "globals.h"

#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_filelst.h"
#include "bflib_sprite.h"

#include "engine_lenses.h"
#include "config_creature.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "creature_instances.h"
#include "creature_graphics.h"
#include "config_lenses.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "thing_objects.h"
#include "thing_navigate.h"
#include "thing_shots.h"
#include "thing_corpses.h"
#include "thing_physics.h"
#include "lens_api.h"
#include "light_data.h"
#include "gui_topmsg.h"
#include "front_simple.h"
#include "frontend.h"
#include "gui_frontmenu.h"
#include "gui_soundmsgs.h"
#include "sounds.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
int creature_swap_idx[CREATURE_TYPES_COUNT];

struct Creatures creatures_NEW[] = {
  { 0,  0, 0, 0, 0, 0, 0, 0, 0, 0x0000, 1},
  {17, 34, 1, 0, 1, 0, 1, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 2, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 1, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 1, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 2, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 4, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 4, 0, 0, 0x0180, 1},
  { 1, 77, 1, 0, 1, 0, 2, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 1, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 1, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 1, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 4, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 5, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 3, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 4, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 1, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 6, 0, 0, 0x0226, 1},
  {17, 34, 1, 0, 1, 0, 6, 0, 0, 0x0100, 1},
  {17, 34, 1, 0, 1, 0, 6, 0, 0, 0x0080, 1},
  {17, 34, 1, 0, 1, 0, 6, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 1, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 4, 0, 0, 0x0180, 0},
  { 1, 77, 1, 0, 1, 0, 1, 0, 0, 0x0100, 1},
  {17, 34, 1, 0, 1, 0, 6, 0, 0, 0x0080, 1},
  {17, 34, 1, 0, 1, 0, 1, 0, 0, 0x0180, 1},
  {17, 34, 1, 0, 1, 0, 6, 0, 0, 0x0100, 1},
  {17, 34, 1, 0, 1, 0, 6, 0, 0, 0x0100, 1},
  {17, 34, 1, 0, 1, 1, 1, 0, 0, 0x0100, 1},
  {17, 34, 1, 0, 1, 0, 3, 0, 0, 0x0100, 1},
  {17, 34, 1, 0, 1, 0, 2, 0, 0, 0x0180, 1},
  { 0,  0, 1, 0, 1, 0, 1, 0, 0, 0x0000, 1},
};
/******************************************************************************/
DLLIMPORT struct Thing *_DK_find_my_next_creature_of_breed_and_job(long breed_idx, long job_idx, long a3);
DLLIMPORT void _DK_anger_set_creature_anger_all_types(struct Thing *thing, long a2);
DLLIMPORT void _DK_change_creature_owner(struct Thing *thing , char nowner);
DLLIMPORT long _DK_remove_all_traces_of_combat(struct Thing *thing);
DLLIMPORT void _DK_cause_creature_death(struct Thing *thing, unsigned char a2);
DLLIMPORT void _DK_apply_spell_effect_to_thing(struct Thing *thing, long spell_idx, long spell_lev);
DLLIMPORT void _DK_creature_cast_spell_at_thing(struct Thing *caster, struct Thing *target, long a3, long no_effects);
DLLIMPORT void _DK_creature_cast_spell(struct Thing *caster, long a2, long a3, long no_effects, long a5);
DLLIMPORT void _DK_set_first_creature(struct Thing *thing);
DLLIMPORT void _DK_remove_first_creature(struct Thing *thing);
DLLIMPORT struct Thing *_DK_get_creature_near(unsigned short pos_x, unsigned short pos_y);
DLLIMPORT struct Thing *_DK_get_creature_near_with_filter(unsigned short pos_x, unsigned short pos_y, Thing_Filter filter, long no_effects);
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
DLLIMPORT short _DK_kill_creature(struct Thing *thing, struct Thing *tngrp, char a1, unsigned char a2, unsigned char a3, unsigned char no_effects);
DLLIMPORT void _DK_update_creature_count(struct Thing *thing);
DLLIMPORT long _DK_process_creature_state(struct Thing *thing);
DLLIMPORT long _DK_move_creature(struct Thing *thing);
DLLIMPORT void _DK_init_creature_level(struct Thing *thing, long nlev);
DLLIMPORT long _DK_check_for_first_person_barrack_party(struct Thing *thing);
DLLIMPORT void _DK_terminate_thing_spell_effect(struct Thing *thing, long a2);
DLLIMPORT void _DK_creature_increase_level(struct Thing *thing);
DLLIMPORT void _DK_thing_death_flesh_explosion(struct Thing *thing);
DLLIMPORT void _DK_thing_death_gas_and_flesh_explosion(struct Thing *thing);
DLLIMPORT void _DK_thing_death_smoke_explosion(struct Thing *thing);
DLLIMPORT void _DK_thing_death_ice_explosion(struct Thing *thing);
DLLIMPORT long _DK_creature_is_group_leader(struct Thing *thing);
DLLIMPORT long _DK_update_creature_levels(struct Thing *thing);
DLLIMPORT long _DK_update_creature(struct Thing *thing);
/******************************************************************************/
TbBool thing_can_be_controlled_as_controller(struct Thing *thing)
{
    if (!thing_exists(thing))
        return false;
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
    crstat = creature_stats_get(get_players_special_digger_breed(player->id_number));
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
  player->controlled_thing_idx = thing->index;
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
    if (creature_is_group_member(thing))
      make_group_member_leader(thing);
  }
  memset(&ilght, 0, sizeof(struct InitLight));
  ilght.mappos.x.val = thing->mappos.x.val;
  ilght.mappos.y.val = thing->mappos.y.val;
  ilght.mappos.z.val = thing->mappos.z.val;
  ilght.field_3 = 1;
  ilght.field_2 = 36;
  ilght.field_0 = 2560;
  ilght.is_dynamic = 1;
  thing->light_id = light_create_light(&ilght);
  if (thing->light_id != 0) {
      light_set_light_never_cache(thing->light_id);
  } else {
    ERRORLOG("Cannot allocate light to new controlled thing");
  }
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
    ERRORLOG("The %s can't be controlled as passenger",
        thing_model_name(thing));
    return false;
  }
  if (is_my_player(player))
  {
    toggle_status_menu(0);
    turn_off_roaming_menus();
  }
  cam = player->acamera;
  player->controlled_thing_idx = thing->index;
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
    SYNCDBG(6,"Starting for %s",thing_model_name(thing));
    //_DK_load_swipe_graphic_for_creature(thing);

    i = creatures[thing->model%CREATURE_TYPES_COUNT].swipe_idx;
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
        ERRORLOG("Unable to load swipe graphics for %s",thing_model_name(thing));
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
    struct PlayerInfo *player;
    struct Dungeon * dungeon;
    struct Thing * heartng;
    int dist;
    int enemy_idx;

    SYNCDBG(17, "Starting");

    //return _DK_get_enemy_dungeon_heart_creature_can_see(thing);

    assert(DUNGEONS_COUNT == PLAYERS_COUNT);

    for (enemy_idx = 0; enemy_idx < DUNGEONS_COUNT; enemy_idx++)
    {
        if ( players_are_enemies(thing->owner, enemy_idx) )
        {
            player = get_player(enemy_idx);
            dungeon = get_players_dungeon(player);
            heartng = thing_get(dungeon->dnheart_idx);
            if (player_exists(player) && (!thing_is_invalid(heartng)))
            {
                dist = get_combat_distance(thing, heartng);
                if (creature_can_see_combat_path(thing, heartng, dist)) {
                    return heartng;
                }
            }
        }
    }

    return NULL;
}

long set_creature_object_combat(struct Thing *crthing, struct Thing *obthing)
{
  return _DK_set_creature_object_combat(crthing, obthing);
}

void set_creature_door_combat(struct Thing *crthing, struct Thing *obthing)
{
    SYNCDBG(18,"Starting");
    _DK_set_creature_door_combat(crthing, obthing);
    SYNCDBG(19,"Finished");
}

void food_eaten_by_creature(struct Thing *crthing, struct Thing *obthing)
{
  _DK_food_eaten_by_creature(crthing, obthing);
}

void anger_apply_anger_to_creature(struct Thing *thing, long anger, long a2, long a3)
{
  _DK_anger_apply_anger_to_creature(thing, anger, a2, a3);
}

void terminate_thing_spell_effect(struct Thing *thing, long spkind)
{
    _DK_terminate_thing_spell_effect(thing, spkind);
}

long get_free_spell_slot(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CastedSpellData *cspell;
    long ci,cval;
    long i,k;
    cctrl = creature_control_get_from_thing(thing);
    cval = LONG_MAX;
    ci = -1;
    for (i=0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        cspell = &cctrl->casted_spells[i];
        // If there's unused slot, return it immediately
        if (cspell->spkind == 0)
        {
            return i;
        }
        // Otherwise, select the one making minimum damage
        k = abs(cspell->field_1);
        if (k < cval)
        {
            cval = k;
            ci = i;
        }
    }
    // Terminate the min damage effect and return its slot index
    cspell = &cctrl->casted_spells[ci];
    terminate_thing_spell_effect(thing, cspell->spkind);
    for (i=0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        cspell = &cctrl->casted_spells[i];
        if (cspell->spkind == 0)
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
    struct MagicStats *magstat;
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
            cctrl->casted_spells[i].spkind = spell_idx;
            cctrl->casted_spells[i].field_1 = splconf->duration;
            cctrl->affected_by_spells |= CCSpl_Freeze;
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
            magstat = &game.magic_stats[PwrK_PROTECT];
            cctrl->casted_spells[i].spkind = spell_idx;
            cctrl->casted_spells[i].field_1 = magstat->power[spell_lev];
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
                  ntng->health = magstat->power[spell_lev] + 1;
                  ntng->word_13 = thing->index;
                  ntng->byte_15 = k;
                  ntng->field_52 = thing->field_52;
                  ntng->field_54 = thing->field_54;
                  angles_to_vector(ntng->field_52, ntng->field_54, 32, &cvect);
                  ntng->acceleration.x.val += cvect.x;
                  ntng->acceleration.y.val += cvect.y;
                  ntng->acceleration.z.val += cvect.z;
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
            cctrl->casted_spells[i].spkind = spell_idx;
            cctrl->casted_spells[i].field_1 = splconf->duration;
            cctrl->spell_flags |= CSF_Rebound;
        }
        break;
    case SplK_Heal:
        crstat = creature_stats_get_from_thing(thing);
        magstat = &game.magic_stats[PwrK_HEALCRTR];
        i = saturate_set_signed(thing->health + magstat->power[spell_lev],16);
        if (i < 0)
        {
          thing->health = 0;
        } else
        {
          k = compute_creature_max_health(crstat->health,cctrl->explevel);
          thing->health = min(i,k);
        }
        cctrl->field_2B0 = 7;
        cctrl->field_2AE = magstat->time;
        break;
    case SplK_Invisibility:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->casted_spells[i].spkind = spell_idx;
            magstat = &game.magic_stats[PwrK_CONCEAL];
            cctrl->casted_spells[i].field_1 = magstat->power[spell_lev];
            cctrl->spell_flags |= CSF_Conceal;
            cctrl->field_AF = 0;
        }
        break;
    case SplK_Teleport:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->casted_spells[i].spkind = spell_idx;
            cctrl->casted_spells[i].field_1 = splconf->duration;
            cctrl->affected_by_spells |= CCSpl_Teleport;
        }
        break;
    case SplK_Speed:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->casted_spells[i].spkind = spell_idx;
            magstat = &game.magic_stats[PwrK_SPEEDCRTR];
            cctrl->casted_spells[i].field_1 = magstat->power[spell_lev];
            cctrl->spell_flags |= CSF_Speed;
            cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        }
        break;
    case SplK_Slow:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->casted_spells[i].spkind = spell_idx;
            cctrl->casted_spells[i].field_1 = splconf->duration;
            cctrl->spell_flags |= CSF_Slow;
            cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        }
        break;
    case SplK_Fly:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->casted_spells[i].spkind = spell_idx;
            cctrl->casted_spells[i].field_1 = splconf->duration;
            cctrl->spell_flags |= CSF_Fly;
            thing->field_25 |= 0x20;
        }
        break;
    case SplK_Sight:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            cctrl->casted_spells[i].spkind = spell_idx;
            cctrl->casted_spells[i].field_1 = splconf->duration;
            cctrl->spell_flags |= CSF_Sight;
        }
        break;
    case SplK_Disease:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
          cctrl->casted_spells[i].spkind = spell_idx;
          magstat = &game.magic_stats[PwrK_DISEASE];
          cctrl->casted_spells[i].field_1 = magstat->power[spell_lev];
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
              ntng->health = magstat->power[spell_lev] + 1;
              ntng->word_13 = thing->index;
              ntng->byte_15 = k;
              ntng->field_52 = thing->field_52;
              ntng->field_54 = thing->field_54;
              angles_to_vector(ntng->field_52, ntng->field_54, 32, &cvect);
              ntng->acceleration.x.val += cvect.x;
              ntng->acceleration.y.val += cvect.y;
              ntng->acceleration.z.val += cvect.z;
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
            external_set_thing_state(thing, CrSt_CreatureChangeToChicken);
            cctrl->field_282 = 10;
            cctrl->field_AD |= 0x02;
            cctrl->casted_spells[i].spkind = spell_idx;
            magstat = &game.magic_stats[PwrK_CHICKEN];
            cctrl->casted_spells[i].field_1 = magstat->power[spell_lev];
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
    struct MagicStats *magstat;
    long i,k;
    cctrl = creature_control_get_from_thing(thing);
    if (spell_lev > SPELL_MAX_LEVEL)
        spell_lev = SPELL_MAX_LEVEL;
    // This pointer may be invalid if spell_idx is incorrect. But we're using it only when correct.
    splconf = &game.spells_config[spell_idx];
    switch (spell_idx)
    {
    case SplK_Freeze:
        cctrl->casted_spells[idx].field_1 = splconf->duration;
        creature_set_speed(thing, 0);
        break;
    case SplK_Armour:
        magstat = &game.magic_stats[PwrK_PROTECT];
        cctrl->casted_spells[idx].field_1 = magstat->power[spell_lev];
        break;
    case SplK_Rebound:
        cctrl->casted_spells[idx].field_1 = splconf->duration;
        break;
    case SplK_Heal:
        crstat = creature_stats_get_from_thing(thing);
        magstat = &game.magic_stats[PwrK_HEALCRTR];
        i = saturate_set_signed(thing->health + magstat->power[spell_lev],16);
        if (i < 0)
        {
          thing->health = 0;
        } else
        {
          k = compute_creature_max_health(crstat->health,cctrl->explevel);
          thing->health = min(i,k);
        }
        cctrl->field_2B0 = 7;
        cctrl->field_2AE = magstat->time;
        break;
    case SplK_Invisibility:
        magstat = &game.magic_stats[PwrK_CONCEAL];
        cctrl->casted_spells[idx].field_1 = magstat->power[spell_lev];
        break;
    case SplK_Teleport:
        cctrl->casted_spells[idx].field_1 = splconf->duration;
        break;
    case SplK_Speed:
        magstat = &game.magic_stats[PwrK_SPEEDCRTR];
        cctrl->casted_spells[idx].field_1 = magstat->power[spell_lev];
        break;
    case SplK_Slow:
        cctrl->casted_spells[idx].field_1 = splconf->duration;
        break;
    case SplK_Light:
        cctrl->casted_spells[idx].field_1 = splconf->duration;
        break;
    case SplK_Fly:
        cctrl->casted_spells[idx].field_1 = splconf->duration;
        break;
    case SplK_Sight:
        cctrl->casted_spells[idx].field_1 = splconf->duration;
        break;
    case SplK_Disease:
        magstat = &game.magic_stats[PwrK_DISEASE];
        cctrl->casted_spells[idx].field_1 = magstat->power[spell_lev];
        cctrl->field_B6 = thing->owner;
        break;
    case SplK_Chicken:
        external_set_thing_state(thing, CrSt_CreatureChangeToChicken);
        cctrl->field_282 = 10;
        magstat = &game.magic_stats[PwrK_CHICKEN];
        cctrl->casted_spells[idx].field_1 = magstat->power[spell_lev];
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
        if (cctrl->casted_spells[i].spkind == spell_idx)
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
  if (obthing->creature.gold_carried <= 0)
  {
    ERRORLOG("GoldPile had no gold so was deleted.");
    delete_thing_structure(obthing, 0);
    return false;
  }
  if (crthing->creature.gold_carried < crstat->gold_hold)
  {
    if (obthing->creature.gold_carried+crthing->creature.gold_carried > crstat->gold_hold)
    {
      i = crstat->gold_hold-crthing->creature.gold_carried;
      crthing->creature.gold_carried += i;
      obthing->creature.gold_carried -= i;
    } else
    {
      crthing->creature.gold_carried += obthing->creature.gold_carried;
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
    unsigned long model_flags;
    long x,y;
    long k;
    SYNCDBG(18,"Starting");
    //return _DK_process_creature_state(thing);
    cctrl = creature_control_get_from_thing(thing);
    model_flags = get_creature_model_flags(thing);
    process_person_moods_and_needs(thing);
    if (creature_available_for_combat_this_turn(thing))
    {
        if (!creature_look_for_combat(thing))
        {
          if ((!cctrl->field_3) && ((model_flags & MF_IsSpecDigger) == 0))
          {
            tgthing = get_enemy_dungeon_heart_creature_can_see(thing);
            if (!thing_is_invalid(tgthing))
              set_creature_object_combat(thing, tgthing);
          }
        }
    }
    if ((cctrl->field_3 & 0x10) == 0)
    {
        if ((cctrl->field_1D0) && ((cctrl->flgfield_1 & CCFlg_NoCompControl) == 0))
        {
            if ( can_change_from_state_to(thing, thing->active_state, CrSt_CreatureDoorCombat) )
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
    if (creature_is_group_member(thing))
    {
        if (!creature_is_group_leader(thing)) {
            process_obey_leader(thing);
        }
    }
    if ((thing->active_state < 1) || (thing->active_state >= CREATURE_STATES_COUNT))
    {
      ERRORLOG("The %s has illegal state[1], T=%d, S=%d, TCS=%d, reset", thing_model_name(thing), (int)thing->index, (int)thing->active_state, (int)thing->continue_state);
      set_start_state(thing);
    }
    // Creatures that are not special diggers will pick up any nearby gold or food
    if (((thing->field_25 & 0x20) == 0) && ((model_flags & MF_IsSpecDigger) == 0))
    {
        tgthing = find_interesting_object_laying_around_thing(thing);
        if (!thing_is_invalid(tgthing))
        {
            if (tgthing->model == 43)
            {
              crstat = creature_stats_get_from_thing(thing);
              if (tgthing->creature.gold_carried > 0)
              {
                  if (thing->creature.gold_carried < crstat->gold_hold)
                  {
                      if (crstat->gold_hold < tgthing->creature.gold_carried + thing->creature.gold_carried)
                      {
                          k = crstat->gold_hold - thing->creature.gold_carried;
                          thing->creature.gold_carried += k;
                          tgthing->creature.gold_carried -= k;
                      } else
                      {
                          thing->creature.gold_carried += tgthing->creature.gold_carried;
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
                if (!is_thing_passenger_controlled(tgthing)) {
                  food_eaten_by_creature(tgthing, thing);
                }
            }
        }
    }
    // Enable this to know which function hangs on update_creature.
    //TODO: rewrite state subfunctions so they won't hang
    //if (game.play_gameturn > 119800)
    SYNCDBG(18,"Executing state %d for %s index %d.",(int)thing->active_state,thing_model_name(thing),(int)thing->index);
    stati = get_thing_active_state_info(thing);
    if (stati->ofsfield_0 == NULL)
        return false;
    k = stati->ofsfield_0(thing);
    SYNCDBG(18,"Finished");
    if (k != -1)
        return false;
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
    create_effect(&thing->mappos, TngEff_Unknown17, thing->owner);
    move_thing_in_map(thing, &lairtng->mappos);
    create_effect(&lairtng->mappos, TngEff_Unknown17, thing->owner);
}

void throw_out_gold(struct Thing *thing)
{
    struct Thing *gldtng;
    long angle,radius,delta;
    long x,y;
    long i;
    for (i = thing->creature.gold_carried; i > 0; i -= delta)
    {
        gldtng = create_object(&thing->mappos, 6, game.neutral_player_num, -1);
        if (thing_is_invalid(gldtng))
            break;
        angle = ACTION_RANDOM(ANGLE_TRIGL_PERIOD);
        radius = ACTION_RANDOM(128);
        x = (radius * LbSinL(angle)) / 256;
        y = (radius * LbCosL(angle)) / 256;
        gldtng->acceleration.x.val += x/256;
        gldtng->acceleration.y.val -= y/256;
        gldtng->acceleration.z.val += ACTION_RANDOM(64) + 96;
        gldtng->field_1 |= 0x04;
        if (i < 400)
            delta = i;
        else
            delta = 400;
        gldtng->creature.gold_carried = delta;
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
        create_effect(&pos, TngEff_Unknown09, thing->owner);
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
        create_effect(&pos, TngEff_Unknown09, thing->owner);
    }
    i = (thing->field_58 >> 1);
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val+i;
    create_effect(&pos, TngEff_Unknown13, thing->owner);
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
    create_effect(&pos, TngEff_Unknown16, thing->owner);
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
        create_effect(&pos, TngEff_Unknown24, thing->owner);
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
    i = creatures[thing->model%CREATURE_TYPES_COUNT].natural_death_kind;
    switch (i)
    {
    case Death_Normal:
        thing_death_normal(thing);
        break;
    case Death_FleshExplode:
        thing_death_flesh_explosion(thing);
        break;
    case Death_GasFleshExplode:
        thing_death_gas_and_flesh_explosion(thing);
        break;
    case Death_SmokeExplode:
        thing_death_smoke_explosion(thing);
        break;
    case Death_IceExplode:
        thing_death_ice_explosion(thing);
        break;
    default:
        WARNLOG("Unexpected %s death cause %d",thing_model_name(thing),(int)i);
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
        if (thing_is_invalid(thing)) {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if (thing->parent_thing_idx == remove_idx)
        {
            thing->parent_thing_idx = thing->index;
            n++;
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT) {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return n;
}

void cause_creature_death(struct Thing *thing, unsigned char no_effects)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    long crmodel;
    //_DK_cause_creature_death(thing, no_effects); return;
    cctrl = creature_control_get_from_thing(thing);
    anger_set_creature_anger_all_types(thing, 0);
    throw_out_gold(thing);
    remove_thing_from_field1D_in_list(&game.thing_lists[1],thing->index);

    crmodel = thing->model;
    crstat = creature_stats_get_from_thing(thing);
    if ((no_effects) || (!thing_exists(thing)))
    {
        if ((game.flags_cd & MFlg_DeadBackToPool) != 0)
            add_creature_to_pool(crmodel, 1, 1);
        delete_thing_structure(thing, 0);
        return;
    }
    if ((crstat->rebirth != 0) && (cctrl->lairtng_idx > 0)
     && (crstat->rebirth-1 <= cctrl->explevel) )
    {
        creature_rebirth_at_lair(thing);
        return;
    }
    if ((cctrl->affected_by_spells & CCSpl_Freeze) != 0)
    {
        if ((game.flags_cd & MFlg_DeadBackToPool) != 0)
            add_creature_to_pool(crmodel, 1, 1);
        thing_death_ice_explosion(thing);
    } else
    if (!creature_model_bleeds(thing->model))
    {
        if ((game.flags_cd & MFlg_DeadBackToPool) != 0)
            add_creature_to_pool(crmodel, 1, 1);
        creature_death_as_nature_intended(thing);
    } else
    if (shot_model_makes_flesh_explosion(cctrl->shot_model))
    {
        if ((game.flags_cd & MFlg_DeadBackToPool) != 0)
            add_creature_to_pool(crmodel, 1, 1);
        thing_death_flesh_explosion(thing);
    } else
    {
        if ((game.flags_cd & MFlg_DeadBackToPool) != 0)
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
  set_camera_zoom(player->acamera, player->dungeon_camera_zoom);
  if (player->id_number == thing->owner)
  {
    turn_off_all_window_menus();
    turn_off_menu(GMnu_CREATURE_QUERY1);
    turn_off_menu(35);
    turn_off_menu(GMnu_CREATURE_QUERY3);
    turn_on_main_panel_menu();
    set_flag_byte(&game.numfield_C,0x40,(game.numfield_C & 0x20) != 0);
  }
  light_turn_light_on(player->field_460);
}

void delete_effects_attached_to_creature(struct Thing *crtng)
{
    struct CreatureControl *cctrl;
    struct Thing *efftng;
    long i,k;
    cctrl = creature_control_get_from_thing(crtng);
    if (creature_control_invalid(cctrl)) {
        return;
    }
    if ((cctrl->spell_flags & CSF_Armour) != 0)
    {
        set_flag_byte(&cctrl->spell_flags, CSF_Armour, false);
        for (i=0; i < 3; i++)
        {
            k = cctrl->field_2B3[i];
            if (k != 0)
            {
                efftng = thing_get(k);
                delete_thing_structure(efftng, 0);
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
                efftng = thing_get(k);
                delete_thing_structure(efftng, 0);
                cctrl->field_2B9[i] = 0;
            }
        }
    }
}

TbBool kill_creature(struct Thing *thing, struct Thing *killertng, char killer_plyr_idx,
      TbBool no_effects, TbBool died_in_battle, TbBool disallow_unconscious)
{
    struct CreatureControl *cctrl;
    struct CreatureControl *cctrlgrp;
    struct CreatureStats *crstat;
    struct Dungeon *dungeon;
    SYNCDBG(18,"Starting");
    //return _DK_kill_creature(thing, killertng, killer_plyr_idx, a4, died_in_battle, disallow_unconscious);
    dungeon = INVALID_DUNGEON;
    cctrl = creature_control_get_from_thing(thing);
    cleanup_current_thing_state(thing);
    remove_all_traces_of_combat(thing);
    if (creature_is_group_member(thing)) {
        remove_creature_from_group(thing);
    }
    if (!thing_is_invalid(killertng))
    {
        if (killertng->owner == game.neutral_player_num)
            died_in_battle = 0;
    }
    if (killer_plyr_idx == game.neutral_player_num) {
      died_in_battle = 0;
    }
    remove_events_thing_is_attached_to(thing);
    if (!thing_exists(thing)) {
        ERRORLOG("Tried to kill nonexisting thing!");
        return false;
    }
    if (thing->owner != game.neutral_player_num) {
        dungeon = get_players_num_dungeon(thing->owner);
    }
    if (!dungeon_invalid(dungeon))
    {
        update_dead_creatures_list(dungeon, thing);
        if (died_in_battle) {
            dungeon->battles_lost++;
        }
    }
    delete_effects_attached_to_creature(thing);
    update_kills_counters(thing, killertng, killer_plyr_idx, died_in_battle);
    if (thing_is_invalid(killertng) || (killertng->owner == game.neutral_player_num) || (killer_plyr_idx == game.neutral_player_num) || dungeon_invalid(dungeon))
    {
        if ((no_effects) && ((thing->field_0 & 0x20) != 0)) {
            prepare_to_controlled_creature_death(thing);
        }
        cause_creature_death(thing, no_effects);
        return true;
    }
    // Now we are sure that killertng and dungeon pointers are correct
    if (thing->owner == killertng->owner)
    {
        if ((get_creature_model_flags(thing) & MF_IsDiptera) && (get_creature_model_flags(killertng) & MF_IsArachnid)) {
            dungeon->lvstats.flies_killed_by_spiders++;
        }
    }
    cctrlgrp = creature_control_get_from_thing(killertng);
    if (!creature_control_invalid(cctrlgrp)) {
        cctrlgrp->field_C2++;
    }
    if (is_my_player_number(thing->owner)) {
        output_message(SMsg_BattleDeath, 40, true);
    } else
    if (is_my_player_number(killertng->owner)) {
        output_message(SMsg_BattleWon, 40, true);
    }
    if (game.hero_player_num == killertng->owner)
    {
        if (player_creature_tends_to(killertng->owner,CrTend_Imprison)) {
            ERRORLOG("Hero have tend to imprison");
        }
    }
    crstat = creature_stats_get_from_thing(killertng);
    anger_apply_anger_to_creature(killertng, crstat->annoy_win_battle, 4, 1);
    if (!creature_control_invalid(cctrlgrp) && died_in_battle)
      cctrlgrp->byte_9A++;
    if (!dungeon_invalid(dungeon)) {
        dungeon->hates_player[killertng->owner] += game.fight_hate_kill_value;
    }
    SYNCDBG(18,"Almost finished");
    if ((disallow_unconscious) || (!player_has_room(killertng->owner,RoK_PRISON))
      || (!player_creature_tends_to(killertng->owner,CrTend_Imprison)))
    {
        if (no_effects == 0) {
            cause_creature_death(thing, no_effects);
            return true;
        }
    }
    if (no_effects)
    {
        if ((thing->field_0 & 0x20) != 0) {
            prepare_to_controlled_creature_death(thing);
        }
        cause_creature_death(thing, no_effects);
        return true;
    }
    clear_creature_instance(thing);
    thing->active_state = CrSt_CreatureUnconscious;
    cctrl = creature_control_get_from_thing(thing);
    cctrl->flgfield_1 |= CCFlg_Immortal;
    cctrl->flgfield_1 |= CCFlg_NoCompControl;
    cctrl->field_280 = 2000;
    thing->health = 1;
    return false;
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
long calculate_shot_damage(struct Thing *thing,long shot_model)
{
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  struct ShotConfigStats *shotst;
  shotst = get_shot_model_stats(shot_model);
  cctrl = creature_control_get_from_thing(thing);
  crstat = creature_stats_get_from_thing(thing);
  return compute_creature_attack_damage(shotst->old->damage, crstat->luck, cctrl->explevel);
}

void creature_fire_shot(struct Thing *firing,struct  Thing *target, unsigned short shot_model, char a2, unsigned char hit_type)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct ShotConfigStats *shotst;
    struct Coord3d pos1;
    struct Coord3d pos2;
    struct ComponentVector cvect;
    struct Thing *shot;
    struct Thing *tmptng;
    short angle_xy,angle_yz;
    long damage;
    long target_idx,i;
    TbBool flag1;
    //_DK_creature_fire_shot(firing,target,shot_model,a2,a3); return;
    cctrl = creature_control_get_from_thing(firing);
    crstat = creature_stats_get_from_thing(firing);
    shotst = get_shot_model_stats(shot_model);
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
      if (( shotst->old->field_48 ) && (target->class_id != TCls_Door))
      {
        flag1 = true;
        pos1.z.val = pos2.z.val;
      }
      angle_xy = get_angle_xy_to(&pos1, &pos2);
      angle_yz = get_angle_yz_to(&pos1, &pos2);
    }
    // Compute shot damage
    if ( shotst->old->field_48 )
    {
      damage = calculate_melee_damage(firing);
    } else
    {
      damage = calculate_shot_damage(firing,shot_model);
    }
    shot = NULL;
    target_idx = 0;
    // Set target index for navigating shots
    if (shot_model_is_navigable(shot_model))
    {
      if (!thing_is_invalid(target))
        target_idx = target->index;
    }
    switch ( shot_model )
    {
    case 4:
    case 12:
        if ((thing_is_invalid(target)) || (get_2d_distance(&firing->mappos, &pos2) > 5120))
        {
            project_point_to_wall_on_angle(&pos1, &pos2, firing->field_52, firing->field_54, 256, 20);
        }
        shot = create_thing(&pos2, TCls_Shot, shot_model, firing->owner, -1);
        if (thing_is_invalid(shot))
          return;
        if (shot_model == 12)
          draw_lightning(&pos1, &pos2, 96, 93);
        else
          draw_lightning(&pos1, &pos2, 96, 60);
        shot->health = shotst->old->health;
        shot->word_14 = shotst->old->damage;
        shot->parent_thing_idx = firing->index;
        break;
    case 7:
        if ((thing_is_invalid(target)) || (get_2d_distance(&firing->mappos, &pos2) > 768))
          project_point_to_wall_on_angle(&pos1, &pos2, firing->field_52, firing->field_54, 256, 4);
        shot = create_thing(&pos2, TCls_Shot, shot_model, firing->owner, -1);
        if (thing_is_invalid(shot))
          return;
        draw_flame_breath(&pos1, &pos2, 96, 2);
        shot->health = shotst->old->health;
        shot->word_14 = shotst->old->damage;
        shot->parent_thing_idx = firing->index;
        break;
    case 13:
        for (i=0; i < 32; i++)
        {
            tmptng = create_thing(&pos1, TCls_Shot, shot_model, firing->owner, -1);
            if (thing_is_invalid(tmptng))
              break;
            shot = tmptng;
            shot->byte_16 = hit_type;
            shot->field_52 = (angle_xy + ACTION_RANDOM(101) - 50) & 0x7FF;
            shot->field_54 = (angle_yz + ACTION_RANDOM(101) - 50) & 0x7FF;
            angles_to_vector(shot->field_52, shot->field_54, shotst->old->speed, &cvect);
            shot->acceleration.x.val += cvect.x;
            shot->acceleration.y.val += cvect.y;
            shot->acceleration.z.val += cvect.z;
            shot->field_1 |= 0x04;
            shot->word_14 = damage;
            shot->health = shotst->old->health;
            shot->parent_thing_idx = firing->index;
        }
        break;
    default:
        shot = create_thing(&pos1, TCls_Shot, shot_model, firing->owner, -1);
        if (thing_is_invalid(shot))
          return;
        shot->field_52 = angle_xy;
        shot->field_54 = angle_yz;
        angles_to_vector(shot->field_52, shot->field_54, shotst->old->speed, &cvect);
        shot->acceleration.x.val += cvect.x;
        shot->acceleration.y.val += cvect.y;
        shot->acceleration.z.val += cvect.z;
        shot->field_1 |= 0x04;
        shot->word_14 = damage;
        shot->health = shotst->old->health;
        shot->parent_thing_idx = firing->index;
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
        WARNLOG("Shot of type %d carries %d damage",(int)shot_model,(int)damage);
      }
#endif
      shot->byte_16 = hit_type;
      if (shotst->old->firing_sound > 0)
      {
        thing_play_sample(firing, shotst->old->firing_sound + UNSYNC_RANDOM(shotst->old->firing_sound_variants),
            100, 0, 3, 0, 3, 256);
      }
      if (shotst->old->shot_sound > 0)
      {
        thing_play_sample(shot, shotst->old->shot_sound, 100, 0, 3, 0, shotst->old->field_20, 256);
      }
      set_flag_byte(&shot->field_25,0x10,flag1);
    }
}

void set_creature_level(struct Thing *thing, long nlvl)
{
  //_DK_set_creature_level(thing, nlvl); return;
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
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
  add_creature_score_to_owner(thing);
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

long get_creature_speed(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long speed;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return 0;
    speed = cctrl->max_speed;
    if (speed < 0)
        speed = 0;
    if (speed > 256)
        speed = 256;
    return speed;
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
    //_DK_set_creature_instance(thing, inst_idx, a2, a3, pos); return;
    if (inst_idx == 0)
        return;
    cctrl = creature_control_get_from_thing(thing);
    inst_inf = creature_instance_info_get(inst_idx);
    if (creature_instance_info_invalid(inst_inf) || (inst_inf->time == -1))
    {
        ERRORLOG("Negative instance");
        return;
    }
    if (inst_inf->force_visibility)
    {
        i = cctrl->field_AF;
        if (i <= inst_inf->force_visibility)
          i = inst_inf->force_visibility;
        cctrl->field_AF = i;
    }
    get_creature_instance_times(thing, inst_idx, &itime, &aitime);
    if ((cctrl->instance_id != CrInst_NULL) && (cctrl->instance_id == inst_idx))
    {
        if (inst_inf->field_1A)
        {
            cctrl->field_D3 = 1;
            return;
        }
    }
    cctrl->instance_id = inst_idx;
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
  TbGraphicsWindow grwnd;
  struct PlayerInfo *player;
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
  LbScreenLoadGraphicsWindow(&grwnd);
  // Draw the buffer on real screen
  setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
  draw_lens_effect(lbDisplay.WScreen, lbDisplay.GraphicsScreenWidth, scrmem, eye_lens_width,
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

struct Thing *get_group_leader(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct Thing *leader;
  cctrl = creature_control_get_from_thing(thing);
  leader = thing_get(cctrl->field_7A & 0xFFF);
  return leader;
}

TbBool creature_is_group_member(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    return ((cctrl->field_7A & 0xFFF) > 0);
}

TbBool creature_is_group_leader(struct Thing *thing)
{
    struct Thing *leader;
    //return _DK_creature_is_group_leader(thing);
    leader = get_group_leader(thing);
    if (thing_is_invalid(leader))
        return false;
    return (leader == thing);
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
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct CreatureControl *secctrl;
    struct Thing *sectng;
    //_DK_remove_first_creature(thing);
    cctrl = creature_control_get_from_thing(thing);
    if ((thing->field_0 & 0x08) == 0)
    {
        ERRORLOG("Thing %d is not in Peter list",(int)thing->index);
        return;
    }
    if (game.neutral_player_num == thing->owner)
    {
      sectng = thing_get(cctrl->field_1D);
      if (!thing_is_invalid(sectng)) {
          secctrl = creature_control_get_from_thing(sectng);
          secctrl->players_next_creature_idx = cctrl->players_next_creature_idx;
      } else {
          game.field_14EA46 = cctrl->players_next_creature_idx;
      }
      sectng = thing_get(cctrl->players_next_creature_idx);
      if (!thing_is_invalid(sectng)) {
          secctrl = creature_control_get_from_thing(sectng);
          secctrl->field_1D = cctrl->field_1D;
      }
    } else
    if ((thing->model != get_players_special_digger_breed(thing->owner))
        || (game.hero_player_num == thing->owner))
    {
        dungeon = get_dungeon(thing->owner);
        sectng = thing_get(cctrl->field_1D);
        if (!thing_is_invalid(sectng)) {
            secctrl = creature_control_get_from_thing(sectng);
            secctrl->players_next_creature_idx = cctrl->players_next_creature_idx;
        } else {
            dungeon->creatr_list_start = cctrl->players_next_creature_idx;
        }
        sectng = thing_get(cctrl->players_next_creature_idx);
        if (!thing_is_invalid(sectng)) {
            secctrl = creature_control_get_from_thing(sectng);
            secctrl->field_1D = cctrl->field_1D;
        }
        if ((cctrl->field_2 & 0x02) == 0)
        {
          dungeon->num_active_creatrs--;
          dungeon->owned_creatures_of_model[thing->model]--;
        }
    } else
    {
        dungeon = get_dungeon(thing->owner);
        sectng = thing_get(cctrl->field_1D);
        if (!thing_is_invalid(sectng)) {
            secctrl = creature_control_get_from_thing(sectng);
            secctrl->players_next_creature_idx = cctrl->players_next_creature_idx;
        } else {
            dungeon->digger_list_start = cctrl->players_next_creature_idx;
        }
        sectng = thing_get(cctrl->players_next_creature_idx);
        if (!thing_is_invalid(sectng)) {
            secctrl = creature_control_get_from_thing(sectng);
            secctrl->field_1D = cctrl->field_1D;
        }
        dungeon->num_active_diggers--;
    }
    cctrl->field_1D = 0;
    cctrl->players_next_creature_idx = 0;
    thing->field_0 &= ~0x08u;
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
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Room *room;
    //_DK_change_creature_owner(thing, nowner);
    cctrl = creature_control_get_from_thing(thing);
    if (thing->light_id != 0) {
        light_delete_light(thing->light_id);
        thing->light_id = 0;
    }
    if ((cctrl->field_7A & 0xFFF) != 0)
        remove_creature_from_group(thing);
    if (cctrl->lairtng_idx != 0)
    {
        room = room_get(cctrl->lair_room_id);
        if (!room_is_invalid(room)) {
            creature_remove_lair_from_room(thing, room);
        } else {
            ERRORDBG(8,"The %s index %d has lair %d in nonexisting room.",thing_model_name(thing),(int)thing->index,(int)cctrl->lairtng_idx);
            cctrl->lairtng_idx = 0;
        }
    }
    if ((thing->field_0 & 0x08) != 0)
      remove_first_creature(thing);
    if (thing->owner != game.neutral_player_num)
    {
        dungeon = get_dungeon(thing->owner);
        dungeon->score -= get_creature_thing_score(thing);
        if ( anger_is_creature_angry(thing) )
            dungeon->creatures_annoyed--;
        remove_events_thing_is_attached_to(thing);
    }
    thing->owner = nowner;
    set_first_creature(thing);
    set_start_state(thing);
    if (thing->owner != game.neutral_player_num)
    {
        dungeon = get_dungeon(thing->owner);
        dungeon->score += get_creature_thing_score(thing);
        if ( anger_is_creature_angry(thing) )
            dungeon->creatures_annoyed++;
    }

}

struct Thing *create_creature(struct Coord3d *pos, unsigned short model, unsigned short owner)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct CreatureData *crdata;
    struct Thing *crtng;
    long i;
    crstat = creature_stats_get(model);
    if (!i_can_allocate_free_thing_structure(TAF_FreeEffectIfNoSlots))
    {
        ERRORDBG(3,"Cannot create breed %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    if (!i_can_allocate_free_control_structure())
    {
        ERRORDBG(3,"Cannot create breed %d for player %d. There are too many creatures allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeCreatrs);
        return INVALID_THING;
    }
    crtng = allocate_free_thing_structure(TAF_FreeEffectIfNoSlots);
    if (crtng->index == 0) {
        ERRORDBG(3,"Should be able to allocate creature %d for player %d, but failed.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    cctrl = allocate_free_control_structure();
    crtng->ccontrol_idx = cctrl->index;
    crtng->class_id = 5;
    crtng->model = model;
    crtng->parent_thing_idx = crtng->index;
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
    if (owner == game.hero_player_num)
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
    add_thing_to_its_class_list(crtng);
    place_thing_in_mapwho(crtng);
    if (owner <= PLAYERS_COUNT)
      set_first_creature(crtng);
    set_start_state(crtng);
    add_creature_score_to_owner(crtng);
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
      && (thing->active_state != CrSt_CreatureUnconscious) && (nmaxim > maximizer) )
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
      && (thing->active_state != CrSt_CreatureUnconscious) && (nmaxim > maximizer) )
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
      && (thing->active_state != CrSt_CreatureUnconscious) && (nmaxim > maximizer) )
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
      && (thing->active_state != CrSt_CreatureUnconscious) && (nmaxim > maximizer) )
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
      && (thing->active_state != CrSt_CreatureUnconscious) )
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
      && (thing->active_state != CrSt_CreatureUnconscious) )
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
        thing = get_player_list_creature_with_filter(dungeon->digger_list_start, filter, &param);
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
        thing = get_player_list_creature_with_filter(dungeon->digger_list_start, filter, &param);
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
        thing = get_player_list_creature_with_filter(dungeon->digger_list_start, filter, &param);
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
          && (thing->active_state != CrSt_CreatureUnconscious) && is_my_player_number(thing->owner) )
        {
          dungeon->selected_creatures_of_model[breed_idx] = 0;
          thing = NULL;
        } else
        {
          cctrl = creature_control_get_from_thing(thing);
          thing = thing_get(cctrl->players_next_creature_idx);
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
          && (thing->active_state != CrSt_CreatureUnconscious) && is_my_player_number(thing->owner)
          && (get_creature_gui_job(thing) == job_idx) )
        {
            cctrl = creature_control_get_from_thing(thing);
            thing = thing_get(cctrl->players_next_creature_idx);
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
            set_players_packet_action(get_my_player(), PckA_Unknown090, thing->index, 0, 0, 0);
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
    if (!can_thing_be_picked_up_by_player(thing, dungeon->owner))
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

struct Thing *create_footprint_sine(struct Coord3d *crtr_pos, unsigned short phase, short nfoot, unsigned short model, unsigned short owner)
{
  struct Coord3d pos;
  unsigned int i;
  pos.x.val = crtr_pos->x.val;
  pos.y.val = crtr_pos->y.val;
  pos.z.val = crtr_pos->z.val;
  switch (nfoot)
  {
  case 1:
      i = (phase - 512);
      pos.x.val +=   (LbSinL(i) << 6) >> 16;
      pos.y.val += -((LbCosL(i) << 6) >> 8) >> 8;
      return create_thing(&pos, TCls_EffectElem, model, owner, -1);
  case 2:
      i = (phase - 512);
      pos.x.val -=   (LbSinL(i) << 6) >> 16;
      pos.y.val -= -((LbCosL(i) << 6) >> 8) >> 8;
      return create_thing(&pos, TCls_EffectElem, model, owner, -1);
  }
  return INVALID_THING;
}

void place_bloody_footprint(struct Thing *thing)
{
    struct Thing *footng;
    struct CreatureControl *cctrl;
    short nfoot;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return;
    }
    nfoot = get_foot_creature_has_down(thing);
    switch (creatures[thing->model%CREATURE_TYPES_COUNT].field_6)
    {
    case 3:
    case 4:
        break;
    case 5:
        if (nfoot)
        {
            footng = create_thing(&thing->mappos, TCls_EffectElem, 23, thing->owner, -1);
            if (!thing_is_invalid(footng))
                cctrl->bloody_footsteps_turns--;
        }
        break;
    default:
        footng = create_footprint_sine(&thing->mappos, thing->field_52, nfoot, 23, thing->owner);
        if (!thing_is_invalid(footng))
            cctrl->bloody_footsteps_turns--;
        break;
    }
  }

  short update_creature_movements(struct Thing *thing)
  {
    struct CreatureControl *cctrl;
    short upd_done;
    int i;
    SYNCDBG(18,"Starting");
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return false;
    }
    upd_done = 0;
    if (cctrl->affected_by_spells != 0)
    {
      upd_done = 1;
      cctrl->pos_BB.x.val = 0;
      cctrl->pos_BB.y.val = 0;
      cctrl->pos_BB.z.val = 0;
      cctrl->move_speed = 0;
      set_flag_byte(&cctrl->field_2,0x01,false);
    } else
    {
      if ( thing->field_0 & 0x20 )
      {
        if ( thing->field_25 & 0x20 )
        {
          if (cctrl->move_speed != 0)
          {
            cctrl->pos_BB.x.val = (LbSinL(thing->field_52)>> 8)
                  * (cctrl->move_speed * LbCosL(thing->field_54) >> 8) >> 16;
            cctrl->pos_BB.y.val = -((LbCosL(thing->field_52) >> 8)
                  * (cctrl->move_speed * LbCosL(thing->field_54) >> 8) >> 8) >> 8;
            cctrl->pos_BB.z.val = cctrl->move_speed * LbSinL(thing->field_54) >> 16;
          }
          if (cctrl->field_CA != 0)
          {
            cctrl->pos_BB.x.val +=   cctrl->field_CA * LbSinL(thing->field_52 - 512) >> 16;
            cctrl->pos_BB.y.val += -(cctrl->field_CA * LbCosL(thing->field_52 - 512) >> 8) >> 8;
          }
        } else
        {
          if (cctrl->move_speed != 0)
          {
            upd_done = 1;
            cctrl->pos_BB.x.val =   cctrl->move_speed * LbSinL(thing->field_52) >> 16;
            cctrl->pos_BB.y.val = -(cctrl->move_speed * LbCosL(thing->field_52) >> 8) >> 8;
          }
          if (cctrl->field_CA != 0)
          {
            upd_done = 1;
            cctrl->pos_BB.x.val +=   cctrl->field_CA * LbSinL(thing->field_52 - 512) >> 16;
            cctrl->pos_BB.y.val += -(cctrl->field_CA * LbCosL(thing->field_52 - 512) >> 8) >> 8;
          }
        }
      } else
      if (cctrl->field_2 & 0x01)
      {
        upd_done = 1;
        set_flag_byte(&cctrl->field_2,0x01,false);
      } else
      if (cctrl->move_speed != 0)
      {
        upd_done = 1;
        cctrl->pos_BB.x.val =   cctrl->move_speed * LbSinL(thing->field_52) >> 16;
        cctrl->pos_BB.y.val = -(cctrl->move_speed * LbCosL(thing->field_52) >> 8) >> 8;
        cctrl->pos_BB.z.val = 0;
      }
      if (((thing->field_25 & 0x20) != 0) && ((thing->field_0 & 0x20) == 0))
      {
        i = get_floor_height_under_thing_at(thing, &thing->mappos) - thing->mappos.z.val + 256;
        if (i > 0)
        {
          upd_done = 1;
          if (i >= 32)
            i = 32;
          cctrl->pos_BB.z.val += i;
        } else
        if (i < 0)
        {
          upd_done = 1;
          i = -i;
          if (i >= 32)
            i = 32;
          cctrl->pos_BB.z.val -= i;
        }
      }
    }
    SYNCDBG(19,"Finished");
    if (upd_done)
      return true;
    else
      return ((cctrl->pos_BB.x.val != 0) || (cctrl->pos_BB.y.val != 0) || (cctrl->pos_BB.z.val != 0));
}

void check_for_creature_escape_from_lava(struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    if (((thing->field_0 & 0x20) == 0) && ((thing->field_25 & 0x02) != 0))
    {
      crstat = creature_stats_get_from_thing(thing);
      if (crstat->hurt_by_lava)
      {
          cctrl = creature_control_get_from_thing(thing);
          if ((!creature_is_escaping_death(thing)) && (cctrl->field_2FE + 64 < game.play_gameturn))
          {
              cctrl->field_2FE = game.play_gameturn;
              if ( cleanup_current_thing_state(thing) )
              {
                if ( setup_move_off_lava(thing) )
                    thing->continue_state = CrSt_CreatureEscapingDeath;
                else
                    set_start_state(thing);
              }
          }
      }
    }
}

void process_creature_leave_footsteps(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Thing *footng;
    struct SlabMap *slb;
    short nfoot;
    cctrl = creature_control_get_from_thing(thing);
    if ((thing->field_25 & 0x01) != 0)
    {
        nfoot = get_foot_creature_has_down(thing);
        if (nfoot)
        {
          create_effect(&thing->mappos, TngEff_Unknown19, thing->owner);
        }
        cctrl->bloody_footsteps_turns = 0;
    } else
    // Bloody footprints
    if (cctrl->bloody_footsteps_turns != 0)
    {
        place_bloody_footprint(thing);
        nfoot = get_foot_creature_has_down(thing);
        footng = create_footprint_sine(&thing->mappos, thing->field_52, nfoot, 23, thing->owner);
        if (!thing_is_invalid(footng)) {
            cctrl->bloody_footsteps_turns--;
        }
    } else
    // Snow footprints
    if (game.texture_id == 2)
    {
        slb = get_slabmap_block(map_to_slab[thing->mappos.x.stl.num], map_to_slab[thing->mappos.y.stl.num]);
        if (slb->kind == SlbT_PATH)
        {
          thing->field_25 |= 0x80u;
          nfoot = get_foot_creature_has_down(thing);
          footng = create_footprint_sine(&thing->mappos, thing->field_52, nfoot, 94, thing->owner);
        }
    }
}

void process_landscape_affecting_creature(struct Thing *thing)
{
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  unsigned long navmap;
  int stl_idx;
  int i;
  SYNCDBG(18,"Starting");
  set_flag_byte(&thing->field_25,0x01,false);
  set_flag_byte(&thing->field_25,0x02,false);
  set_flag_byte(&thing->field_25,0x80,false);
  cctrl = creature_control_get_from_thing(thing);
  if (creature_control_invalid(cctrl))
  {
      ERRORLOG("Invalid creature control; no action");
      return;
  }
  cctrl->field_B9 = 0;

  stl_idx = get_subtile_number(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
  navmap = get_navigation_map(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
  if (((navmap & 0xF) << 8) == thing->mappos.z.val)
  {
    i = get_top_cube_at_pos(stl_idx);
    if ((i & 0xFFFFFFFE) == 40)
    {
      crstat = creature_stats_get_from_thing(thing);
      apply_damage_to_thing_and_display_health(thing, crstat->hurt_by_lava, -1);
      thing->field_25 |= 0x02;
    } else
    if (i == 39)
    {
      thing->field_25 |= 0x01;
    }
    process_creature_leave_footsteps(thing);
    process_creature_standing_on_corpses_at(thing, &thing->mappos);
  }
  check_for_creature_escape_from_lava(thing);
  SYNCDBG(19,"Finished");
}

TbBool add_creature_score_to_owner(struct Thing *thing)
{
    struct Dungeon *dungeon;
    long score;
    if (thing->owner == game.neutral_player_num)
        return false;
    dungeon = get_dungeon(thing->owner);
    if (dungeon_invalid(dungeon))
        return false;
    score = get_creature_thing_score(thing);
    if (dungeon->score < LONG_MAX-score)
        dungeon->score += score;
    else
        dungeon->score = LONG_MAX;
    return true;
}

TbBool remove_creature_score_from_owner(struct Thing *thing)
{
    struct Dungeon *dungeon;
    long score;
    if (thing->owner == game.neutral_player_num)
        return false;
    dungeon = get_dungeon(thing->owner);
    if (dungeon_invalid(dungeon))
        return false;
    score = get_creature_thing_score(thing);
    if (dungeon->score >= score)
        dungeon->score -= score;
    else
        dungeon->score = 0;
    return true;
}

long get_creature_thing_score(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long breed,exp;
    cctrl = creature_control_get_from_thing(thing);
    breed = thing->model;
    if (breed >= CREATURE_TYPES_COUNT)
        breed = 0;
    if (breed < 0)
        breed = 0;
    exp = cctrl->explevel;
    if (exp >= CREATURE_MAX_LEVEL)
        exp = 0;
    if (exp < 0)
        exp = 0;
    return game.creature_scores[breed].value[exp];
}

long update_creature_levels(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    struct CreatureStats *crstat;
    struct PlayerInfo *player;
    struct CreatureControl *cctrl;
    struct Thing *newtng;
    cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->field_AD & 0x40) == 0)
      return 0;
    cctrl->field_AD &= ~0x40;
    remove_creature_score_from_owner(thing);
    // If a creature is not on highest level, just update the level
    if (cctrl->explevel+1 < CREATURE_MAX_LEVEL)
    {
      set_creature_level(thing, cctrl->explevel+1);
      return 1;
    }
    // If it is highest level, maybe we should transform the creature?
    crstat = creature_stats_get_from_thing(thing);
    if (crstat->grow_up == 0)
      return 0;
    // Transforming
    newtng = create_creature(&thing->mappos, crstat->grow_up, thing->owner);
    if (newtng == NULL)
    {
      ERRORLOG("Could not create creature to transform to");
      return 0;
    }
    set_creature_level(newtng, crstat->grow_up_level-1);
    update_creature_health_to_max(newtng);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_282 = 50;
    external_set_thing_state(newtng, CrSt_CreatureBeHappy);
    player = get_player(thing->owner);
    // Switch control if this creature is possessed
    if (is_thing_passenger_controlled(thing))
    {
      leave_creature_as_controller(player, thing);
      control_creature_as_controller(player, newtng);
    }
    if (thing->index == player->controlled_thing_idx)
    {
      player->controlled_thing_idx = newtng->index;
      player->field_31 = newtng->field_9;
    }
    kill_creature(thing, INVALID_THING, -1, 1, 0, 1);
    return -1;
}

long update_creature(struct Thing *thing)
{
    struct PlayerInfo *player;
    struct CreatureControl *cctrl;
    struct Thing *tngp;
    struct Map *map;
    SYNCDBG(18,"Thing index %d",(int)thing->index);
    map = get_map_block_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if ((thing->active_state == CrSt_CreatureUnconscious) && ((map->flags & 0x40) != 0))
    {
        SYNCDBG(8,"Killing unconscious %s index %d on toxic map black.",thing_model_name(thing),(int)thing->index);
        kill_creature(thing, INVALID_THING, -1, 1, 0, 1);
        return 0;
    }
    if (thing->health < 0)
    {
        kill_creature(thing, INVALID_THING, -1, 0, 0, 0);
        return 0;
    }
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        WARNLOG("Killing %s index %d with invalid control.",thing_model_name(thing),(int)thing->index);
        kill_creature(thing, INVALID_THING, -1, 0, 0, 0);
        return 0;
    }
    if (game.field_150356 != 0)
    {
        if ((cctrl->field_2EF != 0) && (cctrl->field_2EF <= game.play_gameturn))
        {
            cctrl->field_2EF = 0;
            create_effect(&thing->mappos, imp_spangle_effects[thing->owner], thing->owner);
            move_thing_in_map(thing, &game.armageddon.mappos);
        }
    }

    if (cctrl->field_B1 > 0)
        cctrl->field_B1--;
    if (cctrl->byte_8B == 0)
        cctrl->byte_8B = game.field_14EA4B;
    if (cctrl->field_302 == 0)
        process_creature_instance(thing);
    update_creature_count(thing);
    if ((thing->field_0 & 0x20) != 0)
    {
        if (cctrl->affected_by_spells == 0)
        {
          if (cctrl->field_302 != 0)
          {
              cctrl->field_302--;
          } else
          if (process_creature_state(thing))
          {
              ERRORLOG("A state return type for a human controlled creature?");
          }
        }
        cctrl = creature_control_get_from_thing(thing);
        player = get_player(thing->owner);
        if ((cctrl->affected_by_spells & CCSpl_Freeze) != 0)
        {
            if ((player->field_3 & 0x04) == 0)
              PaletteSetPlayerPalette(player, blue_palette);
        } else
        {
            if ((player->field_3 & 0x04) != 0)
              PaletteSetPlayerPalette(player, _DK_palette);
        }
    } else
    {
        if (cctrl->affected_by_spells == 0)
        {
          if (cctrl->field_302 > 0)
          {
            cctrl->field_302--;
          } else
          if (process_creature_state(thing))
          {
            return 0;
          }
        }
    }

    if (update_creature_movements(thing))
    {
        thing->velocity.x.val += cctrl->pos_BB.x.val;
        thing->velocity.y.val += cctrl->pos_BB.y.val;
        thing->velocity.z.val += cctrl->pos_BB.z.val;
    }
    move_creature(thing);
    if ((thing->field_0 & 0x20) != 0)
    {
        if ((cctrl->flgfield_1 & CCFlg_Unknown40) == 0)
          cctrl->move_speed /= 2;
        if ((cctrl->flgfield_1 & CCFlg_Unknown80) == 0)
          cctrl->field_CA /= 2;
    } else
    {
        cctrl->move_speed = 0;
    }
    process_spells_affected_by_effect_elements(thing);
    process_landscape_affecting_creature(thing);
    process_disease(thing);
    move_thing_in_map(thing, &thing->mappos);
    set_creature_graphic(thing);
    if (cctrl->field_2B0)
    {
        process_keeper_spell_effect(thing);
    }

    if (thing->word_17 > 0)
        thing->word_17--;

    if (cctrl->field_7A & 0x0FFF)
    {
        if ( creature_is_group_leader(thing) )
          leader_find_positions_for_followers(thing);
    }

    if (cctrl->field_6E > 0)
    {
        tngp = thing_get(cctrl->field_6E);
        if ((tngp->field_1 & 0x01) != 0)
          move_thing_in_map(tngp, &thing->mappos);
    }
    if (update_creature_levels(thing) == -1)
    {
        return 0;
    }
    process_creature_self_spell_casting(thing);
    cctrl->pos_BB.x.val = 0;
    cctrl->pos_BB.y.val = 0;
    cctrl->pos_BB.z.val = 0;
    set_flag_byte(&cctrl->flgfield_1,CCFlg_Unknown40,false);
    set_flag_byte(&cctrl->flgfield_1,CCFlg_Unknown80,false);
    set_flag_byte(&cctrl->field_AD,0x04,false);
    process_thing_spell_effects(thing);
    SYNCDBG(19,"Finished");
    return 1;
}

TbBool creature_is_slappable(const struct Thing *thing, long plyr_idx)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    if (thing->owner != plyr_idx)
    {
      if (creature_is_kept_in_prison(thing) || creature_is_being_tortured(thing))
      {
        cctrl = creature_control_get_from_thing(thing);
        room = room_get(cctrl->work_room_id);
        return (room->owner == plyr_idx);
      }
      return false;
    }
    if (creature_is_being_sacrificed(thing) || creature_is_being_summoned(thing))
      return 0;
    if (creature_is_kept_in_prison(thing) || creature_is_being_tortured(thing))
    {
      cctrl = creature_control_get_from_thing(thing);
      room = room_get(cctrl->work_room_id);
      return (room->owner == plyr_idx);
    }
    return true;
}

TbBool creature_stats_debug_dump(void)
{
    struct Thing *thing;
    long crstate;
    TbBool result;
    unsigned long k;
    int i;
    result = false;
    k = 0;
    i = game.thing_lists[TngList_Creatures].index;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing)) {
            ERRORLOG("Jump to invalid thing detected");
            result = true;
            break;
        }
        i = thing->next_of_class;
        // Per-creature block starts
        crstate = get_creature_state_besides_move(thing);
        if (thing->owner != hero_player_number) {
            switch (crstate)
            {
            case CrSt_GoodDoingNothing:
            case CrSt_GoodReturnsToStart:
            case CrSt_GoodBackAtStart:
            case CrSt_GoodDropsGold:
            case CrSt_GoodLeaveThroughExitDoor:
            case CrSt_GoodWaitInExitDoor:
            case CrSt_GoodAttackRoom1:
            case CrSt_CreatureSearchForGoldToStealInRoom2:
            case CrSt_GoodAttackRoom2:
                ERRORLOG("Player %d %s index %d is in Good-only state %d",(int)thing->owner,thing_model_name(thing),(int)thing->index,(int)crstate);
                result = true;
                break;
            }
        }

        // Per-creature block ends
        k++;
        if (k > THINGS_COUNT) {
            ERRORLOG("Infinite loop detected when sweeping things list");
            result = true;
            break;
        }
    }
    return result;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
