/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file keeper_assist_rules.c
 *     The combat assistant's rule vocabulary and validation (keeper_assist_rules.h): the fork's keeper_assist_rules.c up to its parsing section, as Daimon's port tools copy it (scripts/daimon_port/sync.py in the fork); the fork's state struct is upstream's game. The parsing (the rules as TOML/JSON) is the port's own.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "keeper_assist_rules.h"
#include <string.h>
#include <strings.h>
#include "config_magic.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char *const fact_names[KAF_Count] = {
    "", "health_pct", "health", "level", "force", "is_digger", "on_own_ground", "fleeing", "kind",
    "battle_ally_force", "battle_enemy_force", "force_ratio", "battle_size", "allies_near",
    "gold", "gold_after_wages", "payday_turns", "fights", "fighters", "cta_active", "cta_paying", "obey_active",
    "heart_health_pct",
};
static const char *const target_names[KAT_Count] = { "", "ally_in_fight", "ally_any", "enemy_in_fight" };
static const char *const pick_names[KAP_Count] = { "", "lowest_health", "strongest", "highest_level", "most_valuable" };
static const char *const op_names[KAO_Count] = { "", "<", "<=", ">", ">=", "==", "!=" };

static int64_t by_name(const char *const *names, int64_t count, const char *name)
{
    if ((name == NULL) || (name[0] == 0)) return 0;
    for (int64_t i = 1; i < count; i++) {
        if (strcasecmp(names[i], name) == 0) return i;
    }
    return 0;
}

const char *keeper_assist_fact_name(enum KeeperAssistFact fact) { return ((fact > 0) && (fact < KAF_Count)) ? fact_names[fact] : ""; }
enum KeeperAssistFact keeper_assist_fact_by_name(const char *name) { return (enum KeeperAssistFact)by_name(fact_names, KAF_Count, name); }
const char *keeper_assist_target_name(enum KeeperAssistTarget t) { return ((t > 0) && (t < KAT_Count)) ? target_names[t] : ""; }
enum KeeperAssistTarget keeper_assist_target_by_name(const char *name) { return (enum KeeperAssistTarget)by_name(target_names, KAT_Count, name); }
const char *keeper_assist_pick_name(enum KeeperAssistPick p) { return ((p > 0) && (p < KAP_Count)) ? pick_names[p] : ""; }
enum KeeperAssistPick keeper_assist_pick_by_name(const char *name) { return (enum KeeperAssistPick)by_name(pick_names, KAP_Count, name); }
const char *keeper_assist_op_name(enum KeeperAssistOp op) { return ((op > 0) && (op < KAO_Count)) ? op_names[op] : ""; }
enum KeeperAssistOp keeper_assist_op_by_name(const char *name) { return (enum KeeperAssistOp)by_name(op_names, KAO_Count, name); }

TbBool keeper_assist_fact_needs_target(enum KeeperAssistFact fact)
{
    return (fact > KAF_None) && (fact < KAF_FirstDungeonFact);
}

TbBool keeper_assist_compare(enum KeeperAssistOp op, int64_t lhs, int64_t rhs)
{
    switch (op) {
    case KAO_Lt: return lhs < rhs;
    case KAO_Le: return lhs <= rhs;
    case KAO_Gt: return lhs > rhs;
    case KAO_Ge: return lhs >= rhs;
    case KAO_Eq: return lhs == rhs;
    case KAO_Ne: return lhs != rhs;
    default: return false;
    }
}

enum KeeperAssistTarget keeper_assist_power_target_class(DaiPowerKind power, TbBool *power_off)
{
    *power_off = false;
    switch (power) {
    case PwrK_HEALCRTR:
    case PwrK_SPEEDCRTR:
    case PwrK_PROTECT:
    case PwrK_CONCEAL:
    case PwrK_REBOUND:
    case PwrK_FLIGHT:
    case PwrK_VISION:
        return KAT_AllyInFight;
    case PwrK_CHICKEN:
    case PwrK_DISEASE:
    case PwrK_LIGHTNING:
    case PwrK_CAVEIN:
    case PwrK_SLOW:
    case PwrK_FREEZE:
        return KAT_EnemyInFight;
    case PwrK_CALL2ARMS:
    case PwrK_OBEY:
        *power_off = true;
        return KAT_None;
    default: {
        // A map pack's own power (Phase 7): a spell (UseFunction magic_use_power_apply_spell, 2) or a lightning-like
        // strike (magic_use_power_lightning, 6), by what it is cast on -- enemy creatures (a strike: the ground under
        // them), else own creatures. The hand, slap, possession and the rest stay unsupported.
        const struct PowerConfigStats *powerst = get_power_model_stats(power);
        if ((power <= 0) || (powerst == NULL)) return KAT_None;
        // UseFunction's numbers (config_magic.c's magic_use_func_commands, 14 P6).
        enum { MUF_ApplySpell = 2, MUF_Lightning = 6 };
        const int64_t f = powerst->magic_use_func_idx;
        if (f == MUF_Lightning) return KAT_EnemyInFight;
        if (f != MUF_ApplySpell) return KAT_None;
        if ((powerst->can_cast_flags & PwCast_EnemyCrtrs) != 0) return KAT_EnemyInFight;
        if ((powerst->can_cast_flags & PwCast_OwnedCrtrs) != 0) return KAT_AllyInFight;
        return KAT_None;
    }
    }
}

