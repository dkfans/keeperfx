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
 * @date     17 Mar 2009 - 21 Nov 2012
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
#include "bflib_planar.h"
#include "bflib_vidraw.h"

#include "engine_lenses.h"
#include "engine_arrays.h"
#include "config_creature.h"
#include "config_effects.h"
#include "config_terrain.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "creature_states_lair.h"
#include "creature_states_mood.h"
#include "creature_states_gardn.h"
#include "creature_states_train.h"
#include "creature_states_spdig.h"
#include "creature_instances.h"
#include "creature_graphics.h"
#include "creature_battle.h"
#include "creature_groups.h"
#include "creature_jobs.h"
#include "creature_senses.h"
#include "config_lenses.h"
#include "config_crtrstates.h"
#include "thing_stats.h"
#include "thing_factory.h"
#include "thing_effects.h"
#include "thing_objects.h"
#include "thing_navigate.h"
#include "thing_shots.h"
#include "thing_creature.h"
#include "thing_corpses.h"
#include "thing_physics.h"
#include "lens_api.h"
#include "light_data.h"
#include "room_jobs.h"
#include "map_utils.h"
#include "map_blocks.h"
#include "gui_topmsg.h"
#include "front_simple.h"
#include "frontend.h"
#include "magic.h"
#include "player_instances.h"
#include "player_states.h"
#include "power_hand.h"
#include "power_process.h"
#include "gui_frontmenu.h"
#include "gui_soundmsgs.h"
#include "engine_redraw.h"
#include "sounds.h"
#include "game_legacy.h"

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
extern struct TbLoadFiles swipe_load_file[];
extern struct TbSetupSprite swipe_setup_sprites[];
/******************************************************************************/
DLLIMPORT struct Thing *_DK_find_my_next_creature_of_breed_and_job(long crmodel, long job_idx, long targtng_idx);
DLLIMPORT void _DK_anger_set_creature_anger_all_types(struct Thing *creatng, long reason);
DLLIMPORT void _DK_change_creature_owner(struct Thing *creatng , char nowner);
DLLIMPORT void _DK_cause_creature_death(struct Thing *creatng, unsigned char reason);
DLLIMPORT void _DK_apply_spell_effect_to_thing(struct Thing *creatng, long spell_idx, long spell_lev);
DLLIMPORT void _DK_creature_cast_spell_at_thing(struct Thing *castng, struct Thing *targetng, long targtng_idx, long no_effects);
DLLIMPORT void _DK_creature_cast_spell(struct Thing *castng, long spl_idx, long blocked_flags, long trg_x, long trg_y);
DLLIMPORT void _DK_set_first_creature(struct Thing *creatng);
DLLIMPORT void _DK_remove_first_creature(struct Thing *creatng);
DLLIMPORT struct Thing *_DK_get_creature_near(unsigned short pos_x, unsigned short pos_y);
DLLIMPORT struct Thing *_DK_get_creature_near_with_filter(unsigned short pos_x, unsigned short pos_y, Thing_Filter filter, long no_effects);
DLLIMPORT struct Thing *_DK_get_creature_near_for_controlling(unsigned char a1, long reason, long targtng_idx);
DLLIMPORT void _DK_anger_apply_anger_to_creature(struct Thing *creatng, long anger, long reason, long targtng_idx);
DLLIMPORT long _DK_creature_available_for_combat_this_turn(struct Thing *creatng);
DLLIMPORT struct Thing *_DK_get_enemy_dungeon_heart_creature_can_see(struct Thing *creatng);
DLLIMPORT long _DK_set_creature_object_combat(struct Thing *creatng, struct Thing *goldtng);
DLLIMPORT void _DK_set_creature_door_combat(struct Thing *creatng, struct Thing *goldtng);
DLLIMPORT void _DK_food_eaten_by_creature(struct Thing *foodtng, struct Thing *creatng);
DLLIMPORT void _DK_creature_fire_shot(struct Thing *firing,struct  Thing *targetng, unsigned short a1, char reason, unsigned char targtng_idx);
DLLIMPORT unsigned long _DK_control_creature_as_controller(struct PlayerInfo *player, struct Thing *creatng);
DLLIMPORT unsigned long _DK_control_creature_as_passenger(struct PlayerInfo *player, struct Thing *creatng);
DLLIMPORT void _DK_load_swipe_graphic_for_creature(struct Thing *creatng);
DLLIMPORT unsigned short _DK_find_next_annoyed_creature(unsigned char a1, unsigned short reason);
DLLIMPORT long _DK_creature_instance_has_reset(const struct Thing *creatng, long reason);
DLLIMPORT long _DK_get_human_controlled_creature_target(struct Thing *creatng, long reason);
DLLIMPORT void _DK_set_creature_instance(struct Thing *creatng, long a1, long reason, long targtng_idx, struct Coord3d *pos);
DLLIMPORT void _DK_draw_creature_view(struct Thing *creatng);
DLLIMPORT void _DK_process_creature_standing_on_corpses_at(struct Thing *creatng, struct Coord3d *pos);
DLLIMPORT short _DK_kill_creature(struct Thing *creatng, struct Thing *tngrp, char a1, unsigned char reason, unsigned char targtng_idx, unsigned char no_effects);
DLLIMPORT void _DK_update_creature_count(struct Thing *creatng);
DLLIMPORT long _DK_process_creature_state(struct Thing *creatng);
DLLIMPORT long _DK_move_creature(struct Thing *creatng);
DLLIMPORT void _DK_init_creature_level(struct Thing *creatng, long nlev);
DLLIMPORT long _DK_check_for_first_person_barrack_party(struct Thing *creatng);
DLLIMPORT void _DK_terminate_thing_spell_effect(struct Thing *creatng, long reason);
DLLIMPORT void _DK_creature_increase_level(struct Thing *creatng);
DLLIMPORT void _DK_thing_death_flesh_explosion(struct Thing *creatng);
DLLIMPORT void _DK_thing_death_gas_and_flesh_explosion(struct Thing *creatng);
DLLIMPORT void _DK_thing_death_smoke_explosion(struct Thing *creatng);
DLLIMPORT void _DK_thing_death_ice_explosion(struct Thing *creatng);
DLLIMPORT long _DK_creature_is_group_leader(struct Thing *creatng);
DLLIMPORT long _DK_update_creature_levels(struct Thing *creatng);
DLLIMPORT long _DK_update_creature(struct Thing *creatng);
DLLIMPORT void _DK_process_thing_spell_effects(struct Thing *creatng);
DLLIMPORT void _DK_apply_damage_to_thing_and_display_health(struct Thing *creatng, long a1, char reason);
DLLIMPORT long _DK_creature_is_ambulating(struct Thing *creatng);
DLLIMPORT void _DK_check_for_door_collision_at(struct Thing *creatng, struct Coord3d *pos, long blocked_flags);
DLLIMPORT void _DK_update_tunneller_trail(struct Thing *creatng);
DLLIMPORT void _DK_draw_swipe(void);
DLLIMPORT void _DK_go_to_next_creature_of_breed_and_job(long crmodel, long job_idx);
/******************************************************************************/
/**
 * Returns creature health scaled 0..1000.
 * @param thing The creature thing.
 * @return Health value, not always in range of 0..1000.
 * @note Dying creatures may return negative health, and in some rare cases creatures
 *  can have more health than their max.
 */
int get_creature_health_permil(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    HitPoints health,max_health;
    health = thing->health * 1000;
    max_health = cctrl->max_health;
    if (max_health < 1)
        max_health = 1;
    return health/max_health;
}

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
  if ((thing->class_id == TCls_Object) && object_is_mature_food(thing))
    return true;
  return false;
}

TbBool creatre_is_for_dungeon_diggers_list(const struct Thing *creatng)
{
    if (is_hero_thing(creatng))
        return false;
    return  (creatng->model == get_players_special_digger_model(creatng->owner));
}

/**
 * Creates a barracks party, when creature being possessed is barracking.
 * @param grthing
 * @return Amount of creatures in the party, including the leader.
 */
long check_for_first_person_barrack_party(struct Thing *grthing)
{
    //return _DK_check_for_first_person_barrack_party(grthing);
    if (!thing_is_creature(grthing))
    {
        SYNCDBG(2,"The %s cannot lead a barracks party", thing_model_name(grthing));
        return 0;
    }
    struct Room *room;
    room = get_room_thing_is_on(grthing);
    if (!room_still_valid_as_type_for_thing(room, RoK_BARRACKS, grthing))
    {
        SYNCDBG(2,"Room %s owned by player %d does not allow the %s index %d owner %d to lead a party",room_code_name(room->kind),(int)room->owner,thing_model_name(grthing),(int)grthing->index,(int)grthing->owner);
        return 0;
    }
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    long i, n;
    n = 0;
    i = room->creatures_list;
    k = 0;
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (!creature_control_exists(cctrl))
        {
            ERRORLOG("Jump to invalid creature %d detected",(int)i);
            break;
        }
        i = cctrl->next_in_room;
        // Per creature code
        if (thing->index != grthing->index) {
            if (n == 0) {
                add_creature_to_group_as_leader(grthing, thing);
                n++;
            } else {
                add_creature_to_group(thing, grthing);
            }
            n++;
        }
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
    return n;
}

TbBool control_creature_as_controller(struct PlayerInfo *player, struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    struct InitLight ilght;
    struct Camera *cam;
    //return _DK_control_creature_as_controller(player, thing);
    if (((thing->owner != player->id_number) && (player->work_state != PSt_FreePossession))
      || !thing_can_be_controlled_as_controller(thing))
    {
      if (!control_creature_as_passenger(player, thing))
        return false;
      cam = player->acamera;
      crstat = creature_stats_get(get_players_special_digger_model(player->id_number));
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
    set_selected_creature(player, thing);
    cam = player->acamera;
    if (cam != NULL)
      player->view_mode_restore = cam->viewType;
    thing->alloc_flags |= TAlF_IsControlled;
    thing->field_4F |= TF4F_Unknown01;
    set_start_state(thing);
    set_player_mode(player, PVT_CreatureContrl);
    if (thing_is_creature(thing))
    {
        cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        check_for_first_person_barrack_party(thing);
        if (creature_is_group_member(thing)) {
            make_group_member_leader(thing);
        }
    }
    LbMemorySet(&ilght, 0, sizeof(struct InitLight));
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
    if ((thing->owner != player->id_number) && (player->work_state != PSt_FreeCtrlPassngr))
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
    set_selected_thing(player, thing);
    cam = player->acamera;
    if (cam != NULL)
      player->view_mode_restore = cam->viewType;
    set_player_mode(player, PVT_CreaturePasngr);
    thing->field_4F |= TF4F_Unknown01;
    return true;
}

void free_swipe_graphic(void)
{
    SYNCDBG(6,"Starting");
    if (game.loaded_swipe_idx != -1)
    {
        LbDataFreeAll(swipe_load_file);
        game.loaded_swipe_idx = -1;
    }
    LbSpriteClearAll(swipe_setup_sprites);
}

TbBool load_swipe_graphic_for_creature(const struct Thing *thing)
{
    int swpe_idx;
    int i;
    SYNCDBG(6,"Starting for %s",thing_model_name(thing));
    //_DK_load_swipe_graphic_for_creature(thing);

    i = creatures[thing->model%CREATURE_TYPES_COUNT].swipe_idx;
    if ((i == 0) || (game.loaded_swipe_idx == i))
        return true;
    free_swipe_graphic();
    swpe_idx = i;
    {
        struct TbLoadFiles *t_lfile;
        t_lfile = &swipe_load_file[0];
#ifdef SPRITE_FORMAT_V2
        sprintf(t_lfile->FName, "data/swipe%02d-%d.dat", swpe_idx, 32);
        t_lfile++;
        sprintf(t_lfile->FName, "data/swipe%02d-%d.tab", swpe_idx, 32);
        t_lfile++;
#else
        sprintf(t_lfile->FName, "data/swipe%02d.dat", swpe_idx);
        t_lfile++;
        sprintf(t_lfile->FName, "data/swipe%02d.tab", swpe_idx);
        t_lfile++;
#endif
    }
    if ( LbDataLoadAll(swipe_load_file) )
    {
        free_swipe_graphic();
        ERRORLOG("Unable to load swipe graphics for %s",thing_model_name(thing));
        return false;
    }
    LbSpriteSetupAll(swipe_setup_sprites);
    game.loaded_swipe_idx = swpe_idx;
    return true;
}

void draw_swipe_graphic(void)
{
    struct PlayerInfo *myplyr;
    myplyr = get_my_player();
    //_DK_draw_swipe();
    struct Thing *thing;
    thing = thing_get(myplyr->controlled_thing_idx);
    if (thing_is_creature(thing))
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        if ((cctrl->instance_id == CrInst_SWING_WEAPON_SWORD)
         || (cctrl->instance_id == CrInst_SWING_WEAPON_FIST)
         || (cctrl->instance_id == CrInst_FIRST_PERSON_DIG))
        {
            struct TbSprite *startspr;
            struct TbSprite *endspr;
            struct TbSprite *sprlist;
            long allwidth;
            long i,n;
            lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
            n = (int)cctrl->inst_turn * (5 << 8) / cctrl->inst_total_turns;
            allwidth = 0;
            i = abs(n) >> 8;
            if (i >= SWIPE_SPRITE_FRAMES)
                i = SWIPE_SPRITE_FRAMES-1;
            sprlist = &swipe_sprites[SWIPE_SPRITES_X*SWIPE_SPRITES_Y*i];
            startspr = &sprlist[1];
            endspr = &sprlist[1];
            for (n=0; n < SWIPE_SPRITES_X; n++)
            {
                allwidth += endspr->SWidth;
                endspr++;
            }
            int units_per_px;
            units_per_px = (LbScreenWidth()*59/64) * 16 / allwidth;
            int scrpos_x, scrpos_y;
            scrpos_y = (MyScreenHeight * 16 / units_per_px - (startspr->SHeight + endspr->SHeight)) / 2;
            struct TbSprite *spr;
            if ((myplyr->field_1 & 4) != 0)
            {
                int delta_y;
                delta_y = sprlist[1].SHeight;
                for (i=0; i < SWIPE_SPRITES_X*SWIPE_SPRITES_Y; i+=SWIPE_SPRITES_X)
                {
                    spr = &startspr[i];
                    scrpos_x = (MyScreenWidth * 16 / units_per_px - allwidth) / 2;
                    for (n=0; n < SWIPE_SPRITES_X; n++)
                    {
                        LbSpriteDrawResized(scrpos_x * units_per_px / 16, scrpos_y * units_per_px / 16, units_per_px, spr);
                        scrpos_x += spr->SWidth;
                        spr++;
                    }
                    scrpos_y += delta_y;
                }
            } else
            {
                int delta_y;
                lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4 | Lb_SPRITE_FLIP_HORIZ;
                for (i=0; i < SWIPE_SPRITES_X*SWIPE_SPRITES_Y; i+=SWIPE_SPRITES_X)
                {
                    spr = &sprlist[SWIPE_SPRITES_X+i];
                    delta_y = spr->SHeight;
                    scrpos_x = (MyScreenWidth * 16 / units_per_px - allwidth) / 2;
                    for (n=0; n < SWIPE_SPRITES_X; n++)
                    {
                        LbSpriteDrawResized(scrpos_x * units_per_px / 16, scrpos_y * units_per_px / 16, units_per_px, spr);
                        scrpos_x += spr->SWidth;
                        spr--;
                    }
                    scrpos_y += delta_y;
                }
            }
            lbDisplay.DrawFlags = 0;
            return;
        }
    }
    myplyr->field_1 ^= (myplyr->field_1 ^ 4 * UNSYNC_RANDOM(4)) & 4;
}

long creature_available_for_combat_this_turn(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    // Check once per 8 turns
    if (((game.play_gameturn + creatng->index) & 7) != 0)
    {
        // On first turn in a state, check anyway
        if (game.play_gameturn - cctrl->tasks_check_turn > 1) {
            return false;
        }
    }
    if (creature_is_fleeing_combat(creatng) || creature_affected_by_spell(creatng, SplK_Chicken)) {
        return false;
    }
    if (creature_is_being_unconscious(creatng) || creature_is_dying(creatng)) {
        return false;
    }
    if (thing_is_picked_up(creatng) || creature_is_being_dropped(creatng)) {
        return false;
    }
    if ((creatng->owner == game.neutral_player_num) || ((cctrl->flgfield_1 & CCFlg_NoCompControl) != 0)) {
        return false;
    }
    CrtrStateId i;
    i = get_creature_state_besides_interruptions(creatng);
    return can_change_from_state_to(creatng, i, CrSt_CreatureInCombat);
}

struct Thing *get_players_soul_container_creature_can_see(struct Thing *creatng, PlayerNumber heart_owner)
{
    struct Thing * heartng;
    int dist;
    heartng = get_player_soul_container(heart_owner);
    if (!thing_exists(heartng))
    {
        SYNCDBG(7,"The player %d has no heart",(int)heart_owner);
        return INVALID_THING;
    }
    dist = get_combat_distance(creatng, heartng);
    if (creature_can_see_combat_path(creatng, heartng, dist) > AttckT_Unset) {
        SYNCDBG(7,"The %s index %d owned by player %d can see player %d %s index %d at distance %d",
            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,
            (int)heartng->owner,thing_model_name(heartng),(int)heartng->index,(int)dist);
        return heartng;
    }
    if (creature_can_hear_within_distance(creatng, dist))
    {
        SYNCDBG(7,"The %s index %d owned by player %d can hear player %d %s index %d at distance %d",
            thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,
            (int)heartng->owner,thing_model_name(heartng),(int)heartng->index,(int)dist);
        return heartng;
    }
    SYNCDBG(17,"The %s index %d owned by player %d can't see player %d %s index %d at distance %d",
        thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,
        (int)heartng->owner,thing_model_name(heartng),(int)heartng->index,(int)dist);
    return INVALID_THING;
}

/**
 *
 * @param creatng
 * @return
 * @note originally named get_enemy_dungeon_heart_creature_can_see()
 */
struct Thing *get_enemy_soul_container_creature_can_see(struct Thing *creatng)
{
    PlayerNumber enemy_idx;

    SYNCDBG(17, "Starting");

    //return _DK_get_enemy_dungeon_heart_creature_can_see(creatng);

    assert(DUNGEONS_COUNT == PLAYERS_COUNT);

    for (enemy_idx = 0; enemy_idx < DUNGEONS_COUNT; enemy_idx++)
    {
        if (players_are_enemies(creatng->owner, enemy_idx))
        {
            struct Thing * heartng;
            heartng = get_players_soul_container_creature_can_see(creatng, enemy_idx);
            if (!thing_is_invalid(heartng))
            {
                return heartng;
            }
        }
    }

    return INVALID_THING;
}

void set_creature_combat_object_state(struct Thing *creatng, struct Thing *obthing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->battle_enemy_idx = obthing->index;
    cctrl->long_9E = obthing->creation_turn;
    cctrl->field_AA = 0;
    cctrl->combat_flags |= CmbtF_ObjctFight;
    const struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    if ((crstat->attack_preference == AttckT_Ranged)
      && creature_has_ranged_object_weapon(creatng)) {
        cctrl->combat_state_id = ObjCmbtSt_Ranged;
    } else {
        cctrl->combat_state_id = ObjCmbtSt_Melee;
    }
}

TbBool set_creature_object_combat(struct Thing *creatng, struct Thing *obthing)
{
    SYNCDBG(8,"Starting");
    //return _DK_set_creature_object_combat(creatng, obthing);
    if (!external_set_thing_state(creatng, CrSt_CreatureObjectCombat)) {
        return false;
    }
    set_creature_combat_object_state(creatng, obthing);
    SYNCDBG(19,"Finished");
    return true;
}

