/** @file DaiInbox.h
 *  A keeper's event inbox (02 section 4.4, 01 section 8.1): the keeper-facts tracker's facts, kept until a manager
 *  handles them or they expire. The same kind of fact in the same place refreshes its entry instead of adding one;
 *  an unhandled entry is offered again every turn until it expires (DK2's model, lesson L9). A ring of DAI_MAX_INBOX
 *  entries: when it is full, a new fact takes the oldest handled or expired entry, else the oldest. */
#ifndef DAIMON_DAIINBOX_H
#define DAIMON_DAIINBOX_H

#include "daimon_base_types.h"
#include <array>
#include <cstdint>

#include "globals.h"
#include "config_daimon.h"
#include "keeper_facts_types.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiLimits.h"

struct DaiInboxEntry {
    enum KeeperFactKind kind = KF_None;
    int64_t sub = 0;
    DaiSubtlCoord x = 0, y = 0;
    int64_t args[4] = {};
    DaiTurn turn = 0;          /**< the fact's turn (the last repeat's) */
    DaiTurn expires = 0;
    uint32_t key = 0;           /**< kind, sub and slab: what "the same fact" means */
    bool handled = false;
};

class DaiInbox {
public:
    /** Takes this turn's facts: new entries, or refreshed ones (offered again, a new expiry). */
    void Feed(const KeeperFact *facts, int64_t n, DaiTurn now, const DaimonGlobalConfig &cfg);
    /** The first unhandled, unexpired entry at or after `from` whose kind is in `kinds_mask` (bit 1 << kind); -1 if
     *  none. Entries are in arrival order. */
    int64_t Next(uint32_t kinds_mask, int64_t from) const;
    const DaiInboxEntry &Get(int64_t i) const { return m_ring[(size_t)slot(i)]; }
    void Handle(int64_t i);
    /** Drops entries whose expiry has passed. */
    void Expire(DaiTurn now);
    void Clear();
    /** Entries held (handled ones included, until they expire or are overwritten). */
    int64_t Count() const { return m_count; }
    /** Facts taken, and how many of them refreshed an entry rather than adding one. */
    int64_t Fed() const { return m_fed; }
    int64_t Refreshed() const { return m_refreshed; }
    /** The entries in order, and the counters (a saved game, P0.7). */
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    int64_t slot(int64_t i) const { return (m_first + i) % DAI_MAX_INBOX; }
    void drop_first();
    std::array<DaiInboxEntry, (size_t)DAI_MAX_INBOX> m_ring{};
    int64_t m_first = 0, m_count = 0;
    int64_t m_fed = 0, m_refreshed = 0;
};

#endif
