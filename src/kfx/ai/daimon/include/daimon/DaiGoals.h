/** @file DaiGoals.h
 *  A keeper's goals (02 section 4.5): what the director wants kept or done, with a weight and a status the managers
 *  report. Written by the director only (and, Phase 8, the directive channel); a manager reports through NoteStatus
 *  and nothing else. Phase 0's director keeps one maintenance goal, KeepImps. Requests between managers
 *  (DaiRequests) arrive with their first poster, Phase 1's Economy. */
#ifndef DAIMON_DAIGOALS_H
#define DAIMON_DAIGOALS_H

#include <cstdint>

#include "globals.h"
#include "daimon/DaiFix.h"
#include "daimon/DaiLimits.h"
#include "daimon/DaiPool.h"
#include "daimon/DaiTypes.h"

enum class DaiGoalKind : uint8_t {
    KeepImps, KeepGold, Mine, ExpandTo, Build, Train, Budget, Defend, Attack, Forbid, Custody, Research, Manufacture, Count
};
/** Script: a level's DAIMON_BUILD / DAIMON_ATTACK / DAIMON_DIG (Phase 7, 09 section 13.2a): outranks the director's
 *  goals as a directive does, never dropped by it. */
enum class DaiGoalSource : uint8_t { Director, Maintenance, Request, Directive, Script };
/** Directive: an External seat's agent's (P8.4, 10 section 4), through its lieutenant: outranks a script goal and the
 *  director's (the agent is the keeper's owner; the level's intent comes next). */
/** A script Build goal's key: DAI_SCRIPT_BUILD_KEY x (its line's number + 1) + the DaiRoom, apart from the director's
 *  (key = the DaiRoom); DaiGoalBuildRoom gives the room of either. */
constexpr int64_t DAI_SCRIPT_BUILD_KEY = 1024;
constexpr int64_t DaiGoalBuildRoom(int64_t key) { return (key >= DAI_SCRIPT_BUILD_KEY) ? key % DAI_SCRIPT_BUILD_KEY : key; }
/** Forbid goals' keys: what a manager must not do while one stands. Dismissing anyone (the director's recover). */
constexpr int64_t DAI_FORBID_DISMISS = 1;
/** Defend goals' keys (06 section 11): the keeper's own ground (the defend posture); 1 + r, fortify against rival r's
 *  routes (contest). */
constexpr int64_t DAI_DEFEND_HOME = 0;
/** An Attack goal's count (07 section 5): raids only (the raid posture), no full operation; 0: an operation too;
 *  DAI_ATTACK_FINISH: the finish after an Armageddon (P9), an operation that does not wait to be ready. */
constexpr int64_t DAI_ATTACK_RAIDS = 1;
constexpr int64_t DAI_ATTACK_FINISH = 2;
enum class DaiGoalStatus : uint8_t { Open, Progress, Met, Blocked };

/** A goal's parameters: `count` (KeepImps: how many) and `key`, which with the kind says which goal it is; `at` a
 *  script Build goal's slab rectangle (x0 < 0: none). */
struct DaiGoalParams {
    int64_t count = 0;
    int64_t key = 0;
    DaiRect at{ -1, -1, -1, -1 };
    int64_t aim = -1;                   /**< Attack: the target kind (DaiTargetKind) wanted of that rival, -1 the personality's
                                         *   first_target (a lieutenant's attack objective, 14 R4) */
};

struct DaiGoal {
    uint32_t id = 0;
    DaiGoalKind kind = DaiGoalKind::Count;
    DaiFix weight = 0;
    DaiTurn deadline = 0;              /**< 0: none */
    DaiGoalStatus status = DaiGoalStatus::Open;
    DaiGoalSource source = DaiGoalSource::Director;
    DaiGoalParams p;
    DaiTurn set_turn = 0, status_turn = 0;
    char why[32] = {};                  /**< what the manager said with its last status */
};

class DaiGoals {
public:
    /** Inserts the goal, or updates the one of the same kind and key (its status stays); its id, 0 if full. */
    uint32_t Set(const DaiGoal &g, DaiTurn now);
    void Drop(uint32_t id) { m_goals.Remove(id); }
    /** The director's drop: only a goal of its own source rank (never a script's or a directive's, P8.4). */
    void DropOwn(uint32_t id)
    {
        const DaiGoal *g = m_goals.Find(id);
        if ((g != nullptr) && (g->source != DaiGoalSource::Script) && (g->source != DaiGoalSource::Directive)) m_goals.Remove(id);
    }
    const DaiGoal *Find(DaiGoalKind k, int64_t key = 0) const;
    const DaiGoal *Get(uint32_t id) const { return m_goals.Find(id); }
    void NoteStatus(uint32_t id, DaiGoalStatus s, const char *why, DaiTurn now);
    void Clear() { m_goals.Clear(); }
    int64_t Count() const { return m_goals.Count(); }
    template <typename F> void ForEach(F f) const { m_goals.ForEach(f); }
private:
    friend class DaiSave;
    DaiPool<DaiGoal, DAI_MAX_GOALS> m_goals;
};

const char *DaiGoalKindName(DaiGoalKind k);
const char *DaiGoalStatusName(DaiGoalStatus s);

#endif
