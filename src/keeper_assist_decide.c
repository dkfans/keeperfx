/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file keeper_assist_decide.c
 *     The combat assistant's decision (keeper_assist.h), with no world access but through its callbacks: the fork's keeper_assist.c, its "the decision" section, as Daimon's port tools copy it (scripts/daimon_port/sync.py in the fork). The world side is the port's own.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "keeper_assist.h"
#include <string.h>
#include "bflib_basics.h"
#include "config_magic.h"
#include "keeper_assist_state.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---- the decision (no world access but through the callbacks) ---------------------------------------------------------

/** magic.cfg's UseFunction (magic_use_func_commands): lightning 6, cave-in 9 -- a map pack's own power with one of
 *  them acts as KeeperFX's does. */
#define MAGIC_USE_LIGHTNING 6
#define MAGIC_USE_CAVE_IN 9

static int64_t use_function(DaiPowerKind power)
{
    const struct PowerConfigStats *powerst = get_power_model_stats(power);
    return (powerst != NULL) ? powerst->magic_use_func_idx : 0;
}

static TbBool lightning_like(DaiPowerKind power)
{
    return (power == PwrK_LIGHTNING) || (use_function(power) == MAGIC_USE_LIGHTNING);
}

static TbBool area_power(DaiPowerKind power)
{
    return lightning_like(power) || (power == PwrK_CAVEIN) || (use_function(power) == MAGIC_USE_CAVE_IN);
}

static TbBool rule_uses_fact(const struct KeeperAssistRule *r, enum KeeperAssistFact fact)
{
    for (int i = 0; i < r->cond_count; i++) {
        if (r->cond[i].fact == fact) return true;
    }
    return false;
}

static TbBool conds_hold(const struct KeeperAssistRule *r, const int64_t *dungeon_facts,
    const struct KeeperAssistCandidate *c, int64_t allies_near)
{
    for (int i = 0; i < r->cond_count; i++) {
        const struct KeeperAssistCond *cond = &r->cond[i];
        int64_t value;
        if (keeper_assist_fact_needs_target((enum KeeperAssistFact)cond->fact)) {
            if (c == NULL) return false;
            value = (cond->fact == KAF_AlliesNear) ? allies_near : c->facts[cond->fact];
        } else if ((cond->fact > KAF_None) && (cond->fact < KAF_Count)) {
            value = dungeon_facts[cond->fact];
        } else {
            return false;
        }
        if (!keeper_assist_compare((enum KeeperAssistOp)cond->op, value, cond->value)) return false;
    }
    return true;
}

static enum KeeperAssistPick effective_pick(const struct KeeperAssistRule *r)
{
    if (r->pick != KAP_Default) return (enum KeeperAssistPick)r->pick;
    return (r->target == KAT_EnemyInFight) ? KAP_Strongest : KAP_LowestHealth;
}

/** Whether `a` is a better pick than `b`; a tie goes to the lower thing index, so the choice never depends on order. */
static TbBool better_pick(enum KeeperAssistPick pick, const struct KeeperAssistCandidate *a, const struct KeeperAssistCandidate *b)
{
    int64_t ka = 0, kb = 0, ka2 = 0, kb2 = 0;
    switch (pick) {
    case KAP_LowestHealth: ka = -a->facts[KAF_HealthPct]; kb = -b->facts[KAF_HealthPct]; break;
    case KAP_Strongest:    ka = a->facts[KAF_Force]; kb = b->facts[KAF_Force]; break;
    case KAP_HighestLevel: ka = a->facts[KAF_Level]; kb = b->facts[KAF_Level]; ka2 = a->facts[KAF_Force]; kb2 = b->facts[KAF_Force]; break;
    case KAP_MostValuable: ka = a->score; kb = b->score; break;
    default: break;
    }
    if (ka != kb) return ka > kb;
    if (ka2 != kb2) return ka2 > kb2;
    return a->thing < b->thing;
}

static TbBool recently_targeted(const struct KeeperAssistRuleRun *run, DaiThingIndex thing, int64_t cooldown, DaiTurn now)
{
    if (cooldown <= 0) return false;
    for (int i = 0; i < KEEPER_ASSIST_RECENT_TARGETS; i++) {
        if ((run->recent_target[i] == thing) && (run->recent_target_turn[i] != 0)
         && ((int64_t)(now - run->recent_target_turn[i]) < cooldown)) {
            return true;
        }
    }
    return false;
}

/** Whether the rule would act on this candidate, money aside. */
static TbBool candidate_ok(const struct KeeperAssistRule *r, const struct KeeperAssistRuleRun *run,
    const struct KeeperAssistWorld *world, const int64_t *dungeon_facts, const struct KeeperAssistCandidate *c, DaiTurn now)
{
    if ((c->sets & KEEPER_ASSIST_SET(r->target)) == 0) return false;
    if (recently_targeted(run, c->thing, r->target_cooldown, now)) return false;
    int64_t allies_near = 0;
    if (area_power(r->power) || rule_uses_fact(r, KAF_AlliesNear)) {
        allies_near = world->allies_near(world->ctx, r, c);
    }
    if (area_power(r->power) && (allies_near > r->max_allies_hit)) return false;
    return conds_hold(r, dungeon_facts, c, allies_near);
}

