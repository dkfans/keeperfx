/** @file DaiKeeper.h
 *  The per-keeper root (02 section 6.1): identity, mode, resolved personality, the keeper's random stream, what it
 *  knows, its inbox, goals, jobs and claims, the director, its managers (registry order, 02 section 4.8: Phase 0 has
 *  Workforce), the arbiter and its profile. One turn is Turn() (02 section 5); every DAI_SUMMARY_TURNS a summary line
 *  goes to the log at the normal level. */
#ifndef DAIMON_DAIKEEPER_H
#define DAIMON_DAIKEEPER_H

#include <deque>
#include <memory>
#include <vector>

#include "globals.h"
#include "daimon_base_types.h"
#include "net_main.h"
#include "config_daimon.h"
#include "daimon/DaiArbiter.h"
#include "daimon/DaiDirector.h"
#include "daimon/DaiGoals.h"
#include "daimon/DaiGoldSides.h"
#include "daimon/DaiPerimeter.h"
#include "daimon/DaiLeases.h"
#include "daimon/DaiInbox.h"
#include "daimon/DaiJobPool.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiLedger.h"
#include "daimon/DaiPersonality.h"
#include "daimon/DaiProf.h"
#include "daimon/DaiCtlWorld.h"
#include "daimon/DaiObjectives.h"
#include "daimon/DaiOutput.h"
#include "daimon/DaiPlanWorld.h"
#include "daimon/DaiWorker.h"
#include "daimon/DaiRequests.h"
#include "daimon/DaiSearchPool.h"
#include "daimon/DaiThreatModel.h"
#include "daimon/DaiDifficulty.h"
#include "daimon/DaiRng.h"
#include "daimon/DaiTypes.h"
#include "daimon/IDaiActor.h"
#include "daimon/IDaiManager.h"
#include "daimon/IDaiMode.h"
#include "daimon/IDaiWorldReader.h"

/** The planning delay D at a turn rate (12 section 4): the turns from a gather to its output's apply, at least 1,
 *  the rate over the base rate rounded up -- the wall time a planning turn has stays the same when the game is faster. */
DaiTurn DaiPlanDelay(int64_t turn_rate, int64_t base_rate);
/** Gestures submitted whose result has not come yet, kept against their provisional ids (older ones are forgotten). */
constexpr size_t DAI_VERB_MAP_MAX = 64;

/** A gesture's provisional id (the planning turn's) and the controller's own. */
struct DaiVerbMapping {
    int64_t provisional = 0;
    int64_t real = 0;
};

/** What one gather hands its planning turn (12 section 13.9): everything the plan reads that the main thread goes on
 *  changing, copied, so the next gathers can run while it plans. */
struct DaiSnapshot {
    DaiTurn now = 0;
    DaiTurn lands = 0;                 /**< when its output is applied */
    DaiBudget budget;
    DaimonGlobalConfig cfg{};
    std::vector<KeeperFact> facts;
    std::vector<DaiActResult> results;  /**< the controller's, under provisional ids */
    bool idle = true;                   /**< the controller, at the gather */
    int64_t scanned = 0;                /**< slabs the gather read (the profile) */
    DaiPlanWorld world;
    DaiKnowledge know;
    DaiLeases leases;                   /**< the human's decisions (an assistant's), as of the gather */
    DaiStageClock clock;                /**< the gather's stages, then the plan's */
};

/** A planning turn: its snapshot, its posting on the workers (0: run inline), then its output. */
struct DaiPlanJob {
    int64_t seq = 0;
    DaiSnapshot snap;
    DaiPlanOutput out;
};

/** Turns between a keeper's summary lines in the log (01 section 11). */
constexpr DaiTurn DAI_SUMMARY_TURNS = 1000;

class DaiKeeper {
public:
    DaiKeeper(PlayerNumber plyr, NetUserId slot, std::unique_ptr<IDaiMode> mode, const DaiPersonality &personality,
              int64_t prev_model, DaimonTakeReason why, DaiTurn now, uint64_t seed, IDaiWorldReader &world,
              IDaiActor &actor);

