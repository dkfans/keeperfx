/** @file keeper_reflexes.c
 *     The hand rules' decisions (keeper_reflexes.h), moved from the External seat (seat_reflexes.c, D8).
 */
#include "pre_inc.h"
#include "keeper_reflexes.h"

#include <string.h>

#include "bflib_basics.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PICKS 64
#define MAX_THREATS 64
#define MAX_BATTLE_MEMBERS 64
#define NEAR_DEST 6               /**< subtiles: a creature this near the destination is there already */
/** Map units added to a boulder's reach to its side: a creature at work shuffles about, so one at the lane's edge is in it. */
#define BOULDER_SIDE_MARGIN (DAI_COORD_PER_STL / 2)

struct Pick { DaiThingIndex id; int64_t key; };

static int64_t dist(int64_t ax, int64_t ay, int64_t bx, int64_t by)
{
    const int64_t dx = (ax > bx) ? ax - bx : bx - ax, dy = (ay > by) ? ay - by : by - ay;
    return (dx > dy) ? dx : dy;
}

static TbBool has_cond(const struct KeeperAssistRule *r, enum KeeperAssistFact fact)
{
    for (int c = 0; c < r->cond_count; c++) if (r->cond[c].fact == fact) return true;
    return false;
}

static int64_t hand_cap(const struct KeeperReflexInput *in, const struct KeeperAssistRule *r)
{
    int64_t cap = in->hand_max;
    if ((cap < 1) || (cap > KEEPER_ASSIST_HAND_MAX)) cap = KEEPER_ASSIST_HAND_MAX;
    return (r->count < cap) ? r->count : cap;
}

static void sort_picks(struct Pick *a, int64_t n)  // ascending key, stable
{
    for (int64_t i = 1; i < n; i++) {
        const struct Pick v = a[i];
        int64_t j = i - 1;
        while ((j >= 0) && (a[j].key > v.key)) { a[j + 1] = a[j]; j--; }
        a[j + 1] = v;
    }
}

int64_t keeper_reflexes_lift_turns(int64_t k)
{
    return KEEPER_REFLEX_GRAB_TURNS * (k + 1);
}

int64_t keeper_reflexes_impact_turns(int64_t bx, int64_t by, int dx, int dy, int64_t speed, int64_t cx, int64_t cy, int64_t reach)
{
    if (speed <= 0) return -1;
    const int64_t ahead = (cx - bx) * dx + (cy - by) * dy;
    int64_t side = (cx - bx) * dy - (cy - by) * dx;
    if (side < 0) side = -side;
    if ((side > reach) || (ahead < -reach)) return -1;
    const int64_t gap = ahead - reach;
    return (gap <= 0) ? 0 : (gap + speed - 1) / speed;
}

/** The lift of `n` picks (at most the hand's cap of them is the caller's), to (x, y) or room `room`, carried out. */
static TbBool fire(const struct KeeperReflexInput *in, int slot, const struct KeeperAssistRule *r, const struct Pick *picks, int64_t n,
    int64_t x, int64_t y, int64_t room, DaiThingIndex boulder, KeeperReflexExec exec, void *exec_ctx, const char **refusal)
{
    (void)in;
    struct KeeperReflexLift lift;
    memset(&lift, 0, sizeof(lift));
    lift.slot = slot;
    lift.action = r->action;
    lift.count = (uint8_t)n;
    for (int64_t i = 0; (i < n) && (i < KEEPER_ASSIST_HAND_MAX); i++) lift.things[i] = picks[i].id;
    lift.x = x;
    lift.y = y;
    lift.room_id = room;
    lift.boulder = boulder;
    const char *err = exec(exec_ctx, &lift);
    if (err != NULL) { refusal[slot] = err; return false; }
    refusal[slot] = NULL;
    return true;
}

/** Turns until boulder `b` reaches the creature `t`, or -1: out of its way, further than `along` map units down its lane,
 *  or past its first wall (it turns there). */
