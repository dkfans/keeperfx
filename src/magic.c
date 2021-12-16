/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file magic.c
 *     magic support functions.
 * @par Purpose:
 *     Functions to magic.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "magic.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_sound.h"

#include "player_data.h"
#include "player_instances.h"
#include "player_states.h"
#include "player_utils.h"
#include "dungeon_data.h"
#include "thing_list.h"
#include "game_merge.h"
#include "power_specials.h"
#include "power_hand.h"
#include "thing_creature.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_shots.h"
#include "thing_traps.h"
#include "thing_factory.h"
#include "thing_navigate.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_states_lair.h"
#include "creature_states_mood.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "config_magic.h"
#include "config_effects.h"
#include "gui_soundmsgs.h"
#include "room_jobs.h"
#include "map_blocks.h"
#include "map_columns.h"
#include "sounds.h"
#include "game_legacy.h"
#include "creature_instances.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const long power_sight_close_instance_time[] = {4, 4, 5, 5, 6, 6, 7, 7, 8};

unsigned char destroy_effect[][9] = {
    {88, 88, 88, 88, 79, 88, 88, 88, 88,},//splevel=0
    {88, 88, 88, 88, 32, 88, 88, 88, 88,},
    {88, 88, 88, 79, 32, 79, 88, 88, 88,},
    {88, 79, 88, 79, 32, 79, 88, 79, 88,},
    {88, 79, 88, 79, 32, 79, 88, 79, 88,},
    {88, 88, 88, 32, 32, 32, 88, 88, 88,},
    {88, 32, 88, 32, 32, 32, 88, 32, 88,},
    {79, 32, 79, 32, 32, 32, 79, 32, 79,},
    {32, 32, 32, 32, 32, 32, 32, 32, 32,},//splevel=8
};

/******************************************************************************/
/******************************************************************************/
/**
 * Returns if spell can be casted or given thing and/or coordinates.
 * @param plyr_idx
 * @param pwkind
 * @param stl_x
 * @param stl_y
 * @param thing
 * @param flags
 * @param func_name Caller name for debug logging purposes.
 * @note This replaced can_thing_be_possessed()
 */
TbBool can_cast_spell_f(PlayerNumber plyr_idx, PowerKind pwkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y, const struct Thing *thing, unsigned long flags, const char *func_name)
{
    if ((flags & CastChk_SkipAvailiabilty) == 0)
    {
        if (!is_power_available(plyr_idx, pwkind)) {
            return false;
        }
    }
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player->work_state == PSt_FreeCtrlDirect)
    {
        return true;
    }
    else
    {
        TbBool cast_at_xy;
        TbBool cast_on_tng;
        cast_at_xy = can_cast_power_at_xy(plyr_idx, pwkind, stl_x, stl_y, 0);
        const struct PowerConfigStats *powerst;
        powerst = get_power_model_stats(pwkind);
        cast_on_tng = true;
        if (((powerst->can_cast_flags & PwCast_AllThings) != 0) && ((flags & CastChk_SkipThing) == 0))
        {
            if (thing_exists(thing)) {
                cast_on_tng = can_cast_power_on_thing(plyr_idx, thing, pwkind);
            } else {
                cast_on_tng = false;
            }
        }
        if ((powerst->can_cast_flags & PwCast_ThingOrMap) != 0)
        {
            // Fail only if both functions have failed - one is enough
            if (!cast_at_xy && !cast_on_tng) {
                if ((flags & CastChk_Final) != 0) {
                    WARNLOG("%s: Player %d tried to cast %s on %s which can't be targeted",func_name,(int)plyr_idx,
                        power_code_name(pwkind), (!cast_on_tng)?"a thing":(!cast_at_xy)?"a subtile":"thing or subtile");
                }
                return false;
            }
        } else
        {
            // Fail if any of the functions has failed - we need both
            if (!cast_at_xy || !cast_on_tng) {
                if ((flags & CastChk_Final) != 0) {
                    WARNLOG("%s: Player %d tried to cast %s on %s which can't be targeted",func_name,(int)plyr_idx,
                        power_code_name(pwkind), (!cast_on_tng)?"a thing":(!cast_at_xy)?"a subtile":"thing or subtile");
                }
                return false;
            }
        }
        if ((powerst->config_flags & PwCF_IsParent) != 0)
        {
            // If the power is a parent, then at least one child must allow casting it in given conditions
            TbBool can_cast_child;
            can_cast_child = false;
            int i;
            for (i = 0; i < magic_conf.power_types_count; i++)
            {
                const struct PowerConfigStats *child_powerst;
                child_powerst = get_power_model_stats(i);
                if (child_powerst->parent_power == pwkind)
                {
                    if (can_cast_spell_f(plyr_idx, i, stl_x, stl_y, thing, flags&(~CastChk_Final), func_name)) {
                        if ((flags & CastChk_Final) != 0) {
                            SYNCDBG(7,"%s: Player %d can cast %s; child power %s allows that",func_name,(int)plyr_idx,
                                power_code_name(pwkind),power_code_name(i));
                        }
                        can_cast_child = true;
                        break;
                    }
                }
            }
            if (!can_cast_child) {
                if ((flags & CastChk_Final) != 0) {
                    WARNLOG("%s: Player %d tried to cast %s; child powers do not allow that",func_name,(int)plyr_idx,
                        power_code_name(pwkind));
                }
                return false;
            }
        }
        return true;
    }
}

/**
 * Returns if a keeper power can be casted on specific thing.
 * Originally was can_cast_spell_on_creature().
 * @param plyr_idx
 * @param thing
 * @param pwmodel
 * @return
 */
TbBool can_cast_power_on_thing(PlayerNumber plyr_idx, const struct Thing *thing, PowerKind pwkind)
{
    SYNCDBG(18,"Starting for %s on %s index %d",power_code_name(pwkind),thing_model_name(thing),(int)thing->index);
    // Picked up things are immune to spells
    if (thing_is_picked_up(thing)) {
        return false;
    }
    struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(pwkind);
    if (power_model_stats_invalid(powerst))
        return false;
    if (thing_is_object(thing))
    {
        if ((powerst->can_cast_flags & PwCast_OwnedFood) != 0)
        {
            if (thing->owner == plyr_idx) {
                if (object_is_mature_food(thing))  {
                    return true;
                }
            }
        }
        if ((powerst->can_cast_flags & PwCast_NeutrlFood) != 0)
        {
            if (is_neutral_thing(thing)) {
                if (object_is_mature_food(thing))  {
                    return true;
                }
            }
        }
        if ((powerst->can_cast_flags & PwCast_EnemyFood) != 0)
        {
            if ((thing->owner != plyr_idx) && !is_neutral_thing(thing)) {
                if (object_is_mature_food(thing))  {
                    return true;
                }
            }
        }
        if ((powerst->can_cast_flags & PwCast_OwnedGold) != 0)
        {
            if (thing->owner == plyr_idx) {
                if (object_is_gold_pile(thing) || object_is_gold_hoard(thing)) {
                    return true;
                }
            }
        }
        if ((powerst->can_cast_flags & PwCast_NeutrlGold) != 0)
        {
            if (is_neutral_thing(thing)) {
                if (object_is_gold_pile(thing) || object_is_gold_hoard(thing)) {
                    return true;
                }
            }
        }
        if ((powerst->can_cast_flags & PwCast_EnemyGold) != 0)
        {
            if ((thing->owner != plyr_idx) && !is_neutral_thing(thing)) {
                if (object_is_gold_pile(thing) || object_is_gold_hoard(thing)) {
                    return true;
                }
            }
        }
        if ((powerst->can_cast_flags & PwCast_OwnedSpell) != 0)
        {
            if (thing->owner == plyr_idx) {
                if (thing_is_spellbook(thing))  {
                    return true;
                }
            }
        }
    }
    if (thing_is_shot(thing))
    {
        if ((powerst->can_cast_flags & PwCast_OwnedBoulders) != 0)
        {
            if (thing->owner == plyr_idx) {
                if (shot_is_slappable(thing, plyr_idx))  {
                    return true;
                }
            }
        }
    }
    if (thing_is_deployed_trap(thing))
    {
        // Allow the boulder trap
        if ((powerst->can_cast_flags & PwCast_OwnedBoulders) != 0)
        {
            if (thing->owner == plyr_idx) {
                struct TrapConfigStats *trapst;
                trapst = &gameadd.trapdoor_conf.trap_cfgstats[thing->model];
                if ((trapst->slappable == 1) && trap_is_active(thing)) {
                    return true;
                }
            }
        }
    }
    if (thing_is_creature(thing))
    {
        // Don't allow casting on own creatures kept by enemy - they're out of our control
        if (thing->owner == plyr_idx)
        {
            if (creature_is_kept_in_custody_by_enemy(thing)) {
                return false;
            }
        }
        if ((powerst->can_cast_flags & PwCast_AllCrtrs) == PwCast_AllCrtrs)
        {
            return true;
        }
        if ((powerst->can_cast_flags & PwCast_NConscCrtrs) == 0)
        {
            if (creature_is_being_unconscious(thing) || creature_is_dying(thing)) {
                SYNCDBG(8,"Player %d cannot cast %s on unconscious %s index %d",(int)plyr_idx,power_code_name(pwkind),thing_model_name(thing),(int)thing->index);
                return false;
            }
        }
        if ((powerst->can_cast_flags & PwCast_BoundCrtrs) == 0)
        {
            if (armageddon_blocks_creature_pickup(thing, plyr_idx)) {
                SYNCDBG(8,"Player %d cannot cast %s while armageddon blocks %s index %d",(int)plyr_idx,power_code_name(pwkind),thing_model_name(thing),(int)thing->index);
                return false;
            }
            if (creature_is_dragging_something(thing)) {
                SYNCDBG(8,"Player %d cannot cast %s while %s index %d is dragging something",(int)plyr_idx,power_code_name(pwkind),thing_model_name(thing),(int)thing->index);
                return false;
            }
            if (creature_is_being_sacrificed(thing) || creature_is_being_summoned(thing)) {
                SYNCDBG(8,"Player %d cannot cast %s on %s index %d while entering/leaving",(int)plyr_idx,power_code_name(pwkind),thing_model_name(thing),(int)thing->index);
                return false;
            }
            if (creature_affected_by_spell(thing, SplK_Teleport)) {
                SYNCDBG(8,"Player %d cannot cast %s on %s index %d while teleporting",(int)plyr_idx,power_code_name(pwkind),thing_model_name(thing),(int)thing->index);
                return false;
            }
        }
        // If allowed custody creatures - allow some enemies
        if ((powerst->can_cast_flags & PwCast_CustodyCrtrs) != 0)
        {
            if (creature_is_kept_in_custody_by_player(thing, plyr_idx)) {
                return true;
            }
        }
        if ((powerst->can_cast_flags & PwCast_OwnedCrtrs) != 0)
        {
            if (thing->owner == plyr_idx) {
                return true;
            }
        }
        if ((powerst->can_cast_flags & PwCast_AlliedCrtrs) != 0)
        {
            if (players_are_mutual_allies(plyr_idx, thing->owner)) {
                return true;
            }
        }
        if ((powerst->can_cast_flags & PwCast_EnemyCrtrs) != 0)
        {
            if (players_are_enemies(plyr_idx, thing->owner)) {
                return true;
            }
        }
    }
    SYNCDBG(18,"Player %d cannot cast %s on %s index %d, no condition met",(int)plyr_idx,power_code_name(pwkind),thing_model_name(thing),(int)thing->index);
    return false;
}

