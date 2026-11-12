/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file keeper_assist_rules.h
 *     The combat assistant's rules: a keeper's standing orders for fights (docs/newFeatures/llm/02-rule-language.md).
 * @par Purpose:
 *     The rule as it is stored (in kfx_sim_state, per player) and its fixed vocabulary -- actions, targets, pick
 *     orders, facts, comparisons -- with the names an agent or a profile file uses for them, and the one validation
 *     every way in (the API, a profile, a packet) goes through. Plain data: the evaluation is kfx_ai's
 *     (keeper_assist.c), which this layer must not know about.
 */
/******************************************************************************/
#ifndef DK_KEEPER_ASSIST_RULES_H
#define DK_KEEPER_ASSIST_RULES_H

#include "globals.h"
#include "daimon_base_types.h"
#include <json-dom.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Rules per keeper, and conditions per rule (all must hold; OR is two rules). */
#define KEEPER_ASSIST_RULES 16
/** Complete rule sets a keeper may keep ready to make live in one packet (keeper_assist_state.h's banks). */
#define KEEPER_ASSIST_BANKS 3
#define KEEPER_ASSIST_CONDS 4
/** Highest overcharge level a rule may ask for (the engine's levels are 0-8). */
#define KEEPER_ASSIST_MAX_LEVEL 8
/** Hand rules (docs/plans/llm-bridge-realtime.md, Phase 5): most creatures one firing moves, and the defaults. */
#define KEEPER_ASSIST_HAND_MAX 8
#define KEEPER_ASSIST_DEFAULT_HAND_RADIUS 6       /**< slabs */
#define KEEPER_ASSIST_DEFAULT_MIN_MATCH 50        /**< reinforce: % of the strongest enemy's force each fighter sent must have */
#define KEEPER_ASSIST_DEFAULT_HAND_COOLDOWN 60
#define KEEPER_ASSIST_DEFAULT_WOUNDED_PCT 35      /**< lift_wounded without a health condition */
#define KEEPER_ASSIST_DEFAULT_REINFORCE_RATIO 80  /**< reinforce without a force_ratio condition: losing or matched */

#pragma pack(1)

enum KeeperAssistAction {
    KAA_None = 0,
    KAA_Cast,       /**< cast `power` on a target picked from `target` */
    KAA_PowerOff,   /**< switch a lasting power off (Call to Arms, Must Obey); no target */
    /* Hand rules: carried out by an External seat's own hand (kfx_net's seat reflexes), never by the simulation, and only
     * for an agent: a human player's profile or Options screen cannot hold them. */
    KAA_EvacuateDiggers, /**< lift the keeper's diggers near an enemy to `dest` (its safe spot) */
    KAA_LiftWounded,     /**< lift its wounded fighters out of a fight (to `dest`, else a lair) when no heal rule can heal them */
    KAA_Reinforce,       /**< drop its healthy fighters, each a match for the enemy, beside a fight they can turn */
    KAA_Count,
};

enum KeeperAssistTarget {
    KAT_None = 0,
    KAT_AllyInFight,  /**< the keeper's own creatures in a battle */
    KAT_AllyAny,      /**< all the keeper's own creatures */
    KAT_EnemyInFight, /**< enemy creatures in a battle with one of the keeper's, that the keeper can see */
    KAT_Count,
};

enum KeeperAssistPick {
    KAP_Default = 0,  /**< lowest health for allies, strongest for enemies */
    KAP_LowestHealth,
    KAP_Strongest,
    KAP_HighestLevel,
    KAP_MostValuable,
    KAP_Count,
};

enum KeeperAssistOp {
    KAO_None = 0,
    KAO_Lt, KAO_Le, KAO_Gt, KAO_Ge, KAO_Eq, KAO_Ne,
    KAO_Count,
};

/** Facts a condition compares. Those before KAF_FirstDungeonFact describe the candidate target and need a rule with a
 *  target; the rest describe the keeper's dungeon. Stored as numbers in saves: append, never renumber. */