static int64_t boulder_reaches(const struct KeeperReflexCandidate *b, const struct KeeperReflexCandidate *t, int64_t along)
{
    const int64_t reach = (b->size + t->size) / 2;
    const int64_t turns = keeper_reflexes_impact_turns(b->pos_x, b->pos_y, b->dx, b->dy, b->speed, t->pos_x, t->pos_y, reach + BOULDER_SIDE_MARGIN);
    if ((turns < 0) || (turns * b->speed > along)) return -1;
    const int64_t steps = ((t->pos_x - b->pos_x) * b->dx + (t->pos_y - b->pos_y) * b->dy) / DAI_COORD_PER_STL;
    if (steps > b->lane_clear) return -1;   // a wall at some subtile from 1 to steps - 1 between them
    return turns;
}

/** evacuate_diggers against rolling boulders: the keeper's diggers (or, with an is_digger condition, the creatures it
 *  allows) in a boulder's way within the rule's radius, soonest hit first -- those the hand can still take before it
 *  arrives. Else BOULDER_TOO_CLOSE when every one in its way is too near. */
static TbBool try_evacuate_boulders(const struct KeeperReflexInput *in, const struct KeeperCtlWorld *w, int slot,
    const struct KeeperAssistRule *r, KeeperReflexExec exec, void *exec_ctx, const char **refusal)
{
    TbBool any = false;
    for (int64_t k = 0; k < in->n; k++) any = any || (in->cands[k].role == KRR_Boulder);
    if (!any) return false;
    const int64_t along = 3 * (int64_t)r->radius * DAI_COORD_PER_STL;
    const TbBool all_kinds = has_cond(r, KAF_IsDigger);
    struct Pick picks[MAX_PICKS];
    DaiThingIndex cause[MAX_PICKS];
    int64_t n = 0;
    for (int pass = all_kinds ? 0 : 1; pass < 2; pass++) {
        const uint8_t role = (pass == 0) ? KRR_Fighter : KRR_Digger;
        for (int64_t i = 0; (i < in->n) && (n < MAX_PICKS); i++) {
            const struct KeeperReflexCandidate *t = &in->cands[i];
            if (t->role != role) continue;
            if (dist(t->x, t->y, r->dest_x, r->dest_y) <= NEAR_DEST) continue; // safe already
            int64_t soonest = -1;
            DaiThingIndex by = 0;
            for (int64_t k = 0; k < in->n; k++) {
                if (in->cands[k].role != KRR_Boulder) continue;
                const int64_t turns = boulder_reaches(&in->cands[k], t, along);
                if ((turns >= 0) && ((soonest < 0) || (turns < soonest))) { soonest = turns; by = in->cands[k].thing; }
            }
            if (soonest < 0) continue;
            if (!keeper_assist_conds_hold(r, in->dungeon_facts, &t->ac)) continue;
            picks[n].id = t->thing; picks[n].key = soonest; cause[n] = by; n++;
        }
    }
    if (n == 0) return false;
    // Soonest hit first; keep each one the hand still reaches first, in the order it takes them.
    for (int64_t a = 1; a < n; a++) {
        const struct Pick v = picks[a];
        const DaiThingIndex vc = cause[a];
        int64_t j = a - 1;
        while ((j >= 0) && (picks[j].key > v.key)) { picks[j + 1] = picks[j]; cause[j + 1] = cause[j]; j--; }
        picks[j + 1] = v; cause[j + 1] = vc;
    }
    const int64_t cap = hand_cap(in, r);
    int64_t take = 0;
    DaiThingIndex boulder = 0;
    for (int64_t a = 0; (a < n) && (take < cap); a++) {
        if (picks[a].key <= keeper_reflexes_lift_turns(take)) continue; // it would be hit before the hand closes on it
        if (take == 0) boulder = cause[a];
        picks[take++] = picks[a];
    }
    if (take == 0) { refusal[slot] = "BOULDER_TOO_CLOSE"; return false; }
    DaiSubtlCoord x = 0, y = 0;
    if (w->find_drop_spot(w->ctx, in->plyr, picks[0].id, r->dest_x, r->dest_y, 15, -1, -1, &x, &y) != NULL) {
        refusal[slot] = "NO_DROP_SPOT";
        return false;
    }
    return fire(in, slot, r, picks, take, x, y, 0, boulder, exec, exec_ctx, refusal);
}