void update_power_sight_explored(struct PlayerInfo *player)
{
    SYNCDBG(16,"Starting");
    //_DK_update_power_sight_explored(player);
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);
    if (dungeon->sight_casted_thing_idx == 0) {
        return;
    }
    struct Thing *thing;
    thing = thing_get(dungeon->sight_casted_thing_idx);

    int shift_x;
    int shift_y;
    int i;
    int subshift_x;
    int subshift_y;
    int revealed;
    int stl_x;
    int stl_y;

    for (shift_y=0; shift_y < 2*MAX_SOE_RADIUS; shift_y++)
    {
        stl_y = thing->mappos.y.stl.num - MAX_SOE_RADIUS + shift_y;
        if ((stl_y < 0) || (stl_y > map_subtiles_y)) {
            continue;
        }

        stl_x = thing->mappos.x.stl.num - MAX_SOE_RADIUS;
        for (shift_x = 0; shift_x <= MAX_SOE_RADIUS; shift_x++)
        {
          if (dungeon->soe_explored_flags[shift_y][shift_x])
          {
            revealed = 0;
            i = 1;
            shift_x++;
            for (; shift_x < 2*MAX_SOE_RADIUS; shift_x++)
            {
              if (dungeon->soe_explored_flags[shift_y][shift_x])
                revealed = i;
              ++i;
            }

            int stl_x_beg;
            int stl_x_end;
            stl_x_beg = stl_x;
            stl_x_end = stl_x + revealed;
            if (stl_x_beg < 0) {
                stl_x_beg = 0;
            } else
            if (stl_x_beg > map_subtiles_x-1) {
                stl_x_beg = map_subtiles_x-1;
            }
            if (stl_x_end < 0) {
                stl_x_end = 0;
            } else
            if (stl_x_end > map_subtiles_x-1) {
                stl_x_end = map_subtiles_x-1;
            }
            if (stl_x_end >= stl_x_beg)
            {
                revealed = stl_x_end - stl_x_beg + 1;
                stl_x += revealed;
                subshift_x = stl_x_beg - thing->mappos.x.stl.num + MAX_SOE_RADIUS;
                for (;revealed > 0; revealed--)
                {
                    if (!dungeon->soe_explored_flags[shift_y][subshift_x])
                        dungeon->soe_explored_flags[shift_y][subshift_x] = 1;
                    subshift_x++;
                }
            }
          }
          stl_x++;
        }
    }

    for (shift_x = 0; shift_x < 2*MAX_SOE_RADIUS; shift_x++)
    {
      stl_x = thing->mappos.x.stl.num - MAX_SOE_RADIUS + shift_x;
      if ((stl_x < 0) || (stl_x > map_subtiles_x)) {
          continue;
      }
      stl_y = thing->mappos.y.stl.num - MAX_SOE_RADIUS;
      for (shift_y = 0; shift_y <= MAX_SOE_RADIUS; shift_y++)
      {
        if (dungeon->soe_explored_flags[shift_y][shift_x])
        {
            revealed = 0;
            i = 1;
            shift_y++;
            for (; shift_y < 2*MAX_SOE_RADIUS; shift_y++)
            {
              if (dungeon->soe_explored_flags[shift_y][shift_x])
                revealed = i;
              ++i;
            }

            int stl_y_beg;
            int stl_y_end;
            stl_y_beg = stl_y;
            stl_y_end = stl_y + revealed;
            if (stl_y_end < 0) {
                stl_y_end = 0;
            } else
            if (stl_y_end > map_subtiles_y-1) {
                stl_y_end = map_subtiles_y-1;
            }
            if (stl_y_beg < 0) {
                stl_y_beg = 0;
            } else
            if (stl_y_beg > map_subtiles_y-1) {
                stl_y_beg = map_subtiles_y-1;
            }
            if (stl_y_beg <= stl_y_end)
            {
                revealed = stl_y_end - stl_y_beg + 1;
                stl_y += revealed;
                subshift_y = stl_y_beg - thing->mappos.y.stl.num + MAX_SOE_RADIUS;
                for (; revealed > 0; revealed--)
                {
                    if (!dungeon->soe_explored_flags[subshift_y][shift_x])
                        dungeon->soe_explored_flags[subshift_y][shift_x] = 1;
                    subshift_y++;
                }
            }
        }
        stl_y++;
      }
    }

}

TbBool power_sight_explored(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon->sight_casted_thing_idx <= 0) {
        return false;
    }
    struct Thing *thing;
    thing = thing_get(dungeon->sight_casted_thing_idx);
    if (thing_is_invalid(thing)) {
        return false;
    }
    long soe_x;
    long soe_y;
    soe_x = stl_x - thing->mappos.x.stl.num + MAX_SOE_RADIUS;
    soe_y = stl_y - thing->mappos.y.stl.num + MAX_SOE_RADIUS;
    if ((soe_x < 0) || (soe_x >= 2*MAX_SOE_RADIUS) || (soe_y < 0)  || (soe_y >= 2*MAX_SOE_RADIUS))
      return false;
    return dungeon->soe_explored_flags[soe_y][soe_x];
}

void slap_creature(struct PlayerInfo *player, struct Thing *thing)
{
    struct CreatureStats *crstat;
    struct CreatureControl *cctrl;
    const struct MagicStats *pwrdynst;
    long i;
    crstat = creature_stats_get_from_thing(thing);
    cctrl = creature_control_get_from_thing(thing);

    anger_apply_anger_to_creature(thing, crstat->annoy_slapped, AngR_Other, 1);
    if (crstat->slaps_to_kill > 0)
    {
      i = compute_creature_max_health(crstat->health,cctrl->explevel) / crstat->slaps_to_kill;
      apply_damage_to_thing_and_display_health(thing, i, DmgT_Physical, player->id_number);
    }
    pwrdynst = get_power_dynamic_stats(PwrK_SLAP);
    i = cctrl->slap_turns;
    cctrl->slap_turns = pwrdynst->time;
    if (i == 0)
      cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
    if (thing->active_state != CrSt_CreatureSlapCowers)
    {
        clear_creature_instance(thing);
        cctrl->active_state_bkp = thing->active_state;
        cctrl->continue_state_bkp = thing->continue_state;
        creature_mark_if_woken_up(thing);
        external_set_thing_state(thing, CrSt_CreatureSlapCowers);
    }
    cctrl->field_B1 = 6;
    cctrl->field_27F = 18;
    play_creature_sound(thing, CrSnd_Hurt, 3, 0);
}

