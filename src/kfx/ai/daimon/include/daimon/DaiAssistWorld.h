/** @file DaiAssistWorld.h
 *  The combat assistant's decision (keeper_assist_decide) as Daimon's Tactics runs it (06 section 7): its candidates
 *  and dungeon facts built from the keeper's knowledge -- its own fighters in battles, and only the enemies it sees --
 *  and its world answered by the reader's read-only checks (PowerAvailable, PowerPrice, PowerTakes) and, for the
 *  creatures an area power would also hit, by the creatures the keeper knows of. No simulation call (02 C10).
 *
 *  Differences from the assistant's own candidates (keeper_assist_candidates), where the knowledge has less: its
 *  imps are not candidates (the knowledge has no health for them); an enemy's absolute health and fleeing are
 *  unknown (0); `score` (most_valuable) is the creature's force; allies' creatures are not counted near an area power
 *  (the knowledge keeps only enemies and its own). */
#ifndef DAIMON_DAIASSISTWORLD_H
#define DAIMON_DAIASSISTWORLD_H

#include "daimon_base_types.h"
#include <array>
#include <cstdint>

#include "keeper_assist.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/IDaiPlanWorld.h"

class DaiAssistWorld {
public:
    DaiAssistWorld(const IDaiPlanWorld &world, const DaiKnowledge &k, PlayerNumber p);
    DaiAssistWorld(const DaiAssistWorld &) = delete;
    DaiAssistWorld &operator=(const DaiAssistWorld &) = delete;
    const KeeperAssistWorld *World() const { return &m_world; }
    /** The candidates, as keeper_assist_candidates() orders them: battle by battle (the knowledge's order) its own
     *  creatures in it, then the enemies in it it sees, each with the battle's facts; then (`want_any`) its other
     *  creatures. Up to KEEPER_ASSIST_MAX_CANDIDATES; how many. The world answers for these until the next call. */
    int Candidates(bool want_fights, bool want_any, KeeperAssistCandidate *out);
    /** The dungeon facts (KAF_Count entries, the target facts 0). */
    void DungeonFacts(int64_t *facts) const;

    /** For the callbacks. */
    struct Known {
        DaiThingIndex thing = 0;
        DaiThingRef ref;
        DaiSubtlCoord x = 0, y = 0;
    };
    const Known *Find(DaiThingIndex thing) const;
    const IDaiPlanWorld &Reader() const { return m_reader; }
    const DaiKnowledge &Knowledge() const { return m_k; }
    PlayerNumber Player() const { return m_player; }
private:
    const IDaiPlanWorld &m_reader;
    const DaiKnowledge &m_k;
    PlayerNumber m_player;
    KeeperAssistWorld m_world{};
    std::array<Known, KEEPER_ASSIST_MAX_CANDIDATES> m_known{};
    int m_nknown = 0;
};

#endif