void keeper_assist_default_globals(struct KeeperAssistGlobals *g)
{
    memset(g, 0, sizeof(*g));
    g->interval = KEEPER_ASSIST_DEFAULT_INTERVAL;
    g->charge_time = 1;
    g->min_gap = KEEPER_ASSIST_DEFAULT_MIN_GAP;
}

void keeper_assist_default_rule(struct KeeperAssistRule *r, enum KeeperAssistAction action, DaiPowerKind power,
    enum KeeperAssistTarget target)
{
    memset(r, 0, sizeof(*r));
    r->used = 1;
    r->action = (uint8_t)action;
    r->power = (uint16_t)power;
    r->target = (uint8_t)target;
    r->pick = KAP_Default;
    r->cooldown = (action == KAA_Cast) ? KEEPER_ASSIST_DEFAULT_COOLDOWN : 0;
    r->target_cooldown = ((target == KAT_AllyInFight) || (target == KAT_AllyAny)) ? KEEPER_ASSIST_DEFAULT_ALLY_TARGET_COOLDOWN : 0;
}

static const char *const hand_names[] = { "evacuate_diggers", "lift_wounded", "reinforce" };

TbBool keeper_assist_action_is_hand(uint8_t action)
{
    return (action >= KAA_EvacuateDiggers) && (action < KAA_Count);
}

const char *keeper_assist_hand_name(uint8_t action)
{
    return keeper_assist_action_is_hand(action) ? hand_names[action - KAA_EvacuateDiggers] : "";
}

/** The target set a hand rule's candidates come from (its conditions' target facts are theirs). */
static enum KeeperAssistTarget hand_target(uint8_t action)
{
    return (action == KAA_EvacuateDiggers) ? KAT_AllyAny : KAT_AllyInFight;
}

static const char *validate_conds(const struct KeeperAssistRule *r)
{
    if (r->cond_count > KEEPER_ASSIST_CONDS) return "TOO_MANY_CONDITIONS";
    for (int i = 0; i < r->cond_count; i++) {
        const struct KeeperAssistCond *c = &r->cond[i];
        if ((c->fact <= KAF_None) || (c->fact >= KAF_Count)) return "UNKNOWN_FACT";
        if ((c->op <= KAO_None) || (c->op >= KAO_Count)) return "BAD_OP";
        if (keeper_assist_fact_needs_target((enum KeeperAssistFact)c->fact) && (r->target == KAT_None)) return "FACT_NEEDS_TARGET";
        if ((c->fact == KAF_Kind) && (c->op != KAO_Eq) && (c->op != KAO_Ne)) return "BAD_OP";
    }
    return NULL;
}

static const char *validate_hand(const struct KeeperAssistRule *r)
{
    if (r->power != 0) return "BAD_RULE";
    if (r->target != hand_target(r->action)) return "UNKNOWN_TARGET";
    if ((r->count < 1) || (r->count > KEEPER_ASSIST_HAND_MAX)) return "VALUE_OUT_OF_RANGE";
    if ((r->radius < 1) || (r->radius > 15) || (r->min_match > 200) || (r->cooldown > 10000)) return "VALUE_OUT_OF_RANGE";
    if ((r->action == KAA_EvacuateDiggers) && (r->dest_x == 0) && (r->dest_y == 0)) return "MISSING_DESTINATION";
    if ((r->dest_x >= 3 * MAX_TILES_X) || (r->dest_y >= 3 * MAX_TILES_Y)) return "POSITION_OFF_MAP";
    return validate_conds(r);
}

const char *keeper_assist_rule_validate(const struct KeeperAssistRule *r)
{
    if (keeper_assist_action_is_hand(r->action)) return validate_hand(r);
    if ((r->power < 1) || ((int64_t)r->power >= game.conf.magic_conf.power_types_count)) return "UNKNOWN_POWER";
    TbBool power_off = false;
    const enum KeeperAssistTarget cls = keeper_assist_power_target_class(r->power, &power_off);
    if (r->action == KAA_PowerOff) {
        if (!power_off) return "POWER_NOT_SUPPORTED";
        if (r->target != KAT_None) return "BAD_TARGET_FOR_POWER";
    } else if (r->action == KAA_Cast) {
        if (cls == KAT_None) return "POWER_NOT_SUPPORTED";
        if ((r->target <= KAT_None) || (r->target >= KAT_Count)) return "UNKNOWN_TARGET";
        const TbBool ally_target = (r->target == KAT_AllyInFight) || (r->target == KAT_AllyAny);
        if (ally_target != (cls == KAT_AllyInFight)) return "BAD_TARGET_FOR_POWER";
        if (r->level > KEEPER_ASSIST_MAX_LEVEL) return "BAD_LEVEL";
    } else {
        return "UNKNOWN_ACTION";
    }
    if (r->pick >= KAP_Count) return "UNKNOWN_PICK";
    if ((r->budget < 0) || (r->max_allies_hit > 15) || (r->cooldown > 10000) || (r->target_cooldown > 10000)) return "VALUE_OUT_OF_RANGE";
    return validate_conds(r);
}

const char *keeper_assist_globals_validate(const struct KeeperAssistGlobals *g)
{
    if ((g->interval < 1) || (g->interval > 20)) return "VALUE_OUT_OF_RANGE";
    if ((g->min_gap < 1) || (g->min_gap > 200)) return "VALUE_OUT_OF_RANGE";
    if ((g->reserve < 0) || (g->max_spend < 0)) return "VALUE_OUT_OF_RANGE";
    return NULL;
}


#ifdef __cplusplus
}
#endif
