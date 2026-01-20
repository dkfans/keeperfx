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
#include "pre_inc.h"
#include <assert.h>

#include "thing_creature.h"
#include "globals.h"

#include "bflib_math.h"
#include "bflib_filelst.h"
#include "bflib_sprite.h"
#include "bflib_planar.h"
#include "bflib_vidraw.h"
#include "bflib_sound.h"
#include "bflib_fileio.h"

#include "config_creature.h"
#include "config_crtrstates.h"
#include "config_effects.h"
#include "config_lenses.h"
#include "config_magic.h"
#include "config_terrain.h"
#include "creature_battle.h"
#include "creature_graphics.h"
#include "creature_groups.h"
#include "creature_instances.h"
#include "creature_jobs.h"
#include "creature_senses.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "creature_states_gardn.h"
#include "creature_states_hero.h"
#include "creature_states_lair.h"
#include "creature_states_mood.h"
#include "creature_states_prisn.h"
#include "creature_states_spdig.h"
#include "creature_states_train.h"
#include "dungeon_data.h"
#include "engine_arrays.h"
#include "engine_lenses.h"
#include "local_camera.h"
#include "engine_redraw.h"
#include "front_input.h"
#include "front_simple.h"
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "game_legacy.h"
#include "gui_frontmenu.h"
#include "gui_msgs.h"
#include "gui_soundmsgs.h"
#include "gui_topmsg.h"
#include "kjm_input.h"
#include "lens_api.h"
#include "light_data.h"
#include "magic_powers.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "player_instances.h"
#include "config_players.h"
#include "power_hand.h"
#include "power_process.h"
#include "room_data.h"
#include "room_graveyard.h"
#include "room_jobs.h"
#include "room_library.h"
#include "room_list.h"
#include "sounds.h"
#include "spdigger_stack.h"
#include "thing_corpses.h"
#include "thing_creature.h"
#include "thing_effects.h"
#include "thing_factory.h"
#include "thing_navigate.h"
#include "thing_navigate.h"
#include "thing_objects.h"
#include "thing_physics.h"
#include "thing_shots.h"
#include "thing_stats.h"
#include "thing_traps.h"
#include "lua_triggers.h"
#include "lua_cfg_funcs.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
int creature_swap_idx[CREATURE_TYPES_MAX];
struct TbSpriteSheet * swipe_sprites = NULL;
/******************************************************************************/
/**
 * Returns creature health scaled 0..1000.
 * @param thing The creature thing.
 * @return Health value, not always in range of 0..1000.
 * @note Dying creatures may return negative health, and in some rare cases creatures
 *  can have more health than their max.
 */
HitPoints get_creature_health_permil(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    HitPoints health = thing->health;
    HitPoints max_health = cctrl->max_health;
    if (max_health < 1)
    {
        max_health = 1;
    }
    // Use int64_t as intermediary variable to prevent overflow during the multiplication.
    // HitPoints is a 32-bit type, and multiplying health by 1000 could exceed its capacity.
    // By using int64_t, we ensure that the intermediate result can hold the larger value before it's cast back to HitPoints.
    int64_t health_scaled = ((int64_t)health * 1000) / (int64_t)max_health;
    HitPoints health_permil = health_scaled;
    return health_permil;
}

TbBool thing_can_be_controlled_as_controller(struct Thing *thing)
{
    if (!thing_exists(thing))
        return false;
    if ((thing->class_id == TCls_Creature) && !creature_under_spell_effect(thing, CSAfF_Fear))
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

TbBool creature_is_for_dungeon_diggers_list(const struct Thing *creatng)
{
    return creature_kind_is_for_dungeon_diggers_list(creatng->owner,creatng->model);
}

TbBool creature_kind_is_for_dungeon_diggers_list(PlayerNumber plyr_idx, ThingModel crmodel)
{
    if (player_is_roaming(plyr_idx))
        return false;

    if (crmodel == CREATURE_DIGGER)
        return true;

    struct CreatureModelConfig *crconf;
    crconf = &game.conf.crtr_conf.model[crmodel];
    return flag_is_set(crconf->model_flags,CMF_IsSpecDigger);
}

/**
 * Creates a barracks party, when creature being possessed is barracking.
 * @param grthing
 * @return Amount of creatures in the party, including the leader.
 */
long check_for_first_person_barrack_party(struct Thing *grthing)
{
    if (!thing_is_creature(grthing))
    {
        SYNCDBG(2,"The %s cannot lead a barracks party", thing_model_name(grthing));
        return 0;
    }
    struct Room* room = get_room_thing_is_on(grthing);
    if (!room_still_valid_as_type_for_thing(room, RoRoF_CrMakeGroup, grthing))
    {
        SYNCDBG(2,"Room %s owned by player %d does not allow the %s index %d owner %d to lead a party",room_code_name(room->kind),(int)room->owner,thing_model_name(grthing),(int)grthing->index,(int)grthing->owner);
        return 0;
    }
    long n = 0;
    long i = room->creatures_list;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
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
            if (n >= game.conf.rules[grthing->owner].rooms.barrack_max_party_size || n >= GROUP_MEMBERS_COUNT)
            {
                break;
            }
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
    struct CreatureModelConfig *crconf;
    struct Camera *cam;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (((thing->owner != player->id_number) && (player->work_state != PSt_FreeCtrlDirect))
      || !thing_can_be_controlled_as_controller(thing))
    {
      if (!control_creature_as_passenger(player, thing))
        return false;
      cam = player->acamera;
      crconf = creature_stats_get(get_players_special_digger_model(player->id_number));
      cam->mappos.z.val += get_creature_eye_height(thing);
      return true;
    }
    TbBool chicken = (creature_under_spell_effect(thing, CSAfF_Chicken));
    if (!chicken)
    {
        cctrl->moveto_pos.x.val = 0;
        cctrl->moveto_pos.y.val = 0;
        cctrl->moveto_pos.z.val = 0;
    }
    if (is_my_player(player))
    {
      toggle_status_menu(0);
      turn_off_roaming_menus();
    }
    set_selected_creature(player, thing);
    cam = player->acamera;
    if (cam != NULL)
      player->view_mode_restore = cam->view_mode;
    thing->alloc_flags |= TAlF_IsControlled;
    thing->rendering_flags |= TRF_Invisible;
    if (!chicken)
    {
        set_start_state(thing);
    }
    else
    {
        internal_set_thing_state(thing, CrSt_CreaturePretendChickenSetupMove);
    }
    set_player_mode(player, PVT_CreatureContrl);
    if (thing_is_creature(thing))
    {
        cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        check_for_first_person_barrack_party(thing);
        if (creature_is_group_member(thing)) {
            make_group_member_leader(thing);
        }
    }
    crconf = creature_stats_get(thing->model);
    if ((!crconf->illuminated) && (!creature_under_spell_effect(thing, CSAfF_Light)))
    {
        create_light_for_possession(thing);
    }
    if (thing->class_id == TCls_Creature)
    {
        crconf = creature_stats_get_from_thing(thing);
        setup_eye_lens(crconf->eye_effect);
    }
    return true;
}

TbBool control_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing)
{
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
    struct Camera* cam = player->acamera;
    if (cam != NULL)
      player->view_mode_restore = cam->view_mode;
    set_player_mode(player, PVT_CreaturePasngr);
    thing->rendering_flags |= TRF_Invisible;
    return true;
}

void free_swipe_graphic(void)
{
    SYNCDBG(6,"Starting");
    free_spritesheet(&swipe_sprites);
    game.loaded_swipe_idx = -1;
}

TbBool load_swipe_graphic_for_creature(const struct Thing *thing)
{
    SYNCDBG(6,"Starting for %s",thing_model_name(thing));
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    if ((crconf->swipe_idx == 0) || (game.loaded_swipe_idx == crconf->swipe_idx))
        return true;
    free_swipe_graphic();
    int swpe_idx = crconf->swipe_idx;
    char dat_fname[2048];
    char tab_fname[2048];
#ifdef SPRITE_FORMAT_V2
    strcpy(dat_fname, prepare_file_fmtpath(FGrp_CmpgConfig, "swipe%02d-32.dat", swpe_idx));
    strcpy(tab_fname, prepare_file_fmtpath(FGrp_CmpgConfig, "swipe%02d-32.tab", swpe_idx));
    if (!LbFileExists(dat_fname)) {
        strcpy(dat_fname, prepare_file_fmtpath(FGrp_StdData, "swipe%02d-32.dat", swpe_idx));
        strcpy(tab_fname, prepare_file_fmtpath(FGrp_StdData, "swipe%02d-32.tab", swpe_idx));
    }
#else
    strcpy(dat_fname, prepare_file_fmtpath(FGrp_CmpgConfig, "swipe%02d.dat", swpe_idx));
    strcpy(tab_fname, prepare_file_fmtpath(FGrp_CmpgConfig, "swipe%02d.tab", swpe_idx));
    if (!LbFileExists(dat_fname)) {
        strcpy(dat_fname, prepare_file_fmtpath(FGrp_StdData, "swipe%02d.dat", swpe_idx));
        strcpy(tab_fname, prepare_file_fmtpath(FGrp_StdData, "swipe%02d.tab", swpe_idx));
    }
#endif
    swipe_sprites = load_spritesheet(dat_fname, tab_fname);
    if (!swipe_sprites) {
        free_swipe_graphic();
        ERRORLOG("Unable to load swipe graphics for %s",thing_model_name(thing));
        return false;
    }
    game.loaded_swipe_idx = swpe_idx;
    return true;
}

/**
 * Randomise the draw direction of the swipe sprite in the first-person possession view.
 *
 * Sets PlayerInfo->swipe_sprite_drawLR to either TRUE or FALSE.
 *
 * Draw direction is either: left-to-right (TRUE) or right-to-left (FALSE)
 */
void randomise_swipe_graphic_direction()
{
    struct PlayerInfo* myplyr = get_my_player();
    myplyr->swipe_sprite_drawLR = UNSYNC_RANDOM(2); // equal chance to be left-to-right or right-to-left
}

void draw_swipe_graphic(void)
{
    struct PlayerInfo* myplyr = get_my_player();
    struct Thing* thing = thing_get(myplyr->controlled_thing_idx);
    if (thing_is_creature(thing))
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (instance_draws_possession_swipe(cctrl->instance_id))
        {
            lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
            long n = (int)cctrl->inst_turn * (5 << 8) / cctrl->inst_total_turns;
            long allwidth = 0;
            long i = max(((abs(n) >> 8) -1),0);
            if (i >= SWIPE_SPRITE_FRAMES)
                i = SWIPE_SPRITE_FRAMES-1;
            const struct TbSprite* sprlist = get_sprite(swipe_sprites, SWIPE_SPRITES_X * SWIPE_SPRITES_Y * i);
            if (sprlist == NULL)
            {
                ERRORLOG("Failed to draw swipe sprite for thing %d", (int)thing->index);
                return;
            }
            const struct TbSprite* startspr = &sprlist[1];
            const struct TbSprite* endspr = &sprlist[1];
            for (n=0; n < SWIPE_SPRITES_X; n++)
            {
                allwidth += endspr->SWidth;
                endspr++;
            }
            int units_per_px = (LbScreenWidth() * 59 / 64) * 16 / allwidth;
            int scrpos_y = (MyScreenHeight * 16 / units_per_px - (startspr->SHeight + endspr->SHeight)) / 2;
            const struct TbSprite *spr;
            int scrpos_x;
            if (myplyr->swipe_sprite_drawLR)
            {
                int delta_y = sprlist[1].SHeight;
                for (i=0; i < SWIPE_SPRITES_X*SWIPE_SPRITES_Y; i+=SWIPE_SPRITES_X)
                {
                    spr = &startspr[i];
                    scrpos_x = ((MyScreenWidth + (2 * myplyr->engine_window_x)) * 16 / units_per_px - allwidth)/ 2;
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
                lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4 | Lb_SPRITE_FLIP_HORIZ;
                for (i=0; i < SWIPE_SPRITES_X*SWIPE_SPRITES_Y; i+=SWIPE_SPRITES_X)
                {
                    spr = &sprlist[SWIPE_SPRITES_X+i];
                    int delta_y = spr->SHeight;
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
    // we get here many times a second when in possession mode and not attacking: to randomise the swipe direction
    randomise_swipe_graphic_direction();
}

long creature_available_for_combat_this_turn(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    // Check once per 8 turns
    if (((game.play_gameturn + creatng->index) & 7) != 0)
    {
        // On first turn in a state, check anyway
        if (game.play_gameturn - cctrl->tasks_check_turn > 1) {
            return false;
        }
    }
    if (creature_is_fleeing_combat(creatng) || creature_under_spell_effect(creatng, CSAfF_Chicken)) {
        return false;
    }
    if (creature_is_being_unconscious(creatng) || creature_is_dying(creatng)) {
        return false;
    }
    if (thing_is_picked_up(creatng) || creature_is_being_dropped(creatng)) {
        return false;
    }
    if ((creatng->owner == game.neutral_player_num) || ((cctrl->creature_control_flags & CCFlg_NoCompControl) != 0)) {
        return false;
    }
    CrtrStateId i = get_creature_state_besides_interruptions(creatng);
    return can_change_from_state_to(creatng, i, CrSt_CreatureInCombat);
}

struct Thing *get_players_soul_container_creature_can_see(struct Thing *creatng, PlayerNumber heart_owner)
{
    struct Thing* heartng = get_player_soul_container(heart_owner);
    if (!thing_exists(heartng))
    {
        SYNCDBG(7,"The player %d has no heart",(int)heart_owner);
        return INVALID_THING;
    }
    int dist = get_combat_distance(creatng, heartng);
    if (creature_can_see_combat_path(creatng, heartng, dist)) {
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
    SYNCDBG(17, "Starting");
    assert(DUNGEONS_COUNT == PLAYERS_COUNT);

    for (PlayerNumber enemy_idx = 0; enemy_idx < DUNGEONS_COUNT; enemy_idx++)
    {
        if (players_are_enemies(creatng->owner, enemy_idx))
        {
            struct Thing* heartng = get_players_soul_container_creature_can_see(creatng, enemy_idx);
            if (thing_exists(heartng))
            {
                return heartng;
            }
        }
    }

    return INVALID_THING;
}

void set_creature_combat_object_state(struct Thing *creatng, struct Thing *obthing, short combattype)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->combat.battle_enemy_idx = obthing->index;
    cctrl->combat.battle_enemy_crtn = obthing->creation_turn;
    cctrl->fighting_at_same_position = 0;
    cctrl->combat_flags |= CmbtF_ObjctFight;
    const struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    if ((crconf->attack_preference == AttckT_Ranged) && creature_has_ranged_object_weapon(creatng))
    {
        if (combattype == CrSt_CreatureObjectSnipe)
        {
            cctrl->combat.state_id = ObjCmbtSt_RangedSnipe;
        }
        else
        {
            cctrl->combat.state_id = ObjCmbtSt_Ranged;
        }
    }
    else
    {
        if (combattype == CrSt_CreatureObjectSnipe)
        {
            cctrl->combat.state_id = ObjCmbtSt_MeleeSnipe;
        }
        else
        {
            cctrl->combat.state_id = ObjCmbtSt_Melee;
        }
    }
}

TbBool set_creature_object_combat(struct Thing *creatng, struct Thing *obthing)
{
    SYNCDBG(8,"Starting");
    if (!external_set_thing_state(creatng, CrSt_CreatureObjectCombat)) {
        return false;
    }
    set_creature_combat_object_state(creatng, obthing, CrSt_CreatureObjectCombat);
    SYNCDBG(19,"Finished");
    return true;
}

TbBool set_creature_object_snipe(struct Thing* creatng, struct Thing* obthing)
{
    SYNCDBG(8, "Starting");
    if (!external_set_thing_state(creatng, CrSt_CreatureObjectSnipe)) {
        return false;
    }
    set_creature_combat_object_state(creatng, obthing, CrSt_CreatureObjectSnipe);
    SYNCDBG(19, "Finished");
    return true;
}

void set_creature_combat_door_state(struct Thing *creatng, struct Thing *obthing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->combat.battle_enemy_idx = obthing->index;
    cctrl->combat.battle_enemy_crtn = obthing->creation_turn;
    cctrl->fighting_at_same_position = 0;
    cctrl->combat_flags |= CmbtF_DoorFight;
    const struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    if ((crconf->attack_preference == AttckT_Ranged)
      && creature_has_ranged_object_weapon(creatng)) {
        cctrl->combat.state_id = ObjCmbtSt_Ranged;
    } else {
        cctrl->combat.state_id = ObjCmbtSt_Melee;
    }
}

TbBool set_creature_door_combat(struct Thing *creatng, struct Thing *obthing)
{
    SYNCDBG(8,"Starting");
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->instance_id == CrInst_NULL)
    {
        set_creature_instance(creatng, CrInst_EAT, 0, 0);
    } else
    {
        if (cctrl->hunger_amount > 0) {
            cctrl->hunger_amount--;
        } else
        if (cctrl->hunger_loss < 255) {
              cctrl->hunger_loss++;
        }
        apply_health_to_thing_and_display_health(creatng, game.conf.rules[creatng->owner].health.food_health_gain);
        cctrl->hunger_level = 0;
    }
    // Food is destroyed just below, so the sound must be made by creature
    thing_play_sample(creatng, 112+SOUND_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    anger_apply_anger_to_creature(creatng, crconf->annoy_eat_food, AngR_Hungry, 1);
    struct Dungeon* dungeon = get_players_num_dungeon(creatng->owner);
    if (!dungeon_invalid(dungeon)) {
        dungeon->lvstats.chickens_eaten++;
    }
    if (thing_is_creature(foodtng))
    {
        thing_death_flesh_explosion(foodtng);
    } else
    {
        delete_thing_structure(foodtng, 0);
    }
}

void anger_apply_anger_to_creature_f(struct Thing *creatng, long anger, AnnoyMotive reason, long a3, const char *func_name)
{
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
                long angrpart = 32 * anger / 256;
                anger_increase_creature_anger_f(creatng, angrpart, AngR_Other, func_name);
            }
        }
    } else
    if (anger < 0)
    {
        anger_reduce_creature_anger_f(creatng, anger, reason, func_name);
        if (reason == AngR_Other)
        {
            long angrpart = 32 * anger / 256;
            for (AnnoyMotive reaspart = 1; reaspart < AngR_Other; reaspart++)
            {
                anger_reduce_creature_anger_f(creatng, angrpart, reaspart, func_name);
            }
        }
    }
}

TbBool creature_affected_by_slap(const struct Thing *thing)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    return (cctrl->slap_turns != 0);
}

/* Returns if spell effect is currently set on a thing.
 * @param thing The thing which can have spell effect on.
 * @param spell_flags The spell flags to be checked. */
TbBool creature_under_spell_effect_f(const struct Thing *thing, unsigned long spell_flags, const char *func_name)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("%s: Invalid creature control for thing %s index %d", func_name, thing_model_name(thing), (int)thing->index);
        return false;
    }
    // Return false for instances affecting the caster when no spell flags are set.
    if (spell_flags == 0)
    {
        return false;
    }
    return flag_is_set(cctrl->spell_flags, spell_flags);
}

/* Returns if the creature kind is immune to spell effect.
 * @param thing The thing to be checked.
 * @param spell_flags The spell flags to be checked. */
TbBool creature_is_immune_to_spell_effect_f(const struct Thing *thing, unsigned long spell_flags, const char *func_name)
{
    struct CreatureModelConfig *crconf = creature_stats_get(thing->model);
    if (creature_stats_invalid(crconf))
    {
        ERRORLOG("%s: Invalid creature stats for thing %s index %d", func_name, thing_model_name(thing), (int)thing->index);
        return false;
    }
    // Return false for instances affecting the caster when no spell flags are set.
    if (spell_flags == 0)
    {
        return false;
    }
    return flag_is_set(crconf->immunity_flags, spell_flags);
}

/* Returns an available instance associated to a spell kind that can set spell effect.
 * @param thing The thing that can use the instance.
 * @param spell_flags The spell flags to be checked. */
CrInstance get_available_instance_with_spell_effect(const struct Thing *thing, unsigned long spell_flags)
{
    struct InstanceInfo *inst_inf;
    struct ShotConfigStats* shotst;
    struct SpellConfig *spconf;
    SpellKind spell_idx;
    for (CrInstance i = 0; i < game.conf.crtr_conf.instances_count; i++)
    {
        if (creature_instance_is_available(thing, i))
        {
            inst_inf = creature_instance_info_get(i);
            if (inst_inf->func_idx == 2)
            { // Check if the instance is a spell.
                spell_idx = inst_inf->func_params[0];
            }
            else if (inst_inf->func_idx == 3)
            { // Or a shot.
                shotst = get_shot_model_stats(inst_inf->func_params[0]);
                spell_idx = shotst->cast_spell_kind;
            }
            else
            { // If neither, continue checking.
                continue;
            }
            // Check if the associated spell kind can set the spell flags.
            spconf = get_spell_config(spell_idx);
            if (flag_is_set(spconf->spell_flags, spell_flags))
            {
                return i;
            }
        }
    }
    return CrInst_NULL; // If there is no available instance to find.
}

/* Returns a spell kind that is associated to an instance.
 * @param inst_idx The instance to be checked. */
SpellKind get_spell_kind_from_instance(CrInstance inst_idx)
{
    struct InstanceInfo *inst_inf = creature_instance_info_get(inst_idx);
    if (inst_inf->func_idx == 2)
    { // Check if the instance is a spell.
        return inst_inf->func_params[0];
    }
    else if (inst_inf->func_idx == 3)
    { // Or a shot.
        struct ShotConfigStats* shotst = get_shot_model_stats(inst_inf->func_params[0]);
        return shotst->cast_spell_kind;
    }
    return 0; // If there is no spell kind to find.
}

 /* Returns remaining duration of a spell casted on a thing.
 * @param thing The thing which can have spell effect on.
 * @param spkind The spell kind to be checked. */
GameTurnDelta get_spell_duration_left_on_thing_f(const struct Thing *thing, SpellKind spell_idx, const char *func_name)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("%s: Invalid creature control for thing %d", func_name, (int)thing->index);
        return 0;
    }
    struct CastedSpellData *cspell;
    for (int i = 0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        cspell = &cctrl->casted_spells[i];
        if (cspell->spkind == spell_idx)
        {
            return cspell->duration;
        }
    }
    ERRORLOG("%s: No spell of type %d is found to get spell duration left on %s index %d", func_name, (int)spell_idx, thing_model_name(thing), (int)thing->index);
    return 0;
}

long get_free_spell_slot(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CastedSpellData *cspell;
    long i;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    long cval = INT32_MAX;
    long ci = -1;
    for (i=0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        cspell = &cctrl->casted_spells[i];
        // If there's unused slot, return it immediately
        if (cspell->spkind == 0)
        {
            return i;
        }
        // Otherwise, select the one making minimum damage
        long k = abs(cspell->duration);
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
        if (cspell->spkind == 0)
        {
            return i;
        }
    }
    ERRORLOG("Spell effect has been terminated, but still its slot (%ld) isn't empty!",ci);
    return ci;
}

long get_spell_slot(const struct Thing *thing, SpellKind spkind)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    for (long i = 0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        struct CastedSpellData* cspell = &cctrl->casted_spells[i];
        // If there is a slot with required spell
        if (cspell->spkind == spkind)
        {
            return i;
        }
    }
    // If spell not found
    return -1;
}

TbBool fill_spell_slot(struct Thing *thing, SpellKind spell_idx, GameTurnDelta spell_power, CrtrExpLevel spell_level, PlayerNumber plyr_idx, int slot_idx)
{
    if ((slot_idx < 0) || (slot_idx >= CREATURE_MAX_SPELLS_CASTED_AT))
        return false;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return false;
    struct CastedSpellData* cspell = &cctrl->casted_spells[slot_idx];
    cspell->spkind = spell_idx;
    cspell->duration = spell_power;
    cspell->caster_level = spell_level;
    cspell->caster_owner = plyr_idx;
    return true;
}

TbBool free_spell_slot(struct Thing *thing, int slot_idx)
{
    if ((slot_idx < 0) || (slot_idx >= CREATURE_MAX_SPELLS_CASTED_AT))
        return false;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return false;
    struct CastedSpellData* cspell = &cctrl->casted_spells[slot_idx];
    cspell->spkind = 0;
    cspell->duration = 0;
    cspell->caster_level = 0;
    cspell->caster_owner = 0;
    return true;
}

TbBool set_thing_spell_flags_f(struct Thing *thing, SpellKind spell_idx, GameTurnDelta duration, CrtrExpLevel spell_level, const char *func_name)
{
    struct CreatureModelConfig *crconf = creature_stats_get(thing->model);
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    struct SpellConfig *spconf = get_spell_config(spell_idx);
    const struct PowerConfigStats *powerst = get_power_model_stats(spconf->linked_power);
    struct ComponentVector cvect;
    struct Coord3d pos;
    struct Thing *ntng;
    TbBool affected = false;
    // SLOW.
    if (flag_is_set(spconf->spell_flags, CSAfF_Slow)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Slow)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Slow))
        {
            // Re-set the flag if it was cleared before spell termination.
            set_flag(cctrl->spell_flags, CSAfF_Slow);
        }
        cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        affected = true;
    }
    // SPEED.
    if (flag_is_set(spconf->spell_flags, CSAfF_Speed)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Speed)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Speed))
        {
            set_flag(cctrl->spell_flags, CSAfF_Speed);
        }
        cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        affected = true;
    }
    // ARMOUR.
    if (flag_is_set(spconf->spell_flags, CSAfF_Armour)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Armour)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Armour))
        {
            set_flag(cctrl->spell_flags, CSAfF_Armour);
            long num_protect = 0;
            for (int k = 0; k < 2; k++)
            {
                set_coords_to_cylindric_shift(&pos, &thing->mappos, 32, num_protect, k * (thing->clipbox_size_z >> 1));
                ntng = create_object(&pos, ObjMdl_LightBall, thing->owner, -1); // TODO: Make this configurable.
                if (!thing_is_invalid(ntng))
                {
                    cctrl->spell_thing_index_armour[k] = ntng->index;
                    ntng->health = duration + 1;
                    ntng->armor.belongs_to = thing->index;
                    ntng->armor.shspeed = k;
                    ntng->move_angle_xy = thing->move_angle_xy;
                    ntng->move_angle_z = thing->move_angle_z;
                    angles_to_vector(ntng->move_angle_xy, ntng->move_angle_z, 32, &cvect);
                    ntng->veloc_push_add.x.val += cvect.x;
                    ntng->veloc_push_add.y.val += cvect.y;
                    ntng->veloc_push_add.z.val += cvect.z;
                    ntng->state_flags |= TF1_PushAdd;
                }
                num_protect += DEGREES_120;
            }
        }
        affected = true;
    }
    // REBOUND.
    if (flag_is_set(spconf->spell_flags, CSAfF_Rebound)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Rebound)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Rebound))
        {
            set_flag(cctrl->spell_flags, CSAfF_Rebound);
        }
        affected = true;
    }
    // FLYING.
    if (flag_is_set(spconf->spell_flags, CSAfF_Flying)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Flying)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Flying))
        {
            set_flag(cctrl->spell_flags, CSAfF_Flying);
            thing->movement_flags |= TMvF_Flying;
        }
        affected = true;
    }
    // INVISIBILITY.
    if (flag_is_set(spconf->spell_flags, CSAfF_Invisibility)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Invisibility)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Invisibility))
        {
            set_flag(cctrl->spell_flags, CSAfF_Invisibility);
            cctrl->force_visible = 0;
        }
        affected = true;
    }
    // SIGHT.
    if (flag_is_set(spconf->spell_flags, CSAfF_Sight)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Sight)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Sight))
        {
            set_flag(cctrl->spell_flags, CSAfF_Sight);
        }
        affected = true;
    }
    // LIGHT.
    if (flag_is_set(spconf->spell_flags, CSAfF_Light)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Light)))
    {
        if (!crconf->illuminated)
        {
            if (!creature_under_spell_effect(thing, CSAfF_Light))
            {
                    set_flag(cctrl->spell_flags, CSAfF_Light);
                    illuminate_creature(thing);
            }
            affected = true;
        }
    }
    // DISEASE.
    if (flag_is_set(spconf->spell_flags, CSAfF_Disease)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Disease)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Disease))
        {
            set_flag(cctrl->spell_flags, CSAfF_Disease);
            if (cctrl->disease_caster_plyridx == thing->owner)
            {
                cctrl->disease_caster_plyridx = game.neutral_player_num;
            }
            long num_disease = 0;
            cctrl->disease_start_turn = game.play_gameturn;
            for (int j = 0; j < 3; j++)
            {
                pos.x.val = thing->mappos.x.val;
                pos.y.val = thing->mappos.y.val;
                pos.z.val = thing->mappos.z.val;
                pos.x.val += distance_with_angle_to_coord_x(32, num_disease);
                pos.y.val += distance_with_angle_to_coord_y(32, num_disease);
                pos.z.val += j * (long)(thing->clipbox_size_z >> 1);
                ntng = create_object(&pos, ObjMdl_Disease, thing->owner, -1); // TODO: Make this configurable.
                if (!thing_is_invalid(ntng))
                {
                    cctrl->spell_thing_index_disease[j] = ntng->index;
                    ntng->health = duration + 1;
                    ntng->disease.belongs_to = thing->index;
                    ntng->disease.effect_slot = j;
                    ntng->move_angle_xy = thing->move_angle_xy;
                    ntng->move_angle_z = thing->move_angle_z;
                    angles_to_vector(ntng->move_angle_xy, ntng->move_angle_z, 32, &cvect);
                    ntng->veloc_push_add.x.val += cvect.x;
                    ntng->veloc_push_add.y.val += cvect.y;
                    ntng->veloc_push_add.z.val += cvect.z;
                    ntng->state_flags |= TF1_PushAdd;
                }
                num_disease += DEGREES_120;
            }
        }
        cctrl->active_disease_spell = spell_idx; // Remember the spell_idx for a later use.
        affected = true;
    }
    // CHICKEN.
    if (flag_is_set(spconf->spell_flags, CSAfF_Chicken)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Chicken)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Chicken))
        {
            set_flag(cctrl->spell_flags, CSAfF_Chicken);
            external_set_thing_state(thing, CrSt_CreatureChangeToChicken);
            cctrl->countdown = spconf->countdown;
        }
        else // If spell is reapplied countdown is spconf->countdown / 5.
        {
            external_set_thing_state(thing, CrSt_CreatureChangeToChicken);
            cctrl->countdown = spconf->countdown / 5;
        }
        affected = true;
    }
    // FREEZE.
    if (flag_is_set(spconf->spell_flags, CSAfF_Freeze)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Freeze)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Freeze))
        {
            set_flag(cctrl->spell_flags, CSAfF_Freeze);
            set_flag(cctrl->stateblock_flags, CCSpl_Freeze);
            if ((thing->movement_flags & TMvF_Flying) != 0)
            {
                set_flag(thing->movement_flags, TMvF_Grounded);
                clear_flag(thing->movement_flags, TMvF_Flying);
            }
        }
        creature_set_speed(thing, 0);
        affected = true;
    }
    // MAD_KILLING.
    if (flag_is_set(spconf->spell_flags, CSAfF_MadKilling)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_MadKilling)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_MadKilling))
        {
            set_flag(cctrl->spell_flags, CSAfF_MadKilling);
        }
        affected = true;
    }
    // FEAR.
    if (flag_is_set(spconf->spell_flags, CSAfF_Fear)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Fear))
    && (crconf->fear_stronger > 0))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Fear))
        {
            set_flag(cctrl->spell_flags, CSAfF_Fear);
            setup_combat_flee_position(thing);
            if (is_thing_directly_controlled(thing))
            {
                struct PlayerInfo* player = get_player(thing->owner);
                if (player->controlled_thing_idx == thing->index)
                {
                    char active_menu = game.active_panel_mnu_idx;
                    leave_creature_as_controller(player, thing);
                    control_creature_as_passenger(player, thing);
                    if (is_my_player(player))
                    {
                        turn_off_all_panel_menus();
                        turn_on_menu(active_menu);
                    }
                }
            }
            if (external_set_thing_state(thing, CrSt_CreatureCombatFlee))
            {
                cctrl->flee_start_turn = game.play_gameturn;
            }
        }
        else // If spell is reapplied reset flee_start_turn and state.
        {
            cctrl->flee_start_turn = game.play_gameturn;
            if (get_creature_state_besides_interruptions(thing) != CrSt_CreatureCombatFlee)
            {
                external_set_thing_state(thing, CrSt_CreatureCombatFlee);
            }
        }
        affected = true;
    }
    // TELEPORT.
    if (flag_is_set(spconf->spell_flags, CSAfF_Teleport)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Teleport)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Teleport))
        {
            set_flag(cctrl->spell_flags, CSAfF_Teleport);
            set_flag(cctrl->stateblock_flags, CCSpl_Teleport);
        }
        cctrl->active_teleport_spell = spell_idx; // Remember the spell_idx for a later use.
        affected = true;
    }
    // TIMEBOMB.
    if (flag_is_set(spconf->spell_flags, CSAfF_Timebomb)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Timebomb)))
    {
        if (!creature_under_spell_effect(thing, CSAfF_Timebomb))
        {
            set_flag(cctrl->spell_flags, CSAfF_Timebomb);
            cctrl->timebomb_countdown = duration;
        }
        cctrl->active_timebomb_spell = spell_idx; // Remember the spell_idx for a later use.
        affected = true;
    }
    // HEAL.
    if (flag_is_set(spconf->spell_flags, CSAfF_Heal)
    && (!creature_is_immune_to_spell_effect(thing, CSAfF_Heal)))
    {
        // 'CSAfF_Heal' is only for checking config or immunity, flag is never set to creature.
        HitPoints healing_recovery;
        if (spconf->linked_power == PwrK_None)
        {
            healing_recovery = (thing->health + spconf->healing_recovery);
        }
        else
        {
            healing_recovery = (thing->health + powerst->strength[spell_level]);
        }
        if (healing_recovery < 0)
        {
            thing->health = 0;
        }
        else
        {
            thing->health = min(healing_recovery, cctrl->max_health);
        }
        affected = true;
    }
    if (!affected)
    {
        SYNCDBG(7, "%s: No spell flags %d to set on %s index %d", func_name, (uint)spconf->spell_flags, thing_model_name(thing), (int)thing->index);
    }
    return affected;
}

