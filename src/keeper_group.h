/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file keeper_group.h
 *     A keeper's one group, formed, ordered and released by packets (PckA_FormGroup, PckA_GroupAdd, PckA_OrderGroup,
 *     PckA_ReleaseGroup; docs/plans/ai-plans/v2/06 section 5a): the stand-in for possessing a group's leader, limited as
 *     a human is -- one group, any of the keeper's creatures up to the rules' BarrackMaxPartySize, a barracks for more
 *     than one. Applied the same on every machine, so it works in a network game, unlike Order creature mode (a cheat).
 * @par Purpose:
 *     An order holds the leader in manual control walking to its goal while the others follow it (the engine's group);
 *     process_keeper_groups() ends it as the keeper controller's safety net ends a seat's order -- the creature fights
 *     on arrival (attack), or is handed back when attacked, hungry, owed pay, near pay day or when its hold is up --
 *     but in the simulation, so every machine ends it the same turn. The order helpers below are that net's rules,
 *     shared with the controller (kfx_net's keeper_ctl.c).
 */
/******************************************************************************/
#ifndef DK_KEEPER_GROUP_H
#define DK_KEEPER_GROUP_H

#include "globals.h"
#include "daimon_base_types.h"
#include "keeper_group_state.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Thing;

/** Orders end this many turns before pay day (a held creature does not collect its pay). */
#define KEEPER_ORDER_PAYDAY_MARGIN_TURNS 300
/** The shortest hold an order may ask for or be given. */
#define KEEPER_ORDER_MIN_HOLD_TURNS 200
/** An attack order's goal this near (subtiles) an enemy heart attacks the heart; this near an enemy creature, them. */
#define KEEPER_ATTACK_HEART_REACH 10
#define KEEPER_ATTACK_CREATURE_REACH 6

/** The keeper's group (count 0: none). */
const struct KeeperGroupState *keeper_group_get(PlayerNumber plyr_idx);
/** Whether `thing` is one of the keeper's group. */
TbBool keeper_group_has(PlayerNumber plyr_idx, DaiThingIndex thing);

/** Each returns NULL when it is allowed (and, with `apply`, done), else why not -- the same answer on every machine. */
/** Starts the keeper's group with `leader`: its own creature, orderable, in no group; the keeper has none yet. */
const char *keeper_group_form(PlayerNumber plyr_idx, DaiThingIndex leader, TbBool apply);
/** Adds a member: its own creature, orderable, in no group; at most BarrackMaxPartySize in all, and a barracks. */
const char *keeper_group_add(PlayerNumber plyr_idx, DaiThingIndex member, TbBool apply);
/** Orders the group (enum KeeperGroupOrder: move or attack) to the subtile, held at most `hold_turns` (0: a quarter
 *  of the pay-day gap): the leader orderable and none of them hungry or owed pay, not near pay day, the goal on the map
 *  and not where the leader stands, an attack's goal with an enemy there the keeper sees. */
const char *keeper_group_order(PlayerNumber plyr_idx, uint8_t order, DaiSubtlCoord x, DaiSubtlCoord y, int64_t hold_turns,
    TbBool apply);
/** Dissolves the group: every member out of the engine's group and back to its own behaviour. */
const char *keeper_group_release(PlayerNumber plyr_idx, const char *reason, TbBool apply);
/** Once per game turn, from the game loop: each keeper's group's order, ended as the safety net would. */
void process_keeper_groups(void);

/** The order rules (the keeper controller's safety net uses them too). */
TbBool keeper_order_in_manual_control(const struct Thing *thing);
TbBool keeper_order_orderable(const struct Thing *thing);
int64_t keeper_order_turns_to_payday(PlayerNumber plyr_idx);
int64_t keeper_order_default_hold_turns(PlayerNumber plyr_idx);
int64_t keeper_order_max_hold_turns(PlayerNumber plyr_idx);
TbBool keeper_order_near_hunger_limit(const struct Thing *thing);
TbBool keeper_order_owed_pay(const struct Thing *thing);
/** What an attack order sent to (x, y) would fight there, as the keeper sees it: "door", "heart", "trap", "room" or
 *  "creatures" (keeper_order_start_attack's order, enemy creatures last), or NULL when nothing there is its enemy. */
const char *keeper_order_attack_target_at(PlayerNumber plyr_idx, DaiSubtlCoord x, DaiSubtlCoord y);
/** A creature holding still beside an enemy door at most a slab from the goal has arrived (the walk ends there). */
TbBool keeper_order_stopped_at_enemy_door(const struct Thing *thing, DaiSubtlCoord goal_x, DaiSubtlCoord goal_y);
/** An attack order's creature has arrived: out of manual control and fighting what is there, in the order a creature
 *  at a Call to Arms banner looks -- an enemy door beside it, the enemy heart in sight, an enemy trap beside it, the
 *  enemy room at the goal or under it; with none of those, its own behaviour takes on the enemies about. */
void keeper_order_start_attack(struct Thing *thing, DaiSubtlCoord goal_x, DaiSubtlCoord goal_y);

#ifdef __cplusplus
}
#endif
#endif
