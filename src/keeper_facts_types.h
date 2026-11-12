/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file keeper_facts_types.h
 *     The facts the keeper-facts tracker reports (keeper_facts.h): types only, so code that consumes facts without
 *     reading the game (Daimon's inbox, docs/plans/ai-plans/v2/02-library-architecture.md 3.2) may include it.
 */
/******************************************************************************/
#ifndef DK_KEEPER_FACTS_TYPES_H
#define DK_KEEPER_FACTS_TYPES_H

#include "globals.h"
#include "daimon_base_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** What happened, as a keeper sitting in that seat would notice it. Reported in this order within one update. */
enum KeeperFactKind {
    KF_None = 0,
    KF_PaydayQuarter,   /**< sub: the quarter of the pay-day gap now (0: pay day) */
    KF_Event,           /**< sub: the new or changed event's kind (EvKind_*); x, y: where */
    KF_Objective,       /**< the level's objective text changed (keeper_facts_objective()) */
    KF_Custody,         /**< sub: enum KeeperFactCustody; args[0]: the prisoner */
    KF_Sacrifice,       /**< a temple recipe fired; args[0]: its index in the rules' recipe table */
    KF_VampireProgress, /**< a vampire rose from the graveyard (the rotting-bodies count fell) */
    KF_WallCue,         /**< open ground behind one of the keeper's walls that was not there before */
    KF_AssistRefusal,   /**< sub: enum KeeperFactAssist; args[0]: the rule slot (-1: the gold guard) */
    KF_SightOpen,       /**< Sight of Evil stopped widening: look now, it closes again */
    KF_Loss,            /**< sub: enum KeeperFactLoss; args[0]: room kind (0: floor or wall), [1]: who holds it now
                             (-1: nobody), [2]: slabs this scan; x, y: the first slab's centre subtile */
    KF_LossScan,        /**< after a scan's KF_Loss facts: sub = bits 1 << KFL_* of the kinds lost */
    KF_Intruder,        /**< an enemy creature on the keeper's ground not seen there lately; x, y: the first such */
    KF_TrapFired,       /**< one of its traps went off; args[0]: trap, [1]: model, [2]: owner; x, y */
    KF_EnemyTrap,       /**< an enemy trap it sees went off (or gave itself away); as KF_TrapFired */
    KF_Victory,         /**< sub: the keeper's victory_state now (changed) */
    KF_NewKind,         /**< sub: a creature model the keeper owns now and did not before */
    KF_Count,           /**< how many kinds there are (Daimon's inbox keeps an expiry for each) */
};

enum KeeperFactCustody {
    KFC_New = 0, KFC_Hungry, KFC_TortureHurt, KFC_Interrogated, KFC_Converted, KFC_Gone,
    KFC_COUNT,          /**< not a kind: the reason tables are this long (api_seat_decision.c asserts it) */
};

enum KeeperFactAssist {
    KFA_Budget = 0,     /**< a rule's budget, or the set's cap for the pay day */
    KFA_Reserve,        /**< a cast held back only by the reserve */
    KFA_GoldGuard,      /**< the gold guard switched a lasting power off */
};

enum KeeperFactLoss {
    KFL_RoomCaptured = 0, KFL_RoomDestroyed, KFL_GroundCaptured, KFL_GroundLost,
    KFL_COUNT,          /**< not a kind: the reason tables are this long (api_seat_decision.c asserts it) */
};

struct KeeperFact {
    enum KeeperFactKind kind;
    int64_t sub;
    DaiSubtlCoord x, y;
    int64_t args[4];
    DaiTurn turn;
};

#ifdef __cplusplus
}
#endif

#endif
