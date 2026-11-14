/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file keeper_assist_state.h
 *     The combat assistant's per-keeper state (docs/newFeatures/llm/01-combat-assistant-design.md).
 * @par Purpose:
 *     A keeper's standing orders and what became of them, kept in kfx_sim_state so a save, a resync and a replay all
 *     have them: the live rules, a staging copy that rule changes are written into before they go live at once,
 *     each rule's counters, a charged cast waiting for its charge time, and the latest casts. Evaluated by kfx_ai's
 *     keeper_assist.c; the rule itself is kfx_config's (keeper_assist_rules.h).
 */
/******************************************************************************/
#ifndef DK_KEEPER_ASSIST_STATE_H
#define DK_KEEPER_ASSIST_STATE_H

#include "globals.h"
#include "daimon_base_types.h"
#include "keeper_assist_rules.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Creatures a rule remembers having cast on (its target_cooldown). */
#define KEEPER_ASSIST_RECENT_TARGETS 4
/** Casts kept for the view. */
#define KEEPER_ASSIST_LOG 16

#pragma pack(1)

/** Why a rule last did not cast although something matched it (for the view; 0 = it did, or nothing matched). */
enum KeeperAssistRefusal {
    KAR_None = 0,
    KAR_NotAvailable,  /**< the power is not researched (or not on this level) */
    KAR_Budget,        /**< the rule's budget for this pay period is spent */
    KAR_MaxSpend,      /**< all rules together have spent the keeper's cap for this pay period */
    KAR_Reserve,       /**< the cast would take the gold under the reserve */
    KAR_CannotAfford,  /**< not enough gold at all */
};

struct KeeperAssistRuleRun {
    DaiTurn last_cast_turn;   /**< 0 = never */
    int64_t spent_this_payday;
    int64_t fired_total;
    int64_t spent_total;
    DaiThingIndex recent_target[KEEPER_ASSIST_RECENT_TARGETS];
    DaiTurn recent_target_turn[KEEPER_ASSIST_RECENT_TARGETS];
    uint8_t recent_next;
    uint8_t last_refusal;      /**< enum KeeperAssistRefusal */
    uint8_t reserved[6];
};

/** A charged cast decided but not yet made: a human would still be holding the button. */
struct KeeperAssistPending {
    uint8_t used;
    uint8_t slot;
    uint8_t reserved[6];
    DaiThingIndex target;
    DaiTurn target_creation_turn;  /**< the slot may have been reused by then */
    DaiTurn due;
};

struct KeeperAssistCast {
    DaiTurn turn;
    DaiThingIndex target;     /**< 0 for power_off */
    int64_t cost;
    uint16_t power;
    uint8_t slot;
    uint8_t level;
    uint8_t reserved[4];
};

/** A complete rule set kept ready (docs/plans/llm-bridge-realtime.md, Phase 3): staged and committed into a bank at a calm
 *  moment (PckA_AssistCommitBank), made live in one packet (PckA_AssistUseBank) when the moment comes -- a switch from a
 *  calm set to a war set in one turn rather than a rewrite of up to KEEPER_ASSIST_MAX_PACKETS packets. */
struct KeeperAssistBank {
    struct KeeperAssistGlobals globals;
    struct KeeperAssistRule rules[KEEPER_ASSIST_RULES];
    uint8_t used;
    uint8_t reserved[7];
};

struct KeeperAssistState {
    struct KeeperAssistGlobals globals;
    struct KeeperAssistRule live[KEEPER_ASSIST_RULES];
    struct KeeperAssistRuleRun run[KEEPER_ASSIST_RULES];
    /** Where rule changes are written (one packet at a time) before a commit makes them live together. */
    struct KeeperAssistGlobals staging_globals;
    struct KeeperAssistRule staging[KEEPER_ASSIST_RULES];
    DaiTurn last_eval_turn;
    DaiTurn last_cast_turn;       /**< 0 = never */
    int64_t spent_this_payday;
    int64_t last_payday_progress;  /**< kfx_config_state.pay_day_progress as last seen: a drop is a pay day */
    struct KeeperAssistPending pending;
    struct KeeperAssistCast log[KEEPER_ASSIST_LOG];
    uint64_t log_total;
    struct KeeperAssistBank bank[KEEPER_ASSIST_BANKS];  /**< bank N (1-based to the agent) is bank[N - 1] */
    uint8_t active_bank;   /**< the bank last made live (1..KEEPER_ASSIST_BANKS); 0 once a plain commit changed the set */
    uint8_t executed_by_daimon;    /**< the keeper's Daimon assistant runs these rules (PckA_AssistDelegate): the runner skips it */
    uint8_t reserved_bank[6];
};

#pragma pack()

#ifdef __cplusplus
}
#endif
#endif
