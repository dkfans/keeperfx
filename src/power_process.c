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
#include "power_process.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_math.h"
#include "bflib_memory.h"

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

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT unsigned char _DK_backup_explored[26][26];
#define backup_explored _DK_backup_explored
/******************************************************************************/
DLLIMPORT void _DK_remove_explored_flags_for_power_sight(struct PlayerInfo *player);
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
    SYNCDBG(6,"Setting to %ld",pwkind);
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

TbBool player_uses_power_armageddon(PlayerNumber plyr_idx)
{
    return (game.armageddon_cast_turn != 0) && (game.armageddon_caster_idx == plyr_idx);
}

void process_armageddon(void)
{
    struct PlayerInfo *player;
    long i;
    SYNCDBG(6,"Starting");
    if (game.armageddon_cast_turn == 0)
        return;
    if (game.armageddon.count_down+game.armageddon_cast_turn > game.play_gameturn)
    {
        if (player_cannot_win(game.armageddon_caster_idx))
        {
            // Stop the armageddon if its originator is just losing
            game.armageddon_cast_turn = 0;
        }
    } else
    if (game.armageddon.count_down+game.armageddon_cast_turn == game.play_gameturn)
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
    if (game.armageddon.count_down+game.armageddon_cast_turn < game.play_gameturn)
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

void process_armageddon_influencing_creature(struct Thing *creatng)
{
    if (game.armageddon_cast_turn != 0)
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        // If Armageddon is on, teleport creature to its position
        if ((cctrl->armageddon_teleport_turn != 0) && (cctrl->armageddon_teleport_turn <= game.play_gameturn))
        {
            if (cctrl->instance_id == CrInst_NULL) // Avoid corruption from in progress instances, like claiming floors.
            {
                cctrl->armageddon_teleport_turn = 0;
                create_effect(&creatng->mappos, imp_spangle_effects[creatng->owner], creatng->owner);
                move_thing_in_map(creatng, &game.armageddon.mappos);
            }
        }
    }
}

