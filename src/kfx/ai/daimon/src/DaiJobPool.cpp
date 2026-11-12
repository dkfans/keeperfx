#include "pre_inc.h"
#include "daimon/DaiJobPool.h"

#include <cinttypes>
#include <cstring>

#include "daimon/DaiLedger.h"
#include "daimon/DaiLog.h"
#include "post_inc.h"

const char *DaiJobStateName(DaiJobState s)
{
    switch (s) {
    case DaiJobState::Ready: return "ready";
    case DaiJobState::Running: return "running";
    case DaiJobState::Blocked: return "blocked";
    case DaiJobState::Done: return "done";
    case DaiJobState::Failed: return "failed";
    case DaiJobState::Cancelled: return "cancelled";
    }
    return "?";
}

DaiJob *DaiJobPool::Add(DaiJobKindId kind, DaiDomain domain, uint64_t key, DaiFix utility, DaiTurn now, DaiTurn stall_window)
{
    DaiJob *j = m_jobs.Add();
    if (j == nullptr) {
        DaiLog::Warn(m_player, "the job pool is full (%" PRId64 " jobs)", DAI_MAX_JOBS);
        return nullptr;
    }
    j->kind = kind;
    j->domain = domain;
    j->key = key;
    j->utility = utility;
    j->created = now;
    j->state_turn = now;
    j->last_progress_turn = now;
    j->stall_window = stall_window;
    DaiLog::Debug(m_player, "j%u kind %u created (utility %" PRId64 ")", j->id, (unsigned)kind, utility);
    return j;
}

DaiJob *DaiJobPool::Find(DaiJobKindId kind, uint64_t key)
{
    DaiJob *found = nullptr;
    m_jobs.ForEach([&](DaiJob &j) {
        if ((found == nullptr) && (j.kind == kind) && (j.key == key) && (DaiJobLive(j.state) || (j.state == DaiJobState::Blocked)))
            found = &j;
    });
    return found;
}

DaiJob *DaiJobPool::FindByVerb(int64_t verb_id)
{
    DaiJob *found = nullptr;
    if (verb_id == 0) return nullptr;
    m_jobs.ForEach([&](DaiJob &j) { if ((found == nullptr) && (j.verb_id == verb_id)) found = &j; });
    return found;
}

void DaiJobPool::SetState(DaiJob &j, DaiJobState s, const char *why, DaiTurn now)
{
    const DaiJobState was = j.state;
    j.state = s;
    j.state_turn = now;
    snprintf(j.why, sizeof(j.why), "%s", (why != nullptr) ? why : "");
    if (DaiJobEnded(s)) {
        j.end_seen = false;
        j.verb_id = 0;
    } else if (was == DaiJobState::Blocked) {
        j.last_progress_turn = now; // a retry gets a whole window
    }
    DaiLog::Debug(m_player, "j%u kind %u %s -> %s%s%s", j.id, (unsigned)j.kind, DaiJobStateName(was), DaiJobStateName(s),
        (j.why[0] != '\0') ? ": " : "", j.why);
}

void DaiJobPool::NoteProgress(DaiJob &j, int64_t progress, DaiTurn now)
{
    if (progress == j.last_progress) return;
    j.last_progress = progress;
    j.last_progress_turn = now;
}

int64_t DaiJobPool::CheckStalls(DaiTurn now, DaiLedger &ledger, int64_t *cancel, int64_t max,
                                bool (*held)(const void *ctx, const DaiJob &j), const void *ctx)
{
    int64_t n = 0;
    m_jobs.ForEach([&](DaiJob &j) {
        if ((held != nullptr) && DaiJobLive(j.state) && held(ctx, j)) { j.last_progress_turn = now; return; }
        if (!DaiJobLive(j.state) || (now < j.last_progress_turn + j.stall_window)) return;
        if ((j.verb_id != 0) && (n < max)) cancel[n++] = j.verb_id;
        char why[32];
        snprintf(why, sizeof(why), "STALLED (%" PRIu64 " turns)", (uint64_t)(now - j.last_progress_turn));
        SetState(j, DaiJobState::Blocked, why, now);
        j.retry_after = now + j.stall_window;
        ledger.ReleaseJob(j.id);
    });
    return n;
}

void DaiJobPool::Reap()
{
    uint32_t ids[DAI_MAX_JOBS];
    int64_t n = 0;
    m_jobs.ForEach([&](const DaiJob &j) {
        if (DaiJobEnded(j.state) && (j.state != DaiJobState::Blocked) && j.end_seen) ids[n++] = j.id;
    });
    for (int64_t i = 0; i < n; i++) m_jobs.Remove(ids[i]);
}