TbBool can_cast_power_at_xy(PlayerNumber plyr_idx, PowerKind pwkind,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long allow_flags)
{
    struct Map *mapblk;
    struct SlabMap *slb;
    unsigned long can_cast;
    mapblk = get_map_block_at(stl_x, stl_y);
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    struct SlabAttr *slbattr;
    slbattr = get_slab_attrs(slb);
    const struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(pwkind);
    if (power_model_stats_invalid(powerst))
        return false;
    can_cast = powerst->can_cast_flags | allow_flags;
    // Allow casting only on revealed tiles (unless the spell overrides this)
    if ((can_cast & PwCast_Unrevealed) == 0)
    {
        if (!map_block_revealed(mapblk, plyr_idx))
        {
            // If it's not revealed, we may still accept revealing by SOE spell
            if ((can_cast & PwCast_RevealedTemp) == 0) {
                return false;
            } else
            if (!power_sight_explored(stl_x, stl_y, plyr_idx)) {
                return false;
            }
        }
    }
    if ((can_cast & PwCast_Anywhere) != 0)
    {
        // If allowed casting anywhere, we're done
        return true;
    }
    if ((can_cast & PwCast_NeedsDelay) != 0)
    {
        struct PlayerInfo *player;
        player = get_player(plyr_idx);
        if (game.play_gameturn <= player->field_4E3+20) {
            return false;
        }
    }
    PlayerNumber slb_owner;
    slb_owner = slabmap_owner(slb);
    if ((mapblk->flags & SlbAtFlg_Blocking) != 0)
    {
        if ((can_cast & PwCast_Claimable) != 0)
        {
            if (((mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom|SlbAtFlg_Valuable)) != 0) || (slb->kind == SlbT_ROCK))
            {
                  return false;
            }
        }
        if ((can_cast & PwCast_AllTall) == PwCast_AllTall)
        {
            // If allowed all tall slab, we're good
            return true;
        }
        if ((can_cast & PwCast_NeutrlTall) != 0)
        {
            if (slb_owner == game.neutral_player_num) {
                return true;
            }
        }
        if ((can_cast & PwCast_OwnedTall) != 0)
        {
            if (slb_owner == plyr_idx) {
                return true;
            }
        }
        if ((can_cast & PwCast_AlliedTall) != 0)
        {
            if ((slb_owner != plyr_idx) && players_are_mutual_allies(plyr_idx,slb_owner)) {
                return true;
            }
        }
        if ((can_cast & PwCast_EnemyTall) != 0)
        {
            if (players_are_enemies(plyr_idx,slb_owner)) {
                return true;
            }
        }
    } else
    {
        if ((can_cast & PwCast_Claimable) != 0)
        {
            if (slab_kind_is_liquid(slb->kind))
            {
                  return false;
            }
        }
        if ((can_cast & PwCast_AllGround) == PwCast_AllGround)
        {
            // If allowed all ground slab, we're good
            return true;
        }
        if ((can_cast & PwCast_UnclmdGround) != 0)
        {
            if (slbattr->category == SlbAtCtg_Unclaimed) {
                return true;
            }
        }
        if ((can_cast & PwCast_NeutrlGround) != 0)
        {
            if ((slbattr->category != SlbAtCtg_Unclaimed) && (slb_owner == game.neutral_player_num)) {
                return true;
            }
        }
        if ((can_cast & PwCast_OwnedGround) != 0)
        {
            if ((slbattr->category != SlbAtCtg_Unclaimed) && (slb_owner == plyr_idx)) {
                return true;
            }
        }
        if ((can_cast & PwCast_AlliedGround) != 0)
        {
            if ((slbattr->category != SlbAtCtg_Unclaimed) && (slb_owner != plyr_idx) && players_are_mutual_allies(plyr_idx,slb_owner)) {
                return true;
            }
        }
        if ((can_cast & PwCast_EnemyGround) != 0)
        {
            if ((slbattr->category != SlbAtCtg_Unclaimed) && players_are_enemies(plyr_idx,slb_owner)) {
                return true;
            }
        }
    }
    return false;
}

/**
 * Computes price of a power which has price scaled with given amount.
 * @param plyr_idx Casting player index.
 * @param pwkind Keeper power kind.
 * @param pwlevel Keeper power overload level.
 * @param amount Amount used to scale the price; use 0 to get base price.
 */
GoldAmount compute_power_price_scaled_with_amount(PlayerNumber plyr_idx, PowerKind pwkind, long pwlevel, long amount)
{
    const struct MagicStats *pwrdynst;
    long i;
    pwrdynst = get_power_dynamic_stats(pwkind);
    // Increase price by given amount
    i = amount + 1;
    if (i < 1)
      i = 1;
    return pwrdynst->cost[pwlevel]*i/2;
}

/**
 * Computes current price of given power for given player.
 * @param plyr_idx Casting player index.
 * @param pwkind Keeper power kind.
 * @param pwlevel Keeper power overload level.
 */
GoldAmount compute_power_price(PlayerNumber plyr_idx, PowerKind pwkind, long pwlevel)
{
    struct Dungeon *dungeon;
    struct DungeonAdd* dungeonadd;
    const struct MagicStats *pwrdynst;
    long price;
    switch (pwkind)
    {
    case PwrK_MKDIGGER: // Special price algorithm for "create imp" spell
        dungeon = get_players_num_dungeon(plyr_idx);
        dungeonadd = get_dungeonadd(plyr_idx);
        // Increase price by amount of diggers, reduce by count of sacrificed diggers. Cheaper diggers may be a negative amount.
        price = compute_power_price_scaled_with_amount(plyr_idx, pwkind, pwlevel, dungeon->num_active_diggers - dungeonadd->cheaper_diggers);
        break;
    default:
        pwrdynst = get_power_dynamic_stats(pwkind);
        price = pwrdynst->cost[pwlevel];
        break;
    }
    return price;
}

/**
 * Computes lowest possible price of given power for given player.
 * @param plyr_idx Casting player index.
 * @param pwkind Keeper power kind.
 * @param pwlevel Keeper power overload level.
 */
GoldAmount compute_lowest_power_price(PlayerNumber plyr_idx, PowerKind pwkind, long pwlevel)
{
    const struct MagicStats *pwrdynst;
    long price;
    switch (pwkind)
    {
    case PwrK_MKDIGGER: // Special price algorithm for "create imp" spell
        // To get lowest
        price = compute_power_price_scaled_with_amount(plyr_idx, pwkind, pwlevel, 0);
        break;
    default:
        pwrdynst = get_power_dynamic_stats(pwkind);
        price = pwrdynst->cost[pwlevel];
        break;
    }
    return price;
}
long find_spell_age_percentage(PlayerNumber plyr_idx, PowerKind pwkind)
{
    struct Dungeon *dungeon;
    const struct MagicStats *pwrdynst;
    pwrdynst = get_power_dynamic_stats(pwkind);
    struct Thing * thing;
    thing = INVALID_THING;
    unsigned long curr;
    unsigned long total;
    curr = 0;
    total = 0;
    switch (pwkind)
    {
    case PwrK_SIGHT:
        dungeon = get_players_num_dungeon(plyr_idx);
        if (dungeon->sight_casted_thing_idx > 0)
            thing = thing_get(dungeon->sight_casted_thing_idx);
        if (thing_exists(thing)) {
            curr = game.play_gameturn - thing->creation_turn;
            total = pwrdynst->strength[dungeon->sight_casted_splevel] + 8;
        }
        break;
    case PwrK_CALL2ARMS:
        dungeon = get_players_num_dungeon(plyr_idx);
        if (dungeon->cta_start_turn != 0)
        {
            curr = game.play_gameturn - dungeon->cta_start_turn;
            total = pwrdynst->time;
        }
        break;
    default:
        break;
    }
    if (total > 0)
        return (curr << 8) / total;
    return -1;
}

TbBool pay_for_spell(PlayerNumber plyr_idx, PowerKind pwkind, long pwlevel)
{
    long price;
    if (pwkind >= POWER_TYPES_COUNT)
        return false;
    if (pwlevel >= MAGIC_OVERCHARGE_LEVELS)
        pwlevel = MAGIC_OVERCHARGE_LEVELS;
    if (pwlevel < 0)
        pwlevel = 0;
    price = compute_power_price(plyr_idx, pwkind, pwlevel);
    // Try to take money
    if (take_money_from_dungeon(plyr_idx, price, 1) >= 0)
    {
        return true;
    }
    // If failed, say "you do not have enough gold"
    if (is_my_player_number(plyr_idx))
        output_message(SMsg_GoldNotEnough, 0, true);
    return false;
}

