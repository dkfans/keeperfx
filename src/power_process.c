/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file power_process.c
 *     Keeper powers process functions.
 * @par Purpose:
 *     Functions to check availability and use keeper powers.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 21 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "power_process.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_math.h"
#include "bflib_planar.h"

#include "player_data.h"
#include "dungeon_data.h"
#include "player_utils.h"
#include "thing_shots.h"
#include "thing_objects.h"
#include "thing_physics.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "creature_instances.h"
#include "creature_states.h"
#include "creature_senses.h"
#include "ariadne_wallhug.h"
#include "config_terrain.h"
#include "config_creature.h"
#include "config_effects.h"
#include "front_simple.h"
#include "slab_data.h"
#include "game_legacy.h"
#include "power_hand.h"
#include "player_instances.h"
#include "local_camera.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static unsigned char backup_explored[26][26];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/**
 * Sets keeper power selected by local human player.
 *
 * @param pwkind Power to select.
 * @param sptooltip Tooltip string index.
 * @note Was set_chosen_spell()
 */
void set_chosen_power(PowerKind pwkind, TextStringId sptooltip)
{
    const struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
    if (power_model_stats_invalid(powerst))
      pwkind = 0;
    SYNCDBG(6,"Setting to %u",pwkind);
    game.chosen_spell_type = pwkind;
    game.chosen_spell_spridx = powerst->bigsym_sprite_idx;
    game.chosen_spell_tooltip = sptooltip;
}

void set_chosen_power_none(void)
{
    SYNCDBG(6,"Setting to %d",0);
    game.chosen_spell_type = 0;
    game.chosen_spell_spridx = 0;
    game.chosen_spell_tooltip = 0;
}

unsigned char general_expand_check(void)
{
    struct PlayerInfo* player = get_my_player();
    return (player->cast_expand_level != 0);
}

unsigned char sight_of_evil_expand_check(void)
{
    struct PlayerInfo* myplyr = get_my_player();
    return (myplyr->cast_expand_level != 0) && (!player_uses_power_sight(myplyr->id_number));
}

unsigned char call_to_arms_expand_check(void)
{
    struct PlayerInfo* myplyr = get_my_player();
    return (myplyr->cast_expand_level != 0) && (!player_uses_power_call_to_arms(myplyr->id_number));
}

void process_armageddon(void)
{
    struct PlayerInfo *player;
    long i;
    SYNCDBG(6,"Starting");
    GameTurnDelta countdown = game.conf.rules[game.armageddon_caster_idx].magic.armageddon_count_down;
    if (game.armageddon_cast_turn == 0)
        return;
    if ((game.armageddon_cast_turn + countdown) > game.play_gameturn)
    {
        if (player_cannot_win(game.armageddon_caster_idx))
        {
            // Stop the armageddon if its originator is just losing
            game.armageddon_cast_turn = 0;
        }
    } else
    if ((game.armageddon_cast_turn + countdown) == game.play_gameturn)
    {
        for (i=0; i < PLAYERS_COUNT; i++)
        {
            player = get_player(i);
            if (player_exists(player))
            {
              if (player->is_active == 1)
                reveal_whole_map(player);
            }
        }
    } else
    if ((game.armageddon_cast_turn + countdown) < game.play_gameturn)
    {
        for (i=0; i < PLAYERS_COUNT; i++)
        {
            player = get_player(i);
            if ( (player_exists(player)) && (player->is_active == 1) )
            {
                struct Dungeon* dungeon = get_dungeon(player->id_number);
                if ((player->victory_state == VicS_Undecided) && (dungeon->num_active_creatrs == 0))
                {
                    event_kill_all_players_events(i);
                    set_player_as_lost_level(player);
                    if (is_my_player_number(i))
                        LbPaletteSet(engine_palette);
                    struct Thing* heartng = get_player_soul_container(player->id_number);
                    if (thing_exists(heartng)) {
                        heartng->health = -1;
                    }
                }
            }
        }
    }
}

void teleport_armageddon_influenced_creature(struct Thing* creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->armageddon_teleport_turn = 0;
    create_effect(&creatng->mappos, imp_spangle_effects[get_player_color_idx(creatng->owner)], creatng->owner);
    move_thing_in_map(creatng, &game.armageddon_mappos);
    cleanup_current_thing_state(creatng);
    reset_interpolation_of_thing(creatng);
}

