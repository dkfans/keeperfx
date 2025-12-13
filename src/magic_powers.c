/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file magic_powers.c
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
#include "pre_inc.h"
#include "magic_powers.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "bflib_sound.h"

#include "player_data.h"
#include "player_instances.h"
#include "config_players.h"
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
#include "gui_tooltips.h"
#include "room_jobs.h"
#include "map_blocks.h"
#include "map_columns.h"
#include "sounds.h"
#include "game_legacy.h"
#include "creature_instances.h"
#include "map_locations.h"
#include "lua_triggers.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const long power_sight_close_instance_time[] = {4, 4, 5, 5, 6, 6, 7, 7, 8};

unsigned char destroy_effect[][9] = {
    {88, 88, 88, 88, 79, 88, 88, 88, 88,},//power_level=0
    {88, 88, 88, 88, 32, 88, 88, 88, 88,},
    {88, 88, 88, 79, 32, 79, 88, 88, 88,},
    {88, 79, 88, 79, 32, 79, 88, 79, 88,},
    {88, 79, 88, 79, 32, 79, 88, 79, 88,},
    {88, 88, 88, 32, 32, 32, 88, 88, 88,},
    {88, 32, 88, 32, 32, 32, 88, 32, 88,},
    {79, 32, 79, 32, 32, 32, 79, 32, 79,},
    {32, 32, 32, 32, 32, 32, 32, 32, 32,},//power_level=8
};