TbBool find_power_cast_place(PlayerNumber plyr_idx, PowerKind pwkind, struct Coord3d *pos)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    switch (pwkind)
    {
    case PwrK_SIGHT:
        if (player_uses_power_sight(plyr_idx))
        {
            struct Thing *thing;
            thing = thing_get(dungeon->sight_casted_thing_idx);
            pos->x.val = thing->mappos.x.val;
            pos->y.val = thing->mappos.y.val;
            pos->z.val = thing->mappos.z.val;
            return true;
        }
        break;
    case PwrK_CALL2ARMS:
        if (player_uses_power_call_to_arms(plyr_idx))
        {
            pos->x.val = subtile_coord_center(dungeon->cta_stl_x);
            pos->y.val = subtile_coord_center(dungeon->cta_stl_y);
            pos->z.val = subtile_coord(1,0);
            return true;
        }
        break;
    }
    return false;
}

TbResult magic_use_power_armageddon(PlayerNumber plyr_idx, unsigned long mod_flags)
{
    SYNCDBG(6,"Starting");
    unsigned long your_time_gap;
    unsigned long enemy_time_gap;
    your_time_gap = game.armageddon.count_down + game.play_gameturn;
    enemy_time_gap = game.armageddon.count_down + game.play_gameturn;
    if (game.armageddon_cast_turn != 0) {
        return Lb_OK;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_ARMAGEDDON, 0)) {
            if (is_my_player_number(plyr_idx))
                output_message(SMsg_GoldNotEnough, 0, true);
            return Lb_OK;
        }
    }
    game.armageddon_cast_turn = game.play_gameturn;
    game.armageddon_caster_idx = plyr_idx;
    struct Thing *heartng;
    heartng = get_player_soul_container(plyr_idx);
    game.armageddon.mappos.x.val = heartng->mappos.x.val;
    game.armageddon.mappos.y.val = heartng->mappos.y.val;
    game.armageddon.mappos.z.val = heartng->mappos.z.val;

    struct Thing *thing;
    int i;
    int k;
    k = 0;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Creature);
    i = slist->index;
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
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        // Creatures unaffected by Armageddon
        if (is_neutral_thing(thing) && !gameadd.armegeddon_teleport_neutrals)
        {
            cctrl->armageddon_teleport_turn = 0;
        } else
        // Creatures killed by Armageddon
        if (creature_affected_by_spell(thing, SplK_Chicken))
        {
            kill_creature(thing, heartng, plyr_idx, CrDed_DiedInBattle);
        } else
        // Creatures teleported by Armageddon
        {
            cctrl->armageddon_teleport_turn = your_time_gap;
            if (thing->owner == plyr_idx) {
                your_time_gap += game.armagedon_teleport_your_time_gap;
            } else {
                enemy_time_gap += game.armagedon_teleport_enemy_time_gap;
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    if (enemy_time_gap <= your_time_gap)
        enemy_time_gap = your_time_gap;
    game.armageddon_field_15035A = game.armageddon.duration + enemy_time_gap;
    struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(PwrK_ARMAGEDDON);
    play_non_3d_sample(powerst->select_sound_idx);
    return Lb_SUCCESS;
}

/**
 * Starts and stops the use of Must obey.
 * What differs this power from others is that it is a toggle - pressing once
 * starts the power, and second press disables it.
 * The spell is paid for somewhere else - it takes money every few turns when active.
 * @param plyr_idx
 * @param mod_flags
 * @return
 */
TbResult magic_use_power_obey(PlayerNumber plyr_idx, unsigned long mod_flags)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    // Toggle the spell
    if (dungeon->must_obey_turn != 0) {
        dungeon->must_obey_turn = 0;
    } else {
        dungeon->must_obey_turn = game.play_gameturn;
        struct PowerConfigStats *powerst;
        powerst = get_power_model_stats(PwrK_OBEY);
        play_non_3d_sample(powerst->select_sound_idx);
    }
    update_speed_of_player_creatures_of_model(plyr_idx, 0);
    return Lb_SUCCESS;
}

void turn_off_power_obey(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    dungeon->must_obey_turn = 0;
    update_speed_of_player_creatures_of_model(plyr_idx, 0);
}

void turn_off_power_sight_of_evil(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    long spl_lev;
    long cit;
    long i;
    long imax;
    long k;
    long n;
    dungeon = get_players_num_dungeon(plyr_idx);
    const struct MagicStats *pwrdynst;
    pwrdynst = get_power_dynamic_stats(PwrK_SIGHT);
    spl_lev = dungeon->sight_casted_splevel;
    if (spl_lev > SPELL_MAX_LEVEL)
        spl_lev = SPELL_MAX_LEVEL;
    i = game.play_gameturn - dungeon->sight_casted_gameturn;
    imax = abs(pwrdynst->strength[spl_lev]/4) >> 2;
    if (i > imax)
        i = imax;
    if (i < 0)
        i = 0;
    n = game.play_gameturn - pwrdynst->strength[spl_lev];
    cit = power_sight_close_instance_time[spl_lev];
    k = imax / cit;
    if (k < 1) k = 1;
    dungeon->sight_casted_gameturn = n + i/k - cit;
}

TbResult magic_use_power_hold_audience(PlayerNumber plyr_idx, unsigned long mod_flags)
{
    SYNCDBG(8,"Starting");
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon->hold_audience_cast_turn != 0) {
        return Lb_OK;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_HOLDAUDNC, 0)) {
            return Lb_FAIL;
        }
    }
    dungeon->hold_audience_cast_turn = game.play_gameturn;
    unsigned long k;
    int i;
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct CreatureControl *cctrl;
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!thing_is_picked_up(thing) && !creature_is_kept_in_custody(thing) && !creature_is_being_unconscious(thing))
        {
            create_effect(&thing->mappos, imp_spangle_effects[thing->owner], thing->owner);
            const struct Coord3d *pos;
            pos = dungeon_get_essential_pos(thing->owner);
            move_thing_in_map(thing, pos);
            initialise_thing_state(thing, CrSt_CreatureInHoldAudience);
            cctrl->turns_at_job = -1;
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(PwrK_HOLDAUDNC);
    play_non_3d_sample(powerst->select_sound_idx);
    SYNCDBG(19,"Finished");
    return Lb_SUCCESS;
}

TbResult magic_use_power_chicken(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags)
{
    // If this spell is already casted at that creature, do nothing
    if (thing_affected_by_spell(thing, SplK_Chicken)) {
        return Lb_OK;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_CHICKEN, splevel)) {
            return Lb_FAIL;
        }
    }
    // Check if the creature kind isn't affected by that spell
    if ((get_creature_model_flags(thing) & CMF_NeverChickens) != 0)
    {
        thing_play_sample(thing, 58, 20, 0, 3, 0, 2, 128);
        return Lb_SUCCESS;
    }
    apply_spell_effect_to_thing(thing, SplK_Chicken, splevel);
    struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(PwrK_CHICKEN);
    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    return Lb_SUCCESS;
}

TbResult magic_use_power_disease(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags)
{
    // If this spell is already casted at that creature, do nothing
    if (thing_affected_by_spell(thing, SplK_Disease)) {
        return Lb_OK;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_DISEASE, splevel)) {
            return Lb_FAIL;
        }
    }
    // Check if the creature kind isn't affected by that spell
    if ((get_creature_model_flags(thing) & CMF_NeverSick) != 0)
    {
        thing_play_sample(thing, 58, 20, 0, 3, 0, 2, 128);
        return Lb_SUCCESS;
    }
    apply_spell_effect_to_thing(thing, SplK_Disease, splevel);
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        cctrl->disease_caster_plyridx = plyr_idx;
    }
    struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(PwrK_DISEASE);
    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
    return Lb_SUCCESS;
}

