#include "pre_inc.h"
#include "daimon/DaiProf.h"

#include <cmath>

#include "post_inc.h"

namespace {
int64_t bucket_of(int64_t ns)
{
    if (ns <= DAI_PROF_MIN_NS) return 0;
    const int64_t b = (int64_t)(8.0 * std::log2((double)ns / (double)DAI_PROF_MIN_NS));
    return (b < DAI_PROF_BUCKETS) ? b : DAI_PROF_BUCKETS - 1;
}

void note(DaiProfStageCounts &c, int64_t ns)
{
    c.ns_total += ns;
    if (ns > c.ns_max) c.ns_max = ns;
    c.hist[(size_t)bucket_of(ns)]++;
}
}

const char *DaiProfStageName(DaiProfStage s)
{
    static const char *const names[] = { "apply", "facts", "read", "derive", "copy", "feed", "threat", "rivals", "results", "director" };
    if (s < DaiProfStage::Manager) return names[(size_t)s];
    if (s < DaiProfStage::Arbiter) return DaiDomainName((DaiDomain)((uint8_t)s - (uint8_t)DaiProfStage::Manager));
    if (s == DaiProfStage::Arbiter) return "arbiter";
    if (s == DaiProfStage::Watch) return "watch";
    if (s == DaiProfStage::Tail) return "tail";
    return "?";
}

int64_t DaiProfStageCounts::Percentile(int64_t turns, int64_t per_mille) const
{
    if (turns <= 0) return 0;
    const int64_t want = (turns * per_mille + 999) / 1000;
    int64_t seen = 0;
    for (int64_t b = 0; b < DAI_PROF_BUCKETS; b++) {
        seen += hist[(size_t)b];
        if (seen >= want) return (int64_t)((double)DAI_PROF_MIN_NS * std::exp2((double)(b + 1) / 8.0));
    }
    return ns_max;
}

void DaiProf::End(DaiStageClock &clock, int64_t slabs, int64_t facts, int64_t gestures)
{
    clock.Mark(DaiProfStage::Tail);
    int64_t ns = 0;
    for (int64_t s : clock.ns) ns += s;
    for (DaiProfCounts *c : { &m_window, &m_total }) {
        c->turns++;
        c->ns_total += ns;
        if (ns > c->ns_max) c->ns_max = ns;
        c->slabs_scanned += slabs;
        c->facts += facts;
        c->gestures += gestures;
    }
    for (size_t s = 0; s < m_stages.size(); s++) note(m_stages[s], clock.ns[s]);
    note(m_turn_counts, ns);
}