/** evacuate_diggers: the diggers near a visible enemy fighter that can get to them, nearest the danger first. */
static TbBool try_evacuate(const struct KeeperReflexInput *in, const struct KeeperCtlWorld *w, int slot,
    const struct KeeperAssistRule *r, KeeperReflexExec exec, void *exec_ctx, const char **refusal)
{
    int64_t tx[MAX_THREATS], ty[MAX_THREATS], nt = 0;
    for (int64_t k = 0; (k < in->n) && (nt < MAX_THREATS); k++) {
        if (in->cands[k].role != KRR_Enemy) continue;
        tx[nt] = in->cands[k].x; ty[nt] = in->cands[k].y; nt++;
    }
    if (nt == 0) return false;
    const int64_t reach = 3 * (int64_t)r->radius;
    struct Pick picks[MAX_PICKS];
    int64_t n = 0;
    for (int64_t i = 0; (i < in->n) && (n < MAX_PICKS); i++) {
        const struct KeeperReflexCandidate *t = &in->cands[i];
        if ((t->role != KRR_Digger) || t->ordered) continue;
        const int64_t x = t->x, y = t->y;
        if (dist(x, y, r->dest_x, r->dest_y) <= NEAR_DEST) continue; // safe already
        // Only an enemy that can get to the digger threatens it: one behind rock or walls nothing walks through, however
        // near, does not (lifting the imps there only stopped their work).
        int64_t nearest = -1;
        for (int64_t k = 0; k < nt; k++) {
            const int64_t d = dist(x, y, tx[k], ty[k]);
            if ((d > reach) || ((nearest >= 0) && (d >= nearest))) continue;
            if (!w->connected(w->ctx, (DaiSubtlCoord)tx[k], (DaiSubtlCoord)ty[k], (DaiSubtlCoord)x, (DaiSubtlCoord)y)) continue;
            nearest = d;
        }
        if (nearest < 0) continue;
        if (!keeper_assist_conds_hold(r, in->dungeon_facts, &t->ac)) continue;
        picks[n].id = t->thing; picks[n].key = nearest; n++;
    }
    if (n == 0) return false;
    sort_picks(picks, n);
    const int64_t take = (n < hand_cap(in, r)) ? n : hand_cap(in, r);
    DaiSubtlCoord x = 0, y = 0;
    if (w->find_drop_spot(w->ctx, in->plyr, picks[0].id, r->dest_x, r->dest_y, 15, -1, -1, &x, &y) != NULL) {
        refusal[slot] = "NO_DROP_SPOT";
        return false;
    }
    return fire(in, slot, r, picks, take, x, y, 0, 0, exec, exec_ctx, refusal);
}