    /** One turn of this keeper (12 section 13.5): the outputs planned DAI_PLAN_DELAY turns ago applied; the gather
     *  (facts, the main knowledge, the controller's results); the plan on a copy of the knowledge (inbox, results,
     *  progress and the stall rule, the director, the managers, at most one new gesture from the arbiter) into an
     *  output applied DAI_PLAN_DELAY turns later. */
    /** `turn_rate`: the game's turns a second as it runs them (0: the base rate), which sets D. With `workers` the
     *  planning turn is posted to the keeper's worker (keyed by its player) and this returns at once; without, it runs
     *  here. Throws when a planning turn it had to wait for (one whose output lands now) threw. */
    void Turn(DaiTurn now, const DaiBudget &budgets, const DaimonGlobalConfig &cfg, int64_t turn_rate = 0, DaiWorkers *workers = nullptr);
    /** Every planning turn posted has run and its output waits to be applied: the main thread may read or change the
     *  keeper now (save, dump, release, a test's look). "" or what a planning turn threw (the keeper should go). */
    std::string Settle();
    /** D at the last gather. */
    DaiTurn LastDelay() const { return m_last_delay; }
    /** Lines for the console and the log: `what` is "status" (the keeper, what it knows, its work), "know", "goals",
     *  "requests", "jobs", "claims" or "inbox"; false for any other. */
    bool Dump(const char *what, void (*line)(const char *)) const;
    /** The one-line summary (status, work and profile of the window since the last one). */
    void Summary(char *buf, size_t len) const;
    /** Every claim goes and every gesture is cancelled: the one exit a hand-back, defeat, level end or mode change
     *  takes (lesson L8). The keeper may be dropped after it. Not done by the destructor: a keeper read from a saved
     *  game and then thrown away must not touch the slot it names. */
    void Release();

