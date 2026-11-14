/** @file DaiLeases.h
 *  The human's decisions the assistant leaves alone (docs/plans/ai-plans/v2/09 section 5.3, Daimon P7.11): from the
 *  touch log (keeper_touch_types.h) -- a creature the human picked up, dropped, slapped, cast on or possessed is theirs
 *  for [assistant] lease_turns; a slab they marked or unmarked for digging, a room they built or sold, a trap or door
 *  they placed or sold stays as they left it until they change it (the newest of their decisions on a slab, in each
 *  family, stands). Saved with the keeper.
 */
#ifndef DAIMON_DAILEASES_H
#define DAIMON_DAILEASES_H

#include <cstdint>
#include <map>
#include <utility>
#include <vector>

#include "globals.h"
#include "keeper_touch_types.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiTypes.h"

class DaiLeases {
public:
    /** The slab decisions' families: one standing decision a slab in each. */
    enum class Family : uint8_t { Dig, Room, Trap, Door, Count };
    static constexpr int64_t MAX_SLABS = 4096, MAX_CREATURES = 256;

    void Note(const KeeperTouch &t, DaiTurn now);
    /** The creature is the human's: they touched it within lease_turns. By index and creation turn (02 section 6.2):
     *  a reused index is another creature; a ref whose creation turn is not known (0) is taken as the leased one. */
    bool CreatureLeased(const DaiThingRef &t, DaiTurn now, int64_t lease_turns) const;
    /** The human's standing decision on a slab in a family (KTch_None: none). */
    uint8_t Decision(Family f, DaiSlabCoord x, DaiSlabCoord y) const;
    /** May the assistant mark the slab for digging (the human did not unmark it), take its mark off (they did not mark
     *  it), build or sell there (they did not sell or build it), place or sell a trap or door there. */
    bool MayTag(DaiSlabCoord x, DaiSlabCoord y) const { return Decision(Family::Dig, x, y) != KTch_DigUntag; }
    bool MayUntag(DaiSlabCoord x, DaiSlabCoord y) const { return Decision(Family::Dig, x, y) != KTch_DigTag; }
    bool MayBuild(DaiSlabCoord x, DaiSlabCoord y) const { return Decision(Family::Room, x, y) != KTch_RoomSold; }
    bool MaySellRoom(DaiSlabCoord x, DaiSlabCoord y) const { return Decision(Family::Room, x, y) != KTch_RoomBuilt; }
    bool MayPlaceTrap(DaiSlabCoord x, DaiSlabCoord y) const { return Decision(Family::Trap, x, y) != KTch_TrapSold; }
    bool MaySellTrap(DaiSlabCoord x, DaiSlabCoord y) const { return Decision(Family::Trap, x, y) != KTch_TrapPlaced; }
    bool MayPlaceDoor(DaiSlabCoord x, DaiSlabCoord y) const { return Decision(Family::Door, x, y) != KTch_DoorSold; }
    bool MaySellDoor(DaiSlabCoord x, DaiSlabCoord y) const { return Decision(Family::Door, x, y) != KTch_DoorPlaced; }
    /** The slabs whose standing dig decision is the human's unmark. */
    std::vector<DaiSlabXY> Untagged() const;
    /** Creature leases past lease_turns are dropped. */
    void Expire(DaiTurn now, int64_t lease_turns);
    int64_t SlabCount() const { return (int64_t)m_slabs.size(); }
    int64_t CreatureCount() const { return (int64_t)m_creatures.size(); }
    void Clear() { m_slabs.clear(); m_creatures.clear(); }
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    static int64_t key(Family f, DaiSlabCoord x, DaiSlabCoord y) { return ((int64_t)f << 40) | ((int64_t)(x & 0xFFFFF) << 20) | (int64_t)(y & 0xFFFFF); }
    struct SlabDecision { uint8_t kind = KTch_None; DaiTurn turn = 0; };
    std::map<int64_t, SlabDecision> m_slabs;        /**< ordered: the save's order is the same on every machine */
    struct CreatureLease { DaiTurn created = 0, turn = 0; };
    std::map<DaiThingIndex, CreatureLease> m_creatures;    /**< by index; the creation turn tells a reused index apart */
};

#endif