TbResult magic_use_power_destroy_walls(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags)
{
    // If we can't afford the spell, fail
    SYNCDBG(16,"Starting");
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_DESTRWALLS, splevel)) {
            return Lb_FAIL;
        }
    }
    MapSlabCoord slb_x_start;
    MapSlabCoord slb_y_start;
    MapSlabCoord slb_x_end;
    MapSlabCoord slb_y_end;
    int i;
    slb_x_start = map_to_slab[stl_x] - 1;
    slb_y_start = map_to_slab[stl_y] - 1;
    slb_x_end = slb_x_start + 3;
    slb_y_end = slb_y_start + 3;
    i = 0;
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    for (slb_y=slb_y_start; slb_y < slb_y_end ; slb_y++)
    {
        for (slb_x=slb_x_start; slb_x < slb_x_end ; slb_x++,i++)
        {
            struct SlabMap *slb;
            slb = get_slabmap_block(slb_x, slb_y);
            if (slabmap_block_invalid(slb))
                continue;
            struct Map *mapblk;
            mapblk = get_map_block_at(slab_subtile_center(slb_x),slab_subtile_center(slb_y));
            if (!(mapblk->flags & SlbAtFlg_Blocking) || (mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom|SlbAtFlg_Valuable)) || (slb->kind == SlbT_ROCK) )
              continue;
            TbBool is_revealed;
            is_revealed = subtile_revealed(stl_x, stl_y, plyr_idx);
            unsigned char destreff;
            destreff = destroy_effect[splevel][i];
            if (destreff == 79)
            {
                struct SlabAttr *slbattr;
                slbattr = get_slab_attrs(slb);
                if (slbattr->category == SlbAtCtg_FortifiedWall)
                {
                    place_slab_type_on_map(SlbT_EARTH, slab_subtile_center(slb_x),slab_subtile_center(slb_y), plyr_idx, 0);
                    do_slab_efficiency_alteration(slb_x, slb_y);
                } else
                if (slab_kind_is_friable_dirt(slb->kind))
                {
                    dig_out_block(slab_subtile_center(slb_x),slab_subtile_center(slb_y), plyr_idx);
                    if (is_revealed) {
                        set_slab_explored(plyr_idx, slb_x, slb_y);
                    }
                    clear_slab_dig(slb_x, slb_y, plyr_idx);
                }
            } else
            if (destreff == 32)
            {
                dig_out_block(slab_subtile_center(slb_x),slab_subtile_center(slb_y), plyr_idx);
                if (is_revealed) {
                    set_slab_explored(plyr_idx, slb_x, slb_y);
                }
                clear_slab_dig(slb_x, slb_y, plyr_idx);
            }
        }
    }
    if (is_my_player_number(plyr_idx))
    {
        struct PowerConfigStats *powerst;
        powerst = get_power_model_stats(PwrK_DESTRWALLS);
        play_non_3d_sample(powerst->select_sound_idx);
    }
    return Lb_SUCCESS;
}

TbResult magic_use_power_time_bomb(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags)
{
    struct Thing *thing;
    struct Coord3d pos;
    struct PowerConfigStats *powerst;
    if (!i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots)) {
        return Lb_FAIL;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_TIMEBOMB, 0)) {
            return Lb_FAIL;
        }
    }
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = get_floor_height_at(&pos) + 512;
    //TODO SPELL TIMEBOMB write the spell support
    thing = INVALID_THING;//create_object(&pos, , plyr_idx);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("There was place to create new thing, but creation failed");
        return Lb_OK;
    }
    thing->veloc_push_add.x.val += CREATURE_RANDOM(thing, 321) - 160;
    thing->veloc_push_add.y.val += CREATURE_RANDOM(thing, 321) - 160;
    thing->veloc_push_add.z.val += 40;
    thing->state_flags |= TF1_PushAdd;
    powerst = get_power_model_stats(PwrK_TIMEBOMB);
    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    return Lb_SUCCESS;
}

TbResult magic_use_power_imp(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long mod_flags)
{
    struct Thing *thing;
    struct Thing *heartng;
    struct Coord3d pos;
    struct PowerConfigStats *powerst;
    if (!i_can_allocate_free_control_structure()
     || !i_can_allocate_free_thing_structure(FTAF_FreeEffectIfNoSlots)) {
        return Lb_FAIL;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_MKDIGGER, 0)) {
            return Lb_FAIL;
        }
    }
    heartng = get_player_soul_container(plyr_idx);
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = get_floor_height_at(&pos) + (heartng->clipbox_size_yz >> 1);
    thing = create_creature(&pos, get_players_special_digger_model(plyr_idx), plyr_idx);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("There was place to create new creature, but creation failed");
        return Lb_OK;
    }
    EVM_CREATURE_EVENT("joined", plyr_idx, thing);
    thing->veloc_push_add.x.val += CREATURE_RANDOM(thing, 161) - 80;
    thing->veloc_push_add.y.val += CREATURE_RANDOM(thing, 161) - 80;
    thing->veloc_push_add.z.val += 160;
    thing->state_flags |= TF1_PushAdd;
    thing->move_angle_xy = 0;
    initialise_thing_state(thing, CrSt_ImpBirth);
    powerst = get_power_model_stats(PwrK_MKDIGGER);
    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    play_creature_sound(thing, 3, 2, 0);
    return Lb_SUCCESS;
}

TbResult magic_use_power_heal(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags)
{
    // If the creature has full health, do nothing
    if (!thing_is_creature(thing)) {
        ERRORLOG("Tried to apply spell to invalid creature.");
        return Lb_FAIL;
    }
    if (get_creature_health_permil(thing) >= 1000) {
        SYNCDBG(7,"Full health, can't heal %s.",thing_model_name(thing));
        return Lb_OK;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_HEALCRTR, splevel)) {
            ERRORLOG("No gold to heal %s.",thing_model_name(thing));
            return Lb_FAIL;
        }
    }
    // Apply spell effect
    struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(PwrK_HEALCRTR);
    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    apply_spell_effect_to_thing(thing, SplK_Heal, splevel);
    return Lb_SUCCESS;
}

TbResult magic_use_power_conceal(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags)
{
    if (!thing_is_creature(thing)) {
        ERRORLOG("Tried to apply spell to invalid creature.");
        return Lb_FAIL;
    }
    // If this spell is already casted at that creature, do nothing
    if (thing_affected_by_spell(thing, SplK_Invisibility)) {
        return Lb_OK;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_CONCEAL, splevel)) {
            return Lb_FAIL;
        }
    }
    struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(PwrK_CONCEAL);
    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    apply_spell_effect_to_thing(thing, SplK_Invisibility, splevel);
    return Lb_SUCCESS;
}

TbResult magic_use_power_armour(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags)
{
    if (!thing_is_creature(thing)) {
        ERRORLOG("Tried to apply spell to invalid creature.");
        return Lb_FAIL;
    }
    // If this spell is already casted at that creature, do nothing
    if (thing_affected_by_spell(thing, SplK_Armour)) {
        return Lb_OK;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_PROTECT, splevel)) {
            return Lb_FAIL;
        }
    }
    struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(PwrK_PROTECT);
    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    apply_spell_effect_to_thing(thing, SplK_Armour, splevel);
    return Lb_SUCCESS;
}

TbResult magic_use_power_speed(PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags)
{
    if (!thing_is_creature(thing)) {
        ERRORLOG("Tried to apply spell to invalid creature.");
        return Lb_FAIL;
    }
    if (thing_affected_by_spell(thing, SplK_Speed)) {
        return Lb_OK;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_SPEEDCRTR, splevel)) {
            return Lb_FAIL;
        }
    }
    struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(PwrK_SPEEDCRTR);
    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    apply_spell_effect_to_thing(thing, SplK_Speed, splevel);
    return Lb_SUCCESS;
}

TbResult magic_use_power_lightning(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    const struct MagicStats *pwrdynst;
    struct ShotConfigStats *shotst;
    struct Thing *shtng;
    struct Thing *obtng;
    struct Thing *efftng;
    struct Coord3d pos;
    long range;
    long max_damage;
    long i;
    player = get_player(plyr_idx);
    dungeon = get_dungeon(player->id_number);
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = 0;
    // make sure the spell level is correct
    if (splevel >= MAGIC_OVERCHARGE_LEVELS)
        splevel = MAGIC_OVERCHARGE_LEVELS-1;
    if (splevel < 0)
        splevel = 0;
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_LIGHTNING, splevel)) {
            return Lb_FAIL;
        }
    }
    // And cast it
    shtng = create_shot(&pos, ShM_GodLightning, plyr_idx);
    if (!thing_is_invalid(shtng))
    {
        shtng->mappos.z.val = get_thing_height_at(shtng, &shtng->mappos) + COORD_PER_STL/2;
        shtng->shot.hit_type = THit_CrtrsOnly;
        shtng->shot.byte_19 = splevel;
    }
    pwrdynst = get_power_dynamic_stats(PwrK_LIGHTNING);
    shotst = get_shot_model_stats(ShM_GodLightning);
    dungeon->camera_deviate_jump = 256;
    i = pwrdynst->strength[splevel];
    max_damage = i * shotst->damage;
    range = (i << 8) / 2;
    if (power_sight_explored(stl_x, stl_y, plyr_idx))
        max_damage /= 4;
    obtng = create_object(&pos, 124, plyr_idx, -1);
    if (!thing_is_invalid(obtng))
    {
        obtng->byte_13 = splevel;
        obtng->field_4F |= TF4F_Unknown01;
    }
    i = electricity_affecting_area(&pos, plyr_idx, range, max_damage);
    SYNCDBG(9,"Affected %ld targets within range %ld, damage %ld",i,range,max_damage);
    if (!thing_is_invalid(shtng))
    {
        efftng = create_effect(&shtng->mappos, TngEff_DamageBlood, shtng->owner);
        if (!thing_is_invalid(efftng))
        {
            struct PowerConfigStats *powerst;
            powerst = get_power_model_stats(PwrK_LIGHTNING);
            thing_play_sample(efftng, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        }
    }
    player->field_4E3 = game.play_gameturn;
    return Lb_SUCCESS;
}

