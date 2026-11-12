/** @file DaiObjectives.h
 *  What the keeper's fights are for (02 section 4.8a, 06 section 5.2): Defence (and from Phase 5 Offence, from Phase
 *  6 Custody) post objectives; Tactics, which alone acts in fights, serves them and writes each one's report. State,
 *  not a manager's: managers never call each other. An objective is keyed by (owner, kind, key): posting again
 *  updates it and keeps its id and report. Only its owner changes or drops it; only Tactics writes reports. */
#ifndef DAIMON_DAIOBJECTIVES_H
#define DAIMON_DAIOBJECTIVES_H

#include <cstdint>

#include "globals.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiFix.h"
#include "daimon/DaiLimits.h"
#include "daimon/DaiPool.h"
#include "daimon/DaiTypes.h"

enum class DaiObjectiveKind : uint8_t {
    HoldHeart, HoldArea, ProtectImps, Intercept, Escort,    // Phase 4 (06 section 5.2)
    Assault, Raid, Scout, Withdraw,                         // Phase 5 (07 section 4)
    SpendTrap,                                              // P5.13: one creature (target) stands on a hazard's trap
    Count
};
/** A raid's kind (07, 08). */
enum class DaiObjectiveSub : uint8_t { None, PortalSteal, ImpHunt, Harass, Jailbreak, Count };
/** How it goes, as Tactics last saw it. */
enum class DaiObjectiveState : uint8_t { Idle, Engaged, Winning, Holding, Losing, Lost, Done, Count };

struct DaiObjectiveReport {
    DaiObjectiveState state = DaiObjectiveState::Idle;
    int64_t own_force = 0, enemy_force = 0;     /**< in its engagements, the enemy's as seen */
    int64_t enemy_levels = 0;                   /**< the enemy creatures' levels there, imps apart (P5.12) */
    int64_t losses = 0, kills = 0;              /**< creatures lost and killed in them since it was posted */
    DaiFix ratio_trend = 0;                     /**< the change of enemy/own force since the last report */
    int64_t gold_spent = 0;
    int64_t target_health = -1;                 /**< the heart's or the target room's health %, -1 none */
    DaiTurn updated = 0;
};

struct DaiObjective {
    uint32_t id = 0;
    DaiDomain owner = DaiDomain::Count;
    DaiObjectiveKind kind = DaiObjectiveKind::HoldArea;
    DaiObjectiveSub sub = DaiObjectiveSub::None;
    uint64_t key = 0;                   /**< the owner's own name for it: posting again with it updates it */
    DaiRect area;                       /**< slabs */
    DaiThingRef target;                 /**< a thing it is about (Intercept, Assault, Raid), or none */
    PlayerNumber rival = -1;
    DaiSlabXY staging{ -1, -1 };
    DaiFix stakes = DAI_FIX_ONE;        /**< the owner's weight; Tactics multiplies it by what is at risk */
    int64_t gold_per_kturn = 0;         /**< what casts and upkeep for it may spend a 1000 turns */
    DaiFix home_guard_share = 0;        /**< Assault, Raid: the share of force kept home while it runs */
    DaiTurn posted = 0;
    DaiTurn until = 0;                 /**< 0: until dropped */
    DaiObjectiveReport report;
};

class DaiObjectives {
public:
    /** Inserts or updates the owner's objective of this kind and key; its id, 0 if the table is full. */
    uint32_t Post(DaiDomain owner, const DaiObjective &ob, DaiTurn now);
    /** The owner's own objective goes (another's is left alone). */
    void Drop(DaiDomain owner, uint32_t id);
    const DaiObjective *Find(uint32_t id) const { return m_objectives.Find(id); }
    const DaiObjective *Find(DaiDomain owner, DaiObjectiveKind kind, uint64_t key) const;
    const DaiObjectiveReport *Report(uint32_t id) const;
    /** Tactics' alone: another domain's write is refused (false). */
    bool WriteReport(DaiDomain writer, uint32_t id, const DaiObjectiveReport &r);
    void Expire(DaiTurn now);
    void Clear() { m_objectives.Clear(); }
    int64_t Count() const { return m_objectives.Count(); }
    template <typename F> void ForEach(F f) const { m_objectives.ForEach(f); }
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    DaiPool<DaiObjective, DAI_MAX_OBJECTIVES> m_objectives;
};

const char *DaiObjectiveKindName(DaiObjectiveKind k);
const char *DaiObjectiveStateName(DaiObjectiveState s);
const char *DaiObjectiveSubName(DaiObjectiveSub s);

#endif