    PlayerNumber GetPlayer() const { return m_player; }
    NetUserId GetSlot() const { return m_slot; }
    const IDaiMode &GetMode() const { return *m_mode; }
    IDaiMode &Mode() { return *m_mode; }
    const DaiPersonality &GetPersonality() const { return m_personality; }
    const DaiDifficulty &GetDifficulty() const { return m_difficulty; }
    /** At the take or a load, before its first turn. */
    void SetDifficulty(const DaiDifficulty &d) { m_difficulty = d; }
    /** DAIMON_SET (Phase 7): one of its personality's keys; saved with it (daimon.bin keeps every key's value). */
    bool SetPersonalityKey(const char *dotted, const char *text, int64_t *was, int64_t *now, std::string *error)
    {
        return m_personality.Set(dotted, text, was, now, error);
    }
    /** "random" chose its personality (05 section 5): the screens hide it until the end. */
    bool RandomPersonality() const { return m_random_personality; }
    void SetRandomPersonality(bool r) { m_random_personality = r; }
    /** The knowledge the last planning turn decided on. */
    const DaiKnowledge &GetKnowledge() const { return m_plan_know; }
    /** The main thread's: read each gather, the controller's map questions answered from it. */
    const DaiKnowledge &GetMainKnowledge() const { return m_knowledge; }
    /** Outputs planned or being planned and not yet applied (tests). */
    int64_t PendingOutputs() const { return (int64_t)(m_pending.size() + m_inflight.size()); }
    const DaiInbox &GetInbox() const { return m_inbox; }
    const DaiJobPool &GetJobs() const { return m_jobs; }
    const DaiLedger &GetLedger() const { return m_ledger; }
    const DaiArbiter &GetArbiter() const { return m_arbiter; }
    /** The assistant's last status line (main thread: written when a plan's output lands). */
    const std::string &StatusLine() const { return m_status; }
    const DaiDirector &GetDirector() const { return m_director; }
    const DaiGoals &GetGoals() const { return m_goals; }
    const DaiRequests &GetRequests() const { return m_requests; }
    /** A request posted as manager `from` would post it (the ftests' way to ask a service before its asker exists). */
    uint32_t PostRequest(DaiDomain from, const DaiRequest &r, DaiTurn now) { return m_requests.Post(from, r, now); }
    uint32_t SetGoal(const DaiGoal &g, DaiTurn now) { return m_goals.Set(g, now); }
    template <typename F> void ForEachGoal(F f) const { m_goals.ForEach(f); }
    void DropGoal(uint32_t id) { m_goals.Drop(id); }
    /** The goals themselves, for the directives (the facade, settled first). */
    DaiGoals &Goals() { return m_goals; }
    /** The human's touch log (09 section 5.3, P7.11): one packet's touches into the leases. */
    /** An assistant's human, or a lieutenant's agent: a creature it touched that one of the keeper's jobs holds fails
     *  that job (PREEMPTED_BY_PLAYER / PREEMPTED_BY_AGENT, 10 section 7.2), which plans again around the lease. */
    void NoteTouches(const KeeperTouch *t, int64_t n, DaiTurn now);
    const DaiLeases &Leases() const { return m_leases; }
    /** A directive's perimeter (P8.4): opened, or closed again. */
    void SetPerimeterOpen(bool open) { m_perimeter.SetOpenState(open); }
    /** The human's decisions carried over from an assistant this one replaces (a personality changed, P7.13). */
    void SetLeases(const DaiLeases &l) { m_leases = l; m_human_kept_stale = true; }
    const DaiObjectives &GetObjectives() const { return m_objectives; }
    const DaiPerimeter &GetPerimeter() const { return m_perimeter; }
    const DaiForecastOut &GetForecast() const { return m_forecast; }
    /** The manager of a domain (nullptr if it has none): tests and the dump read its state. */
    const IDaiManager *Manager(DaiDomain d) const;
    const DaiSearchPool &GetSearches() const { return m_searches; }
    /** Where its enemies are and how they would come (06 section 3). */
    const DaiThreatModel &GetThreat() const { return m_threat; }
    const DaiRivals &GetRivals() const { return m_rivals; }
    /** What was left of each budget after the last turn (02 section 5): below 0 means a class overspent it. */
    const DaiBudget &LastBudgetLeft() const { return m_budget_left; }
    /** The controller's map questions for this keeper's slot, answered from its knowledge. */
    const KeeperCtlWorld *CtlWorld() const { return m_ctl_world.World(); }
    /** The keeper's answers to the controller's map questions (their counts). */
    const DaiCtlWorld &GetCtlWorld() const { return m_ctl_world; }
    const DaiProf &GetProf() const { return m_prof; }
    /** Jobs that ended, by state, since the keeper was taken (tests, the dump). */
    int64_t JobsEnded(DaiJobState s) const { return m_ended[(size_t)s]; }
    /** Jobs of a kind that ended failed or stalled since the keeper was built or loaded (not saved: for tests). */
    int64_t JobsFailedOfKind(DaiJobKindId kind) const { return (kind < m_failed_kind.size()) ? m_failed_kind[kind] : 0; }
    int64_t GetPrevModel() const { return m_prev_model; }
    DaimonTakeReason GetTakeReason() const { return m_why; }
    /** A defeated keeper stays Daimon's (its flag is simulation state the replay keeps) but does nothing more. */
    void SetDefeated();
    bool IsDefeated() const { return m_defeated; }
private:
    /** A provisional verb (a job's verb_id) withdrawn on the main thread: the controller's real verb if it landed, else
     *  its submit still waiting in the outputs. */
    void cancel_provisional(int64_t provisional);
    std::string status_line() const;
    void log_summary(DaiTurn now, const DaimonGlobalConfig &cfg);
    friend class DaiSave;
    void collect(DaiTurn upto);
    void apply(DaiTurn now);
    void gather(DaiTurn now, int64_t turn_rate, DaiSnapshot &s);
    void plan(DaiPlanJob &job);
    void take_results(const DaiTurnCtx &c, const std::vector<DaiActResult> &results);
    void track_jobs(const DaiTurnCtx &c, IDaiActor &act);
    std::vector<DaiSlabXY> watch_slabs(const DaiTurnCtx &c) const;
    void run_managers(const DaiTurnCtx &c, const DaimonGlobalConfig &cfg, DaiKnowledge &know, DaiStageClock &clock);
    void know_lines(void (*line)(const char *)) const;

