/** @file DaiWorldReaderSim.h
 *  IDaiWorldReader over the simulation: the only Daimon class that calls simulation functions (02 principle 1). Only
 *  functions with no side effects on the simulation or the saved navigation state (principle 8; the known traps are
 *  listed in DaiWorldReaderSim.cpp); the fog through keeper_visibility.h.
 *
 *  The fog audit (01 section 7): when switched on, every datum about another keeper the reader returns is checked
 *  again against the game's visibility rules, and each one the keeper could not have seen is counted. Tests and the
 *  dai_knowledge_readonly ftest switch it on and require zero. */
#ifndef DAIMON_DAIWORLDREADERSIM_H
#define DAIMON_DAIWORLDREADERSIM_H

#include <array>
#include <memory>

#include "daimon/DaiLimits.h"
#include "daimon/IDaiWorldReader.h"

struct KeeperFactsTracker;

class DaiWorldReaderSim : public IDaiWorldReader {
public:
    DaiWorldReaderSim();
    ~DaiWorldReaderSim() override;

    void MapSize(DaiSlabCoord *w, DaiSlabCoord *h) const override;
    void Slab(PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y, DaiSlabLook *out) const override;
    void Own(PlayerNumber p, DaiOwnLook *out) const override;
    int64_t OwnRooms(PlayerNumber p, DaiRoomLook *out, int64_t max) const override;
    int64_t OwnRoomAt(PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y) const override;
    int64_t OwnCreatures(PlayerNumber p, DaiCreatureLook *out, int64_t max, int64_t from) const override;
    int64_t KindCount() const override;
    void Kind(PlayerNumber p, DaiThingModel m, DaiKindLook *out) const override;
    void Research(PlayerNumber p, DaiResearchLook *out) const override;
    int64_t OwnFood(PlayerNumber p, DaiFoodLook *out, int64_t max) const override;
    int64_t OwnBookSlabs(PlayerNumber p, DaiSlabXY *out, int64_t max) const override;
    void Stock(PlayerNumber p, DaiStockLook *out) const override;
    uint64_t PredictHandDropJob(PlayerNumber p, const DaiThingRef &c, DaiSlabCoord x, DaiSlabCoord y) const override;
    int64_t VisibleThings(PlayerNumber p, const DaiRect &area, DaiThingLook *out, int64_t max) const override;
    DaiPowerKind PowerOf(DaiPower d) const override;
    int64_t PowerKindCount() const override;
    void ContentSpecs(std::vector<DaiPowerSpec> *powers, std::vector<DaiTrapSpec> *traps,
                      std::vector<DaiDoorSpec> *doors) const override;
    void LiveExclusion(PlayerNumber p, DaiLiveExclusion *out) const override;
    void HumanFightRules(PlayerNumber p, DaiHumanFightRules *out) const override;
    bool PowerAvailable(PlayerNumber p, DaiPowerKind k) const override;
    int64_t PowerPrice(PlayerNumber p, DaiPowerKind k, int64_t level) const override;
    RoomKind RoomOf(DaiRoom d) const override;
    int64_t SpecialBoxes(PlayerNumber p, DaiSpecialLook *out, int64_t max) const override;
    int64_t DeadCreatures(PlayerNumber p, DaiDeadLook *out, int64_t max) const override;
    int64_t OwnImps(PlayerNumber p, DaiImpLook *out, int64_t max) const override;
    int64_t OwnMarks(PlayerNumber p, DaiSlabXY *out, int64_t max) const override;
    DaiBridgeRoles BridgeRoles() const override;
    int64_t Battles(PlayerNumber p, DaiBattleLook *out, int64_t max) const override;
    int64_t OwnTraps(PlayerNumber p, DaiTrapLook *out, int64_t max) const override;
    int64_t OwnDoors(PlayerNumber p, DaiDoorLook *out, int64_t max) const override;
    bool PowerCastable(PlayerNumber p, DaiPowerKind k, DaiSubtlCoord x, DaiSubtlCoord y, const DaiThingRef *target) const override;
    bool PowerTakes(PlayerNumber p, DaiPowerKind k, const DaiThingRef &target) const override;
    int64_t HeroGates(PlayerNumber p, DaiSlabXY *out, int64_t max) const override;
    bool SightOfEvil(PlayerNumber p, DaiSightLook *out) const override;
    bool SightShows(PlayerNumber p, DaiSubtlCoord x, DaiSubtlCoord y) const override;
    int64_t EnemyDefences(PlayerNumber p, DaiEnemyDefenceLook *out, int64_t max) const override;
    int64_t RivalRooms(PlayerNumber p, DaiRivalRoomLook *out, int64_t max) const override;
    int64_t PowerAreaRange(DaiPowerKind k, int64_t level, bool *own_too) const override;
    bool RoomAvailable(PlayerNumber p, RoomKind k) const override;
    bool RivalKeeper(PlayerNumber p, PlayerNumber r) const override;
    void Alliance(PlayerNumber p, PlayerNumber r, DaiAllianceLook *out) const override;
    void Custody(PlayerNumber p, DaiCustodyLook *out) const override;
    int64_t Recipes(DaiRecipeLook *out, int64_t max) const override;
    void SetSeesAll(PlayerNumber p, bool on) override;
    void SetKnowsPool(PlayerNumber p, bool on) override;
    int64_t RoomPrice(PlayerNumber p, RoomKind k) const override;
    int64_t Facts(PlayerNumber p, DaiTurn now, KeeperFact *out, int64_t max) override;
    void Rebaseline(PlayerNumber p) override;
    /** Drops every keeper's tracker (a new level). */
    void Reset();

    struct FogAudit {
        int64_t checked = 0;    /**< data about other keepers returned while the audit was on */
        int64_t hidden = 0;     /**< ... that the keeper could not have seen */
    };
    void SetFogAudit(bool on) { m_audit_on = on; }
    const FogAudit &GetFogAudit() const { return m_audit; }
    void ClearFogAudit() { m_audit = FogAudit(); }
private:
    /** The keeper's dig marks as a slab bitmap, built once per game turn. */
    bool marked(PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y) const;
    void audit(bool visible) const;

    std::array<std::unique_ptr<KeeperFactsTracker>, DAI_MAX_PLAYERS> m_trackers;
    mutable std::array<std::unique_ptr<uint8_t[]>, DAI_MAX_PLAYERS> m_marks;
    mutable std::array<DaiTurn, DAI_MAX_PLAYERS> m_marks_turn{};
    mutable std::array<int64_t, DAI_MAX_PLAYERS> m_marks_size{};
    bool m_audit_on = false;
    mutable FogAudit m_audit;
    std::array<bool, DAI_MAX_PLAYERS> m_sees_all{}, m_knows_pool{};
    bool sees_all(PlayerNumber p) const { return (p >= 0) && (p < DAI_MAX_PLAYERS) && m_sees_all[(size_t)p]; }
    /** What the keeper sees of another's creature: as a human (keeper_creature_visible_to), or in its Sight of Evil
     *  (not an invisible enemy); and a thing's place (a heart): revealed, or in its sight. */
    bool creature_seen(const struct Thing *th, PlayerNumber p) const;
    bool place_seen(const struct Thing *th, PlayerNumber p) const;
};

#endif