/** The price if the keeper may spend it on this rule now, else -1 (and why, in the rule's last_refusal). */
static int64_t affordable_price(struct KeeperAssistState *st, int slot, const struct KeeperAssistWorld *world,
    const int64_t *dungeon_facts)
{
    const struct KeeperAssistRule *r = &st->live[slot];
    struct KeeperAssistRuleRun *run = &st->run[slot];
    if (!world->power_available(world->ctx, r->power)) { run->last_refusal = KAR_NotAvailable; return -1; }
    const int64_t price = world->price(world->ctx, r->power, r->level);
    const int64_t gold = dungeon_facts[KAF_Gold];
    if (price > gold) { run->last_refusal = KAR_CannotAfford; return -1; }
    if (gold - price < st->globals.reserve) { run->last_refusal = KAR_Reserve; return -1; }
    if ((r->budget > 0) && (run->spent_this_payday + price > r->budget)) { run->last_refusal = KAR_Budget; return -1; }
    if ((st->globals.max_spend > 0) && (st->spent_this_payday + price > st->globals.max_spend)) { run->last_refusal = KAR_MaxSpend; return -1; }
    run->last_refusal = KAR_None;
    return price;
}

struct KeeperAssistDecision keeper_assist_decide(struct KeeperAssistState *st, const struct KeeperAssistWorld *world,
    const int64_t *dungeon_facts, const struct KeeperAssistCandidate *cands, int ncands, DaiTurn now,
    TbBool casts_allowed)
{
    struct KeeperAssistDecision dec = { -1, -1, 0 };
    const TbBool gap_passed = (st->last_cast_turn == 0) || ((int64_t)(now - st->last_cast_turn) >= st->globals.min_gap);
    for (int slot = 0; slot < KEEPER_ASSIST_RULES; slot++) {
        const struct KeeperAssistRule *r = &st->live[slot];
        struct KeeperAssistRuleRun *run = &st->run[slot];
        if (!r->used) continue;
        if (r->action == KAA_PowerOff) {
            const int64_t active = (r->power == PwrK_CALL2ARMS) ? dungeon_facts[KAF_CtaActive]
                                 : (r->power == PwrK_OBEY) ? dungeon_facts[KAF_ObeyActive] : 0;
            if (!active || !conds_hold(r, dungeon_facts, NULL, 0)) continue;
            dec.slot = slot;
            return dec;
        }
        if ((r->action != KAA_Cast) || !casts_allowed || !gap_passed) continue;
        if ((run->last_cast_turn != 0) && ((int64_t)(now - run->last_cast_turn) < r->cooldown)) continue;
        const enum KeeperAssistPick pick = effective_pick(r);
        int best = -1;
        for (int ci = 0; ci < ncands; ci++) {
            const struct KeeperAssistCandidate *c = &cands[ci];
            if ((best >= 0) && !better_pick(pick, c, &cands[best])) continue;
            if (!candidate_ok(r, run, world, dungeon_facts, c, now)) continue;
            if (!world->can_target(world->ctx, r, c)) continue;
            best = ci;
        }
        if (best < 0) continue;
        const int64_t price = affordable_price(st, slot, world, dungeon_facts);
        if (price < 0) continue;
        dec.slot = slot;
        dec.cand = best;
        dec.price = price;
        return dec;
    }
    return dec;
}

void keeper_assist_use_rules(struct KeeperAssistState *st, const struct KeeperFightRules *rules)
{
    if (rules->globals != NULL) st->globals = *rules->globals;
    for (int s = 0; s < KEEPER_ASSIST_RULES; s++) {
        struct KeeperAssistRule want;
        memset(&want, 0, sizeof(want));
        if ((rules->rules != NULL) && (s < rules->nrules)) want = rules->rules[s];
        if (memcmp(&want, &st->live[s], sizeof(want)) == 0) continue;
        st->live[s] = want;
        memset(&st->run[s], 0, sizeof(st->run[s]));
        if (st->pending.used && (st->pending.slot == s)) memset(&st->pending, 0, sizeof(st->pending));
    }
}

static void log_action(struct KeeperAssistState *st, int slot, DaiThingIndex target, int64_t cost, DaiTurn now)
{
    struct KeeperAssistCast *e = &st->log[st->log_total % KEEPER_ASSIST_LOG];
    memset(e, 0, sizeof(*e));
    e->turn = now;
    e->target = target;
    e->cost = cost;
    e->power = st->live[slot].power;
    e->slot = (uint8_t)slot;
    e->level = st->live[slot].level;
    st->log_total++;
}

void keeper_assist_note_cast(struct KeeperAssistState *st, int slot, DaiThingIndex target, int64_t cost, DaiTurn now)
{
    struct KeeperAssistRuleRun *run = &st->run[slot];
    run->last_cast_turn = now;
    run->fired_total++;
    run->spent_total += cost;
    run->spent_this_payday += cost;
    run->recent_target[run->recent_next] = target;
    run->recent_target_turn[run->recent_next] = now;
    run->recent_next = (uint8_t)((run->recent_next + 1) % KEEPER_ASSIST_RECENT_TARGETS);
    run->last_refusal = KAR_None;
    st->last_cast_turn = now;
    st->spent_this_payday += cost;
    log_action(st, slot, target, cost, now);
}

TbBool keeper_assist_conds_hold(const struct KeeperAssistRule *rule, const int64_t *dungeon_facts, const struct KeeperAssistCandidate *cand)
{
    return conds_hold(rule, dungeon_facts, cand, 0);
}

#ifdef __cplusplus
}
#endif