void process_armageddon_influencing_creature(struct Thing *creatng)
{
    if (game.armageddon_cast_turn != 0)
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        // If Armageddon is on, teleport creature to its position
        if ((cctrl->armageddon_teleport_turn != 0) && (cctrl->armageddon_teleport_turn <= game.play_gameturn))
        {
            teleport_armageddon_influenced_creature(creatng);
        }
    }
}

void process_disease(struct Thing *creatng)
{
    SYNCDBG(18, "Starting");
    struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
    struct CreatureControl *tngcctrl;
    if (!creature_under_spell_effect(creatng, CSAfF_Disease))
    {
        return;
    }
    if (THING_RANDOM(creatng, 100) < game.conf.rules[creatng->owner].magic.disease_transfer_percentage)
    {
        SubtlCodedCoords stl_num = get_subtile_number(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
        for (long n = 0; n < AROUND_MAP_LENGTH; n++)
        {
            struct Map *mapblk = get_map_block_at_pos(stl_num + game.around_map[n]);
            unsigned long k = 0;
            long i = get_mapwho_thing_index(mapblk);
            while (i != 0)
            {
                struct Thing *thing = thing_get(i);
                if (thing_is_invalid(thing))
                {
                    WARNLOG("Jump out of things array");
                    break;
                }
                i = thing->next_on_mapblk;
                // Per thing code.
                tngcctrl = creature_control_get_from_thing(thing);
                if (thing_is_creature(thing)
                && !creature_is_for_dungeon_diggers_list(thing)
                && (thing->owner != cctrl->disease_caster_plyridx)
                && !creature_under_spell_effect(thing, CSAfF_Disease)
                && !creature_is_immune_to_spell_effect(thing, CSAfF_Disease)
                && (cctrl->disease_caster_plyridx != game.neutral_player_num))
                { // Apply the spell kind stored in 'active_disease_spell'.
                    apply_spell_effect_to_thing(thing, cctrl->active_disease_spell, cctrl->exp_level, creatng->owner);
                    tngcctrl->disease_caster_plyridx = cctrl->disease_caster_plyridx;
                }
                // Per thing code ends.
                k++;
                if (k > THINGS_COUNT)
                {
                    ERRORLOG("Infinite loop detected when sweeping things list");
                    break_mapwho_infinite_chain(mapblk);
                    break;
                }
            }
        }
    }
    if (((game.play_gameturn - cctrl->disease_start_turn) % game.conf.rules[creatng->owner].magic.disease_lose_health_time) == 0)
    {
        apply_damage_to_thing_and_display_health(creatng, game.conf.rules[creatng->owner].magic.disease_lose_percentage_health * cctrl->max_health / 100, cctrl->disease_caster_plyridx);
    }
}

void lightning_modify_palette(struct Thing *thing)
{
    struct PlayerInfo* myplyr = get_my_player();

    if (thing->health == 0)
    {
      PaletteSetPlayerPalette(myplyr, engine_palette);
      myplyr->additional_flags &= ~PlaAF_LightningPaletteIsActive;
      return;
    }
    if (myplyr->acamera == NULL)
    {
        ERRORLOG("No active camera");
        return;
    }
    if (((thing->health % 8) != 7) && (thing->health != 1) && (UNSYNC_RANDOM(4) != 0))
    {
        if ((myplyr->additional_flags & PlaAF_LightningPaletteIsActive) != 0)
        {
            if (get_chessboard_distance(&myplyr->acamera->mappos, &thing->mappos) < 11520)
            {
                PaletteSetPlayerPalette(myplyr, engine_palette);
                myplyr->additional_flags &= ~PlaAF_LightningPaletteIsActive;
            }
        }
        return;
    }
    if ((myplyr->view_mode != PVM_ParchFadeIn) && (myplyr->view_mode != PVM_ParchFadeOut) && (myplyr->view_mode != PVM_ParchmentView))
    {
        if ((myplyr->additional_flags & PlaAF_LightningPaletteIsActive) == 0)
        {
            if (get_chessboard_distance(&myplyr->acamera->mappos, &thing->mappos) < 11520)
            {
              PaletteSetPlayerPalette(myplyr, lightning_palette);
              myplyr->additional_flags |= PlaAF_LightningPaletteIsActive;
            }
        }
    }
}

void update_god_lightning_ball(struct Thing *thing)
{
    if (thing->health <= 0)
    {
        lightning_modify_palette(thing);
        return;
    }
    struct ShotConfigStats* shotst;
    long i = (game.play_gameturn - thing->creation_turn) % 16;
    struct Thing* target;
    switch (i)
    {
    case 0:
        god_lightning_choose_next_creature(thing);
        break;
    case 1:
        target = thing_get(thing->shot.target_idx);
        if (!thing_exists(target))
            break;
        shotst = get_shot_model_stats(thing->model);
        draw_lightning(&thing->mappos,&target->mappos, shotst->effect_spacing, shotst->effect_id);
        break;
    case 2:
    {
        target = thing_get(thing->shot.target_idx);
        if (!thing_exists(target))
            break;
        shotst = get_shot_model_stats(thing->model);
        apply_damage_to_thing_and_display_health(target, shotst->damage, thing->owner);
        if (target->health < 0)
        {
            struct CreatureControl* cctrl = creature_control_get_from_thing(target);
            cctrl->shot_model = thing->model;
            kill_creature(target, INVALID_THING, thing->owner, CrDed_DiedInBattle);
        }
        thing->shot.target_idx = 0;
        break;
    }
    }
}

void god_lightning_choose_next_creature(struct Thing *shotng)
{
    SYNCDBG(16,"Starting for %s index %d owner %d",thing_model_name(shotng),(int)shotng->index,(int)shotng->owner);

    long best_dist = INT32_MAX;
    struct Thing* best_thing = INVALID_THING;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
    struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    unsigned long k = 0;
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
        // Per-thing code
        //TODO use hit_type instead of hard coded conditions
        if (!players_are_mutual_allies(shotng->owner,thing->owner) && !thing_is_picked_up(thing)
            && !creature_is_being_unconscious(thing) && !creature_is_dying(thing))
        {
            long dist = get_2d_distance(&shotng->mappos, &thing->mappos);
            if (dist < best_dist)
            {
                if (shotst->max_range > dist)
                {
                    if (line_of_sight_2d(&shotng->mappos, &thing->mappos)) {
                        best_dist = dist;
                        best_thing = thing;
                    }
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > slist->count)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    SYNCDBG(8,"The best target for %s index %d owner %d is %s index %d owner %d",
        thing_model_name(shotng),(int)shotng->index,(int)shotng->owner,
        thing_model_name(best_thing),(int)best_thing->index,(int)best_thing->owner);
    if (thing_exists(best_thing)) {
        shotng->shot.target_idx = best_thing->index;
    } else {
        shotng->shot.target_idx = 0;
    }
}

void draw_god_lightning(struct Thing *shotng)
{
    struct PlayerInfo* player = get_player(shotng->owner);
    const struct Camera* cam = get_local_camera(player->acamera);
    if (cam == NULL) {
        return;
    }
    for (int i = DEGREES_45; i < DEGREES_360; i += DEGREES_90)
    {
        struct Coord3d locpos;
        locpos.x.val = (shotng->mappos.x.val + (LbSinL(i + cam->rotation_angle_x) >> (LbFPMath_TrigmBits - 10))) + 128;
        locpos.y.val = (shotng->mappos.y.val - (LbCosL(i + cam->rotation_angle_x) >> (LbFPMath_TrigmBits - 10))) + 128;
        locpos.z.val = shotng->mappos.z.val + subtile_coord(12,0);
        struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model); //default ShM_GodLightning
        draw_lightning(&locpos, &shotng->mappos, shotst->effect_spacing, shotst->effect_id);
    }
}

TbBool player_uses_power_call_to_arms(PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    return (dungeon->cta_start_turn != 0);
}

void creature_stop_affected_by_call_to_arms(struct Thing *thing)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    cctrl->called_to_arms = false;
    if (!thing_is_picked_up(thing) && !creature_is_being_unconscious(thing))
    {
        if (creature_is_called_to_arms(thing))
        {
            set_start_state(thing);
        }
    }
}

TbBool reset_creature_if_affected_by_cta(struct Thing *thing)
{
    if (creature_affected_by_call_to_arms(thing))
    {
        creature_stop_affected_by_call_to_arms(thing);
        return true;
    }
    return false;
}

void turn_off_power_call_to_arms(PlayerNumber plyr_idx)
{
    if (!player_uses_power_call_to_arms(plyr_idx)) {
        return;
    }
    struct PlayerInfo* player = get_player(plyr_idx);
    {
        struct Thing* objtng = thing_get(player->cta_flag_idx);
        set_call_to_arms_as_dying(objtng);
        struct Dungeon* dungeon = get_players_dungeon(player);
        dungeon->cta_start_turn = 0;
    }
    reset_all_players_creatures_affected_by_cta(plyr_idx);
}

void store_backup_explored_flags_for_power_sight(struct PlayerInfo *player, struct Coord3d *soe_pos)
{
    struct Dungeon* dungeon = get_players_dungeon(player);
    MapSubtlCoord stl_y = (long)soe_pos->y.stl.num - MAX_SOE_RADIUS;
    for (long soe_y = 0; soe_y < 2 * MAX_SOE_RADIUS; soe_y++, stl_y++)
    {
        MapSubtlCoord stl_x = (long)soe_pos->x.stl.num - MAX_SOE_RADIUS;
        for (long soe_x = 0; soe_x < 2 * MAX_SOE_RADIUS; soe_x++, stl_x++)
        {
            if (dungeon->soe_explored_flags[soe_y][soe_x])
            {
                struct Map* mapblk = get_map_block_at(stl_x, stl_y);
                if (!map_block_invalid(mapblk))
                {
                    if (map_block_revealed(mapblk, player->id_number))
                        backup_explored[soe_y][soe_x] |= 0x01;
                    if ((mapblk->flags & SlbAtFlg_Unexplored) != 0)
                        backup_explored[soe_y][soe_x] |= 0x02;
                    if ((mapblk->flags & SlbAtFlg_TaggedValuable) != 0)
                        backup_explored[soe_y][soe_x] |= 0x04;
                }
            }
        }
    }
}

void update_vertical_explored_flags_for_power_sight(struct PlayerInfo *player, struct Coord3d *soe_pos)
{
    struct Dungeon* dungeon = get_players_dungeon(player);
    MapSubtlCoord stl_y = (long)soe_pos->y.stl.num - MAX_SOE_RADIUS;
    for (long soe_y = 0; soe_y < 2 * MAX_SOE_RADIUS; soe_y++, stl_y++)
    {
        if ( (stl_y >= 0) && (stl_y <= game.map_subtiles_y) )
        {
            MapSubtlCoord stl_x = (long)soe_pos->x.stl.num - MAX_SOE_RADIUS;
            for (long soe_x = 0; soe_x <= MAX_SOE_RADIUS; soe_x++, stl_x++)
            {
                if (dungeon->soe_explored_flags[soe_y][soe_x])
                {
                    soe_x++;
                    // Find max value for delta
                    long delta = 0;
                    long i;
                    for (i = 1; soe_x < 2 * MAX_SOE_RADIUS; soe_x++, i++)
                    {
                        if (dungeon->soe_explored_flags[soe_y][soe_x])
                            delta = i;
                    }
                    long boundstl_x = stl_x + delta;
                    if (stl_x < 0)
                    {
                        stl_x = 0;
                    } else
                    if (stl_x > game.map_subtiles_x-1)
                    {
                        stl_x = game.map_subtiles_x-1;
                    }
                    if (boundstl_x < 0)
                    {
                        boundstl_x = 0;
                    } else
                    if (boundstl_x > game.map_subtiles_x-1)
                    {
                        boundstl_x = game.map_subtiles_x-1;
                    }
                    if (boundstl_x >= stl_x)
                    {
                        delta = boundstl_x - stl_x + 1;
                        long slb_y = subtile_slab(stl_y);
                        for (i=0; i < delta; i++)
                        {
                            struct Map* mapblk = get_map_block_at(stl_x + i, stl_y);
                            reveal_map_block(mapblk, player->id_number);
                            long slb_x = subtile_slab(stl_x + i);
                            struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
                            struct SlabConfigStats* slabst = get_slab_stats(slb);
                            if ( !slabst->is_diggable )
                                mapblk->flags &= ~(SlbAtFlg_TaggedValuable|SlbAtFlg_Unexplored);
                            mapblk++;
                        }
                        stl_x += delta;
                    }
                }
            }
        }
    }
}

TbBool player_uses_power_sight(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if (!player_exists(player)) {
        return false;
    }
    struct Dungeon* dungeon = get_players_dungeon(player);
    return (dungeon->sight_casted_thing_idx > 0);
}

TbBool player_uses_power_obey(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if (!player_exists(player)) {
        return false;
    }
    struct Dungeon* dungeon = get_players_dungeon(player);
    return (dungeon->must_obey_turn != 0);
}

TbBool player_uses_power_hold_audience(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if (!player_exists(player)) {
        return false;
    }
    struct Dungeon* dungeon = get_players_dungeon(player);
    return (dungeon->hold_audience_cast_turn != 0);
}

void update_horizonal_explored_flags_for_power_sight(struct PlayerInfo *player, struct Coord3d *soe_pos)
{
    struct Dungeon* dungeon = get_players_dungeon(player);
    long stl_x = (long)soe_pos->x.stl.num - MAX_SOE_RADIUS;
    for (long soe_x = 0; soe_x < 2 * MAX_SOE_RADIUS; soe_x++, stl_x++)
    {
        if ( (stl_x >= 0) && (stl_x <= 255) )
        {
            long stl_y = (long)soe_pos->y.stl.num - MAX_SOE_RADIUS;
            for (long soe_y = 0; soe_y <= MAX_SOE_RADIUS; soe_y++, stl_y++)
            {
                if (dungeon->soe_explored_flags[soe_y][soe_x])
                {
                    soe_y++;
                    // Find max value for delta
                    long delta = 0;
                    long i;
                    for (i = 1; soe_y < 2 * MAX_SOE_RADIUS; soe_y++, i++)
                    {
                        if (dungeon->soe_explored_flags[soe_y][soe_x])
                            delta = i;
                    }
                    long boundstl_y = stl_y + delta;
                    if (boundstl_y < 0)
                    {
                        boundstl_y = 0;
                    } else
                    if (boundstl_y > game.map_subtiles_y-1)
                    {
                        boundstl_y = game.map_subtiles_y-1;
                    }
                    if (stl_y < 0)
                    {
                        stl_y = 0;
                    } else
                    if (stl_y > game.map_subtiles_y-1)
                    {
                        stl_y = game.map_subtiles_y-1;
                    }
                    if (stl_y <= boundstl_y)
                    {
                      delta = boundstl_y - stl_y + 1;
                      long slb_x = subtile_slab(stl_x);
                      for (i=0; i < delta; i++)
                      {
                          long slb_y = subtile_slab(stl_y + i);
                          struct Map* mapblk = get_map_block_at(stl_x, stl_y + i);
                          reveal_map_block(mapblk, player->id_number);
                          struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
                          struct SlabConfigStats* slabst = get_slab_stats(slb);
                          if ( !slabst->is_diggable )
                              mapblk->flags &= ~(SlbAtFlg_TaggedValuable|SlbAtFlg_Unexplored);
                      }
                      stl_y += delta;
                    }
                }
            }
        }
    }
}

void update_explored_flags_for_power_sight(struct PlayerInfo *player)
{
    SYNCDBG(9,"Starting");
    struct Dungeon* dungeon = get_players_dungeon(player);
    memset(backup_explored, 0, sizeof(backup_explored));
    if (dungeon->sight_casted_thing_idx == 0) {
        return;
    }
    struct Thing* thing = thing_get(dungeon->sight_casted_thing_idx);
    if (!thing_is_object(thing)) {
        ERRORLOG("Sight thing index %d invalid", (int)dungeon->sight_casted_thing_idx);
        turn_off_power_sight_of_evil(player->id_number);
        dungeon->sight_casted_thing_idx = 0;
        return;
    }
    TRACE_THING(thing);
    // Fill the backup_explored array
    store_backup_explored_flags_for_power_sight(player, &thing->mappos);
    update_vertical_explored_flags_for_power_sight(player, &thing->mappos);
    update_horizonal_explored_flags_for_power_sight(player, &thing->mappos);

}

void remove_explored_flags_for_power_sight(struct PlayerInfo *player)
{
    SYNCDBG(9, "Starting");
    unsigned char backup_flags;
    struct Dungeon *dungeon = get_players_dungeon(player);

    if (!dungeon->sight_casted_thing_idx)
        return;
    struct Thing *sightng = thing_get(dungeon->sight_casted_thing_idx);
    struct Coord3d *soe_pos = &sightng->mappos;

    MapSubtlCoord stl_y = (long)soe_pos->y.stl.num - MAX_SOE_RADIUS;
    for (long soe_y = 0; soe_y < 2 * MAX_SOE_RADIUS; soe_y++, stl_y++)
    {
        MapSubtlCoord stl_x = (long)soe_pos->x.stl.num - MAX_SOE_RADIUS;
        for (long soe_x = 0; soe_x < 2 * MAX_SOE_RADIUS; soe_x++, stl_x++)
        {
            if (dungeon->soe_explored_flags[soe_y][soe_x])
            {
                struct Map* mapblk = get_map_block_at(stl_x, stl_y);
                if (!map_block_invalid(mapblk))
                {
                    backup_flags = backup_explored[soe_y][soe_x];
                    conceal_map_block(mapblk, player->id_number);
                    if ((backup_flags & 1) != 0)
                        reveal_map_block(mapblk, player->id_number);
                    if ((backup_flags & 2) != 0)
                        mapblk->flags |= SlbAtFlg_Unexplored;
                    if ((backup_flags & 4) != 0)
                        mapblk->flags |= SlbAtFlg_TaggedValuable;
                }
            }
        }
    }
}

void process_timebomb(struct Thing *creatng)
{
    SYNCDBG(18,"Starting");
    if (!creature_under_spell_effect(creatng, CSAfF_Timebomb))
    {
        return;
    }
    if (thing_is_picked_up(creatng))
    {
        return;
    }
    update_creature_speed(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct Thing* timetng = thing_get(cctrl->timebomb_countdown_id);
    if (!thing_exists(timetng))
    {
        if ((cctrl->timebomb_countdown % game_num_fps) == 0)
        {
            long time = (cctrl->timebomb_countdown / game_num_fps);
            timetng = create_price_effect(&creatng->mappos, creatng->owner, time);
            cctrl->timebomb_countdown_id = timetng->index;
            thing_play_sample(creatng, 853, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS); // 853 is hardcoded ticking sound, could be configurable.
        }
    }
    else
    {
        if (timetng->health <= 0)
        {
            delete_thing_structure(timetng, 0);
            cctrl->timebomb_countdown_id = 0;
        }
        else
        {
            struct Coord3d pos;
            pos.x.val = creatng->mappos.x.val;
            pos.y.val = creatng->mappos.y.val;
            pos.z.val = timetng->mappos.z.val;
            move_thing_in_map(timetng, &pos);
        }
    }
    struct Thing* trgtng = thing_get(cctrl->timebomb_target_id);
    if (thing_exists(trgtng))
    {
        if ((creatng->mappos.x.stl.num == trgtng->mappos.x.stl.num) && (creatng->mappos.y.stl.num == trgtng->mappos.y.stl.num))
        {
            if (abs(creatng->mappos.z.val - trgtng->mappos.z.val) <= creatng->solid_size_z)
            {
                timebomb_explode(creatng);
                return;
            }
        }
    }
    if (cctrl->timebomb_countdown != 0)
    {
        cctrl->timebomb_countdown--;
    }
    else
    {
        timebomb_explode(creatng);
    }
}

#define WEIGHT_DIVISOR 64

void timebomb_explode(struct Thing *creatng)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
    struct SpellConfig *spconf = get_spell_config(cctrl->active_timebomb_spell);
    struct ShotConfigStats *shotst = get_shot_model_stats(spconf->shot_model);
    SYNCDBG(8, "Explode Timebomb");
    struct Thing *shotng = create_shot(&creatng->mappos, spconf->shot_model, creatng->owner);
    if (!thing_is_invalid(shotng))
    {
        create_used_effect_or_element(&creatng->mappos, shotst->explode.effect1_model, creatng->owner, creatng->index);
        create_used_effect_or_element(&creatng->mappos, shotst->explode.effect2_model, creatng->owner, creatng->index);
        if (shotst->explode.around_effect1_model != 0)
        {
            create_effect_around_thing(creatng, shotst->explode.around_effect1_model);
        }
        if (shotst->explode.around_effect2_model > 0)
        {
            create_effect_around_thing(creatng, shotst->explode.around_effect2_model);
        }
        if (creature_model_bleeds(creatng->model))
        {
            create_effect_around_thing(creatng, TngEff_Blood5);
        }
        long weight = compute_creature_weight(creatng);
        struct CreatureModelConfig *crconf = creature_stats_get_from_thing(creatng);
        long dist = (compute_creature_attack_range(shotst->area_range * COORD_PER_STL, crconf->luck, cctrl->exp_level) * weight) / WEIGHT_DIVISOR;
        HitPoints damage = (compute_creature_attack_spell_damage(shotst->area_damage, crconf->luck, cctrl->exp_level, creatng->owner) * weight) / WEIGHT_DIVISOR;
        long blow_strength = (shotst->area_blow * weight) / WEIGHT_DIVISOR;
        HitTargetFlags hit_targets = hit_type_to_hit_targets(shotst->area_hit_type);
        shot_kill_creature(shotng, creatng);
        explosion_affecting_area(shotng, &shotng->mappos, dist, damage, blow_strength, hit_targets);
    }
}
/******************************************************************************/
