/** @file DaiKnowledge.h
 *  What a Daimon keeper knows of the world (02 section 4.3, 01 section 7): its own dungeon, exact and fresh each
 *  turn; the map as last seen, one packed word a slab, refreshed by a rolling scan within a slab budget and at once
 *  where a fact points; what it has seen of the other keepers. Read only through IDaiWorldReader, so it is never more
 *  than a human in the seat could know (D2). Decision classes read it through the const accessors. */
#ifndef DAIMON_DAIKNOWLEDGE_H
#define DAIMON_DAIKNOWLEDGE_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include "daimon/DaiLimits.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiPool.h"
#include "daimon/DaiReachMap.h"
#include "daimon/DaiTypes.h"
#include "daimon/IDaiWorldReader.h"

/** One slab as known (unpacked from the map's 32-bit word). */
struct DaiSlabInfo {
    DaiTerrain terrain = DaiTerrain::Unknown;
    PlayerNumber owner = -1;
    RoomKind room = 0;
    bool revealed = false;
    bool marked = false;
    bool dirty = false;         /**< a fact pointed here; re-read next update */
    bool reach = false;         /**< an imp can stand on it (the last complete reach map, 03 section 5) */
    uint16_t seen = 0;          /**< the turn it was last read, / 64, low 11 bits */
};

/** What the keeper has seen of another keeper. */
struct DaiRivalSighting {
    DaiTurn last_seen = 0;     /**< 0: never */
    bool heart_seen = false;
    DaiSubtlCoord heart_x = 0, heart_y = 0;
    int64_t creatures_last_cycle = 0;   /**< creatures seen in the last full scan of the map */
    int64_t force_last_cycle = 0;       /**< their levels summed */
    DaiTurn force_turn = 0;            /**< when that cycle ended */
    int64_t creatures_this_cycle = 0;
    int64_t force_this_cycle = 0;
};

/** Phase 5 (07 section 3.1): another keeper's trap or door, or room, as last seen -- remembered after it goes out of
 *  view (seen through a Sight of Evil that has closed), forgotten once its slab is in view again without it. */
struct DaiEnemyDefence {
    DaiEnemyDefenceLook look;
    DaiTurn seen = 0;
};
struct DaiRivalRoom {
    DaiRivalRoomLook look;
    DaiTurn seen = 0;
};

/** What Excavation is doing with a vein (it sets the status, 03 section 7; the table only builds and updates veins). */
enum class DaiVeinStatus : uint8_t { Untouched, Routing, Mining, Exhausted, Unreachable, Count };

/** A connected run of revealed valuable slabs (4 sides: gold, dense gold and gems, 03 section 4). */
/** What planning changed of a vein (Excavation's fields), by its id. */
struct DaiVeinEdit {
    uint32_t id = 0;
    DaiVeinStatus status = DaiVeinStatus::Untouched;
    DaiSlabCoord ex = 0, ey = 0;
    int32_t route_cost = -1;
    DaiTurn retry_after = 0;
};

struct DaiVein {
    uint32_t id = 0;            /**< stable while the vein lasts: a vein that changes shape keeps the id it overlaps most */
    DaiVeinStatus status = DaiVeinStatus::Untouched;
    int32_t gold_slabs = 0, dense_slabs = 0, gem_slabs = 0;
    int64_t gold_left = 0;      /**< summed over its gold and dense gold slabs */
    DaiSlabCoord cx = 0, cy = 0;    /**< the vein slab nearest its centroid (never outside a donut-shaped vein) */
    DaiRect box;                    /**< the rectangle round its slabs (a scan of the vein reads only this) */
    DaiSlabCoord ex = 0, ey = 0;    /**< the entry Excavation's last search found */
    int32_t route_cost = -1;    /**< turns, from that search; -1 unknown */
    DaiTurn retry_after = 0;   /**< an Unreachable vein is tried again after this turn */
    int64_t Slabs() const { return (int64_t)gold_slabs + dense_slabs + gem_slabs; }
};

constexpr int64_t DAI_MAX_VEINS = 64;

/** One of the keeper's rooms by Daimon's own identity (04 section 2.3): the game's room index changes when a room is
 *  sold or split, so a room is known by its kind and a slab inside it. Kept while it stands and after it is lost (a
 *  rebuild may want it back, 04 section 6), until the table needs the slot. */
