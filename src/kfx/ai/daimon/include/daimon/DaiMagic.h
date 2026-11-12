/** @file DaiMagic.h
 *  DaiMagic (02 C2, 07 sections 4.2, 7): the keeper's powers cast outside fights, on another manager's request (Offence's
 *  from P5.5) -- a Sight of Evil peek at a slab, Destroy Walls on a revealed wall. Each request is checked when served:
 *  the power available and castable there (the game's own read-only check), its price at the level asked; then a
 *  magic_cast job at the slab's centre, its gold claimed by the arbiter when it runs. A request that cannot be served,
 *  or whose cast fails, is marked refused for the asker to see. Fights' casts are Tactics' (06 section 7). It counts the
 *  Destroy Walls casts made and their gold (the tournament's measure, 07 section 13).
 *  Must Obey (P9) is its own: the whole keeper's, for its fights and its work. On while obey_fighters of its creatures
 *  fight, or (fights_and_work) obey_workers of them research, manufacture or train, with obey_reserve gold after the
 *  next wages; off when neither holds or the gold is under obey_floor; at most one switch every obey_hold turns, but
 *  for the floor. Its price while on is in the forecast's upkeep.
 *  Special boxes (P9, the personality's specials): one its hand can reach used at once -- reveal map, steal hero,
 *  multiply creatures, increase level, make safe, gold; heal all once heal_all_hurt of its creatures are under
 *  heal_all_below % health; resurrect on its best dead (its kind's score at its level). Never transfer creature or
 *  locate hidden world (a campaign's own choices: Phase 8's lieutenant may ask for them), nor the harmful or a level
 *  script's own. A use refused is not tried again for SPECIAL_RETRY turns.
 *  Armageddon (P9, the personality's armageddon "finisher"): looked at every ARMAGEDDON_LOOK turns, cast when
 *  ArmageddonReady. Under any keeper's Armageddon Must Obey goes on whatever the gold, Tactics' spells keep nothing
 *  back (DaiFightRules' final), and the forecast keeps every gold piece for them. */
#ifndef DAIMON_DAIMAGIC_H
#define DAIMON_DAIMAGIC_H

#include "daimon_base_types.h"
#include "daimon/DaiRivals.h"
#include "daimon/IDaiManager.h"

class DaiMagic : public IDaiManager {
public:
    const char *GetName() const override { return "magic"; }
    DaiDomain GetDomain() const override { return DaiDomain::Magic; }
    void Plan(const DaiTurnCtx &c, DaiManagerWrites &w) override;
    void OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w) override;
    void Reset() override
    {
        m_walls_cast = m_walls_gold = 0;
        m_obey_switched = 0; m_obey_turns = 0; m_obey_since = 0;
        m_armageddon_cast = m_armageddon_next = 0;
    }
    void Save(DaiBlobWriter &w) const override
    {
        w.I64(m_walls_cast); w.I64(m_walls_gold); w.U64(m_obey_switched); w.I64(m_obey_turns); w.U64(m_obey_since);
        w.U64(m_armageddon_cast); w.U64(m_armageddon_next);
    }
    bool Load(DaiBlobReader &r) override
    {
        m_walls_cast = r.Range(0, INT64_MAX);
        m_walls_gold = r.Range(0, INT64_MAX);
        m_obey_switched = (DaiTurn)r.U64();
        m_obey_turns = r.Range(0, INT64_MAX);
        m_obey_since = (DaiTurn)r.U64();
        m_armageddon_cast = (DaiTurn)r.U64();
        m_armageddon_next = (DaiTurn)r.U64();
        return r.Ok();
    }
    int64_t WallsCast() const { return m_walls_cast; }
    int64_t WallsGold() const { return m_walls_gold; }
    /** Turns Must Obey has been on (the tournament's measure), counted when it goes off. */
    int64_t ObeyTurns() const { return m_obey_turns; }
    /** The power a service casts (DaiPower::Count: none). */
    static DaiPower PowerFor(DaiService s);
    /** Job keys: the request's id under this tag. */
    static constexpr uint64_t KEY_REQUEST = 1ull << 40;
    /** Must Obey's switch job; a special box's use. */
    static constexpr uint64_t KEY_OBEY = 1, KEY_SPECIAL = 2, KEY_ARMAGEDDON = 3, KEY_WORK_SPEED = 4;
    static constexpr DaiTurn ARMAGEDDON_LOOK = 200;
    static constexpr int64_t ARMAGEDDON_TRAP_REACH = 5;     /**< slabs from the heart (DAI_HEART_TRAP_REACH: the same) */
    /** Whether the keeper would cast Armageddon now against rivals_force, and if not why (pure: tests). */
    static bool ArmageddonReady(const DaiKnowledge &k, const DaiPersonality &p, const DaiForecastOut &f, int64_t rivals_force, const char **why);
    /** Every rival keeper's estimated force together, in levels: KnownForce (the query panel's creature count at the
     *  levels seen) and DaiRivals' growth guess; -1 with no rival. */
    static int64_t RivalsForce(const DaiKnowledge &k, const DaiRivals &rivals, PlayerNumber me, DaiTurn now);
    DaiTurn ArmageddonCast() const { return m_armageddon_cast; }
    static constexpr DaiTurn SPECIAL_RETRY = 200;
    /** Whether a box of this kind is used now, and for Resurrect whom (pure: tests). */
    static bool SpecialWanted(const DaiKnowledge &k, const DaiPersonality &p, DaiSpecial kind, DaiSpecialParams *sp);
private:
    void obey(const DaiTurnCtx &c, DaiManagerWrites &w);
    void specials(const DaiTurnCtx &c, DaiManagerWrites &w);
    void armageddon(const DaiTurnCtx &c, DaiManagerWrites &w);
    void work_speed(const DaiTurnCtx &c, DaiManagerWrites &w);
    DaiTurn m_armageddon_cast = 0, m_armageddon_next = 0;
    DaiTurn m_special_retry = 0;   /**< after a refused use (not saved: a load tries again) */
    const char *m_armageddon_why = nullptr;    /**< the last reason not to cast, for the log (not saved) */
    int64_t m_walls_cast = 0, m_walls_gold = 0;
    DaiTurn m_obey_switched = 0;   /**< when it last switched Must Obey (0: never) */
    int64_t m_obey_turns = 0;
    DaiTurn m_obey_since = 0;      /**< on since (0: off as it last saw it) */
};

#endif