TbBool clear_thing_spell_flags_f(struct Thing *thing, unsigned long spell_flags, const char *func_name)
{
    struct CreatureModelConfig *crconf = creature_stats_get(thing->model);
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    TbBool cleared = false;
    // SLOW.
    if (flag_is_set(spell_flags, CSAfF_Slow)
    && (creature_under_spell_effect(thing, CSAfF_Slow)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Slow);
        cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        cleared = true;
    }
    // SPEED.
    if (flag_is_set(spell_flags, CSAfF_Speed)
    && (creature_under_spell_effect(thing, CSAfF_Speed)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Speed);
        cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        cleared = true;
    }
    // ARMOUR.
    if (flag_is_set(spell_flags, CSAfF_Armour)
    && (creature_under_spell_effect(thing, CSAfF_Armour)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Armour);
        delete_armour_effects_attached_to_creature(thing);
        cleared = true;
    }
    // REBOUND.
    if (flag_is_set(spell_flags, CSAfF_Rebound)
    && (creature_under_spell_effect(thing, CSAfF_Rebound)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Rebound);
        cleared = true;
    }
    // FLYING.
    if (flag_is_set(spell_flags, CSAfF_Flying)
    && (creature_under_spell_effect(thing, CSAfF_Flying)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Flying);
        // TODO: Strange condition regarding the fly, check why it's here?
        if (!flag_is_set(game.conf.crtr_conf.model[thing->model].model_flags, CMF_IsDiptera))
        {
            clear_flag(thing->movement_flags, TMvF_Flying);
        }
        cleared = true;
    }
    // INVISIBILITY.
    if (flag_is_set(spell_flags, CSAfF_Invisibility)
    && (creature_under_spell_effect(thing, CSAfF_Invisibility)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Invisibility);
        cctrl->force_visible = 0;
        cleared = true;
    }
    // SIGHT.
    if (flag_is_set(spell_flags, CSAfF_Sight)
    && (creature_under_spell_effect(thing, CSAfF_Sight)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Sight);
        cleared = true;
    }
    // LIGHT.
    if (flag_is_set(spell_flags, CSAfF_Light)
    && (creature_under_spell_effect(thing, CSAfF_Light)))
    {
        if (!crconf->illuminated)
        {
            clear_flag(cctrl->spell_flags, CSAfF_Light);
            if (thing->light_id != 0)
            {
                if (flag_is_set(thing->rendering_flags, TRF_Invisible))
                {
                    light_set_light_intensity(thing->light_id, (light_get_light_intensity(thing->light_id) - 20));
                    struct Light *lgt = &game.lish.lights[thing->light_id];
                    lgt->radius = 2560;
                }
                else
                {
                    light_delete_light(thing->light_id);
                    thing->light_id = 0;
                }
            }
            cleared = true;
        }
    }
    // DISEASE.
    if (flag_is_set(spell_flags, CSAfF_Disease)
    && (creature_under_spell_effect(thing, CSAfF_Disease)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Disease);
        delete_disease_effects_attached_to_creature(thing);
        cctrl->active_disease_spell = 0;
        cleared = true;
    }
    // CHICKEN.
    if (flag_is_set(spell_flags, CSAfF_Chicken)
    && (creature_under_spell_effect(thing, CSAfF_Chicken)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Chicken);
        external_set_thing_state(thing, CrSt_CreatureChangeFromChicken);
        cctrl->countdown = 10;
        cleared = true;
        set_creature_size_stuff(thing);
    }
    // FREEZE.
    if (flag_is_set(spell_flags, CSAfF_Freeze)
    && (creature_under_spell_effect(thing, CSAfF_Freeze)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Freeze);
        clear_flag(cctrl->stateblock_flags, CCSpl_Freeze);
        if (flag_is_set(thing->movement_flags, TMvF_Grounded))
        {
            set_flag(thing->movement_flags, TMvF_Flying);
            clear_flag(thing->movement_flags, TMvF_Grounded);
        }
        cleared = true;
    }
    // MAD_KILLING.
    if (flag_is_set(spell_flags, CSAfF_MadKilling)
    && (creature_under_spell_effect(thing, CSAfF_MadKilling)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_MadKilling);
        remove_all_traces_of_combat(thing);
        cleared = true;
    }
    // FEAR.
    if (flag_is_set(spell_flags, CSAfF_Fear)
    && (creature_under_spell_effect(thing, CSAfF_Fear)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Fear);
        if (is_thing_passenger_controlled_by_player(thing,thing->owner))
        {
            struct PlayerInfo* player = get_player(thing->owner);
            char active_menu = game.active_panel_mnu_idx;
            leave_creature_as_passenger(player, thing);
            control_creature_as_controller(player, thing);
            if (is_my_player(player))
            {
                turn_off_all_panel_menus();
                turn_on_menu(active_menu);
            }
        }
        cleared = true;
    }
    // TELEPORT.
    if (flag_is_set(spell_flags, CSAfF_Teleport)
    && (creature_under_spell_effect(thing, CSAfF_Teleport)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Teleport);
        clear_flag(cctrl->stateblock_flags, CCSpl_Teleport);
        cctrl->active_teleport_spell = 0;
        cleared = true;
    }
    // TIMEBOMB.
    if (flag_is_set(spell_flags, CSAfF_Timebomb)
    && (creature_under_spell_effect(thing, CSAfF_Timebomb)))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Timebomb);
        thing->veloc_push_add.x.val = 0;
        thing->veloc_push_add.y.val = 0;
        clear_flag(thing->state_flags, TF1_PushAdd);
        cleanup_current_thing_state(thing);
        set_start_state(thing);
        cctrl->active_timebomb_spell = 0;
        cleared = true;
    }
    // HEAL.
    if (flag_is_set(spell_flags, CSAfF_Heal))
    {
        // 'CSAfF_Heal' is never set but we still want to mark it cleared to free the spell slot.
        cleared = true;
    }
    if (!cleared)
    {
        SYNCDBG(7, "%s: No spell flags %d to clear on %s index %d", func_name, (uint)spell_flags, thing_model_name(thing), (int)thing->index);
    }
    return cleared;
}

GameTurnDelta get_spell_full_duration(SpellKind spell_idx, CrtrExpLevel spell_level)
{
    // If not linked to a keeper power, use the duration set on the spell, otherwise use the strength or duration of the linked power.
    struct SpellConfig *spconf = get_spell_config(spell_idx);
    const struct PowerConfigStats *powerst = get_power_model_stats(spconf->linked_power);
    GameTurnDelta duration;
    if (spconf->linked_power == 0)
    {
        duration = spconf->duration;
    }
    else if (powerst->duration == 0)
    {
        duration = powerst->strength[spell_level];
    }
    else
    {
        duration = powerst->duration;
    }
    return duration;
}

TbBool spell_is_continuous(SpellKind spell_idx, GameTurnDelta duration)
{
    if (duration > 0)
    {
        struct SpellConfig *spconf = get_spell_config(spell_idx);
        if ((spconf->damage != 0 && spconf->damage_frequency > 0)
        || (spconf->aura_effect != 0 && spconf->aura_duration > 0 && spconf->aura_frequency > 0))
        {
            return true;
        }
        return false;
    }
    return false;
}

void update_aura_effect_to_thing(struct Thing *thing, SpellKind spell_idx)
{
    struct SpellConfig *spconf = get_spell_config(spell_idx);
    if ((spconf->aura_effect != 0) && (spconf->aura_duration > 0))
    {
        struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
        cctrl->spell_aura = spconf->aura_effect;
        cctrl->spell_aura_duration = spconf->aura_duration;
    }
}

void first_apply_spell_effect_to_thing(struct Thing *thing, SpellKind spell_idx, CrtrExpLevel spell_level, PlayerNumber plyr_idx)
{
    if (spell_level > SPELL_MAX_LEVEL)
    {
        spell_level = SPELL_MAX_LEVEL;
    }
    GameTurnDelta duration = get_spell_full_duration(spell_idx, spell_level);
    long i = get_free_spell_slot(thing);
    if (i != -1)
    {
        // Fill the spell slot if the spell has a continuous effect.
        if (set_thing_spell_flags(thing, spell_idx, duration, spell_level)
        || spell_is_continuous(spell_idx, duration))
        {
            fill_spell_slot(thing, spell_idx, duration, spell_level, plyr_idx, i);
            update_aura_effect_to_thing(thing, spell_idx);
        }
    }
    return;
}

void reapply_spell_effect_to_thing(struct Thing *thing, SpellKind spell_idx, CrtrExpLevel spell_level, PlayerNumber plyr_idx, int slot_idx)
{
    if (spell_level > SPELL_MAX_LEVEL)
    {
        spell_level = SPELL_MAX_LEVEL;
    }
    GameTurnDelta duration = get_spell_full_duration(spell_idx, spell_level);
    // Reset the spell duration if the spell has a continuous effect.
    if (set_thing_spell_flags(thing, spell_idx, duration, spell_level)
    || spell_is_continuous(spell_idx, duration))
    {
        struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
        struct CastedSpellData *cspell = &cctrl->casted_spells[slot_idx];
        cspell->duration = duration;
        cspell->caster_level = spell_level;
        cspell->caster_owner = plyr_idx;
        update_aura_effect_to_thing(thing, spell_idx);
    }
    return;
}

void apply_spell_effect_to_thing(struct Thing *thing, SpellKind spell_idx, CrtrExpLevel spell_level, PlayerNumber plyr_idx)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature %s index %d tried to accept spell %s", thing_model_name(thing), (int)thing->index, spell_code_name(spell_idx));
        return; // Exit the function, control is invalid.
    }
    struct SpellConfig *spconf = get_spell_config(spell_idx);
    if (spell_config_is_invalid(spconf))
    {
        ERRORLOG("Spell %s config is invalid", spell_code_name(spell_idx));
        return; // Exit the function, spell config is invalid.
    }
    // TODO: Add a check for specific spell_idx immunity that checks if creature is immune to a specific spell.
    /*
    if (implement_new_function_to_check_for_specific_spell_immunity(thing, spell_idx))
    {
        return; // Exit the function, creature is immune to spell_idx.
    }
    */
    // Make sure the creature level isn't larger than max spell level.
    if (spell_level > SPELL_MAX_LEVEL)
    {
        spell_level = SPELL_MAX_LEVEL;
    }
    GameTurnDelta duration = get_spell_full_duration(spell_idx, spell_level);
    // Check for cleansing one-time effect.
    if (spconf->cleanse_flags > 0
    && any_flag_is_set(spconf->cleanse_flags, cctrl->spell_flags))
    {
        clean_spell_effect(thing, spconf->cleanse_flags);
        if (spconf->spell_flags == 0
        && !spell_is_continuous(spell_idx, duration))
        {
            update_aura_effect_to_thing(thing, spell_idx);
            return; // Exit the function, no continuous effect to apply.
        }
    }
    // Check for damage/heal one-time effect.
    if ((spconf->damage != 0) && (spconf->damage_frequency == 0))
    {
        process_thing_spell_damage_or_heal_effects(thing, spell_idx, spell_level, plyr_idx);
        if (spconf->spell_flags == 0
        && !spell_is_continuous(spell_idx, duration))
        {
            update_aura_effect_to_thing(thing, spell_idx);
            return; // Exit the function, no continuous effect to apply.
        }
    }
    // Check for immunities against each spell flags set on spell_idx.
    if (((spconf->spell_flags > 0) && creature_is_immune_to_spell_effect(thing, spconf->spell_flags))
    && !spell_is_continuous(spell_idx, duration))
    {
        SYNCDBG(7, "Creature %s index %d is immune to each spell flags %d set on %s", thing_model_name(thing), (int)thing->index, (uint)spconf->spell_flags, spell_code_name(spell_idx));
        return; // Exit the function, creature is immune to each spell flags set on spell_idx and there are no other continuous effects.
    }
    // Lastly, check if spell is not continuous.
    if ((spconf->spell_flags == 0) && (!spell_is_continuous(spell_idx, duration)))
    {
        update_aura_effect_to_thing(thing, spell_idx);
        return; // Exit the function, no further processing is required.
    }
    SYNCDBG(6, "Applying %s to %s index %d", spell_code_name(spell_idx), thing_model_name(thing), (int)thing->index);
    for (int i = 0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        if (cctrl->casted_spells[i].spkind == spell_idx)
        {
            reapply_spell_effect_to_thing(thing, spell_idx, spell_level, plyr_idx, i);
            return; // Exit the function, spell is already active.
        }
    }
    first_apply_spell_effect_to_thing(thing, spell_idx, spell_level, plyr_idx);
}

void terminate_thing_spell_effect(struct Thing *thing, SpellKind spell_idx)
{
    TRACE_THING(thing);
    struct SpellConfig *spconf = get_spell_config(spell_idx);
    clear_thing_spell_flags(thing, spconf->spell_flags);
    int slot_idx = get_spell_slot(thing, spell_idx);
    if (slot_idx >= 0)
    {
        free_spell_slot(thing, slot_idx);
    }
    return;
}

void terminate_all_actives_damage_over_time_spell_effects(struct Thing *thing)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    struct CastedSpellData *cspell;
    struct SpellConfig *spconf;
    for (int i = 0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        cspell = &cctrl->casted_spells[i];
        spconf = get_spell_config(cspell->spkind);
        if (spconf->damage != 0)
        {
            terminate_thing_spell_effect(thing, cspell->spkind);
        }
    }
}

/* Clears spell effect on a thing.
 * It first checks for an active spell match and terminates the associated spell.
 * If no exact match is found, it clears only the flag without affecting others.
 * This ensures that spells with multiple flags remain intact.
 * This is used to stop a spell effect before its duration ends, like Temple cures.
 * @param thing The thing which can have spell effect on.
 * @param spell_flags The spell flags to be cleaned. */
void clean_spell_effect_f(struct Thing *thing, unsigned long spell_flags, const char *func_name)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("%s: Invalid creature control for thing %d", func_name, (int)thing->index);
        return;
    }
    struct CastedSpellData *cspell;
    struct SpellConfig *spconf;
    // First check for an exact match with the active spells.
    for (int i = 0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        cspell = &cctrl->casted_spells[i];
        spconf = get_spell_config(cspell->spkind);
        if (spconf->spell_flags == spell_flags)
        {
            terminate_thing_spell_effect(thing, cspell->spkind);
            return;
        }
    }
    // If no exact match is found, then check for each spell flags separately without terminating a spell.
    if (!clear_thing_spell_flags(thing, spell_flags))
    {
        // Shouldn't happen within this function but if it does then log it.
        ERRORLOG("%s: No spell flags %d to clear on %s index %d", func_name, (uint)spell_flags, thing_model_name(thing), (int)thing->index);
    }
    return;
}

void process_thing_spell_teleport_effects(struct Thing *thing, struct CastedSpellData *cspell)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct SpellConfig* spconf = get_spell_config(cspell->spkind);
    struct Room* room = NULL;
    const struct Thing* desttng = NULL;
    int32_t distance = INT32_MAX;
    struct Dungeon *dungeon = get_players_num_dungeon(thing->owner);
    RoomKind rkind = 0;
    long i;
    TbBool allowed = true;
    clear_messages_from_player(MsgType_CreatureInstance, CrInst_TELEPORT);
    if (cspell->duration == spconf->duration / 2)
    {
        PlayerNumber plyr_idx = get_appropriate_player_for_creature(thing);
        struct PlayerInfo* player = get_player(plyr_idx);
        struct Coord3d pos;
        pos.x.val = subtile_coord_center(cctrl->teleport_x);
        pos.y.val = subtile_coord_center(cctrl->teleport_y);
        pos.z.val = get_floor_height_at(&pos);
        if (thing_in_wall_at(thing, &pos))
        {
            if (creature_is_dragging_something(thing))
            {
                struct Thing *droptng = thing_get(cctrl->dragtng_idx);
                if (droptng->class_id == TCls_Creature)
                {
                    stop_creature_being_dragged_by(droptng, thing);
                }
                else
                {
                    creature_drop_dragged_object(thing, droptng);
                }
            }
            const struct Coord3d* newpos = NULL;
            struct Coord3d room_pos;
            switch(player->teleport_destination)
            {
                case 6: // Dungeon Heart
                {
                    newpos = dungeon_get_essential_pos(thing->owner);
                    break;
                }
                case 16: // Fight
                {
                    if (active_battle_exists(thing->owner))
                    {
                        long count = 0;
                        if (player->battleid > BATTLES_COUNT)
                        {
                            player->battleid = 1;
                        }
                        for (i = player->battleid; i <= BATTLES_COUNT; i++)
                        {
                            if (i > BATTLES_COUNT)
                            {
                                i = 1;
                                player->battleid = 1;
                            }
                            count++;
                            struct CreatureBattle* battle = creature_battle_get(i);
                            if ( (battle->fighters_num != 0) && (battle_with_creature_of_player(thing->owner, i)) )
                            {
                                struct Thing* tng = thing_get(battle->first_creatr);
                                TRACE_THING(tng);
                                if (creature_can_navigate_to(thing, &tng->mappos, NavRtF_NoOwner))
                                {
                                    pos.x.val = tng->mappos.x.val;
                                    pos.y.val = tng->mappos.y.val;
                                    player->battleid = i + 1;
                                    break;
                                }
                            }
                            if (count >= BATTLES_COUNT)
                            {
                                player->battleid = 1;
                                break;
                            }
                            if (i >= BATTLES_COUNT)
                            {
                                i = 0;
                                player->battleid = 1;
                                continue;
                            }
                        }
                    }
                    else
                    {
                        allowed = false;
                    }
                    break;
                }
                case 17: // Last work room
                {
                    room = room_get(cctrl->last_work_room_id);
                    break;
                }
                case 18: // Call to Arms
                {
                    struct Coord3d cta_pos;
                    cta_pos.x.val = subtile_coord_center(dungeon->cta_stl_x);
                    cta_pos.y.val = subtile_coord_center(dungeon->cta_stl_y);
                    cta_pos.z.val = subtile_coord(1,0);
                    if (creature_can_navigate_to(thing, &cta_pos, NavRtF_NoOwner))
                    {
                        pos = cta_pos;
                    }
                    else
                    {
                        allowed = false;
                    }
                    break;
                }
                case 19: // Lair
                {
                    desttng = thing_get(cctrl->lairtng_idx);
                    break;
                }
                default:
                {
                    rkind = zoom_key_room_order[player->teleport_destination];
                }
            }
            if (rkind > 0)
            {
                long count = 0;
                if (player->nearest_teleport)
                {
                    room = find_room_nearest_to_position(thing->owner, rkind, &thing->mappos, &distance);
                }
                else
                {
                    do
                    {
                        if (count >= count_player_rooms_of_type(thing->owner, rkind))
                        {
                            break;
                        }
                        room = room_get(find_next_room_of_type(thing->owner, rkind));
                        find_first_valid_position_for_thing_anywhere_in_room(thing, room, &room_pos);
                        count++;
                    }
                    while (!creature_can_navigate_to(thing, &room_pos, NavRtF_NoOwner));
                }
            }
            if (!room_is_invalid(room))
            {
                room_pos.x.val = subtile_coord_center(room->central_stl_x);
                room_pos.y.val = subtile_coord_center(room->central_stl_y);
                allowed = creature_can_navigate_to(thing, &room_pos, NavRtF_NoOwner);
                if (!allowed)
                {
                    if (find_random_valid_position_for_thing_in_room(thing, room, &room_pos))
                    {
                        allowed = (creature_can_navigate_to(thing, &room_pos, NavRtF_NoOwner) || rkind == RoK_DUNGHEART);
                    }
                }
            }
            if (!allowed)
            {
                desttng = thing_get(cctrl->lairtng_idx);
                if (thing_is_object(desttng))
                {
                    newpos = &desttng->mappos;
                }
                else
                {
                    newpos = dungeon_get_essential_pos(thing->owner);
                }
                pos.x.val = newpos->x.val;
                pos.y.val = newpos->y.val;
                pos.z.val = newpos->z.val;
            }
            else
            {
                if ( (pos.x.val == subtile_coord_center(cctrl->teleport_x)) && (pos.y.val == subtile_coord_center(cctrl->teleport_y)) )
                {
                    if (thing_is_object(desttng))
                    {
                        newpos = &desttng->mappos;
                    }
                    if (newpos != NULL)
                    {
                        pos.x.val = newpos->x.val;
                        pos.y.val = newpos->y.val;
                        pos.z.val = newpos->z.val;
                    }
                    else if (!room_is_invalid(room))
                    {
                        pos = room_pos;
                    }
                    else if ( (room_is_invalid(room)) && (newpos == NULL) )
                    {
                        newpos = dungeon_get_essential_pos(thing->owner);
                        pos.x.val = newpos->x.val;
                        pos.y.val = newpos->y.val;
                        pos.z.val = newpos->z.val;
                    }
                }
            }

        }
        pos.z.val += subtile_coord(2,0);
        move_thing_in_map(thing, &pos);
        remove_all_traces_of_combat(thing);
        reset_interpolation_of_thing(thing);
        ariadne_invalidate_creature_route(thing);
        check_map_explored(thing, pos.x.stl.num, pos.y.stl.num);
        if (!flag_is_set(thing->movement_flags, TMvF_Flying))
        {
            thing->veloc_push_add.x.val += THING_RANDOM(thing, 193) - 96;
            thing->veloc_push_add.y.val += THING_RANDOM(thing, 193) - 96;
            thing->veloc_push_add.z.val += THING_RANDOM(thing, 96) + 40;
            set_flag(thing->state_flags, TF1_PushAdd);
        }
        player->teleport_destination = 19;
    }
}

void process_thing_spell_damage_or_heal_effects(struct Thing *thing, SpellKind spell_idx, CrtrExpLevel caster_level, PlayerNumber caster_owner)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    struct SpellConfig *spconf = get_spell_config(spell_idx);
    HitPoints damage;
    // If percent based, update the damage value.
    if (flag_is_set(spconf->properties_flags, SPF_PercentBased)
    || flag_is_set(spconf->properties_flags, SPF_MaxHealth))
    {
        // Percent based on max health.
        if (flag_is_set(spconf->properties_flags, SPF_MaxHealth))
        {
            damage = cctrl->max_health * spconf->damage / 100;
            if (damage > cctrl->max_health)
            {
                damage = cctrl->max_health;
            }
        }
        else // Percent based on current health.
        {
            damage = thing->health * spconf->damage / 100;
            if (damage > thing->health)
            {
                damage = thing->health;
            }
        }
    }
    // Or if it's fixed damage.
    else if (flag_is_set(spconf->properties_flags, SPF_FixedDamage))
    {
        damage = compute_creature_spell_damage_over_time(spconf->damage, 0, caster_owner);
    }
    else // Else computes normally.
    {
        damage = compute_creature_spell_damage_over_time(spconf->damage, caster_level, caster_owner);
    }
    // Apply damage.
    if (damage >= 0)
    {
        apply_damage_to_thing_and_display_health(thing, damage, caster_owner);
    }
    else // Or heal if damage is negative.
    {
        apply_health_to_thing_and_display_health(thing, -damage);
    }
}

void process_thing_spell_effects(struct Thing *thing)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    for (int i = 0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        struct CastedSpellData *cspell = &cctrl->casted_spells[i];
        if (cspell->spkind == 0)
        {
            continue;
        }
        struct SpellConfig *spconf = get_spell_config(cspell->spkind);
        // Terminate the spell if its duration expires, or if the spell flags are cleared and no other continuous effects are active.
        if ((cspell->duration <= 0)
            || ((spconf->spell_flags > 0) && !flag_is_set(cctrl->spell_flags, spconf->spell_flags) && !spell_is_continuous(cspell->spkind, cspell->duration)))
        {
            terminate_thing_spell_effect(thing, cspell->spkind);
            continue;
        }
        else if (spconf->aura_frequency > 0)
        {
            if (cspell->duration % spconf->aura_frequency == 0)
            {
                // Reapply aura effect if possible.
                update_aura_effect_to_thing(thing, cspell->spkind);
            }
        }
        // Process spell with damage (or heal) over time.
        if ((spconf->damage != 0) && (spconf->damage_frequency > 0))
        {
            if (cspell->duration % spconf->damage_frequency == 0)
            {
                process_thing_spell_damage_or_heal_effects(thing, cspell->spkind, cspell->caster_level, cspell->caster_owner);
            }
        }
        // Process spell with teleport flag.
        if (cspell->spkind == cctrl->active_teleport_spell)
        {
            process_thing_spell_teleport_effects(thing, cspell);
        }
        /* Process spell with cleansing & CSAfF_SpellBlocks.
        if (flag_is_set(spconf->spell_flags, CSAfF_SpellBlocks)
        && any_flag_is_set(spconf->cleanse_flags, cctrl->spell_flags))
        {
            clean_spell_effect(thing, spconf->cleanse_flags);
        } TODO: Implements CSAfF_SpellBlocks. */
        cspell->duration--;
    }
    // Slap is not in spell array, it is so common that has its own dedicated duration.
    if (cctrl->slap_turns > 0)
    {
        cctrl->slap_turns--;
        if (cctrl->slap_turns <= 0)
        {
            cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        }
    }
}