struct DaiRoomRecord {
    uint32_t id = 0;
    RoomKind kind = 0;
    DaiSlabCoord ax = 0, ay = 0;    /**< a slab inside it */
    int64_t index = 0;              /**< the game's room this turn; 0: lost */
    int64_t slabs = 0;              /**< as last seen */
    DaiRect box;                    /**< as last seen */
    DaiTurn since = 0;             /**< the turn it was first seen */
    DaiTurn lost = 0;              /**< the turn it went; 0 while it stands */
};

/** The keeper's creatures this turn, counted (imps apart), with its lairs and hatcheries (04 sections 7.1-7.2). */
struct DaiCreatureSummary {
    int64_t creatures = 0;
    std::array<int64_t, DAI_MAX_KINDS> by_kind{};
    std::array<int64_t, (size_t)DaiAngerReason::Count> angry{};     /**< angry or livid, by reason */
    int64_t livid = 0;
    std::array<int64_t, (size_t)DaiActivity::Count> doing{};       /**< by what they are doing */
    int64_t wounded = 0;            /**< below their kind's heal requirement */
    int64_t wages = 0;              /**< their pay summed (a pay day's bill) */
    int64_t lair_capacity = 0, lair_used = 0;   /**< in lair-size units; used counts the homeless creatures' lair sizes */
    int64_t food = 0, food_capacity = 0;        /**< chickens in its hatcheries, and room for them */
    int64_t LairFree() const { return lair_capacity - lair_used; }
};

class DaiKnowledge {
public:
    /** Sizes the map; every slab Unknown, the scan at the top-left. */
    void Init(DaiSlabCoord w, DaiSlabCoord h);
    /** One turn: own state, the slabs the facts name, `budget.slab_scan` slabs of the rolling scan (decremented by
     *  what it read), sightings of other keepers in the rows it read; then the vein table if a valuable slab came or
     *  went, and `budget.reach_visits` slab visits of the reach map's flood. A map of another size starts over. */
    void Update(IDaiWorldReader &world, PlayerNumber p, const KeeperFact *facts, int64_t nfacts, DaiTurn now, DaiBudget &budget);
    /** Update's two halves: what it reads from the world, then what it works out from that (the vein table and the
     *  reach map's flood), which reads nothing (12 section 13: the profile times them apart). */
    void Read(IDaiWorldReader &world, PlayerNumber p, const KeeperFact *facts, int64_t nfacts, DaiTurn now, DaiBudget &budget);
    void Derive(PlayerNumber p, DaiBudget &budget);
    /** Re-read these slabs at the next update (outside the scan budget). */
    void MarkDirty(const DaiRect &r);