    PlayerNumber m_player;
    NetUserId m_slot;
    std::unique_ptr<IDaiMode> m_mode;
    DaiPersonality m_personality;
    DaiDifficulty m_difficulty;
    bool m_random_personality = false;
    int64_t m_prev_model;
    DaimonTakeReason m_why;
    DaiTurn m_taken_turn;
    DaiTurn m_last_turn = 0;
    DaiRng m_rng;
    bool m_defeated = false;
    IDaiWorldReader *m_world;
    IDaiActor *m_actor;
    DaiKnowledge m_knowledge;           /**< the main thread's (gather, apply, the controller's questions) */
    DaiKnowledge m_plan_know;           /**< the knowledge the last planning turn decided on (its snapshot's) */
    DaiInbox m_inbox;
    DaiGoals m_goals;
    DaiRequests m_requests;
    DaiObjectives m_objectives;
    DaiSearchPool m_searches;
    DaiIncomeEma m_income;              /**< the forecast's one history (saved) */
    DaiGoldSides m_gold_sides;          /**< whose side each vein is on, for the forecast (plan 13 section 31; saved) */
    DaiPerimeter m_perimeter;           /**< the slabs that close its home off (plan 13 section 32; saved) */
    DaiLeases m_leases;                 /**< the human's decisions an assistant leaves alone (P7.11; saved) */
    bool m_human_kept_stale = true;     /**< the leases' unmarked slabs not yet in the knowledge's Kept (not saved) */
    DaiForecastOut m_forecast;          /**< this turn's, for every manager */
    DaiThreatModel m_threat;            /**< Defence's (Phase 4); updated each turn after the knowledge */
    DaiRivals m_rivals;                 /**< Offence's (Phase 5); updated each turn after the threat model */
    DaiBudget m_budget_left;
    DaiCtlWorld m_ctl_world{ m_knowledge };
    DaiDirector m_director;
    DaiJobPool m_jobs;
    DaiLedger m_ledger;
    DaiArbiter m_arbiter;
    std::vector<std::unique_ptr<IDaiManager>> m_managers;
    int64_t m_last_result = 0;          /**< the controller's last result id taken (its own ids) */
    // The pipeline (12 section 13.5).
    // Main thread: the outputs done and not applied, the planning turns posted (oldest first), D's state.
    std::deque<DaiPlanOutput> m_pending;
    std::deque<std::shared_ptr<DaiPlanJob>> m_inflight;
    DaiWorkers *m_workers = nullptr;
    DaiTurn m_last_lands = 0;          /**< the last output's landing turn: none lands before it */
    DaiTurn m_last_delay = 1;
    std::vector<DaiQuestion> m_questions;   /**< asked by the outputs applied this turn, for the gather */
    // Planning side: its outputs not landed yet (their landing turn, their vein edits: still part of its view), the
    // landing turn of its last gesture (the controller is busy with it until then), the turn it last planned.
    std::deque<std::pair<DaiTurn, std::vector<DaiVeinEdit>>> m_unlanded;
    DaiTurn m_last_submit_lands = 0;
    /** Its gestures with no result yet (provisional id, the turn it planned it): at D > 1 up to D of them may be out at
     *  once, queued in the controller behind the running one (12 section 13.13). */
    std::vector<std::pair<int64_t, DaiTurn>> m_outstanding;
    DaiTurn m_plan_turn = 0;
    std::vector<DaiVerbMapping> m_verb_map;
    std::string m_status = "waiting for work";      /**< the assistant's line (apply), see StatusLine */
    std::vector<DaiActResult> m_refused;    /**< submits refused at an apply, for the next gather (provisional ids) */
    int64_t m_next_verb = 0;            /**< the last provisional id given */
    DaiProf m_prof;
    std::array<int64_t, (size_t)DaiJobState::Cancelled + 1> m_ended{};
    std::array<int64_t, DAI_JOB_KINDS> m_failed_kind{};
};

#endif