TbResult magic_use_power_sight(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags)
{
    const struct MagicStats *pwrdynst;
    struct Dungeon *dungeon;
    struct Thing *thing;
    struct Coord3d pos;
    long cit;
    long cdt;
    long cgt;
    long cdlimit;
    long i;
    dungeon = get_dungeon(plyr_idx);
    pwrdynst = get_power_dynamic_stats(PwrK_SIGHT);
    if (player_uses_power_sight(plyr_idx))
    {
        cdt = game.play_gameturn - dungeon->sight_casted_gameturn;
        cdlimit = pwrdynst->strength[dungeon->sight_casted_splevel] >> 4;
        if (cdt < 0) {
            cdt = 0;
        } else
        if (cdt > cdlimit) {
            cdt = cdlimit;
        }
        cit = power_sight_close_instance_time[dungeon->sight_casted_splevel];
        cgt = game.play_gameturn - pwrdynst->strength[dungeon->sight_casted_splevel];
        i = cdlimit / cit;
        if (i > 0) {
            dungeon->sight_casted_gameturn = cgt + cdt/i - cit;
        } else {
            dungeon->sight_casted_gameturn = cgt;
        }
        thing = thing_get(dungeon->sight_casted_thing_idx);
        if (cgt < (long)thing->creation_turn)
        {
            dungeon->computer_enabled |= 0x04;
            dungeon->sight_casted_stl_x = stl_x;
            dungeon->sight_casted_stl_y = stl_y;
        }
        return Lb_OK;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the spell, fail
        if (!pay_for_spell(plyr_idx, PwrK_SIGHT, splevel)) {
            if (is_my_player_number(plyr_idx))
                output_message(SMsg_GoldNotEnough, 0, true);
            return Lb_FAIL;
        }
    }
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = subtile_coord_center(5);
    thing = create_object(&pos, 123, plyr_idx, -1);
    if (!thing_is_invalid(thing))
    {
        struct PowerConfigStats *powerst;
        powerst = get_power_model_stats(PwrK_SIGHT);
        dungeon->sight_casted_gameturn = game.play_gameturn;
        thing->health = 2;
        dungeon->sight_casted_splevel = splevel;
        dungeon->sight_casted_thing_idx = thing->index;
        LbMemorySet(dungeon->soe_explored_flags, 0, sizeof(dungeon->soe_explored_flags));
        thing->field_4F |= TF4F_Unknown01;
        thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, -1, 3, 0, 3, FULL_LOUDNESS);
    }
    return Lb_SUCCESS;
}

TbResult magic_use_power_cave_in(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags)
{
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slb_y = subtile_slab_fast(stl_y);
    slb_x = subtile_slab_fast(stl_x);
    struct Map *mapblk;
    mapblk = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    long i;
    unsigned long k;
    k = 0;
    i = get_mapwho_thing_index(mapblk);
    struct Thing *thing;
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if ((thing->class_id == TCls_EffectElem) && (thing->model == 46)) {
            break;
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            i = 0;
            break;
        }
    }
    if (i == 0)
    {
        if (plyr_idx != game.neutral_player_num)
        {
            if ((mod_flags & PwMod_CastForFree) == 0)
            {
                // If we can't afford the spell, fail
                if (!pay_for_spell(plyr_idx, PwrK_CAVEIN, splevel)) {
                    return Lb_FAIL;
                }
            }
            struct Dungeon *dungeon;
            dungeon = get_players_num_dungeon(plyr_idx);
            dungeon->lvstats.num_caveins++;
        }
        struct Coord3d pos;
        pos.x.val = subtile_coord_center(slab_subtile_center(slb_x));
        pos.y.val = subtile_coord_center(slab_subtile_center(slb_y));
        pos.z.val = 0;
        thing = create_thing(&pos, TCls_CaveIn, splevel, plyr_idx, -1);
        struct PowerConfigStats *powerst;
        powerst = get_power_model_stats(PwrK_CAVEIN);
        thing_play_sample(thing, powerst->select_sound_idx, 25, 0, 3, 0, 2, FULL_LOUDNESS);
    }
    return Lb_SUCCESS;
}

/**
 * Changes creature state and marks it as being affected by CTA spell.
 *
 * @param cta_pos Position where the CTA spell is casted.
 * @param creatng The target creature thing.
 * @return
 */
TbBool update_creature_influenced_by_call_to_arms_at_pos(struct Thing *creatng, const struct Coord3d *cta_pos)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    if (!creature_can_navigate_to_with_storage(creatng, cta_pos, NavRtF_Default) || process_creature_needs_to_heal_critical(creatng) || (creatng->continue_state == CrSt_CreatureCombatFlee))
    {
        creature_stop_affected_by_call_to_arms(creatng);
        return false;
    }
    if (!creature_is_called_to_arms(creatng))
    {
        if (!external_set_thing_state(creatng, CrSt_ArriveAtCallToArms))
        {
            return false;
        }
    }
    setup_person_move_to_coord(creatng, cta_pos, NavRtF_Default);
    creatng->continue_state = CrSt_ArriveAtCallToArms;
    cctrl->spell_flags |= CSAfF_CalledToArms;
    if ((cctrl->flgfield_1 & CCFlg_NoCompControl) != 0) {
        WARNLOG("The %s index %d is called to arms with no comp control, fixing",thing_model_name(creatng),(int)creatng->index);
        cctrl->flgfield_1 &= ~CCFlg_NoCompControl;
    }
    return true;
}

long update_creatures_influenced_by_call_to_arms(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    SYNCDBG(8,"Starting");
    dungeon = get_players_num_dungeon(plyr_idx);
    struct Coord3d cta_pos;
    cta_pos.x.val = subtile_coord_center(dungeon->cta_stl_x);
    cta_pos.y.val = subtile_coord_center(dungeon->cta_stl_y);
    cta_pos.z.val = get_floor_height_at(&cta_pos);
    long count;
    count = 0;
    unsigned long k;
    int i;
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct Thing *thing;
        struct CreatureControl *cctrl;
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!thing_is_picked_up(thing) && !creature_is_being_unconscious(thing))
        {
            if (creature_affected_by_call_to_arms(thing))
            {
                struct StateInfo *stati;
                stati = get_thing_state_info_num(get_creature_state_besides_interruptions(thing));
                if (stati->react_to_cta || creature_is_called_to_arms(thing))
                {
                    if (update_creature_influenced_by_call_to_arms_at_pos(thing, &cta_pos)) {
                        count++;
                    } else {
                        set_start_state(thing);
                    }
                }
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
    return count;
}

/**
 * Casts CTA on given map coordinates.
 * Does no castability checking.
 * To use the casting, higher level function should be used.
 * @param plyr_idx The casting player.
 * @param stl_x The target subtile, X coord.
 * @param stl_y The target subtile, Y coord.
 * @param splevel Power overcharge level.
 * @return
 * @see magic_use_available_power_on_thing()
 * @see magic_use_available_power_on_subtile()
 */
TbResult magic_use_power_call_to_arms(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long splevel, unsigned long mod_flags)
{
    struct Dungeon *dungeon;
    struct PlayerInfo *player;
    SYNCDBG(8,"Starting");
    player = get_player(plyr_idx);
    dungeon = get_players_dungeon(player);
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = get_floor_height_at(&pos);
    struct Thing *objtng;
    objtng = thing_get(player->field_43C);
    if ((dungeon->cta_start_turn == 0) || !thing_is_object(objtng))
    {
          objtng = create_object(&pos, 24, plyr_idx, -1);
          if (thing_is_invalid(objtng)) {
              ERRORLOG("Cannot create call to arms");
              return 0;
          }
          dungeon->cta_start_turn = game.play_gameturn;
          dungeon->cta_splevel = splevel;
          dungeon->cta_stl_x = stl_x;
          dungeon->cta_stl_y = stl_y;
          player->field_43C = objtng->index;
          objtng->mappos.z.val = get_thing_height_at(objtng, &objtng->mappos);
          set_call_to_arms_as_birthing(objtng);
          SYNCDBG(9,"Created birthing CTA");
          return 1;
    }
    dungeon->cta_start_turn = game.play_gameturn;
    dungeon->cta_stl_x = stl_x;
    dungeon->cta_stl_y = stl_y;
    set_call_to_arms_as_rebirthing(objtng);
    update_creatures_influenced_by_call_to_arms(plyr_idx);
    SYNCDBG(19,"Finished");
    return 1;
}

TbResult magic_use_power_slap_thing(PlayerNumber plyr_idx, struct Thing *thing, unsigned long mod_flags)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    if (!thing_exists(thing)) {
        return Lb_FAIL;
    }
    player = get_player(plyr_idx);
    dungeon = get_dungeon(player->id_number);
    if ((player->instance_num == PI_Whip) || (game.play_gameturn - dungeon->last_creature_dropped_gameturn <= 10)) {
        return Lb_OK;
    }
    EVM_CREATURE_EVENT("slap", plyr_idx, thing);
    player->influenced_thing_idx = thing->index;
    player->influenced_thing_creation = thing->creation_turn;
    set_player_instance(player, PI_Whip, 0);
    dungeon->lvstats.num_slaps++;
    return Lb_SUCCESS;
}