    DaiSlabInfo SlabAt(DaiSlabCoord x, DaiSlabCoord y) const;
    /** The step from one slab onto the next (4 sides) climbs two: onto a guard post -- anyone's -- from water. No
     *  creature moved by route takes it, a flyer neither (P5.14; a bridge is at floor height, lava is never walked). */
    bool RaisedStep(DaiSlabCoord fx, DaiSlabCoord fy, DaiSlabCoord tx, DaiSlabCoord ty) const;
    /** The keeper's perimeter (plan 13 section 32; DaiPerimeter sets it each planning turn, not saved here): the
     *  slabs its imps never dig -- the dig search's costs and the room layout read Kept -- the gates once it is open,
     *  its own ways out, where Fortify puts a door, and the lining. */
    void SetPerimeter(const std::vector<DaiSlabXY> &kept, const std::vector<DaiSlabXY> &gates, const std::vector<DaiSlabXY> &lining)
    {
        m_kept = kept;
        m_gates = gates;
        m_lining = lining;
    }
    bool Kept(DaiSlabCoord x, DaiSlabCoord y) const
    {
        for (const DaiSlabXY &s : m_kept) if ((s.x == x) && (s.y == y)) return true;
        if (!m_human_kept.empty() && (x >= 0) && (y >= 0) && (x < m_w) && (y < m_h))
            return m_human_kept[(size_t)y * (size_t)m_w + (size_t)x] != 0;
        return false;
    }
    /** The slabs the human unmarked for digging (the assistant, P7.12; DaiKeeper sets them each turn from its leases,
     *  not saved here): kept as the perimeter's are -- the dig search and the room layout keep off them -- but no
     *  manager unmarks or fortifies them (KeptSlabs is the perimeter's alone). */
    void SetHumanKept(const std::vector<DaiSlabXY> &kept);
    const std::vector<DaiSlabXY> &Gates() const { return m_gates; }
    const std::vector<DaiSlabXY> &KeptSlabs() const { return m_kept; }
    /** The slabs just inside the perimeter's gaps: Defence has them dug and claimed, so its imps fortify the gaps. */
    const std::vector<DaiSlabXY> &Lining() const { return m_lining; }
    /** Diggable as the keeper knows it: earth, gold, dense gold, gems (never unknown or rock). */
    bool IsDiggable(DaiSlabCoord x, DaiSlabCoord y) const;
    /** Diggable with a reached slab beside it: where the keeper's imps can start (03 section 5). */
    bool DiggableFromReach(DaiSlabCoord x, DaiSlabCoord y) const { return IsDiggable(x, y) && m_reach.NextToReached(x, y); }
    const DaiReachMap &Reach() const { return m_reach; }
    /** The gold left in a gold or dense gold slab as last read (0 for anything else). */
    int64_t GoldLeft(DaiSlabCoord x, DaiSlabCoord y) const;
    /** The vein a slab is in (nullptr: none, or the table was full). */
    const DaiVein *VeinAt(DaiSlabCoord x, DaiSlabCoord y) const;
    const DaiVein *Vein(uint32_t id) const { return m_veins.Find(id); }
    /** Excavation's handle on a vein's status and route (nothing else of a vein is its to change). The vein is noted as
     *  edited: a planning turn hands its edits back to the main thread's knowledge (12 section 13.5). */
    DaiVein *VeinForUpdate(uint32_t id)
    {
        DaiVein *v = m_veins.Find(id);
        if ((v != nullptr) && (std::find(m_vein_edits.begin(), m_vein_edits.end(), id) == m_vein_edits.end())) m_vein_edits.push_back(id);
        return v;
    }
    /** The planning fields of the veins edited since the last call, as they stand now; the list starts again. */
    std::vector<DaiVeinEdit> TakeVeinEdits();
    /** Puts a planning turn's vein edits into this knowledge (a vein no longer in the table is left). */
    void ApplyVeinEdit(const DaiVeinEdit &e);
    template <typename F> void ForEachVein(F f) const { m_veins.ForEach(f); }
    int64_t VeinCount() const { return m_veins.Count(); }
    /** The slabs whose terrain, owner or reveal changed after sequence number `since` (a search that explored them
     *  must start over, 03 section 7.2): f(x, y) for each, oldest first. False when the log no longer reaches back to
     *  `since` (more changes than it keeps): treat everything as changed. */
    template <typename F> bool ChangesSince(uint64_t since, F f) const
    {
        if (m_change_seq - since > (uint64_t)m_change_log.size()) return false;
        for (uint64_t s = since; s < m_change_seq; s++) {
            const int32_t at = m_change_log[(size_t)(s % m_change_log.size())];
            f((DaiSlabCoord)(at % m_w), (DaiSlabCoord)(at / m_w));
        }
        return true;
    }
    /** The sequence number of the next change: a reader keeps it and asks ChangesSince(it) later. */
    uint64_t ChangeSeq() const { return m_change_seq; }
    /** Times the vein table was rebuilt (profile counter). */
    int64_t VeinRebuilds() const { return m_vein_rebuilds; }
    /** One of Daimon's rooms this turn: available to the keeper, and its price a slab. */
    const DaiBuildLook &Build(DaiRoom d) const { return m_build[(size_t)d]; }
    /** The game's room kind for one of Daimon's rooms (as DaiSlabInfo::room holds it; 0 if the game has none). */
    RoomKind RoomKindOf(DaiRoom d) const { return m_room_kind[(size_t)d]; }
    /** The room slabs connected (4 sides) to (x, y) that are of its kind and owner, in breadth-first order from it; at
     *  most `max`. A neutral entrance or room as one target. */
    int64_t RoomCluster(DaiSlabCoord x, DaiSlabCoord y, DaiSlabXY *out, int64_t max) const;
    /** The keeper's own dig marks this turn (exact: read every update). */
    int64_t OwnMarkCount() const { return (int64_t)m_marks.size(); }
    /** The keeper's own imps this turn (at most DAI_MAX_OWN_IMPS). */
    std::span<const DaiImpLook> OwnImps() const { return std::span<const DaiImpLook>(m_imps.data(), (size_t)m_nimps); }
    /** The special boxes its hand can reach, and its dead a Resurrect box could bring back (P9). */
    std::span<const DaiSpecialLook> Specials() const { return std::span<const DaiSpecialLook>(m_specials.data(), (size_t)m_nspecials); }
    std::span<const DaiDeadLook> Dead() const { return std::span<const DaiDeadLook>(m_dead.data(), (size_t)m_ndead); }
    const DaiBridgeRoles &BridgeRoles() const { return m_bridge; }
    DaiSlabCoord Width() const { return m_w; }
    DaiSlabCoord Height() const { return m_h; }
    const DaiOwnLook &Own() const { return m_own; }
    /** One of Daimon's powers this turn: available to the keeper, and its price uncharged. */
    const DaiPowerLook &Power(DaiPower d) const { return m_powers[(size_t)d]; }
    /** A map pack's own power (Phase 7: one of the catalogue's, not KeeperFX's): available and its price this turn;
     *  an empty look for any other kind. */
    DaiPowerLook OwnPower(int64_t kind) const
    {
        for (const auto &e : m_own_powers) if (e.first == kind) return e.second;
        return DaiPowerLook();
    }
    std::span<const DaiRoomLook> Rooms() const { return std::span<const DaiRoomLook>(m_rooms.data(), (size_t)m_nrooms); }
    /** Its rooms by role (14 K2): any with one of `roles`; the first such (nullptr if none); their slabs; whether a
     *  creature stands in such a room. */
    bool HasRoomRole(uint64_t roles) const
    {
        for (const DaiRoomLook &r : Rooms()) if ((r.roles & roles) != 0) return true;
        return false;
    }
    const DaiRoomLook *FirstRoomWith(uint64_t roles) const
    {
        for (const DaiRoomLook &r : Rooms()) if ((r.roles & roles) != 0) return &r;
        return nullptr;
    }
    int64_t SlabsWithRole(uint64_t roles) const
    {
        int64_t n = 0;
        for (const DaiRoomLook &r : Rooms()) if ((r.roles & roles) != 0) n += r.slabs;
        return n;
    }
    bool InRoomWith(const DaiCreatureLook &cr, uint64_t roles) const
    {
        for (const DaiRoomLook &r : Rooms()) if ((r.kind == cr.in_room) && ((r.roles & roles) != 0)) return true;
        return false;
    }
    /** The room look of the game's room `index` this turn; nullptr if the keeper has none. */
    const DaiRoomLook *RoomByIndex(int64_t index) const;
    /** Daimon's identities of the keeper's rooms, standing and lost (04 section 2.3). */
    template <typename F> void ForEachRoomRecord(F f) const { m_room_records.ForEach(f); }
    const DaiRoomRecord *RoomRecord(uint32_t id) const { return m_room_records.Find(id); }
    /** The record of the game's room `index` this turn; nullptr if none. */
    const DaiRoomRecord *RoomRecordOf(int64_t index) const;
    /** The keeper's own creatures this turn, imps apart (at most DAI_MAX_OWN_CREATURES). */
    std::span<const DaiCreatureLook> Creatures() const { return std::span<const DaiCreatureLook>(m_creatures.data(), (size_t)m_ncreatures); }
    /** One of its creatures by its reference (index and creation turn), or by index; nullptr if none (14 K3). */
    const DaiCreatureLook *Creature(const DaiThingRef &r) const
    {
        for (const DaiCreatureLook &c : Creatures()) if (c.ref == r) return &c;
        return nullptr;
    }
    const DaiCreatureLook *CreatureByIdx(DaiThingIndex idx) const
    {
        for (const DaiCreatureLook &c : Creatures()) if (c.ref.idx == idx) return &c;
        return nullptr;
    }
    const DaiCreatureSummary &CreatureSummary() const { return m_summary; }
    /** A creature kind's rules and what the keeper may have of it (an empty look for an unknown kind). */
    const DaiKindLook &Kind(DaiThingModel m) const;
    int64_t KindCount() const { return m_nkinds; }
    const DaiResearchLook &Research() const { return m_research; }
    /** A slab of the keeper's libraries holding one of its spellbooks (this turn). */
    bool BookAt(DaiSlabCoord x, DaiSlabCoord y) const;
    /** The keeper's mature chickens this turn (at most DAI_MAX_OWN_FOOD). */
    std::span<const DaiFoodLook> Food() const { return std::span<const DaiFoodLook>(m_food.data(), (size_t)m_nfood); }
    /** Phase 4 (06 section 3): the battles its creatures fight in, its traps and doors, and the enemy creatures it sees
     *  anywhere (rival keepers', heroes', neutral), this update. */
    std::span<const DaiBattleLook> Battles() const { return std::span<const DaiBattleLook>(m_battles.data(), (size_t)m_nbattles); }
    std::span<const DaiTrapLook> Traps() const { return std::span<const DaiTrapLook>(m_traps.data(), (size_t)m_ntraps); }
    std::span<const DaiDoorLook> Doors() const { return std::span<const DaiDoorLook>(m_doors.data(), (size_t)m_ndoors); }
    std::span<const DaiThingLook> Enemies() const { return std::span<const DaiThingLook>(m_enemies.data(), (size_t)m_nenemies); }
    /** The hero gates the keeper has seen (read every DAI_HERO_GATE_READ turns; unsaved, read again after a load). */
    std::span<const DaiSlabXY> HeroGates() const { return std::span<const DaiSlabXY>(m_hero_gates.data(), (size_t)m_nhero_gates); }
    /** The keeper's own door on slab (x, y) this update; nullptr if none. */
    const DaiDoorLook *DoorAt(DaiSlabCoord x, DaiSlabCoord y) const;
    /** Phase 5 (07 sections 3.1, 7): its Sight of Evil this update; the other keepers' (and heroes') traps and doors
     *  and rooms it has seen (read every DAI_ENEMY_DEFENCE_READ / DAI_RIVAL_ROOM_READ turns, saved); the one on a slab. */
    const DaiSightLook &Sight() const { return m_sight; }
    std::span<const DaiEnemyDefence> EnemyDefences() const { return std::span<const DaiEnemyDefence>(m_enemy_defences); }
    std::span<const DaiRivalRoom> RivalRooms() const { return std::span<const DaiRivalRoom>(m_rival_rooms); }
    const DaiEnemyDefence *EnemyDefenceAt(DaiSlabCoord x, DaiSlabCoord y) const;
    const DaiStockLook &Stock() const { return m_stock; }
    /** Every power, trap and door kind of the level with its role (Phase 7, 09 section 13.3): built at the first update
     *  (and after a load), logged then. */
    const DaiContent &Content() const { return m_content; }
    /** What the keeper's human is doing now (Daimon P7.10): read every update; empty for a keeper no human plays. */
    const DaiLiveExclusion &LiveExclusion() const { return m_live; }
    /** The human's combat-assistant rules (P7.12), re-read every update. */
    const DaiHumanFightRules &HumanFightRules() const { return m_human_rules; }
    /** The keeper's custody and special rooms, and the temple recipes in the table's order (Phase 6). */
    const DaiCustodyLook &Custody() const { return m_custody; }
    std::span<const DaiRecipeLook> Recipes() const { return std::span<const DaiRecipeLook>(m_recipes); }
    const DaiRivalSighting &Sighting(PlayerNumber r) const;
    /** The rival's force as last counted in a full scan, decayed by age (halved each DAI_FORCE_HALF_LIFE turns), or,
     *  if more, its creature count on the query panel (a human's view: imps not counted, P9) at the mean level of its
     *  creatures in that scan (none seen: the keeper's own mean): the one raw estimate DaiRivals, DaiThreatModel and
     *  every other reader build on (02 section 4.13c). 0 if never seen and none counted. */
    int64_t KnownForce(PlayerNumber r, DaiTurn now) const;
    /** (x, y) lies within `radius` slabs of keeper `p`'s heart or of one of its rooms, bridges apart: the ground a fight
     *  counts as at home on (P5.12: the director's Defend and the offence's home fight read it alike). */
    bool NearHome(PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y, int64_t radius) const;
    /** Whose knowledge it is. */
    PlayerNumber Player() const { return m_player; }
    /** `r` is another keeper in the game and not an ally (the reader's RivalKeeper, read each update). */
    bool RivalKeeper(PlayerNumber r) const { return (r >= 0) && (r < DAI_MAX_PLAYERS) && m_rival_keeper[(size_t)r]; }
    /** `r` on the keeper's alliance panel (07 section 5a; read each update). */
    const DaiAllianceLook &Alliance(PlayerNumber r) const
    {
        static const DaiAllianceLook none;
        return ((r >= 0) && (r < DAI_MAX_PLAYERS)) ? m_alliance[(size_t)r] : none;
    }