void set_creature_combat_door_state(struct Thing *creatng, struct Thing *obthing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    cctrl->battle_enemy_idx = obthing->index;
    cctrl->long_9E = obthing->creation_turn;
    cctrl->field_AA = 0;
    cctrl->combat_flags |= CmbtF_DoorFight;
    const struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    if ((crstat->attack_preference == AttckT_Ranged)
      && creature_has_ranged_object_weapon(creatng)) {
        cctrl->combat_state_id = ObjCmbtSt_Ranged;
    } else {
        cctrl->combat_state_id = ObjCmbtSt_Melee;
    }
}

TbBool set_creature_door_combat(struct Thing *creatng, struct Thing *obthing)
{
    SYNCDBG(8,"Starting");
    //_DK_set_creature_door_combat(creatng, obthing);
    if (!external_set_thing_state(creatng, CrSt_CreatureDoorCombat)) {
        SYNCDBG(8,"Cannot enter door combat");
        return false;
    }
    set_creature_combat_door_state(creatng, obthing);
    SYNCDBG(19,"Finished");
    return true;
}

void food_eaten_by_creature(struct Thing *foodtng, struct Thing *creatng)
{
    //_DK_food_eaten_by_creature(foodtng, creatng);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->instance_id == CrInst_NULL)
    {
        set_creature_instance(creatng, CrInst_EAT, 1, 0, 0);
    } else
    {
        if (cctrl->hunger_amount > 0) {
            cctrl->hunger_amount--;
        } else
        if (cctrl->hunger_loss < 255) {
              cctrl->hunger_loss++;
        }
        apply_health_to_thing_and_display_health(creatng, game.food_health_gain);
        cctrl->hunger_level = 0;
    }
    // Food is destroyed just below, so the sound must be made by creature
    thing_play_sample(creatng, 112+UNSYNC_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(creatng);
    anger_apply_anger_to_creature(creatng, crstat->annoy_eat_food, AngR_Hungry, 1);
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(creatng->owner);
    if (!dungeon_invalid(dungeon)) {
        dungeon->lvstats.chickens_eaten++;
    }
    if (thing_is_creature(foodtng))
    {
        foodtng->health = -1;
    } else
    {
        delete_thing_structure(foodtng, 0);
    }
}

void anger_apply_anger_to_creature_f(struct Thing *creatng, long anger, AnnoyMotive reason, long a3, const char *func_name)
{
    //_DK_anger_apply_anger_to_creature(creatng, anger, reason, a3); return;
    SYNCDBG(17,"The %s index %d owner %d will be applied with %d anger",
        thing_model_name(creatng),(int)creatng->index,(int)creatng->owner,(int)anger);
    if (!creature_can_get_angry(creatng)) {
        return;
    }
    if (anger > 0)
    {
        anger_increase_creature_anger_f(creatng, anger, reason, func_name);
        if (reason != AngR_Other)
        {
            if (anger_free_for_anger_increase(creatng))
            {
                long angrpart;
                angrpart = 32 * anger / 256;
                anger_increase_creature_anger_f(creatng, angrpart, AngR_Other, func_name);
            }
        }
    } else
    if (anger < 0)
    {
        anger_reduce_creature_anger_f(creatng, anger, reason, func_name);
        if (reason == AngR_Other)
        {
            AnnoyMotive reaspart;
            long angrpart;
            angrpart = 32 * anger / 256;
            for (reaspart = 1; reaspart < AngR_Other; reaspart++) {
                anger_reduce_creature_anger_f(creatng, angrpart, reaspart, func_name);
            }
        }
    }
}

/**
 * Returns if a creature is affected by given spell.
 * @param thing The creature thing.
 * @param spkind The spell, from SpellKind enumeration.
 * @return True if the creature is affected, false otherwise.
 */
TbBool creature_affected_by_spell(const struct Thing *thing, SpellKind spkind)
{
    const struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    switch (spkind)
    {
    case SplK_Freeze:
        return ((cctrl->stateblock_flags & CCSpl_Freeze) != 0);
    case SplK_Armour:
        return ((cctrl->spell_flags & CSAfF_Armour) != 0);
    case SplK_Rebound:
        return ((cctrl->spell_flags & CSAfF_Rebound) != 0);
    case SplK_Invisibility:
        return ((cctrl->spell_flags & CSAfF_Invisibility) != 0);
    case SplK_Teleport:
        return ((cctrl->stateblock_flags & CCSpl_Teleport) != 0);
    case SplK_Speed:
        return ((cctrl->spell_flags & CSAfF_Speed) != 0);
    case SplK_Slow:
        return ((cctrl->spell_flags & CSAfF_Slow) != 0);
    case SplK_Fly:
        return ((cctrl->spell_flags & CSAfF_Flying) != 0);
    case SplK_Sight:
        return ((cctrl->spell_flags & CSAfF_Sight) != 0);
    case SplK_Disease:
        return ((cctrl->spell_flags & CSAfF_Disease) != 0);
    case SplK_Chicken:
        return ((cctrl->spell_flags & CSAfF_Chicken) != 0);
    // Handle spells with no continuous effect
    case SplK_Lightning:
    case SplK_Heal:
    case SplK_Missile:
    case SplK_NavigMissile:
    case SplK_Grenade:
    case SplK_WordOfPower:
    case SplK_TimeBomb:
    case SplK_Fireball:
    case SplK_FireBomb:
    case SplK_FlameBreath:
    case SplK_Drain:
    case SplK_PoisonCloud:
        return ((cctrl->spell_flags & CSAfF_PoisonCloud) != 0);
    case SplK_Fear:
        return false;//TODO CREATURE_SPELL find out how to check this
    case SplK_Wind:
        return false;//TODO CREATURE_SPELL find out how to check this
    case SplK_Light:
        return false;//TODO CREATURE_SPELL find out how to check this
    case SplK_Hailstorm:
        return false;//TODO CREATURE_SPELL find out how to check this
    case SplK_CrazyGas:
        return false;//TODO CREATURE_SPELL find out how to check this
    default:
        SYNCDBG(3,"Unrecognized spell kind %d",(int)spkind);
        return false;
    }

}

TbBool creature_affected_by_slap(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    return (cctrl->slap_turns != 0);
}

/**
 * Returns remaining duration of a spell casted on a thing.
 * @param thing The thing which can have spells casted on.
 * @param spkind The spell kind to be checked.
 * @see thing_affected_by_spell()
 */
GameTurnDelta get_spell_duration_left_on_thing_f(const struct Thing *thing, SpellKind spkind, const char *func_name)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("%s: Invalid creature control for thing %d",func_name,(int)thing->index);
        return 0;
    }
    long i;
    for (i=0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        struct CastedSpellData *cspell;
        cspell = &cctrl->casted_spells[i];
        if (cspell->spkind == spkind) {
            return cspell->duration;
        }
    }
    if (strcmp(func_name, "thing_affected_by_spell") != 0)
        ERRORLOG("%s: No spell of type %d on %s index %d",func_name,(int)spkind,thing_model_name(thing),(int)thing->index);
    return 0;
}

/**
 * Returns if given spell is within list of spells affected by a thing.
 * @param thing The thing which can have spells casted on.
 * @param spkind The spell kind to be checked.
 * @see get_spell_duration_left_on_thing() to get remaining time of the affection
 * @see creature_affected_by_spell() to get more reliable info for creatures
 */
TbBool thing_affected_by_spell(const struct Thing *thing, SpellKind spkind)
{
    return (get_spell_duration_left_on_thing(thing, spkind) > 0);
}

long get_free_spell_slot(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl *cctrl;
    struct CastedSpellData *cspell;
    long ci,cval;
    long i,k;
    cctrl = creature_control_get_from_thing(creatng);
    cval = LONG_MAX;
    ci = -1;
    for (i=0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        cspell = &cctrl->casted_spells[i];
        // If there's unused slot, return it immediately
        if (cspell->spkind == SplK_None)
        {
            return i;
        }
        // Otherwise, select the one making minimum damage
        k = abs(cspell->duration);
        if (k < cval)
        {
            cval = k;
            ci = i;
        }
    }
    // Terminate the min damage effect and return its slot index
    cspell = &cctrl->casted_spells[ci];
    terminate_thing_spell_effect(creatng, cspell->spkind);
    for (i=0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        cspell = &cctrl->casted_spells[i];
        if (cspell->spkind == SplK_None)
        {
            return i;
        }
    }
    ERRORLOG("Spell effect has been terminated, but still its slot (%ld) isn't empty!",ci);
    return ci;
}

long get_spell_slot(const struct Thing *thing, SpellKind spkind)
{
    struct CreatureControl *cctrl;
    struct CastedSpellData *cspell;
    long i;
    cctrl = creature_control_get_from_thing(thing);
    for (i=0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        cspell = &cctrl->casted_spells[i];
        // If there is a slot with required spell
        if (cspell->spkind == spkind)
        {
            return i;
        }
    }
    // If spell not found
    return -1;
}

TbBool fill_spell_slot(struct Thing *thing, long slot_idx, SpellKind spell_idx, long spell_power)
{
    struct CreatureControl *cctrl;
    struct CastedSpellData *cspell;
    if ((slot_idx < 0) || (slot_idx >= CREATURE_MAX_SPELLS_CASTED_AT))
        return false;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return false;
    cspell = &cctrl->casted_spells[slot_idx];
    cspell->spkind = spell_idx;
    cspell->duration = spell_power;
    return true;
}

TbBool free_spell_slot(struct Thing *thing, long slot_idx)
{
    struct CreatureControl *cctrl;
    struct CastedSpellData *cspell;
    if ((slot_idx < 0) || (slot_idx >= CREATURE_MAX_SPELLS_CASTED_AT))
        return false;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return false;
    cspell = &cctrl->casted_spells[slot_idx];
    cspell->spkind = SplK_None;
    cspell->duration = 0;
    return true;
}

void first_apply_spell_effect_to_thing(struct Thing *thing, SpellKind spell_idx, long spell_lev)
{
    struct CreatureControl *cctrl;
    struct SpellConfig *splconf;
    const struct MagicStats *pwrdynst;
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
            fill_spell_slot(thing, i, spell_idx, splconf->duration);
            cctrl->stateblock_flags |= CCSpl_Freeze;
            if ((thing->movement_flags & TMvF_Flying) != 0)
            {
                cctrl->spell_flags |= CSAfF_Grounded;
                thing->movement_flags &= ~TMvF_Flying;
            }
            creature_set_speed(thing, 0);
        }
        break;
    case SplK_Armour:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            pwrdynst = get_power_dynamic_stats(PwrK_PROTECT);
            fill_spell_slot(thing, i, spell_idx, pwrdynst->strength[spell_lev]);
            n = 0;
            cctrl->spell_flags |= CSAfF_Armour;
            for (k=0; k < 3; k++)
            {
                set_coords_to_cylindric_shift(&pos, &thing->mappos, 32, n, k * (thing->clipbox_size_yz >> 1) );
                ntng = create_object(&pos, 51, thing->owner, -1);
                if (!thing_is_invalid(ntng))
                {
                    cctrl->spell_tngidx_armour[k] = ntng->index;
                    ntng->health = pwrdynst->strength[spell_lev] + 1;
                    ntng->word_13 = thing->index;
                    ntng->byte_15 = k;
                    ntng->move_angle_xy = thing->move_angle_xy;
                    ntng->move_angle_z = thing->move_angle_z;
                    angles_to_vector(ntng->move_angle_xy, ntng->move_angle_z, 32, &cvect);
                    ntng->veloc_push_add.x.val += cvect.x;
                    ntng->veloc_push_add.y.val += cvect.y;
                    ntng->veloc_push_add.z.val += cvect.z;
                    ntng->state_flags |= TF1_PushAdd;
                }
                n += 2*LbFPMath_PI/3;
            }
        }
        break;
    case SplK_Rebound:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            fill_spell_slot(thing, i, spell_idx, splconf->duration);
            cctrl->spell_flags |= CSAfF_Rebound;
        }
        break;
    case SplK_Heal:
        pwrdynst = get_power_dynamic_stats(PwrK_HEALCRTR);
        i = saturate_set_signed(thing->health + pwrdynst->strength[spell_lev],16);
        if (i < 0)
        {
          thing->health = 0;
        } else {
          thing->health = min(i,cctrl->max_health);
        }
        cctrl->field_2B0 = 7;
        cctrl->field_2AE = pwrdynst->time;
        break;
    case SplK_Invisibility:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            pwrdynst = get_power_dynamic_stats(PwrK_CONCEAL);
            fill_spell_slot(thing, i, spell_idx, pwrdynst->strength[spell_lev]);
            cctrl->spell_flags |= CSAfF_Invisibility;
            cctrl->force_visible = 0;
        }
        break;
    case SplK_Teleport:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            fill_spell_slot(thing, i, spell_idx, splconf->duration);
            cctrl->stateblock_flags |= CCSpl_Teleport;
        }
        break;
    case SplK_Speed:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            pwrdynst = get_power_dynamic_stats(PwrK_SPEEDCRTR);
            fill_spell_slot(thing, i, spell_idx, pwrdynst->strength[spell_lev]);
            cctrl->spell_flags |= CSAfF_Speed;
            cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        }
        break;
    case SplK_Slow:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            fill_spell_slot(thing, i, spell_idx, splconf->duration);
            cctrl->spell_flags |= CSAfF_Slow;
            cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        }
        break;
    case SplK_Fly:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            fill_spell_slot(thing, i, spell_idx, splconf->duration);
            cctrl->spell_flags |= CSAfF_Flying;
            thing->movement_flags |= TMvF_Flying;
        }
        break;
    case SplK_Sight:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            fill_spell_slot(thing, i, spell_idx, splconf->duration);
            cctrl->spell_flags |= CSAfF_Sight;
        }
        break;
    case SplK_Disease:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            pwrdynst = get_power_dynamic_stats(PwrK_DISEASE);
          fill_spell_slot(thing, i, spell_idx, pwrdynst->strength[spell_lev]);
          n = 0;
          cctrl->spell_flags |= CSAfF_Disease;
          cctrl->disease_caster_plyridx = thing->owner;
          cctrl->disease_start_turn = game.play_gameturn;
          for (k=0; k < 3; k++)
          {
              pos.x.val = thing->mappos.x.val;
              pos.y.val = thing->mappos.y.val;
              pos.z.val = thing->mappos.z.val;
              pos.x.val += distance_with_angle_to_coord_x(32,n);
              pos.y.val += distance_with_angle_to_coord_y(32,n);
              pos.z.val += k * (long)(thing->clipbox_size_yz >> 1);
              ntng = create_object(&pos, 112, thing->owner, -1);
              if (!thing_is_invalid(ntng))
              {
                cctrl->spell_tngidx_disease[k] = ntng->index;
                ntng->health = pwrdynst->strength[spell_lev] + 1;
                ntng->word_13 = thing->index;
                ntng->byte_15 = k;
                ntng->move_angle_xy = thing->move_angle_xy;
                ntng->move_angle_z = thing->move_angle_z;
                angles_to_vector(ntng->move_angle_xy, ntng->move_angle_z, 32, &cvect);
                ntng->veloc_push_add.x.val += cvect.x;
                ntng->veloc_push_add.y.val += cvect.y;
                ntng->veloc_push_add.z.val += cvect.z;
                ntng->state_flags |= TF1_PushAdd;
              }
              n += 2*LbFPMath_PI/3;
          }
        }
        break;
    case SplK_Chicken:
        i = get_free_spell_slot(thing);
        if (i != -1)
        {
            pwrdynst = get_power_dynamic_stats(PwrK_CHICKEN);
            fill_spell_slot(thing, i, spell_idx, pwrdynst->strength[spell_lev]);
            external_set_thing_state(thing, CrSt_CreatureChangeToChicken);
            cctrl->countdown_282 = 10;
            cctrl->spell_flags |= CSAfF_Chicken;
        }
        break;
    default:
        WARNLOG("No action for spell %d at level %d",(int)spell_idx,(int)spell_lev);
        break;
    }
}

void reapply_spell_effect_to_thing(struct Thing *thing, long spell_idx, long spell_lev, long idx)
{
    struct CreatureControl *cctrl;
    struct SpellConfig *splconf;
    const struct MagicStats *pwrdynst;
    long i;
    cctrl = creature_control_get_from_thing(thing);
    if (spell_lev > SPELL_MAX_LEVEL)
        spell_lev = SPELL_MAX_LEVEL;
    struct CastedSpellData * cspell;
    cspell = &cctrl->casted_spells[idx];
    // This pointer may be invalid if spell_idx is incorrect. But we're using it only when correct.
    splconf = &game.spells_config[spell_idx];
    switch (spell_idx)
    {
    case SplK_Freeze:
        cspell->duration = splconf->duration;
        creature_set_speed(thing, 0);
        break;
    case SplK_Armour:
        pwrdynst = get_power_dynamic_stats(PwrK_PROTECT);
        cspell->duration = pwrdynst->strength[spell_lev];
        break;
    case SplK_Rebound:
        cspell->duration = splconf->duration;
        break;
    case SplK_Heal:
        pwrdynst = get_power_dynamic_stats(PwrK_HEALCRTR);
        i = saturate_set_signed(thing->health + pwrdynst->strength[spell_lev],16);
        if (i < 0)
        {
          thing->health = 0;
        } else {
          thing->health = min(i,cctrl->max_health);
        }
        cctrl->field_2B0 = 7;
        cctrl->field_2AE = pwrdynst->time;
        break;
    case SplK_Invisibility:
        pwrdynst = get_power_dynamic_stats(PwrK_CONCEAL);
        cspell->duration = pwrdynst->strength[spell_lev];
        break;
    case SplK_Teleport:
        cspell->duration = splconf->duration;
        break;
    case SplK_Speed:
        pwrdynst = get_power_dynamic_stats(PwrK_SPEEDCRTR);
        cspell->duration = pwrdynst->strength[spell_lev];
        break;
    case SplK_Slow:
        cspell->duration = splconf->duration;
        break;
    case SplK_Light:
        cspell->duration = splconf->duration;
        break;
    case SplK_Fly:
        cspell->duration = splconf->duration;
        break;
    case SplK_Sight:
        cspell->duration = splconf->duration;
        break;
    case SplK_Disease:
        pwrdynst = get_power_dynamic_stats(PwrK_DISEASE);
        cspell->duration = pwrdynst->strength[spell_lev];
        cctrl->disease_caster_plyridx = thing->owner;
        break;
    case SplK_Chicken:
        external_set_thing_state(thing, CrSt_CreatureChangeToChicken);
        cctrl->countdown_282 = 10;
        pwrdynst = get_power_dynamic_stats(PwrK_CHICKEN);
        cspell->duration = pwrdynst->strength[spell_lev];
        break;
    default:
        WARNLOG("No action for spell %d at level %d",(int)spell_idx,(int)spell_lev);
        break;
    }
}

void apply_spell_effect_to_thing(struct Thing *thing, SpellKind spell_idx, long spell_lev)
{
    struct CreatureControl *cctrl;
    long i;
    // Make sure the creature level isn't larger than max spell level
    if (spell_lev > SPELL_MAX_LEVEL)
        spell_lev = SPELL_MAX_LEVEL;
    SYNCDBG(6,"Applying %s to %s index %d",spell_code_name(spell_idx),thing_model_name(thing),(int)thing->index);
    //_DK_apply_spell_effect_to_thing(thing, spell_idx, spell_lev); return;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature tried to accept spell %s",spell_code_name(spell_idx));
        return;
    }
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