/** lift_wounded: fighters in a fight under the health threshold, the most hurt first -- when no heal rule can heal them. */
static TbBool try_lift_wounded(const struct KeeperReflexInput *in, const struct KeeperCtlWorld *w, int slot,
    const struct KeeperAssistRule *r, KeeperReflexExec exec, void *exec_ctx, const char **refusal)
{
    struct Pick picks[MAX_PICKS];
    int64_t n = 0;
    const TbBool own_threshold = has_cond(r, KAF_HealthPct);
    for (int64_t i = 0; (i < in->n) && (n < MAX_PICKS); i++) {
        const struct KeeperReflexCandidate *t = &in->cands[i];
        if ((t->role != KRR_Wounded) || t->ordered) continue;
        if (!own_threshold && (t->ac.facts[KAF_HealthPct] >= KEEPER_ASSIST_DEFAULT_WOUNDED_PCT)) continue;
        if (!keeper_assist_conds_hold(r, in->dungeon_facts, &t->ac)) continue;
        picks[n].id = t->thing; picks[n].key = t->ac.facts[KAF_HealthPct]; n++;
    }
    if (n == 0) return false;
    if (in->heal_ready) { refusal[slot] = "HEAL_READY"; return false; }
    sort_picks(picks, n);
    const int64_t take = (n < hand_cap(in, r)) ? n : hand_cap(in, r);
    int64_t x = 0, y = -1, room = 0;
    if ((r->dest_x != 0) || (r->dest_y != 0)) {
        DaiSubtlCoord sx = 0, sy = 0;
        if (w->find_drop_spot(w->ctx, in->plyr, picks[0].id, r->dest_x, r->dest_y, 15, -1, -1, &sx, &sy) != NULL) {
            refusal[slot] = "NO_DROP_SPOT";
            return false;
        }
        x = sx; y = sy;
    } else {
        room = in->lair_room;
        if (room == 0) { refusal[slot] = "NO_LAIR"; return false; }
        x = room;
    }
    return fire(in, slot, r, picks, take, x, y, room, 0, exec, exec_ctx, refusal);
}

/** reinforce: the fight the keeper is doing worst in that its healthy, matching fighters from elsewhere could turn. */
static TbBool try_reinforce(const struct KeeperReflexInput *in, const struct KeeperCtlWorld *w, int slot,
    const struct KeeperAssistRule *r, KeeperReflexExec exec, void *exec_ctx, const char **refusal)
{
    const char *why = NULL;
    do {
        int64_t best_ratio = -1, best_x = 0, best_y = 0, best_ally = 0, best_enemy = 0, best_max_enemy = 0, best_ex = -1, best_ey = -1;
        for (int64_t i = 0; i < in->n;) {
            const uint8_t role = in->cands[i].role;
            if ((role != KRR_BattleOwn) && (role != KRR_BattleEnemy)) { i++; continue; }
            // One battle's members, together.
            const BattleIndex b = in->cands[i].battle;
            int64_t ally = 0, enemy = 0, max_enemy = 0, sx = 0, sy = 0, nown = 0, members = 0, ex = -1, ey = -1;
            const struct KeeperReflexCandidate *strongest_own = NULL;
            for (; (i < in->n) && (in->cands[i].battle == b)
                   && ((in->cands[i].role == KRR_BattleOwn) || (in->cands[i].role == KRR_BattleEnemy)); i++) {
                const struct KeeperReflexCandidate *t = &in->cands[i];
                if (members >= MAX_BATTLE_MEMBERS) continue;
                if (t->role == KRR_BattleOwn) {
                    ally += t->force;
                    sx += t->x; sy += t->y; nown++;
                    if ((strongest_own == NULL) || (t->force > strongest_own->force)) strongest_own = t;
                } else {
                    enemy += t->force;
                    if (t->force > max_enemy) { max_enemy = t->force; ex = t->x; ey = t->y; }
                }
                members++;
            }
            if ((nown == 0) || (enemy == 0)) continue;
            const int64_t ratio = enemy * 100 / ((ally > 0) ? ally : 1);
            // The rule's conditions read the fight as one of the keeper's creatures in it sees it.
            struct KeeperAssistCandidate c = strongest_own->ac;
            c.facts[KAF_BattleAllyForce] = ally;
            c.facts[KAF_BattleEnemyForce] = enemy;
            c.facts[KAF_ForceRatio] = ratio;
            c.facts[KAF_BattleSize] = members;
            if (!has_cond(r, KAF_ForceRatio) && (ratio < KEEPER_ASSIST_DEFAULT_REINFORCE_RATIO)) continue;
            if (!keeper_assist_conds_hold(r, in->dungeon_facts, &c)) continue;
            if (ratio > best_ratio) {
                best_ratio = ratio; best_x = sx / nown; best_y = sy / nown;
                best_ally = ally; best_enemy = enemy; best_max_enemy = max_enemy; best_ex = ex; best_ey = ey;
            }
        }
        if (best_ratio < 0) return false;
        // Who could go: healthy fighters out of every fight, away from this one, each a match for its strongest enemy.
        const int64_t floor = best_max_enemy * (int64_t)r->min_match / 100;
        const int64_t reach = 3 * (int64_t)r->radius;
        struct Pick picks[MAX_PICKS];
        int64_t n = 0;
        for (int64_t i = 0; (i < in->n) && (n < MAX_PICKS); i++) {
            const struct KeeperReflexCandidate *t = &in->cands[i];
            if ((t->role != KRR_Fighter) || t->digger || (t->battle != 0) || t->ordered) continue;
            if ((t->health_pct < 60) || (dist(t->x, t->y, best_x, best_y) <= reach)) continue;
            if (t->force < floor) continue;
            picks[n].id = t->thing; picks[n].key = -t->force; n++;   // strongest first
        }
        if (n == 0) { why = "NO_MATCH"; break; }
        sort_picks(picks, n);
        // Only enough to turn it (a tenth over the enemy's force), and none if even all it may send would not.
        const int64_t cap = hand_cap(in, r);
        int64_t take = 0, sum = best_ally;
        while ((take < n) && (take < cap) && (sum * 100 < best_enemy * 110)) { sum += -picks[take].key; take++; }
        if (sum < best_enemy) { why = "WOULD_NOT_TURN_IT"; break; }
        if (take == 0) return false;
        // Beside the fight, where they can walk to its strongest enemy: not in a pocket the rock seals off from it.
        DaiSubtlCoord x = 0, y = 0;
        if (w->find_drop_spot(w->ctx, in->plyr, picks[0].id, (DaiSubtlCoord)best_x, (DaiSubtlCoord)best_y, 9, (DaiSubtlCoord)best_ex,
                (DaiSubtlCoord)best_ey, &x, &y) != NULL) { why = "NO_DROP_SPOT"; break; }
        return fire(in, slot, r, picks, take, x, y, 0, 0, exec, exec_ctx, refusal);
    } while (0);
    refusal[slot] = why;
    return false;
}