void process_thing_spell_effects_while_blocked(struct Thing *thing)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    for (int i = 0; i < CREATURE_MAX_SPELLS_CASTED_AT; i++)
    {
        struct CastedSpellData *cspell = &cctrl->casted_spells[i];
        if (cspell->spkind == 0)
        {
            continue;
        }
        if (cspell->duration > 0)
        {
            cspell->duration--;
        }
    }
    // Slap is not in spell array, it is so common that has its own dedicated duration.
    if (cctrl->slap_turns > 0)
    {
        cctrl->slap_turns--;
        if (cctrl->slap_turns <= 0)
        {
            cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        }
    }
}

/**
 * Casts a spell by caster creature targeted at given thing, most likely using shot to transfer the spell.
 * @param castng The caster creature.
 * @param targetng The target thing.
 * @param spl_idx Spell index to be casted.
 * @param shot_level Spell level to be casted.
 */
void creature_cast_spell_at_thing(struct Thing *castng, struct Thing *targetng, SpellKind spl_idx, CrtrExpLevel shot_level)
{
    unsigned char hit_type;
    if ((castng->alloc_flags & TAlF_IsControlled) != 0)
    {
        if ((targetng->class_id == TCls_Object) || (targetng->class_id == TCls_Trap))
            hit_type = THit_CrtrsNObjcts;
        else
            hit_type = THit_CrtrsOnly;
    } else
    {
        if ((targetng->class_id == TCls_Object) || (targetng->class_id == TCls_Trap))
            hit_type = THit_CrtrsNObjctsNotOwn;
        else
        if (targetng->owner == castng->owner)
            hit_type = THit_CrtrsOnlyOwn;
        else
            hit_type = THit_CrtrsOnlyNotOwn;
    }
    struct SpellConfig* spconf = get_spell_config(spl_idx);
    if (spell_config_is_invalid(spconf))
    {
        ERRORLOG("The %s owned by player %d tried to cast invalid spell %d",thing_model_name(castng),(int)castng->owner,(int)spl_idx);
        return;
    }

    SYNCDBG(12,"The %s(%u) fire shot(%s) at %s(%u) with shot level %d, hit type: 0x%02X", thing_model_name(castng), castng->index,
        shot_code_name(spconf->shot_model), thing_model_name(targetng), targetng->index, (int)shot_level, hit_type);
    thing_fire_shot(castng, targetng, spconf->shot_model, shot_level, hit_type);
}

void teleport_familiar_to_summoner(struct Thing *famlrtng, struct Thing* creatng)
{
    create_effect(&famlrtng->mappos, imp_spangle_effects[get_player_color_idx(famlrtng->owner)], famlrtng->owner);
    move_thing_in_map(famlrtng, &creatng->mappos);
    cleanup_current_thing_state(famlrtng);
    reset_interpolation_of_thing(famlrtng);

    famlrtng->veloc_push_add.x.val += THING_RANDOM(famlrtng, 161) - 80;
    famlrtng->veloc_push_add.y.val += THING_RANDOM(famlrtng, 161) - 80;
    famlrtng->veloc_push_add.z.val += 0;
    set_flag(famlrtng->state_flags, TF1_PushAdd);
    set_flag(famlrtng->movement_flags, TMvF_MagicFall);
    famlrtng->move_angle_xy = ANGLE_NORTH;
}

/**
 * Spell creates creatures to help the caster. When the creatures have a limited duration they will group up with caster.
 * @param model The creature kind to be creates.
 * @param level The creature experience level.
 * @param count How many creatures are created.
 * @param duration How many gameturns the creatures will live. Set to 0 for infinite.
 */
void thing_summon_temporary_creature(struct Thing* creatng, ThingModel model, char level, char count, GameTurn duration, long spl_idx)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* famlrtng;
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    struct CreatureControl* famcctrl;
    short sumxp = level - 1;
    if (level <= 0)
    {
        sumxp = cctrl->exp_level + level;
    }
    short sumcount = count;
    if (count <= 0)
    {
        sumcount = cctrl->exp_level+1 + count;
    }
    if (duration == 0)
    {
        for (int j = 0; j < sumcount; j++)
        {
            famlrtng = activate_trap_spawn_creature(creatng, model);
            if (!thing_is_invalid(famlrtng))
            {
                creature_change_multiple_levels(famlrtng, sumxp);
            }
        }
    }
    else
    {
        for (int j = 0; j < sumcount; j++)
        {
            if (j >= FAMILIAR_MAX)
            {
                WARNLOG("Trying to summon creature beyond max %d", FAMILIAR_MAX);
                break;
            }
            if (cctrl->familiar_idx[j] == 0)
            {
                famlrtng = activate_trap_spawn_creature(creatng, model);
                if (!thing_is_invalid(famlrtng))
                {
                    cctrl->familiar_idx[j] = famlrtng->index;
                    famcctrl = creature_control_get_from_thing(famlrtng);
                    //create list for summons for all dungeons
                    add_creature_to_summon_list(dungeon, famlrtng->index);
                    //remember your Summoner
                    famcctrl->summoner_idx = creatng->index;
                    //remember the spell that created you
                    famcctrl->summon_spl_idx = spl_idx;
                    creature_change_multiple_levels(famlrtng, sumxp);
                    remove_first_creature(famlrtng); //temporary units are not real creatures
                    famcctrl->unsummon_turn = game.play_gameturn + duration;
                    set_flag(famcctrl->creature_state_flags, TF2_SummonedCreature);
                    struct Thing* leadtng = get_group_leader(creatng);
                    if (leadtng == creatng)
                    {
                        if (get_no_creatures_in_group(creatng) < GROUP_MEMBERS_COUNT)
                        {
                            add_creature_to_group(famlrtng, creatng);
                        }
                    }
                    else
                    {
                        if (get_no_creatures_in_group(creatng) == 0) //Only make the caster a party leader if he is not already a member of another party
                        {
                            add_creature_to_group_as_leader(creatng, famlrtng);
                        }
                        if (get_no_creatures_in_group(creatng) < GROUP_MEMBERS_COUNT)
                        {
                            add_creature_to_group(famlrtng, creatng);
                        }
                    }
                }
                else
                {
                    cctrl->familiar_idx[j] = 0;
                }
            }
            else
            {
                //reset the creature duration
                famlrtng = thing_get(cctrl->familiar_idx[j]);
                if (thing_is_creature(famlrtng))
                {
                    if (famlrtng->model == model)
                    {
                        famcctrl = creature_control_get_from_thing(famlrtng);
                        famcctrl->unsummon_turn = game.play_gameturn + duration;
                        level_up_familiar(famlrtng);
                        if ((famcctrl->follow_leader_fails > 0) || (get_chessboard_distance(&creatng->mappos, &famlrtng->mappos) > subtile_coord(12, 0))) //if it's not getting to the summoner, teleport it there
                        {
                            teleport_familiar_to_summoner(famlrtng, creatng);
                        }
                    }
                    else
                    {
                        // there's multiple summon types on this creature.
                        sumcount++;
                    }
                }
                else
                {
                    //creature has already died, clear it and go again.
                    remove_creature_from_summon_list(dungeon, famlrtng->index);
                    cctrl->familiar_idx[j] = 0;
                    j--;
                }
            }
        }
    }
}

void level_up_familiar(struct Thing* famlrtng)
{
    struct CreatureControl *famlrcctrl = creature_control_get_from_thing(famlrtng);
    //get summoner of familiar
    struct Thing* summonertng = thing_get(famlrcctrl->summoner_idx);
    struct CreatureControl *summonercctrl = creature_control_get_from_thing(summonertng);
    short summonerxp = summonercctrl->exp_level;
    //get spell the summoner used to make this familiar
    struct SpellConfig* spconf = get_spell_config(famlrcctrl->summon_spl_idx);
    char level = spconf->crtr_summon_level;
    //calculate correct level for familiar
    short sumxp = level - 1;
    if (level <= 0)
    {
        //we know already the Summoner will levelup next turn?
        if ((summonercctrl->exp_level_up) && (summonercctrl->exp_level + 1 < CREATURE_MAX_LEVEL))
        {
            summonerxp += 1;
        }
        sumxp = summonerxp + level;
    }
    //level up the summon
    char expdiff = sumxp - famlrcctrl->exp_level;
    if (expdiff > 0)
    {
        creature_change_multiple_levels(famlrtng, expdiff);
    }
}

void add_creature_to_summon_list(struct Dungeon* dungeon, ThingIndex famlrtng)
{
    if (dungeon->num_summon < MAX_SUMMONS)
    {
        dungeon->summon_list[dungeon->num_summon] = famlrtng;
        dungeon->num_summon++;
    } else
    {
        ERRORLOG("Reached maximum limit of summons");
    }
}

void remove_creature_from_summon_list(struct Dungeon* dungeon, ThingIndex famlrtng)
{
    if (dungeon->num_summon == 0) {
        ERRORLOG("No summons to remove");
        return;
    }
    for (int i = 0; i < dungeon->num_summon;i++){
        if (dungeon->summon_list[i] == famlrtng) {
            // Shift the rest of the list one position forward
            for (int j = i; j < dungeon->num_summon -1; j++) {
                dungeon->summon_list[j] = dungeon->summon_list[j + 1];
            }
            dungeon->summon_list[dungeon->num_summon - 1] = 0;
            dungeon->num_summon--;
            return;
        }
    }
}
/**
 * @brief Casts a spell by caster creature targeted at given coordinates, most likely using shot to transfer the spell.
 *
 * @param castng The caster creature.
 * @param spl_idx Spell index to be casted.
 * @param shot_level Spell level to be casted.
 * @param trg_x
 * @param trg_y
 */
void creature_cast_spell(struct Thing *castng, SpellKind spl_idx, CrtrExpLevel shot_level, MapSubtlCoord trg_x, MapSubtlCoord trg_y)
{
    long i;
    struct SpellConfig* spconf = get_spell_config(spl_idx);
    struct CreatureControl* cctrl = creature_control_get_from_thing(castng);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature tried to cast spell %d",(int)spl_idx);
        return;
    }
    if (flag_is_set(spconf->spell_flags, CSAfF_Teleport))
    {
        cctrl->teleport_x = trg_x;
        cctrl->teleport_y = trg_y;
    }
    // Check if the spell can be fired as a shot. It is definitely not if casted on itself.
    if ((spconf->shot_model > 0) && (cctrl->targtng_idx != 0) && (cctrl->targtng_idx != castng->index))
    {
        if ((castng->alloc_flags & TAlF_IsControlled) != 0)
          i = THit_CrtrsNObjcts;
        else
        {
            if (castng->owner == thing_get(cctrl->targtng_idx)->owner)
            {
                i = THit_CrtrsOnlyOwn;
            }
            else
            {
                i = THit_CrtrsOnlyNotOwn;
            }
        }
        thing_fire_shot(castng, INVALID_THING, spconf->shot_model, shot_level, i);
    }
    // Check if the spell can be self-casted
    else if (spconf->caster_affected)
    {
        if (spconf->caster_affect_sound > 0)
        {
            thing_play_sample(castng, spconf->caster_affect_sound + SOUND_RANDOM(spconf->caster_sounds_count), NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
        }
        apply_spell_effect_to_thing(castng, spl_idx, cctrl->exp_level, castng->owner);
    }
    else if (spconf->shot_model > 0)
    {
        // Note that Wind has shot model and its CastAtThing is 0, besides, the target index is itself.
        if ((castng->alloc_flags & TAlF_IsControlled) != 0)
            i = THit_CrtrsNObjcts;
        else
            i = THit_CrtrsOnlyNotOwn;

        const struct InstanceInfo* inst_inf = creature_instance_info_get(cctrl->instance_id);
        if (flag_is_set(inst_inf->instance_property_flags, InstPF_RangedBuff))
        {
            ERRORLOG("The %s(%d) tried to fire Ranged Buff's shot(%s) without a target!",
                thing_model_name(castng), castng->index, shot_code_name(spconf->shot_model));
        }
        else
        {
            thing_fire_shot(castng, INVALID_THING, spconf->shot_model, shot_level, i);
        }
    }

    if (spconf->crtr_summon_model > 0)
    {
        thing_summon_temporary_creature(castng, spconf->crtr_summon_model, spconf->crtr_summon_level, spconf->crtr_summon_amount, spconf->duration, spl_idx);
    }
    struct Thing *effect = create_used_effect_or_element(&castng->mappos, spconf->cast_effect_model, castng->owner, castng->index);
    if (!thing_is_invalid(effect))
    {
        effect->parent_idx = castng->index;
    }
}

