/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file keeper_group.c
 *     A keeper's one group, formed, ordered and released by packets (keeper_group.h).
 */
/******************************************************************************/
#include "pre_inc.h"
#include "keeper_group.h"

#include <stdlib.h>
#include <string.h>

#include "bflib_basics.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "creature_control.h"
#include "creature_groups.h"
#include "creature_states.h"
#include "creature_states_combt.h"
#include "dungeon_data.h"
#include "game_legacy.h"
#include "map_data.h"
#include "player_data.h"
#include "room_data.h"
#include "slab_data.h"
#include "thing_creature.h"
#include "thing_data.h"
#include "thing_doors.h"
#include "thing_list.h"
#include "thing_navigate.h"
#include "thing_traps.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

_Static_assert(KEEPER_GROUP_MAX == GROUP_MEMBERS_COUNT, "a keeper's group holds as many creatures as the engine's");

static TbBool slab_on_map(int64_t x, int64_t y)
{
    return (x >= 0) && (y >= 0) && (x < game.map_tiles_x) && (y < game.map_tiles_y);
}

// ---- the order rules ----------------------------------------------------------------------------------------------------

TbBool keeper_order_in_manual_control(const struct Thing *thing)
{
    return (thing->active_state == CrSt_ManualControl) || (thing->continue_state == CrSt_ManualControl);
}

TbBool keeper_order_orderable(const struct Thing *thing)
{
    return !thing_is_invalid(thing) && thing_is_creature(thing) && !thing_is_picked_up(thing) && !creature_is_dying(thing)
        && !creature_is_being_unconscious(thing) && !creature_is_kept_in_custody(thing);
}

static int64_t payday_gap(PlayerNumber plyr_idx)
{
    return (int64_t)game.conf.rules[plyr_idx].gameplay.pay_day_gap;
}

int64_t keeper_order_turns_to_payday(PlayerNumber plyr_idx)
{
    const int64_t gap = payday_gap(plyr_idx);
    const int64_t per_turn = (int64_t)game.conf.rules[plyr_idx].gameplay.pay_day_speed / 100;
    const int64_t progress = (int64_t)game.pay_day_progress[plyr_idx];
    if ((per_turn <= 0) || (gap <= progress)) {
        return 0;
    }
    return (gap - progress + per_turn - 1) / per_turn;
}

int64_t keeper_order_default_hold_turns(PlayerNumber plyr_idx)
{
    const int64_t q = payday_gap(plyr_idx) / 4;
    return (q > KEEPER_ORDER_MIN_HOLD_TURNS) ? q : KEEPER_ORDER_MIN_HOLD_TURNS;
}

int64_t keeper_order_max_hold_turns(PlayerNumber plyr_idx)
{
    const int64_t h = payday_gap(plyr_idx) / 2;
    return (h > KEEPER_ORDER_MIN_HOLD_TURNS) ? h : KEEPER_ORDER_MIN_HOLD_TURNS;
}

TbBool keeper_order_near_hunger_limit(const struct Thing *thing)
{
    const struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    const struct CreatureModelConfig *crconf = creature_stats_get_from_thing(thing);
    if (creature_control_invalid(cctrl) || creature_stats_invalid(crconf) || (crconf->hunger_rate == 0)) {
        return false;
    }
    return (cctrl->hunger_level * 4) >= ((int64_t)crconf->hunger_rate * 3);
}

TbBool keeper_order_owed_pay(const struct Thing *thing)
{
    const struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    return !creature_control_invalid(cctrl) && (cctrl->paydays_owed > 0);
}