TbBool keeper_reflexes_decide(const struct KeeperReflexInput *in, const struct KeeperCtlWorld *world, KeeperReflexExec exec,
    void *exec_ctx, const char **refusal)
{
    for (int s = 0; s < KEEPER_ASSIST_RULES; s++) {
        const struct KeeperAssistRule *r = &in->rules[s];
        if (!r->used || !keeper_assist_action_is_hand(r->action)) continue;
        const TbBool cooling = (in->last_fire_turn[s] != 0) && (in->now - in->last_fire_turn[s] < r->cooldown);
        TbBool fired = false;
        // A boulder does not wait for the cooldown: it is a few seconds away, and the creatures in its way are others.
        if (r->action == KAA_EvacuateDiggers) fired = try_evacuate_boulders(in, world, s, r, exec, exec_ctx, refusal);
        if (fired) return true;
        if (cooling) continue;
        switch (r->action) {
        case KAA_EvacuateDiggers: fired = try_evacuate(in, world, s, r, exec, exec_ctx, refusal); break;
        case KAA_LiftWounded: fired = try_lift_wounded(in, world, s, r, exec, exec_ctx, refusal); break;
        case KAA_Reinforce: fired = try_reinforce(in, world, s, r, exec, exec_ctx, refusal); break;
        default: break;
        }
        if (fired) return true; // one trip at a time
    }
    return false;
}

#ifdef __cplusplus
}
#endif