/******************************************************************************/
static TbResult magic_use_power_hand         (PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_apply_spell  (PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_slap_thing   (PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_possess_thing(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_call_to_arms (PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_lightning    (PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_imp          (PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_sight        (PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_cave_in      (PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_destroy_walls(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_obey         (PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_hold_audience(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_armageddon   (PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);
static TbResult magic_use_power_tunneller    (PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);

typedef TbResult (*Magic_use_Func)(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags);

const Magic_use_Func magic_use_func_list[] = {
     NULL,
     &magic_use_power_hand,
     &magic_use_power_apply_spell,
     &magic_use_power_slap_thing,
     &magic_use_power_possess_thing,
     &magic_use_power_call_to_arms,
     &magic_use_power_lightning,
     &magic_use_power_imp,
     &magic_use_power_sight,
     &magic_use_power_cave_in,
     &magic_use_power_destroy_walls,
     &magic_use_power_obey,
     &magic_use_power_hold_audience,
     &magic_use_power_armageddon,
     &magic_use_power_tunneller,
};
/******************************************************************************/

/**
 * Returns if power can be casted or given thing and/or coordinates.
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
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player->work_state == PSt_FreeDestroyWalls)
    {
        struct SlabConfigStats *slabst = get_slab_stats(get_slabmap_for_subtile(stl_x, stl_y));
        return ( (slabst->category == SlbAtCtg_FortifiedWall) || (slabst->category == SlbAtCtg_FriableDirt) );
    }
    else if ( (player->work_state == PSt_FreeCastDisease) || (player->work_state == PSt_FreeTurnChicken) )
    {
        return (slab_is_wall(subtile_slab(stl_x), subtile_slab(stl_y)) == false);
    }
    if ((flags & CastChk_SkipAvailiabilty) == 0)
    {
        if (!is_power_available(plyr_idx, pwkind)) {
            return false;
        }
    }
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
            for (i = 0; i < game.conf.magic_conf.power_types_count; i++)
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
    if ((powerst->can_cast_flags & PwCast_NeedsDelay) != 0)
    {
        struct PlayerInfo* player;
        player = get_player(plyr_idx);
        if (game.play_gameturn <= player->power_of_cooldown_turn) {
            return false;
        }
    }
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
        if ((powerst->can_cast_flags & PwCast_OwnedObjects) != 0)
        {
            if (thing->owner == plyr_idx) {
                if (object_is_pickable_by_hand_to_hold(thing)) {
                    return true;
                }
            }
        }
        if ((powerst->can_cast_flags & PwCast_NeutrlObjects) != 0)
        {
            if (is_neutral_thing(thing)) {
                if (object_is_pickable_by_hand_to_hold(thing)) {
                    return true;
                }
            }
        }
        if ((powerst->can_cast_flags & PwCast_EnemyObjects) != 0)
        {
            if ((thing->owner != plyr_idx) && !is_neutral_thing(thing)) {
                if (object_is_pickable_by_hand_to_hold(thing)) {
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
                trapst = &game.conf.trapdoor_conf.trap_cfgstats[thing->model];
                if ((trapst->slappable > 0) && trap_is_active(thing)) {
                    return true;
                }
            }
        }
    }
    if (thing_is_creature(thing))
    {
        struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
        if (creature_is_leaving_and_cannot_be_stopped(thing))
        {
            return false;
        }
        if (creature_is_for_dungeon_diggers_list(thing))
        {
            if (powerst->can_cast_flags & PwCast_DiggersNot)
            {
                return false;
            }
        }
        else
        {
            if (powerst->can_cast_flags & PwCast_DiggersOnly)
            {
                return false;
            }
        }
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
            if (flag_is_set(cctrl->stateblock_flags, CCSpl_Teleport)) {
                SYNCDBG(8,"Player %d cannot cast %s on %s index %d while teleporting",(int)plyr_idx,power_code_name(pwkind),thing_model_name(thing),(int)thing->index);
                return false;
            }
            if (creature_under_spell_effect(thing, CSAfF_Timebomb)) {
                SYNCDBG(8,"Player %d cannot cast %s on %s index %d because TimeBomb blocks it",(int)plyr_idx,power_code_name(pwkind),thing_model_name(thing),(int)thing->index);
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
            if (!players_creatures_tolerate_each_other(plyr_idx, thing->owner))
            {
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
        if ((stl_y < 0) || (stl_y > game.map_subtiles_y)) {
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
            if (stl_x_beg > game.map_subtiles_x-1) {
                stl_x_beg = game.map_subtiles_x-1;
            }
            if (stl_x_end < 0) {
                stl_x_end = 0;
            } else
            if (stl_x_end > game.map_subtiles_x-1) {
                stl_x_end = game.map_subtiles_x-1;
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
      if ((stl_x < 0) || (stl_x > game.map_subtiles_x)) {
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
            if (stl_y_end > game.map_subtiles_y-1) {
                stl_y_end = game.map_subtiles_y-1;
            }
            if (stl_y_beg < 0) {
                stl_y_beg = 0;
            } else
            if (stl_y_beg > game.map_subtiles_y-1) {
                stl_y_beg = game.map_subtiles_y-1;
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
    struct Thing *thing = thing_get(dungeon->sight_casted_thing_idx);
    if (!thing_exists(thing)) {
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
    struct CreatureModelConfig *crconf;
    struct CreatureControl *cctrl;
    const struct PowerConfigStats *powerst;
    long i;
    crconf = creature_stats_get_from_thing(thing);
    cctrl = creature_control_get_from_thing(thing);
    anger_apply_anger_to_creature(thing, crconf->annoy_slapped, AngR_Other, 1);
    if (crconf->slaps_to_kill > 0)
    {
        HitPoints slap_damage = calculate_correct_creature_max_health(thing) / crconf->slaps_to_kill;
        apply_damage_to_thing_and_display_health(thing, slap_damage, player->id_number);
    }
    powerst = get_power_model_stats(PwrK_SLAP);
    i = cctrl->slap_turns;
    cctrl->slap_turns = powerst->duration;
    if (i == 0)
    {
        cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
    }
    if (thing->active_state != CrSt_CreatureSlapCowers)
    {
        clear_creature_instance(thing);
        if (thing->active_state != CrSt_CreatureCastingPreparation)
        {
            // If the creature was in CreatureCastingPreparation state,
            // its active and continue states have been assigned to those bkp states,
            // so we don't need to assign them again.
            cctrl->active_state_bkp = thing->active_state;
            cctrl->continue_state_bkp = thing->continue_state;
        }
        creature_mark_if_woken_up(thing);
        external_set_thing_state(thing, CrSt_CreatureSlapCowers);
    }
    cctrl->frozen_on_hit = 6; // Could be configurable.
    cctrl->cowers_from_slap_turns = 18; // Could be configurable.
    play_creature_sound(thing, CrSnd_Slap, 3, 0);
}

TbBool can_cast_power_at_xy(PlayerNumber plyr_idx, PowerKind pwkind, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long allow_flags)
{
    struct Map *mapblk;
    struct SlabMap *slb;
    unsigned long long can_cast;
    mapblk = get_map_block_at(stl_x, stl_y);
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    struct SlabConfigStats *slabst;
    slabst = get_slab_stats(slb);
    const struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(pwkind);
    if (power_model_stats_invalid(powerst))
        return false;
    can_cast = powerst->can_cast_flags | allow_flags;
    // Allow casting only on revealed tiles (unless the power overrides this)
    if ((can_cast & PwCast_Unrevealed) == 0)
    {
        if (!map_block_revealed(mapblk, plyr_idx))
        {
            // If it's not revealed, we may still accept revealing by SOE power
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
        if (game.play_gameturn <= player->power_of_cooldown_turn) {
            return false;
        }
    }
    PlayerNumber slb_owner;
    slb_owner = slabmap_owner(slb);
    TbBool subtile_is_liquid_or_path = ( (subtile_is_liquid(stl_x, stl_y)) || (subtile_is_unclaimed_path(stl_x, stl_y)) );
    if ( ((mapblk->flags & SlbAtFlg_Blocking) != 0) && (!subtile_is_liquid_or_path) )
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
            if (subtile_is_liquid(stl_x, stl_y))
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
            if (slabst->category == SlbAtCtg_Unclaimed) {
                return true;
            }
            if (subtile_is_liquid_or_path)
            {
                return true;
            }
        }
        if ((can_cast & PwCast_NeutrlGround) != 0)
        {
            if ((slabst->category != SlbAtCtg_Unclaimed) && (slb_owner == game.neutral_player_num)) {
                return true;
            }
        }
        if ((can_cast & PwCast_OwnedGround) != 0)
        {
            if ((slabst->category != SlbAtCtg_Unclaimed) && (slb_owner == plyr_idx)) {
                return true;
            }
        }
        if ((can_cast & PwCast_AlliedGround) != 0)
        {
            if ((slabst->category != SlbAtCtg_Unclaimed) && (slb_owner != plyr_idx) && players_are_mutual_allies(plyr_idx,slb_owner)) {
                return true;
            }
        }
        if ((can_cast & PwCast_EnemyGround) != 0)
        {
            if ((slabst->category != SlbAtCtg_Unclaimed) && players_are_enemies(plyr_idx,slb_owner)) {
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
 * @param power_level Keeper power overload level.
 * @param amount Amount used to scale the price; use 0 to get base price.
 */
GoldAmount compute_power_price_scaled_with_amount(PlayerNumber plyr_idx, PowerKind pwkind, KeepPwrLevel power_level, long amount)
{
    const struct PowerConfigStats *powerst = get_power_model_stats(pwkind);
    if (amount < 0)
        amount = 0;
    return powerst->cost[power_level] + (powerst->cost[0] * amount);
}

/**
 * Computes current price of given power for given player.
 * @param plyr_idx Casting player index.
 * @param pwkind Keeper power kind.
 * @param power_level Keeper power overload level.
 */
GoldAmount compute_power_price(PlayerNumber plyr_idx, PowerKind pwkind, KeepPwrLevel power_level)
{
    struct Dungeon *dungeon;
    const struct PowerConfigStats *powerst = get_power_model_stats(pwkind);
    long amount;
    long price;
    switch (powerst->cost_formula)
    {
    case Cost_Digger: // Special price algorithm for "create imp" power
        dungeon = get_players_num_dungeon(plyr_idx);
        // Increase price by amount of diggers, reduce by count of sacrificed diggers. Cheaper diggers may be a negative amount.
        if (get_players_special_digger_model(plyr_idx) == powerst->creature_model)
        {
            amount = (dungeon->num_active_diggers - dungeon->cheaper_diggers);
        }
        else
        {
            amount = dungeon->owned_creatures_of_model[powerst->creature_model];
        }
        price = compute_power_price_scaled_with_amount(plyr_idx, pwkind, power_level, amount);
        break;
    case Cost_Dwarf:
        dungeon = get_players_num_dungeon(plyr_idx);
        amount = (dungeon->owned_creatures_of_model[powerst->creature_model] / 7); //Dwarves come in pairs of 7
        price = compute_power_price_scaled_with_amount(plyr_idx, pwkind, power_level, amount);
        break;
    case Cost_Default:
    default:
        price = powerst->cost[power_level];
        break;
    }
    return price;
}

/**
 * Computes lowest possible price of given power for given player.
 * @param plyr_idx Casting player index.
 * @param pwkind Keeper power kind.
 * @param power_level Keeper power overload level.
 */
GoldAmount compute_lowest_power_price(PlayerNumber plyr_idx, PowerKind pwkind, KeepPwrLevel power_level)
{
    const struct PowerConfigStats *powerst;
    long price;
    switch (pwkind)
    {
    case PwrK_MKDIGGER: // Special price algorithm for "create imp" power
        // To get lowest
        price = compute_power_price_scaled_with_amount(plyr_idx, pwkind, power_level, 0);
        break;
    default:
        powerst = get_power_model_stats(pwkind);
        price = powerst->cost[power_level];
        break;
    }
    return price;
}
long find_spell_age_percentage(PlayerNumber plyr_idx, PowerKind pwkind)
{
    struct Dungeon *dungeon;
    const struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(pwkind);
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
            total = powerst->strength[dungeon->sight_casted_power_level] + 8;
        }
        break;
    case PwrK_CALL2ARMS:
        dungeon = get_players_num_dungeon(plyr_idx);
        if (dungeon->cta_start_turn != 0)
        {
            curr = game.play_gameturn - dungeon->cta_start_turn;
            total = powerst->duration;
        }
        break;
    default:
        break;
    }
    if (total > 0)
        return (curr << 8) / total;
    return -1;
}

TbBool pay_for_spell(PlayerNumber plyr_idx, PowerKind pwkind, KeepPwrLevel power_level)
{
    long price;
    if (pwkind >= game.conf.magic_conf.power_types_count)
        return false;
    if (power_level >= MAGIC_OVERCHARGE_LEVELS)
        power_level = MAGIC_OVERCHARGE_LEVELS;
    price = compute_power_price(plyr_idx, pwkind, power_level);
    // Try to take money
    if (take_money_from_dungeon(plyr_idx, price, 1) >= 0)
    {
        return true;
    }
    // If failed, say "you do not have enough gold"
    if (is_my_player_number(plyr_idx))
        output_message(SMsg_GoldNotEnough, 0);
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

static TbResult magic_use_power_armageddon(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
{
    SYNCDBG(6,"Starting");
    unsigned long your_time_gap;
    unsigned long enemy_time_gap;
    your_time_gap = game.conf.rules[plyr_idx].magic.armageddon_count_down + game.play_gameturn;
    enemy_time_gap = game.conf.rules[plyr_idx].magic.armageddon_count_down + game.play_gameturn;
    if (game.armageddon_cast_turn != 0) {
        return Lb_OK;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the power, fail
        if (!pay_for_spell(plyr_idx, power_kind, 0)) {
            if (is_my_player_number(plyr_idx))
                output_message(SMsg_GoldNotEnough, 0);
            return Lb_OK;
        }
    }
    game.armageddon_cast_turn = game.play_gameturn;
    game.armageddon_caster_idx = plyr_idx;
    struct Thing *heartng;
    heartng = get_player_soul_container(plyr_idx);
    game.armageddon_mappos.x.val = heartng->mappos.x.val;
    game.armageddon_mappos.y.val = heartng->mappos.y.val;
    game.armageddon_mappos.z.val = heartng->mappos.z.val;

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
        if (is_neutral_thing(thing) && !game.conf.rules[plyr_idx].magic.armageddon_teleport_neutrals) // Creatures unaffected by Armageddon.
        {
            cctrl->armageddon_teleport_turn = 0;
        }
        else if (creature_under_spell_effect(thing, CSAfF_Chicken)) // Creatures killed by Armageddon.
        {
            kill_creature(thing, heartng, plyr_idx, CrDed_DiedInBattle);
        }
        else // Creatures teleported by Armageddon.
        {
            cctrl->armageddon_teleport_turn = your_time_gap;
            if (thing->owner == plyr_idx) {
                your_time_gap += game.conf.rules[plyr_idx].magic.armageddon_teleport_your_time_gap;
            } else {
                enemy_time_gap += game.conf.rules[plyr_idx].magic.armageddon_teleport_enemy_time_gap;
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
    game.armageddon_over_turn = game.conf.rules[plyr_idx].magic.armageddon_duration + enemy_time_gap;
    if (plyr_idx == my_player_number)
    {
        struct PowerConfigStats *powerst = get_power_model_stats(power_kind);
        play_non_3d_sample(powerst->select_sound_idx);
    }
    return Lb_SUCCESS;
}

/**
 * Starts and stops the use of Must obey.
 * What differs this power from others is that it is a toggle - pressing once
 * starts the power, and second press disables it.
 * The power is paid for somewhere else - it takes money every few turns when active.
 * @param plyr_idx
 * @param mod_flags
 * @return
 */
static TbResult magic_use_power_obey(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    // Toggle the power
    if (dungeon->must_obey_turn != 0) {
        dungeon->must_obey_turn = 0;
    } else {
        dungeon->must_obey_turn = game.play_gameturn;
        if (plyr_idx == my_player_number)
        {
            struct PowerConfigStats *powerst = get_power_model_stats(PwrK_OBEY);
            play_non_3d_sample(powerst->select_sound_idx);
        }
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
    KeepPwrLevel power_level;
    long cit;
    long i;
    long imax;
    long k;
    long n;
    dungeon = get_players_num_dungeon(plyr_idx);
    const struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(PwrK_SIGHT);
    power_level = dungeon->sight_casted_power_level;
    if (power_level > POWER_MAX_LEVEL)
        power_level = POWER_MAX_LEVEL;
    i = game.play_gameturn - dungeon->sight_casted_gameturn;
    imax = abs(powerst->strength[power_level]/4) >> 2;
    if (i > imax)
        i = imax;
    if (i < 0)
        i = 0;
    n = game.play_gameturn - powerst->strength[power_level];
    cit = power_sight_close_instance_time[power_level];
    k = imax / cit;
    if (k < 1) k = 1;
    dungeon->sight_casted_gameturn = n + i/k - cit;
}

static TbResult magic_use_power_hold_audience(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
{
    SYNCDBG(8,"Starting");
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon->hold_audience_cast_turn != 0) {
        return Lb_OK;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the power, fail
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
        if (!thing_is_picked_up(thing) && !creature_is_kept_in_custody(thing) && !creature_is_being_unconscious(thing) && !creature_is_leaving_and_cannot_be_stopped(thing))
        {
            create_effect(&thing->mappos, imp_spangle_effects[get_player_color_idx(thing->owner)], thing->owner);
            const struct Coord3d *pos;
            pos = dungeon_get_essential_pos(thing->owner);
            move_thing_in_map(thing, pos);
            reset_interpolation_of_thing(thing);
            initialise_thing_state(thing, CrSt_CreatureInHoldAudience);
            cctrl->turns_at_job = -1;

            struct Thing* famlrtng; //familiars are not in the dungeon creature list
            for (short j = 0; j < FAMILIAR_MAX; j++)
            {
                if (cctrl->familiar_idx[j])
                {
                    famlrtng = thing_get(cctrl->familiar_idx[j]);
                    teleport_familiar_to_summoner(famlrtng, thing);
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
    if (plyr_idx == my_player_number)
    {
        struct PowerConfigStats *powerst = get_power_model_stats(PwrK_HOLDAUDNC);
        play_non_3d_sample(powerst->select_sound_idx);
    }
    SYNCDBG(19,"Finished");
    return Lb_SUCCESS;
}

static TbResult magic_use_power_hand(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
{
    if (power_hand_is_full(get_player(plyr_idx)))
        return Lb_FAIL;
    else
    if (place_thing_in_power_hand(thing, plyr_idx))
        return Lb_SUCCESS;
    else
        return Lb_FAIL;
}

static TbResult magic_use_power_destroy_walls(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
{
    // If we can't afford the power, fail
    SYNCDBG(16,"Starting");
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the power, fail
        if (!pay_for_spell(plyr_idx, power_kind, power_level)) {
            return Lb_FAIL;
        }
    }
    MapSlabCoord slb_x_start = subtile_slab(stl_x) - 1;
    MapSlabCoord slb_y_start = subtile_slab(stl_y) - 1;
    MapSlabCoord slb_x_end = slb_x_start + 3;
    MapSlabCoord slb_y_end = slb_y_start + 3;
    int i = 0;
    TbBool is_revealed = subtile_revealed(stl_x, stl_y, plyr_idx);
    for (MapSlabCoord slb_y=slb_y_start; slb_y < slb_y_end ; slb_y++)
    {
        for (MapSlabCoord slb_x=slb_x_start; slb_x < slb_x_end ; slb_x++,i++)
        {
            struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
            if (slabmap_block_invalid(slb))
                continue;
            struct Map *mapblk = get_map_block_at(slab_subtile_center(slb_x),slab_subtile_center(slb_y));
            if (!(mapblk->flags & SlbAtFlg_Blocking) || (mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom|SlbAtFlg_Valuable)) || (slb->kind == SlbT_ROCK) )
              continue;
            unsigned char destreff = destroy_effect[power_level][i];
            if (destreff == 79)
            {
                struct SlabConfigStats *slabst = get_slab_stats(slb);
                if (slabst->category == SlbAtCtg_FortifiedWall)
                {
                    place_slab_type_on_map(SlbT_EARTH, slab_subtile_center(slb_x),slab_subtile_center(slb_y), game.neutral_player_num, 0);
                    create_dirt_rubble_for_dug_slab(slb_x, slb_y);
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
        struct PowerConfigStats *powerst = get_power_model_stats(power_kind);
        play_non_3d_sample(powerst->select_sound_idx);
    }
    return Lb_SUCCESS;
}

static TbResult magic_use_power_imp(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
{
    struct Thing *heartng;
    struct Coord3d pos;
    struct Dungeon *dungeon;
    struct CreatureControl* cctrl;
    struct PowerConfigStats *powerst = get_power_model_stats(power_kind);
    if (!i_can_allocate_free_control_structure()
     || !i_can_allocate_free_thing_structure(TCls_Creature)) {
        return Lb_FAIL;
    }
    if (!creature_count_below_map_limit(0))
    {
        SYNCLOG("Player %d attempts to create creature %s at map creature limit", plyr_idx, creature_code_name(powerst->creature_model));
        return Lb_FAIL;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the power, fail
        if (!pay_for_spell(plyr_idx, power_kind, power_level)) {
            return Lb_FAIL;
        }
    }
    heartng = get_player_soul_container(plyr_idx);
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = get_floor_height_at(&pos) + (heartng->clipbox_size_z >> 1);
    thing = create_creature(&pos, powerst->creature_model, plyr_idx);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("There was place to create new creature, but creation failed");
        return Lb_OK;
    }
    // Temporary unit
    if (powerst->duration > 0)
    {
        cctrl = creature_control_get_from_thing(thing);
        dungeon = get_dungeon(thing->owner);
        add_creature_to_summon_list(dungeon, thing->index);
        //Just set it to self-summoned
        cctrl->summoner_idx = thing->index;
        cctrl->summon_spl_idx = 0;
        remove_first_creature(thing); //temporary units are not real creatures
        cctrl->unsummon_turn = game.play_gameturn + powerst->duration;
        set_flag(cctrl->creature_state_flags, TF2_SummonedCreature);
    }
    if (powerst->strength[power_level] != 0)
    {
        creature_change_multiple_levels(thing, powerst->strength[power_level]);
    }
    thing->veloc_push_add.x.val += THING_RANDOM(thing, 161) - 80;
    thing->veloc_push_add.y.val += THING_RANDOM(thing, 161) - 80;
    thing->veloc_push_add.z.val += 160;
    thing->state_flags |= TF1_PushAdd;
    thing->move_angle_xy = ANGLE_NORTH;
    initialise_thing_state(thing, CrSt_ImpBirth);

    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    play_creature_sound(thing, CrSnd_Happy, 2, 0);
    return Lb_SUCCESS;
}

static TbResult magic_use_power_tunneller(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
{
    struct Coord3d pos;
    struct PowerConfigStats *powerst = get_power_model_stats(power_kind);
    struct Dungeon* dungeon;
    if (!i_can_allocate_free_control_structure()
     || !i_can_allocate_free_thing_structure(TCls_Creature)) {
        return Lb_FAIL;
    }
    if (!creature_count_below_map_limit(0))
    {
        SYNCLOG("Player %d attempts to create creature %s at map creature limit", plyr_idx, creature_code_name(powerst->creature_model));
        return Lb_FAIL;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the power, fail
        if (!pay_for_spell(plyr_idx, power_kind, power_level)) {
            return Lb_FAIL;
        }
    }
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = get_ceiling_height(&pos);
    thing = create_creature(&pos, powerst->creature_model, plyr_idx);
    create_effect(&thing->mappos, TngEff_CeilingBreach, thing->owner); //do breach before fail, for better player feedback
    if (thing_is_invalid(thing))
    {
        ERRORLOG("There was place to create new creature, but creation failed");
        return Lb_OK;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (powerst->duration > 0)
    {
        dungeon = get_dungeon(thing->owner);
        add_creature_to_summon_list(dungeon, thing->index);
        //Just set it to self-summoned
        cctrl->summoner_idx = thing->index;
        cctrl->summon_spl_idx = 0;
        remove_first_creature(thing); //temporary units are not real creatures
        cctrl->unsummon_turn = game.play_gameturn + powerst->duration;
        set_flag(cctrl->creature_state_flags, TF2_SummonedCreature);
    }
    initialise_thing_state(thing, CrSt_CreatureHeroEntering);
    thing->rendering_flags |= TRF_Invisible;
    cctrl->countdown = 16;
    if (powerst->strength[power_level] != 0)
    {
        creature_change_multiple_levels(thing, powerst->strength[power_level]);
    }

    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    play_creature_sound(thing, CrSnd_Happy, 2, 0);
    return Lb_SUCCESS;
}

static TbResult magic_use_power_apply_spell(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
{
    struct PowerConfigStats *powerst = get_power_model_stats(power_kind);
    struct SpellConfig *spconf = get_spell_config(powerst->spell_idx);
    // If this power is already casted at that creature, do nothing.
    if (creature_under_spell_effect(thing, spconf->spell_flags))
    {
        return Lb_OK;
    }
    // If the creature is at full health and 'CSAfF_Heal' is the only flag in spell_idx, do nothing.
    if ((get_creature_health_permil(thing) >= 1000) && (spconf->spell_flags == CSAfF_Heal))
    {
        SYNCDBG(7, "Can't heal with %s on creature %s index %d is full health.", power_code_name(power_kind), thing_model_name(thing), (int)thing->index);
        return Lb_OK;
    }
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the power, fail.
        if (!pay_for_spell(plyr_idx, power_kind, power_level))
        {
            return Lb_FAIL;
        }
    }
    // Check if the creature kind isn't affected by that power.
    if (creature_is_immune_to_spell_effect(thing, spconf->spell_flags))
    {
        // Refusal sound.
        thing_play_sample(thing, 58, 20, 0, 3, 0, 2, 128);
        return Lb_SUCCESS;
    }
    // Create an effect originating from the ceiling.
    if (powerst->effect_id != 0)
    {
        struct Coord3d effpos = thing->mappos;
        effpos.z.val = get_ceiling_height_above_thing_at(thing, &thing->mappos);
        create_used_effect_or_element(&effpos, powerst->effect_id, thing->owner, 0);
    }
    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
    apply_spell_effect_to_thing(thing, powerst->spell_idx, power_level, plyr_idx);
    // Special cases.
    if (flag_is_set(spconf->spell_flags, CSAfF_Disease))
    { // Set disease_caster_plyridx if spell_idx has 'CSAfF_Disease'.
        struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
        cctrl->disease_caster_plyridx = plyr_idx;
    }
    if (flag_is_set(spconf->spell_flags, CSAfF_Timebomb))
    { // Only initialise state if spell_idx has 'CSAfF_Timebomb'.
        initialise_thing_state(thing, CrSt_Timebomb);
    }
    return Lb_SUCCESS;
}

static TbResult magic_use_power_lightning(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    const struct PowerConfigStats *powerst;
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
    pos.z.val = get_floor_height_at(&pos);
    // make sure the power level is correct
    if (power_level >= MAGIC_OVERCHARGE_LEVELS)
        power_level = MAGIC_OVERCHARGE_LEVELS-1;
    if ((mod_flags & PwMod_CastForFree) == 0)
    {
        // If we can't afford the power, fail
        if (!pay_for_spell(plyr_idx, power_kind, power_level)) {
            return Lb_FAIL;
        }
    }
    // And cast it
    shtng = create_shot(&pos, ShM_GodLightning, plyr_idx);
    if (!thing_is_invalid(shtng))
    {
        shtng->mappos.z.val = get_thing_height_at(shtng, &shtng->mappos) + COORD_PER_STL/2;
        shtng->shot.hit_type = THit_CrtrsOnly;
        shtng->shot.shot_level = power_level;
    }
    powerst = get_power_model_stats(power_kind);
    shotst = get_shot_model_stats(ShM_GodLightning);
    dungeon->camera_deviate_jump = 256;
    i = powerst->strength[power_level];
    max_damage = i * shotst->damage;
    range = (i << 8) / 2;
    if (power_sight_explored(stl_x, stl_y, plyr_idx))
        max_damage /= 4;
    struct Coord3d objpos;
    // Compensate for effect element position offset
    objpos.x.val = pos.x.val + 128;
    objpos.y.val = pos.y.val + 128;
    objpos.z.val = get_floor_height_at(&pos);
    obtng = create_object(&objpos, ObjMdl_PowerLightning, plyr_idx, -1);
    if (!thing_is_invalid(obtng))
    {
        obtng->lightning.power_level = power_level;
        obtng->rendering_flags |= TRF_Invisible;
    }
    i = electricity_affecting_area(&pos, plyr_idx, range, max_damage);
    SYNCDBG(9,"Affected %ld targets within range %ld, damage %ld",i,range,max_damage);
    if (!thing_is_invalid(shtng))
    {
        efftng = create_effect(&shtng->mappos, TngEff_Dummy, shtng->owner);
        if (!thing_is_invalid(efftng))
        {
            thing_play_sample(efftng, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        }
    }
    return Lb_SUCCESS;
}

static TbResult magic_use_power_sight(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
{
    const struct PowerConfigStats *powerst;
    struct Dungeon *dungeon;
    struct Coord3d pos;
    long cit;
    long cdt;
    long cgt;
    long cdlimit;
    long i;
    dungeon = get_dungeon(plyr_idx);
    powerst = get_power_model_stats(PwrK_SIGHT);
    if (player_uses_power_sight(plyr_idx))
    {
        cdt = game.play_gameturn - dungeon->sight_casted_gameturn;
        cdlimit = powerst->strength[dungeon->sight_casted_power_level] >> 4;
        if (cdt < 0) {
            cdt = 0;
        } else
        if (cdt > cdlimit) {
            cdt = cdlimit;
        }
        cit = power_sight_close_instance_time[dungeon->sight_casted_power_level];
        cgt = game.play_gameturn - powerst->strength[dungeon->sight_casted_power_level];
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
        // If we can't afford the power, fail
        if (!pay_for_spell(plyr_idx, PwrK_SIGHT, power_level)) {
            if (is_my_player_number(plyr_idx))
                output_message(SMsg_GoldNotEnough, 0);
            return Lb_FAIL;
        }
    }
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = subtile_coord_center(5);
    thing = create_object(&pos, ObjMdl_PowerSight, plyr_idx, -1);
    if (!thing_is_invalid(thing))
    {
        dungeon->sight_casted_gameturn = game.play_gameturn;
        thing->health = 2;
        dungeon->sight_casted_power_level = power_level;
        dungeon->sight_casted_thing_idx = thing->index;
        memset(dungeon->soe_explored_flags, 0, sizeof(dungeon->soe_explored_flags));
        thing->rendering_flags |= TRF_Invisible;
        thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, -1, 3, 0, 3, FULL_LOUDNESS);
    }
    return Lb_SUCCESS;
}

static TbResult magic_use_power_cave_in(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
{
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slb_y = subtile_slab(stl_y);
    slb_x = subtile_slab(stl_x);
    struct Map *mapblk;
    mapblk = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    long i;
    unsigned long k;
    k = 0;
    i = get_mapwho_thing_index(mapblk);
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
                // If we can't afford the power, fail
                if (!pay_for_spell(plyr_idx, power_kind, power_level)) {
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
        thing = create_thing(&pos, TCls_CaveIn, power_level, plyr_idx, -1);
        struct PowerConfigStats *powerst;
        powerst = get_power_model_stats(power_kind);
        thing_play_sample(thing, powerst->select_sound_idx, 25, 0, 3, 0, 2, FULL_LOUDNESS);
    }
    return Lb_SUCCESS;
}

/**
 * Changes creature state and marks it as being affected by CTA power.
 *
 * @param cta_pos Position where the CTA power is casted.
 * @param creatng The target creature thing.
 * @return
 */
TbBool update_creature_influenced_by_call_to_arms_at_pos(struct Thing *creatng, const struct Coord3d *cta_pos)
{
    struct CreatureControl *cctrl = creature_control_get_from_thing(creatng);
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
    cctrl->called_to_arms = true;
    if (flag_is_set(cctrl->creature_control_flags, CCFlg_NoCompControl))
    {
        WARNLOG("The %s index %d is called to arms with no comp control, fixing", thing_model_name(creatng), (int)creatng->index);
        clear_flag(cctrl->creature_control_flags, CCFlg_NoCompControl);
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
                struct CreatureStateConfig *stati;
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
 * @param power_level Power overcharge level.
 * @return
 * @see magic_use_available_power_on_thing()
 * @see magic_use_available_power_on_subtile()
 */
static TbResult magic_use_power_call_to_arms(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
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
    objtng = thing_get(player->cta_flag_idx);
    if ((dungeon->cta_start_turn == 0) || !thing_is_object(objtng))
    {
          objtng = create_object(&pos, ObjMdl_CTAEnsign, plyr_idx, -1);
          if (thing_is_invalid(objtng)) {
              ERRORLOG("Cannot create call to arms");
              return 0;
          }
          dungeon->cta_start_turn = game.play_gameturn;
          dungeon->cta_power_level = power_level;
          dungeon->cta_stl_x = stl_x;
          dungeon->cta_stl_y = stl_y;
          if (flag_is_set(mod_flags, PwMod_CastForFree))
          {
              dungeon->cta_free = 1;
          }
          player->cta_flag_idx = objtng->index;
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

static TbResult magic_use_power_slap_thing(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
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
    player->influenced_thing_idx = thing->index;
    player->influenced_thing_creation = thing->creation_turn;
    set_player_instance(player, PI_Whip, 0);
    dungeon->lvstats.num_slaps++;
    return Lb_SUCCESS;
}

static TbResult magic_use_power_possess_thing(PowerKind power_kind, PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, KeepPwrLevel power_level, unsigned long mod_flags)
{
    struct PlayerInfo *player;
    if (!thing_exists(thing)) {
        return Lb_FAIL;
    }
    player = get_player(plyr_idx);
    player->influenced_thing_idx = thing->index;
    player->influenced_thing_creation = thing->creation_turn;
    player->first_person_dig_claim_mode = false;
    player->teleport_destination = 19; // reset to default behaviour
    player->battleid = 1;
    // Note that setting Direct Control player instance requires player->influenced_thing_idx to be set correctly
    set_player_instance(player, PI_DirctCtrl, 0);
    if (is_my_player(player)) {
        clear_flag(tool_tip_box.flags, TTip_Visible);
    }
    return Lb_SUCCESS;
}

static void magic_power_hold_audience_update(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    SYNCDBG(8,"Starting");
    if ( game.play_gameturn - dungeon->hold_audience_cast_turn <= game.conf.rules[plyr_idx].magic.hold_audience_time) {
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
    struct CreatureStateConfig *stati;
    stati = get_thing_state_info_num(nstat);
    if (!creature_affected_by_call_to_arms(creatng) || stati->react_to_cta)
    {
        if (stati->react_to_cta
          && (creature_affected_by_call_to_arms(creatng) || get_chessboard_distance(&creatng->mappos, cta_pos) < range))
        {
            if (update_creature_influenced_by_call_to_arms_at_pos(creatng, cta_pos))
            {
                creature_mark_if_woken_up(creatng);
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
            !creature_is_being_unconscious(thing) && !creature_is_dying(thing) && !creature_is_leaving_and_cannot_be_stopped(thing))
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
    struct Dungeon *dungeon = get_players_num_dungeon(plyr_idx);
    long duration = game.play_gameturn - dungeon->cta_start_turn;
    const struct PowerConfigStats *powerst = get_power_model_stats(PwrK_CALL2ARMS);
    struct SlabMap *slb = get_slabmap_for_subtile(dungeon->cta_stl_x, dungeon->cta_stl_y);
    TbBool free = ((slabmap_owner(slb) == plyr_idx) || dungeon->cta_free);
    if (!free)
    {
        if ((game.conf.rules[plyr_idx].game.allies_share_cta) && (players_are_mutual_allies(plyr_idx, slabmap_owner(slb))))
        {
            free = true;
        }
    }
    if (((powerst->duration < 1) || ((duration % powerst->duration) == 0)) && !free)
    {
        if (!pay_for_spell(plyr_idx, PwrK_CALL2ARMS, dungeon->cta_power_level))
        {
            turn_off_power_call_to_arms(plyr_idx);
            return;
        }
    }
    if ((duration % 16) == 0)
    {
        long range = subtile_coord(powerst->strength[dungeon->cta_power_level],0);
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
    const struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(PwrK_OBEY);
    if ((delta % powerst->duration) == 0)
    {
        if (!pay_for_spell(plyr_idx, PwrK_OBEY, 0)) {
            magic_use_power_obey(PwrK_OBEY,plyr_idx,INVALID_THING,0,0,0, PwMod_Default);
        }
    }
}

void process_dungeon_power_magic(void)
{
    SYNCDBG(8,"Starting");
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
                if (game.play_gameturn > game.armageddon_over_turn)
                {
                  game.armageddon_cast_turn = 0;
                  game.armageddon_over_turn = 0;
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
 * @param power_level Power overcharge level.
 * @param thing The target thing.
 * @param stl_x The casting subtile, X coord.
 * @param stl_y The casting subtile, Y coord.
 */
TbResult magic_use_available_power_on_thing(PlayerNumber plyr_idx, PowerKind pwkind,
    KeepPwrLevel power_level, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing, unsigned long mod_flags)
{
    TbResult ret;
    if (!is_power_available(plyr_idx, pwkind)) {
        // It shouldn't be possible to select unavailable power
        WARNLOG("Player %d tried to cast %s which is unavailable",(int)plyr_idx,power_code_name(pwkind));
        ret = Lb_FAIL;
    }
    else
    {
        ret = magic_use_power_on_thing(plyr_idx, pwkind, power_level, stl_x, stl_y, thing, mod_flags);
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


TbResult magic_use_power_direct(PlayerNumber plyr_idx, PowerKind pwkind,
    KeepPwrLevel power_level, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing, unsigned long allow_flags)
{
    lua_on_power_cast(plyr_idx, pwkind, power_level, stl_x, stl_y, thing);

    const struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
    if(powerst->magic_use_func_idx > 0 && magic_use_func_list[powerst->magic_use_func_idx] != NULL)
    {
        return magic_use_func_list[powerst->magic_use_func_idx](pwkind, plyr_idx, thing, stl_x, stl_y, power_level, allow_flags);
    }
    else if (powerst->magic_use_func_idx < 0)
    {
        return luafunc_magic_use_power(powerst->magic_use_func_idx, plyr_idx, pwkind, power_level, stl_x, stl_y, thing, allow_flags);
    }
    else
    {
        WARNLOG("Player %d tried to cast %s which has no valid function",(int)plyr_idx,power_code_name(pwkind));
        return Lb_FAIL;
    }
}

/**
 * Unified function for using powers which are castable on things. Without checks for availiability
 *
 * @param plyr_idx The casting player.
 * @param spl_idx Power kind to be casted.
 * @param power_level Power overcharge level.
 * @param thing The target thing.
 * @param stl_x The casting subtile, X coord.
 * @param stl_y The casting subtile, Y coord.
 */
TbResult magic_use_power_on_thing(PlayerNumber plyr_idx, PowerKind pwkind,
    KeepPwrLevel power_level, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing, unsigned long mod_flags)
{
    const struct PowerConfigStats* powerst = get_power_model_stats(pwkind);

    TbResult ret;
    ret = Lb_OK;
    if (!thing_exists(thing)) {
        WARNLOG("Player %d tried to cast %s on non-existing thing",(int)plyr_idx,power_code_name(pwkind));
        ret = Lb_FAIL;
    }
    if (ret == Lb_OK)
    {// Zero coords mean we should take real ones from the thing. But even if they're not zero, we might want to fix them sometimes
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
        if (power_level > MAGIC_OVERCHARGE_LEVELS) {
            WARNLOG("Overcharge level %d out of range, adjusting",(int)power_level);
            power_level = MAGIC_OVERCHARGE_LEVELS;
        }
    }
    if (ret == Lb_OK)
    {
        ret = magic_use_power_direct(plyr_idx, pwkind, power_level, stl_x, stl_y, thing, mod_flags);
    }
    if (ret == Lb_SUCCESS)
    {
        get_player(plyr_idx)->power_of_cooldown_turn = game.play_gameturn + powerst->cast_cooldown;
    }
    return ret;
}

/**
 * Unified function for using powers which are castable on map subtile.
 *
 * @param plyr_idx The casting player.
 * @param pwkind Power kind to be casted.
 * @param power_level Power overcharge level.
 * @param stl_x The target subtile, X coord.
 * @param stl_y The target subtile, Y coord.
 * @param allow_flags Additional castability flags, to loosen constaints in the power config.
 * @return
 */
TbResult magic_use_available_power_on_subtile(PlayerNumber plyr_idx, PowerKind pwkind,
    KeepPwrLevel power_level, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long allow_flags, unsigned long mod_flags)
{
    TbResult ret;
    ret = Lb_OK;
    if (!is_power_available(plyr_idx, pwkind)) {
        // It shouldn't be possible to select unavailable power
        WARNLOG("Player %d tried to cast %s which is unavailable",(int)plyr_idx,power_code_name(pwkind));
        ret = Lb_FAIL;
    }
    if (ret == Lb_OK)
    {
        ret = magic_use_power_on_subtile(plyr_idx, pwkind, power_level, stl_x, stl_y, allow_flags, mod_flags);
    }
    if (ret == Lb_FAIL) {
        // Make a rejection sound
        if (is_my_player_number(plyr_idx))
            play_non_3d_sample(119);
    }
    return ret;
}


TbResult magic_use_power_on_subtile(PlayerNumber plyr_idx, PowerKind pwkind,
    KeepPwrLevel power_level, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned long allow_flags,unsigned long mod_flags)
{
    TbResult ret;
    ret = Lb_OK;
    TbBool cast_at_xy;
    cast_at_xy = can_cast_power_at_xy(plyr_idx, pwkind, stl_x, stl_y, allow_flags);
    // Fail if the function has failed
    if (!cast_at_xy) {
        SYNCDBG(7,"Player %d tried to cast %s on %s which can't be targeted now",
            (int)plyr_idx,power_code_name(pwkind),"a subtile");
        ret = Lb_FAIL;
    }

    if (ret == Lb_OK)
    {
        if (power_level > MAGIC_OVERCHARGE_LEVELS) {
            WARNLOG("Overcharge level %d out of range, adjusting",(int)power_level);
            power_level = MAGIC_OVERCHARGE_LEVELS;
        }
    }
    if (ret == Lb_OK)
    {
        ret = magic_use_power_direct(plyr_idx,pwkind,power_level,stl_x, stl_y,INVALID_THING, mod_flags);
    }

    if (ret == Lb_SUCCESS)
    {
        const struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
        get_player(plyr_idx)->power_of_cooldown_turn = game.play_gameturn + powerst->cast_cooldown;
    }
    return ret;
}

/**
 * Unified function for using powers which are castable without any particular target.
 *
 * @param plyr_idx The casting player.
 * @param spl_idx Power kind to be casted.
 * @param power_level Power overcharge level.
 * @return
 */
TbResult magic_use_available_power_on_level(PlayerNumber plyr_idx, PowerKind spl_idx,
    KeepPwrLevel power_level, unsigned long mod_flags)
{
    if (!is_power_available(plyr_idx, spl_idx)) {
        // It shouldn't be possible to select unavailable power
        WARNLOG("Player %d tried to cast unavailable power %d",(int)plyr_idx,(int)spl_idx);
        return Lb_FAIL;
    }
    return magic_use_power_on_level(plyr_idx, spl_idx, power_level, mod_flags);
}

TbResult magic_use_power_on_level(PlayerNumber plyr_idx, PowerKind pwkind,
    KeepPwrLevel power_level, unsigned long mod_flags)
{
    if (power_level > MAGIC_OVERCHARGE_LEVELS) {
        power_level = MAGIC_OVERCHARGE_LEVELS;
    }
    return magic_use_power_direct(plyr_idx,pwkind,power_level,0,0,INVALID_THING,mod_flags);
}

void directly_cast_spell_on_thing(PlayerNumber plyr_idx, PowerKind pwkind, ThingIndex thing_idx, KeepPwrLevel power_level)
{
    struct Thing *thing;
    thing = thing_get(thing_idx);
    magic_use_available_power_on_thing(plyr_idx, pwkind, power_level,
        thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing, PwMod_Default);
}

/**
 * Casts keeper power on a specific creature, or position of the creature depending on the power.
 * @param thing The creature to target.
 * @param pwkind The ID of the Keeper Power.
 * @param power_level The overcharge level of the keeperpower. Is ignored when not applicable.
 * @param caster The player number of the player who is made to cast the power.
 * @param is_free If gold is used when casting the power. It will fail to cast if it is not free and money is not available.
 * @return TbResult whether the power was successfully cast
 */
TbResult script_use_power_on_creature(struct Thing* thing, short pwkind, KeepPwrLevel power_level, PlayerNumber caster, TbBool is_free)
{
    if (thing_is_in_power_hand_list(thing, thing->owner))
    {
        char block = pwkind == PwrK_SLAP;
        block |= pwkind == PwrK_CALL2ARMS;
        block |= pwkind == PwrK_CAVEIN;
        block |= pwkind == PwrK_LIGHTNING;
        block |= pwkind == PwrK_MKDIGGER;
        block |= pwkind == PwrK_SIGHT;
        if (block)
        {
            SYNCDBG(5, "Found creature to use power on but it is being held.");
            return Lb_FAIL;
        }
    }

    MapSubtlCoord stl_x = thing->mappos.x.stl.num;
    MapSubtlCoord stl_y = thing->mappos.y.stl.num;
    unsigned long mod_flags = is_free ? PwMod_CastForFree : 0;

    return magic_use_power_direct(caster,pwkind,power_level,stl_x,stl_y,thing,mod_flags);
}

int get_power_overcharge_level(struct PlayerInfo *player)
{
    int i;
    i = (player->cast_expand_level >> 2);
    if (i > POWER_MAX_LEVEL)
        return POWER_MAX_LEVEL;
    return i;
}

/**
 * @return Is it necessary to continue trying the operation of increasing spell level
 */
TbBool update_power_overcharge(struct PlayerInfo *player, int pwkind)
{
  struct Dungeon *dungeon;
  int i;
  if (pwkind >= game.conf.magic_conf.power_types_count)
      return false;
  dungeon = get_dungeon(player->id_number);
  const struct PowerConfigStats *powerst;
  powerst = get_power_model_stats(pwkind);
  i = (player->cast_expand_level+1) >> 2;
  if (i > POWER_MAX_LEVEL)
    i = POWER_MAX_LEVEL;
  if (powerst->cost[i] <= dungeon->total_money_owned)
  {
    // If we have more money, increase overcharge
    if (((player->cast_expand_level+1) >> 2) <= POWER_MAX_LEVEL)
    {
      player->cast_expand_level++;
    }
    if (((player->cast_expand_level+1) >> 2) <= POWER_MAX_LEVEL)
    {
      return true;
    }
  } else
  {
    // If we don't have money, decrease the charge
    while (powerst->cost[i] > dungeon->total_money_owned)
    {
      i--;
      if (i < 0) break;
    }
    if (i >= 0)
      player->cast_expand_level = (i << 2) + 1;
    else
      player->cast_expand_level = 0;
  }
  return false;
}

/**
 * Casts power at a location set by subtiles.
 * @param plyr_idx caster player.
 * @param stl_x subtile's x position.
 * @param stl_y subtile's y position
 * @param fml_bytes encoded bytes: f=cast for free flag,m=power kind,l=power level.
 * @return TbResult whether the power was successfully cast
 */
TbResult script_use_power_at_pos(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long fml_bytes)
{
    char is_free = (fml_bytes >> 16) != 0;
    PowerKind powerKind = (fml_bytes >> 8) & 255;
    KeepPwrLevel power_level = fml_bytes & 255;

    unsigned long allow_flags = PwCast_AllGround | PwCast_Unrevealed;
    unsigned long mod_flags = 0;
    if (is_free)
        set_flag(mod_flags,PwMod_CastForFree);

    return magic_use_power_on_subtile(plyr_idx, powerKind, power_level, stl_x, stl_y, allow_flags, mod_flags);
}

/**
 * Casts power at a location set by action point/hero gate.
 * @param plyr_idx caster player.
 * @param target action point/hero gate.
 * @param fml_bytes encoded bytes: f=cast for free flag,m=power kind,l=power level.
 * @return TbResult whether the power was successfully cast
 */
TbResult script_use_power_at_location(PlayerNumber plyr_idx, TbMapLocation target, long fml_bytes)
{
    SYNCDBG(0, "Using power at location of type %lu", target);
    int32_t x = 0;
    int32_t y = 0;
    find_map_location_coords(target, &x, &y, plyr_idx, __func__);
    if ((x == 0) && (y == 0))
    {
        WARNLOG("Can't decode location %lu", target);
        return Lb_FAIL;
    }
    return script_use_power_at_pos(plyr_idx, x, y, fml_bytes);
}

/**
 * Casts a power for player.
 * @param plyr_idx caster player.
 * @param power_kind the power: magic id.
 * @param free cast for free flag.
 * @return TbResult whether the power was successfully cast
 */
TbResult script_use_power(PlayerNumber plyr_idx, PowerKind power_kind, char free)
{
    return magic_use_power_on_level(plyr_idx, power_kind, 1, free != 0 ? PwMod_CastForFree : 0); // power_level gets ignored anyway -> pass 1
}

/**
 * Cast a keeper power on a creature which meets given criteria.
 * @param plyr_idx The player whose creature will be affected.
 * @param crmodel Model of the creature to find.
 * @param criteria Criteria, from CreatureSelectCriteria enumeration.
 * @param fmcl_bytes encoded bytes: f=cast for free flag,m=power kind,c=caster player index,l=power level.
 * @return TbResult whether the power was successfully cast
 */
TbResult script_use_power_on_creature_matching_criterion(PlayerNumber plyr_idx, long crmodel, long criteria, long fmcl_bytes)
{
    struct Thing* thing = script_get_creature_by_criteria(plyr_idx, crmodel, criteria);
    if (thing_is_invalid(thing)) {
        SYNCDBG(5, "No matching player %d creature of model %d (%s) found to use power on.", (int)plyr_idx, (int)crmodel, creature_code_name(crmodel));
        return Lb_FAIL;
    }

    char is_free = (fmcl_bytes >> 24) != 0;
    PowerKind pwkind = (fmcl_bytes >> 16) & 255;
    PlayerNumber caster = (fmcl_bytes >> 8) & 255;
    KeepPwrLevel power_level = fmcl_bytes & 255;
    return script_use_power_on_creature(thing, pwkind, power_level, caster, is_free);
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