void terminate_thing_spell_effect(struct Thing *thing, SpellKind spkind)
{
    struct CreatureControl *cctrl;
    int slot_idx;
    ThingIndex eff_idx;
    long i;
    //_DK_terminate_thing_spell_effect(thing, spkind);
    TRACE_THING(thing);
    slot_idx = get_spell_slot(thing, spkind);
    cctrl = creature_control_get_from_thing(thing);
    switch ( spkind )
    {
    case SplK_Freeze:
        cctrl->stateblock_flags &= ~CCSpl_Freeze;
        if ((cctrl->spell_flags & CSAfF_Grounded) != 0)
        {
            thing->movement_flags |= TMvF_Flying;
            cctrl->spell_flags &= ~CSAfF_Grounded;
        }
        break;
    case SplK_Armour:
        cctrl->spell_flags &= ~CSAfF_Armour;
        for (i=0; i < 3; i++)
        {
            eff_idx = cctrl->spell_tngidx_armour[i];
            if (eff_idx > 0) {
                struct Thing * efftng;
                efftng = thing_get(eff_idx);
                delete_thing_structure(efftng, 0);
                cctrl->spell_tngidx_armour[i] = 0;
            }
        }
        break;
    case SplK_Rebound:
        cctrl->spell_flags &= ~CSAfF_Rebound;
        break;
    case SplK_Invisibility:
        cctrl->spell_flags &= ~CSAfF_Invisibility;
        cctrl->force_visible = 0;
        break;
    case SplK_Teleport:
        cctrl->stateblock_flags &= ~CCSpl_Teleport;
        break;
    case SplK_Speed:
        cctrl->spell_flags &= ~CSAfF_Speed;
        cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        break;
    case SplK_Slow:
        cctrl->spell_flags &= ~CSAfF_Slow;
        cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        break;
    case SplK_Fly:
        //TODO SPELLS Strange condition regarding the fly - verify why it's here
        if ((get_creature_model_flags(thing) & CMF_IsDiptera) == 0)
            thing->movement_flags &= ~TMvF_Flying;
        cctrl->spell_flags &= ~CSAfF_Flying;
        break;
    case SplK_Sight:
        cctrl->spell_flags &= ~CSAfF_Sight;
        break;
    case SplK_Disease:
        cctrl->spell_flags &= ~CSAfF_Disease;
        for (i=0; i < 3; i++)
        {
            eff_idx = cctrl->spell_tngidx_disease[i];
            if (eff_idx > 0) {
                struct Thing * efftng;
                efftng = thing_get(eff_idx);
                delete_thing_structure(efftng, 0);
                cctrl->spell_tngidx_disease[i] = 0;
            }
        }
        break;
    case SplK_Chicken:
        cctrl->spell_flags &= ~CSAfF_Chicken;
        external_set_thing_state(thing, CrSt_CreatureChangeFromChicken);
        cctrl->countdown_282 = 10;
        break;
    }
    if (slot_idx >= 0) {
        free_spell_slot(thing, slot_idx);
    }
}

void process_thing_spell_teleport_effects(struct Thing *thing, struct CastedSpellData *cspell)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    struct SpellConfig *splconf;
    splconf = &game.spells_config[SplK_Teleport];
    if (cspell->duration == splconf->duration / 2)
    {
        struct Coord3d pos;
        pos.x.val = subtile_coord_center(cctrl->teleport_x);
        pos.y.val = subtile_coord_center(cctrl->teleport_y);
        pos.z.val = get_floor_height_at(&pos);
        if (thing_in_wall_at(thing, &pos))
        {
            const struct Coord3d *newpos;
            newpos = NULL;
            {
                const struct Thing *lairtng;
                lairtng = thing_get(cctrl->lairtng_idx);
                if (thing_is_object(lairtng)) {
                    newpos = &lairtng->mappos;
                }
            }
            if (newpos == NULL)
            {
                newpos = dungeon_get_essential_pos(thing->owner);
            }
            pos.x.val = newpos->x.val;
            pos.y.val = newpos->y.val;
            pos.z.val = newpos->z.val;
        }
        pos.z.val += subtile_coord(2,0);
        move_thing_in_map(thing, &pos);
        ariadne_invalidate_creature_route(thing);
        check_map_explored(thing, pos.x.stl.num, pos.y.stl.num);
        if ((thing->movement_flags & TMvF_Flying) == 0)
        {
            thing->veloc_push_add.x.val += ACTION_RANDOM(193) - 96;
            thing->veloc_push_add.y.val += ACTION_RANDOM(193) - 96;
            thing->veloc_push_add.z.val += ACTION_RANDOM(96) + 40;
            thing->state_flags |= TF1_PushAdd;
        }
    }
}

void process_thing_spell_effects(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //_DK_process_thing_spell_effects(thing);
    cctrl = creature_control_get_from_thing(thing);
    int i;
    for (i=0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        struct CastedSpellData * cspell;
        cspell = &cctrl->casted_spells[i];
        if (cspell->spkind == SplK_None)
            continue;
        switch (cspell->spkind)
        {
        case SplK_Teleport:
            process_thing_spell_teleport_effects(thing, cspell);
            break;
        default:
            break;
        }
        cspell->duration--;
        if (cspell->duration <= 0) {
            terminate_thing_spell_effect(thing, cspell->spkind);
        }
    }
    // Slap is not in spell array, it is so common that has its own dedicated duration
    if (cctrl->slap_turns > 0)
    {
        cctrl->slap_turns--;
        if (cctrl->slap_turns <= 0) {
            cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        }
    }
}

void process_thing_spell_effects_while_blocked(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    int i;
    for (i=0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        struct CastedSpellData * cspell;
        cspell = &cctrl->casted_spells[i];
        if (cspell->spkind == SplK_None)
            continue;
        if (cspell->duration > 0) {
            cspell->duration--;
        }
    }
    // Slap is not in spell array, it is so common that has its own dedicated duration
    if (cctrl->slap_turns > 0) {
        cctrl->slap_turns--;
    }
}

short creature_take_wage_from_gold_pile(struct Thing *creatng,struct Thing *goldtng)
{
    struct CreatureStats *crstat;
    long i;
    crstat = creature_stats_get_from_thing(creatng);
    if (goldtng->creature.gold_carried <= 0)
    {
      ERRORLOG("GoldPile had no gold so was deleted.");
      delete_thing_structure(goldtng, 0);
      return false;
    }
    if (creatng->creature.gold_carried < crstat->gold_hold)
    {
      if (goldtng->creature.gold_carried+creatng->creature.gold_carried > crstat->gold_hold)
      {
        i = crstat->gold_hold-creatng->creature.gold_carried;
        creatng->creature.gold_carried += i;
        goldtng->creature.gold_carried -= i;
      } else
      {
        creatng->creature.gold_carried += goldtng->creature.gold_carried;
        delete_thing_structure(goldtng, 0);
      }
    }
    anger_apply_anger_to_creature(creatng, crstat->annoy_got_wage, AngR_NotPaid, 1);
    return true;
}

/**
 * Casts a spell by caster creature targeted at given thing, most likely using shot to transfer the spell.
 * @param castng The caster creature.
 * @param targetng The target thing.
 * @param spl_idx Spell index to be casted.
 * @param shot_lvl Spell level to be casted.
 */
void creature_cast_spell_at_thing(struct Thing *castng, struct Thing *targetng, long spl_idx, long shot_lvl)
{
    const struct SpellInfo *spinfo;
    unsigned char hit_type;
    if ((castng->alloc_flags & TAlF_IsControlled) != 0)
    {
        if (targetng->class_id == TCls_Object)
            hit_type = THit_CrtrsNObjcts;
        else
            hit_type = THit_CrtrsOnly;
    } else
    {
        if (targetng->class_id == TCls_Object)
            hit_type = THit_CrtrsNObjctsNotOwn;
        else
        if (targetng->owner == castng->owner)
            hit_type = THit_CrtrsOnly;
        else
            hit_type = THit_CrtrsOnlyNotOwn;
    }
    spinfo = get_magic_info(spl_idx);
    if (magic_info_is_invalid(spinfo))
    {
        ERRORLOG("The %s owned by player %d tried to cast invalid spell %d",thing_model_name(castng),(int)castng->owner,(int)spl_idx);
        return;
    }
    creature_fire_shot(castng, targetng, spinfo->shot_model, shot_lvl, hit_type);
}

/**
 * Casts a spell by caster creature targeted at given coordinates, most likely using shot to transfer the spell.
 * @param castng The caster creature.
 * @param spl_idx Spell index to be casted.
 * @param shot_lvl Spell level to be casted.
 */
void creature_cast_spell(struct Thing *castng, long spl_idx, long shot_lvl, long trg_x, long trg_y)
{
    const struct SpellInfo *spinfo;
    struct CreatureControl *cctrl;
    struct Thing *efthing;
    long i;
    //_DK_creature_cast_spell(caster, spl_idx, shot_lvl, trg_x, trg_y);
    spinfo = get_magic_info(spl_idx);
    cctrl = creature_control_get_from_thing(castng);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature tried to cast spell %d",(int)spl_idx);
        return;
    }
    if (spl_idx == SplK_Teleport)
    {
        cctrl->teleport_x = trg_x;
        cctrl->teleport_y = trg_y;
    }
    // Check if the spell can be fired as a shot
    if (spinfo->shot_model > 0)
    {
        if ((castng->alloc_flags & TAlF_IsControlled) != 0)
          i = THit_CrtrsNObjcts;
        else
          i = THit_CrtrsOnlyNotOwn;
        creature_fire_shot(castng, INVALID_THING, spinfo->shot_model, shot_lvl, i);
    } else
    // Check if the spell can be self-casted
    if (spinfo->caster_affected)
    {
        i = (long)spinfo->caster_affect_sound;
        if (i > 0)
          thing_play_sample(castng, i, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
        apply_spell_effect_to_thing(castng, spl_idx, cctrl->explevel);
    }
    // Check if the spell has an effect associated
    if (spinfo->cast_effect_model != 0)
    {
        efthing = create_effect(&castng->mappos, spinfo->cast_effect_model, castng->owner);
        if (!thing_is_invalid(efthing))
        {
          if (spinfo->cast_effect_model == 14)
            efthing->byte_16 = 3;
        }
    }
}

void update_creature_count(struct Thing *creatng)
{
    TRACE_THING(creatng);
    //_DK_update_creature_count(thing);
    if (!thing_exists(creatng)) {
        return;
    }
    if (is_hero_thing(creatng) || is_neutral_thing(creatng)) {
        return;
    }
    if (thing_is_picked_up(creatng) || creature_is_being_unconscious(creatng)) {
        return;
    }
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(creatng->owner);
    if (dungeon_invalid(dungeon)) {
        return;
    }
    int statyp;
    statyp = get_creature_state_type(creatng);
    dungeon->field_64[creatng->model][statyp]++;
    int job_idx;
    job_idx = get_creature_gui_job(creatng);
    if (can_thing_be_picked_up_by_player(creatng, creatng->owner))
    {
        if (!creature_is_dragging_or_being_dragged(creatng)) {
            dungeon->guijob_all_creatrs_count[creatng->model][job_idx]++;
        }
    }
    if (anger_is_creature_angry(creatng))
    {
        dungeon->guijob_angry_creatrs_count[creatng->model][job_idx]++;
    }
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
    i = thing->next_on_mapblk;
    if (thing->class_id == TCls_Object)
    {
      if ((thing->model == 43) && thing_touching_floor(thing))
        return thing;
      if (object_is_mature_food(thing))
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

struct Thing *find_interesting_object_laying_around_thing(struct Thing *creatng)
{
    long k;
    for (k=0; k < AROUND_TILES_COUNT; k++)
    {
        struct Map *mapblk;
        long stl_x,stl_y;
        stl_x = creatng->mappos.x.stl.num + around[k].delta_x;
        stl_y = creatng->mappos.y.stl.num + around[k].delta_y;
        mapblk = get_map_block_at(stl_x,stl_y);
        if (!map_block_invalid(mapblk))
        {
            if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
            {
                struct Thing *thing;
                thing = find_gold_pile_or_chicken_laying_on_mapblk(mapblk);
                if (!thing_is_invalid(thing))
                    return thing;
            }
        }
    }
    return INVALID_THING;
}

TbBool thing_can_be_eaten(struct Thing *thing)
{
    if (thing_is_mature_food(thing) || (thing_is_creature(thing) && creature_affected_by_spell(thing, SplK_Chicken)))
    {
        if (is_thing_directly_controlled(thing) || is_thing_passenger_controlled(thing) || thing_is_picked_up(thing)) {
            return false;
        }
        return true;
    }
    return false;
}

TbBool creature_pick_up_interesting_object_laying_nearby(struct Thing *creatng)
{
    struct Thing *tgthing;
    struct CreatureStats *crstat;
    tgthing = find_interesting_object_laying_around_thing(creatng);
    if (thing_is_invalid(tgthing)) {
        return false;
    }
    if (object_is_gold_laying_on_ground(tgthing))
    {
        crstat = creature_stats_get_from_thing(creatng);
        if (tgthing->valuable.gold_stored > 0)
        {
            if (creatng->creature.gold_carried < crstat->gold_hold)
            {
                if (crstat->gold_hold < tgthing->valuable.gold_stored + creatng->creature.gold_carried)
                {
                    long k;
                    k = crstat->gold_hold - creatng->creature.gold_carried;
                    creatng->creature.gold_carried += k;
                    tgthing->valuable.gold_stored -= k;
                } else
                {
                    creatng->creature.gold_carried += tgthing->valuable.gold_stored;
                    delete_thing_structure(tgthing, 0);
                }
            }
        } else
        {
            ERRORLOG("GoldPile with no gold!");
            delete_thing_structure(tgthing, 0);
        }
        anger_apply_anger_to_creature(creatng, crstat->annoy_got_wage, AngR_NotPaid, 1);
        return true;
    }
    if (thing_can_be_eaten(tgthing))
    {
        food_eaten_by_creature(tgthing, creatng);
        return true;
    }
    return false;
}

TngUpdateRet process_creature_state(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    unsigned long model_flags;
    long x,y;
    SYNCDBG(19,"Starting for %s index %d owned by player %d",thing_model_name(thing),(int)thing->index,(int)thing->owner);
    TRACE_THING(thing);
    //return _DK_process_creature_state(thing);
    cctrl = creature_control_get_from_thing(thing);
    model_flags = get_creature_model_flags(thing);

    process_person_moods_and_needs(thing);
    if (creature_available_for_combat_this_turn(thing))
    {
        TbBool fighting;
        fighting = creature_look_for_combat(thing);
        if (!fighting) {
            fighting = creature_look_for_enemy_heart_combat(thing);
        }
    }
    if ((cctrl->combat_flags & CmbtF_DoorFight) == 0)
    {
        if ((cctrl->collided_door_subtile > 0) && ((cctrl->flgfield_1 & CCFlg_NoCompControl) == 0))
        {
            if ( can_change_from_state_to(thing, thing->active_state, CrSt_CreatureDoorCombat) )
            {
                struct Thing *doortng;
                x = stl_num_decode_x(cctrl->collided_door_subtile);
                y = stl_num_decode_y(cctrl->collided_door_subtile);
                doortng = get_door_for_position(x,y);
                if (!thing_is_invalid(doortng))
                {
                    if (thing->owner != doortng->owner) {
                        set_creature_door_combat(thing, doortng);
                    }
                }
                cctrl->collided_door_subtile = 0;
            }
        }
    }
    if (creature_is_group_member(thing))
    {
        if (!creature_is_group_leader(thing)) {
            process_obey_leader(thing);
        }
    }
    if ((thing->active_state < 1) || (thing->active_state >= CREATURE_STATES_COUNT))
    {
        ERRORLOG("The %s index %d has illegal state[1], S=%d, TCS=%d, reset", thing_model_name(thing), (int)thing->index, (int)thing->active_state, (int)thing->continue_state);
        set_start_state(thing);
    }

    // Creatures that are not special diggers will pick up any nearby gold or food
    if (((thing->movement_flags & TMvF_Flying) == 0) && ((model_flags & CMF_IsSpecDigger) == 0))
    {
        if (!creature_is_being_unconscious(thing) && !creature_is_dying(thing) &&
            !thing_is_picked_up(thing) && !creature_is_being_dropped(thing))
        {
            creature_pick_up_interesting_object_laying_nearby(thing);
        }
    }
    // Enable this to know which function hangs on update_creature.
    //TODO CREATURE_AI rewrite state subfunctions so they won't hang
    //if (game.play_gameturn > 119800)
    SYNCDBG(18,"Executing state %s for %s index %d.",creature_state_code_name(thing->active_state),thing_model_name(thing),(int)thing->index);
    struct StateInfo *stati;
    stati = get_thing_active_state_info(thing);
    if (stati->process_state != NULL) {
        short k;
        k = stati->process_state(thing);
        if (k == CrStRet_Deleted) {
            SYNCDBG(18,"Finished with creature deleted");
            return TUFRet_Deleted;
        }
    }
    SYNCDBG(18,"Finished");
    return TUFRet_Modified;
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
TbBool update_kills_counters(struct Thing *victim, struct Thing *killer,
    PlayerNumber def_plyr_idx, CrDeathFlags flags)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(victim);
    if ((flags & CrDed_DiedInBattle) != 0)
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
    if ((cctrl->fighting_player_idx != -1) && (game.neutral_player_num != cctrl->fighting_player_idx))
    {
        return inc_player_kills_counter(cctrl->fighting_player_idx, victim);
    }
    return false;
}

long creature_is_ambulating(struct Thing *thing)
{
    //return _DK_creature_is_ambulating(thing);
    int n, i;
    n = get_creature_model_graphics(thing->model, CGI_Ambulate);
    i = convert_td_iso(n);
    if (i != thing->anim_sprite)
        return 0;
    return 1;
}

TbBool check_for_door_collision_at(struct Thing *thing, struct Coord3d *pos, unsigned long blocked_flags)
{
    SYNCDBG(18,"Starting for %s",thing_model_name(thing));
    //_DK_check_for_door_collision_at(thing, pos, a3); return;
    int nav_sizexy;
    nav_sizexy = thing_nav_sizexy(thing)/2;
    MapSubtlCoord start_x, end_x, start_y, end_y;
    start_x = coord_subtile(pos->x.val - nav_sizexy);
    end_x = coord_subtile(pos->x.val + nav_sizexy);
    start_y = coord_subtile(pos->y.val - nav_sizexy);
    end_y = coord_subtile(pos->y.val + nav_sizexy);
    MapSubtlCoord stl_x, stl_y;
    if ((blocked_flags & 0x01) != 0)
    {
        stl_x = end_x;
        if (thing->mappos.x.val >= pos->x.val)
            stl_x = start_x;
        for (stl_y = start_y; stl_y <= end_y; stl_y++)
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x, stl_y);
            if ((mapblk->flags & SlbAtFlg_IsDoor) != 0) {
                SYNCDBG(18,"Door collision at X with %s",thing_model_name(thing));
                struct CreatureControl *cctrl;
                cctrl = creature_control_get_from_thing(thing);
                cctrl->collided_door_subtile = get_subtile_number(stl_x, stl_y);
                return true;
            }
        }
    }
    if ((blocked_flags & 0x02) != 0)
    {
        stl_y = end_y;
        if (thing->mappos.y.val >= pos->y.val)
            stl_y = start_y;
        for (stl_x = start_x; stl_x <= end_x; stl_x++)
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x, stl_y);
            if ((mapblk->flags & SlbAtFlg_IsDoor) != 0) {
                SYNCDBG(18,"Door collision at Y with %s",thing_model_name(thing));
                struct CreatureControl *cctrl;
                cctrl = creature_control_get_from_thing(thing);
                cctrl->collided_door_subtile = get_subtile_number(stl_x, stl_y);
                return true;
            }
        }
    }
    SYNCDBG(18,"No door collision with %s",thing_model_name(thing));
    return false;
}

