/** @file DaiGoldSides.h
 *  Whose side of the map each gold vein is on (plan 13 section 31), for the gold outlook (DaiForecast's side_gold and
 *  horizon) and Excavation's contested veins. Each keeper's route from its heart -- the keeper's own and every rival
 *  heart it has seen -- in slabs, across the map as the keeper knows it: impenetrable rock (all of it shown from the
 *  start), gems (mined in place, never dug through) and lava, abyss and impassable ground as far as revealed stop it;
 *  earth, gold, water, floor and rooms are a slab each, whoever owns them. The same measure for every keeper, so what
 *  the fog hides of a rival's tunnels does not tilt it. A vein goes to the keeper with the shortest route; every keeper
 *  within [economy] gold_contest_share % of that shares it evenly (three ways on a three-keeper map, not halves); a
 *  vein no heart reaches is nobody's.
 *
 *  The straight line it replaces (each heart's distance to the vein's centre) put gold across a rock wall or a lava lake
 *  on the near keeper's side, and on levels 60 and 79 (hearts close together, kept apart by rock) called nearly every
 *  vein contested. Worked out every DAI_SIDES_INTERVAL turns, and at once when another rival heart is seen; saved. */
#ifndef DAIMON_DAIGOLDSIDES_H
#define DAIMON_DAIGOLDSIDES_H

#include "daimon_base_types.h"
#include <cstdint>
#include <vector>

#include "globals.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiFix.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiPersonality.h"

constexpr int64_t DAI_SIDES_INTERVAL = 2000;
constexpr int32_t DAI_SIDES_NEVER = INT32_MAX;

class DaiGoldSides {
public:
    struct Vein {
        uint32_t id = 0;
        DaiFix share = 0;           /**< the keeper's share of its gold, 0..1 */
        int32_t sharers = 0;        /**< keepers sharing it (0: no heart reaches it) */
        int32_t own_route = DAI_SIDES_NEVER, best_route = DAI_SIDES_NEVER;  /**< slabs from the keeper's heart; the least */
    };
    /** Works the table out when DAI_SIDES_INTERVAL turns have passed or the rival hearts seen are not those of the last
     *  table. */
    void Update(const DaiKnowledge &k, const DaiPersonality &p, DaiTurn now);
    /** Works it out now. */
    void Compute(const DaiKnowledge &k, const DaiPersonality &p, DaiTurn now);
    /** Vein `id` as the last table has it; nullptr if the table has none (no table, or a vein newer than it). */
    const Vein *Find(uint32_t id) const;
    /** The last table had the keeper's heart and a rival heart to measure from. */
    bool Split() const { return m_split; }
    /** The rival hearts it was measured from. */
    int64_t HeartsSeen() const;
    DaiTurn Turn() const { return m_turn; }
    const std::vector<Vein> &Veins() const { return m_veins; }
    /** A keeper's route in slabs from heart (hx, hy) to every slab (DAI_SIDES_NEVER where it cannot go), as above. */
    static void Routes(const DaiKnowledge &k, DaiSlabCoord hx, DaiSlabCoord hy, std::vector<int32_t> *out);
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    uint32_t hearts_seen(const DaiKnowledge &k) const;

    std::vector<Vein> m_veins;      /**< by id */
    bool m_split = false;
    uint32_t m_hearts = 0;          /**< the rival hearts seen (a bit a player) when it was worked out */
    DaiTurn m_turn = 0;            /**< 0: never */
};

#endif