const char *keeper_order_attack_target_at(PlayerNumber plyr_idx, DaiSubtlCoord x, DaiSubtlCoord y)
{
    const DaiSlabCoord sx = DaiSubtileSlab(x), sy = DaiSubtileSlab(y);
    for (int64_t dy = -1; dy <= 1; dy++) {
        for (int64_t dx = -1; dx <= 1; dx++) {
            if ((dx != 0) && (dy != 0)) continue; // the slab and the four beside it, as check_for_door_to_fight
            if (!slab_on_map(sx + dx, sy + dy)) continue;
            const DaiSubtlCoord cx = DaiSlabSubtileCenter(sx + dx), cy = DaiSlabSubtileCenter(sy + dy);
            const struct Thing *door = get_door_for_position(cx, cy);
            if (!thing_is_invalid(door) && players_are_enemies(plyr_idx, door->owner) && thing_revealed(door, plyr_idx)
                && !door_is_hidden_to_player((struct Thing *)door, plyr_idx)) return "door";
        }
    }
    for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) {
        if ((p == plyr_idx) || !players_are_enemies(plyr_idx, p)) continue;
        // Only a heart it has seen: an order near an unseen one must not tell the keeper it is there.
        const struct Thing *heart = find_players_dungeon_heart(p);
        if (!thing_is_invalid(heart) && thing_revealed(heart, plyr_idx) && (llabs((int64_t)heart->mappos.x.stl.num - x) <= KEEPER_ATTACK_HEART_REACH)
            && (llabs((int64_t)heart->mappos.y.stl.num - y) <= KEEPER_ATTACK_HEART_REACH)) return "heart";
    }
    for (int64_t dy = -1; dy <= 1; dy++) {
        for (int64_t dx = -1; dx <= 1; dx++) {
            if ((dx != 0) && (dy != 0)) continue;
            if (!slab_on_map(sx + dx, sy + dy)) continue;
            const struct Thing *trap = get_trap_for_position(DaiSlabSubtileCenter(sx + dx), DaiSlabSubtileCenter(sy + dy));
            if (!thing_is_invalid(trap) && players_are_enemies(plyr_idx, trap->owner) && thing_revealed(trap, plyr_idx)
                && (!get_trap_model_stats(trap->model)->hidden || trap->trap.revealed)) return "trap";
        }
    }
    const struct Room *room = subtile_room_get(x, y);
    if (!room_is_invalid(room) && subtile_revealed(x, y, plyr_idx) && !players_creatures_tolerate_each_other(plyr_idx, room->owner)
        && !room_cannot_vandalise(room->kind)) {
        return "room";
    }
    int64_t idx = game.thing_lists[TngList_Creatures].index;
    for (int64_t guard = 0; (idx > 0) && (guard < THINGS_COUNT); guard++) {
        const struct Thing *t = thing_get(idx);
        idx = t->next_of_class;
        if (!players_are_enemies(plyr_idx, t->owner) || !thing_revealed(t, plyr_idx) || creature_is_invisible(t)) continue;
        if ((llabs((int64_t)t->mappos.x.stl.num - x) <= KEEPER_ATTACK_CREATURE_REACH)
            && (llabs((int64_t)t->mappos.y.stl.num - y) <= KEEPER_ATTACK_CREATURE_REACH)) return "creatures";
    }
    return NULL;
}

TbBool keeper_order_stopped_at_enemy_door(const struct Thing *thing, DaiSubtlCoord goal_x, DaiSubtlCoord goal_y)
{
    const DaiSlabCoord gx = DaiSubtileSlab(goal_x), gy = DaiSubtileSlab(goal_y);
    for (int64_t n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++) {
        const DaiSlabCoord sx = DaiSubtileSlab(thing->mappos.x.stl.num) + (int64_t)small_around[n].delta_x;
        const DaiSlabCoord sy = DaiSubtileSlab(thing->mappos.y.stl.num) + (int64_t)small_around[n].delta_y;
        if ((llabs((int64_t)sx - gx) > 1) || (llabs((int64_t)sy - gy) > 1)) continue;
        const struct Thing *door = get_door_for_position(DaiSlabSubtileCenter(sx), DaiSlabSubtileCenter(sy));
        if (!thing_is_invalid(door) && (door->owner != thing->owner)) return true;
    }
    return false;
}