void process_disease(struct Thing *creatng)
{
    SYNCDBG(18,"Starting");
    //_DK_process_disease(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (!creature_affected_by_spell(creatng, SplK_Disease)) {
        return;
    }
    if (CREATURE_RANDOM(creatng, 100) < game.disease_transfer_percentage)
    {
        SubtlCodedCoords stl_num = get_subtile_number(creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
        for (long n = 0; n < AROUND_MAP_LENGTH; n++)
        {
            struct Map* mapblk = get_map_block_at_pos(stl_num + around_map[n]);
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
              if (thing_is_creature(thing) && ((get_creature_model_flags(thing) & CMF_IsSpecDigger) == 0) && ((get_creature_model_flags(thing) & CMF_NeverSick) == 0)
                && (thing->owner != cctrl->disease_caster_plyridx) && !creature_affected_by_spell(thing, SplK_Disease) && (cctrl->disease_caster_plyridx != game.neutral_player_num))
              {
                  struct CreatureControl* tngcctrl = creature_control_get_from_thing(thing);
                  apply_spell_effect_to_thing(thing, SplK_Disease, cctrl->explevel);
                  tngcctrl->disease_caster_plyridx = cctrl->disease_caster_plyridx;
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
        }
    }
    if (((game.play_gameturn - cctrl->disease_start_turn) % game.disease_lose_health_time) == 0)
    {
        apply_damage_to_thing_and_display_health(creatng, game.disease_lose_percentage_health * cctrl->max_health / 100, DmgT_Biological, cctrl->disease_caster_plyridx);
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
            if (get_2d_box_distance(&myplyr->acamera->mappos, &thing->mappos) < 11520)
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
            if (get_2d_box_distance(&myplyr->acamera->mappos, &thing->mappos) < 11520)
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
    long i = (game.play_gameturn - thing->creation_turn) % 16;
    struct Thing* target;
    switch (i)
    {
    case 0:
        god_lightning_choose_next_creature(thing);
        break;
    case 1:
        target = thing_get(thing->shot.target_idx);
        if (thing_is_invalid(target))
            break;
        draw_lightning(&thing->mappos,&target->mappos, 96, TngEffElm_ElectricBall3);
        break;
    case 2:
    {
        target = thing_get(thing->shot.target_idx);
        if (thing_is_invalid(target))
            break;
        struct ShotConfigStats* shotst = get_shot_model_stats(ShM_GodLightBall);
        apply_damage_to_thing_and_display_health(target, shotst->damage, shotst->damage_type, thing->owner);
        if (target->health < 0)
        {
            struct CreatureControl* cctrl = creature_control_get_from_thing(target);
            cctrl->shot_model = ShM_GodLightBall;
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
    //_DK_god_lightning_choose_next_creature(shotng); return;

    long best_dist = LONG_MAX;
    struct Thing* best_thing = INVALID_THING;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Creature);
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
        if ((shotng->owner != thing->owner) && !thing_is_picked_up(thing)
            && !creature_is_being_unconscious(thing) && !creature_is_dying(thing))
        {
            long dist = get_2d_distance(&shotng->mappos, &thing->mappos);
            if (dist < best_dist)
            {
                const struct MagicStats* pwrdynst = get_power_dynamic_stats(PwrK_LIGHTNING);
                int spell_lev = shotng->shot.byte_19;
                if (spell_lev > SPELL_MAX_LEVEL)
                    spell_lev = SPELL_MAX_LEVEL;
                if (subtile_coord(pwrdynst->strength[spell_lev],0) > dist)
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
    if (!thing_is_invalid(best_thing)) {
        shotng->shot.target_idx = best_thing->index;
    } else {
        shotng->shot.target_idx = 0;
    }
}

void draw_god_lightning(struct Thing *shotng)
{
    //_DK_draw_god_lightning(shotng); return;
    struct PlayerInfo* player = get_player(shotng->owner);
    const struct Camera* cam = player->acamera;
    if (cam == NULL) {
        return;
    }
    for (int i = LbFPMath_PI / 4; i < 2 * LbFPMath_PI; i += LbFPMath_PI / 2)
    {
        struct Coord3d locpos;
        locpos.x.val = shotng->mappos.x.val;
        locpos.y.val = shotng->mappos.y.val;
        locpos.z.val = shotng->mappos.z.val;
        locpos.x.val +=  (LbSinL(i + cam->orient_a) >> (LbFPMath_TrigmBits - 10));
        locpos.y.val += -(LbCosL(i + cam->orient_a) >> (LbFPMath_TrigmBits - 10));
        locpos.z.val = subtile_coord(12,0);
        draw_lightning(&locpos, &shotng->mappos, 256, TngEffElm_ElectricBall3);
    }
}

TbBool player_uses_power_call_to_arms(PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    return (dungeon->cta_start_turn != 0);
}

void creature_stop_affected_by_call_to_arms(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->spell_flags &= ~CSAfF_CalledToArms;
    if (!thing_is_picked_up(thing) && !creature_is_being_unconscious(thing))
    {
        if (creature_is_called_to_arms(thing)) {
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
    //_DK_turn_off_call_to_arms(plyr_idx);
    if (!player_uses_power_call_to_arms(plyr_idx)) {
        return;
    }
    struct PlayerInfo* player = get_player(plyr_idx);
    {
        struct Thing* objtng = thing_get(player->field_43C);
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
        if ( (stl_y >= 0) && (stl_y <= 255) )
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
                    if (stl_x > map_subtiles_x-1)
                    {
                        stl_x = map_subtiles_x-1;
                    }
                    if (boundstl_x < 0)
                    {
                        boundstl_x = 0;
                    } else
                    if (boundstl_x > map_subtiles_x-1)
                    {
                        boundstl_x = map_subtiles_x-1;
                    }
                    if (boundstl_x >= stl_x)
                    {
                        delta = boundstl_x - stl_x + 1;
                        long slb_y = subtile_slab_fast(stl_y);
                        for (i=0; i < delta; i++)
                        {
                            struct Map* mapblk = get_map_block_at(stl_x + i, stl_y);
                            reveal_map_block(mapblk, player->id_number);
                            long slb_x = subtile_slab_fast(stl_x + i);
                            struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
                            struct SlabAttr* slbattr = get_slab_attrs(slb);
                            if ( !slbattr->is_diggable )
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
                    if (boundstl_y > map_subtiles_y-1)
                    {
                        boundstl_y = map_subtiles_y-1;
                    }
                    if (stl_y < 0)
                    {
                        stl_y = 0;
                    } else
                    if (stl_y > map_subtiles_y-1)
                    {
                        stl_y = map_subtiles_y-1;
                    }
                    if (stl_y <= boundstl_y)
                    {
                      delta = boundstl_y - stl_y + 1;
                      long slb_x = subtile_slab_fast(stl_x);
                      for (i=0; i < delta; i++)
                      {
                          long slb_y = subtile_slab_fast(stl_y + i);
                          struct Map* mapblk = get_map_block_at(stl_x, stl_y + i);
                          reveal_map_block(mapblk, player->id_number);
                          struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
                          struct SlabAttr* slbattr = get_slab_attrs(slb);
                          if ( !slbattr->is_diggable )
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
    LbMemorySet(backup_explored, 0, sizeof(backup_explored));
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
    SYNCDBG(9,"Starting");
    _DK_remove_explored_flags_for_power_sight(player);
}
/******************************************************************************/