    /** Slabs the last update's rolling scan read (profile counter, 02 section 8). */
    int64_t LastScanned() const { return m_last_scanned; }
    /** Slabs re-read because a fact named them, last update. */
    int64_t LastFactReads() const { return m_last_fact_reads; }
    /** Full scans of the map completed. */
    int64_t ScanCycles() const { return m_cycles; }
    DaiTurn BuiltTurn() const { return m_built_turn; }
    /** What a saved game keeps (P0.7): the map, the scan's place, the dirty slabs, the rivals, the gold, the veins,
     *  the reach map and its flood in progress, and the keeper's own state as last read (the next update reads it
     *  again, but a view right after a load shows it). */
    void Save(DaiBlobWriter &w) const;
    bool Load(DaiBlobReader &r);
private:
    void read_slab(IDaiWorldReader &world, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y, DaiTurn now);
    void note_things(IDaiWorldReader &world, PlayerNumber p, const DaiRect &rows, DaiTurn now);
    void end_cycle(DaiTurn now);
    DaiReachMap::Walk walk(int64_t at, PlayerNumber p) const;
    bool post_word(uint32_t wd) const;
    bool raised_step(int64_t from, int64_t to) const;
    void rebuild_veins();
    void read_marks(IDaiWorldReader &world, PlayerNumber p);
    void read_creatures(IDaiWorldReader &world, PlayerNumber p);
    void match_rooms(IDaiWorldReader &world, PlayerNumber p, DaiTurn now);
    void summarise();
    uint32_t &word(DaiSlabCoord x, DaiSlabCoord y) { return m_slab[(size_t)y * (size_t)m_w + (size_t)x]; }
    uint32_t word(DaiSlabCoord x, DaiSlabCoord y) const { return m_slab[(size_t)y * (size_t)m_w + (size_t)x]; }