enum KeeperAssistFact {
    KAF_None = 0,
    KAF_HealthPct,        /**< health * 100 / max health */
    KAF_Health,
    KAF_Level,            /**< 1-based */
    KAF_Force,            /**< the creature's score at its level, scaled by the health it has left */
    KAF_IsDigger,
    KAF_OnOwnGround,      /**< standing on the keeper's (or a mutual ally's) ground */
    KAF_Fleeing,
    KAF_Kind,             /**< creature model id: == and != only */
    KAF_BattleAllyForce,  /**< summed force of the keeper's creatures in the target's battle */
    KAF_BattleEnemyForce, /**< summed force of the visible enemies in it */
    KAF_ForceRatio,       /**< enemy force * 100 / ally force: over 100, losing on paper */
    KAF_BattleSize,
    KAF_AlliesNear,       /**< the keeper's and its allies' creatures the power would also hit (Lightning, Cave-in) */
    KAF_FirstDungeonFact,
    KAF_Gold = KAF_FirstDungeonFact,
    KAF_GoldAfterWages,   /**< gold minus the next pay day's wage bill */
    KAF_PaydayTurns,
    KAF_Fights,
    KAF_Fighters,         /**< the keeper's creatures that are not diggers */
    KAF_CtaActive,
    KAF_CtaPaying,        /**< Call to Arms is on and being charged for (its banner is off the keeper's ground) */
    KAF_ObeyActive,
    KAF_HeartHealthPct,
    KAF_Count,
};

struct KeeperAssistCond {
    uint8_t fact;   /**< enum KeeperAssistFact */
    uint8_t op;     /**< enum KeeperAssistOp */
    int32_t value;
};

/** One standing order. `used` = 0 is an empty slot. */
struct KeeperAssistRule {
    uint8_t used;
    uint8_t action;          /**< enum KeeperAssistAction */
    uint16_t power;          /**< DaiPowerKind */
    uint8_t level;           /**< overcharge level, 0-8 */
    uint8_t target;          /**< enum KeeperAssistTarget (KAT_None for power_off) */
    uint8_t pick;            /**< enum KeeperAssistPick */
    uint8_t cond_count;
    uint8_t max_allies_hit;  /**< Lightning, Cave-in: most own/allied creatures the cast may also hit */
    uint16_t cooldown;       /**< turns between two casts by this rule */
    uint16_t target_cooldown;/**< turns before this rule casts on the same creature again */
    int32_t budget;          /**< gold this rule may spend per pay period; 0 = no limit */
    struct KeeperAssistCond cond[KEEPER_ASSIST_CONDS];
    uint16_t dest_x, dest_y; /**< hand rules: the subtile to drop on (0, 0: none -- a lair for the wounded) */
    uint8_t count;           /**< hand rules: most creatures one firing moves (1..KEEPER_ASSIST_HAND_MAX) */
    uint8_t radius;          /**< hand rules: slabs from an enemy (evacuate) or a fight (reinforce) that count as near it */
    uint8_t min_match;       /**< reinforce: % of the strongest enemy's force each fighter sent must have (0..200) */
    uint8_t reserved_hand;
};

/** The keeper-wide settings. */
struct KeeperAssistGlobals {
    uint8_t enabled;
    uint8_t interval;        /**< turns between two evaluations, 1-20 */
    uint8_t charge_time;     /**< wait 4 turns per overcharge level before a cast, as a human holding the button must */
    uint8_t reserved;
    uint16_t min_gap;        /**< turns between any two casts, 1-200 */
    int32_t reserve;         /**< no cast may leave less gold than this */
    int32_t max_spend;       /**< gold all rules together may spend per pay period; 0 = no limit */
};

#pragma pack()

#define KEEPER_ASSIST_DEFAULT_INTERVAL 2
#define KEEPER_ASSIST_DEFAULT_MIN_GAP 4
#define KEEPER_ASSIST_DEFAULT_COOLDOWN 20
#define KEEPER_ASSIST_DEFAULT_ALLY_TARGET_COOLDOWN 100
/** Turns a human holds the button per overcharge level (process_dungeon_control_packet_spell_overcharge). */
#define KEEPER_ASSIST_TURNS_PER_LEVEL 4

/** Whether a rule action is a hand rule (an External seat's reflex, not the simulation's). */
TbBool keeper_assist_action_is_hand(uint8_t action);
const char *keeper_assist_hand_name(uint8_t action);

/** Default globals: disabled, interval 2, charge time on, min gap 4, no reserve, no cap. */
void keeper_assist_default_globals(struct KeeperAssistGlobals *g);
/** A fresh rule with the defaults for its action and target (cooldowns, pick); `used` set. */
void keeper_assist_default_rule(struct KeeperAssistRule *r, enum KeeperAssistAction action, DaiPowerKind power,
    enum KeeperAssistTarget target);

/** NULL when the rule can be stored, else an error code (docs/newFeatures/llm/02 section 6). A power that is not
 *  researched yet is not an error: the rule waits for it. */
const char *keeper_assist_rule_validate(const struct KeeperAssistRule *r);
/** NULL when the globals are in range, else an error code. */
const char *keeper_assist_globals_validate(const struct KeeperAssistGlobals *g);

/** Which target a power is cast at: an ally (own creature), an enemy, or none (a power that can only be switched off).
 *  KAT_None with *power_off false = the assistant does not drive this power. */