unsigned int get_creature_blocked_flags_at(struct Thing *thing, struct Coord3d *newpos)
{
    struct Coord3d pos;
    unsigned int flags;
    flags = 0;
    pos.x.val = newpos->x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val;
    if ( creature_cannot_move_directly_to(thing, &pos) ) {
        flags |= 0x01;
    }
    pos.x.val = thing->mappos.x.val;
    pos.y.val = newpos->y.val;
    pos.z.val = thing->mappos.z.val;
    if ( creature_cannot_move_directly_to(thing, &pos) ) {
        flags |= 0x02;
    }
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = newpos->z.val;
    if ( creature_cannot_move_directly_to(thing, &pos) ) {
        flags |= 0x04;
    }
    switch (flags)
    {
    case 0:
      if ( creature_cannot_move_directly_to(thing, newpos) ) {
          flags = 0x07;
      }
      break;
    case 1:
      pos.x.val = thing->mappos.x.val;
      pos.y.val = newpos->y.val;
      pos.z.val = newpos->z.val;
      if (creature_cannot_move_directly_to(thing, &pos) < 1) {
          flags = 0x01;
      } else {
          flags = 0x07;
      }
      break;
    case 2:
      pos.x.val = newpos->x.val;
      pos.y.val = thing->mappos.y.val;
      pos.z.val = newpos->z.val;
      if (creature_cannot_move_directly_to(thing, &pos) < 1) {
          flags = 0x02;
      } else {
          flags = 0x07;
      }
      break;
    case 4:
      pos.x.val = newpos->x.val;
      pos.y.val = newpos->y.val;
      pos.z.val = thing->mappos.z.val;
      if ( creature_cannot_move_directly_to(thing, &pos) ) {
          flags = 0x07;
      }
      break;
    }
    return flags;
}