    std::vector<uint32_t> m_slab;
    DaiSlabCoord m_w = 0, m_h = 0;
    DaiSlabCoord m_scan_x = 0, m_scan_y = 0;
    DaiOwnLook m_own;
    std::array<DaiPowerLook, (size_t)DaiPower::Count> m_powers{};
    std::array<DaiRoomLook, 256> m_rooms{};
    int64_t m_nrooms = 0;
    DaiPool<DaiRoomRecord, DAI_MAX_ROOM_RECORDS> m_room_records;
    std::array<DaiCreatureLook, DAI_MAX_OWN_CREATURES> m_creatures{};  /**< re-read every update (not saved) */
    int64_t m_ncreatures = 0;
    std::array<DaiKindLook, DAI_MAX_KINDS> m_kinds{};                  /**< re-read every update (not saved) */
    int64_t m_nkinds = 0;
    DaiResearchLook m_research;                                         /**< re-read every update (not saved) */
    std::array<DaiFoodLook, DAI_MAX_OWN_FOOD> m_food{};                 /**< re-read every update (not saved) */
    std::array<DaiSlabXY, DAI_MAX_OWN_BOOK_SLABS> m_books{};            /**< re-read every update (not saved) */
    int64_t m_nbooks = 0;
    int64_t m_nfood = 0;
    std::array<DaiBattleLook, DAI_MAX_BATTLES> m_battles{};             /**< re-read every update (not saved), as below */
    int64_t m_nbattles = 0;
    std::array<DaiTrapLook, DAI_MAX_OWN_TRAPS> m_traps{};
    int64_t m_ntraps = 0;
    std::array<DaiDoorLook, DAI_MAX_OWN_DOORS> m_doors{};
    int64_t m_ndoors = 0;
    std::array<DaiSlabXY, DAI_MAX_HERO_GATES> m_hero_gates{};
    int64_t m_nhero_gates = 0;
    DaiTurn m_hero_gates_turn = 0;     /**< when they were last read (0: not since the keeper was built or loaded) */
    std::array<DaiThingLook, DAI_MAX_ENEMIES> m_enemies{};
    int64_t m_nenemies = 0;
    // Phase 5: the sight (every update, not saved); what it has seen of other keepers' defences and rooms (saved).
    DaiSightLook m_sight;
    std::vector<DaiEnemyDefence> m_enemy_defences;
    std::vector<DaiRivalRoom> m_rival_rooms;
    DaiTurn m_enemy_defences_turn = 0, m_rival_rooms_turn = 0;
    /** The slab is in view now: revealed (it stays so), or in the keeper's Sight of Evil. */
    bool in_view(IDaiWorldReader &world, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y) const;
    void read_rivals(IDaiWorldReader &world, PlayerNumber p, DaiTurn now);
    DaiStockLook m_stock;                                               /**< re-read every update (not saved) */
    DaiContent m_content;                                               /**< built once (not saved) */
    DaiHumanFightRules m_human_rules;                                   /**< re-read every update (not saved) */
    DaiLiveExclusion m_live;                                            /**< re-read every update (not saved) */
    std::vector<std::pair<int64_t, DaiPowerLook>> m_own_powers;         /**< the pack's own powers: re-read every update */
    DaiCustodyLook m_custody;                                           /**< re-read every update (not saved) */
    std::vector<DaiRecipeLook> m_recipes;                               /**< re-read every update (not saved) */
    DaiCreatureSummary m_summary;                                       /**< from the above, every update */
    std::array<DaiRivalSighting, DAI_MAX_PLAYERS> m_rivals{};
    std::array<bool, DAI_MAX_PLAYERS> m_rival_keeper{};
    std::array<DaiAllianceLook, DAI_MAX_PLAYERS> m_alliance{};
    std::vector<DaiSlabXY> m_dirty;
    std::vector<int32_t> m_gold;        /**< gold left, a slab */
    std::vector<uint8_t> m_slab_vein;   /**< the vein's pool slot + 1, a slab; 0 none */
    DaiPool<DaiVein, DAI_MAX_VEINS> m_veins;
    std::vector<uint32_t> m_vein_edits;     /**< veins planning edited since TakeVeinEdits (not saved) */
    std::vector<uint8_t> m_human_kept;      /**< a w by h grid, empty while there are none */
    std::vector<DaiSlabXY> m_kept, m_gates, m_lining;  /**< the perimeter's (not saved: set each planning turn) */
    bool m_veins_stale = false;         /**< a valuable slab came or went: rebuilt at the end of the update */
    int64_t m_vein_rebuilds = 0;
    DaiReachMap m_reach;
    PlayerNumber m_player = -1;         /**< whose knowledge it is (the reach map's own ground) */
    std::array<DaiBuildLook, (size_t)DaiRoom::Count> m_build{};
    std::array<RoomKind, (size_t)DaiRoom::Count> m_room_kind{};
    std::array<DaiImpLook, DAI_MAX_OWN_IMPS> m_imps{};  /**< re-read every update (not saved) */
    int64_t m_nimps = 0;
    std::array<DaiSpecialLook, 32> m_specials{};        /**< re-read every update (not saved) */
    int64_t m_nspecials = 0;
    std::array<DaiDeadLook, 64> m_dead{};
    int64_t m_ndead = 0;
    std::vector<int32_t> m_marks;       /**< own marks (slab indexes) as last read: a mark that goes was dug or cleared */
    std::array<int32_t, 1024> m_change_log{};   /**< slab indexes, by sequence number modulo the size (saved: a search
                                                     *   saved part-way reads on from where it was) */
    uint64_t m_change_seq = 0;
    DaiBridgeRoles m_bridge;
    int64_t m_last_scanned = 0, m_last_fact_reads = 0, m_cycles = 0;
    DaiTurn m_built_turn = 0;
};

/** Turns over which a rival's counted force halves in KnownForce. */
constexpr int64_t DAI_FORCE_HALF_LIFE = 2000;

#endif