/**
 * Get a thing and slap it.
 * @deprecated To be removed and replaced by magic_use_available_power_on_thing() call.
 * @param plyr_idx
 * @param stl_x
 * @param stl_y
 * @return
 */
TbResult magic_use_power_slap(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long mod_flags)
{
    struct Thing *thing;
    thing = get_nearest_thing_for_slap(plyr_idx, subtile_coord_center(stl_x), subtile_coord_center(stl_y));
    return magic_use_power_slap_thing(plyr_idx, thing, PwMod_Default);
}

TbResult magic_use_power_possess_thing(PlayerNumber plyr_idx, struct Thing *thing, unsigned long mod_flags)
{
    struct PlayerInfo *player;
    if (!thing_exists(thing)) {
        return Lb_FAIL;
    }
    player = get_player(plyr_idx);
    player->influenced_thing_idx = thing->index;
    teleport_destination = 18;
    first_person_dig_claim_mode = false;
    battleid = 1;
    // Note that setting Direct Control player instance requires player->influenced_thing_idx to be set correctly
    set_player_instance(player, PI_DirctCtrl, 0);
    return Lb_SUCCESS;
}

void magic_power_hold_audience_update(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    SYNCDBG(8,"Starting");
    if ( game.play_gameturn - dungeon->hold_audience_cast_turn <= game.hold_audience_time) {
        return;
    }
    // Dispose hold audience effect
    dungeon->hold_audience_cast_turn = 0;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    int i;
    dungeon = get_players_num_dungeon(plyr_idx);
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (get_creature_state_besides_interruptions(thing) == CrSt_CreatureInHoldAudience) {
            set_start_state(thing);
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    SYNCDBG(19,"Finished");
}

TbBool affect_creature_by_power_call_to_arms(struct Thing *creatng, long range, const struct Coord3d *cta_pos)
{
    int nstat;
    nstat = get_creature_state_besides_interruptions(creatng);
    struct StateInfo *stati;
    stati = get_thing_state_info_num(nstat);
    if (!creature_affected_by_call_to_arms(creatng) || stati->react_to_cta)
    {
        if (stati->react_to_cta
          && (creature_affected_by_call_to_arms(creatng) || get_2d_box_distance(&creatng->mappos, cta_pos) < range))
        {
            creature_mark_if_woken_up(creatng);
            if (update_creature_influenced_by_call_to_arms_at_pos(creatng, cta_pos)) {
                return true;
            }
        }
    }
    return false;
}

int affect_nearby_creatures_by_power_call_to_arms(PlayerNumber plyr_idx, long range, const struct Coord3d * pos)
{
    struct Dungeon *dungeon;
    unsigned long k;
    int i;
    int n;
    SYNCDBG(8,"Starting");
    dungeon = get_players_num_dungeon(plyr_idx);
    n = 0;
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!thing_is_picked_up(thing) && !creature_is_kept_in_custody(thing) &&
            !creature_is_being_unconscious(thing) && !creature_is_dying(thing))
        {
            if (affect_creature_by_power_call_to_arms(thing, range, pos)) {
                n++;
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
    SYNCDBG(19,"Finished");
    return n;
}

void process_magic_power_call_to_arms(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    long duration;
    duration = game.play_gameturn - dungeon->cta_start_turn;
    const struct MagicStats *pwrdynst;
    pwrdynst = get_power_dynamic_stats(PwrK_CALL2ARMS);

    struct SlabMap *slb;
    slb = get_slabmap_for_subtile(dungeon->cta_stl_x, dungeon->cta_stl_y);
    if (((pwrdynst->time < 1) || ((duration % pwrdynst->time) == 0)) && (slabmap_owner(slb) != plyr_idx))
    {
        if (!pay_for_spell(plyr_idx, PwrK_CALL2ARMS, dungeon->cta_splevel)) {
            if (is_my_player_number(plyr_idx))
                output_message(SMsg_GoldNotEnough, 0, true);
            turn_off_power_call_to_arms(plyr_idx);
            return;
        }
    }
    if ((duration % 16) == 0)
    {
        long range;
        range = subtile_coord(pwrdynst->strength[dungeon->cta_splevel],0);
        struct Coord3d cta_pos;
        cta_pos.x.val = subtile_coord_center(dungeon->cta_stl_x);
        cta_pos.y.val = subtile_coord_center(dungeon->cta_stl_y);
        cta_pos.z.val = subtile_coord(1,0);
        affect_nearby_creatures_by_power_call_to_arms(plyr_idx, range, &cta_pos);
    }
}

void process_magic_power_must_obey(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    long delta;
    delta = game.play_gameturn - dungeon->must_obey_turn;
    const struct MagicStats *pwrdynst;
    pwrdynst = get_power_dynamic_stats(PwrK_OBEY);
    if ((delta % pwrdynst->time) == 0)
    {
        if (!pay_for_spell(plyr_idx, PwrK_OBEY, 0)) {
            magic_use_power_obey(plyr_idx, PwMod_Default);
        }
    }
}

void process_dungeon_power_magic(void)
{
    SYNCDBG(8,"Starting");
    //_DK_process_dungeon_power_magic();
    long i;
    for (i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo *player;
        player = get_player(i);
        if (player_exists(player))
        {
            if (player_uses_power_call_to_arms(i))
            {
                process_magic_power_call_to_arms(i);
            }
            if (player_uses_power_hold_audience(i))
            {
                magic_power_hold_audience_update(i);
            }
            if (player_uses_power_obey(i))
            {
                process_magic_power_must_obey(i);
            }
            if (game.armageddon_cast_turn > 0)
            {
                if (game.play_gameturn > game.armageddon_field_15035A)
                {
                  game.armageddon_cast_turn = 0;
                  game.armageddon_field_15035A = 0;
                }
            }
        }
    }
}

/**
 * Unified function for using powers which are castable on things.
 *
 * @param plyr_idx The casting player.
 * @param spl_idx Power kind to be casted.
 * @param splevel Power overcharge level.
 * @param thing The target thing.
 * @param stl_x The casting subtile, X coord.
 * @param stl_y The casting subtile, Y coord.
 */
TbResult magic_use_available_power_on_thing(PlayerNumber plyr_idx, PowerKind pwkind,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing, unsigned long allow_flags)
{
    TbResult ret;
    if (!is_power_available(plyr_idx, pwkind)) {
        // It shouldn't be possible to select unavailable spell
        WARNLOG("Player %d tried to cast %s which is unavailable",(int)plyr_idx,power_code_name(pwkind));
        ret = Lb_FAIL;
    }
    else
    {
        ret = magic_use_power_on_thing(plyr_idx, pwkind, splevel, stl_x, stl_y, thing, allow_flags);
    }
    if (ret == Lb_FAIL) {
        // Make a rejection sound
        if (is_my_player_number(plyr_idx))
        {
            play_non_3d_sample(119);
        }
    }
    return ret;
}

/**
 * Unified function for using powers which are castable on things. Without checks for availiability
 *
 * @param plyr_idx The casting player.
 * @param spl_idx Power kind to be casted.
 * @param splevel Power overcharge level.
 * @param thing The target thing.
 * @param stl_x The casting subtile, X coord.
 * @param stl_y The casting subtile, Y coord.
 */
TbResult magic_use_power_on_thing(PlayerNumber plyr_idx, PowerKind pwkind,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing, unsigned long allow_flags)
{
    TbResult ret;
    ret = Lb_OK;
    if (!thing_exists(thing)) {
        WARNLOG("Player %d tried to cast %s on non-existing thing",(int)plyr_idx,power_code_name(pwkind));
        ret = Lb_FAIL;
    }
    if (ret == Lb_OK)
    {// Zero coords mean we should take real ones from the thing. But even if they're not zero, we might want to fix them sometimes
        const struct PowerConfigStats *powerst;
        powerst = get_power_model_stats(pwkind);
        if (((stl_x == 0) && (stl_y == 0)) || ((powerst->can_cast_flags & PwCast_AllThings) != 0)) {
            stl_x = thing->mappos.x.stl.num;
            stl_y = thing->mappos.y.stl.num;
        }
    }
    if (ret == Lb_OK)
    {
        if (!can_cast_spell(plyr_idx, pwkind, stl_x, stl_y, thing, CastChk_Final | CastChk_SkipAvailiabilty)) {
            ret = Lb_FAIL;
        }
    }
    if (ret == Lb_OK)
    {
        if (splevel > MAGIC_OVERCHARGE_LEVELS) {
            WARNLOG("Overcharge level %d out of range, adjusting",(int)splevel);
            splevel = MAGIC_OVERCHARGE_LEVELS;
        }
    }
    if (ret == Lb_OK)
    {
        switch (pwkind)
        {
        case PwrK_HAND:
            //Note that we shouldn't use magic_use_power_hand(), that function is for interpreting input
            if (power_hand_is_full(get_player(plyr_idx)))
                ret = Lb_FAIL;
            else
            if (place_thing_in_power_hand(thing, plyr_idx))
                ret = Lb_SUCCESS;
            else
                ret = Lb_FAIL;
            break;
        case PwrK_HEALCRTR:
            ret = magic_use_power_heal(plyr_idx, thing, stl_x, stl_y, splevel, allow_flags);
            break;
        case PwrK_SPEEDCRTR:
            ret = magic_use_power_speed(plyr_idx, thing, stl_x, stl_y, splevel, allow_flags);
            break;
        case PwrK_PROTECT:
            ret = magic_use_power_armour(plyr_idx, thing, stl_x, stl_y, splevel, allow_flags);
            break;
        case PwrK_CONCEAL:
            ret = magic_use_power_conceal(plyr_idx, thing, stl_x, stl_y, splevel, allow_flags);
            break;
        case PwrK_DISEASE:
            ret = magic_use_power_disease(plyr_idx, thing, stl_x, stl_y, splevel, allow_flags);
            break;
        case PwrK_CHICKEN:
            ret = magic_use_power_chicken(plyr_idx, thing, stl_x, stl_y, splevel, allow_flags);
            break;
        case PwrK_SLAP:
            ret = magic_use_power_slap_thing(plyr_idx, thing, allow_flags);
            break;
        case PwrK_POSSESS:
            ret = magic_use_power_possess_thing(plyr_idx, thing, allow_flags);
            break;
        case PwrK_CALL2ARMS:
            ret = magic_use_power_call_to_arms(plyr_idx, stl_x, stl_y, splevel, allow_flags);
            break;
        case PwrK_LIGHTNING:
            ret = magic_use_power_lightning(plyr_idx, stl_x, stl_y, splevel, allow_flags);
            break;
        default:
            ERRORLOG("Power not supported here: %d", (int)pwkind);
            ret = Lb_FAIL;
            break;
        }
    }
    return ret;
}

/**
 * Unified function for using powers which are castable on map subtile.
 *
 * @param plyr_idx The casting player.
 * @param pwkind Power kind to be casted.
 * @param splevel Power overcharge level.
 * @param stl_x The target subtile, X coord.
 * @param stl_y The target subtile, Y coord.
 * @param allow_flags Additional castability flags, to loosen constaints in the spell config.
 * @return
 */
TbResult magic_use_available_power_on_subtile(PlayerNumber plyr_idx, PowerKind pwkind,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long allow_flags)
{
    TbResult ret;
    ret = Lb_OK;
    if (!is_power_available(plyr_idx, pwkind)) {
        // It shouldn't be possible to select unavailable spell
        WARNLOG("Player %d tried to cast %s which is unavailable",(int)plyr_idx,power_code_name(pwkind));
        ret = Lb_FAIL;
    }
    if (ret == Lb_OK)
    {
        ret = magic_use_power_on_subtile(plyr_idx, pwkind, splevel, stl_x, stl_y, allow_flags);
    }
    if (ret == Lb_FAIL) {
        // Make a rejection sound
        if (is_my_player_number(plyr_idx))
            play_non_3d_sample(119);
    }
    return ret;
}

TbResult magic_use_power_on_subtile(PlayerNumber plyr_idx, PowerKind pwkind,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long allow_flags)
{
    TbResult ret;
    ret = Lb_OK;
    TbBool cast_at_xy;
    cast_at_xy = can_cast_power_at_xy(plyr_idx, pwkind, stl_x, stl_y, allow_flags);
    // Fail if the function has failed
    if (!cast_at_xy) {
        WARNLOG("Player %d tried to cast %s on %s which can't be targeted",
            (int)plyr_idx,power_code_name(pwkind),"a subtile");
        ret = Lb_FAIL;
    }

    if (ret == Lb_OK)
    {
        if (splevel > MAGIC_OVERCHARGE_LEVELS) {
            WARNLOG("Overcharge level %d out of range, adjusting",(int)splevel);
            splevel = MAGIC_OVERCHARGE_LEVELS;
        }
    }
    if (ret == Lb_OK)
    {
        SYNCDBG(7,"Player %d is casting %s level %d",(int)plyr_idx,power_code_name(pwkind),(int)splevel);
        switch (pwkind)
        {
        case PwrK_MKDIGGER:
            ret = magic_use_power_imp(plyr_idx, stl_x, stl_y, allow_flags);
            break;
        case PwrK_SIGHT:
            ret = magic_use_power_sight(plyr_idx, stl_x, stl_y, splevel, allow_flags);
            break;
        case PwrK_CALL2ARMS:
            ret = magic_use_power_call_to_arms(plyr_idx, stl_x, stl_y, splevel, allow_flags);
            break;
        case PwrK_CAVEIN:
            ret = magic_use_power_cave_in(plyr_idx, stl_x, stl_y, splevel, allow_flags);
            break;
        case PwrK_LIGHTNING:
            ret = magic_use_power_lightning(plyr_idx, stl_x, stl_y, splevel, allow_flags);
            break;
        case PwrK_DESTRWALLS:
            ret = magic_use_power_destroy_walls(plyr_idx, stl_x, stl_y, splevel, allow_flags);
            break;
        case PwrK_TIMEBOMB:
            ret = magic_use_power_time_bomb(plyr_idx, stl_x, stl_y, splevel, allow_flags);
            break;
        default:
            ERRORLOG("Power not supported here: %d", (int)pwkind);
            ret = Lb_FAIL;
            break;
        }
    }
    return ret;
}

/**
 * Unified function for using powers which are castable without any particular target.
 *
 * @param plyr_idx The casting player.
 * @param spl_idx Power kind to be casted.
 * @param splevel Power overcharge level.
 * @return
 */
TbResult magic_use_available_power_on_level(PlayerNumber plyr_idx, PowerKind spl_idx,
    unsigned short splevel, unsigned long allow_flags)
{
    if (!is_power_available(plyr_idx, spl_idx)) {
        // It shouldn't be possible to select unavailable spell
        WARNLOG("Player %d tried to cast unavailable spell %d",(int)plyr_idx,(int)spl_idx);
        return Lb_FAIL;
    }
    return magic_use_power_on_level(plyr_idx, spl_idx, splevel, allow_flags);
}

TbResult magic_use_power_on_level(PlayerNumber plyr_idx, PowerKind spl_idx,
    unsigned short splevel, unsigned long allow_flags)
{
    if (splevel > MAGIC_OVERCHARGE_LEVELS) {
        splevel = MAGIC_OVERCHARGE_LEVELS;
    }
    switch (spl_idx)
    {
    case PwrK_OBEY:
        return magic_use_power_obey(plyr_idx, allow_flags);
    case PwrK_HOLDAUDNC:
        return magic_use_power_hold_audience(plyr_idx, allow_flags);
    case PwrK_ARMAGEDDON:
        return magic_use_power_armageddon(plyr_idx, allow_flags);
    default:
        ERRORLOG("Power not supported here: %d", (int)spl_idx);
        break;
    }
    return Lb_FAIL;
}

void directly_cast_spell_on_thing(PlayerNumber plyr_idx, PowerKind spl_idx, ThingIndex thing_idx, long splevel)
{
    struct Thing *thing;
    thing = thing_get(thing_idx);
    magic_use_available_power_on_thing(plyr_idx, spl_idx, splevel,
        thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, PwMod_Default);
}

int get_power_overcharge_level(struct PlayerInfo *player)
{
    int i;
    i = (player->cast_expand_level >> 2);
    if (i > SPELL_MAX_LEVEL)
        return SPELL_MAX_LEVEL;
    return i;
}

TbBool update_power_overcharge(struct PlayerInfo *player, int pwkind)
{
  struct Dungeon *dungeon;
  int i;
  if (pwkind >= POWER_TYPES_COUNT)
      return false;
  dungeon = get_dungeon(player->id_number);
  const struct MagicStats *pwrdynst;
  pwrdynst = get_power_dynamic_stats(pwkind);
  i = (player->cast_expand_level+1) >> 2;
  if (i > SPELL_MAX_LEVEL)
    i = SPELL_MAX_LEVEL;
  if (pwrdynst->cost[i] <= dungeon->total_money_owned)
  {
    // If we have more money, increase overcharge
    player->cast_expand_level++;
  } else
  {
    // If we don't have money, decrease the charge
    while (pwrdynst->cost[i] > dungeon->total_money_owned)
    {
      i--;
      if (i < 0) break;
    }
    if (i >= 0)
      player->cast_expand_level = (i << 2) + 1;
    else
      player->cast_expand_level = 0;
  }
  return (i < SPELL_MAX_LEVEL);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