void update_tunneller_trail(struct Thing *thing)
{
    //_DK_update_tunneller_trail(thing); return;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    // Shift all elements freeing first item
    int i;
    for (i = 4; i > 0; i--) {
        cctrl->party.member_pos_stl[i] = cctrl->party.member_pos_stl[i-1];
    }
    // Fill the first item
    cctrl->party.member_pos_stl[0] = get_subtile_number(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
}

long move_creature(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //return _DK_move_creature(thing);
    cctrl = creature_control_get_from_thing(thing);
    struct Coord3d *tngpos;
    struct Coord3d pvpos;
    int velo_x,velo_y,velo_z;
    tngpos = &thing->mappos;
    pvpos.x.val = tngpos->x.val;
    pvpos.y.val = tngpos->y.val;
    pvpos.z.val = tngpos->z.val;
    velo_x = thing->velocity.x.val;
    velo_y = thing->velocity.y.val;
    velo_z = thing->velocity.z.val;
    cctrl->flgfield_1 &= ~CCFlg_Unknown08;
    struct Coord3d nxpos;
    if (thing_in_wall_at(thing, &thing->mappos) && !creature_can_pass_throgh_wall_at(thing, &thing->mappos))
    {
        nxpos.x.val = tngpos->x.val;
        nxpos.y.val = tngpos->y.val;
        nxpos.z.val = tngpos->z.val;
        if (get_nearest_valid_position_for_creature_at(thing, &nxpos)) {
            move_thing_in_map(thing, &nxpos);
        }
        cctrl->flgfield_1 |= CCFlg_Unknown08;
    }
    if ((get_creature_model_flags(thing) & CMF_TremblingFat) != 0)
    {
      if (creature_is_ambulating(thing))
        {
            if (thing->field_48 > 3)
            {
                velo_y = 0;
                velo_x = 0;
            }
        }
    }
    if ((velo_x != 0) || (velo_y != 0) || (velo_z != 0))
    {
        if (velo_x < -256) {
            velo_x = -256;
        } else if (velo_x > 256) {
            velo_x = 256;
        }
        if (velo_y < -256) {
            velo_y = -256;
        } else if (velo_y > 256) {
            velo_y = 256;
        }
        if (velo_z < -256) {
            velo_z = -256;
        } else if (velo_z > 256) {
            velo_z = 256;
        }
        nxpos.x.val = velo_x + tngpos->x.val;
        nxpos.y.val = velo_y + tngpos->y.val;
        nxpos.z.val = velo_z + tngpos->z.val;
        if ((thing->movement_flags & TMvF_Flying) != 0)
        {
            if (thing_in_wall_at(thing, &nxpos) && !creature_can_pass_throgh_wall_at(thing, &nxpos))
            {
                long blocked_flags;
                blocked_flags = get_thing_blocked_flags_at(thing, &nxpos);
                if (cctrl->collided_door_subtile == 0) {
                    check_for_door_collision_at(thing, &nxpos, blocked_flags);
                }
                slide_thing_against_wall_at(thing, &nxpos, blocked_flags);
            }
        }
        else
        {
            if (thing_in_wall_at(thing, &nxpos) && !creature_can_pass_throgh_wall_at(thing, &nxpos))
            {
                if (creature_cannot_move_directly_to(thing, &nxpos))
                {
                    long blocked_flags;
                    blocked_flags = get_creature_blocked_flags_at(thing, &nxpos);
                    if (cctrl->collided_door_subtile == 0) {
                        check_for_door_collision_at(thing, &nxpos, blocked_flags);
                    }
                    slide_thing_against_wall_at(thing, &nxpos, blocked_flags);
                }
                else
                {
                    nxpos.z.val = get_thing_height_at(thing, &nxpos);
                }
            }
        }
        if ((cctrl->flgfield_1 & CCFlg_Unknown10) != 0)
        {
            struct Thing * collidtng;
            collidtng = get_thing_collided_with_at_satisfying_filter(thing, &nxpos, collide_filter_thing_is_of_type, 5, -1);
            if (!thing_is_invalid(collidtng))
            {
                nxpos.x.val = tngpos->x.val;
                nxpos.y.val = tngpos->y.val;
                nxpos.z.val = tngpos->z.val;
            }
        }
        if ((tngpos->x.stl.num != nxpos.x.stl.num) || (tngpos->y.stl.num != nxpos.y.stl.num))
        {
            if (thing->model == get_players_special_digger_model(game.hero_player_num)) {
                update_tunneller_trail(thing);
            }
            if ((subtile_slab_fast(tngpos->x.stl.num) != subtile_slab_fast(nxpos.x.stl.num))
             || (subtile_slab_fast(tngpos->y.stl.num) != subtile_slab_fast(nxpos.y.stl.num)))
            {
                check_map_explored(thing, nxpos.x.stl.num, nxpos.y.stl.num);
                CreatureStateFunc2 callback;
                callback = states[thing->active_state].move_from_slab;
                if (callback != NULL)
                {
                    callback(thing);
                }
            }
        }
        move_thing_in_map(thing, &nxpos);
    }
    {
        long angle;
        long dist;
        angle = LbArcTanAngle(cctrl->moveaccel.x.val, cctrl->moveaccel.y.val);
        if (get_angle_difference(angle, thing->move_angle_xy) <= LbFPMath_PI/2) {
            dist = get_2d_distance(&pvpos, tngpos);
        } else {
            dist = -get_2d_distance(&pvpos, tngpos);
        }
        cctrl->field_9 = dist;
    }
    return 1;
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
    cctrl = creature_control_get_from_thing(thing);
    lairtng = thing_get(cctrl->lairtng_idx);
    if (thing_is_invalid(lairtng))
    {
        // If creature has no lair - treat dungeon heart as lair
        lairtng = get_player_soul_container(thing->owner);
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
    // Compute how many pots we want to drop
    int num_pots_to_drop;
    num_pots_to_drop = (thing->creature.gold_carried + game.pot_of_gold_holds - 1) / game.pot_of_gold_holds;
    if (num_pots_to_drop > 8)
        num_pots_to_drop = 8;
    GoldAmount gold_dropped;
    gold_dropped = 0;
    // Now do the dropping
    int npot;
    for (npot = 0; npot < num_pots_to_drop; npot++)
    {
        // Create a new pot object
        struct Thing *gldtng;
        gldtng = create_object(&thing->mappos, 6, game.neutral_player_num, -1);
        if (thing_is_invalid(gldtng))
            break;
        // Update its position and acceleration
        long angle,radius;
        long x,y;
        angle = ACTION_RANDOM(2*LbFPMath_PI);
        radius = ACTION_RANDOM(128);
        x = (radius * LbSinL(angle)) / 256;
        y = (radius * LbCosL(angle)) / 256;
        gldtng->veloc_push_add.x.val += x/256;
        gldtng->veloc_push_add.y.val -= y/256;
        gldtng->veloc_push_add.z.val += ACTION_RANDOM(64) + 96;
        gldtng->state_flags |= TF1_PushAdd;
        // Set the amount of gold and mark that we've dropped that gold
        GoldAmount delta;
        delta = (thing->creature.gold_carried - gold_dropped) / (num_pots_to_drop - npot);
        gldtng->valuable.gold_stored = delta;
        // Update size of the gold object
        add_gold_to_pile(gldtng, 0);
        gold_dropped += delta;
    }
}

void thing_death_normal(struct Thing *thing)
{
    struct Thing *deadtng;
    struct Coord3d memaccl;
    long memp1;
    memp1 = thing->move_angle_xy;
    memaccl.x.val = thing->veloc_base.x.val;
    memaccl.y.val = thing->veloc_base.y.val;
    memaccl.z.val = thing->veloc_base.z.val;
    deadtng = destroy_creature_and_create_corpse(thing, 1);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Cannot create dead thing");
        return;
    }
    deadtng->move_angle_xy = memp1;
    deadtng->veloc_base.x.val = memaccl.x.val;
    deadtng->veloc_base.y.val = memaccl.y.val;
    deadtng->veloc_base.z.val = memaccl.z.val;
}

/**
 * Creates an effect of death with bloody flesh explosion, killing the creature.
 * @param thing
 */
void thing_death_flesh_explosion(struct Thing *thing)
{
    struct Thing *deadtng;
    struct Coord3d pos;
    struct Coord3d memaccl;
    long memp1;
    long i;
    //_DK_thing_death_flesh_explosion(thing);return;
    memp1 = thing->move_angle_xy;
    memaccl.x.val = thing->veloc_base.x.val;
    memaccl.y.val = thing->veloc_base.y.val;
    memaccl.z.val = thing->veloc_base.z.val;
    for (i = 0; i <= thing->clipbox_size_yz; i+=64)
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
    deadtng->move_angle_xy = memp1;
    deadtng->veloc_base.x.val = memaccl.x.val;
    deadtng->veloc_base.y.val = memaccl.y.val;
    deadtng->veloc_base.z.val = memaccl.z.val;
    thing_play_sample(deadtng, 47, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
}

void thing_death_gas_and_flesh_explosion(struct Thing *thing)
{
    struct Thing *deadtng;
    struct Coord3d pos;
    struct Coord3d memaccl;
    long memp1;
    long i;
    //_DK_thing_death_gas_and_flesh_explosion(thing);return;
    memp1 = thing->move_angle_xy;
    memaccl.x.val = thing->veloc_base.x.val;
    memaccl.y.val = thing->veloc_base.y.val;
    memaccl.z.val = thing->veloc_base.z.val;
    for (i = 0; i <= thing->clipbox_size_yz; i+=64)
    {
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = thing->mappos.z.val+i;
        create_effect(&pos, TngEff_Unknown09, thing->owner);
    }
    i = (thing->clipbox_size_yz >> 1);
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
    deadtng->move_angle_xy = memp1;
    deadtng->veloc_base.x.val = memaccl.x.val;
    deadtng->veloc_base.y.val = memaccl.y.val;
    deadtng->veloc_base.z.val = memaccl.z.val;
    thing_play_sample(deadtng, 47, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
}

void thing_death_smoke_explosion(struct Thing *thing)
{
    struct Thing *deadtng;
    struct Coord3d pos;
    struct Coord3d memaccl;
    long memp1;
    long i;
    //_DK_thing_death_smoke_explosion(thing);return;
    memp1 = thing->move_angle_xy;
    memaccl.x.val = thing->veloc_base.x.val;
    memaccl.y.val = thing->veloc_base.y.val;
    memaccl.z.val = thing->veloc_base.z.val;
    i = (thing->clipbox_size_yz >> 1);
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
    deadtng->move_angle_xy = memp1;
    deadtng->veloc_base.x.val = memaccl.x.val;
    deadtng->veloc_base.y.val = memaccl.y.val;
    deadtng->veloc_base.z.val = memaccl.z.val;
    thing_play_sample(deadtng, 47, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
}

/**
 * Creates an effect of frozen body explosion and kills the creature.
 * The ice explosion effect uses same corpse as flesh explosion.
 * @param thing
 */
void thing_death_ice_explosion(struct Thing *thing)
{
    struct Thing *deadtng;
    struct Coord3d pos;
    struct Coord3d memaccl;
    long memp1;
    long i;
    //_DK_thing_death_ice_explosion(thing);return;
    memp1 = thing->move_angle_xy;
    memaccl.x.val = thing->veloc_base.x.val;
    memaccl.y.val = thing->veloc_base.y.val;
    memaccl.z.val = thing->veloc_base.z.val;
    for (i = 0; i <= thing->clipbox_size_yz; i+=64)
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
    deadtng->move_angle_xy = memp1;
    deadtng->veloc_base.x.val = memaccl.x.val;
    deadtng->veloc_base.y.val = memaccl.y.val;
    deadtng->veloc_base.z.val = memaccl.z.val;
    thing_play_sample(deadtng, 47, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
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
 * Removes given parent index in things from given StructureList.
 * Works only for things for whom parent is a thing (which are shots).
 * @return Gives amount of items updated.
 * TODO figure out what this index is, then rename and move this function.
 */
unsigned long remove_parent_thing_from_things_in_list(struct StructureList *list,long remove_idx)
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
        if (thing->parent_idx == remove_idx)
        {
            thing->parent_idx = thing->index;
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

void cause_creature_death(struct Thing *thing, CrDeathFlags flags)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    long crmodel;
    //_DK_cause_creature_death(thing, (flags & CrDed_NoEffects) != 0); return;
    cctrl = creature_control_get_from_thing(thing);
    anger_set_creature_anger_all_types(thing, 0);
    throw_out_gold(thing);
    remove_parent_thing_from_things_in_list(&game.thing_lists[TngList_Shots],thing->index);

    crmodel = thing->model;
    crstat = creature_stats_get_from_thing(thing);
    if (!thing_exists(thing)) {
        flags |= CrDed_NoEffects;
    }
    if (((flags & CrDed_NoEffects) == 0) && (crstat->rebirth != 0)
     && (cctrl->lairtng_idx > 0) && (crstat->rebirth-1 <= cctrl->explevel))
    {
        creature_rebirth_at_lair(thing);
        return;
    }
    // Beyond this point, the creature thing is bound to be deleted
    if (((flags & CrDed_NotReallyDying) == 0) || ((gameadd.classic_bugs_flags & ClscBug_ResurrectRemoved) != 0))
    {
        // If the creature is leaving dungeon, or being transformed, then CrDed_NotReallyDying should be set
        update_dead_creatures_list_for_owner(thing);
    }
    if ((flags & CrDed_NoEffects) != 0)
    {
        if ((game.flags_cd & MFlg_DeadBackToPool) != 0)
            add_creature_to_pool(crmodel, 1, 1);
        delete_thing_structure(thing, 0);
    } else
    if (!creature_model_bleeds(thing->model))
    {
        // Non-bleeding creatures have no flesh explosion effects
        if ((game.flags_cd & MFlg_DeadBackToPool) != 0)
            add_creature_to_pool(crmodel, 1, 1);
        creature_death_as_nature_intended(thing);
    } else
    if (creature_affected_by_spell(thing, SplK_Freeze))
    {
        if ((game.flags_cd & MFlg_DeadBackToPool) != 0)
            add_creature_to_pool(crmodel, 1, 1);
        thing_death_ice_explosion(thing);
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

void prepare_to_controlled_creature_death(struct Thing *thing)
{
  struct PlayerInfo *player;
  player = get_player(thing->owner);
  leave_creature_as_controller(player, thing);
  player->influenced_thing_idx = 0;
  if (player->id_number == thing->owner)
    setup_eye_lens(0);
  set_camera_zoom(player->acamera, player->dungeon_camera_zoom);
  if (player->id_number == thing->owner)
  {
    turn_off_all_window_menus();
    turn_off_query_menus();
    turn_on_main_panel_menu();
    set_flag_byte(&game.status_flags, Status_ShowStatusMenu, (game.status_flags & Status_ShowGui) != 0);
  }
  light_turn_light_on(player->field_460);
}

void delete_effects_attached_to_creature(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    struct Thing *efftng;
    long i,k;
    cctrl = creature_control_get_from_thing(creatng);
    if (creature_control_invalid(cctrl)) {
        return;
    }
    if (creature_affected_by_spell(creatng, SplK_Armour))
    {
        cctrl->spell_flags &= ~CSAfF_Armour;
        for (i=0; i < 3; i++)
        {
            k = cctrl->spell_tngidx_armour[i];
            if (k != 0)
            {
                efftng = thing_get(k);
                delete_thing_structure(efftng, 0);
                cctrl->spell_tngidx_armour[i] = 0;
            }
        }
    }
    if (creature_affected_by_spell(creatng, SplK_Disease))
    {
        cctrl->spell_flags &= ~CSAfF_Disease;
        for (i=0; i < 3; i++)
        {
            k = cctrl->spell_tngidx_disease[i];
            if (k != 0)
            {
                efftng = thing_get(k);
                delete_thing_structure(efftng, 0);
                cctrl->spell_tngidx_disease[i] = 0;
            }
        }
    }
}

// Old code compatibility function - to be removed when no references remain unrewritten
TbBool kill_creature_compat(struct Thing *creatng, struct Thing *killertng, PlayerNumber killer_plyr_idx,
      TbBool no_effects, TbBool died_in_battle, TbBool disallow_unconscious)
{
    return kill_creature(creatng, killertng, killer_plyr_idx,
        (no_effects?CrDed_NoEffects:0) | (died_in_battle?CrDed_DiedInBattle:0) | (disallow_unconscious?CrDed_NoUnconscious:0) );
}

TbBool kill_creature(struct Thing *creatng, struct Thing *killertng,
    PlayerNumber killer_plyr_idx, CrDeathFlags flags)
{
    struct Dungeon *dungeon;
    SYNCDBG(18,"Starting");
    TRACE_THING(creatng);
    //return _DK_kill_creature(creatng, killertng, killer_plyr_idx, (flags&CrDed_NoEffects)!=0, (flags&CrDed_DiedInBattle)!=0, (flags&CrDed_NoUnconscious)!=0);
    dungeon = INVALID_DUNGEON;
    cleanup_creature_state_and_interactions(creatng);
    if (!thing_is_invalid(killertng))
    {
        if (killertng->owner == game.neutral_player_num) {
            flags &= ~CrDed_DiedInBattle;
        }
    }
    if (killer_plyr_idx == game.neutral_player_num) {
        flags &= ~CrDed_DiedInBattle;
    }
    if (!thing_exists(creatng)) {
        ERRORLOG("Tried to kill non-existing thing!");
        return false;
    }
    // Dying creatures must be visible
    if (creature_affected_by_spell(creatng, SplK_Invisibility)) {
        terminate_thing_spell_effect(creatng, SplK_Invisibility);
    }
    if (!is_neutral_thing(creatng)) {
        dungeon = get_players_num_dungeon(creatng->owner);
    }
    if (!dungeon_invalid(dungeon))
    {
        if ((flags & CrDed_DiedInBattle) != 0) {
            dungeon->battles_lost++;
        }
    }
    update_kills_counters(creatng, killertng, killer_plyr_idx, flags);
    if (thing_is_invalid(killertng) || (killertng->owner == game.neutral_player_num) ||
        (killer_plyr_idx == game.neutral_player_num) || dungeon_invalid(dungeon))
    {
        if ((flags & CrDed_NoEffects) && ((creatng->alloc_flags & TAlF_IsControlled) != 0)) {
            prepare_to_controlled_creature_death(creatng);
        }
        cause_creature_death(creatng, flags);
        return true;
    }
    // Now we are sure that killertng and dungeon pointers are correct
    if (creatng->owner == killertng->owner)
    {
        if ((get_creature_model_flags(creatng) & CMF_IsDiptera) && (get_creature_model_flags(killertng) & CMF_IsArachnid)) {
            dungeon->lvstats.flies_killed_by_spiders++;
        }
    }
    struct CreatureControl *cctrlgrp;
    cctrlgrp = creature_control_get_from_thing(killertng);
    if (!creature_control_invalid(cctrlgrp)) {
        cctrlgrp->kills_num++;
    }
    if (is_my_player_number(creatng->owner)) {
        output_message(SMsg_BattleDeath, MESSAGE_DELAY_BATTLE, true);
    } else
    if (is_my_player_number(killertng->owner)) {
        output_message(SMsg_BattleWon, MESSAGE_DELAY_BATTLE, true);
    }
    if (is_hero_thing(killertng))
    {
        if (player_creature_tends_to(killertng->owner,CrTend_Imprison)) {
            ERRORLOG("Hero have tend to imprison");
        }
    }
    {
        struct CreatureStats *crstat;
        crstat = creature_stats_get_from_thing(killertng);
        anger_apply_anger_to_creature(killertng, crstat->annoy_win_battle, AngR_Other, 1);
    }
    if (!creature_control_invalid(cctrlgrp) && ((flags & CrDed_DiedInBattle) != 0)) {
        cctrlgrp->byte_9A++;
    }
    if (!dungeon_invalid(dungeon)) {
        dungeon->hates_player[killertng->owner] += game.fight_hate_kill_value;
    }
    SYNCDBG(18,"Almost finished");
    if (((flags & CrDed_NoUnconscious) != 0) || (!player_has_room(killertng->owner,RoK_PRISON))
      || (!player_creature_tends_to(killertng->owner,CrTend_Imprison)))
    {
        if ((flags & CrDed_NoEffects) == 0) {
            cause_creature_death(creatng, flags);
            return true;
        }
    }
    if ((flags & CrDed_NoEffects) != 0)
    {
        if ((creatng->alloc_flags & TAlF_IsControlled) != 0) {
            prepare_to_controlled_creature_death(creatng);
        }
        cause_creature_death(creatng, flags);
        return true;
    }
    make_creature_unconscious(creatng);
    creatng->health = 1;
    return false;
}

void process_creature_standing_on_corpses_at(struct Thing *creatng, struct Coord3d *pos)
{
    //_DK_process_creature_standing_on_corpses_at(thing, pos);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    struct Map *mapblk;
    mapblk = get_map_block_at(pos->x.stl.num,pos->y.stl.num);
    long i;
    unsigned long k;
    k = 0;
    i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if (thing->class_id == TCls_DeadCreature)
        {
            if (!is_hero_thing(creatng))
            {
                int annoy_val;
                struct CreatureStats *crstat;
                crstat = creature_stats_get_from_thing(creatng);
                if (thing->owner == creatng->owner) {
                    annoy_val = crstat->annoy_on_dead_friend;
                } else {
                    annoy_val = crstat->annoy_on_dead_enemy;
                }
                anger_apply_anger_to_creature(creatng, annoy_val, AngR_Other, 1);
            }
            cctrl->bloody_footsteps_turns = 20;
            cctrl->field_B9 = thing->index;
            // Stop after one body was found
            break;
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

/**
 * Calculates damage made by a creature by hand (using strength).
 * @param thing The creature which will be inflicting the damage.
 */
long calculate_melee_damage(const struct Thing *creatng)
{
    const struct CreatureControl *cctrl;
    const struct CreatureStats *crstat;
    cctrl = creature_control_get_from_thing(creatng);
    crstat = creature_stats_get_from_thing(creatng);
    long strength;
    strength = compute_creature_max_strength(crstat->strength,cctrl->explevel);
    return compute_creature_attack_melee_damage(strength, crstat->luck, cctrl->explevel);
}

/**
 * Calculates damage made by a creature using specific shot model.
 * @param thing The creature which will be shooting.
 * @param shot_model Shot kind which will be created.
 */
long calculate_shot_damage(const struct Thing *creatng, ThingModel shot_model)
{
    const struct CreatureControl *cctrl;
    const struct CreatureStats *crstat;
    const struct ShotConfigStats *shotst;
    shotst = get_shot_model_stats(shot_model);
    cctrl = creature_control_get_from_thing(creatng);
    crstat = creature_stats_get_from_thing(creatng);
    return compute_creature_attack_spell_damage(shotst->old->damage, crstat->luck, cctrl->explevel);
}

/**
 * Projects damage made by a creature using specific shot model.
 * Gives a best estimate of the damage, but shouldn't be used to actually inflict it.
 * @param thing The creature which will be shooting.
 * @param shot_model Shot kind which will be created.
 */
long project_creature_shot_damage(const struct Thing *thing, ThingModel shot_model)
{
    const struct CreatureControl *cctrl;
    const struct CreatureStats *crstat;
    const struct ShotConfigStats *shotst;
    shotst = get_shot_model_stats(shot_model);
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);
    long damage;
    if ( shotst->old->is_melee )
    {
        // Project melee damage
        long strength;
        strength = compute_creature_max_strength(crstat->strength,cctrl->explevel);
        damage = project_creature_attack_melee_damage(strength, crstat->luck, cctrl->explevel);
    } else
    {
        // Project shot damage
        damage = project_creature_attack_spell_damage(shotst->old->damage, crstat->luck, cctrl->explevel);
    }
    return damage;
}

void creature_fire_shot(struct Thing *firing, struct Thing *target, ThingModel shot_model, char shot_lvl, unsigned char hit_type)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct ShotConfigStats *shotst;
    struct Coord3d pos1;
    struct Coord3d pos2;
    struct ComponentVector cvect;
    struct Thing *shotng;
    struct Thing *tmptng;
    short angle_xy,angle_yz;
    long damage;
    long target_idx,i;
    TbBool flag1;
    //_DK_creature_fire_shot(firing,target,shot_model,shot_lvl,a3); return;
    cctrl = creature_control_get_from_thing(firing);
    crstat = creature_stats_get_from_thing(firing);
    shotst = get_shot_model_stats(shot_model);
    flag1 = false;
    // Prepare source position
    pos1.x.val = firing->mappos.x.val;
    pos1.y.val = firing->mappos.y.val;
    pos1.z.val = firing->mappos.z.val;
    pos1.x.val += distance_with_angle_to_coord_x(cctrl->field_2C1, firing->move_angle_xy+LbFPMath_PI/2);
    pos1.y.val += distance_with_angle_to_coord_y(cctrl->field_2C1, firing->move_angle_xy+LbFPMath_PI/2);
    pos1.x.val += distance_with_angle_to_coord_x(cctrl->field_2C3, firing->move_angle_xy);
    pos1.y.val += distance_with_angle_to_coord_y(cctrl->field_2C3, firing->move_angle_xy);
    pos1.z.val += (cctrl->field_2C5);
    // Compute launch angles
    if (thing_is_invalid(target))
    {
        angle_xy = firing->move_angle_xy;
        angle_yz = firing->move_angle_z;
    } else
    {
        pos2.x.val = target->mappos.x.val;
        pos2.y.val = target->mappos.y.val;
        pos2.z.val = target->mappos.z.val;
        pos2.z.val += (target->clipbox_size_yz >> 1);
        if ((shotst->old->is_melee) && (target->class_id != TCls_Door))
        {
          flag1 = true;
          pos1.z.val = pos2.z.val;
        }
        angle_xy = get_angle_xy_to(&pos1, &pos2);
        angle_yz = get_angle_yz_to(&pos1, &pos2);
    }
    // Compute shot damage
    if (shotst->old->is_melee)
    {
        damage = calculate_melee_damage(firing);
    } else
    {
        damage = calculate_shot_damage(firing,shot_model);
    }
    shotng = NULL;
    target_idx = 0;
    // Set target index for navigating shots
    if (shot_model_is_navigable(shot_model))
    {
      if (!thing_is_invalid(target))
        target_idx = target->index;
    }
    switch ( shot_model )
    {
    case ShM_Lightning:
    case ShM_Drain:
        if ((thing_is_invalid(target)) || (get_2d_distance(&firing->mappos, &pos2) > 5120))
        {
            project_point_to_wall_on_angle(&pos1, &pos2, firing->move_angle_xy, firing->move_angle_z, 256, 20);
        }
        shotng = create_thing(&pos2, TCls_Shot, shot_model, firing->owner, -1);
        if (thing_is_invalid(shotng))
          return;
        if (shot_model == 12)
          draw_lightning(&pos1, &pos2, 96, 93);
        else
          draw_lightning(&pos1, &pos2, 96, 60);
        shotng->health = shotst->health;
        shotng->shot.damage = shotst->old->damage;
        shotng->parent_idx = firing->index;
        break;
    case ShM_FlameBreathe:
        if ((thing_is_invalid(target)) || (get_2d_distance(&firing->mappos, &pos2) > 768))
          project_point_to_wall_on_angle(&pos1, &pos2, firing->move_angle_xy, firing->move_angle_z, 256, 4);
        shotng = create_thing(&pos2, TCls_Shot, shot_model, firing->owner, -1);
        if (thing_is_invalid(shotng))
          return;
        draw_flame_breath(&pos1, &pos2, 96, 2);
        shotng->health = shotst->health;
        shotng->shot.damage = shotst->old->damage;
        shotng->parent_idx = firing->index;
        break;
    case ShM_Hail_storm:
        for (i=0; i < 32; i++)
        {
            tmptng = create_thing(&pos1, TCls_Shot, shot_model, firing->owner, -1);
            if (thing_is_invalid(tmptng))
              break;
            shotng = tmptng;
            shotng->shot.hit_type = hit_type;
            shotng->move_angle_xy = (angle_xy + ACTION_RANDOM(101) - 50) & LbFPMath_AngleMask;
            shotng->move_angle_z = (angle_yz + ACTION_RANDOM(101) - 50) & LbFPMath_AngleMask;
            angles_to_vector(shotng->move_angle_xy, shotng->move_angle_z, shotst->old->speed, &cvect);
            shotng->veloc_push_add.x.val += cvect.x;
            shotng->veloc_push_add.y.val += cvect.y;
            shotng->veloc_push_add.z.val += cvect.z;
            shotng->state_flags |= TF1_PushAdd;
            shotng->shot.damage = damage;
            shotng->health = shotst->health;
            shotng->parent_idx = firing->index;
        }
        break;
    default:
        shotng = create_thing(&pos1, TCls_Shot, shot_model, firing->owner, -1);
        if (thing_is_invalid(shotng))
            return;
        shotng->move_angle_xy = angle_xy;
        shotng->move_angle_z = angle_yz;
        angles_to_vector(shotng->move_angle_xy, shotng->move_angle_z, shotst->old->speed, &cvect);
        shotng->veloc_push_add.x.val += cvect.x;
        shotng->veloc_push_add.y.val += cvect.y;
        shotng->veloc_push_add.z.val += cvect.z;
        shotng->state_flags |= TF1_PushAdd;
        shotng->shot.damage = damage;
        shotng->health = shotst->health;
        shotng->parent_idx = firing->index;
        shotng->shot.target_idx = target_idx;
        shotng->shot.dexterity = compute_creature_max_dexterity(crstat->dexterity,cctrl->explevel);
        break;
    }
    if (!thing_is_invalid(shotng))
    {
#if (BFDEBUG_LEVEL > 0)
      damage = shotng->word_14;
      // Special debug code that shows amount of damage the shot will make
      if ((start_params.debug_flags & DFlg_ShotsDamage) != 0)
          create_price_effect(&pos1, my_player_number, damage);
      if ((damage < 0) || (damage > 2000))
      {
        WARNLOG("Shot of type %d carries %d damage",(int)shot_model,(int)damage);
      }
#endif
      shotng->shot.hit_type = hit_type;
      if (shotst->old->firing_sound > 0)
      {
        thing_play_sample(firing, shotst->old->firing_sound + UNSYNC_RANDOM(shotst->old->firing_sound_variants),
            100, 0, 3, 0, 3, FULL_LOUDNESS);
      }
      if (shotst->old->shot_sound > 0)
      {
        thing_play_sample(shotng, shotst->old->shot_sound, NORMAL_PITCH, 0, 3, 0, shotst->old->field_20, FULL_LOUDNESS);
      }
      set_flag_byte(&shotng->movement_flags,TMvF_Unknown10,flag1);
    }
}

void set_creature_level(struct Thing *thing, long nlvl)
{
    //_DK_set_creature_level(thing, nlvl); return;
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    long old_max_health,max_health;
    crstat = creature_stats_get_from_thing(thing);
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Creature has no control");
        return;
    }
    if (nlvl > CREATURE_MAX_LEVEL-1) {
        ERRORLOG("Level %d too high, bounding",(int)nlvl);
        nlvl = CREATURE_MAX_LEVEL-1;
    }
    if (nlvl < 0) {
        ERRORLOG("Level %d too low, bounding",(int)nlvl);
        nlvl = 0;
    }
    old_max_health = compute_creature_max_health(crstat->health,cctrl->explevel);
    if (old_max_health < 1)
        old_max_health = 1;
    cctrl->explevel = nlvl;
    max_health = compute_creature_max_health(crstat->health,cctrl->explevel);
    cctrl->max_health = max_health;
    set_creature_size_stuff(thing);
    if (old_max_health > 0)
        thing->health = saturate_set_signed( (thing->health*max_health)/old_max_health, 16);
    else
        thing->health = -1;
    creature_increase_available_instances(thing);
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

/** Retrieves speed of a creature.
 *
 * @param thing
 * @return
 */
long get_creature_speed(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long speed;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return 0;
    speed = cctrl->max_speed;
    if (speed < 0)
        speed = 0;
    if (speed > MAX_VELOCITY)
        speed = MAX_VELOCITY;
    return speed;
}

long get_human_controlled_creature_target(struct Thing *thing, long a2)
{
  return _DK_get_human_controlled_creature_target(thing, a2);
}

long creature_instance_has_reset(const struct Thing *thing, long inst_idx)
{
    const struct CreatureControl *cctrl;
    const struct InstanceInfo *inst_inf;
    long ritime;
    //return _DK_creature_instance_has_reset(thing, a2);
    cctrl = creature_control_get_from_thing(thing);
    inst_inf = creature_instance_info_get(inst_idx);
    long delta;
    delta = (long)game.play_gameturn - (long)cctrl->instance_use_turn[inst_idx];
    if ((thing->alloc_flags & TAlF_IsControlled) != 0)
    {
        ritime = inst_inf->fp_reset_time;
    } else
    {
        ritime = inst_inf->reset_time;
    }
    return (delta >= ritime);
}

/**
 * Gives times a creature spends on an instance, including spell modifiers.
 * @param thing The creature thing.
 * @param inst_idx Instance for which times will be computed.
 * @param ritime Returns instance duration turns.
 * @param raitime Returns instance turn on which action function is executed.
 */
void get_creature_instance_times(const struct Thing *thing, long inst_idx, long *ritime, long *raitime)
{
    struct InstanceInfo *inst_inf;
    long itime,aitime;
    inst_inf = creature_instance_info_get(inst_idx);
    if ((thing->alloc_flags & TAlF_IsControlled) != 0)
    {
        itime = inst_inf->fp_time;
        aitime = inst_inf->fp_action_time;
    } else
    {
        itime = inst_inf->time;
        aitime = inst_inf->action_time;
    }
    if (creature_affected_by_spell(thing, SplK_Slow))
    {
        aitime *= 2;
        itime *= 2;
    }
    if (creature_affected_by_spell(thing, SplK_Speed))
    {
        aitime /= 2;
        itime /= 2;
    } else
    if (creature_affected_by_slap(thing))
    {
        aitime = 3 * aitime / 4;
        itime = 3 * itime / 4;
    } else
    if (!is_neutral_thing(thing))
    {
        if (player_uses_power_obey(thing->owner))
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

void set_creature_instance(struct Thing *thing, CrInstance inst_idx, long a2, long targtng_idx, const struct Coord3d *pos)
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
        ERRORLOG("Cannot set negative instance %d to %s index %d",(int)inst_idx,thing_model_name(thing),(int)thing->index);
        return;
    }
    if (inst_inf->force_visibility > 0)
    {
        i = cctrl->force_visible;
        if (i <= inst_inf->force_visibility)
          i = inst_inf->force_visibility;
        cctrl->force_visible = i;
    }
    get_creature_instance_times(thing, inst_idx, &itime, &aitime);
    if ((cctrl->instance_id != CrInst_NULL) && (cctrl->instance_id == inst_idx))
    {
        if ((inst_inf->flags & InstPF_RepeatTrigger) != 0)
        {
            cctrl->inst_repeat = 1;
            return;
        }
    }
    cctrl->instance_id = inst_idx;
    cctrl->targtng_idx = targtng_idx;
    cctrl->inst_turn = 0;
    cctrl->inst_total_turns = itime;
    cctrl->inst_action_turns = aitime;
    i = get_creature_model_graphics(thing->model,inst_inf->graphics_idx);
    cctrl->instance_anim_step_turns = get_lifespan_of_animation(i, 1) / itime;
    if (pos != NULL)
    {
        cctrl->targtstl_x = coord_subtile(pos->x.val);
        cctrl->targtstl_y = coord_subtile(pos->y.val);
    } else
    {
        cctrl->targtstl_x = 0;
        cctrl->targtstl_y = 0;
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
      engine(player,&player->cameras[1]);
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
  engine(player,&player->cameras[1]);
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

void set_first_creature(struct Thing *thing)
{
    _DK_set_first_creature(thing);
}

void remove_first_creature(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct CreatureControl *secctrl;
    struct Thing *sectng;
    //_DK_remove_first_creature(creatng);
    cctrl = creature_control_get_from_thing(creatng);
    if ((creatng->alloc_flags & TAlF_InDungeonList) == 0)
    {
        ERRORLOG("The %s index %d is not in Peter list",thing_model_name(creatng),(int)creatng->index);
        return;
    }
    if (is_neutral_thing(creatng))
    {
      sectng = thing_get(cctrl->players_prev_creature_idx);
      if (!thing_is_invalid(sectng)) {
          secctrl = creature_control_get_from_thing(sectng);
          secctrl->players_next_creature_idx = cctrl->players_next_creature_idx;
      } else {
          game.nodungeon_creatr_list_start = cctrl->players_next_creature_idx;
      }
      sectng = thing_get(cctrl->players_next_creature_idx);
      if (!thing_is_invalid(sectng)) {
          secctrl = creature_control_get_from_thing(sectng);
          secctrl->players_prev_creature_idx = cctrl->players_prev_creature_idx;
      }
    } else
    if (creatre_is_for_dungeon_diggers_list(creatng))
    {
        dungeon = get_dungeon(creatng->owner);
        sectng = thing_get(cctrl->players_prev_creature_idx);
        if (!thing_is_invalid(sectng)) {
            secctrl = creature_control_get_from_thing(sectng);
            secctrl->players_next_creature_idx = cctrl->players_next_creature_idx;
        } else {
            dungeon->digger_list_start = cctrl->players_next_creature_idx;
        }
        sectng = thing_get(cctrl->players_next_creature_idx);
        if (!thing_is_invalid(sectng)) {
            secctrl = creature_control_get_from_thing(sectng);
            secctrl->players_prev_creature_idx = cctrl->players_prev_creature_idx;
        }
        if ((cctrl->flgfield_2 & TF2_Spectator) == 0)
        {
            dungeon->num_active_diggers--;
            dungeon->owned_creatures_of_model[creatng->model]--;
        }
    } else
    {
        dungeon = get_dungeon(creatng->owner);
        sectng = thing_get(cctrl->players_prev_creature_idx);
        if (!thing_is_invalid(sectng)) {
            secctrl = creature_control_get_from_thing(sectng);
            secctrl->players_next_creature_idx = cctrl->players_next_creature_idx;
        } else {
            dungeon->creatr_list_start = cctrl->players_next_creature_idx;
        }
        sectng = thing_get(cctrl->players_next_creature_idx);
        if (!thing_is_invalid(sectng)) {
            secctrl = creature_control_get_from_thing(sectng);
            secctrl->players_prev_creature_idx = cctrl->players_prev_creature_idx;
        }
        if ((cctrl->flgfield_2 & TF2_Spectator) == 0)
        {
            dungeon->num_active_creatrs--;
            dungeon->owned_creatures_of_model[creatng->model]--;
        }
    }
    cctrl->players_prev_creature_idx = 0;
    cctrl->players_next_creature_idx = 0;
    creatng->alloc_flags &= ~TAlF_InDungeonList;
}

TbBool thing_is_creature(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return false;
  if (thing->class_id != TCls_Creature)
    return false;
  return true;
}

TbBool thing_is_dead_creature(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return false;
  if (thing->class_id != TCls_DeadCreature)
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
  return ((get_creature_model_flags(thing) & CMF_IsSpecDigger) != 0);
}

void anger_set_creature_anger_all_types(struct Thing *thing, long a2)
{
    _DK_anger_set_creature_anger_all_types(thing, a2);
}

struct Room *get_creature_lair_room(const struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->lairtng_idx <= 0) {
        return INVALID_ROOM;
    }
    return room_get(cctrl->lair_room_id);
}

TbBool creature_has_lair_room(const struct Thing *creatng)
{
    struct Room *room;
    room = get_creature_lair_room(creatng);
    if (!room_is_invalid(room) && (room->kind == RoK_LAIR)) {
        return true;
    }
    return false;
}

TbBool remove_creature_lair(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Room *room;
    cctrl = creature_control_get_from_thing(thing);
    if (cctrl->lairtng_idx <= 0) {
        return false;
    }
    room = room_get(cctrl->lair_room_id);
    if (!room_is_invalid(room)) {
        creature_remove_lair_totem_from_room(thing, room);
        return true;
    } else {
        ERRORDBG(8,"The %s index %d has lair %d in non-existing room.",thing_model_name(thing),(int)thing->index,(int)cctrl->lairtng_idx);
        cctrl->lairtng_idx = 0;
    }
    return false;
}

void change_creature_owner(struct Thing *creatng, PlayerNumber nowner)
{
    struct Dungeon *dungeon;
    SYNCDBG(6,"Starting for %s, owner %d to %d",thing_model_name(creatng),(int)creatng->owner,(int)nowner);
    //_DK_change_creature_owner(thing, nowner); return;
    // Remove the creature from old owner
    if (creatng->light_id != 0) {
        light_delete_light(creatng->light_id);
        creatng->light_id = 0;
    }
    cleanup_creature_state_and_interactions(creatng);
    remove_creature_lair(creatng);
    if ((creatng->alloc_flags & TAlF_InDungeonList) != 0) {
        remove_first_creature(creatng);
    }
    if (!is_neutral_thing(creatng))
    {
        dungeon = get_dungeon(creatng->owner);
        dungeon->score -= get_creature_thing_score(creatng);
        if (anger_is_creature_angry(creatng))
            dungeon->creatures_annoyed--;
        remove_events_thing_is_attached_to(creatng);
    }
    // Add the creature to new owner
    creatng->owner = nowner;
    set_first_creature(creatng);
    set_start_state(creatng);
    if (!is_neutral_thing(creatng))
    {
        dungeon = get_dungeon(creatng->owner);
        dungeon->score += get_creature_thing_score(creatng);
        if ( anger_is_creature_angry(creatng) )
            dungeon->creatures_annoyed++;
    }
}

struct Thing *create_creature(struct Coord3d *pos, ThingModel model, PlayerNumber owner)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct CreatureData *crdata;
    struct Thing *crtng;
    long i;
    crstat = creature_stats_get(model);
    if (!i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots))
    {
        ERRORDBG(3,"Cannot create %s for player %d. There are too many things allocated.",creature_code_name(model),(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    if (!i_can_allocate_free_control_structure())
    {
        ERRORDBG(3,"Cannot create %s for player %d. There are too many creatures allocated.",creature_code_name(model),(int)owner);
        erstat_inc(ESE_NoFreeCreatrs);
        return INVALID_THING;
    }
    crtng = allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots);
    if (crtng->index == 0) {
        ERRORDBG(3,"Should be able to allocate %s for player %d, but failed.",creature_code_name(model),(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    cctrl = allocate_free_control_structure();
    crtng->ccontrol_idx = cctrl->index;
    crtng->class_id = TCls_Creature;
    crtng->model = model;
    crtng->parent_idx = crtng->index;
    crtng->mappos.x.val = pos->x.val;
    crtng->mappos.y.val = pos->y.val;
    crtng->mappos.z.val = pos->z.val;
    crtng->clipbox_size_xy = crstat->size_xy;
    crtng->clipbox_size_yz = crstat->size_yz;
    crtng->solid_size_xy = crstat->thing_size_xy;
    crtng->field_5C = crstat->thing_size_yz;
    crtng->field_20 = 32;
    crtng->field_22 = 0;
    crtng->field_23 = 32;
    crtng->field_24 = 8;
    crtng->movement_flags |= TMvF_Unknown08;
    crtng->owner = owner;
    crtng->move_angle_xy = 0;
    crtng->move_angle_z = 0;
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
    crtng->creation_turn = game.play_gameturn;
    cctrl->joining_age = 17+ACTION_RANDOM(13);
    cctrl->blood_type = ACTION_RANDOM(BLOOD_TYPES_COUNT);
    if (owner == game.hero_player_num)
    {
      cctrl->party.target_plyr_idx = -1;
      cctrl->byte_8C = 1;
    }
    cctrl->flee_pos.x.val = crtng->mappos.x.val;
    cctrl->flee_pos.y.val = crtng->mappos.y.val;
    cctrl->flee_pos.z.val = crtng->mappos.z.val;
    cctrl->flee_pos.z.val = get_thing_height_at(crtng, pos);
    cctrl->fighting_player_idx = -1;
    if (crstat->flying) {
        crtng->movement_flags |= TMvF_Flying;
    }
    set_creature_level(crtng, 0);
    crtng->health = cctrl->max_health;
    add_thing_to_its_class_list(crtng);
    place_thing_in_mapwho(crtng);
    if (owner <= PLAYERS_COUNT)
      set_first_creature(crtng);
    set_start_state(crtng);
    add_creature_score_to_owner(crtng);
    crdata = creature_data_get(crtng->model);
    cctrl->active_instance = crdata->flags;
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
          cctrl->spell_flags |= CSAfF_ExpLevelUp;
          return true;
      }
  }
  return false;
}

/**
 * Creates creature of random evil kind, and with random experience level.
 * @param x
 * @param y
 * @param owner
 * @param max_lv
 * @return
 */
TbBool create_random_evil_creature(MapCoord x, MapCoord y, PlayerNumber owner, CrtrExpLevel max_lv)
{
    struct Thing *thing;
    struct Coord3d pos;
    ThingModel crmodel;
    while (1) {
        crmodel = ACTION_RANDOM(crtr_conf.model_count) + 1;
        // Accept only evil creatures
        struct CreatureModelConfig *crconf;
        crconf = &crtr_conf.model[crmodel];
        if ((crconf->model_flags & CMF_IsSpectator) != 0) {
            continue;
        }
        if ((crconf->model_flags & CMF_IsEvil) != 0) {
            break;
        }
    }
    pos.x.val = x;
    pos.y.val = y;
    pos.z.val = 0;
    thing = create_creature(&pos, crmodel, owner);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Cannot create evil creature %s at (%ld,%ld)",creature_code_name(crmodel),x,y);
        return false;
    }
    pos.z.val = get_thing_height_at(thing, &pos);
    if (thing_in_wall_at(thing, &pos))
    {
        delete_thing_structure(thing, 0);
        ERRORLOG("Evil creature %s at (%ld,%ld) deleted because is in wall",creature_code_name(crmodel),x,y);
        return false;
    }
    thing->mappos.x.val = pos.x.val;
    thing->mappos.y.val = pos.y.val;
    thing->mappos.z.val = pos.z.val;
    remove_first_creature(thing);
    set_first_creature(thing);
    set_start_state(thing);
    CrtrExpLevel lv;
    lv = ACTION_RANDOM(max_lv);
    set_creature_level(thing, lv);
    return true;
}

/**
 * Creates creature of random hero kind, and with random experience level.
 * @param x
 * @param y
 * @param owner
 * @param max_lv
 * @return
 */
TbBool create_random_hero_creature(MapCoord x, MapCoord y, PlayerNumber owner, CrtrExpLevel max_lv)
{
  struct Thing *thing;
  struct Coord3d pos;
  ThingModel crmodel;
  while (1) {
      crmodel = ACTION_RANDOM(crtr_conf.model_count) + 1;
      // Accept only evil creatures
      struct CreatureModelConfig *crconf;
      crconf = &crtr_conf.model[crmodel];
      if ((crconf->model_flags & CMF_IsSpectator) != 0) {
          continue;
      }
      if ((crconf->model_flags & CMF_IsEvil) == 0) {
          break;
      }
  }
  pos.x.val = x;
  pos.y.val = y;
  pos.z.val = 0;
  thing = create_creature(&pos, crmodel, owner);
  if (thing_is_invalid(thing))
  {
      ERRORLOG("Cannot create player %d hero %s at (%ld,%ld)",(int)owner,creature_code_name(crmodel),x,y);
      return false;
  }
  pos.z.val = get_thing_height_at(thing, &pos);
  if (thing_in_wall_at(thing, &pos))
  {
      delete_thing_structure(thing, 0);
      ERRORLOG("Hero %s at (%ld,%ld) deleted because is in wall",creature_code_name(crmodel),x,y);
      return false;
  }
  thing->mappos.x.val = pos.x.val;
  thing->mappos.y.val = pos.y.val;
  thing->mappos.z.val = pos.z.val;
  remove_first_creature(thing);
  set_first_creature(thing);
//  set_start_state(thing); - simplified to the following two commands
  game.field_14E498 = game.play_gameturn;
  game.field_14E49C++;
  CrtrExpLevel lv;
  lv = ACTION_RANDOM(max_lv);
  set_creature_level(thing, lv);
  return true;
}

/**
 * Creates a special digger specific to given player and owned by that player.
 * @param x
 * @param y
 * @param owner
 * @return
 */
TbBool create_owned_special_digger(MapCoord x, MapCoord y, PlayerNumber owner)
{
    struct Thing *thing;
    struct Coord3d pos;
    ThingModel crmodel;
    crmodel = get_players_special_digger_model(owner);
    pos.x.val = x;
    pos.y.val = y;
    pos.z.val = 0;
    thing = create_creature(&pos, crmodel, owner);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Cannot create creature %s at (%ld,%ld)",creature_code_name(crmodel),x,y);
        return false;
    }
    pos.z.val = get_thing_height_at(thing, &pos);
    if (thing_in_wall_at(thing, &pos))
    {
        delete_thing_structure(thing, 0);
        ERRORLOG("Creature %s at (%ld,%ld) deleted because is in wall",creature_code_name(crmodel),x,y);
        return false;
    }
    thing->mappos.x.val = pos.x.val;
    thing->mappos.y.val = pos.y.val;
    thing->mappos.z.val = pos.z.val;
    remove_first_creature(thing);
    set_first_creature(thing);
    return true;
}

/**
 * Filter function for selecting creature which is fighting and is not affected by a specific spell.
 * A specific thing can be selected either by class, model and owner.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with specific thing which is dragged.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_in_fight_and_not_affected_by_spell(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->combat_flags != 0) && !creature_is_being_unconscious(thing))
    {
        if ((param->plyr_idx >= 0) && (thing->owner != param->plyr_idx))
            return -1;
        if ((param->model_id > 0) && (thing->model != param->model_id))
            return -1;
        if ((param->class_id > 0) && (thing->class_id != param->class_id))
            return -1;
        if ((param->num1 != PwrK_None) && thing_affected_by_spell(thing, param->num1))
            return -1;
        return get_creature_thing_score(thing);
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function for selecting creature which is dragging a specific thing.
 * A specific thing can be selected either by index, or by class, model and owner.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with specific thing which is dragged.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_dragging_specific_thing(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if (param->num1 > 0)
    {
        if (cctrl->dragtng_idx == param->num1) {
            return LONG_MAX;
        }
        return -1;
    }
    if ((param->class_id > 0) || (param->model_id > 0) || (param->plyr_idx >= 0))
    {
        struct Thing *dragtng;
        dragtng = thing_get(cctrl->dragtng_idx);
        if ((param->plyr_idx >= 0) && (dragtng->owner != param->plyr_idx))
            return -1;
        if ((param->model_id > 0) && (dragtng->model != param->model_id))
            return -1;
        if ((param->class_id > 0) && (dragtng->class_id != param->class_id))
            return -1;
        return LONG_MAX;
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function for selecting most experienced creature.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with creature model, owner and GUI job to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_most_experienced(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    long nmaxim;
    cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    nmaxim = cctrl->explevel+1;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
        && (thing->class_id == param->class_id)
        && ((param->model_id == -1) || (thing->model == param->model_id))
        && ((param->num1 == -1) || (get_creature_gui_job(thing) == param->num1))
        && (nmaxim > maximizer) )
    {
        return nmaxim;
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function for selecting most experienced and pickable creature.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with creature model, owner and GUI job to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_most_experienced_and_pickable1(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    long nmaxim;
    cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    nmaxim = cctrl->explevel+1;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
        && (thing->class_id == param->class_id)
        && ((param->model_id == -1) || (thing->model == param->model_id))
        && ((param->num1 == -1) || (get_creature_gui_job(thing) == param->num1))
        && !thing_is_picked_up(thing)
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
 * @param param Struct with creature model, owner and GUI job to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_most_experienced_and_pickable2(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    long nmaxim;
    cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    nmaxim = cctrl->explevel+1;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
        && (thing->class_id == param->class_id)
        && ((param->model_id == -1) || (thing->model == param->model_id))
        && ((param->num1 == -1) || (get_creature_gui_job(thing) == param->num1))
        && !thing_is_picked_up(thing)
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
 * Filter function for selecting least experienced creature.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with creature model, owner and GUI job to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_least_experienced(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    long nmaxim;
    cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    nmaxim = CREATURE_MAX_LEVEL-cctrl->explevel;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((param->model_id == -1) || (thing->model == param->model_id))
      && ((param->num1 == -1) || (get_creature_gui_job(thing) == param->num1))
      && (nmaxim > maximizer) )
    {
        return nmaxim;
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function for selecting least experienced and pickable creature.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with creature model, owner and GUI job to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_least_experienced_and_pickable1(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    long nmaxim;
    cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    nmaxim = CREATURE_MAX_LEVEL-cctrl->explevel;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((param->model_id == -1) || (thing->model == param->model_id))
      && ((param->num1 == -1) || (get_creature_gui_job(thing) == param->num1))
      && !thing_is_picked_up(thing)
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
 * @param param Struct with creature model, owner and GUI job to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_least_experienced_and_pickable2(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct CreatureControl *cctrl;
    long nmaxim;
    cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    nmaxim = CREATURE_MAX_LEVEL-cctrl->explevel;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((param->model_id == -1) || (thing->model == param->model_id))
      && ((param->num1 == -1) || (get_creature_gui_job(thing) == param->num1))
      && !thing_is_picked_up(thing)
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
 * Filter function for selecting first creature with given GUI Job.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with creature model, owner and GUI Job to be accepted.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_of_gui_job(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((param->model_id == -1) || (thing->model == param->model_id))
      && ((param->num1 == -1) || (get_creature_gui_job(thing) == param->num1))) // job_idx
    {
        // New 'maximizer' equal to MAX_LONG will stop the sweeping
        // and return this thing immediately.
        return LONG_MAX;
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
long player_list_creature_filter_of_gui_job_and_pickable1(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((param->model_id == -1) || (thing->model == param->model_id))
      && !thing_is_picked_up(thing)
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
long player_list_creature_filter_of_gui_job_and_pickable2(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((param->model_id == -1) || (thing->model == param->model_id))
      && !thing_is_picked_up(thing)
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

/**
 * Returns a creature in fight which gives highest score value.
 * @return The thing in fight, or invalid thing if not found.
 */
struct Thing *find_players_highest_score_creature_in_fight_not_affected_by_spell(PlayerNumber plyr_idx, PowerKind pwkind)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    struct Dungeon *dungeon;
    struct Thing *creatng;
    dungeon = get_players_num_dungeon(plyr_idx);
    param.plyr_idx = -1;
    param.class_id = 0;
    param.model_id = 0;
    param.num1 = pwkind;
    filter = player_list_creature_filter_in_fight_and_not_affected_by_spell;
    creatng = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    if (thing_is_invalid(creatng)) {
        creatng = get_player_list_creature_with_filter(dungeon->digger_list_start, filter, &param);
    }
    return creatng;
}

/**
 * Returns a creature who is dragging given thing, if it belongs to given player.
 * @param plyr_idx Player index whose creatures are to be checked.
 * @param dragtng The thing being dragged.
 * @return The thing which is dragging, or invalid thing if not found.
 * @see find_creature_dragging_thing()
 */
struct Thing *find_players_creature_dragging_thing(PlayerNumber plyr_idx, const struct Thing *dragtng)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    struct Dungeon *dungeon;
    struct Thing *creatng;
    dungeon = get_players_num_dungeon(plyr_idx);
    filter = player_list_creature_filter_dragging_specific_thing;
    param.class_id = TCls_Creature;
    param.model_id = -1;
    param.plyr_idx = -1;
    param.num1 = dragtng->index;
    param.num2 = -1;
    param.num3 = -1;
    creatng = get_player_list_creature_with_filter(dungeon->digger_list_start, filter, &param);
    if (thing_is_invalid(creatng)) {
        creatng = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    return creatng;
}

/**
 * Returns a creature who is dragging given thing.
 * @param dragtng The thing being dragged.
 * @return The thing which is dragging, or invalid thing if not found.
 */
struct Thing *find_creature_dragging_thing(const struct Thing *dragtng)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    filter = player_list_creature_filter_dragging_specific_thing;
    param.class_id = TCls_Creature;
    param.model_id = -1;
    param.plyr_idx = -1;
    param.num1 = dragtng->index;
    param.num2 = -1;
    param.num3 = -1;
    return get_nth_thing_of_class_with_filter(filter, &param, 0);
}

/**
 * Returns highest level creature of given kind which is owned by given player.
 * @param breed_idx The creature kind index, or -1 if all special diggers are to be accepted.
 * @param pick_check Changes the check function which determines whether the creature is pickable.
 * @return
 */
struct Thing *find_players_highest_level_creature_of_breed_and_gui_job(long crmodel, long job_idx, PlayerNumber plyr_idx, unsigned char pick_check)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    param.plyr_idx = plyr_idx;
    param.class_id = TCls_Creature;
    param.model_id = crmodel;
    param.num1 = job_idx;
    switch (pick_check)
    {
    default:
        WARNLOG("Invalid check selection, %d",(int)pick_check);
        // no break
    case 0:
        filter = player_list_creature_filter_most_experienced;
        break;
    case 1:
        filter = player_list_creature_filter_most_experienced_and_pickable1;
        break;
    case 2:
        filter = player_list_creature_filter_most_experienced_and_pickable2;
        break;
    }
    TbBool is_spec_digger;
    is_spec_digger = false;
    if (crmodel > 0) {
        //TODO DIGGERS For now, only player-specific special diggers are on the diggers list
        is_spec_digger = (crmodel == get_players_special_digger_model(plyr_idx));
        //struct CreatureModelConfig *crconf;
        //crconf = &crtr_conf.model[crmodel];
        //is_spec_digger = ((crconf->model_flags & MF_IsSpectator) != 0);
    }
    struct Thing *thing;
    thing = INVALID_THING;
    if ((is_spec_digger) || (crmodel == -1))
    {
        thing = get_player_list_creature_with_filter(dungeon->digger_list_start, filter, &param);
    }
    if (((!is_spec_digger) || (crmodel == -1)) && thing_is_invalid(thing))
    {
        thing = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    return thing;
}

/**
 * Returns lowest level creature of given kind which is owned by given player.
 * @param breed_idx The creature kind index, or -1 if all are to be accepted.
 * @param pick_check Changes the check function which determines whether the creature is pickable.
 * @return
 */
struct Thing *find_players_lowest_level_creature_of_breed_and_gui_job(long crmodel, long job_idx, PlayerNumber plyr_idx, unsigned char pick_check)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    param.plyr_idx = plyr_idx;
    param.class_id = TCls_Creature;
    param.model_id = crmodel;
    param.num1 = job_idx;
    switch (pick_check)
    {
    default:
        WARNLOG("Invalid check selection, %d",(int)pick_check);
        // no break
    case 0:
        filter = player_list_creature_filter_least_experienced;
        break;
    case 1:
        filter = player_list_creature_filter_least_experienced_and_pickable1;
        break;
    case 2:
        filter = player_list_creature_filter_least_experienced_and_pickable2;
        break;
    }
    TbBool is_spec_digger;
    is_spec_digger = false;
    if (crmodel > 0) {
        //TODO DIGGERS For now, only player-specific special diggers are on the diggers list
        is_spec_digger = (crmodel == get_players_special_digger_model(plyr_idx));
        //struct CreatureModelConfig *crconf;
        //crconf = &crtr_conf.model[crmodel];
        //is_spec_digger = ((crconf->model_flags & MF_IsSpectator) != 0);
    }
    struct Thing *thing;
    thing = INVALID_THING;
    if ((!is_spec_digger) || (crmodel == -1))
    {
        thing = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    if (((is_spec_digger) || (crmodel == -1)) && thing_is_invalid(thing))
    {
        thing = get_player_list_creature_with_filter(dungeon->digger_list_start, filter, &param);
    }
    return thing;
}

/**
 * Returns first creature of given kind which is doing given job and is owned by given player.
 * @param breed_idx The creature kind index, or -1 if all special diggers are to be accepted.
 * @param job_idx Creature GUI job, or -1 if all jobs are to be accepted.
 * @param pick_check Changes the check function which determines whether the creature is pickable.
 * @return
 */
struct Thing *find_players_first_creature_of_breed_and_gui_job(long crmodel, long job_idx, PlayerNumber plyr_idx, unsigned char pick_check)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    struct Dungeon *dungeon;
    SYNCDBG(5,"Searching for model %d, GUI job %d",(int)crmodel,(int)job_idx);
    dungeon = get_players_num_dungeon(plyr_idx);
    param.plyr_idx = plyr_idx;
    param.class_id = TCls_Creature;
    param.model_id = crmodel;
    param.num1 = job_idx;
    switch (pick_check)
    {
    default:
        WARNLOG("Invalid check selection, %d",(int)pick_check);
        // no break
    case 0:
        filter = player_list_creature_filter_of_gui_job;
        break;
    case 1:
        filter = player_list_creature_filter_of_gui_job_and_pickable1;
        break;
    case 2:
        filter = player_list_creature_filter_of_gui_job_and_pickable2;
        break;
    }
    TbBool is_spec_digger;
    is_spec_digger = false;
    if (crmodel > 0) {
        //TODO DIGGERS For now, only player-specific special diggers are on the diggers list
        is_spec_digger = (crmodel == get_players_special_digger_model(plyr_idx));
        //struct CreatureModelConfig *crconf;
        //crconf = &crtr_conf.model[crmodel];
        //is_spec_digger = ((crconf->model_flags & MF_IsSpectator) != 0);
    }
    struct Thing *thing;
    thing = INVALID_THING;
    if ((!is_spec_digger) || (crmodel == -1))
    {
        thing = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    if (((is_spec_digger) || (crmodel == -1)) && thing_is_invalid(thing))
    {
        thing = get_player_list_creature_with_filter(dungeon->digger_list_start, filter, &param);
    }
    return thing;
}

/**
 * Gives next creature of given kind and GUI job which belongs to given player.
 *
 * @param breed_idx The creature kind index, or -1 if all special diggers are to be accepted.
 * @param job_idx Creature GUI job, or -1 if all jobs are to be accepted.
 * @param plyr_idx Player to whom the thing should belong to.
 * @param pick_check Changes the check function which determines whether the creature is pickable.
 * @return
 */
struct Thing *find_players_next_creature_of_breed_and_gui_job(long crmodel, long job_idx, PlayerNumber plyr_idx, unsigned char pick_flags)
{
    struct CreatureControl *cctrl;
    struct Dungeon *dungeon;
    struct Thing *thing;
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    long i;
    SYNCDBG(5,"Searching for model %d, GUI job %d",(int)crmodel,(int)job_idx);
    //return _DK_find_my_next_creature_of_breed_and_job(crmodel, job_idx, (pick_flags & TPF_PickableCheck) != 0);
    thing = INVALID_THING;
    dungeon = get_players_num_dungeon(plyr_idx);
    /* Check if we should start the search with a creature after last one, not from start of the list */
    if ((pick_flags & TPF_OrderedPick) == 0)
    {
        if (crmodel != -1)
        {
            i = dungeon->selected_creatures_of_model[crmodel];
            thing = thing_get(i);
            // If the index is invalid, don't try to use it
            if (!thing_exists(thing) || !thing_is_creature(thing) || (thing->model != crmodel) || (thing->owner != plyr_idx))
            {
                dungeon->selected_creatures_of_model[crmodel] = 0;
                thing = INVALID_THING;
            }
        } else
        if (job_idx != -1)
        {
            i = dungeon->selected_creatures_of_gui_job[job_idx];
            thing = thing_get(i);
            // If the index is invalid, don't try to use it
            if (!thing_exists(thing) || !thing_is_creature(thing) ||  (thing->owner != plyr_idx))
            {
                dungeon->selected_creatures_of_gui_job[job_idx] = 0;
                thing = INVALID_THING;
            }
        }
    }
    // If the thing previously picked up seem right, allow next creature to be checked first
    if (!thing_is_invalid(thing))
    {
        cctrl = creature_control_get_from_thing(thing);
        if ((pick_flags & TPF_ReverseOrder) != 0)
        {
            thing = thing_get(cctrl->players_prev_creature_idx);
        } else
        {
            thing = thing_get(cctrl->players_next_creature_idx);
        }
    }

    /* If requested ordered pick, get either highest or lowest level creature */
    if ((pick_flags & TPF_OrderedPick) != 0)
    {
        if ((pick_flags & TPF_ReverseOrder) != 0)
        {
            thing = find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, job_idx, plyr_idx, (pick_flags & TPF_PickableCheck) ? 1 : 2);
        } else
        {
            thing = find_players_highest_level_creature_of_breed_and_gui_job(crmodel, job_idx, plyr_idx, (pick_flags & TPF_PickableCheck) ? 1 : 2);
        }
    } else
    /* If filtering is unordered, use the index of previous creature */
    if (!thing_is_invalid(thing))
    {
        param.plyr_idx = plyr_idx;
        param.class_id = TCls_Creature;
        param.model_id = crmodel;
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
    // If nothing found yet, use an algorithm which returns a first match
    if (thing_is_invalid(thing))
    {
        thing = find_players_first_creature_of_breed_and_gui_job(crmodel, job_idx, plyr_idx, (pick_flags & TPF_PickableCheck) ? 1 : 2);
    }
    // If no matches were found, then there are simply no matching creatures
    if (thing_is_invalid(thing))
    {
        return INVALID_THING;
    }
    // Remember the creature we've found
    if (crmodel != -1)
    {
        if (thing->model != crmodel) {
            ERRORLOG("Searched for model %d, but found %d.",(int)crmodel,(int)thing->model);
        }
        dungeon->selected_creatures_of_model[thing->model] = thing->index;
    }
    if (job_idx != -1)
    {
        if (get_creature_gui_job(thing) != job_idx) {
            ERRORLOG("Searched for GUI job %d, but found %d.",(int)job_idx,(int)get_creature_gui_job(thing));
        }
        dungeon->selected_creatures_of_gui_job[get_creature_gui_job(thing)] = thing->index;
    }
    return thing;
}

struct Thing *pick_up_creature_of_model_and_gui_job(long crmodel, long job_idx, PlayerNumber plyr_idx, unsigned char pick_flags)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    thing = find_players_next_creature_of_breed_and_gui_job(crmodel, job_idx, plyr_idx, pick_flags);
    if (thing_is_invalid(thing))
    {
        SYNCDBG(2,"Can't find creature of model %d and GUI job %d.",(int)crmodel,(int)job_idx);
        return INVALID_THING;
    }
    dungeon = get_dungeon(plyr_idx);
    if ((crmodel > 0) && (crmodel < CREATURE_TYPES_COUNT))
    {
        if ((job_idx == -1) || (dungeon->guijob_all_creatrs_count[crmodel][job_idx & 0x03]))
        {
            set_players_packet_action(get_player(plyr_idx), PckA_UsePwrHandPick, thing->index, 0);
        }
    } else
    if ((crmodel == -1))
    {
        set_players_packet_action(get_player(plyr_idx), PckA_UsePwrHandPick, thing->index, 0);
    } else
    {
        ERRORLOG("Creature model %d out of range.",(int)crmodel);
    }
    return thing;
}

/**
 *
 * @param crmodel
 * @param job_idx
 * @note originally was go_to_next_creature_of_breed_and_job()
 */
void go_to_next_creature_of_model_and_gui_job(long crmodel, long job_idx)
{
    //_DK_go_to_next_creature_of_breed_and_job(crmodel, job_idx); return;
    struct Thing *creatng;
    creatng = find_players_next_creature_of_breed_and_gui_job(crmodel, job_idx, my_player_number, 0);
    if (!thing_is_invalid(creatng))
    {
        struct Packet *pckt;
        pckt = get_packet_direct(my_player_number);
        set_packet_action(pckt, PckA_Unknown087, creatng->mappos.x.val, creatng->mappos.y.val);
    }
}

TbBool creature_is_doing_job_in_room_of_kind(const struct Thing *creatng, RoomKind rkind)
{
    {
        // Check if we're just starting a job related to that room
        CrtrStateId pvstate;
        CrtrStateId nxstate;
        pvstate = get_creature_state_besides_interruptions(creatng);
        nxstate = get_initial_state_for_job(get_job_for_room(rkind, JoKF_None, Job_NULL));
        if ((pvstate != CrSt_Unused) && (pvstate == nxstate)) {
            return true;
        }
    }
    {
        // Check if we're already working in that room kind
        struct Room *room;
        room = get_room_creature_works_in(creatng);
        if (!room_is_invalid(room)) {
            return (room->kind == rkind);
        }
    }
    return false;
}

long player_list_creature_filter_needs_to_be_placed_in_room_for_job(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct Computer2 *comp;
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    SYNCDBG(19,"Starting");
    comp = (struct Computer2 *)(param->ptr1);
    dungeon = comp->dungeon;
    if (!can_thing_be_picked_up_by_player(thing, dungeon->owner))
        return -1;
    if (creature_is_being_dropped(thing))
        return -1;
    cctrl = creature_control_get_from_thing(thing);
    crstat = creature_stats_get_from_thing(thing);

    // If the creature is too angry to help it
    if (creature_is_doing_anger_job(thing) || anger_is_creature_livid(thing))
    {
        // If the creature is not running free, then leave it where it is
        if (creature_is_kept_in_prison(thing) ||
            creature_is_being_tortured(thing) ||
            creature_is_being_sacrificed(thing))
            return -1;
        // Try torturing it
        if (dungeon_has_room(dungeon, get_room_for_job(Job_PAINFUL_TORTURE)))
        {
            param->num2 = Job_PAINFUL_TORTURE;
            return LONG_MAX;
        }
        // Or putting in prison
        if (dungeon_has_room(dungeon, get_room_for_job(Job_CAPTIVITY)))
        {
            param->num2 = Job_CAPTIVITY;
            return LONG_MAX;
        }
        // If we can't, then just let it leave the dungeon
        if (dungeon_has_room(dungeon, get_room_for_job(Job_EXEMPT)))
        {
            param->num2 = Job_EXEMPT;
            return LONG_MAX;
        }
    }

    int health_permil;
    health_permil = get_creature_health_permil(thing);
    // If it's angry but not furious, or has lost half or health due to disease,
    // then should be placed in temple
    if ((anger_is_creature_angry(thing) ||
     (creature_affected_by_spell(thing, SplK_Disease) && (health_permil < 500)))
     && creature_can_do_job_for_player(thing, dungeon->owner, Job_TEMPLE_PRAY, JobChk_None))
    {
        // If already at temple, then don't do anything
        if (creature_is_doing_temple_pray_activity(thing))
            return -1;
        if (dungeon_has_room(dungeon, get_room_for_job(Job_TEMPLE_PRAY)))
        {
            param->num2 = Job_TEMPLE_PRAY;
            return LONG_MAX;
        }
    }

    // If the creature require healing, then drop it to lair
    if (creature_can_do_healing_sleep(thing))
    {
        if (cctrl->combat_flags)
        {
            // Simplified algorithm when creature is in combat
            if (health_permil < 1000*crstat->heal_threshold/256)
            {
                // If already at lair, then don't do anything
                if (creature_is_doing_lair_activity(thing))
                    return -1;
                // otherwise, put it into room we want
                if (dungeon_has_room(dungeon, get_room_for_job(Job_TAKE_SLEEP)))
                {
                    param->num2 = Job_TAKE_SLEEP;
                    return LONG_MAX;
                }
            }
            return -1;
        } else
        {
            // Be more careful when not in combat
            if ((health_permil < 1000*crstat->heal_threshold/256) || !creature_has_lair_room(thing))
            {
                // If already at lair, then don't do anything
                if (creature_is_doing_lair_activity(thing))
                    return -1;
                // don't force it to lair if it wants to eat or take salary
                if (creature_is_doing_garden_activity(thing) || creature_is_taking_salary_activity(thing))
                    return -1;
                // otherwise, put it into room we want
                if (dungeon_has_room(dungeon, get_room_for_job(Job_TAKE_SLEEP)))
                {
                    param->num2 = Job_TAKE_SLEEP;
                    return LONG_MAX;
                }
            }
        }
    }

    // If creature is hungry, place it at garden
    if (hunger_is_creature_hungry(thing))
    {
        // If already at garden, then don't do anything
        if (creature_is_doing_garden_activity(thing))
            return -1;
        // don't force it if it wants to take salary
        if (creature_is_taking_salary_activity(thing))
            return -1;
        // otherwise, put it into room we want
        if (dungeon_has_room(dungeon, get_room_for_job(Job_TAKE_FEED)))
        {
            param->num2 = Job_TAKE_FEED;
            return LONG_MAX;
        }
    }

    // If creature wants salary, let it go get the gold
    if ( cctrl->field_48 )
    {
        // If already taking salary, then don't do anything
        if (creature_is_taking_salary_activity(thing))
            return -1;
        if (dungeon_has_room(dungeon, get_room_for_job(Job_TAKE_SALARY)))
        {
            param->num2 = Job_TAKE_SALARY;
            return LONG_MAX;
        }
    }

    TbBool force_state_reset;
    force_state_reset = false;
    // Creatures may have primary jobs other than training, or selected when there was no possibility to train
    // Make sure they are re-assigned sometimes
    if (creature_could_be_placed_in_better_room(comp, thing))
    {
        force_state_reset = true;
    }

    // Get other rooms the creature may work in
    if (creature_state_is_unset(thing) || force_state_reset)
    {
        CreatureJob new_job;
        new_job = get_job_to_place_creature_in_room(comp, thing);
        // Make sure the place we've selected is not the same as the one creature works in now
        if (!creature_is_doing_job_in_room_of_kind(thing, get_room_for_job(new_job)))
        {
            param->num2 = new_job;
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
      pos.x.val += distance_with_angle_to_coord_x(64, i);
      pos.y.val += distance_with_angle_to_coord_y(64, i);
      return create_thing(&pos, TCls_EffectElem, model, owner, -1);
  case 2:
      i = (phase - 512);
      pos.x.val -= distance_with_angle_to_coord_x(64, i);
      pos.y.val -= distance_with_angle_to_coord_y(64, i);
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
            if (!thing_is_invalid(footng)) {
                cctrl->bloody_footsteps_turns--;
            }
        }
        break;
    default:
        footng = create_footprint_sine(&thing->mappos, thing->move_angle_xy, nfoot, 23, thing->owner);
        if (!thing_is_invalid(footng)) {
            cctrl->bloody_footsteps_turns--;
        }
        break;
    }
  }

TbBool update_controlled_creature_movement(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    TbBool upd_done = false;
    if ((thing->movement_flags & TMvF_Flying) != 0)
    {
        if (cctrl->move_speed != 0)
        {
            cctrl->moveaccel.x.val = distance3d_with_angles_to_coord_x(cctrl->move_speed, thing->move_angle_xy, thing->move_angle_z);
            cctrl->moveaccel.y.val = distance3d_with_angles_to_coord_y(cctrl->move_speed, thing->move_angle_xy, thing->move_angle_z);
            cctrl->moveaccel.z.val = distance_with_angle_to_coord_z(cctrl->move_speed, thing->move_angle_z);
        }
        if (cctrl->orthogn_speed != 0)
        {
            cctrl->moveaccel.x.val += distance_with_angle_to_coord_x(cctrl->orthogn_speed, thing->move_angle_xy - LbFPMath_PI/2);
            cctrl->moveaccel.y.val += distance_with_angle_to_coord_y(cctrl->orthogn_speed, thing->move_angle_xy - LbFPMath_PI/2);
        }
    } else
    {
        if (cctrl->move_speed != 0)
        {
            cctrl->moveaccel.x.val = distance_with_angle_to_coord_x(cctrl->move_speed, thing->move_angle_xy);
            cctrl->moveaccel.y.val = distance_with_angle_to_coord_y(cctrl->move_speed, thing->move_angle_xy);
            upd_done = true;
        }
        if (cctrl->orthogn_speed != 0)
        {
            cctrl->moveaccel.x.val += distance_with_angle_to_coord_x(cctrl->orthogn_speed, thing->move_angle_xy - LbFPMath_PI/2);
            cctrl->moveaccel.y.val += distance_with_angle_to_coord_y(cctrl->orthogn_speed, thing->move_angle_xy - LbFPMath_PI/2);
            upd_done = true;
        }
    }
    return upd_done;
}

TbBool update_flight_altitude_towards_typical(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    MapCoordDelta thing_curr_alt, i;
    cctrl = creature_control_get_from_thing(thing);
    struct Coord3d nxpos;
    nxpos.x.val = thing->mappos.x.val + cctrl->moveaccel.x.val;
    nxpos.y.val = thing->mappos.y.val + cctrl->moveaccel.y.val;
    nxpos.z.val = subtile_coord(1,0);
    MapCoord floor_height, ceiling_height;
    get_floor_and_ceiling_height_under_thing_at(thing, &nxpos, &floor_height, &ceiling_height);
    thing_curr_alt = thing->mappos.z.val;
    SYNCDBG(16,"The height for %s index %d owner %d must fit between %d and %d, now is %d",thing_model_name(thing),(int)thing->index,(int)thing->owner,(int)floor_height,(int)ceiling_height,(int)thing_curr_alt);
    MoveSpeed max_speed;
    max_speed = cctrl->max_speed / 8;
    if (max_speed < 1)
        max_speed = 1;
    i = floor_height + NORMAL_FLYING_ALTITUDE;
    MapCoordDelta max_pos_to_ceiling = ceiling_height - thing->clipbox_size_yz;
    if ((floor_height < max_pos_to_ceiling) && (i > max_pos_to_ceiling))
        i = max_pos_to_ceiling;
    i -= thing_curr_alt;
    if (i > 0)
    {
        if (i >= max_speed)
            i = max_speed;
        cctrl->moveaccel.z.val += i;
        return true;
    }
    else if (i < 0)
    {
        i = -i;
        if (i >= max_speed)
            i = max_speed;
        cctrl->moveaccel.z.val -= i;
        return true;
    }
    return false;
}

short update_creature_movements(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    short upd_done;
    SYNCDBG(18,"Starting");
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return false;
    }
    upd_done = 0;
    if (cctrl->stateblock_flags != 0)
    {
        upd_done = 1;
        cctrl->moveaccel.x.val = 0;
        cctrl->moveaccel.y.val = 0;
        cctrl->moveaccel.z.val = 0;
        cctrl->move_speed = 0;
        cctrl->flgfield_2 &= ~TF2_Unkn01;
    } else
    {
      if ((thing->alloc_flags & TAlF_IsControlled) != 0)
      {
          if (update_controlled_creature_movement(thing)) {
              upd_done = 1;
          }
      } else
      if ((cctrl->flgfield_2 & TF2_Unkn01) != 0)
      {
          upd_done = 1;
          cctrl->flgfield_2 &= ~TF2_Unkn01;
      } else
      if (cctrl->move_speed != 0)
      {
          upd_done = 1;
          cctrl->moveaccel.x.val = distance_with_angle_to_coord_x(cctrl->move_speed, thing->move_angle_xy);
          cctrl->moveaccel.y.val = distance_with_angle_to_coord_y(cctrl->move_speed, thing->move_angle_xy);
          cctrl->moveaccel.z.val = 0;
      }
      if (((thing->movement_flags & TMvF_Flying) != 0) && ((thing->alloc_flags & TAlF_IsControlled) == 0))
      {
          if (update_flight_altitude_towards_typical(thing)) {
              upd_done = 1;
          }
      }
    }
    SYNCDBG(19,"Finished for %s index %d with acceleration (%d,%d,%d)",thing_model_name(thing),
        (int)thing->index,(int)cctrl->moveaccel.x.val,(int)cctrl->moveaccel.y.val,(int)cctrl->moveaccel.z.val);
    if (upd_done) {
        return true;
    } else {
        return ((cctrl->moveaccel.x.val != 0) || (cctrl->moveaccel.y.val != 0) || (cctrl->moveaccel.z.val != 0));
    }
}

void check_for_creature_escape_from_lava(struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    if (((thing->alloc_flags & TAlF_IsControlled) == 0) && ((thing->movement_flags & TMvF_IsOnLava) != 0))
    {
      crstat = creature_stats_get_from_thing(thing);
      if (crstat->hurt_by_lava > 0)
      {
          cctrl = creature_control_get_from_thing(thing);
          if ((!creature_is_escaping_death(thing)) && (cctrl->field_2FE + 64 < game.play_gameturn))
          {
              cctrl->field_2FE = game.play_gameturn;
              if ( cleanup_current_thing_state(thing) )
              {
                if ( setup_move_off_lava(thing) ) {
                    thing->continue_state = CrSt_CreatureEscapingDeath;
                } else {
                    set_start_state(thing);
                }
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
    if ((thing->movement_flags & TMvF_IsOnWater) != 0)
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
        footng = create_footprint_sine(&thing->mappos, thing->move_angle_xy, nfoot, 23, thing->owner);
        if (!thing_is_invalid(footng)) {
            cctrl->bloody_footsteps_turns--;
        }
    } else
    // Snow footprints
    if (game.texture_id == 2)
    {
        slb = get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        if (slb->kind == SlbT_PATH)
        {
          thing->movement_flags |= TMvF_Unknown80;
          nfoot = get_foot_creature_has_down(thing);
          footng = create_footprint_sine(&thing->mappos, thing->move_angle_xy, nfoot, 94, thing->owner);
        }
    }
}

/**
 * Applies given damage points to a creature, considering its defensive abilities, and shows health flower.
 * Uses the creature defense value to compute the actual damage.
 * Can be used only to make damage - never to heal creature.
 *
 * @param thing
 * @param dmg
 * @param damage_type
 * @param inflicting_plyr_idx
 */
HitPoints apply_damage_to_thing_and_display_health(struct Thing *thing, HitPoints dmg, DamageType damage_type, PlayerNumber inflicting_plyr_idx)
{
    //_DK_apply_damage_to_thing_and_display_health(thing, a1, inflicting_plyr_idx);
    HitPoints cdamage;
    if (dmg > 0)
    {
        cdamage = apply_damage_to_thing(thing, dmg, damage_type, inflicting_plyr_idx);
    } else {
        cdamage = 0;
    }
    if (cdamage > 0) {
        thing->creature.health_bar_turns = 8;
    }
    return cdamage;
}

void process_landscape_affecting_creature(struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    unsigned long navheight;
    int stl_idx;
    int i;
    SYNCDBG(18,"Starting");
    thing->movement_flags &= ~TMvF_IsOnWater;
    thing->movement_flags &= ~TMvF_IsOnLava;
    thing->movement_flags &= ~TMvF_Unknown80;
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return;
    }
    cctrl->field_B9 = 0;

    stl_idx = get_subtile_number(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
    navheight = get_navigation_map_floor_height(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
    if (subtile_coord(navheight,0) == thing->mappos.z.val)
    {
        i = get_top_cube_at_pos(stl_idx);
        if (cube_is_lava(i))
        {
            crstat = creature_stats_get_from_thing(thing);
            apply_damage_to_thing_and_display_health(thing, crstat->hurt_by_lava, DmgT_Heatburn, -1);
            thing->movement_flags |= TMvF_IsOnLava;
        } else
        if (cube_is_water(i))
        {
            thing->movement_flags |= TMvF_IsOnWater;
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
    if (is_neutral_thing(thing))
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
    if (is_neutral_thing(thing))
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

void init_creature_scores(void)
{
    long i,k;
    long score,max_score;
    // compute maximum score
    max_score = 0;
    for (i=0; i < CREATURE_TYPES_COUNT; i++)
    {
        score = compute_creature_kind_score(i,CREATURE_MAX_LEVEL-1);
        if ((score <= 0) && (i != 0) && (i != CREATURE_TYPES_COUNT-1))
        {
          ERRORLOG("Couldn't get creature %d score value", (int)i);
          continue;
        }
        if (score > max_score)
        {
          max_score = score;
        }
    }
    if (max_score <= 0)
    {
        ERRORLOG("Creatures have no score");
        return;
    }
    // now compute scores for experience levels
    for (i=0; i < CREATURE_TYPES_COUNT; i++)
    {
        for (k=0; k < CREATURE_MAX_LEVEL; k++)
        {
          score = compute_creature_kind_score(i,k);
          score = saturate_set_unsigned(200*score / max_score, 8);
          if ((score <= 0) && (i != 0) && (i != 31))
          {
            //WARNMSG("Couldn't get creature %d score for lev %d", i, k);
            score = 1;
          }
          game.creature_scores[i].value[k] = score;
        }
    }
}

long get_creature_thing_score(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    long crmodel,exp;
    cctrl = creature_control_get_from_thing(thing);
    crmodel = thing->model;
    if (crmodel >= CREATURE_TYPES_COUNT)
        crmodel = 0;
    if (crmodel < 0)
        crmodel = 0;
    exp = cctrl->explevel;
    if (exp >= CREATURE_MAX_LEVEL)
        exp = 0;
    if (exp < 0)
        exp = 0;
    return game.creature_scores[crmodel].value[exp];
}

long update_creature_levels(struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct PlayerInfo *player;
    struct CreatureControl *cctrl;
    struct Thing *newtng;
    SYNCDBG(18,"Starting");
    cctrl = creature_control_get_from_thing(thing);
    if ((cctrl->spell_flags & CSAfF_ExpLevelUp) == 0)
        return 0;
    cctrl->spell_flags &= ~CSAfF_ExpLevelUp;
    // If a creature is not on highest level, just update the level
    if (cctrl->explevel+1 < CREATURE_MAX_LEVEL)
    {
        remove_creature_score_from_owner(thing); // the opposite is in set_creature_level()
        set_creature_level(thing, cctrl->explevel+1);
        return 1;
    }
    // If it is highest level, maybe we should transform the creature?
    crstat = creature_stats_get_from_thing(thing);
    if (crstat->grow_up == 0) {
        return 0;
    }
    // Transforming
    newtng = create_creature(&thing->mappos, crstat->grow_up, thing->owner);
    if (thing_is_invalid(newtng))
    {
        ERRORLOG("Could not create creature to transform %s to",thing_model_name(thing));
        return 0;
    }
    set_creature_level(newtng, crstat->grow_up_level-1);
    update_creature_health_to_max(newtng);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->countdown_282 = 50;
    external_set_thing_state(newtng, CrSt_CreatureBeHappy);
    player = get_player(thing->owner);
    // Switch control if this creature is possessed
    if (is_thing_directly_controlled(thing))
    {
        leave_creature_as_controller(player, thing);
        control_creature_as_controller(player, newtng);
    }
    if (is_thing_passenger_controlled(thing))
    {
        leave_creature_as_passenger(player, thing);
        control_creature_as_passenger(player, newtng);
    }
    // If not directly nor passenger controlled, but still player is doing something with it
    if (thing->index == player->controlled_thing_idx)
    {
        set_selected_creature(player, newtng);
    }
    remove_creature_score_from_owner(thing); // kill_creature() doesn't call this
    kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects|CrDed_NoUnconscious|CrDed_NotReallyDying);
    return -1;
}

TngUpdateRet update_creature(struct Thing *thing)
{
    struct PlayerInfo *player;
    struct CreatureControl *cctrl;
    struct Thing *tngp;
    SYNCDBG(19,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    TRACE_THING(thing);
    if ((thing->active_state == CrSt_CreatureUnconscious) && subtile_is_door(thing->mappos.x.stl.num, thing->mappos.y.stl.num))
    {
        SYNCDBG(8,"Killing unconscious %s index %d on door block.",thing_model_name(thing),(int)thing->index);
        kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects|CrDed_NoUnconscious);
        return TUFRet_Deleted;
    }
    if (thing->health < 0)
    {
        kill_creature(thing, INVALID_THING, -1, CrDed_Default);
        return TUFRet_Deleted;
    }
    cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        WARNLOG("Killing %s index %d with invalid control.",thing_model_name(thing),(int)thing->index);
        kill_creature(thing, INVALID_THING, -1, CrDed_Default);
        return TUFRet_Deleted;
    }
    process_armageddon_influencing_creature(thing);

    if (cctrl->field_B1 > 0)
        cctrl->field_B1--;
    if (cctrl->force_visible > 0)
        cctrl->force_visible--;
    if (cctrl->byte_8B == 0)
        cctrl->byte_8B = game.field_14EA4B;
    if (cctrl->field_302 == 0) {
        process_creature_instance(thing);
    }
    update_creature_count(thing);
    if ((thing->alloc_flags & TAlF_IsControlled) != 0)
    {
        if ((cctrl->stateblock_flags == 0) || creature_state_cannot_be_blocked(thing))
        {
            if (cctrl->field_302 > 0)
            {
                cctrl->field_302--;
            } else
            if (process_creature_state(thing) == TUFRet_Deleted)
            {
                ERRORLOG("Human controlled creature has been deleted by state routine.");
                return TUFRet_Deleted;
            }
        }
        cctrl = creature_control_get_from_thing(thing);
        player = get_player(thing->owner);
        if (creature_affected_by_spell(thing, SplK_Freeze))
        {
            if ((player->field_3 & 0x04) == 0)
              PaletteSetPlayerPalette(player, blue_palette);
        } else
        {
            if ((player->field_3 & 0x04) != 0)
              PaletteSetPlayerPalette(player, engine_palette);
        }
    } else
    {
        if ((cctrl->stateblock_flags == 0) || creature_state_cannot_be_blocked(thing))
        {
            if (cctrl->field_302 > 0)
            {
                cctrl->field_302--;
            } else
            if (process_creature_state(thing) == TUFRet_Deleted)
            {
                return TUFRet_Deleted;
            }
        }
    }

    if (update_creature_movements(thing))
    {
        SYNCDBG(19,"The %s index %d acceleration is (%d,%d,%d)",thing_model_name(thing),
            (int)thing->index,(int)cctrl->moveaccel.x.val,(int)cctrl->moveaccel.y.val,(int)cctrl->moveaccel.z.val);
        thing->velocity.x.val += cctrl->moveaccel.x.val;
        thing->velocity.y.val += cctrl->moveaccel.y.val;
        thing->velocity.z.val += cctrl->moveaccel.z.val;
    }
    move_creature(thing);
    if ((thing->alloc_flags & TAlF_IsControlled) != 0)
    {
        if ((cctrl->flgfield_1 & CCFlg_Unknown40) == 0)
          cctrl->move_speed /= 2;
        if ((cctrl->flgfield_1 & CCFlg_Unknown80) == 0)
          cctrl->orthogn_speed /= 2;
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

    if (thing->creature.health_bar_turns > 0)
        thing->creature.health_bar_turns--;

    if (creature_is_group_member(thing))
    {
        if (creature_is_group_leader(thing)) {
            leader_find_positions_for_followers(thing);
        }
    }

    if (cctrl->dragtng_idx > 0)
    {
        tngp = thing_get(cctrl->dragtng_idx);
        if ((tngp->state_flags & TF1_IsDragged1) != 0)
          move_thing_in_map(tngp, &thing->mappos);
    }
    if (update_creature_levels(thing) == -1)
    {
        return TUFRet_Deleted;
    }
    process_creature_self_spell_casting(thing);
    cctrl->moveaccel.x.val = 0;
    cctrl->moveaccel.y.val = 0;
    cctrl->moveaccel.z.val = 0;
    cctrl->flgfield_1 &= ~CCFlg_Unknown40;
    cctrl->flgfield_1 &= ~CCFlg_Unknown80;
    cctrl->spell_flags &= ~CSAfF_PoisonCloud;
    process_thing_spell_effects(thing);
    SYNCDBG(19,"Finished");
    return TUFRet_Modified;
}

TbBool creature_is_slappable(const struct Thing *thing, PlayerNumber plyr_idx)
{
    struct Room *room;
    if (creature_is_being_unconscious(thing))
    {
        return false;
    }
    if (thing->owner != plyr_idx)
    {
      if (creature_is_kept_in_prison(thing) || creature_is_being_tortured(thing))
      {
          room = get_room_creature_works_in(thing);
          return (room->owner == plyr_idx);
      }
      return false;
    }
    if (creature_is_being_sacrificed(thing) || creature_is_being_summoned(thing))
    {
        return false;
    }
    if (creature_is_kept_in_prison(thing) || creature_is_being_tortured(thing))
    {
        room = get_room_creature_works_in(thing);
        return (room->owner == plyr_idx);
    }
    return true;
}

TbBool creature_is_invisible(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    return creature_affected_by_spell(thing, SplK_Invisibility) && (cctrl->force_visible <= 0);
}

TbBool creature_can_see_invisible(const struct Thing *thing)
{
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(thing);
    return (crstat->can_see_invisible) || creature_affected_by_spell(thing, SplK_Sight);
}

int claim_neutral_creatures_in_sight(struct Thing *creatng, struct Coord3d *pos, int can_see_slabs)
{
    long i,n;
    unsigned long k;
    MapSlabCoord slb_x, slb_y;
    slb_x = subtile_slab_fast(pos->x.stl.num);
    slb_y = subtile_slab_fast(pos->y.stl.num);
    n = 0;
    i = game.nodungeon_creatr_list_start;
    k = 0;
    while (i != 0)
    {
        struct Thing *thing;
        struct CreatureControl *cctrl;
        thing = thing_get(i);
        cctrl = creature_control_get_from_thing(thing);
        i = cctrl->players_next_creature_idx;
        // Per thing code starts
        int dx, dy;
        dx = abs(slb_x - subtile_slab_fast(thing->mappos.x.stl.num));
        dy = abs(slb_y - subtile_slab_fast(thing->mappos.y.stl.num));
        if ((dx <= can_see_slabs) && (dy <= can_see_slabs))
        {
            if (is_neutral_thing(thing) && line_of_sight_3d(&thing->mappos, pos))
            {
                change_creature_owner(thing, creatng->owner);
                mark_creature_joined_dungeon(thing);
                n++;
            }
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return n;
}

TbBool change_creature_owner_if_near_dungeon_heart(struct Thing *creatng)
{
    PlayerNumber plyr_idx;
    for (plyr_idx=0; plyr_idx < game.neutral_player_num; plyr_idx++)
    {
        struct PlayerInfo *player;
        player = get_player(plyr_idx);
        if ( ((player->allocflags & PlaF_Allocated) != 0) && (player->field_2C == 1) && (player->victory_state != VicS_LostLevel) )
        {
            struct Thing *heartng;
            heartng = get_player_soul_container(plyr_idx);
            if (thing_exists(heartng) && (get_2d_box_distance(&creatng->mappos, &heartng->mappos) < subtile_coord(6,0)))
            {
                change_creature_owner(creatng, plyr_idx);
                mark_creature_joined_dungeon(creatng);
                return true;
            }
        }
    }
    return false;
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
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Creature);
    i = slist->index;
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
        if (!is_hero_thing(thing)) {
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