void keeper_order_start_attack(struct Thing *thing, DaiSubtlCoord goal_x, DaiSubtlCoord goal_y)
{
    set_start_state(thing);
    struct Thing *target = check_for_door_to_fight(thing);
    if (thing_exists(target)) {
        set_creature_door_combat(thing, target);
    } else if (!creature_look_for_enemy_heart_combat(thing)) {
        target = check_for_object_to_fight(thing);
        if (thing_exists(target)) {
            set_creature_object_combat(thing, target);
        } else if (!attempt_to_destroy_enemy_room(thing, goal_x, goal_y)) {
            attempt_to_destroy_enemy_room(thing, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
        }
    }
}

// ---- the group ----------------------------------------------------------------------------------------------------------

static struct KeeperGroupState *group_of(PlayerNumber plyr_idx)
{
    return &game.keeper_group[plyr_idx];
}

const struct KeeperGroupState *keeper_group_get(PlayerNumber plyr_idx)
{
    static const struct KeeperGroupState none;
    return ((plyr_idx >= 0) && (plyr_idx < PLAYERS_COUNT)) ? group_of(plyr_idx) : &none;
}

/** Member `i` while it is still the creature that joined, else NULL. */
static struct Thing *member_thing(const struct KeeperGroupState *g, int i)
{
    struct Thing *thing = thing_get(g->member[i]);
    if (thing_is_invalid(thing) || !thing_is_creature(thing) || (thing->creation_turn != g->member_creation[i])) return NULL;
    return thing;
}

TbBool keeper_group_has(PlayerNumber plyr_idx, DaiThingIndex thing)
{
    const struct KeeperGroupState *g = keeper_group_get(plyr_idx);
    for (int i = 0; i < g->count; i++) {
        if (g->member[i] == thing) return true;
    }
    return false;
}

/** The checks a new member (or leader) passes. */
static const char *joiner_ok(PlayerNumber plyr_idx, DaiThingIndex idx, struct Thing **out)
{
    struct Thing *thing = thing_get(idx);
    if ((idx <= 0) || thing_is_invalid(thing) || !thing_is_creature(thing)) return "NO_SUCH_THING";
    if (thing->owner != plyr_idx) return "NOT_YOURS";
    if (!keeper_order_orderable(thing)) return "NOT_ORDERABLE";
    if (keeper_group_has(plyr_idx, idx)) return "IN_GROUP";
    if (creature_is_group_member(thing)) return "IN_ANOTHER_GROUP";
    *out = thing;
    return NULL;
}

const char *keeper_group_form(PlayerNumber plyr_idx, DaiThingIndex leader, TbBool apply)
{
    if ((plyr_idx < 0) || (plyr_idx >= PLAYERS_COUNT)) return "NO_SUCH_PLAYER";
    struct KeeperGroupState *g = group_of(plyr_idx);
    if (g->count > 0) return "HAS_GROUP";
    struct Thing *thing = NULL;
    const char *why = joiner_ok(plyr_idx, leader, &thing);
    if ((why != NULL) || !apply) return why;
    memset(g, 0, sizeof(*g));
    g->count = 1;
    g->member[0] = thing->index;
    g->member_creation[0] = thing->creation_turn;
    SYNCDBG(6, "Player %d formed a group led by %s index %d", (int)plyr_idx, thing_model_name(thing), (int)thing->index);
    return NULL;
}

const char *keeper_group_add(PlayerNumber plyr_idx, DaiThingIndex member, TbBool apply)
{
    if ((plyr_idx < 0) || (plyr_idx >= PLAYERS_COUNT)) return "NO_SUCH_PLAYER";
    struct KeeperGroupState *g = group_of(plyr_idx);
    if (g->count == 0) return "NO_GROUP";
    struct Thing *leader = member_thing(g, 0);
    if (leader == NULL) return "NO_GROUP";
    int64_t cap = game.conf.rules[plyr_idx].rooms.barrack_max_party_size;
    if (cap > KEEPER_GROUP_MAX) cap = KEEPER_GROUP_MAX;
    if (g->count >= cap) return "GROUP_FULL";
    // A human forms a group of more than one in a barracks (any size; 07 Q6).
    if (!player_has_room_of_role(plyr_idx, RoRoF_CrMakeGroup)) return "NO_BARRACKS";
    struct Thing *thing = NULL;
    const char *why = joiner_ok(plyr_idx, member, &thing);
    if ((why != NULL) || !apply) return why;
    if (!add_creature_to_group(thing, leader)) return "CANNOT_JOIN";
    g->member[g->count] = thing->index;
    g->member_creation[g->count] = thing->creation_turn;
    g->count++;
    return NULL;
}

const char *keeper_group_order(PlayerNumber plyr_idx, uint8_t order, DaiSubtlCoord x, DaiSubtlCoord y, int64_t hold_turns,
    TbBool apply)
{
    if ((plyr_idx < 0) || (plyr_idx >= PLAYERS_COUNT)) return "NO_SUCH_PLAYER";
    struct KeeperGroupState *g = group_of(plyr_idx);
    if (g->count == 0) return "NO_GROUP";
    struct Thing *leader = member_thing(g, 0);
    if ((leader == NULL) || !keeper_order_orderable(leader)) return "NOT_ORDERABLE";
    if ((order != KGO_Move) && (order != KGO_Attack)) return "UNKNOWN_ORDER";
    if ((hold_turns < 0) || (hold_turns > keeper_order_max_hold_turns(plyr_idx))) return "BAD_HOLD";
    for (int i = 0; i < g->count; i++) {
        const struct Thing *t = member_thing(g, i);
        if (t == NULL) continue;
        if (keeper_order_near_hunger_limit(t)) return "HUNGRY";
        if (keeper_order_owed_pay(t)) return "OWED_PAY";
    }
    if (keeper_order_turns_to_payday(plyr_idx) <= KEEPER_ORDER_PAYDAY_MARGIN_TURNS) return "PAYDAY_TOO_CLOSE";
    if ((x < 0) || (y < 0) || (x >= game.map_subtiles_x) || (y >= game.map_subtiles_y)) return "POSITION_OFF_MAP";
    if ((x == leader->mappos.x.stl.num) && (y == leader->mappos.y.stl.num)) return "ALREADY_THERE";
    if ((order == KGO_Attack) && (keeper_order_attack_target_at(plyr_idx, x, y) == NULL)) return "NOTHING_TO_ATTACK";
    if (!apply) return NULL;
    // As Order creature mode does with a creature it selects: held, the group's leader, sent on its way.
    if (creature_is_group_member(leader) && (get_group_leader(leader) != leader)) make_group_member_leader(leader);
    initialise_thing_state(leader, CrSt_ManualControl);
    if (!setup_person_move_to_position(leader, x, y, NavRtF_Default)) {
        set_start_state(leader);
        return "CANNOT_REACH";
    }
    leader->continue_state = CrSt_ManualControl;
    const DaiTurn now = get_gameturn();
    g->order = order;
    g->goal_x = x;
    g->goal_y = y;
    g->ordered_turn = now;
    g->hold_until = now + ((hold_turns > 0) ? hold_turns : keeper_order_default_hold_turns(plyr_idx));
    return NULL;
}

/** Out of the engine's group, every member; back to its own behaviour, each that was held (unless `keep_state`). */
static void dissolve(PlayerNumber plyr_idx, TbBool keep_state)
{
    struct KeeperGroupState *g = group_of(plyr_idx);
    for (int i = g->count - 1; i >= 0; i--) {
        struct Thing *thing = member_thing(g, i);
        if (thing == NULL) continue;
        if (creature_is_group_member(thing)) remove_creature_from_group(thing);
        if (!keep_state && keeper_order_in_manual_control(thing)) set_start_state(thing);
    }
    memset(g, 0, sizeof(*g));
}

const char *keeper_group_release(PlayerNumber plyr_idx, const char *reason, TbBool apply)
{
    if ((plyr_idx < 0) || (plyr_idx >= PLAYERS_COUNT)) return "NO_SUCH_PLAYER";
    if (group_of(plyr_idx)->count == 0) return "NO_GROUP";
    if (!apply) return NULL;
    SYNCDBG(6, "Player %d's group released (%s)", (int)plyr_idx, (reason != NULL) ? reason : "");
    dissolve(plyr_idx, false);
    return NULL;
}

/** Members that died, changed owner or left the engine's group are no longer in it; without its leader, no group. */
static TbBool prune(PlayerNumber plyr_idx)
{
    struct KeeperGroupState *g = group_of(plyr_idx);
    const struct Thing *leader = member_thing(g, 0);
    if ((leader == NULL) || (leader->owner != plyr_idx) || creature_is_dying(leader)) return false;
    int kept = 1;
    for (int i = 1; i < g->count; i++) {
        struct Thing *thing = member_thing(g, i);
        if ((thing == NULL) || (thing->owner != plyr_idx) || creature_is_dying(thing) || !creature_is_group_member(thing)
            || (get_group_leader(thing) != leader)) {
            if ((thing != NULL) && keeper_order_in_manual_control(thing)) set_start_state(thing);
            continue;
        }
        g->member[kept] = g->member[i];
        g->member_creation[kept] = g->member_creation[i];
        kept++;
    }
    for (int i = kept; i < g->count; i++) { g->member[i] = 0; g->member_creation[i] = 0; }
    g->count = (uint8_t)kept;
    return true;
}

static void process_group(PlayerNumber plyr_idx, DaiTurn now)
{
    struct KeeperGroupState *g = group_of(plyr_idx);
    if (!prune(plyr_idx)) {
        keeper_group_release(plyr_idx, "leader_lost", true);
        return;
    }
    if (g->order == KGO_None) return;
    struct Thing *leader = member_thing(g, 0);
    if (!keeper_order_in_manual_control(leader)) {
        // Handed back by something else (the hand, a script): the order is over.
        if (now > g->ordered_turn + 2) keeper_group_release(plyr_idx, "order_ended", true);
        return;
    }
    if ((g->order == KGO_Attack) && (leader->active_state == CrSt_ManualControl)
        && (((llabs((int64_t)leader->mappos.x.stl.num - g->goal_x) <= 1) && (llabs((int64_t)leader->mappos.y.stl.num - g->goal_y) <= 1))
            || keeper_order_stopped_at_enemy_door(leader, g->goal_x, g->goal_y))) {
        // Arrived: every member fights what is there, out of the group.
        struct Thing *things[KEEPER_GROUP_MAX];
        int n = 0;
        for (int i = 0; i < g->count; i++) {
            struct Thing *thing = member_thing(g, i);
            if (thing != NULL) things[n++] = thing;
        }
        const DaiSubtlCoord gx = g->goal_x, gy = g->goal_y;
        dissolve(plyr_idx, true);
        for (int i = 0; i < n; i++) keeper_order_start_attack(things[i], gx, gy);
        SYNCDBG(6, "Player %d's group arrived and attacks", (int)plyr_idx);
        return;
    }
    const char *why = NULL;
    for (int i = 0; (i < g->count) && (why == NULL); i++) {
        const struct Thing *thing = member_thing(g, i);
        if (thing == NULL) continue;
        // A held creature does not fight back on its own, so an enemy engaging one of them would kill it where it stands.
        if (creature_is_being_attacked_by_enemy_player((struct Thing *)thing)) why = "attacked";
        else if (keeper_order_near_hunger_limit(thing)) why = "hungry";
        else if (keeper_order_owed_pay(thing)) why = "owed_pay";
    }
    if ((why == NULL) && (keeper_order_turns_to_payday(plyr_idx) <= KEEPER_ORDER_PAYDAY_MARGIN_TURNS)) why = "payday";
    if ((why == NULL) && (now >= g->hold_until)) why = "max_hold";
    if (why != NULL) keeper_group_release(plyr_idx, why, true);
}

void process_keeper_groups(void)
{
    const DaiTurn now = get_gameturn();
    for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) {
        if (group_of(p)->count > 0) process_group(p, now);
    }
}

#ifdef __cplusplus
}
#endif