void update_creature_count(struct Thing *creatng)
{
    TRACE_THING(creatng);
    struct CreatureControl* cctrl;
    if (!thing_exists(creatng)) {
        return;
    }
    if (is_hero_thing(creatng) || is_neutral_thing(creatng)) {
        return;
    }
    if (thing_is_picked_up(creatng) || creature_is_being_unconscious(creatng)) {
        return;
    }
    cctrl = creature_control_get_from_thing(creatng);
    if (flag_is_set(cctrl->creature_state_flags, TF2_SummonedCreature)){
        return;
    }
    struct Dungeon* dungeon = get_players_num_dungeon(creatng->owner);
    if (dungeon_invalid(dungeon)) {
        return;
    }
    int statyp = get_creature_state_type(creatng);
    dungeon->crmodel_state_type_count[creatng->model][statyp]++;
    int job_idx = get_creature_gui_job(creatng);
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
        if (thing->class_id == TCls_Object)
        {
            if ((thing->model == ObjMdl_Goldl) && thing_touching_floor(thing))
                return thing;
            if (object_is_mature_food(thing))
            {
                struct Room* room = get_room_thing_is_on(thing);
                if (room_is_invalid(room))
                    return thing;
                if (!room_role_matches(room->kind, RoRoF_FoodStorage) && !room_role_matches(room->kind, RoRoF_Torture) && !room_role_matches(room->kind, RoRoF_Prison))
                    return thing;
            }
        }
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

struct Thing *find_interesting_object_laying_around_thing(struct Thing *creatng)
{
    for (long k = 0; k < AROUND_TILES_COUNT; k++)
    {
        long stl_x = creatng->mappos.x.stl.num + around[k].delta_x;
        long stl_y = creatng->mappos.y.stl.num + around[k].delta_y;
        struct Map* mapblk = get_map_block_at(stl_x, stl_y);
        if (!map_block_invalid(mapblk))
        {
            if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
            {
                struct Thing* thing = find_gold_pile_or_chicken_laying_on_mapblk(mapblk);
                if (!thing_is_invalid(thing))
                    return thing;
            }
        }
    }
    return INVALID_THING;
}

TbBool thing_can_be_eaten(struct Thing *thing)
{
    if (thing_is_mature_food(thing)
    || (thing_is_creature(thing) && creature_under_spell_effect(thing, CSAfF_Chicken)))
    {
        if (is_thing_directly_controlled(thing) || is_thing_passenger_controlled(thing) || thing_is_picked_up(thing))
        {
            return false;
        }
        return true;
    }
    return false;
}

TbBool creature_pick_up_interesting_object_laying_nearby(struct Thing *creatng)
{
    struct Thing* tgthing = find_interesting_object_laying_around_thing(creatng);
    if (thing_is_invalid(tgthing)) {
        return false;
    }
    if (object_is_gold_laying_on_ground(tgthing))
    {
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
        if (tgthing->valuable.gold_stored > 0)
        {
            if (creatng->creature.gold_carried < crconf->gold_hold)
            {
                if (crconf->gold_hold < tgthing->valuable.gold_stored + creatng->creature.gold_carried)
                {
                    long k = crconf->gold_hold - creatng->creature.gold_carried;
                    creatng->creature.gold_carried += k;
                    tgthing->valuable.gold_stored -= k;
                } else
                {
                    creatng->creature.gold_carried += tgthing->valuable.gold_stored;
                    delete_thing_structure(tgthing, 0);
                }
                thing_play_sample(creatng, 32, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
            }
        } else
        {
            ERRORLOG("GoldPile with no gold!");
            delete_thing_structure(tgthing, 0);
        }
        anger_apply_anger_to_creature(creatng, crconf->annoy_got_wage, AngR_NotPaid, 1);
        return true;
    }
    if (thing_can_be_eaten(tgthing) && creature_able_to_eat(creatng))
    {
        food_eaten_by_creature(tgthing, creatng);
        return true;
    }
    return false;
}

void creature_look_for_hidden_doors(struct Thing *creatng)
{
    const struct StructureList *slist = get_list_for_thing_class(TCls_Door);
    long i = slist->index;
    while (i > 0)
    {
        struct Thing *doortng = thing_get(i);
        if (thing_is_invalid(doortng))
        {
            break;
        }
        if (door_is_hidden_to_player(doortng, creatng->owner))
        {
            MapSubtlCoord z = doortng->mappos.z.stl.num;
            doortng->mappos.z.stl.num = 2;
            // TODO: Could add a creature property 'DETECT_MECHANISM' allowing them to see secret door but not invisible creatures.
            if (creature_under_spell_effect(creatng, CSAfF_Sight))
            {
                if (creature_can_see_thing_ignoring_specific_door(creatng, doortng, doortng))
                {
                    reveal_secret_door_to_player(doortng, creatng->owner);
                }
            }
            else if (creature_can_see_thing(creatng, doortng))
            { // When closed the door itself blocks sight to the doortng so this checks if open, and in sight.
                reveal_secret_door_to_player(doortng, creatng->owner);
            }
            doortng->mappos.z.stl.num = z;
        }
        i = doortng->next_of_class;
    }
}

TngUpdateRet process_creature_state(struct Thing *thing)
{
    SYNCDBG(19,"Starting for %s index %d owned by player %d",thing_model_name(thing),(int)thing->index,(int)thing->owner);
    TRACE_THING(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    unsigned long model_flags = get_creature_model_flags(thing);

    process_person_moods_and_needs(thing);
    if (creature_available_for_combat_this_turn(thing))
    {
        TbBool fighting = creature_look_for_combat(thing);
        if (!fighting) {
            fighting = creature_look_for_enemy_heart_combat(thing);
        }
        if (!fighting) {
            fighting = creature_look_for_enemy_object_combat(thing);
        }
    }
    creature_look_for_hidden_doors(thing);
    if ((cctrl->combat_flags & CmbtF_DoorFight) == 0)
    {
        if ((cctrl->collided_door_subtile > 0) && ((cctrl->creature_control_flags & CCFlg_NoCompControl) == 0))
        {
            if ( can_change_from_state_to(thing, thing->active_state, CrSt_CreatureDoorCombat) )
            {
                long x = stl_num_decode_x(cctrl->collided_door_subtile);
                long y = stl_num_decode_y(cctrl->collided_door_subtile);
                struct Thing* doortng = get_door_for_position(x, y);
                if ((!thing_is_invalid(doortng)) && (thing->owner != PLAYER_NEUTRAL))
                {
                    if (thing->owner != doortng->owner)
                    {
                        if (set_creature_door_combat(thing, doortng))
                        {
                            // If the door gets attacked, we're not running into it
                            cctrl->collided_door_subtile = 0;
                        }
                    }
                }
                else
                {
                    // If the door does not exist, clear this field too.
                    cctrl->collided_door_subtile = 0;
                    set_start_state(thing);
                }
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
    if (((thing->movement_flags & TMvF_Flying) == 0) && ((model_flags & (CMF_IsSpecDigger|CMF_IsDiggingCreature)) == 0))
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
    struct CreatureStateConfig* stati = get_thing_active_state_info(thing);
    if (stati->process_state != 0) {
        short k = 0;
        if (stati->process_state > 0)
            k = process_func_list[stati->process_state](thing);
        else
            k = luafunc_crstate_func(stati->process_state, thing);

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
    struct Dungeon* killer_dungeon = get_players_num_dungeon(killer_idx);
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(victim);
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
    int n = get_creature_model_graphics(thing->model, CGI_Ambulate);
    int i = convert_td_iso(n);
    if (i != thing->anim_sprite)
        return 0;
    return 1;
}

TbBool check_for_door_collision_at(struct Thing *thing, struct Coord3d *pos, unsigned long blocked_flags)
{
    SYNCDBG(18,"Starting for %s",thing_model_name(thing));
    int nav_sizexy = thing_nav_sizexy(thing) / 2;
    MapSubtlCoord start_x = coord_subtile(pos->x.val - nav_sizexy);
    MapSubtlCoord end_x = coord_subtile(pos->x.val + nav_sizexy);
    MapSubtlCoord start_y = coord_subtile(pos->y.val - nav_sizexy);
    MapSubtlCoord end_y = coord_subtile(pos->y.val + nav_sizexy);
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    if ((blocked_flags & 0x01) != 0)
    {
        stl_x = end_x;
        if (thing->mappos.x.val >= pos->x.val)
            stl_x = start_x;
        for (stl_y = start_y; stl_y <= end_y; stl_y++)
        {
            struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            if ((mapblk->flags & SlbAtFlg_IsDoor) != 0) {
                SYNCDBG(18,"Door collision at X with %s",thing_model_name(thing));
                struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
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
            struct Map* mapblk = get_map_block_at(stl_x, stl_y);
            if ((mapblk->flags & SlbAtFlg_IsDoor) != 0) {
                SYNCDBG(18,"Door collision at Y with %s",thing_model_name(thing));
                struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
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
    unsigned int flags = 0;
    struct Coord3d pos;
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    // Shift all elements freeing first item
    for (int i = 4; i > 0; i--)
    {
        cctrl->party.member_pos_stl[i] = cctrl->party.member_pos_stl[i-1];
    }
    // Fill the first item
    cctrl->party.member_pos_stl[0] = get_subtile_number(thing->mappos.x.stl.num,thing->mappos.y.stl.num);
}

long move_creature(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct Coord3d* tngpos = &thing->mappos;
    struct Coord3d pvpos;
    pvpos.x.val = tngpos->x.val;
    pvpos.y.val = tngpos->y.val;
    pvpos.z.val = tngpos->z.val;
    int velo_x = thing->velocity.x.val;
    int velo_y = thing->velocity.y.val;
    int velo_z = thing->velocity.z.val;
    cctrl->creature_control_flags &= ~CCFlg_RepositionedInWall;
    struct Coord3d nxpos;
    if (thing_in_wall_at(thing, &thing->mappos) && !creature_can_pass_through_wall_at(thing, &thing->mappos))
    {
        nxpos.x.val = tngpos->x.val;
        nxpos.y.val = tngpos->y.val;
        nxpos.z.val = tngpos->z.val;
        if (get_nearest_valid_position_for_creature_at(thing, &nxpos)) {
            move_thing_in_map(thing, &nxpos);
        }
        cctrl->creature_control_flags |= CCFlg_RepositionedInWall;
    }
    if ((get_creature_model_flags(thing) & CMF_Fat) != 0)
    {
        if (creature_is_ambulating(thing))
        {
            if (thing->current_frame > (keepersprite_frames(thing->anim_sprite)/2))
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
            if (thing_in_wall_at(thing, &nxpos) && !creature_can_pass_through_wall_at(thing, &nxpos))
            {
                if (creature_cannot_move_directly_to(thing, &nxpos))
                {
                    long blocked_flags = get_creature_blocked_flags_at(thing, &nxpos);
                    if (cctrl->collided_door_subtile == 0) {
                        check_for_door_collision_at(thing, &nxpos, blocked_flags);
                    }
                    slide_thing_against_wall_at(thing, &nxpos, blocked_flags);
                }
                else
                {
                    nxpos.z.val = tngpos->z.val;
                }
            }
        }
        else
        {
            if (thing_in_wall_at(thing, &nxpos) && !creature_can_pass_through_wall_at(thing, &nxpos))
            {
                if (creature_cannot_move_directly_to(thing, &nxpos))
                {
                    long blocked_flags = get_creature_blocked_flags_at(thing, &nxpos);
                    if (cctrl->collided_door_subtile == 0) {
                        check_for_door_collision_at(thing, &nxpos, blocked_flags);
                    }
                    slide_thing_against_wall_at(thing, &nxpos, blocked_flags);
                }
                else
                {
                    nxpos.z.val = tngpos->z.val;
                }
            }
        }
        if ((cctrl->creature_control_flags & CCFlg_AvoidCreatureCollision) != 0)
        {
            struct Thing* collidtng = get_thing_collided_with_at_satisfying_filter(thing, &nxpos, collide_filter_thing_is_of_type, 5, -1);
            if (!thing_is_invalid(collidtng))
            {
                nxpos.x.val = tngpos->x.val;
                nxpos.y.val = tngpos->y.val;
                nxpos.z.val = tngpos->z.val;
            }
        }
        if ((tngpos->x.stl.num != nxpos.x.stl.num) || (tngpos->y.stl.num != nxpos.y.stl.num))
        {
            if (is_hero_tunnelling_to_attack(thing)) {
                update_tunneller_trail(thing);
            }
            if ((subtile_slab(tngpos->x.stl.num) != subtile_slab(nxpos.x.stl.num))
             || (subtile_slab(tngpos->y.stl.num) != subtile_slab(nxpos.y.stl.num)))
            {
                check_map_explored(thing, nxpos.x.stl.num, nxpos.y.stl.num);
                struct CreatureStateConfig* stati = get_thing_active_state_info(thing);
                if (!state_info_invalid(stati)) {
                    if (stati->move_from_slab > 0)
                    {
                        move_from_slab_func_list[stati->move_from_slab](thing);
                    }
                    else if (stati->move_from_slab < 0)
                    {
                        luafunc_crstate_func(stati->move_from_slab, thing);
                    }
                }
            }
        }
        move_thing_in_map(thing, &nxpos);
    }
    {
        long angle = LbArcTanAngle(cctrl->moveaccel.x.val, cctrl->moveaccel.y.val);
        long dist;
        if (get_angle_difference(angle, thing->move_angle_xy) <= DEGREES_90)
        {
            dist = get_2d_distance(&pvpos, tngpos);
        }
        else
        {
            dist = -get_2d_distance(&pvpos, tngpos);
        }
        cctrl->distance_to_destination = dist;
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct Thing* lairtng = thing_get(cctrl->lairtng_idx);
    if (!thing_exists(lairtng))
    {
        // If creature has no lair - treat dungeon heart as lair
        lairtng = get_player_soul_container(thing->owner);
    }
    if (cctrl->exp_level > 0)
        set_creature_level(thing, cctrl->exp_level-1);
    thing->health = cctrl->max_health;
    if (creature_under_spell_effect(thing, CSAfF_Timebomb))
    {
        clear_flag(cctrl->spell_flags, CSAfF_Timebomb);
        thing->veloc_push_add.x.val = 0;
        thing->veloc_push_add.y.val = 0;
        clear_flag(thing->state_flags, TF1_PushAdd);
        cleanup_current_thing_state(thing);
        set_start_state(thing);
        cctrl->active_timebomb_spell = 0;
    }
    if (!thing_exists(lairtng))
        return;
    create_effect(&thing->mappos, TngEff_HarmlessGas2, thing->owner);
    move_thing_in_map(thing, &lairtng->mappos);
    reset_interpolation_of_thing(thing);
    lua_on_creature_rebirth(thing);
    create_effect(&lairtng->mappos, TngEff_HarmlessGas2, thing->owner);
}

void throw_out_gold(struct Thing* thing, long amount)
{
    int num_pots_to_drop;
    // Compute if we want bags or pots
    int dropject = 6; //GOLD object
    if ((game.conf.rules[thing->owner].game.pot_of_gold_holds > game.conf.rules[thing->owner].game.bag_gold_hold) && (amount <= game.conf.rules[thing->owner].game.bag_gold_hold))
    {
            dropject = 136; //Drop GOLD_BAG object when we're dealing with small amounts
            num_pots_to_drop = 1;
    }
    else //drop pots
    {
        // Compute how many pots we want to drop
        num_pots_to_drop = ((amount + game.conf.rules[thing->owner].game.pot_of_gold_holds - 1) / game.conf.rules[thing->owner].game.pot_of_gold_holds);
        if (num_pots_to_drop > 8)
        {
            num_pots_to_drop = 8;
        }
    }

    GoldAmount gold_dropped = 0;
    // Now do the dropping
    for (int npot = 0; npot < num_pots_to_drop; npot++)
    {
        // Create a new pot object
        struct Thing* gldtng = create_object(&thing->mappos, dropject, game.neutral_player_num, -1);
        if (thing_is_invalid(gldtng))
            break;
        // Update its position and acceleration
        long angle = THING_RANDOM(thing, DEGREES_360);
        long radius = THING_RANDOM(thing, 128);
        long x = (radius * LbSinL(angle)) / 256;
        long y = (radius * LbCosL(angle)) / 256;
        gldtng->veloc_push_add.x.val += x/256;
        gldtng->veloc_push_add.y.val -= y/256;
        gldtng->veloc_push_add.z.val += THING_RANDOM(thing, 64) + 96;
        gldtng->state_flags |= TF1_PushAdd;
        // Set the amount of gold and mark that we've dropped that gold
        GoldAmount delta = (amount - gold_dropped) / (num_pots_to_drop - npot);
        gldtng->valuable.gold_stored = delta;
        // Update size of the gold object
        add_gold_to_pile(gldtng, 0);
        gold_dropped += delta;
    }
}

void creature_throw_out_gold(struct Thing* creatng)
{
    if (creatng->creature.gold_carried <= 0)
    {
        return;
    }
    throw_out_gold(creatng, creatng->creature.gold_carried);
}

struct Thing* thing_death_normal(struct Thing *thing)
{
    long memp1 = thing->move_angle_xy;
    struct Coord3d memaccl;
    memaccl.x.val = thing->veloc_base.x.val;
    memaccl.y.val = thing->veloc_base.y.val;
    memaccl.z.val = thing->veloc_base.z.val;
    struct Thing* deadtng = destroy_creature_and_create_corpse(thing, DCrSt_Dying);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Cannot create dead thing");
        return INVALID_THING;
    }
    struct Coord3d pos;
    TbBool move_allowed = get_thing_next_position(&pos, deadtng);
    if (!positions_equivalent(&deadtng->mappos, &pos))
    {
        if ((move_allowed) && !thing_in_wall_at(deadtng, &pos))
        {
            deadtng->move_angle_xy = memp1;
            deadtng->veloc_base.x.val = memaccl.x.val;
            deadtng->veloc_base.y.val = memaccl.y.val;
            deadtng->veloc_base.z.val = memaccl.z.val;
        }
    }
    return deadtng;
}

/**
 * Creates an effect of death with bloody flesh explosion, killing the creature.
 * @param thing
 */
struct Thing* thing_death_flesh_explosion(struct Thing *thing)
{
    long memp1 = thing->move_angle_xy;
    struct Coord3d memaccl;
    memaccl.x.val = thing->veloc_base.x.val;
    memaccl.y.val = thing->veloc_base.y.val;
    memaccl.z.val = thing->veloc_base.z.val;
    for (long i = 0; i <= thing->clipbox_size_z; i += 64)
    {
        struct Coord3d pos;
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = thing->mappos.z.val+i;
        create_effect(&pos, TngEff_Blood4, thing->owner);
    }
    struct Thing* deadtng = destroy_creature_and_create_corpse(thing, DCrSt_Dead);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Cannot create dead thing");
        return INVALID_THING;
    }
    deadtng->move_angle_xy = memp1;
    deadtng->veloc_base.x.val = memaccl.x.val;
    deadtng->veloc_base.y.val = memaccl.y.val;
    deadtng->veloc_base.z.val = memaccl.z.val;
    thing_play_sample(deadtng, 47, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
    return deadtng;
}

struct Thing* thing_death_gas_and_flesh_explosion(struct Thing *thing)
{
    struct Coord3d pos;
    long i;
    long memp1 = thing->move_angle_xy;
    struct Coord3d memaccl;
    memaccl.x.val = thing->veloc_base.x.val;
    memaccl.y.val = thing->veloc_base.y.val;
    memaccl.z.val = thing->veloc_base.z.val;
    for (i = 0; i <= thing->clipbox_size_z; i+=64)
    {
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = thing->mappos.z.val+i;
        create_effect(&pos, TngEff_Blood4, thing->owner);
    }
    i = (thing->clipbox_size_z >> 1);
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val+i;
    create_effect(&pos, TngEff_Gas3, thing->owner);
    struct Thing* deadtng = destroy_creature_and_create_corpse(thing, DCrSt_Dead);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Cannot create dead thing");
        return INVALID_THING;
    }
    deadtng->move_angle_xy = memp1;
    deadtng->veloc_base.x.val = memaccl.x.val;
    deadtng->veloc_base.y.val = memaccl.y.val;
    deadtng->veloc_base.z.val = memaccl.z.val;
    thing_play_sample(deadtng, 47, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
    return deadtng;
}

struct Thing* thing_death_smoke_explosion(struct Thing *thing)
{
    long memp1 = thing->move_angle_xy;
    struct Coord3d memaccl;
    memaccl.x.val = thing->veloc_base.x.val;
    memaccl.y.val = thing->veloc_base.y.val;
    memaccl.z.val = thing->veloc_base.z.val;
    long i = (thing->clipbox_size_z >> 1);
    struct Coord3d pos;
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val+i;
    create_effect(&pos, TngEff_HarmlessGas1, thing->owner);
    struct Thing* deadtng = destroy_creature_and_create_corpse(thing, DCrSt_Dead);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Cannot create dead thing");
        return INVALID_THING;
    }
    deadtng->move_angle_xy = memp1;
    deadtng->veloc_base.x.val = memaccl.x.val;
    deadtng->veloc_base.y.val = memaccl.y.val;
    deadtng->veloc_base.z.val = memaccl.z.val;
    thing_play_sample(deadtng, 47, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
    return deadtng;
}

/**
 * Creates an effect of frozen body explosion and kills the creature.
 * The ice explosion effect uses same corpse as flesh explosion.
 * @param thing
 */
struct Thing* thing_death_ice_explosion(struct Thing *thing)
{
    long memp1 = thing->move_angle_xy;
    struct Coord3d memaccl;
    memaccl.x.val = thing->veloc_base.x.val;
    memaccl.y.val = thing->veloc_base.y.val;
    memaccl.z.val = thing->veloc_base.z.val;
    for (long i = 0; i <= thing->clipbox_size_z; i += 64)
    {
        struct Coord3d pos;
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = thing->mappos.z.val+i;
        create_effect(&pos, TngEff_DeathIceExplosion, thing->owner);
    }
    struct Thing* deadtng = destroy_creature_and_create_corpse(thing, DCrSt_Dead);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Cannot create dead thing");
        return INVALID_THING;
    }
    deadtng->move_angle_xy = memp1;
    deadtng->veloc_base.x.val = memaccl.x.val;
    deadtng->veloc_base.y.val = memaccl.y.val;
    deadtng->veloc_base.z.val = memaccl.z.val;
    thing_play_sample(deadtng, 47, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
    return deadtng;
}

struct Thing* creature_death_as_nature_intended(struct Thing *thing)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    switch (crconf->natural_death_kind)
    {
    case Death_Normal:
        return thing_death_normal(thing);
    case Death_FleshExplode:
        return thing_death_flesh_explosion(thing);
    case Death_GasFleshExplode:
        return thing_death_gas_and_flesh_explosion(thing);
    case Death_SmokeExplode:
        return thing_death_smoke_explosion(thing);
    case Death_IceExplode:
        return thing_death_ice_explosion(thing);
    default:
        WARNLOG("Unexpected %s death cause %d",thing_model_name(thing), crconf->natural_death_kind);
        return INVALID_THING;
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
    SYNCDBG(18,"Starting");
    unsigned long n = 0;
    unsigned long k = 0;
    int i = list->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
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

struct Thing* cause_creature_death(struct Thing *thing, CrDeathFlags flags)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    anger_set_creature_anger_all_types(thing, 0);
    remove_parent_thing_from_things_in_list(&game.thing_lists[TngList_Shots],thing->index);
    ThingModel crmodel = thing->model;
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    if (!thing_exists(thing))
    {
        set_flag(flags,CrDed_NoEffects);
    }

    if ((!flag_is_set(flags,CrDed_NoEffects)) && (crconf->rebirth != 0)
     && (cctrl->lairtng_idx > 0) && (crconf->rebirth-1 <= cctrl->exp_level)
        && (!flag_is_set(flags,CrDed_NoRebirth)) )
    {
        creature_rebirth_at_lair(thing);
        return INVALID_THING;
    }

    if (!flag_is_set(flags,CrDed_NotReallyDying))
    {
        lua_on_creature_death(thing);
    }

    creature_throw_out_gold(thing);
    // Beyond this point, the creature thing is bound to be deleted
    if ((!flag_is_set(flags,CrDed_NotReallyDying)) || (flag_is_set(game.conf.rules[thing->owner].game.classic_bugs_flags,ClscBug_ResurrectRemoved)))
    {
        // If the creature is leaving dungeon, or being transformed, then CrDed_NotReallyDying should be set
        update_dead_creatures_list_for_owner(thing);
    }
    if (flag_is_set(get_creature_model_flags(thing), CMF_EventfulDeath)) //updates LAST_DEATH_EVENT for mapmakers
    {
        struct Dungeon* dungeon = get_dungeon(thing->owner);
        if (!dungeon_invalid(dungeon))
        {
            memcpy(&dungeon->last_eventful_death_location, &thing->mappos, sizeof(struct Coord3d));
        }
    }
    if (flag_is_set(flags, CrDed_NoEffects))
    {
        if (flag_is_set(game.mode_flags, MFlg_DeadBackToPool))
        {
            add_creature_to_pool(crmodel, 1);
        }
        delete_thing_structure(thing, 0);
    } else
    if (!creature_model_bleeds(thing->model))
    {
        // Non-bleeding creatures have no flesh explosion effects
        if ((game.mode_flags & MFlg_DeadBackToPool) != 0)
            add_creature_to_pool(crmodel, 1);
        return creature_death_as_nature_intended(thing);
    } else
    if (creature_under_spell_effect(thing, CSAfF_Freeze))
    {
        if ((game.mode_flags & MFlg_DeadBackToPool) != 0)
            add_creature_to_pool(crmodel, 1);
        return thing_death_ice_explosion(thing);
    } else
    if (shot_model_makes_flesh_explosion(cctrl->shot_model))
    {
        if ((game.mode_flags & MFlg_DeadBackToPool) != 0)
            add_creature_to_pool(crmodel, 1);
        return thing_death_flesh_explosion(thing);
    } else
    {
        if ((game.mode_flags & MFlg_DeadBackToPool) != 0)
            add_creature_to_pool(crmodel, 1);
        return creature_death_as_nature_intended(thing);
    }
    return INVALID_THING;
}

void prepare_to_controlled_creature_death(struct Thing *thing)
{
    struct PlayerInfo* player = get_player(thing->owner);
    leave_creature_as_controller(player, thing);
    player->influenced_thing_idx = 0;
    player->influenced_thing_creation = 0;
    if (player->id_number == thing->owner)
        setup_eye_lens(0);
    set_camera_zoom(player->acamera, player->dungeon_camera_zoom);
    if (player->id_number == thing->owner)
    {
        turn_off_all_window_menus();
        turn_off_query_menus();
        turn_on_main_panel_menu();
        set_flag_value(game.operation_flags, GOF_ShowPanel, (game.operation_flags & GOF_ShowGui) != 0);
  }
  light_turn_light_on(player->cursor_light_idx);
  PaletteSetPlayerPalette(player, engine_palette);
}

void delete_armour_effects_attached_to_creature(struct Thing *thing)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        return;
    }
    struct Thing *efftng;
    for (int i = 0; i < 3; i++)
    {
        ThingIndex eff_idx = cctrl->spell_thing_index_armour[i];
        if (eff_idx != 0)
        {
            efftng = thing_get(eff_idx);
            delete_thing_structure(efftng, 0);
            cctrl->spell_thing_index_armour[i] = 0;
        }
    }
}

void delete_disease_effects_attached_to_creature(struct Thing *thing)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        return;
    }
    struct Thing *efftng;
    for (int i = 0; i < 3; i++)
    {
        ThingIndex eff_idx = cctrl->spell_thing_index_disease[i];
        if (eff_idx != 0)
        {
            efftng = thing_get(eff_idx);
            delete_thing_structure(efftng, 0);
            cctrl->spell_thing_index_disease[i] = 0;
        }
    }
}

void delete_familiars_attached_to_creature(struct Thing* sumntng)
{
    struct Thing* famlrtng;
    struct CreatureControl* scctrl = creature_control_get_from_thing(sumntng);
    struct CreatureControl* fcctrl;
    if (creature_control_invalid(scctrl)) {
        return;
    }
    for (short i = 0; i < FAMILIAR_MAX; i++)
    {
        if (scctrl->familiar_idx[i])
        {
            famlrtng = thing_get(scctrl->familiar_idx[i]);
            fcctrl = creature_control_get_from_thing(famlrtng);
            fcctrl->unsummon_turn = game.play_gameturn;
        }
    }
}

struct Thing *kill_creature(struct Thing *creatng, struct Thing *killertng, PlayerNumber killer_plyr_idx, CrDeathFlags flags)
{
    SYNCDBG(18, "Starting");
    TRACE_THING(creatng);
    cleanup_creature_state_and_interactions(creatng);
    if (!thing_is_invalid(killertng))
    {
        if (killertng->owner == game.neutral_player_num)
        {
            clear_flag(flags, CrDed_DiedInBattle);
        }
    }
    if (killer_plyr_idx == game.neutral_player_num)
    {
        clear_flag(flags, CrDed_DiedInBattle);
    }
    if (!thing_exists(creatng))
    {
        ERRORLOG("Tried to kill non-existing thing!");
        return INVALID_THING;
    }
    // Creature must be visible and not chicken & clear Rebound for some reason.
    if (creature_under_spell_effect(creatng, CSAfF_Invisibility))
    {
        clean_spell_effect(creatng, CSAfF_Invisibility);
    }
    if (creature_under_spell_effect(creatng, CSAfF_Chicken))
    {
        clean_spell_effect(creatng, CSAfF_Chicken);
    }
    if (creature_under_spell_effect(creatng, CSAfF_Rebound))
    {
        clean_spell_effect(creatng, CSAfF_Rebound);
    }
    // Terminate all the actives spell effects with damage > 0.
    terminate_all_actives_damage_over_time_spell_effects(creatng);
    struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
    if ((cctrl->unsummon_turn > 0) && (cctrl->unsummon_turn > game.play_gameturn))
    {
        create_effect_around_thing(creatng, ball_puff_effects[get_player_color_idx(creatng->owner)]);
        set_flag(flags, CrDed_NotReallyDying | CrDed_NoEffects);
        if (flag_is_set(flags, CrDed_NoEffects) && flag_is_set(creatng->alloc_flags, TAlF_IsControlled))
        {
            prepare_to_controlled_creature_death(creatng);
        }
        return cause_creature_death(creatng, flags);
    }
    struct Dungeon *dungeon = (!is_neutral_thing(creatng)) ? get_players_num_dungeon(creatng->owner) : INVALID_DUNGEON;
    if (!dungeon_invalid(dungeon))
    {
        if (flag_is_set(flags, CrDed_DiedInBattle))
        {
            dungeon->battles_lost++;
        }
    }
    update_kills_counters(creatng, killertng, killer_plyr_idx, flags);

    // 'killertng' could be a trap, so verify if it has valid creature control before increasing the kill count and adjusting its anger.
    struct CreatureControl* cctrlgrp = creature_control_get_from_thing(killertng);
    if (!creature_control_invalid(cctrlgrp))
    {
        cctrlgrp->kills_num++;
        if (!players_creatures_tolerate_each_other(killertng->owner, creatng->owner))
        {
            cctrlgrp->kills_num_enemy++;
        }
        else
        {
            cctrlgrp->kills_num_allied++;
        }
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(killertng);
        anger_apply_anger_to_creature(killertng, crconf->annoy_win_battle, AngR_Other, 1);
    }

    if (thing_is_invalid(killertng) || (killertng->owner == game.neutral_player_num) || (killer_plyr_idx == game.neutral_player_num) || dungeon_invalid(dungeon))
    {
        if (flag_is_set(flags, CrDed_NoEffects) && flag_is_set(creatng->alloc_flags, TAlF_IsControlled))
        {
            prepare_to_controlled_creature_death(creatng);
        }
        return cause_creature_death(creatng, flags);
    }
    // Now we are sure that killertng and dungeon pointers are correct.
    if (creatng->owner == killertng->owner)
    {
        if ((get_creature_model_flags(creatng) & CMF_IsDiptera) && (get_creature_model_flags(killertng) & CMF_IsArachnid))
        {
            dungeon->lvstats.flies_killed_by_spiders++;
        }
    }
    if (is_my_player_number(creatng->owner))
    {
        if (flag_is_set(flags, CrDed_DiedInBattle))
        {
            output_message_far_from_thing(creatng, SMsg_BattleDeath, MESSAGE_DURATION_BATTLE);
        }
    }
    else if (is_my_player_number(killertng->owner))
    {
        output_message_far_from_thing(creatng, SMsg_BattleWon, MESSAGE_DURATION_BATTLE);
    }
    SYNCDBG(18, "Almost finished");
    if (!creature_can_be_set_unconscious(creatng, killertng, flags))
    {
        if (!flag_is_set(flags, CrDed_NoEffects))
        {
            return cause_creature_death(creatng, flags);
        }
    }
    if (flag_is_set(flags, CrDed_NoEffects))
    {
        if (flag_is_set(creatng->alloc_flags, TAlF_IsControlled))
        {
            prepare_to_controlled_creature_death(creatng);
        }
        return cause_creature_death(creatng, flags);
    }
    make_creature_unconscious(creatng);
    creatng->health = 1;
    return INVALID_THING;
}

void process_creature_standing_on_corpses_at(struct Thing *creatng, struct Coord3d *pos)
{
    SYNCDBG(18,"Starting for %s at %d,%d",thing_model_name(creatng),(int)pos->x.stl.num,(int)pos->y.stl.num);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Map* mapblk = get_map_block_at(pos->x.stl.num, pos->y.stl.num);
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
        if (thing->class_id == TCls_DeadCreature)
        {
            if (!is_hero_thing(creatng))
            {
                struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
                int annoy_val;
                if (thing->owner == creatng->owner)
                {
                    annoy_val = crconf->annoy_on_dead_friend;
                }
                else
                {
                    annoy_val = crconf->annoy_on_dead_enemy;
                }
                anger_apply_anger_to_creature(creatng, annoy_val, AngR_Other, 1);
            }
            cctrl->bloody_footsteps_turns = 20;
            cctrl->corpse_to_piss_on = thing->index;
            // Stop after one body was found
            break;
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
}

/**
 * Calculates damage made by a creature by hand (using strength).
 * @param thing The creature which will be inflicting the damage.
 */
long calculate_melee_damage(struct Thing *creatng, short damage_percent)
{
    const struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    const struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    long strength = calculate_correct_creature_strength(creatng);
    long damage = compute_creature_attack_melee_damage(strength, crconf->luck, cctrl->exp_level, creatng);
    if (damage_percent != 0)
    {
        damage = (damage * damage_percent) / 100;
    }
    return damage;
}

/**
 * Projects damage made by a creature by hand (using strength).
 * Gives a best estimate of the damage, but shouldn't be used to actually inflict it.
 * @param thing The creature which will be inflicting the damage.
 */
long project_melee_damage(const struct Thing *creatng)
{
    const struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    const struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    long strength = calculate_correct_creature_strength(creatng);
    return project_creature_attack_melee_damage(strength, 0, crconf->luck, cctrl->exp_level, creatng);
}

/**
 * Calculates damage made by a creature using specific shot model.
 * @param thing The creature which will be shooting.
 * @param shot_model Shot kind which will be created.
 */
long calculate_shot_damage(struct Thing *creatng, ThingModel shot_model)
{
    const struct ShotConfigStats* shotst = get_shot_model_stats(shot_model);
    const struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    const struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    return compute_creature_attack_spell_damage(shotst->damage, crconf->luck, cctrl->exp_level, creatng->owner);
}

static void shot_init_lizard(const struct Thing *target, short angle_xy, unsigned char dexterity, struct Thing *shotng)
{
    if (!thing_is_invalid(target))
    {
        long range = 2200 - (dexterity * 19);
        range = range < 1 ? 1 : range;
        long rnd = (THING_RANDOM(shotng, 2 * range) - range);
        rnd = rnd < (range / 3) && rnd > 0 ? (THING_RANDOM(shotng, range / 2) + (range / 2)) + 200 : rnd + 200;
        rnd = rnd > -(range / 3) && rnd < 0 ? -(THING_RANDOM(shotng, range / 3) + (range / 3)) : rnd;
        long x = move_coord_with_angle_x(target->mappos.x.val, rnd, angle_xy);
        long y = move_coord_with_angle_y(target->mappos.y.val, rnd, angle_xy);
        int posint = y / game.conf.crtr_conf.sprite_size;
        shotng->shot_lizard.x = x;
        shotng->shot_lizard.posint = posint;
        shotng->shot_lizard2.range = range / 10;
    }
}

static void shot_set_start_pos(const struct Thing *firing, const struct ShotConfigStats *shotst, struct Coord3d *pos1)
{
    if (map_is_solid_at_height(pos1->x.stl.num, pos1->y.stl.num, pos1->z.val, pos1->z.val + shotst->size_z))
    {
        pos1->x.val = firing->mappos.x.val;
        pos1->y.val = firing->mappos.y.val;
    }
}

void thing_fire_shot(struct Thing *firing, struct Thing *target, ThingModel shot_model, CrtrExpLevel shot_level, unsigned char hit_type)
{
    struct Coord3d pos2;
    struct Thing *tmptng;
    short angle_xy;
    short angle_yz;
    short speed;
    long damage;
    unsigned char dexterity, max_dexterity;

    struct ShotConfigStats* shotst = get_shot_model_stats(shot_model);
    TbBool flag1 = false;
    // Prepare source position
    struct Coord3d pos1;
    pos1.x.val = firing->mappos.x.val;
    pos1.y.val = firing->mappos.y.val;
    pos1.z.val = firing->mappos.z.val;
    if (firing->class_id == TCls_Trap)
    {
        if (shotst->fire_logic == ShFL_Volley)
        {
            if (!firing->trap.volley_fire)
            {
                firing->trap.volley_fire = true;
                firing->trap.volley_repeat = shotst->effect_amount - 1; // N x shots + (N - 1) x pauses and one shot is this one
                firing->trap.volley_delay = shotst->effect_spacing;
                firing->trap.firing_at = thing_is_invalid(target)? 0 : target->index;
            }
            else
            {
                firing->trap.volley_delay = shotst->effect_spacing;
                if (firing->trap.volley_repeat == 0)
                    return;
                firing->trap.volley_repeat--;
            }
        }
        struct TrapConfigStats *trapst = get_trap_model_stats(firing->model);
        firing->move_angle_xy = get_angle_xy_to(&firing->mappos, &target->mappos); //visually rotates the trap
        pos1.x.val += distance_with_angle_to_coord_x(trapst->shot_shift_x, firing->move_angle_xy + DEGREES_90);
        pos1.y.val += distance_with_angle_to_coord_y(trapst->shot_shift_x, firing->move_angle_xy + DEGREES_90);
        pos1.x.val += distance_with_angle_to_coord_x(trapst->shot_shift_y, firing->move_angle_xy);
        pos1.y.val += distance_with_angle_to_coord_y(trapst->shot_shift_y, firing->move_angle_xy);
        pos1.z.val += trapst->shot_shift_z;

        max_dexterity = UCHAR_MAX;
        dexterity = max_dexterity/4 + THING_RANDOM(firing, max_dexterity/2);
    }
    else
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(firing);
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(firing);
        if (shotst->fire_logic == ShFL_Volley)
        {
            if (!firing->creature.volley_fire)
            {
                firing->creature.volley_fire = true;
                firing->creature.volley_repeat = shotst->effect_amount - 1; // N x shots + (N - 1) x pauses and one shot is this one
                cctrl->inst_action_turns += shotst->effect_spacing + 1; // because of post check
            }
            else
            {
                cctrl->inst_action_turns += shotst->effect_spacing + 1;
                if (firing->creature.volley_repeat == 0)
                    return;
                firing->creature.volley_repeat--;
            }
        }

        dexterity = calculate_correct_creature_dexterity(firing);
        max_dexterity = crconf->dexterity + ((crconf->dexterity * cctrl->exp_level * game.conf.crtr_conf.exp.dexterity_increase_on_exp) / 100);

        pos1.x.val += distance_with_angle_to_coord_x((cctrl->shot_shift_x + (cctrl->shot_shift_x * game.conf.crtr_conf.exp.size_increase_on_exp * cctrl->exp_level) / 100), firing->move_angle_xy + DEGREES_90);
        pos1.y.val += distance_with_angle_to_coord_y((cctrl->shot_shift_x + (cctrl->shot_shift_x * game.conf.crtr_conf.exp.size_increase_on_exp * cctrl->exp_level) / 100), firing->move_angle_xy + DEGREES_90);
        pos1.x.val += distance_with_angle_to_coord_x((cctrl->shot_shift_y + (cctrl->shot_shift_y * game.conf.crtr_conf.exp.size_increase_on_exp * cctrl->exp_level) / 100), firing->move_angle_xy);
        pos1.y.val += distance_with_angle_to_coord_y((cctrl->shot_shift_y + (cctrl->shot_shift_y * game.conf.crtr_conf.exp.size_increase_on_exp * cctrl->exp_level) / 100), firing->move_angle_xy);
        pos1.z.val += (cctrl->shot_shift_z + (cctrl->shot_shift_z * game.conf.crtr_conf.exp.size_increase_on_exp * cctrl->exp_level) / 100);
    }
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
        pos2.z.val += (target->clipbox_size_z >> 1);
        if (((shotst->model_flags & ShMF_StrengthBased) != 0) && ((shotst->model_flags & ShMF_ReboundImmune) != 0) && (target->class_id != TCls_Door))
        {
          flag1 = true;
          pos1.z.val = pos2.z.val;
        }
        angle_xy = get_angle_xy_to(&pos1, &pos2);
        angle_yz = get_angle_yz_to(&pos1, &pos2);
    }
    // Compute shot damage
    damage = shotst->damage;
    if (shotst->fixed_damage == 0)
    {
        if ((shotst->model_flags & ShMF_StrengthBased) != 0)
        {
            damage = calculate_melee_damage(firing,shotst->damage);
        }
        else
        {
            damage = calculate_shot_damage(firing, shot_model);
        }
    }
    if ((shotst->model_flags & ShMF_Disarming) && thing_is_deployed_trap(target))
    {
        hit_type = THit_TrapsAll;
    }
    struct Thing* shotng = NULL;
    long target_idx = 0;
    // Set target index for navigating shots
    if (!thing_is_invalid(target))
    {
        target_idx = target->index;
    }
    struct ComponentVector cvect;

    switch (shotst->fire_logic)
    {
    case ShFL_Beam:
        if ((thing_is_invalid(target)) || (get_2d_distance(&firing->mappos, &pos2) > shotst->max_range))
        {
            project_point_to_wall_on_angle(&pos1, &pos2, firing->move_angle_xy, firing->move_angle_z, COORD_PER_STL, shotst->max_range/COORD_PER_STL);
        }
        shotng = create_thing(&pos2, TCls_Shot, shot_model, firing->owner, -1);
        if (thing_is_invalid(shotng))
          return;
        draw_lightning(&pos1, &pos2, shotst->effect_spacing, shotst->effect_id);
        shotng->health = shotst->health;
        shotng->shot.damage = damage;
        shotng->parent_idx = firing->index;
        break;
    case ShFL_Breathe:
        if ((thing_is_invalid(target)) || (get_2d_distance(&firing->mappos, &pos2) > shotst->max_range))
          project_point_to_wall_on_angle(&pos1, &pos2, firing->move_angle_xy, firing->move_angle_z, COORD_PER_STL, shotst->max_range/COORD_PER_STL);
        shotng = create_thing(&pos2, TCls_Shot, shot_model, firing->owner, -1);
        if (thing_is_invalid(shotng))
          return;

        draw_flame_breath(&pos1, &pos2, shotst->effect_spacing, shotst->effect_amount,shotst->effect_id, shotng->index);
        shotng->health = shotst->health;
        shotng->shot.damage = damage;
        shotng->parent_idx = firing->index;
        break;
    case ShFL_Hail:
    {
        long i;
        shot_set_start_pos(firing, shotst, &pos1);
        for (i = 0; i < shotst->effect_amount; i++)
        {
            tmptng = create_thing(&pos1, TCls_Shot, shot_model, firing->owner, -1);
            if (thing_is_invalid(tmptng))
              break;
            shotng = tmptng;

            if (shotst->speed_deviation)
            {
                speed = (short)(shotst->speed - (shotst->speed_deviation/2) + (THING_RANDOM(shotng, shotst->speed_deviation)));
            }
            else
            {
                speed = shotst->speed;
            }
            shotng->shot.hit_type = hit_type;
            shotng->move_angle_xy = (short)((angle_xy + THING_RANDOM(shotng, 2 * shotst->spread_xy + 1) - shotst->spread_xy) & ANGLE_MASK);
            shotng->move_angle_z = (short)((angle_yz + THING_RANDOM(shotng, 2 * shotst->spread_z + 1) - shotst->spread_z) & ANGLE_MASK);
            angles_to_vector(shotng->move_angle_xy, shotng->move_angle_z, speed, &cvect);
            shotng->veloc_push_add.x.val += cvect.x;
            shotng->veloc_push_add.y.val += cvect.y;
            shotng->veloc_push_add.z.val += cvect.z;
            shotng->state_flags |= TF1_PushAdd;
            shotng->shot.damage = damage;
            shotng->health = shotst->health;
            shotng->parent_idx = firing->index;
        }
        break;
    }
    case ShFL_Volley:
        // fallthrough
    case ShFL_Lizard:
    case ShFL_Default:
    default:
        shot_set_start_pos(firing, shotst, &pos1);
        shotng = create_thing(&pos1, TCls_Shot, shot_model, firing->owner, -1);
        if (thing_is_invalid(shotng))
            return;
        if (shotst->spread_xy || shotst->spread_z)
        {
            shotng->move_angle_xy = (short)((angle_xy + THING_RANDOM(shotng, 2 * shotst->spread_xy + 1) - shotst->spread_xy) & ANGLE_MASK);
            shotng->move_angle_z = (short)((angle_yz + THING_RANDOM(shotng, 2 * shotst->spread_z + 1) - shotst->spread_z) & ANGLE_MASK);
        }
        else
        {
            shotng->move_angle_xy = angle_xy;
            shotng->move_angle_z = angle_yz;
        }
        if (shotst->speed_deviation)
        {
            speed = (short)(shotst->speed - (shotst->speed_deviation / 2) + (THING_RANDOM(shotng, shotst->speed_deviation)));
        }
        else
        {
            speed = shotst->speed;
        }
        angles_to_vector(shotng->move_angle_xy, shotng->move_angle_z, speed, &cvect);
        shotng->veloc_push_add.x.val += cvect.x;
        shotng->veloc_push_add.y.val += cvect.y;
        shotng->veloc_push_add.z.val += cvect.z;
        shotng->state_flags |= TF1_PushAdd;
        shotng->shot.damage = damage;
        shotng->health = shotst->health;
        shotng->parent_idx = firing->index;
        shotng->shot.target_idx = target_idx;
        shotng->shot.dexterity = dexterity;
        if (shotst->fire_logic == ShFL_Lizard)
        {
            shot_init_lizard(target, angle_xy, dexterity, shotng);
        }
        break;
    }
    if (!thing_is_invalid(shotng))
    {
      damage = shotng->shot.damage;
      // Special debug code that shows amount of damage the shot will make
      if (flag_is_set(start_params.debug_flags, DFlg_ShotsDamage))
          create_price_effect(&pos1, my_player_number, damage);
#if (BFDEBUG_LEVEL > 0)
      if ((damage < 0) || (damage > 2000))
      {
        WARNLOG("Shot of type %d carries %d damage",(int)shot_model,(int)damage);
      }
#endif
      shotng->shot.hit_type = hit_type;
      if (shotst->firing_sound > 0)
      {
        thing_play_sample(firing, shotst->firing_sound + SOUND_RANDOM(shotst->firing_sound_variants),
            100, 0, 3, 0, 3, FULL_LOUDNESS);
      }
      if (shotst->shot_sound > 0)
      {
        thing_play_sample(shotng, shotst->shot_sound, NORMAL_PITCH, 0, 3, 0, shotst->sound_priority, FULL_LOUDNESS);
      }
      set_flag_value(shotng->movement_flags, TMvF_GoThroughWalls, flag1);
    }
}

void set_creature_level(struct Thing *thing, CrtrExpLevel exp_level)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Creature has no control");
        return;
    }
    if (exp_level > CREATURE_MAX_LEVEL - 1)
    {
        ERRORLOG("Level %d too high, bounding", (int)exp_level);
        exp_level = CREATURE_MAX_LEVEL - 1;
    }
    cctrl->exp_level = exp_level;
    set_creature_size_stuff(thing);
    update_relative_creature_health(thing);
    creature_increase_available_instances(thing);
    add_creature_score_to_owner(thing);
}

void init_creature_level(struct Thing *thing, CrtrExpLevel exp_level)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Creature has no control");
        return;
    }
    set_creature_level(thing, exp_level);
    thing->health = cctrl->max_health;
}

/** Retrieves speed of a creature.
 *
 * @param thing
 * @return
 */
long get_creature_speed(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
        return 0;
    long speed = cctrl->max_speed;
    if (speed < 0)
        speed = 0;
    if (speed > MAX_VELOCITY)
        speed = MAX_VELOCITY;
    return speed;
}

short get_creature_eye_height(const struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (!creature_control_invalid(cctrl))
    {
        int base_height;
        if (creature_under_spell_effect(creatng, CSAfF_Chicken))
        {
            base_height = 100;
        }
        else
        {
            struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
            base_height = crconf->base_eye_height;
        }
        return (base_height + (base_height * game.conf.crtr_conf.exp.size_increase_on_exp * cctrl->exp_level) / 100);
    }
    return 0;
}

/**
 * @brief Get the target of the given instance in the Possession mode.
 *
 * @param thing The Thing object of the caster.
 * @param inst_id The index of the instance be used.
 * @param packet The caster's owner's control's or action's packet. This is optional.
 * @return ThingIndex The index of the target thing.
 */
ThingIndex get_human_controlled_creature_target(struct Thing *thing, CrInstance inst_id, struct Packet *packet)
{
    ThingIndex index = 0;
    struct InstanceInfo *inst_inf = creature_instance_info_get(inst_id);

    if((inst_inf->instance_property_flags & InstPF_SelfBuff) != 0)
    {
        if ((inst_inf->instance_property_flags & InstPF_RangedBuff) == 0 ||
            ((packet != NULL) && (packet->additional_packet_values & PCAdV_CrtrContrlPressed) != 0))
        {
            // If it doesn't has RANGED_BUFF or the Possession key (default:left shift) is pressed,
            // cast on the caster itself.
            return thing->index;
        }
    }
    if((inst_inf->instance_property_flags & InstPF_RangedBuff) != 0)
    {
        if(inst_inf->primary_target != 5 && inst_inf->primary_target != 6)
        {
            ERRORLOG("The instance %d has RANGED_BUFF property but has no valid primary target.", inst_id);
        }
    }

    struct Thing *i;
    long angle_xy_to;
    long angle_difference;
    int smallest_angle_diff = INT_MAX;
    static const int range = 20;
    static const int max_hit_angle = 39;
    MapSubtlCoord stl_x = thing->mappos.x.stl.num;
    MapSubtlCoord stl_x_lower = stl_x - range;
    MapSubtlCoord stl_x_upper = stl_x + range;
    if ((stl_x - range) < 0)
        stl_x_lower = 0;
    if (stl_x_upper > game.map_subtiles_x)
        stl_x_upper = game.map_subtiles_x;
    MapSubtlCoord stl_y = thing->mappos.y.stl.num;
    MapSubtlCoord stl_y_lower = stl_y - range;
    MapSubtlCoord stl_y_upper = stl_y + range;
    if (stl_y + range > game.map_subtiles_y)
        stl_y_upper = game.map_subtiles_y;
    if (stl_y_lower < 0)
        stl_y_lower = 0;

    if (stl_y_upper < stl_y_lower)
    {
        return 0;
    }
    if (stl_y_upper >= stl_y_lower)
    {
        for (MapSubtlDelta y_step = ((stl_y_upper - stl_y_lower) + 1); y_step > 0; y_step--)
        {
            MapSubtlCoord x = stl_x_lower;
            if (x <= stl_x_upper)
            {
                for (MapSubtlDelta x_step = ((stl_x_upper - stl_x_lower) + 1); x_step > 0; x_step--)
                {
                    struct Map *mapblk = get_map_block_at(x, stl_y_lower);
                    for (i = thing_get(get_mapwho_thing_index(mapblk));
                         !thing_is_invalid(i);
                         i = thing_get(i->next_on_mapblk))
                    {
                        if (i != thing)
                        {
                            TbBool is_valid_target;
                            switch (inst_inf->primary_target)
                            {
                                case 1:
                                    is_valid_target = ((thing_is_creature(i) && !creature_is_being_unconscious(i)) || thing_is_dungeon_heart(i));
                                    break;
                                case 2:
                                    is_valid_target = (thing_is_creature(i) && !creature_is_being_unconscious(i));
                                    break;
                                case 3:
                                    is_valid_target = (((thing_is_creature(i) && !creature_is_being_unconscious(i)) || thing_is_dungeon_heart(i)) && i->owner != thing->owner);
                                    break;
                                case 4:
                                    is_valid_target = ((thing_is_creature(i) && !creature_is_being_unconscious(i)) && i->owner != thing->owner);
                                    break;
                                case 5:
                                    is_valid_target = (((thing_is_creature(i) && !creature_is_being_unconscious(i)) || thing_is_dungeon_heart(i)) && i->owner == thing->owner);
                                    break;
                                case 6:
                                    is_valid_target = ((thing_is_creature(i) && !creature_is_being_unconscious(i)) && i->owner == thing->owner);
                                    break;
                                case 7:
                                    is_valid_target = ((thing_is_creature(i) && !creature_is_being_unconscious(i)) || thing_is_dungeon_heart(i) || thing_is_deployed_trap(i));
                                    break;
                                case 8:
                                    is_valid_target = true;
                                    break;
                                default:
                                    ERRORLOG("Illegal primary target type for shot: %d", (int)inst_inf->primary_target);
                                    is_valid_target = false;
                                    break;
                            }
                            if (is_valid_target)
                            {
                                angle_xy_to = get_angle_xy_to(&thing->mappos, &i->mappos);
                                angle_difference = get_angle_difference(angle_xy_to, thing->move_angle_xy);
                                if (angle_difference >= max_hit_angle || !creature_can_see_thing(thing, i))
                                    angle_difference = INT32_MAX;
                                if (smallest_angle_diff > angle_difference)
                                {
                                    smallest_angle_diff = angle_difference;
                                    index = i->index;
                                }
                            }
                        }
                    }
                    x++;
                }
            }
            stl_y_lower++;
        }
    }
    return index;
}

/**
 * @brief Process the logic when player (in Possesson mode) uses a creature's instance.
 *
 * @param thing The creature being possessed.
 * @param inst_id The instance that player wants to use.
 * @param packet The packet being processed.
 * @return ThingIndex The target's index.
 */
ThingIndex process_player_use_instance(struct Thing *thing, CrInstance inst_id, struct Packet *packet)
{
    ThingIndex target_idx = get_human_controlled_creature_target(thing, inst_id, packet);
    struct InstanceInfo *inst_inf = creature_instance_info_get(inst_id);
    TbBool ok = false;
    struct Thing *target = thing_get(target_idx);
    if (flag_is_set(inst_inf->instance_property_flags, InstPF_RangedBuff) && inst_inf->validate_target_func != 0)
    {
        ok = creature_instances_validate_func_list[inst_inf->validate_target_func](thing, target, inst_id,
            inst_inf->validate_target_func_params[0], inst_inf->validate_target_func_params[1]);
    }
    if (!ok)
    {
        target = 0;
    }

    if (flag_is_set(inst_inf->instance_property_flags, InstPF_NeedsTarget))
    {
        if (target_idx == 0)
        {
            // If cannot find a valid target, do not use it and don't consider it used.

            // Make a rejection sound
            play_non_3d_sample(119);
            return 0;
        }
    }

    set_creature_instance(thing, inst_id, target_idx, 0);
    return target_idx;
}

long creature_instance_has_reset(const struct Thing *thing, long inst_idx)
{
    long ritime;
    const struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    const struct InstanceInfo* inst_inf = creature_instance_info_get(inst_idx);
    long delta = (long)game.play_gameturn - (long)cctrl->instance_use_turn[inst_idx];
    if ((thing->alloc_flags & TAlF_IsControlled) != 0)
    {
        ritime = inst_inf->fp_reset_time + cctrl->inst_total_turns - cctrl->inst_action_turns;
    } else
    {
        ritime = inst_inf->reset_time + cctrl->inst_total_turns - cctrl->inst_action_turns;
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
void get_creature_instance_times(const struct Thing *thing, long inst_idx, int32_t *ritime, int32_t *raitime)
{
    long itime;
    long aitime;
    struct InstanceInfo *inst_inf = creature_instance_info_get(inst_idx);
    if ((thing->alloc_flags & TAlF_IsControlled) != 0)
    {
        itime = inst_inf->fp_time;
        aitime = inst_inf->fp_action_time;
    }
    else
    {
        itime = inst_inf->time;
        aitime = inst_inf->action_time;
    }
    if (creature_under_spell_effect(thing, CSAfF_Slow))
    {
        aitime *= 2;
        itime *= 2;
    }
    if (creature_under_spell_effect(thing, CSAfF_Speed))
    {
        aitime /= 2;
        itime /= 2;
    }
    else if (creature_affected_by_slap(thing))
    {
        aitime = 3 * aitime / 4;
        itime = 3 * itime / 4;
    }
    else if (!is_neutral_thing(thing))
    {
        if (player_uses_power_obey(thing->owner))
        {
            aitime -= aitime / 4;
            itime -= itime / 4;
        }
    }
    if (aitime <= 1)
    {
        aitime = 1;
    }
    if (itime <= 1)
    {
        itime = 1;
    }
    *ritime = itime;
    *raitime = aitime;
}

void set_creature_instance(struct Thing *thing, CrInstance inst_idx, long targtng_idx, const struct Coord3d *pos)
{
    long i;
    short no_loop = 0;
    if (inst_idx == 0)
        return;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct InstanceInfo* inst_inf = creature_instance_info_get(inst_idx);
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
    int32_t itime;
    int32_t aitime;
    get_creature_instance_times(thing, inst_idx, &itime, &aitime);
    if ((cctrl->instance_id != CrInst_NULL) && (cctrl->instance_id == inst_idx))
    {
        if ((inst_inf->instance_property_flags & InstPF_RepeatTrigger) != 0)
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

    //Animations loop so they end with the starting frame again
    if (inst_inf->no_animation_loop)
    {
        no_loop = itime;
    }
    cctrl->instance_anim_step_turns = (get_lifespan_of_animation(i, 1) / itime) - no_loop;
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

unsigned short find_next_annoyed_creature(PlayerNumber plyr_idx, unsigned short current_annoyed_creature_idx)
{
    struct Thing *current_annoyed_creature = thing_get(current_annoyed_creature_idx);
    struct Thing **current_ptr = &current_annoyed_creature;
    struct Dungeon *dungeon = get_dungeon(plyr_idx);
    struct Thing *creatng;
    struct CreatureControl* cctrl;

    if ((current_annoyed_creature->alloc_flags & TAlF_Exists) == 0 ||
         !thing_is_creature(current_annoyed_creature) ||
         (current_annoyed_creature->alloc_flags & TAlF_IsInLimbo) != 0 ||
         (current_annoyed_creature->state_flags & TF1_InCtrldLimbo) != 0 ||
         current_annoyed_creature->active_state == CrSt_CreatureUnconscious)
    {
        creatng = thing_get(dungeon->creatr_list_start);
        if (thing_is_invalid(creatng))
        {
            dungeon->zoom_annoyed_creature_idx = 0;
        }
        else
        {
            while (!anger_is_creature_angry(creatng))
            {
                cctrl = creature_control_get_from_thing(creatng);
                creatng = thing_get(cctrl->players_next_creature_idx);
                if (!thing_exists(creatng))
                {
                    dungeon->zoom_annoyed_creature_idx = 0;
                    return 0;
                }
            }
            dungeon->zoom_annoyed_creature_idx = creatng->index;
            return creatng->index;
        }
    }
    else
    {
        cctrl = creature_control_get_from_thing(thing_get(dungeon->zoom_annoyed_creature_idx));
        creatng = thing_get(cctrl->players_next_creature_idx);

        if (thing_exists(creatng) &&
            thing_is_creature(creatng) &&
            (creatng->alloc_flags & TAlF_IsInLimbo) == 0 &&
            (creatng->state_flags & TF1_InCtrldLimbo) == 0 &&
            creatng->active_state != CrSt_CreatureUnconscious)
        {
            TbBool found = true;
            while (!anger_is_creature_angry(creatng) || (creatng->alloc_flags & TAlF_Exists) == 0 || !thing_is_creature(creatng) ||
                   (creatng->alloc_flags & TAlF_IsInLimbo) != 0 || (creatng->state_flags & TF1_InCtrldLimbo) != 0 || creatng->active_state == CrSt_CreatureUnconscious)
            {
                cctrl = creature_control_get_from_thing(creatng);
                creatng = thing_get(cctrl->players_next_creature_idx);
                if (!thing_exists(creatng))
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                dungeon->zoom_annoyed_creature_idx = creatng->index;
                return creatng->index;
            }
        }
        creatng = thing_get(dungeon->creatr_list_start);
        if (*current_ptr != creatng)
        {
            while (!thing_is_invalid(creatng))
            {
                if (anger_is_creature_angry(creatng) &&
                    (creatng->alloc_flags & TAlF_Exists) != 0 &&
                    thing_is_creature(creatng) &&
                    (creatng->alloc_flags & TAlF_IsInLimbo) == 0 &&
                    (creatng->state_flags & TF1_InCtrldLimbo) == 0 &&
                    creatng->active_state != CrSt_CreatureUnconscious)
                {
                    dungeon->zoom_annoyed_creature_idx = creatng->index;
                    return creatng->index;
                }
                cctrl = creature_control_get_from_thing(creatng);
                creatng = thing_get(cctrl->players_next_creature_idx);
                if (*current_ptr == creatng)
                    return dungeon->zoom_annoyed_creature_idx;
            }
        }
        return dungeon->zoom_annoyed_creature_idx;
    }
    return 0;
}

void draw_creature_view(struct Thing *thing)
{
  // If no eye lens required - just draw on the screen, directly
  struct PlayerInfo* player = get_my_player();
  struct Camera* render_cam = get_local_camera(&player->cameras[CamIV_FirstPerson]);
  if (((game.mode_flags & MFlg_EyeLensReady) == 0) || (eye_lens_memory == NULL) || (game.applied_lens_type == 0))
  {
      engine(player, render_cam);
      return;
  }
  // So there is an eye lens - we have to put a buffer in place of screen,
  // draw on that buffer, an then copy it to screen applying lens effect.
  unsigned char* scrmem = eye_lens_spare_screen_memory;
  // Store previous graphics settings
  unsigned char* wscr_cp = lbDisplay.WScreen;
  TbGraphicsWindow grwnd;
  LbScreenStoreGraphicsWindow(&grwnd);
  // Prepare new settings
  memset(scrmem, 0, eye_lens_width*eye_lens_height*sizeof(TbPixel));
  lbDisplay.WScreen = scrmem;
  lbDisplay.GraphicsScreenHeight = eye_lens_height;
  lbDisplay.GraphicsScreenWidth = eye_lens_width;
  LbScreenSetGraphicsWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  // Draw on our buffer
  setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
  engine(player, render_cam);
  // Restore original graphics settings
  lbDisplay.WScreen = wscr_cp;
  LbScreenLoadGraphicsWindow(&grwnd);
  // Draw the buffer on real screen
  setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
  draw_lens_effect(lbDisplay.WScreen, lbDisplay.GraphicsScreenWidth, scrmem, eye_lens_width,
      MyScreenWidth/pixel_size, MyScreenHeight/pixel_size, game.applied_lens_type);
}

struct Thing *get_creature_near_for_controlling(PlayerNumber plyr_idx, MapCoord x, MapCoord y)
{
    MapCoordDelta nearest_distance = INT32_MAX;
    struct Thing *nearest_thing = INVALID_THING;

    for (long k = 0; k < AROUND_TILES_COUNT; k++)
    {

        MapSubtlCoord stl_x = coord_subtile(x) + around[k].delta_x;
        MapSubtlCoord stl_y = coord_subtile(y) + around[k].delta_y;
        struct Map* mapblk = get_map_block_at(stl_x, stl_y);
        unsigned long j = 0;
        for (int i = get_mapwho_thing_index(mapblk); i != 0;)
        {
            struct Thing* thing = thing_get(i);
            i = thing->next_on_mapblk;


            if (can_cast_spell(plyr_idx,PwrK_POSSESS,stl_x,stl_y,thing,CastChk_Default ))
            {
                MapCoordDelta distance = chessboard_distance(thing->mappos.x.val, thing->mappos.y.val, x, y);
                if (distance < nearest_distance)
                {
                    nearest_distance = distance;
                    nearest_thing = thing;
                }
            }

            j++;
            if (j > THINGS_COUNT)
            {
                ERRORLOG("Infinite loop detected when sweeping things list");
                break_mapwho_infinite_chain(mapblk);
                break;
            }
        }
    }
    return nearest_thing;
}

/**
 * Puts a creature as first in a list of owning player creatures.
 *
 * This function is called at map loading, during creature creation; this means it cannot use get_players_num_dungeon().
 *
 * @param thing The creature thing.
 */
void set_first_creature(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);

    if ((creatng->alloc_flags & TAlF_InDungeonList) != 0) {
        ERRORLOG("Thing already in Peter list");
        return;
    }

    SYNCDBG(16,"Starting for %s index %d owner %d",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);

    if (is_neutral_thing(creatng))
    {
        if (game.nodungeon_creatr_list_start > 0)
        {
            struct Thing* prevtng = thing_get(game.nodungeon_creatr_list_start);
            struct CreatureControl* prevctrl = creature_control_get_from_thing(prevtng);
            prevctrl->players_prev_creature_idx = creatng->index;
            cctrl->players_next_creature_idx = game.nodungeon_creatr_list_start;
            cctrl->players_prev_creature_idx = 0;
            game.nodungeon_creatr_list_start = creatng->index;
        } else
        {
            cctrl->players_next_creature_idx = 0;
            cctrl->players_prev_creature_idx = 0;
            game.nodungeon_creatr_list_start = creatng->index;
        }
        creatng->alloc_flags |= TAlF_InDungeonList;
    } else
    if (!creature_is_for_dungeon_diggers_list(creatng))
    {
        struct Dungeon* dungeon = get_dungeon(creatng->owner);
        if (dungeon->creatr_list_start > 0)
        {
            struct Thing* prevtng = thing_get(dungeon->creatr_list_start);
            struct CreatureControl* prevctrl = creature_control_get_from_thing(prevtng);
            prevctrl->players_prev_creature_idx = creatng->index;
            cctrl->players_next_creature_idx = dungeon->creatr_list_start;
            cctrl->players_prev_creature_idx = 0;
            dungeon->creatr_list_start = creatng->index;
        } else
        {
            cctrl->players_next_creature_idx = 0;
            cctrl->players_prev_creature_idx = 0;
            dungeon->creatr_list_start = creatng->index;
        }
        if (!flag_is_set(cctrl->creature_state_flags,TF2_Spectator) && !(flag_is_set(cctrl->creature_state_flags, TF2_SummonedCreature)))
        {
            dungeon->owned_creatures_of_model[creatng->model]++;
            dungeon->num_active_creatrs++;
        }
        creatng->alloc_flags |= TAlF_InDungeonList;
    }
    else
    {
        struct Dungeon* dungeon = get_dungeon(creatng->owner);
        if (dungeon->digger_list_start > 0)
        {
            struct Thing* prevtng = thing_get(dungeon->digger_list_start);
            struct CreatureControl* prevctrl = creature_control_get_from_thing(prevtng);
            prevctrl->players_prev_creature_idx = creatng->index;
            cctrl->players_next_creature_idx = dungeon->digger_list_start;
            cctrl->players_prev_creature_idx = 0;
            dungeon->digger_list_start = creatng->index;
        }
        else
        {
            cctrl->players_next_creature_idx = 0;
            cctrl->players_prev_creature_idx = 0;
            dungeon->digger_list_start = creatng->index;
        }
        if (!flag_is_set(cctrl->creature_state_flags, TF2_Spectator) && !(flag_is_set(cctrl->creature_state_flags, TF2_SummonedCreature)))
        {
            dungeon->num_active_diggers++;
            dungeon->owned_creatures_of_model[creatng->model]++;
        }
        creatng->alloc_flags |= TAlF_InDungeonList;
    }
}

void recalculate_all_creature_digger_lists()
{
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if (plyr_idx == PLAYER_NEUTRAL)
            continue;
        recalculate_player_creature_digger_lists(plyr_idx);
    }

    for (long crtr_model = 0; crtr_model < game.conf.crtr_conf.model_count; crtr_model++)
    {
        struct CreatureModelConfig *crconf = creature_stats_get(crtr_model);
        if ((crconf->model_flags & (CMF_IsSpecDigger|CMF_IsDiggingCreature)) != 0)
        {
            crconf->evil_start_state = CrSt_ImpDoingNothing;
            crconf->good_start_state = CrSt_TunnellerDoingNothing;
        } else
        {
            crconf->evil_start_state = CrSt_CreatureDoingNothing;
            crconf->good_start_state = CrSt_GoodDoingNothing;
        }
    }
}

void recalculate_player_creature_digger_lists(PlayerNumber plr_idx)
{
    ThingIndex previous_digger = 0;
    ThingIndex previous_creature = 0;

    struct Dungeon* dungeon = get_dungeon(plr_idx);
    dungeon->digger_list_start = 0;
    dungeon->creatr_list_start = 0;
    dungeon->num_active_diggers = 0;
    dungeon->num_active_creatrs = 0;


    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    long i = slist->index;
    long k = 0;
    while (i > 0)
    {
        struct Thing* creatng = thing_get(i);
        if (thing_is_invalid(creatng))
          break;

        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        if (flag_is_set(cctrl->creature_state_flags, TF2_SummonedCreature))
        {
            i = creatng->next_of_class;
            k++;
            continue;
        }
        if(creatng->owner == plr_idx)
        {
            if(creature_is_for_dungeon_diggers_list(creatng))
            {

                if(dungeon->digger_list_start == 0)
                {
                    dungeon->digger_list_start = i;
                }
                else
                {
                    struct CreatureControl* prevcctrl = creature_control_get_from_thing(thing_get(previous_digger));
                    prevcctrl->players_next_creature_idx = i;
                }
                cctrl->players_next_creature_idx = 0;
                cctrl->players_prev_creature_idx = previous_digger;
                if (!flag_is_set(cctrl->creature_state_flags,TF2_Spectator) && !(flag_is_set(cctrl->creature_state_flags, TF2_SummonedCreature)))
                {
                    dungeon->num_active_diggers++;
                }
                previous_digger = i;
            }
            else
            {

                if(dungeon->creatr_list_start == 0)
                {
                    dungeon->creatr_list_start = i;
                }
                else
                {
                    struct CreatureControl* prevcctrl = creature_control_get_from_thing(thing_get(previous_creature));
                    prevcctrl->players_next_creature_idx = i;
                }
                cctrl->players_next_creature_idx = 0;
                cctrl->players_prev_creature_idx = previous_creature;
                if (!flag_is_set(cctrl->creature_state_flags,TF2_Spectator) && !(flag_is_set(cctrl->creature_state_flags, TF2_SummonedCreature)))
                {
                    dungeon->num_active_creatrs++;
                }
                previous_creature = i;
            }
        }

        i = creatng->next_of_class;
        k++;
        if (k > slist->count)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
}

void remove_first_creature(struct Thing *creatng)
{
    struct Dungeon *dungeon;
    struct CreatureControl *secctrl;
    struct Thing *sectng;
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if ((creatng->alloc_flags & TAlF_InDungeonList) == 0)
    {
        ERRORLOG("The %s index %d is not in Peter list",thing_model_name(creatng),(int)creatng->index);
        return;
    }
    if (is_neutral_thing(creatng))
    {
      sectng = thing_get(cctrl->players_prev_creature_idx);
      if (thing_exists(sectng)) {
          secctrl = creature_control_get_from_thing(sectng);
          secctrl->players_next_creature_idx = cctrl->players_next_creature_idx;
      } else {
          game.nodungeon_creatr_list_start = cctrl->players_next_creature_idx;
      }
      sectng = thing_get(cctrl->players_next_creature_idx);
      if (thing_exists(sectng)) {
          secctrl = creature_control_get_from_thing(sectng);
          secctrl->players_prev_creature_idx = cctrl->players_prev_creature_idx;
      }
    } else
    if (creature_is_for_dungeon_diggers_list(creatng))
    {
        dungeon = get_dungeon(creatng->owner);
        sectng = thing_get(cctrl->players_prev_creature_idx);
        if (thing_exists(sectng)) {
            secctrl = creature_control_get_from_thing(sectng);
            secctrl->players_next_creature_idx = cctrl->players_next_creature_idx;
        } else {
            dungeon->digger_list_start = cctrl->players_next_creature_idx;
        }
        sectng = thing_get(cctrl->players_next_creature_idx);
        if (thing_exists(sectng)) {
            secctrl = creature_control_get_from_thing(sectng);
            secctrl->players_prev_creature_idx = cctrl->players_prev_creature_idx;
        }
        if (!flag_is_set(cctrl->creature_state_flags, TF2_Spectator) && !flag_is_set(cctrl->creature_state_flags, TF2_SummonedCreature))
        {
            dungeon->owned_creatures_of_model[creatng->model]--;
            dungeon->num_active_diggers--;
        }
    } else
    {
        dungeon = get_dungeon(creatng->owner);
        sectng = thing_get(cctrl->players_prev_creature_idx);
        if (thing_exists(sectng)) {
            secctrl = creature_control_get_from_thing(sectng);
            secctrl->players_next_creature_idx = cctrl->players_next_creature_idx;
        } else {
            dungeon->creatr_list_start = cctrl->players_next_creature_idx;
        }
        sectng = thing_get(cctrl->players_next_creature_idx);
        if (thing_exists(sectng)) {
            secctrl = creature_control_get_from_thing(sectng);
            secctrl->players_prev_creature_idx = cctrl->players_prev_creature_idx;
        }
        if (!flag_is_set(cctrl->creature_state_flags, TF2_Spectator) && !flag_is_set(cctrl->creature_state_flags, TF2_SummonedCreature))
        {
            dungeon->owned_creatures_of_model[creatng->model]--;
            dungeon->num_active_creatrs--;
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
  return (thing->class_id == TCls_Creature);
}

TbBool thing_is_dead_creature(const struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return false;
  return (thing->class_id == TCls_DeadCreature);
}

/** Returns if a thing is digger creature.
 *
 * @param thing The thing to be checked.
 * @return True if the thing is creature and digger or special digger, false otherwise.
 */
TbBool thing_is_creature_digger(const struct Thing *thing)
{
  if (!thing_is_creature(thing))
    return false;
  return any_flag_is_set(get_creature_model_flags(thing),(CMF_IsSpecDigger|CMF_IsDiggingCreature));
}

/** Returns if a thing is special digger creature.
 *
 * @param thing The thing to be checked.
 * @return True if the thing is creature and special digger, false otherwise.
 */
TbBool thing_is_creature_special_digger(const struct Thing* thing)
{
    if (!thing_is_creature(thing))
        return false;
    return flag_is_set(get_creature_model_flags(thing),CMF_IsSpecDigger);
}

/** Returns if a thing the creature type set as spectator, normally the floating spirit.
  * @param thing The thing to be checked.
 * @return True if the thing is creature and listed as spectator , false otherwise.
 */
TbBool thing_is_creature_spectator(const struct Thing* thing)
{
    if (!thing_is_creature(thing))
        return false;

    ThingModel breed = game.conf.crtr_conf.spectator_breed;
    if (breed == 0)
    {
        WARNLOG("There is no spectator breed");
        breed = game.conf.crtr_conf.special_digger_good;
    }
    return (thing->model == breed);
}


void anger_set_creature_anger_all_types(struct Thing *thing, long new_value)
{
    if (creature_can_get_angry(thing))
    {
      for (AnnoyMotive anger_type = 1; anger_type < AngR_ListEnd; anger_type++ )
      {
        anger_set_creature_anger(thing, new_value, anger_type);
      }
    }
}

struct Room *get_creature_lair_room(const struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->lairtng_idx <= 0) {
        return INVALID_ROOM;
    }
    return room_get(cctrl->lair_room_id);
}

TbBool creature_has_lair_room(const struct Thing *creatng)
{
    struct Room* room = get_creature_lair_room(creatng);
    if (!room_is_invalid(room) && room_role_matches(room->kind,get_room_role_for_job(Job_TAKE_SLEEP))) {
        return true;
    }
    return false;
}

TbBool remove_creature_lair(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (cctrl->lairtng_idx <= 0) {
        return false;
    }
    struct Room* room = room_get(cctrl->lair_room_id);
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
    struct CreatureControl* cctrl;
    SYNCDBG(6,"Starting for %s, owner %d to %d",thing_model_name(creatng),(int)creatng->owner,(int)nowner);
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
        cctrl = creature_control_get_from_thing(creatng);
        cctrl->paydays_owed = 0;
        cctrl->paydays_advanced = 0;
        cctrl->idle.start_gameturn = game.play_gameturn;
    }
}

/**
 * If the total creature count is low enough for a creature to be generated
 * @param temp_creature when set to 1, it will still generate if it would mean going 1 over a temporary limit
  * @return true if a creature may be generated, false if not.
 */
TbBool creature_count_below_map_limit(TbBool temp_creature)
{
    if (game.thing_lists[TngList_Creatures].count >= CREATURES_COUNT-1)
        return false;

    return ((game.thing_lists[TngList_Creatures].count - temp_creature) < game.conf.rules[0].game.creatures_count);
}

struct Thing *create_creature(struct Coord3d *pos, ThingModel model, PlayerNumber owner)
{
    struct CreatureModelConfig *crconf = creature_stats_get(model);
    if (game.thing_lists[TngList_Creatures].count >= CREATURES_COUNT)
    {
        ERRORLOG("Cannot create %s for player %d. Creature limit %d reached.", creature_code_name(model), (int)owner, CREATURES_COUNT);
        return INVALID_THING;
    }
    if (!i_can_allocate_free_thing_structure(TCls_Creature))
    {
        ERRORDBG(3, "Cannot create %s for player %d. There are too many things allocated.", creature_code_name(model), (int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    if (!i_can_allocate_free_control_structure())
    {
        ERRORDBG(3, "Cannot create %s for player %d. There are too many creatures allocated.", creature_code_name(model), (int)owner);
        erstat_inc(ESE_NoFreeCreatrs);
        return INVALID_THING;
    }
    struct Thing *crtng = allocate_free_thing_structure(TCls_Creature);
    if (crtng->index == 0)
    {
        ERRORDBG(3, "Should be able to allocate %s for player %d, but failed.", creature_code_name(model), (int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    struct CreatureControl *cctrl = allocate_free_control_structure();
    crtng->ccontrol_idx = cctrl->index;
    crtng->class_id = TCls_Creature;
    crtng->model = model;
    crtng->parent_idx = crtng->index;
    crtng->mappos.x.val = pos->x.val;
    crtng->mappos.y.val = pos->y.val;
    crtng->mappos.z.val = pos->z.val;
    crtng->clipbox_size_xy = crconf->size_xy;
    crtng->clipbox_size_z = crconf->size_z;
    crtng->solid_size_xy = crconf->thing_size_xy;
    crtng->solid_size_z = crconf->thing_size_z;
    crtng->fall_acceleration = 32;
    crtng->bounce_angle = 0;
    crtng->inertia_floor = 32;
    crtng->inertia_air = 8;
    crtng->movement_flags |= TMvF_ZeroVerticalVelocity;
    crtng->owner = owner;
    crtng->move_angle_xy = ANGLE_NORTH;
    crtng->move_angle_z = 0;
    cctrl->max_speed = calculate_correct_creature_maxspeed(crtng);
    cctrl->shot_shift_x = crconf->shot_shift_x;
    cctrl->shot_shift_y = crconf->shot_shift_y;
    cctrl->shot_shift_z = crconf->shot_shift_z;
    long i = get_creature_anim(crtng, CGI_Stand);
    set_thing_draw(crtng, i, crconf->walking_anim_speed, game.conf.crtr_conf.sprite_size, 0, 0, ODC_Default);
    cctrl->exp_level = 1;
    cctrl->max_health = calculate_correct_creature_max_health(crtng);
    crtng->health = cctrl->max_health;
    crtng->owner = owner;
    crtng->mappos.x.val = pos->x.val;
    crtng->mappos.y.val = pos->y.val;
    crtng->mappos.z.val = pos->z.val;
    crtng->creation_turn = game.play_gameturn;
    cctrl->joining_age = 17 + THING_RANDOM(crtng, 13);
    cctrl->blood_type = THING_RANDOM(crtng, BLOOD_TYPES_COUNT);
    if (player_is_roaming(owner))
    {
        cctrl->hero.hero_state = -1;
        cctrl->hero.ready_for_attack_flag = 1;
    }
    cctrl->flee_pos.x.val = crtng->mappos.x.val;
    cctrl->flee_pos.y.val = crtng->mappos.y.val;
    cctrl->flee_pos.z.val = crtng->mappos.z.val;
    cctrl->flee_pos.z.val = get_thing_height_at(crtng, pos);
    cctrl->fighting_player_idx = -1;
    if (crconf->flying)
    {
        crtng->movement_flags |= TMvF_Flying;
    }
    set_creature_level(crtng, 0);
    crtng->health = cctrl->max_health;
    add_thing_to_its_class_list(crtng);
    place_thing_in_mapwho(crtng);
    if (owner <= PLAYERS_COUNT)
    {
        set_first_creature(crtng);
    }
    set_start_state(crtng);
    add_creature_score_to_owner(crtng);
    cctrl->active_instance_id = creature_choose_first_available_instance(crtng);
    if (crconf->illuminated)
    {
        illuminate_creature(crtng);
    }
    return crtng;
}

TbBool creature_increase_level(struct Thing *thing)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return false;
    }
    struct Dungeon *dungeon = get_dungeon(thing->owner);
    if (dungeon->creature_max_level[thing->model] > cctrl->exp_level)
    {
        struct CreatureModelConfig *crconf = creature_stats_get_from_thing(thing);
        if ((cctrl->exp_level < CREATURE_MAX_LEVEL - 1) || (crconf->grow_up != 0))
        {
            cctrl->exp_level_up = true;
            return true;
        }
    }
    return false;
}

TbBool creature_change_multiple_levels(struct Thing *thing, int count)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return false;
    }
    struct Dungeon* dungeon = get_dungeon(thing->owner);
    int k = 0;
    if (count > 0)
    {
        for (int i = 0; i < count; i++)
        {
            if (dungeon->creature_max_level[thing->model] > cctrl->exp_level)
            {
                struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
                if ((cctrl->exp_level < CREATURE_MAX_LEVEL - 1) || (crconf->grow_up != 0))
                {
                    cctrl->exp_level_up = true;
                    update_creature_levels(thing);
                    k++;
                }
            }
        }
        if (k != 0)
        {
            return true;
        }
        return false;
    }
    else
    {
        remove_creature_score_from_owner(thing);
        if (cctrl->exp_level < abs(count))
        {
            set_creature_level(thing, 0);
        }
        else
        {
            set_creature_level(thing, cctrl->exp_level + count);
        }
        return true;
    }
}

/**
 * Creates a special digger specific to given player and owned by that player.
 * @param x
 * @param y
 * @param owner
 * @return
 */
struct Thing *create_owned_special_digger(MapCoord x, MapCoord y, PlayerNumber owner)
{
    ThingModel crmodel = get_players_special_digger_model(owner);
    struct Coord3d pos;
    pos.x.val = x;
    pos.y.val = y;
    pos.z.val = 0;
    struct Thing* thing = create_creature(&pos, crmodel, owner);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Cannot create creature %s at (%d,%d)",creature_code_name(crmodel),x,y);
        return INVALID_THING;
    }
    pos.z.val = get_thing_height_at(thing, &pos);
    if (thing_in_wall_at(thing, &pos))
    {
        delete_thing_structure(thing, 0);
        ERRORLOG("Creature %s at (%d,%d) deleted because is in wall",creature_code_name(crmodel),x,y);
        return INVALID_THING;
    }
    thing->mappos.x.val = pos.x.val;
    thing->mappos.y.val = pos.y.val;
    thing->mappos.z.val = pos.z.val;
    remove_first_creature(thing);
    set_first_creature(thing);
    return thing;
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
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    struct SpellConfig *spconf = get_spell_config(param->primary_number);
    if ((cctrl->combat_flags != 0) && !creature_is_being_unconscious(thing))
    {
        if ((param->plyr_idx >= 0) && (thing->owner != param->plyr_idx))
        {
            return -1;
        }
        if (!thing_matches_model(thing, param->model_id))
        {
            return -1;
        }
        if ((param->class_id > 0) && (thing->class_id != param->class_id))
        {
            return -1;
        }
        if ((spconf->spell_flags != 0) && (creature_under_spell_effect(thing, spconf->spell_flags)))
        {
            return -1;
        }
        if (creature_is_immune_to_spell_effect(thing, spconf->spell_flags))
        {
            return -1;
        }
        return get_creature_thing_score(thing);
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Filter function for selecting creature which is dragging a specific thing.
 * A specific thing is selected by index.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with specific thing which is dragged.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_dragging_specific_thing(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (param->primary_number > 0)
    {
        if (cctrl->dragtng_idx == param->primary_number) {
            return INT32_MAX;
        }
        return -1;
    }
    ERRORLOG("No thing index to find dragging creature with");
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    CrtrExpLevel nmaxim = cctrl->exp_level + 1;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
        && (thing->class_id == param->class_id)
        && ((thing_matches_model(thing,param->model_id)))
        && ((param->primary_number == -1) || (get_creature_gui_job(thing) == param->primary_number))
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    CrtrExpLevel nmaxim = cctrl->exp_level + 1;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
        && (thing->class_id == param->class_id)
        && ((thing_matches_model(thing,param->model_id)))
        && ((param->primary_number == -1) || (get_creature_gui_job(thing) == param->primary_number))
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    CrtrExpLevel nmaxim = cctrl->exp_level + 1;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
        && (thing->class_id == param->class_id)
        && ((thing_matches_model(thing,param->model_id)))
        && ((param->primary_number == -1) || (get_creature_gui_job(thing) == param->primary_number))
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    CrtrExpLevel nmaxim = CREATURE_MAX_LEVEL - cctrl->exp_level;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((thing_matches_model(thing,param->model_id)))
      && ((param->primary_number == -1) || (get_creature_gui_job(thing) == param->primary_number))
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    CrtrExpLevel nmaxim = CREATURE_MAX_LEVEL - cctrl->exp_level;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((thing_matches_model(thing,param->model_id)))
      && ((param->primary_number == -1) || (get_creature_gui_job(thing) == param->primary_number))
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    // New 'maximizer' value. Should be at least 1; maximum is, in this case, CREATURE_MAX_LEVEL.
    CrtrExpLevel nmaxim = CREATURE_MAX_LEVEL - cctrl->exp_level;
    if ( ((param->plyr_idx == -1) || (thing->owner == param->plyr_idx))
      && (thing->class_id == param->class_id)
      && ((thing_matches_model(thing,param->model_id)))
      && ((param->primary_number == -1) || (get_creature_gui_job(thing) == param->primary_number))
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
      && ((thing_matches_model(thing,param->model_id)))
      && ((param->primary_number == -1) || (get_creature_gui_job(thing) == param->primary_number))) // job_idx
    {
        // New 'maximizer' equal to MAX_LONG will stop the sweeping
        // and return this thing immediately.
        return INT32_MAX;
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
      && ((thing_matches_model(thing,param->model_id)))
      && !thing_is_picked_up(thing)
      && ((param->primary_number == -1) || (get_creature_gui_job(thing) == param->primary_number)) // job_idx
      && (thing->active_state != CrSt_CreatureUnconscious) )
    {
      if (can_thing_be_picked_up_by_player(thing, param->plyr_idx))
      {
          // New 'maximizer' equal to MAX_LONG will stop the sweeping
          // and return this thing immediately.
          return INT32_MAX;
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
      && ((thing_matches_model(thing,param->model_id)))
      && !thing_is_picked_up(thing)
      && ((param->primary_number == -1) || (get_creature_gui_job(thing) == param->primary_number))
      && (thing->active_state != CrSt_CreatureUnconscious) )
    {
      if (can_thing_be_picked_up2_by_player(thing, param->plyr_idx))
      {
          // New 'maximizer' equal to MAX_LONG will stop the sweeping
          // and return this thing immediately.
          return INT32_MAX;
      }
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

/**
 * Returns a creature in fight which gives highest score value.
 * @return The thing in fight, or invalid thing if not found.
 */
struct Thing *find_players_highest_score_creature_in_fight_not_affected_by_spell(PlayerNumber plyr_idx, SpellKind spell_kind)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    struct CompoundTngFilterParam param;
    param.plyr_idx = -1;
    param.class_id = 0;
    param.model_id = CREATURE_ANY;
    param.primary_number = spell_kind;
    Thing_Maximizer_Filter filter = player_list_creature_filter_in_fight_and_not_affected_by_spell;
    struct Thing* creatng = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    if (thing_is_invalid(creatng)) {
        creatng = get_player_list_creature_with_filter(dungeon->digger_list_start, filter, &param);
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
    SYNCDBG(19,"Starting");
    Thing_Maximizer_Filter filter = player_list_creature_filter_dragging_specific_thing;
    struct CompoundTngFilterParam param;
    param.class_id = TCls_Creature;
    param.model_id = CREATURE_ANY;
    param.plyr_idx = -1;
    param.primary_number = dragtng->index;
    param.secondary_number = -1;
    param.tertiary_number = -1;
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
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    struct CompoundTngFilterParam param;
    param.plyr_idx = plyr_idx;
    param.class_id = TCls_Creature;
    param.model_id = crmodel;
    param.primary_number = job_idx;
    switch (pick_check)
    {
    default:
        WARNLOG("Invalid check selection, %d",(int)pick_check);
        // fall through
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
    TbBool is_spec_digger = (crmodel > 0) && creature_kind_is_for_dungeon_diggers_list(plyr_idx, crmodel);
    struct Thing* thing = INVALID_THING;
    if ((!is_spec_digger) || (crmodel == CREATURE_ANY))
    {
        thing = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    if (((is_spec_digger) || (crmodel == CREATURE_ANY)) && thing_is_invalid(thing))
    {
        thing = get_player_list_creature_with_filter(dungeon->digger_list_start, filter, &param);
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
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    struct CompoundTngFilterParam param;
    param.plyr_idx = plyr_idx;
    param.class_id = TCls_Creature;
    param.model_id = crmodel;
    param.primary_number = job_idx;
    switch (pick_check)
    {
    default:
        WARNLOG("Invalid check selection, %d",(int)pick_check);
        // fall through
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
    TbBool is_spec_digger = (crmodel > 0) && creature_kind_is_for_dungeon_diggers_list(plyr_idx, crmodel);
    struct Thing* thing = INVALID_THING;
    if ((!is_spec_digger) || (crmodel == CREATURE_ANY) || (crmodel == CREATURE_NOT_A_DIGGER))
    {
        if (!player_is_neutral(plyr_idx))
        {
            thing = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
        }
        else
        {
            thing = get_player_list_creature_with_filter(game.nodungeon_creatr_list_start, filter, &param);
        }
    }
    if (((is_spec_digger) || (crmodel == CREATURE_ANY) || (crmodel == CREATURE_DIGGER)) && thing_is_invalid(thing))
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
    SYNCDBG(5,"Searching for model %d, GUI job %d",(int)crmodel,(int)job_idx);
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    struct CompoundTngFilterParam param;
    param.plyr_idx = plyr_idx;
    param.class_id = TCls_Creature;
    param.model_id = crmodel;
    param.primary_number = job_idx;
    switch (pick_check)
    {
    default:
        WARNLOG("Invalid check selection, %d",(int)pick_check);
        // fall through
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
    TbBool is_spec_digger = (crmodel > 0) && creature_kind_is_for_dungeon_diggers_list(plyr_idx, crmodel);
    struct Thing* thing = INVALID_THING;
    if ((!is_spec_digger) || (crmodel == CREATURE_ANY))
    {
        thing = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    if (((is_spec_digger) || (crmodel == CREATURE_ANY) || (crmodel == CREATURE_DIGGER)) && thing_is_invalid(thing))
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
    SYNCDBG(5,"Searching for model %d, GUI job %d",(int)crmodel,(int)job_idx);
    struct Thing* thing = INVALID_THING;
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    /* Check if we should start the search with a creature after last one, not from start of the list */
    if ((pick_flags & TPF_OrderedPick) == 0)
    {
        long i;
        if (crmodel != CREATURE_ANY)
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
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
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
        struct CompoundTngFilterParam param;
        param.plyr_idx = plyr_idx;
        param.class_id = TCls_Creature;
        param.model_id = crmodel;
        param.primary_number = job_idx;
        Thing_Maximizer_Filter filter;
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
    if (crmodel != CREATURE_ANY)
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
    struct Thing* thing = find_players_next_creature_of_breed_and_gui_job(crmodel, job_idx, plyr_idx, pick_flags);
    if (thing_is_invalid(thing))
    {
        SYNCDBG(2,"Can't find creature of model %d and GUI job %d.",(int)crmodel,(int)job_idx);
        return INVALID_THING;
    }
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (crmodel < game.conf.crtr_conf.model_count)
    {
        if ((job_idx == -1) || (((job_idx & 0x03) <= 2) && dungeon->guijob_all_creatrs_count[crmodel][job_idx & 0x03]))
        {
            set_players_packet_action(get_player(plyr_idx), PckA_UsePwrHandPick, thing->index, 0, 0, 0);
        }
    } else
    if (crmodel == CREATURE_ANY)
    {
        set_players_packet_action(get_player(plyr_idx), PckA_UsePwrHandPick, thing->index, 0, 0, 0);
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
  * @param pick_flags
 * @note originally was go_to_next_creature_of_breed_and_job()
 */
void go_to_next_creature_of_model_and_gui_job(long crmodel, long job_idx, unsigned char pick_flags)
{
    struct Thing* creatng = find_players_next_creature_of_breed_and_gui_job(crmodel, job_idx, my_player_number, pick_flags);
    if (!thing_is_invalid(creatng))
    {
        struct PlayerInfo* player = get_my_player();
        set_players_packet_action(player, PckA_ZoomToPosition, creatng->mappos.x.val, creatng->mappos.y.val, 0, 0);
    }
}

TbBool creature_is_doing_job_in_room_role(const struct Thing *creatng, RoomRole rrole)
{
    {
        // Check if we're just starting a job related to that room
        CrtrStateId pvstate = get_creature_state_besides_interruptions(creatng);
        CrtrStateId nxstate = get_initial_state_for_job(get_job_for_room_role(rrole, JoKF_None, Job_NULL));
        if ((pvstate != CrSt_Unused) && (pvstate == nxstate)) {
            return true;
        }
    }
    {
        // Check if we're already working in that room kind
        struct Room* room = get_room_creature_works_in(creatng);
        if (!room_is_invalid(room)) {
            return ((get_room_roles(room->kind) & rrole) != 0);
        }
    }
    return false;
}

long player_list_creature_filter_needs_to_be_placed_in_room_for_job(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    SYNCDBG(19,"Starting for %s index %d owner %d",thing_model_name(thing),(int)thing->index,(int)thing->owner);
    struct Computer2* comp = (struct Computer2*)(param->primary_pointer);
    struct Dungeon* dungeon = comp->dungeon;
    if (!can_thing_be_picked_up_by_player(thing, dungeon->owner)) {
        return -1;
    }
    if (creature_is_being_dropped(thing)) {
        return -1;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);

    // If the creature is too angry to help it
    if (creature_is_doing_anger_job(thing) || anger_is_creature_livid(thing))
    {
        // If the creature is not running free, then leave it where it is
        if (creature_is_kept_in_prison(thing) ||
            creature_is_being_tortured(thing) ||
            creature_is_being_sacrificed(thing)) {
            return -1;
        }
        // Try torturing it
        if (player_has_room_of_role(dungeon->owner, get_room_role_for_job(Job_PAINFUL_TORTURE)))
        {
            param->secondary_number = Job_PAINFUL_TORTURE;
            return INT32_MAX;
        }
        // Or putting in prison
        if (player_has_room_of_role(dungeon->owner, get_room_role_for_job(Job_CAPTIVITY)))
        {
            param->secondary_number = Job_CAPTIVITY;
            return INT32_MAX;
        }
        // If we can't, then just let it leave the dungeon
        if (player_has_room_of_role(dungeon->owner, get_room_role_for_job(Job_EXEMPT)))
        {
            param->secondary_number = Job_EXEMPT;
            return INT32_MAX;
        }
    }

    HitPoints health_permil = get_creature_health_permil(thing);
    // If it's angry but not furious, or has lost health due to disease, then should be placed in temple.
    if ((anger_is_creature_angry(thing)
    || (creature_under_spell_effect(thing, CSAfF_Disease) && (health_permil <= (game.conf.rules[thing->owner].computer.disease_to_temple_pct * 10))))
    && creature_can_do_job_for_player(thing, dungeon->owner, Job_TEMPLE_PRAY, JobChk_None))
    {
        // If already at temple, then don't do anything
        if (creature_is_doing_temple_pray_activity(thing))
            return -1;
        if (player_has_room_of_role(dungeon->owner, get_room_role_for_job(Job_TEMPLE_PRAY)))
        {
            param->secondary_number = Job_TEMPLE_PRAY;
            return INT32_MAX;
        }
    }

    // If the creature require healing, then drop it to lair. When in combat, try to cast heal first.
    if (cctrl->combat_flags)
    {
        // Simplified algorithm when creature is in combat
        if (creature_requires_healing(thing))
        {
            // If already at lair, then don't do anything
            if (!creature_is_doing_lair_activity(thing))
            {
                // cast heal if we can, don't always use max level to appear lifelike
                CrtrExpLevel spell_level = PLAYER_RANDOM(dungeon->owner, 4) + 5;
                if (computer_able_to_use_power(comp, PwrK_HEALCRTR, spell_level, 1))
                {
                    if (try_game_action(comp, dungeon->owner, GA_UsePwrHealCrtr, spell_level, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->index, 1) > Lb_OK)
                    {
                        return INT32_MAX;
                    } else
                    {
                        return -1;
                    }
                } else
                // otherwise, put it into room we want
                {
                    if (creature_can_do_healing_sleep(thing))
                    {
                        if (player_has_room_of_role(dungeon->owner, get_room_role_for_job(Job_TAKE_SLEEP)))
                        {
                            param->secondary_number = Job_TAKE_SLEEP;
                            return INT32_MAX;
                        }
                    }
                }
            }
        }
        return -1;
    } else
    {
        if (creature_can_do_healing_sleep(thing))
        {
            // Be more careful when not in combat
            if ((health_permil < 1000*crconf->heal_threshold/256) || !creature_has_lair_room(thing))
            {
                // If already at lair, then don't do anything
                if (creature_is_doing_lair_activity(thing))
                    return -1;
                // don't force it to lair if it wants to eat or take salary
                if (creature_is_doing_garden_activity(thing) || creature_is_taking_salary_activity(thing))
                    return -1;
                // otherwise, put it into room we want
                if (player_has_room_of_role(dungeon->owner, get_room_role_for_job(Job_TAKE_SLEEP)))
                {
                    param->secondary_number = Job_TAKE_SLEEP;
                    return INT32_MAX;
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
        if (player_has_room_of_role(dungeon->owner, get_room_role_for_job(Job_TAKE_FEED)))
        {
            param->secondary_number = Job_TAKE_FEED;
            return INT32_MAX;
        }
    }

    // If creature wants salary, let it go get the gold
    if ( cctrl->paydays_owed )
    {
        // If already taking salary, then don't do anything
        if (creature_is_taking_salary_activity(thing))
            return -1;
        if (player_has_room_of_role(dungeon->owner, get_room_role_for_job(Job_TAKE_SALARY)))
        {
            param->secondary_number = Job_TAKE_SALARY;
            return INT32_MAX;
        }
    }

    TbBool force_state_reset = false;
    // Creatures may have primary jobs other than training, or selected when there was no possibility to train
    // Make sure they are re-assigned sometimes
    if (creature_could_be_placed_in_better_room(comp, thing))
    {
        force_state_reset = true;
    }

    // Get other rooms the creature may work in
    if (creature_state_is_unset(thing) || force_state_reset)
    {
        CreatureJob new_job = get_job_to_place_creature_in_room(comp, thing);
        // Make sure the place we've selected is not the same as the one creature works in now
        if (!creature_is_doing_job_in_room_role(thing, get_room_role_for_job(new_job)))
        {
            param->secondary_number = new_job;
            return INT32_MAX;
        }
    }
    return -1;
}

struct Thing *create_footprint_sine(struct Coord3d *crtr_pos, unsigned short phase, short nfoot, unsigned short model, unsigned short owner)
{
  struct Coord3d pos;
  pos.x.val = crtr_pos->x.val;
  pos.y.val = crtr_pos->y.val;
  pos.z.val = crtr_pos->z.val;
  unsigned int i;
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return;
    }
    short nfoot = get_foot_creature_has_down(thing);
    struct Thing* footng = create_footprint_sine(&thing->mappos, thing->move_angle_xy, nfoot, TngEffElm_Blood4, thing->owner);
    if (!thing_is_invalid(footng))
    {
        cctrl->bloody_footsteps_turns--;
    }
}

TbBool update_controlled_creature_movement(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    TbBool upd_done = false;
    if ((thing->movement_flags & TMvF_Flying) != 0)
    {
        if (cctrl->move_speed != 0)
        {
            cctrl->moveaccel.x.val = distance3d_with_angles_to_coord_x(cctrl->move_speed, thing->move_angle_xy, thing->move_angle_z);
            cctrl->moveaccel.y.val = distance3d_with_angles_to_coord_y(cctrl->move_speed, thing->move_angle_xy, thing->move_angle_z);
            if (cctrl->vertical_speed == 0)
            {
                cctrl->moveaccel.z.val = distance_with_angle_to_coord_z(cctrl->move_speed, thing->move_angle_z);
            }
        }
        if (cctrl->orthogn_speed != 0)
        {
            cctrl->moveaccel.x.val += distance_with_angle_to_coord_x(cctrl->orthogn_speed, thing->move_angle_xy - DEGREES_90);
            cctrl->moveaccel.y.val += distance_with_angle_to_coord_y(cctrl->orthogn_speed, thing->move_angle_xy - DEGREES_90);
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
            cctrl->moveaccel.x.val += distance_with_angle_to_coord_x(cctrl->orthogn_speed, thing->move_angle_xy - DEGREES_90);
            cctrl->moveaccel.y.val += distance_with_angle_to_coord_y(cctrl->orthogn_speed, thing->move_angle_xy - DEGREES_90);
            upd_done = true;
        }
    }
    return upd_done;
}

TbBool update_flight_altitude_towards_typical(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct Coord3d nxpos;
    nxpos.x.val = thing->mappos.x.val + cctrl->moveaccel.x.val;
    nxpos.y.val = thing->mappos.y.val + cctrl->moveaccel.y.val;
    nxpos.z.val = subtile_coord(1,0);
    MapCoord floor_height, ceiling_height;
    get_floor_and_ceiling_height_under_thing_at(thing, &nxpos, &floor_height, &ceiling_height);
    MapCoordDelta thing_curr_alt = thing->mappos.z.val;
    SYNCDBG(16,"The height for %s index %d owner %d must fit between %d and %d, now is %d",thing_model_name(thing),(int)thing->index,(int)thing->owner,(int)floor_height,(int)ceiling_height,(int)thing_curr_alt);
    MoveSpeed max_speed = cctrl->max_speed / 8;
    if (max_speed < 1)
        max_speed = 1;
    MapCoordDelta i = floor_height + NORMAL_FLYING_ALTITUDE;
    MapCoordDelta max_pos_to_ceiling = ceiling_height - thing->clipbox_size_z;
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
    SYNCDBG(18,"Starting");
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return false;
    }
    short upd_done = 0;
    if (cctrl->stateblock_flags != 0)
    {
        upd_done = 1;
        cctrl->moveaccel.x.val = 0;
        cctrl->moveaccel.y.val = 0;
        cctrl->moveaccel.z.val = 0;
        cctrl->move_speed = 0;
        cctrl->creature_state_flags &= ~TF2_CreatureIsMoving;
    } else
    {
      if ((thing->alloc_flags & TAlF_IsControlled) != 0)
      {
          if (update_controlled_creature_movement(thing)) {
              upd_done = 1;
          }
      } else
      if ((cctrl->creature_state_flags & TF2_CreatureIsMoving) != 0)
      {
          upd_done = 1;
          cctrl->creature_state_flags &= ~TF2_CreatureIsMoving;
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
    if (((thing->alloc_flags & TAlF_IsControlled) == 0) && ((thing->movement_flags & TMvF_IsOnLava) != 0))
    {
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
        if (crconf->hurt_by_lava > 0)
        {
            struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
            if ((!creature_is_escaping_death(thing)) && (cctrl->lava_escape_since + 64 < game.play_gameturn))
            {
                cctrl->lava_escape_since = game.play_gameturn;
                if (cleanup_current_thing_state(thing))
                {
                    if (setup_move_off_lava(thing))
                    {
                        thing->continue_state = CrSt_CreatureEscapingDeath;
                    }
                    else
                    {
                        set_start_state(thing);
                    }
                }
            }
      }
    }
}

/**
 * Get's an effect element for a footstep.
 */
ThingModel get_footstep_effect_element(struct Thing* thing)
{
    static const unsigned char tileset_footstep_textures[TEXTURE_VARIATIONS_COUNT] =
    { TngEffElm_None,       TngEffElm_None,         TngEffElm_IceMelt3,     TngEffElm_None,
      TngEffElm_None,       TngEffElm_None,         TngEffElm_None,         TngEffElm_None,
      TngEffElm_StepSand,   TngEffElm_StepGypsum,   TngEffElm_None,         TngEffElm_None,
      TngEffElm_None,       TngEffElm_None,         TngEffElm_None,         TngEffElm_None
    };

    short texture;
        unsigned char ext_txtr = game.slab_ext_data[get_slab_number(subtile_slab(thing->mappos.x.stl.num), subtile_slab(thing->mappos.y.stl.num))];
    if (ext_txtr == 0)
    {
        // Default map texture
        texture = game.texture_id;
    }
    else
    {
        // Slab specific texture
        texture = ext_txtr - 1;
    }

    return tileset_footstep_textures[texture];
}

void process_creature_leave_footsteps(struct Thing *thing)
{
    struct Thing *footng;
    short nfoot;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (flag_is_set(thing->movement_flags,TMvF_IsOnWater))
    {
        nfoot = get_foot_creature_has_down(thing);
        if (nfoot)
        {
          create_effect(&thing->mappos, TngEff_Drip1, thing->owner);
        }
        cctrl->bloody_footsteps_turns = 0;
    } else
    // Bloody footprints
    if (cctrl->bloody_footsteps_turns != 0)
    {
        place_bloody_footprint(thing);
        nfoot = get_foot_creature_has_down(thing);
        footng = create_footprint_sine(&thing->mappos, thing->move_angle_xy, nfoot, TngEffElm_Blood4, thing->owner);
        if (!thing_is_invalid(footng)) {
            cctrl->bloody_footsteps_turns--;
        }
    } else
    {
        // Tileset footprints, formerly Snow footprints.
        ThingModel footprint = get_footstep_effect_element(thing);
        if (footprint != TngEffElm_None)
        {
            struct SlabMap* slb = get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
            if (slb->kind == SlbT_PATH)
            {
                set_flag(thing->movement_flags,TMvF_IsOnSnow);
                nfoot = get_foot_creature_has_down(thing);
                footng = create_footprint_sine(&thing->mappos, thing->move_angle_xy, nfoot, footprint, thing->owner);
            }
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
 * @param inflicting_plyr_idx
 */
HitPoints apply_damage_to_thing_and_display_health(struct Thing *thing, HitPoints dmg, PlayerNumber inflicting_plyr_idx)
{
    HitPoints cdamage;
    if (dmg > 0)
    {
        cdamage = apply_damage_to_thing(thing, dmg, inflicting_plyr_idx);
    } else {
        cdamage = 0;
    }
    if (cdamage > 0) {
        thing->creature.health_bar_turns = 8;
    }
    return cdamage;
}

void process_magic_fall_effect(struct Thing *thing)
{
    if (flag_is_set(thing->movement_flags, TMvF_MagicFall))
    {
        GameTurnDelta dturn = game.play_gameturn - thing->creation_turn;
        if ((dturn & 1) == 0)
        {
            create_effect_element(&thing->mappos, birth_effect_element[get_player_color_idx(thing->owner)], thing->owner);
        }
        struct CreatureModelConfig *crconf = creature_stats_get_from_thing(thing);
        creature_turn_to_face_angle(thing, thing->move_angle_xy + crconf->max_turning_speed);
        if ((dturn > 32) || thing_touching_floor(thing))
        {
            clear_flag(thing->movement_flags, TMvF_MagicFall);
        }
    }
}

void process_landscape_affecting_creature(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    thing->movement_flags &= ~TMvF_IsOnWater;
    thing->movement_flags &= ~TMvF_IsOnLava;
    thing->movement_flags &= ~TMvF_IsOnSnow;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        ERRORLOG("Invalid creature control; no action");
        return;
    }
    cctrl->corpse_to_piss_on = 0;

    int stl_idx = get_subtile_number(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    unsigned long navheight = get_navigation_map_floor_height(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    if (subtile_coord(navheight,0) == thing->mappos.z.val)
    {
        int i = get_top_cube_at_pos(stl_idx);
        if (cube_is_lava(i))
        {
            struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
            apply_damage_to_thing_and_display_health(thing, crconf->hurt_by_lava, -1);
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
    if (is_neutral_thing(thing))
        return false;
    struct Dungeon* dungeon = get_dungeon(thing->owner);
    if (dungeon_invalid(dungeon))
        return false;
    long score = get_creature_thing_score(thing);
    if (dungeon->score < INT32_MAX-score)
        dungeon->score += score;
    else
        dungeon->score = INT32_MAX;
    return true;
}

TbBool remove_creature_score_from_owner(struct Thing *thing)
{
    if (is_neutral_thing(thing))
        return false;
    struct Dungeon* dungeon = get_dungeon(thing->owner);
    if (dungeon_invalid(dungeon))
        return false;
    long score = get_creature_thing_score(thing);
    if (dungeon->score >= score)
        dungeon->score -= score;
    else
        dungeon->score = 0;
    return true;
}

void init_creature_scores(void)
{
    SYNCDBG(8, "Starting");
    long i;
    long score;
    // compute maximum score
    long max_score = 0;
    for (i=0; i < game.conf.crtr_conf.model_count; i++)
    {
        score = compute_creature_kind_score(i,CREATURE_MAX_LEVEL-1);
        if ((score <= 0) && (i != 0) && (i != game.conf.crtr_conf.model_count -1))
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
    for (i=0; i < game.conf.crtr_conf.model_count; i++)
    {
        for (CrtrExpLevel k = 0; k < CREATURE_MAX_LEVEL; k++)
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    ThingModel crmodel = thing->model;
    if (crmodel >= game.conf.crtr_conf.model_count)
        crmodel = 0;
    if (crmodel < 0)
        crmodel = 0;
    CrtrExpLevel exp = cctrl->exp_level;
    if (exp >= CREATURE_MAX_LEVEL)
        exp = CREATURE_MAX_LEVEL - 1;
    return game.creature_scores[crmodel].value[exp];
}

void transfer_creature_data_and_gold(struct Thing *oldtng, struct Thing *newtng)
{
    struct CreatureControl* oldcctrl = creature_control_get_from_thing(oldtng);
    struct CreatureControl* newcctrl = creature_control_get_from_thing(newtng);
    struct CreatureModelConfig* ncrconf = creature_stats_get_from_thing(newtng);

    strcpy(newcctrl->creature_name, oldcctrl->creature_name);
    newcctrl->blood_type = oldcctrl->blood_type;
    newcctrl->kills_num = oldcctrl->kills_num;
    newcctrl->kills_num_allied = oldcctrl->kills_num_allied;
    newcctrl->kills_num_enemy = oldcctrl->kills_num_enemy;
    newcctrl->joining_age = oldcctrl->joining_age;
    newtng->creation_turn = oldtng->creation_turn;

    if (ncrconf->gold_hold >= oldtng->creature.gold_carried)
    {
        newtng->creature.gold_carried += oldtng->creature.gold_carried;
        oldtng->creature.gold_carried = 0;
    }
    else
    {
        newtng->creature.gold_carried = ncrconf->gold_hold;
        oldtng->creature.gold_carried -= ncrconf->gold_hold;
    }
    return;
}

long update_creature_levels(struct Thing *thing)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    if (!cctrl->exp_level_up)
    {
        return 0;
    }
    cctrl->exp_level_up = false;
    lua_on_level_up(thing);

    // If a creature is not on highest level, just update the level.
    if (cctrl->exp_level+1 < CREATURE_MAX_LEVEL)
    {
        remove_creature_score_from_owner(thing); // the opposite is in set_creature_level()
        set_creature_level(thing, cctrl->exp_level+1);
        check_experience_upgrade(thing);
        return 1;
    }
    // If it is highest level, check if the creature can grow up.
    struct CreatureModelConfig *crconf = creature_stats_get_from_thing(thing);
    if (crconf->grow_up == 0)
    {
        return 0;
    }
    if (!grow_up_creature(thing, crconf->grow_up, crconf->grow_up_level))
    {
        return 0;
    }
    return -1;
}

TngUpdateRet update_creature(struct Thing *thing)
{
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl))
    {
        WARNLOG("Killing %s index %d with invalid control %d.(%d)",thing_model_name(thing),(int)thing->index, thing->ccontrol_idx, game.conf.rules[thing->owner].game.creatures_count);
        kill_creature(thing, INVALID_THING, -1, CrDed_Default);
        return TUFRet_Deleted;
    }
    if ((cctrl->unsummon_turn > 0) && (cctrl->unsummon_turn < game.play_gameturn))
    {
        create_effect_around_thing(thing, ball_puff_effects[get_player_color_idx(thing->owner)]);
        kill_creature(thing, INVALID_THING, -1, CrDed_NotReallyDying| CrDed_NoEffects);
        return TUFRet_Deleted;
    }
    process_armageddon_influencing_creature(thing);

    if (cctrl->frozen_on_hit > 0)
        cctrl->frozen_on_hit--;
    if (cctrl->force_visible > 0)
        cctrl->force_visible--;
    if (cctrl->hand_blocked_turns > 0)
        cctrl->hand_blocked_turns--;
    if (cctrl->regular_creature.navigation_map_changed == 0)
        cctrl->regular_creature.navigation_map_changed = game.map_changed_for_nagivation;
    if ((cctrl->stopped_for_hand_turns == 0) || (cctrl->instance_id == CrInst_EAT))
    {
        process_creature_instance(thing);
    }
    update_creature_count(thing);
    if (flag_is_set(thing->alloc_flags,TAlF_IsControlled))
    {
        if ((cctrl->stateblock_flags == 0) || creature_state_cannot_be_blocked(thing))
        {
            if (cctrl->stopped_for_hand_turns > 0)
            {
                cctrl->stopped_for_hand_turns--;
            } else
            if (process_creature_state(thing) == TUFRet_Deleted)
            {
                ERRORLOG("Human controlled creature has been deleted by state routine.");
                return TUFRet_Deleted;
            }
        }
        struct PlayerInfo* player = get_player(thing->owner);
        if (creature_under_spell_effect(thing, CSAfF_Freeze))
        {
            if (!flag_is_set(player->additional_flags, PlaAF_FreezePaletteIsActive))
            {
                PaletteSetPlayerPalette(player, blue_palette);
            }
        }
        else
        {
            if (flag_is_set(player->additional_flags, PlaAF_FreezePaletteIsActive))
            {
                PaletteSetPlayerPalette(player, engine_palette);
            }
        }
    } else
    {
        if ((cctrl->stateblock_flags == 0) || creature_state_cannot_be_blocked(thing))
        {
            if (cctrl->stopped_for_hand_turns > 0)
            {
                cctrl->stopped_for_hand_turns--;
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
    if (flag_is_set(thing->alloc_flags, TAlF_IsControlled))
    {
        if (!flag_is_set(cctrl->creature_control_flags, CCFlg_MoveY))
        {
            cctrl->move_speed /= 2;
        }
        if (!flag_is_set(cctrl->creature_control_flags, CCFlg_MoveX))
        {
            cctrl->orthogn_speed /= 2;
        }
        if (!flag_is_set(cctrl->creature_control_flags, CCFlg_MoveZ))
        {
            cctrl->vertical_speed /= 2;
        }
    }
    else
    {
        cctrl->move_speed = 0;
    }
    process_spells_affected_by_effect_elements(thing);
    process_magic_fall_effect(thing);
    process_landscape_affecting_creature(thing);
    process_disease(thing);
    move_thing_in_map(thing, &thing->mappos);
    set_creature_graphic(thing);
    if (cctrl->spell_aura)
    {
        process_keeper_spell_aura(thing);
    }

    if (thing->creature.health_bar_turns > 0)
        thing->creature.health_bar_turns--;

    if (creature_is_group_member(thing))
    {
        if (creature_is_group_leader(thing)) {
            leader_find_positions_for_followers(thing);
        }
    }
    else
    {
        if (((game.play_gameturn + thing->index) % 41) == 0) //Check sometimes to move the familiar back into the group
        {
            if (cctrl->summoner_idx > 0)
            {
                if (!creature_is_kept_in_custody(thing))
                {
                    struct Thing* summoner = thing_get(cctrl->summoner_idx);
                    if (!creature_is_kept_in_custody(summoner))
                    {
                        add_creature_to_group(thing, summoner);
                    }
                }
            }
        }
    }

    if (cctrl->dragtng_idx > 0)
    {
        struct Thing* tngp = thing_get(cctrl->dragtng_idx);
        if (flag_is_set(tngp->state_flags, TF1_IsDragged1))
        {
            struct Coord3d* tngpos = &thing->mappos;
            struct Coord3d pvpos;
            pvpos.x.val = tngpos->x.val - (2 * thing->velocity.x.val);
            pvpos.y.val = tngpos->y.val - (2 * thing->velocity.y.val);
            pvpos.z.val = tngpos->z.val;

            move_thing_in_map(tngp, &pvpos);
            tngp->move_angle_xy = thing->move_angle_xy; //corpse gets rotated along with creature
        }
    }
    if (update_creature_levels(thing) == -1)
    {
        return TUFRet_Deleted;
    }

    if (!process_creature_self_spell_casting(thing))
    {
        // If this creature didn't cast anything to itself, try to help others.
        process_creature_ranged_buff_spell_casting(thing);
    }
    // Reset acceleration.
    cctrl->moveaccel.x.val = 0;
    cctrl->moveaccel.y.val = 0;
    cctrl->moveaccel.z.val = 0;
    // Clear flags and process spell effects.
    clear_flag(cctrl->creature_control_flags, CCFlg_MoveX | CCFlg_MoveY | CCFlg_MoveZ);
    clear_flag(cctrl->spell_flags, CSAfF_PoisonCloud | CSAfF_Wind);
    process_timebomb(thing);
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
    if (creature_is_leaving_and_cannot_be_stopped(thing))
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
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    return (creature_under_spell_effect(thing, CSAfF_Invisibility) && (cctrl->force_visible <= 0));
}

TbBool creature_can_see_invisible(const struct Thing *thing)
{
    struct CreatureModelConfig *crconf = creature_stats_get_from_thing(thing);
    return (creature_under_spell_effect(thing, CSAfF_Sight) || (crconf->can_see_invisible));
}

int claim_neutral_creatures_in_sight(struct Thing *creatng, struct Coord3d *pos, int can_see_slabs)
{
    MapSlabCoord slb_x = subtile_slab(pos->x.stl.num);
    MapSlabCoord slb_y = subtile_slab(pos->y.stl.num);
    long n = 0;
    long i = game.nodungeon_creatr_list_start;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->players_next_creature_idx;
        // Per thing code starts
        int dx = abs(slb_x - subtile_slab(thing->mappos.x.stl.num));
        int dy = abs(slb_y - subtile_slab(thing->mappos.y.stl.num));
        if ((dx <= can_see_slabs) && (dy <= can_see_slabs))
        {
            if (is_neutral_thing(thing) && line_of_sight_3d(&thing->mappos, pos))
            {
                if (creature_is_leaving_and_cannot_be_stopped(thing) || creature_is_leaving_and_cannot_be_stopped(creatng))
                    return false;

                // Unless the relevant classic bug is enabled,
                // neutral creatures in custody (prison/torture) can only be claimed by the player who holds it captive
                // and neutral creatures can not be claimed by creatures in custody.
                if ((game.conf.rules[creatng->owner].game.classic_bugs_flags & ClscBug_PassiveNeutrals)
                    || (get_room_creature_works_in(thing)->owner == creatng->owner && !creature_is_kept_in_custody(creatng))
                    || !(creature_is_kept_in_custody(thing) || creature_is_kept_in_custody(creatng)))
                {
                    change_creature_owner(thing, creatng->owner);
                    mark_creature_joined_dungeon(thing);
                    if (is_my_player_number(thing->owner))
                    {
                        output_message(SMsg_CreaturesJoinedYou, MESSAGE_DURATION_CRTR_JOINED);
                    }
                    n++;
                }
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
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if (player_is_neutral(plyr_idx))
            continue;
        struct PlayerInfo* player = get_player(plyr_idx);
        if ( ((player->allocflags & PlaF_Allocated) != 0) && (player->is_active == 1) && (player->victory_state != VicS_LostLevel) )
        {
            struct Thing* heartng = get_player_soul_container(plyr_idx);
            if (thing_exists(heartng) && (get_chessboard_distance(&creatng->mappos, &heartng->mappos) < subtile_coord(6,0)))
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
    TbBool result = false;
    unsigned long k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    int i = slist->index;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing)) {
            ERRORLOG("Jump to invalid thing detected");
            result = true;
            break;
        }
        i = thing->next_of_class;
        // Per-creature block starts
        long crstate = get_creature_state_besides_move(thing);
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
            case CrSt_GoodArrivedAtAttackRoom:
            case CrSt_GoodArrivedAtSabotageRoom:
            case CrSt_GoodWanderToCreatureCombat:
            case CrSt_GoodWanderToObjectCombat:
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

void create_light_for_possession(struct Thing *creatng)
{
    struct InitLight ilght;
    memset(&ilght, 0, sizeof(struct InitLight));
    ilght.mappos.x.val = creatng->mappos.x.val;
    ilght.mappos.y.val = creatng->mappos.y.val;
    ilght.mappos.z.val = creatng->mappos.z.val;
    ilght.flags = 1;
    ilght.intensity = 36;
    ilght.radius = 2560;
    ilght.is_dynamic = 1;
    creatng->light_id = light_create_light(&ilght);
    if (creatng->light_id != 0) {
        light_set_light_never_cache(creatng->light_id);
    } else {
      ERRORLOG("Cannot allocate light to new controlled thing");
    }
}

void illuminate_creature(struct Thing *creatng)
{
    if (creatng->light_id == 0)
    {
        create_light_for_possession(creatng);
    }
    light_set_light_intensity(creatng->light_id, (light_get_light_intensity(creatng->light_id) + 20));
    struct Light* lgt = &game.lish.lights[creatng->light_id];
    lgt->radius <<= 1;
}

struct Thing *script_create_creature_at_location(PlayerNumber plyr_idx, ThingModel crmodel, TbMapLocation location, char spawn_type)
{
    struct Coord3d pos;

    if (!creature_count_below_map_limit(0))
    {
        WARNLOG("Could not create creature %s from script to due to creature limit", creature_code_name(crmodel));
        return INVALID_THING;
    }

    if (!get_coords_at_location(&pos, location,true))
    {
        return INVALID_THING;
    }

    if (spawn_type == SpwnT_Default)
    {
        switch (get_map_location_type(location))
        {
        case MLoc_ACTIONPOINT:
            if (player_is_roaming(plyr_idx))
            {
                spawn_type = SpwnT_Fall;
            }
            else
            {
                spawn_type = SpwnT_None;
            }
            break;
        case MLoc_HEROGATE:
            spawn_type = SpwnT_Jump;
            break;
        }
    }

    struct Thing* thing = create_thing_at_position_then_move_to_valid_and_add_light(&pos, TCls_Creature, crmodel, plyr_idx);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Couldn't create %s at location %d", creature_code_name(crmodel), (int)location);
            // Error is already logged
        return INVALID_THING;
    }

    // Lord of the land random speech message.
    if (flag_is_set(get_creature_model_flags(thing), CMF_IsLordOfLand))
    {
        output_message(SMsg_LordOfLandComming, MESSAGE_DURATION_LORD);
        output_message(SMsg_EnemyLordQuote + SOUND_RANDOM(8), MESSAGE_DURATION_LORD);
    }

    switch (spawn_type)
    {
    case SpwnT_Default:
    case SpwnT_None:
        // no special behavior
        break;
    case SpwnT_Jump:
        set_flag(thing->movement_flags, TMvF_MagicFall);
        thing->veloc_push_add.x.val += PLAYER_RANDOM(plyr_idx, 193) - 96;
        thing->veloc_push_add.y.val += PLAYER_RANDOM(plyr_idx, 193) - 96;
        if (flag_is_set(thing->movement_flags, TMvF_Flying))
        {
            thing->veloc_push_add.z.val -= PLAYER_RANDOM(plyr_idx, 32);
        }
        else
        {
            thing->veloc_push_add.z.val += PLAYER_RANDOM(plyr_idx, 96) + 80;
        }
        set_flag(thing->state_flags, TF1_PushAdd);
        break;
    case SpwnT_Fall:
        thing->mappos.z.val = get_ceiling_height(&thing->mappos);
        create_effect(&thing->mappos, TngEff_CeilingBreach, thing->owner);
        initialise_thing_state(thing, CrSt_CreatureHeroEntering);
        set_flag(thing->rendering_flags, TRF_Invisible);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        cctrl->countdown = 24;
        break;
    case SpwnT_Initialize:
        init_creature_state(thing);
        break;
    default:
        ERRORLOG("Invalid spawn type %d", spawn_type);
        break;
    }
    return thing;
}

struct Thing *script_create_new_creature(PlayerNumber plyr_idx, ThingModel crmodel, TbMapLocation location, long carried_gold, CrtrExpLevel exp_level, char spawn_type)
{
    struct Thing* creatng = script_create_creature_at_location(plyr_idx, crmodel, location, spawn_type);
    if (thing_is_invalid(creatng))
        return INVALID_THING;
    creatng->creature.gold_carried = carried_gold;
    init_creature_level(creatng, exp_level);
    return creatng;
}

void script_process_new_creatures(PlayerNumber plyr_idx, ThingModel crmodel, TbMapLocation location, long copies_num, long carried_gold, CrtrExpLevel exp_level, char spawn_type)
{
    for (long i = 0; i < copies_num; i++)
    {
        script_create_new_creature(plyr_idx, crmodel, location, carried_gold, exp_level, spawn_type);
    }
}

/**
 * @brief Picking up things as a possessed creature
 *
 * @param creatng
 * @param picktng
 * @param plyr_idx
 */
void controlled_creature_pick_thing_up(struct Thing *creatng, struct Thing *picktng, PlayerNumber plyr_idx)
{
    if (picktng->class_id == TCls_Creature)
    {
        set_creature_being_dragged_by(picktng, creatng);
    }
    else
    {
        if ((picktng->owner != creatng->owner) && (picktng->owner != game.neutral_player_num) )
        {
            if (thing_is_workshop_crate(picktng))
            {
                update_workshop_object_pickup_event(creatng, picktng);
            }
            else if ( (thing_is_spellbook(picktng)) || (thing_is_special_box(picktng)) )
            {
                update_library_object_pickup_event(creatng, picktng);
            }
        }
        creature_drag_object(creatng, picktng);
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->pickup_object_id = picktng->index;
    struct CreatureSound* crsound = get_creature_sound(creatng, CrSnd_Hurt);
    unsigned short smpl_idx = crsound->index + 1;
    thing_play_sample(creatng, smpl_idx, 90, 0, 3, 0, 2, FULL_LOUDNESS * 5/4);
    display_controlled_pick_up_thing_name(picktng, (GUI_MESSAGES_DELAY >> 4), plyr_idx);
}
/**
 * @brief Dropping down things at a specific place as a possessed creature
 *
 * @param creatng
 * @param droptng
 * @param plyr_idx
 */
void controlled_creature_drop_thing(struct Thing *creatng, struct Thing *droptng, PlayerNumber plyr_idx)
{
    long volume = FULL_LOUDNESS;
    if (droptng->class_id == TCls_Creature)
    {
        stop_creature_being_dragged_by(droptng, creatng);
    }
    else
    {
        creature_drop_dragged_object(creatng, droptng);
    }
    clear_messages_from_player(MsgType_Room, RoK_LIBRARY);
    clear_messages_from_player(MsgType_Room, RoK_WORKSHOP);
    unsigned short smpl_idx, pitch;
    if (subtile_has_water_on_top(droptng->mappos.x.stl.num, droptng->mappos.y.stl.num))
    {
        smpl_idx = 21 + SOUND_RANDOM(4);
        pitch = 75;
    }
    else
    {
        switch(droptng->class_id)
        {
            case TCls_Object:
            {
                smpl_idx = 992;
                struct ObjectConfigStats* objst = get_object_model_stats(droptng->model);
                switch (objst->genre)
                {
                    case OCtg_WrkshpBox:
                    case OCtg_SpecialBox:
                    {
                        pitch = 25;
                        break;
                    }
                    case OCtg_Spellbook:
                    {
                        pitch = 90;
                        break;
                    }
                    default:
                    {
                        pitch = 0;
                        break;
                    }
                }
                break;
            }
            case TCls_Creature:
            {
                long weight = compute_creature_weight(droptng);
                if (weight >= 0 && weight <= 99)
                {
                    pitch = 240;
                    volume = FULL_LOUDNESS / 2;
                }
                else if (weight >= 100 && weight <= 199)
                {
                    pitch = 120;
                    volume = FULL_LOUDNESS * 3 / 4;
                }
                else
                {
                    pitch = 75;
                }
                smpl_idx = 17 + SOUND_RANDOM(4);
                break;
            }
            case TCls_DeadCreature:
            {
                smpl_idx = 58;
                pitch = 50;
                break;
            }
            default:
            {
                smpl_idx = 0;
                pitch = 0;
                break;
            }
        }
    }
    thing_play_sample(droptng, smpl_idx, pitch, 0, 3, 0, 2, volume);
    struct Room* room = subtile_room_get(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
    if (!room_is_invalid(room))
    {
        if (room->owner == creatng->owner)
        {
            if (room_role_matches(room->kind, RoRoF_PowersStorage))
            {
                if (thing_is_spellbook(droptng))
                {
                    if (add_item_to_room_capacity(room, true))
                    {
                        droptng->owner = creatng->owner;
                        add_power_to_player(book_thing_to_power_kind(droptng), creatng->owner);
                    }
                    else
                    {
                        WARNLOG("Adding %s index %d to %s room capacity failed",thing_model_name(droptng),(int)droptng->index,room_role_code_name(RoRoF_PowersStorage));
                        if (is_my_player_number(plyr_idx))
                        {
                            output_message(SMsg_LibraryTooSmall, 0);
                        }
                    }
                }
                else if (thing_is_special_box(droptng))
                {
                    droptng->owner = creatng->owner;
                }
            }
            else if (room_role_matches(room->kind, RoRoF_CratesStorage))
            {
                if (thing_is_workshop_crate(droptng))
                {
                    if (add_item_to_room_capacity(room, true))
                    {
                        droptng->owner = creatng->owner;
                        add_workshop_item_to_amounts(room->owner, crate_thing_to_workshop_item_class(droptng), crate_thing_to_workshop_item_model(droptng));
                    }
                    else
                    {
                        WARNLOG("Adding %s index %d to %s room capacity failed",thing_model_name(droptng),(int)droptng->index,room_role_code_name(RoRoF_CratesStorage));
                        if (is_my_player_number(plyr_idx))
                        {
                            output_message(SMsg_WorkshopTooSmall, 0);
                        }
                    }
                }
            }
            else if (room_role_matches(room->kind, RoRoF_DeadStorage))
            {
                if (thing_is_dead_creature(droptng))
                {
                    if (add_body_to_graveyard(droptng, room))
                    {
                        droptng->owner = creatng->owner;
                    }
                    else
                    {
                        if (is_my_player_number(plyr_idx))
                        {
                            output_message(SMsg_GraveyardTooSmall, 0);
                        }
                    }
                }
            }
            else if (room_role_matches(room->kind, RoRoF_Prison))
            {
                if (thing_is_creature(droptng))
                {
                    if (creature_is_being_unconscious(droptng))
                    {
                        if (room->used_capacity < room->total_capacity)
                        {
                            make_creature_conscious(droptng);
                            initialise_thing_state(droptng, CrSt_CreatureArrivedAtPrison);
                            struct CreatureControl* dropctrl = creature_control_get_from_thing(droptng);
                            dropctrl->creature_control_flags |= CCFlg_NoCompControl;
                        }
                        else
                        {
                            if (is_my_player_number(plyr_idx))
                            {
                                output_message(SMsg_PrisonTooSmall, 0);
                            }
                        }
                    }
                }
            }
             else if (room_role_matches(room->kind, RoRoF_LairStorage))
            {
                if(game.conf.rules[creatng->owner].workers.drag_to_lair)
                {
                    if (thing_is_creature(droptng) && (creatng->owner == droptng->owner))
                    {
                        if (creature_is_being_unconscious(droptng))
                        {
                            struct CreatureControl* dropctrl = creature_control_get_from_thing(droptng);
                            //creature already has a lair rom
                            if (dropctrl->lair_room_id == room->index)
                            {
                                make_creature_conscious(droptng);
                                initialise_thing_state(droptng, CrSt_CreatureGoingHomeToSleep);
                            }
                            //creature doesn't have a lair room but it will and can sleep here
                            if ((game.conf.rules[creatng->owner].workers.drag_to_lair == 2)
                                && (dropctrl->lair_room_id == 0)
                                && (creature_can_do_healing_sleep(droptng))
                                && (room_has_enough_free_capacity_for_creature_job(room, droptng, Job_TAKE_SLEEP)))
                            {
                                make_creature_conscious(droptng);
                                initialise_thing_state(droptng, CrSt_CreatureChangeLair);
                            }
                            set_flag(dropctrl->creature_control_flags,CCFlg_NoCompControl);

                        }
                    }
                }
            }
        }
    }
}

void direct_control_pick_up_or_drop(PlayerNumber plyr_idx, struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* dragtng = thing_get(cctrl->dragtng_idx);
    struct PlayerInfo* player = get_player(plyr_idx);
    if (!thing_is_invalid(dragtng))
    {
        if (thing_is_trap_crate(dragtng))
        {
            struct Thing *traptng = thing_get(player->selected_fp_thing_pickup);
            if (!thing_is_invalid(traptng))
            {
                if (traptng->class_id == TCls_Trap)
                {
                    cctrl->arming_thing_id = traptng->index;
                    internal_set_thing_state(creatng, CrSt_CreatureArmsTrap);
                    return;
                }
            }
        }
        controlled_creature_drop_thing(creatng, dragtng, plyr_idx);
    }
    else
    {
        struct Thing* picktng = thing_get(player->selected_fp_thing_pickup);
        struct Room* room;
        if (!thing_is_invalid(picktng))
        {
            if (object_is_gold_pile(picktng))
            {
                struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
                if (creatng->creature.gold_carried < crconf->gold_hold)
                {
                    cctrl->pickup_object_id = picktng->index;
                    internal_set_thing_state(creatng, CrSt_ImpPicksUpGoldPile);
                    return;
                }
                else
                {
                    if (is_thing_directly_controlled_by_player(creatng, plyr_idx))
                    {
                        if (is_my_player_number(plyr_idx))
                        {
                            play_non_3d_sample(119);
                        }
                        return;
                    }
                }
            }
            room = get_room_thing_is_on(picktng);
            if (!room_is_invalid(room))
            {
                if ( (room_role_matches(room->kind, RoRoF_CratesStorage)) && (room->owner == creatng->owner) )
                {
                    if (thing_is_workshop_crate(picktng) &! object_is_ignored_by_imps(picktng))
                    {
                        if (picktng->owner == creatng->owner)
                        {
                            if (!remove_item_from_room_capacity(room))
                            {
                                return;
                            }
                            if (remove_workshop_item_from_amount_stored(picktng->owner, crate_thing_to_workshop_item_class(picktng), crate_thing_to_workshop_item_model(picktng), WrkCrtF_NoOffmap) != WrkCrtS_Stored)
                            {
                                return;
                            }
                        }
                    }
                    else
                    {
                        if (is_thing_directly_controlled_by_player(creatng, plyr_idx))
                        {
                            if (is_my_player_number(plyr_idx))
                            {
                                play_non_3d_sample(119);
                            }
                            return;
                        }
                    }
                }
            }
            controlled_creature_pick_thing_up(creatng, picktng, plyr_idx);
        }
        else
        {
            room = get_room_thing_is_on(creatng);
            if (!room_is_invalid(room))
            {
                if (room_role_matches(room->kind, RoRoF_GoldStorage))
                {
                    if (room->owner == creatng->owner)
                    {
                        if (creatng->creature.gold_carried > 0)
                        {
                            internal_set_thing_state(creatng, CrSt_ImpDropsGold);
                        }
                    }
                }
            }
        }
    }
}

void display_controlled_pick_up_thing_name(struct Thing *picktng, unsigned long timeout, PlayerNumber plyr_idx)
{
    char id;
    char str[255] = "";
    char type;
    if (thing_is_trap_crate(picktng))
    {
        struct TrapConfigStats* trapst = get_trap_model_stats(crate_thing_to_workshop_item_model(picktng));
        str_append(str, sizeof(str), get_string(trapst->name_stridx));
        id = RoK_WORKSHOP;
        type = MsgType_Room;
    }
    else if (thing_is_door_crate(picktng))
    {
        struct DoorConfigStats* doorst = get_door_model_stats(crate_thing_to_workshop_item_model(picktng));
        str_append(str, sizeof(str), get_string(doorst->name_stridx));
        id = RoK_WORKSHOP;
        type = MsgType_Room;
    }
    else if (thing_is_spellbook(picktng))
    {
        str_append(str, sizeof(str), get_string(get_power_name_strindex(book_thing_to_power_kind(picktng))));
        id = RoK_LIBRARY;
        type = MsgType_Room;
    }
    else if (thing_is_special_box(picktng))
    {
        char msg_buf[255];
        if (thing_is_custom_special_box(picktng))
        {
            if (game.box_tooltip[picktng->custom_box.box_kind][0] == 0)
            {
                str_append(str, sizeof(str), get_string(get_special_description_strindex(box_thing_to_special(picktng))));
                strcpy(msg_buf, str);
                snprintf(str, sizeof(str), "%s", strtok(msg_buf, ":"));
            }
            else
            {
                str_append(str, sizeof(str), game.box_tooltip[picktng->custom_box.box_kind]);
                char *split = strchr(str, ':');
                if ((int)(split - str) > -1)
                {
                    strcpy(msg_buf, str);
                    snprintf(str, sizeof(str), "%s", strtok(msg_buf, ":"));
                }
            }
        }
        else
        {
            str_append(str, sizeof(str), get_string(get_special_description_strindex(box_thing_to_special(picktng))));
            strcpy(msg_buf, str);
            snprintf(str, sizeof(str), "%s", strtok(msg_buf, ":"));
        }
        id = RoK_LIBRARY;
        type = MsgType_Room;
    }
    else if (object_is_gold_pile(picktng))
    {
        struct PlayerInfo* player = get_my_player();
        struct Thing* creatng = thing_get(player->influenced_thing_idx);
        if (thing_is_creature(creatng))
        {
            struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
            long gold_remaining = (crconf->gold_hold - creatng->creature.gold_carried);
            long value = (picktng->creature.gold_carried > gold_remaining) ? gold_remaining : picktng->creature.gold_carried;
            if (value < picktng->creature.gold_carried)
            {
                snprintf(str, sizeof(str), "%d (%ld)", picktng->creature.gold_carried, value);
            }
            else
            {
                snprintf(str, sizeof(str), "%d", picktng->creature.gold_carried);
            }
        }
        id = 3;
        type = MsgType_Query;
    }
    else if (thing_is_creature(picktng))
    {
        id = picktng->owner;
        type = MsgType_Player;
        struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[picktng->model];
        snprintf(str, sizeof(str), "%s", get_string(crconf->namestr_idx));
    }
    else if (picktng->class_id == TCls_DeadCreature)
    {
        id = RoK_GRAVEYARD;
        type = MsgType_Room;
        struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[picktng->model];
        snprintf(str, sizeof(str), "%s", get_string(crconf->namestr_idx));
    }
    else
    {
        return;
    }
    zero_messages();
    targeted_message_add(type, id, plyr_idx, timeout, str);
}

struct Thing *controlled_get_thing_to_pick_up(struct Thing *creatng)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(ShM_Dig);
    unsigned char radius = 0;
    struct Coord3d pos;
    pos.x.val = creatng->mappos.x.val;
    pos.y.val = creatng->mappos.y.val;
    struct Thing *result = NULL;
    MapCoordDelta old_distance = INT32_MAX;
    MapCoordDelta new_distance;
    long dx = distance_with_angle_to_coord_x(shotst->speed, creatng->move_angle_xy);
    long dy = distance_with_angle_to_coord_y(shotst->speed, creatng->move_angle_xy);
    do
    {
        struct Map *blk = get_map_block_at(pos.x.stl.num, pos.y.stl.num);
        for (struct Thing* picktng = thing_get(get_mapwho_thing_index(blk)); (!thing_is_invalid(picktng)); picktng = thing_get(picktng->next_on_mapblk))
        {
            if (picktng != creatng)
            {
                if (thing_is_pickable_by_digger(picktng, creatng))
                {
                    if (line_of_sight_3d(&creatng->mappos, &picktng->mappos))
                    {
                        new_distance = get_chessboard_3d_distance(&creatng->mappos, &picktng->mappos);
                        if (new_distance < old_distance)
                        {
                            old_distance = new_distance;
                            result = picktng;
                        }
                    }
                }
            }
        }
        pos.x.val += dx;
        pos.y.val += dy;
        radius++;
    }
    while (radius <= shotst->health);
    return result;
}

TbBool thing_is_pickable_by_digger(struct Thing *picktng, struct Thing *creatng)
{
    if (check_place_to_pretty_excluding(creatng, subtile_slab(creatng->mappos.x.stl.num), subtile_slab(creatng->mappos.y.stl.num))
        || (check_place_to_convert_excluding(creatng, subtile_slab(creatng->mappos.x.stl.num), subtile_slab(creatng->mappos.y.stl.num)) ) )
    {
        return false;
    }
    struct SlabMap *slb = get_slabmap_thing_is_on(picktng);
    if (object_is_gold_pile(picktng))
    {
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
        return ( ( (slabmap_owner(slb) == creatng->owner) || (subtile_is_unclaimed_path(picktng->mappos.x.stl.num, picktng->mappos.y.stl.num)) || (subtile_is_liquid(picktng->mappos.x.stl.num, picktng->mappos.y.stl.num)) ) &&
                  (creatng->creature.gold_carried < crconf->gold_hold) );
    }
    else if (thing_is_creature(picktng))
    {
        if (creature_is_being_unconscious(picktng))
        {
            if ((game.conf.rules[creatng->owner].workers.drag_to_lair > 0) && (picktng->owner == creatng->owner))
            {
                return (picktng->owner == creatng->owner);
            }
            else
            {
                return (picktng->owner != creatng->owner);
            }
        }
    }
    else if (thing_is_dead_creature(picktng))
    {
        return ( (get_room_of_role_slabs_count(creatng->owner, RoRoF_DeadStorage) > 0) && (corpse_ready_for_collection(picktng)) );
    }
    else if (thing_can_be_picked_to_place_in_player_room_of_role(picktng, creatng->owner, RoRoF_PowersStorage, TngFRPickF_Default))
    {
        if(!creature_can_pickup_library_object_at_subtile(creatng, picktng->mappos.x.stl.num, picktng->mappos.y.stl.num))
        {
            return false;
        }
        return (get_room_of_role_slabs_count(creatng->owner, RoRoF_PowersStorage) > 0);
    }
    else if (thing_can_be_picked_to_place_in_player_room_of_role(picktng, creatng->owner, RoRoF_CratesStorage, TngFRPickF_Default))
    {
        return (get_room_of_role_slabs_count(creatng->owner, RoRoF_CratesStorage) > 0);
    }
    else if (thing_is_trap_crate(picktng)) // for trap crates in one's own Workshop
    {
        struct Room* room = get_room_thing_is_on(picktng);
        if (!room_is_invalid(room))
        {
            if (room_role_matches(room->kind, RoRoF_CratesStorage))
            {
                if (room->owner == creatng->owner)
                {
                    if ( (picktng->owner == room->owner) && (picktng->owner == creatng->owner) )
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

struct Thing *controlled_get_trap_to_rearm(struct Thing *creatng)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(ShM_Dig);
    unsigned char radius = 0;
    struct Coord3d pos;
    pos.x.val = creatng->mappos.x.val;
    pos.y.val = creatng->mappos.y.val;
    long dx = distance_with_angle_to_coord_x(shotst->speed, creatng->move_angle_xy);
    long dy = distance_with_angle_to_coord_y(shotst->speed, creatng->move_angle_xy);
    do
    {
        struct Thing* traptng = get_trap_for_position(pos.x.stl.num, pos.y.stl.num);
        if (!thing_is_invalid(traptng))
        {
            if (traptng->owner == creatng->owner)
            {
                struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
                struct Thing* dragtng = thing_get(cctrl->dragtng_idx);
                if (traptng->model == crate_to_workshop_item_model(dragtng->model))
                {
                    if (traptng->trap.num_shots == 0)
                    {
                        return traptng;
                    }
                }
            }
        }
        pos.x.val += dx;
        pos.y.val += dy;
        radius++;
    }
    while (radius <= shotst->health);
    return INVALID_THING;
}

void controlled_continue_looking_excluding_diagonal(struct Thing *creatng, MapSubtlCoord *stl_x, MapSubtlCoord *stl_y)
{
    MapSubtlCoord x = *stl_x;
    MapSubtlCoord y = *stl_y;
    if ( (creatng->move_angle_xy >= ANGLE_NORTHWEST) || (creatng->move_angle_xy < ANGLE_NORTHEAST) )
    {
        y--;
    }
    else if ( (creatng->move_angle_xy >= ANGLE_SOUTHEAST) && (creatng->move_angle_xy <= ANGLE_SOUTHWEST) )
    {
        y++;
    }
    else if ( (creatng->move_angle_xy >= ANGLE_SOUTHWEST) && (creatng->move_angle_xy <= ANGLE_NORTHWEST) )
    {
        x--;
    }
    else if ( (creatng->move_angle_xy >= ANGLE_NORTHEAST) && (creatng->move_angle_xy <= ANGLE_SOUTHEAST) )
    {
        x++;
    }
    *stl_x = x;
    *stl_y = y;
}

PlayerNumber get_appropriate_player_for_creature(struct Thing *creatng)
{
    if ((creatng->alloc_flags & TAlF_IsControlled) != 0)
    {
        for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
        {
            if (is_thing_directly_controlled_by_player(creatng, plyr_idx))
            {
                return plyr_idx;
            }
        }
    }
    return creatng->owner;
}

static int filter_criteria_type(long desc_type)
{
    return desc_type & 0x0F;
}

static long filter_criteria_loc(long desc_type)
{
    return desc_type >> 4;
}

struct Thing* script_get_creature_by_criteria(PlayerNumber plyr_idx, ThingModel crmodel, short criteria)
{
    switch (filter_criteria_type(criteria))
    {
    case CSelCrit_Any:
        return get_random_players_creature_of_model(plyr_idx, crmodel);
    case CSelCrit_MostExperienced:
        return find_players_highest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Any, plyr_idx, 0);
    case CSelCrit_MostExpWandering:
        return find_players_highest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Wandering, plyr_idx, 0);
    case CSelCrit_MostExpWorking:
        return find_players_highest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Working, plyr_idx, 0);
    case CSelCrit_MostExpFighting:
        return find_players_highest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Fighting, plyr_idx, 0);
    case CSelCrit_LeastExperienced:
        return find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Any, plyr_idx, 0);
    case CSelCrit_LeastExpWandering:
        return find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Wandering, plyr_idx, 0);
    case CSelCrit_LeastExpWorking:
        return find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Working, plyr_idx, 0);
    case CSelCrit_LeastExpFighting:
        return find_players_lowest_level_creature_of_breed_and_gui_job(crmodel, CrGUIJob_Fighting, plyr_idx, 0);
    case CSelCrit_NearOwnHeart:
    {
        const struct Coord3d* pos = dungeon_get_essential_pos(plyr_idx);
        return get_creature_near_and_owned_by(pos->x.val, pos->y.val, plyr_idx, crmodel);
    }
    case CSelCrit_NearEnemyHeart:
        //return get_creature_in_range_around_any_of_enemy_heart(plyr_idx, crmodel, 11);
    case CSelCrit_OnEnemyGround:
        return get_random_players_creature_of_model_on_territory(plyr_idx, crmodel, 0);
    case CSelCrit_OnFriendlyGround:
        return get_random_players_creature_of_model_on_territory(plyr_idx, crmodel, 1);
    case CSelCrit_OnNeutralGround:
        return get_random_players_creature_of_model_on_territory(plyr_idx, crmodel, 2);
    case CSelCrit_NearAP:
    {
        int loc = filter_criteria_loc(criteria);
        struct ActionPoint* apt = action_point_get(loc);
        if (!action_point_exists(apt))
        {
            WARNLOG("Action point is invalid:%d", apt->num);
            return INVALID_THING;
        }
        if (apt->range == 0)
        {
            WARNLOG("Action point with zero range:%d", apt->num);
            return INVALID_THING;
        }
        // Action point range should be inside spiral in subtiles
        int dist = 2 * coord_subtile(apt->range + COORD_PER_STL - 1) + 1;
        dist = dist * dist;

        Thing_Maximizer_Filter filter = near_map_block_creature_filter_diagonal_random;
        struct CompoundTngFilterParam param;
        param.model_id = crmodel;
        param.plyr_idx = (unsigned char)plyr_idx;
        param.primary_number = apt->mappos.x.val;
        param.secondary_number = apt->mappos.y.val;
        param.tertiary_number = apt->range;
        return get_thing_spiral_near_map_block_with_filter(apt->mappos.x.val, apt->mappos.y.val,
            dist,
            filter, &param);
    }
    default:
        ERRORLOG("Invalid level up criteria %d", (int)criteria);
        return INVALID_THING;
    }
}

void query_creature(struct PlayerInfo *player, ThingIndex index, TbBool reset, TbBool zoom)
{
    if (is_my_player(player))
    {
        MenuID menu;
        if (reset)
        {
            menu = GMnu_CREATURE_QUERY1;
        }
        else
        {
            if (menu_is_active(GMnu_CREATURE_QUERY1))
            {
                menu = GMnu_CREATURE_QUERY1;
            }
            else if (menu_is_active(GMnu_CREATURE_QUERY2))
            {
                menu = GMnu_CREATURE_QUERY2;
            }
            else if (menu_is_active(GMnu_CREATURE_QUERY3))
            {
                menu = GMnu_CREATURE_QUERY3;
            }
            else if (menu_is_active(GMnu_CREATURE_QUERY4))
            {
                menu = GMnu_CREATURE_QUERY4;
            }
            else
            {
                menu = GMnu_CREATURE_QUERY1;
            }
        }
        turn_off_all_panel_menus();
        initialise_tab_tags_and_menu(menu);
        turn_on_menu(menu);
    }
    struct Thing* creatng = thing_get(index);
    player->influenced_thing_idx = index;
    player->influenced_thing_creation = creatng->creation_turn;
    if (zoom)
    {
        player->zoom_to_pos_x = creatng->mappos.x.val;
        player->zoom_to_pos_y = creatng->mappos.y.val;
        set_player_instance(player, PI_ZoomToPos, 0);
    }
    set_player_instance(player, PI_QueryCrtr, 0);
}

TbBool creature_can_be_queried(struct PlayerInfo *player, struct Thing *creatng)
{
    if (creature_is_leaving_and_cannot_be_stopped(creatng))
        return false;

    switch (player->work_state)
    {
        case PSt_CreatrInfo:
        case PSt_CreatrQuery:
        {
            if (!subtile_revealed(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, player->id_number))
            {
                return false;
            }
            if (creatng->owner != player->id_number)
            {
                return creature_is_kept_in_custody_by_player(creatng, player->id_number);
            }
            else
            {
                if (creature_is_kept_in_custody_by_enemy(creatng))
                {
                    return false;
                }
            }
            break;
        }
        case PSt_CreatrInfoAll:
        case PSt_QueryAll:
        {
            return subtile_revealed(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, player->id_number);
        }
        default:
        {
            return false;
        }
    }
    return true;
}

TbBool creature_can_be_transferred(const struct Thing* thing)
{
    return ((get_creature_model_flags(thing) & CMF_NoTransfer) == 0);
}

/* Returns a random creature kind with model flags as argument. */
ThingModel get_random_creature_kind_with_model_flags(unsigned long model_flags)
{
    // Array to store the IDs of creatures kinds with model flags.
    ThingModel creature_kind_with_model_flags_array[CREATURE_TYPES_MAX];
    // Counter for the number of creatures kinds found.
    short creature_kind_with_model_flags_count = 0;
    // Loop through all available creatures kinds.
    for (ThingModel crkind = 0; crkind < game.conf.crtr_conf.model_count; crkind++)
    {
        // Check if the creature kind has the flag.
        if (flag_is_set(game.conf.crtr_conf.model[crkind].model_flags, model_flags))
        {
            // Ensure we don't exceed the maximum array size.
            if (creature_kind_with_model_flags_count < CREATURE_TYPES_MAX)
            {
                // Add the creature kind to the array.
                creature_kind_with_model_flags_array[creature_kind_with_model_flags_count++] = crkind;
            } else {
                break;
            }
        }
    }
    if (creature_kind_with_model_flags_count > 0)
    {
        // Get a random creature kind from the list.
        short random_idx = GAME_RANDOM(creature_kind_with_model_flags_count);
        return creature_kind_with_model_flags_array[random_idx];
    }
    // Return -1 if no suitable creature kind is found.
    return -1;
}

/* Returns a random creature kind, excluding spectators and diggers.
 * Appropriate means evil and good creatures randomise within their respective classes. */
ThingModel get_random_appropriate_creature_kind(ThingModel original_model)
{
    struct CreatureModelConfig *newconf;
    struct CreatureModelConfig *oldconf = &game.conf.crtr_conf.model[original_model];
    ThingModel random_model;
    while (true)
    {
        random_model = GAME_RANDOM(game.conf.crtr_conf.model_count) + 1;
        // Exclude out-of-bounds model number.
        if (random_model >= game.conf.crtr_conf.model_count)
        {
            continue;
        }
        // Exclude same creature kind, spectators and diggers.
        newconf = &game.conf.crtr_conf.model[random_model];
        if ((random_model == original_model) || (any_flag_is_set(newconf->model_flags, (CMF_IsSpectator|CMF_IsSpecDigger|CMF_IsDiggingCreature))))
        {
            continue;
        }
        // Evil randomise into evil, good randomise into good.
        if ((flag_is_set(newconf->model_flags, CMF_IsEvil)) && (flag_is_set(oldconf->model_flags, CMF_IsEvil)))
        {
            break;
        }
        if ((!flag_is_set(newconf->model_flags, CMF_IsEvil)) && (!flag_is_set(oldconf->model_flags, CMF_IsEvil)))
        {
            break;
        }
    }
    return random_model;
}

TbBool grow_up_creature(struct Thing *thing, ThingModel grow_up_model, CrtrExpLevel grow_up_level)
{
    if (grow_up_model == CREATURE_NOT_A_DIGGER)
    {
        grow_up_model = get_random_appropriate_creature_kind(thing->model);
    }
    if (!creature_count_below_map_limit(1))
    {
        WARNLOG("Could not create creature to transform %s to due to creature limit", thing_model_name(thing));
        return false;
    }
    struct Thing *newtng = create_creature(&thing->mappos, grow_up_model, thing->owner);
    if (thing_is_invalid(newtng))
    {
        ERRORLOG("Could not create creature to transform %s to", thing_model_name(thing));
        return false;
    }
    // Randomise new level if 'grow_up_level' was set to 0 on the creature config.
    if (grow_up_level == 0)
    {
        set_creature_level(newtng, GAME_RANDOM(CREATURE_MAX_LEVEL));
    }
    else
    {
        set_creature_level(newtng, grow_up_level - 1);
    }
    transfer_creature_data_and_gold(thing, newtng); // Transfer the blood type, creature name, kill count, joined age and carried gold to the new creature.
    update_creature_health_to_max(newtng);
    struct CreatureControl *cctrl = creature_control_get_from_thing(newtng);
    cctrl->countdown = 50;
    external_set_thing_state(newtng, CrSt_CreatureBeHappy);
    struct PlayerInfo *player = get_player(thing->owner);
    // Switch control if this creature is possessed.
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
    // If not directly nor passenger controlled, but still player is doing something with it.
    if (thing->index == player->controlled_thing_idx)
    {
        set_selected_creature(player, newtng);
    }
    remove_creature_score_from_owner(thing); // kill_creature() doesn't call this.
    // Handles picked up by player case.
    if (thing_is_picked_up_by_player(thing, thing->owner))
    {
        struct Dungeon *dungeon = get_dungeon(thing->owner);
        if (get_thing_in_hand_id(thing, thing->owner) >= 0)
        {
            dungeon->things_in_hand[get_thing_in_hand_id(thing, thing->owner)] = newtng->index;
            remove_thing_from_limbo(thing);
            place_thing_in_limbo(newtng);
        }
        else
        {
            ERRORLOG("Picked up thing is not in player hand list");
        }
    }
    kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects | CrDed_NoUnconscious | CrDed_NotReallyDying);
    return true;
}

TbResult script_use_spell_on_creature(PlayerNumber plyr_idx, struct Thing *thing, SpellKind spkind, CrtrExpLevel spell_level)
{
    struct SpellConfig *spconf = get_spell_config(spkind);
    if (!creature_is_immune_to_spell_effect(thing, spconf->spell_flags))
    { // Immunity is handled in 'apply_spell_effect_to_thing', but this command plays sounds, so check for it.
        if (thing_is_picked_up(thing))
        {
            SYNCDBG(5, "Found creature to cast the spell on but it is being held.");
            return Lb_FAIL;
        }
        if (spconf->caster_affect_sound)
        {
            thing_play_sample(thing, spconf->caster_affect_sound + SOUND_RANDOM(spconf->caster_sounds_count), NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
        }
        apply_spell_effect_to_thing(thing, spkind, spell_level, plyr_idx);
        if (flag_is_set(spconf->spell_flags, CSAfF_Disease))
        {
            struct CreatureControl *cctrl;
            cctrl = creature_control_get_from_thing(thing);
            cctrl->disease_caster_plyridx = game.neutral_player_num; // Does not spread.
        }
        return Lb_SUCCESS;
    }
    else
    {
        return Lb_FAIL;
    }
}

/**
 * Cast a spell on a creature which meets given criteria.
 * @param plyr_idx The player whose creature will be affected.
 * @param crmodel Model of the creature to find.
 * @param criteria Criteria, from CreatureSelectCriteria enumeration.
 * @param fmcl_bytes encoded bytes: f=cast for free flag,m=spell kind,c=caster player index,l=spell level.
 * @return TbResult whether the spell was successfully cast
 */
TbResult script_use_spell_on_creature_with_criteria(PlayerNumber plyr_idx, ThingModel crmodel, short criteria, SpellKind spell_idx, CrtrExpLevel charge)
{
    struct Thing *thing = script_get_creature_by_criteria(plyr_idx, crmodel, criteria);
    if (thing_is_invalid(thing))
    {
        SYNCDBG(5, "No matching player %d creature of model %d (%s) found to use spell on.", (int)plyr_idx, (int)crmodel, creature_code_name(crmodel));
        return Lb_FAIL;
    }
    return script_use_spell_on_creature(plyr_idx, thing, spell_idx, charge);
}

void script_move_creature(struct Thing* thing, TbMapLocation location, ThingModel effect_id)
{

    if (effect_id < 0)
    {
        effect_id = ball_puff_effects[thing->owner];
    }

    struct Coord3d pos;
    if(!get_coords_at_location(&pos,location,false)) {
        SYNCDBG(5,"No valid coords for location %d",(int)location);
        return;
    }
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);

    if (effect_id > 0)
    {
        create_effect(&thing->mappos, effect_id, game.neutral_player_num);
        create_effect(&pos, effect_id, game.neutral_player_num);
    }
    move_thing_in_map(thing, &pos);
    reset_interpolation_of_thing(thing);
    if (!is_thing_some_way_controlled(thing))
    {
        initialise_thing_state(thing, CrSt_CreatureDoingNothing);
    }
    cctrl->turns_at_job = -1;
    check_map_explored(thing, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
}

void script_move_creature_with_criteria(PlayerNumber plyr_idx, ThingModel crmodel, long select_id, TbMapLocation location, ThingModel effect_id, long count)
{
    for (int i = 0; i < count; i++)
    {
        struct Thing *thing = script_get_creature_by_criteria(plyr_idx, crmodel, select_id);
        if (thing_is_invalid(thing) || thing_is_picked_up(thing)) {
            continue;
        }
        script_move_creature(thing, location, effect_id);
    }
}

/**
 * Modifies player's creatures' anger.
 * @param plyr_idx target player
 * @param anger anger value. Use double AnnoyLevel (from creature's config file) to fully piss creature. More for longer calm time
 */
TbBool script_change_creatures_annoyance(PlayerNumber plyr_idx, ThingModel crmodel, long operation, long anger)
{
    SYNCDBG(8, "Starting");
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    unsigned long k = 0;
    int i = dungeon->creatr_list_start;
    if (creature_kind_is_for_dungeon_diggers_list(plyr_idx,crmodel))
    {
        i = dungeon->digger_list_start;
    }
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Per creature code

        if (thing_matches_model(thing,crmodel))
        {
            i = cctrl->players_next_creature_idx;
            if (operation == SOpr_SET)
            {
                anger_set_creature_anger(thing, 0, AngR_NotPaid);
                anger_set_creature_anger(thing, 0, AngR_NoLair);
                anger_set_creature_anger(thing, 0, AngR_Hungry);
                anger_set_creature_anger(thing, anger, AngR_Other);
            }
            else if (operation == SOpr_INCREASE)
            {
                AnnoyMotive motive = anger_get_creature_anger_type(thing);
                if (motive)
                {
                    anger_increase_creature_anger(thing, anger, motive);
                }
                else
                {
                    anger_increase_creature_anger(thing, anger, AngR_Other);
                }
            }
            else if (operation == SOpr_DECREASE)
            {
                anger_apply_anger_to_creature_all_types(thing, -anger);
            }
            else if (operation == SOpr_MULTIPLY)
            {
                anger_set_creature_anger(thing, cctrl->annoyance_level[AngR_Other] * anger, AngR_Other);
                anger_set_creature_anger(thing, cctrl->annoyance_level[AngR_NotPaid] * anger, AngR_NotPaid);
                anger_set_creature_anger(thing, cctrl->annoyance_level[AngR_NoLair] * anger, AngR_NoLair);
                anger_set_creature_anger(thing, cctrl->annoyance_level[AngR_Hungry] * anger, AngR_Hungry);
            }

        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    SYNCDBG(19, "Finished");
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
