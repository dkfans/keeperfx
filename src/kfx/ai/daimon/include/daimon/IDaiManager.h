/** @file IDaiManager.h
 *  A manager (02 section 4.8): one domain's planner. Once a turn, in registry order and filtered by the keeper's mode,
 *  it reads the knowledge, its own jobs and the inbox, and adds, refreshes or cancels its own jobs through
 *  DaiManagerWrites -- the only writes it has. Managers never call each other. */
#ifndef DAIMON_IDAIMANAGER_H
#define DAIMON_IDAIMANAGER_H

#include <algorithm>
#include <cstdint>

#include "globals.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiGoals.h"
#include "daimon/DaiInbox.h"
#include "daimon/DaiJob.h"
#include "daimon/DaiJobPool.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiObjectives.h"
#include "daimon/IDaiJobKind.h"
#include "daimon/DaiRequests.h"
#include "daimon/DaiTurnCtx.h"
#include "daimon/DaiTypes.h"

/** What one manager may change this turn: its own domain's jobs, the inbox's handled marks, the status of the goals
 *  it serves, and its own requests. */
class DaiManagerWrites {
public:
    DaiManagerWrites(DaiDomain domain, DaiJobPool &jobs, DaiInbox &inbox, DaiGoals &goals, DaiRequests &requests, DaiTurn now,
                     DaiTurn stall_window, DaiKnowledge *know = nullptr, DaiObjectives *objectives = nullptr)
        : m_domain(domain), m_jobs(jobs), m_inbox(inbox), m_goals(goals), m_requests(requests), m_now(now),
          m_stall_window(stall_window), m_know(know), m_objectives(objectives) {}
    /** A new Ready job of the manager's domain; nullptr if the pool is full. */
    DaiJob *AddJob(DaiJobKindId kind, uint64_t key, DaiFix utility)
    {
        const auto kinds = DaiJobKinds();
        const DaiTurn least = ((kind < kinds.size()) && (kinds[kind] != nullptr)) ? kinds[kind]->MinStallWindow() : 0;
        return m_jobs.Add(kind, m_domain, key, utility, m_now, std::max(m_stall_window, least));
    }
    /** The manager's unfinished job of this kind and key, or nullptr. */
    DaiJob *FindJob(DaiJobKindId kind, uint64_t key)
    {
        DaiJob *j = m_jobs.Find(kind, key);
        return ((j != nullptr) && (j->domain == m_domain)) ? j : nullptr;
    }
    /** A Blocked job tried again, with a whole stall window. */
    void Retry(DaiJob &j) { if ((j.domain == m_domain) && (j.state == DaiJobState::Blocked)) m_jobs.SetState(j, DaiJobState::Ready, "", m_now); }
    /** A Ready, Running or Blocked job dropped. */
    void Cancel(DaiJob &j, const char *why)
    {
        if ((j.domain == m_domain) && (DaiJobLive(j.state) || (j.state == DaiJobState::Blocked)))
            m_jobs.SetState(j, DaiJobState::Cancelled, why, m_now);
    }
    void HandleInbox(int64_t i) { m_inbox.Handle(i); }
    void NoteGoal(uint32_t goal_id, DaiGoalStatus s, const char *why) { m_goals.NoteStatus(goal_id, s, why, m_now); }
    /** A request under the manager's own domain (posting again refreshes it); 0 if the table is full. */
    uint32_t PostRequest(const DaiRequest &r) { return m_requests.Post(m_domain, r, m_now); }
    void WithdrawRequest(uint32_t id) { m_requests.Withdraw(m_domain, id); }
    /** A service request addressed to this manager acted on. */
    void MarkServed(uint32_t id) { m_requests.MarkServed(m_domain, id); }
    /** ...and could not be done. */
    void MarkRefused(uint32_t id) { m_requests.MarkRefused(m_domain, id); }
    /** Excavation's handle on a vein's status, entry and route (03 section 4: the table builds veins, Excavation says
     *  what it is doing with them); nullptr for any other domain. */
    DaiVein *Vein(uint32_t id) { return ((m_domain == DaiDomain::Excavation) && (m_know != nullptr)) ? m_know->VeinForUpdate(id) : nullptr; }
    /** An objective under the manager's own domain (posting again with its kind and key updates it); 0 if the table is
     *  full or there is none. */
    uint32_t PostObjective(const DaiObjective &o) { return (m_objectives != nullptr) ? m_objectives->Post(m_domain, o, m_now) : 0; }
    void DropObjective(uint32_t id) { if (m_objectives != nullptr) m_objectives->Drop(m_domain, id); }
    /** Tactics' report on an objective (another domain's is refused). */
    bool WriteReport(uint32_t id, const DaiObjectiveReport &r) { return (m_objectives != nullptr) && m_objectives->WriteReport(m_domain, id, r); }
    DaiDomain GetDomain() const { return m_domain; }
private:
    DaiDomain m_domain;
    DaiJobPool &m_jobs;
    DaiInbox &m_inbox;
    DaiGoals &m_goals;
    DaiRequests &m_requests;
    DaiTurn m_now;
    DaiTurn m_stall_window;
    DaiKnowledge *m_know;
    DaiObjectives *m_objectives;
};

class IDaiManager {
public:
    virtual ~IDaiManager() = default;
    virtual const char *GetName() const = 0;
    virtual DaiDomain GetDomain() const = 0;
    /** Once a turn: read, then add / refresh / cancel own jobs. */
    virtual void Plan(const DaiTurnCtx &c, DaiManagerWrites &w) = 0;
    /** One of its jobs ended (Done, Failed, Blocked, Cancelled) since the last Plan; called once per end. */
    virtual void OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w) { (void)c; (void)j; (void)w; }
    /** Handed back, or the level ended. */
    virtual void Reset() {}
    /** The manager's private state in a saved game (its own section, by name). */
    /** Its state's format in the save (inside MGRS): a manager whose saved version differs starts afresh, the others
     *  are read. Bump it when Save's layout changes (14 F1). */
    virtual uint16_t SaveVersion() const { return 1; }
    virtual void Save(DaiBlobWriter &w) const { (void)w; }
    virtual bool Load(DaiBlobReader &r) { (void)r; return true; }
};

#endif