enum KeeperAssistTarget keeper_assist_power_target_class(DaiPowerKind power, TbBool *power_off);
/** Whether a fact describes the target (and so needs a rule with one). */
TbBool keeper_assist_fact_needs_target(enum KeeperAssistFact fact);
/** Compares `lhs` with `rhs` by `op` (false for an unknown op). */
TbBool keeper_assist_compare(enum KeeperAssistOp op, int64_t lhs, int64_t rhs);

/** Names as an agent or a profile writes them: "health_pct", "ally_in_fight", "lowest_health", "<", ... A name
 *  lookup returns 0 (the None/Default entry) when it does not know the name. */
const char *keeper_assist_fact_name(enum KeeperAssistFact fact);
enum KeeperAssistFact keeper_assist_fact_by_name(const char *name);
const char *keeper_assist_target_name(enum KeeperAssistTarget target);
enum KeeperAssistTarget keeper_assist_target_by_name(const char *name);
const char *keeper_assist_pick_name(enum KeeperAssistPick pick);
enum KeeperAssistPick keeper_assist_pick_by_name(const char *name);
const char *keeper_assist_op_name(enum KeeperAssistOp op);
enum KeeperAssistOp keeper_assist_op_by_name(const char *name);

/** Reads a rule set as an agent (set_assist's JSON) or a profile / preset (TOML) writes it -- docs/newFeatures/llm/02
 *  section 1 -- into the whole set and the globals. Globals left out keep `current`'s values (the defaults if the keeper
 *  never had any); "rules" left out keeps `current_rules`, an empty list clears them. NULL on success, else an error
 *  code, with *bad_rule the index of the rule at fault (-1 for the globals). */
const char *keeper_assist_parse_set(VALUE *root, const struct KeeperAssistRule *current_rules, const struct KeeperAssistGlobals *current,
    struct KeeperAssistRule *rules, struct KeeperAssistGlobals *globals, int *bad_rule);
/** Loads the presets (fxdata/combat_assist_presets.toml, or `path` when given) into `out`: a dict of preset name ->
 *  {description, the set_assist fields, rules}. False (and `out` an empty dict) when the file is missing or broken. */
TbBool keeper_assist_load_presets(const char *path, VALUE *out);
/** One preset of a loaded `presets` dict as a whole rule set, read like set_assist from defaults. NULL on success, else
 *  "UNKNOWN_PRESET" or the rule's error (with *bad_rule). */
const char *keeper_assist_preset_set(VALUE *presets, const char *name, struct KeeperAssistRule *rules,
    struct KeeperAssistGlobals *globals, int *bad_rule);

/** Which parts of a set_assist request were given (keeper_assist_given): each setting, and the rules. A change sent to
 *  run later (queued behind other verbs) takes what it left out from the set as it is when it runs, not as it was when
 *  it was sent -- else a change of settings alone, queued behind a change of rules, put the old rules back. */
enum KeeperAssistGiven {
    KAGV_Enabled = 0x01,
    KAGV_Interval = 0x02,
    KAGV_ChargeTime = 0x04,
    KAGV_MinGap = 0x08,
    KAGV_Reserve = 0x10,
    KAGV_MaxSpend = 0x20,
    KAGV_Rules = 0x40,
    KAGV_Known = 0x80,      /**< set: the other bits say what was given; clear (0): the whole set was */
};
/** The KAGV_* bits of what `root` (set_assist's JSON) gives, KAGV_Known included. */
uint8_t keeper_assist_given(VALUE *root);
/** The set a change makes of `base` (the live set, or the bank's, as it is when the change runs): what `given` says the
 *  change gave from `want_rules` / `want`, the rest from `base_rules` / `base` (the defaults for settings a keeper never
 *  had). `given` 0: the change's whole set. */
void keeper_assist_overlay(uint8_t given, const struct KeeperAssistRule *want_rules, const struct KeeperAssistGlobals *want,
    const struct KeeperAssistRule *base_rules, const struct KeeperAssistGlobals *base,
    struct KeeperAssistRule *rules, struct KeeperAssistGlobals *globals);

/** One rule (a dict in those words) into `r`; NULL or the error code. */
const char *keeper_assist_parse_rule(VALUE *rv, struct KeeperAssistRule *r);
/** The settings found in `root` (enabled, interval, charge_time, min_gap, reserve, max_spend) over `g`'s values. */
const char *keeper_assist_parse_globals(VALUE *root, struct KeeperAssistGlobals *g);
/** Adds a rule's definition to `e` in the same words: power / power_off, level, target, pick, cooldowns, budget,
 *  max_allies_hit, when. */
void keeper_assist_rule_to_value(VALUE *e, const struct KeeperAssistRule *r);

#ifdef __cplusplus
}
#endif
#endif
