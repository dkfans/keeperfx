/** @file DaiCreatures.h
 *  Creatures (04 section 7): keeps the keeper's creatures and puts them to work, by hand, a trip at a time.
 *    - Trouble: a livid creature whose anger is not put right within [creatures] livid_grace turns is dismissed on the
 *      portal, when the personality allows it for its kind (dismiss_livid: never, harmful, always; 04 Q4) -- or, with
 *      the personality's discipline (08 section 8), jailed while its cause is put right (prison: unpaid with the pay
 *      day near and the gold there, hungry with chickens, no lair with one growing) or racked when its kind is the one
 *      the pay discount wants (torture); carried to its lair again after [custody] discipline_hold turns.
 *    - Care: the angriest unpaid creature is paid by hand when the pay day is far; a hungry one is brought a chicken;
 *      a wounded idle one far from its lair is carried there; one angry for no reason it can mend, diseased or a
 *      chicken is carried to a temple to pray (08 section 7.1). (Hatcheries and lairs big enough are the director's
 *      programme; Construction builds them.)
 *    - Work: idle creatures are dropped in the work room worth most to them -- the library while research remains,
 *      the workshop while the Manufacture goal wants workers, the training room up to the personality's level while the
 *      gold allows -- weighing the kind's skill, the room's efficiency, stress and the walk; a kind's NotDoJobs never.
 *      At most [creatures] moves_per_kturn trips each 1,000 turns: every trip is a gesture, and the hand angers.
 *    - Arrivals: KeepGold is weighed up while the gold would not pay the creatures likely to come next.
 *    - Scavenging (08 section 7a): every [creatures] scavenge_interval, each kind it has is weighed -- the weakest
 *      rival's creature of that kind in sight (its requirement against the scavengers' work, a turn at a time), else a
 *      draw from the pool while the kind is not known dry -- its worth by the personality's scavenge_appetite against
 *      the scavengers' gold over those turns, in force by its own force per gold of pay. The best kind gets up to
 *      scavengers_max of its creatures in the scavenger room (asked for when there is none); they come out when the kind
 *      is no longer worth it or the pay days run short. A pool draw tried pool_try_turns with nothing come marks the
 *      kind dry for pool_dry_turns. A creature being scavenged goes to pray (its keeper's temple protects it).
 *  One hand job at a time (the keeper has one hand). */
#ifndef DAIMON_DAICREATURES_H
#define DAIMON_DAICREATURES_H

#include "daimon_base_types.h"
#include <array>
#include <vector>

#include "daimon/IDaiManager.h"

/** Pay a creature by hand only when its pay day is further off than this, and not the same creature again sooner. */
constexpr DaiTurn DAI_PAY_LEAD_TURNS = 500, DAI_PAY_AGAIN_TURNS = 2000;
/** A wounded creature is carried to its lair only when it is further than this (slabs): it walks a shorter way. */
constexpr int64_t DAI_LAIR_FAR_SLABS = 15;
/** Creatures one trip carries to the same room, at most (the hand holds more; a trip of a few keeps them in reach). */
constexpr int64_t DAI_MOVE_PER_TRIP = 4;

class DaiCreatures : public IDaiManager {
public:
    const char *GetName() const override { return "creatures"; }
    DaiDomain GetDomain() const override { return DaiDomain::Creatures; }
    void Plan(const DaiTurnCtx &c, DaiManagerWrites &w) override;
    void OnJobEnd(const DaiTurnCtx &c, DaiJob &j, DaiManagerWrites &w) override;
    void Reset() override;
    void Save(DaiBlobWriter &w) const override;
    bool Load(DaiBlobReader &r) override;

    /** What Creatures remembers of one creature. */
    struct Note {
        DaiThingRef ref;
        DaiTurn livid_since = 0;   /**< 0: not livid */
        DaiTurn paid = 0;          /**< last paid by hand */
        DaiTurn moved = 0;         /**< last carried */
        DaiTurn need_since = 0;    /**< angry or wounded since (0: neither): the difficulty's care_delay counts from it */
        DaiTurn jailed = 0;        /**< jailed or racked for discipline (0: not) */
        DaiTurn hand_retry = 0;    /**< a discipline trip (dismissal) refused: none for it before this */
    };
    const Note *NoteOf(const DaiThingRef &r) const;
    /** What a creature's kind brings to a work room (skill, efficiency, stress, the walk); 0 or less: not worth it. */
    static int64_t WorkValue(const DaiTurnCtx &c, const DaiCreatureLook &cr, DaiWork w, const DaiRoomLook &room);
private:
    void track(const DaiTurnCtx &c);
    bool trouble(const DaiTurnCtx &c, DaiManagerWrites &w);
    bool care(const DaiTurnCtx &c, DaiManagerWrites &w);
    bool work(const DaiTurnCtx &c, DaiManagerWrites &w);
    bool train_imps(const DaiTurnCtx &c, DaiManagerWrites &w);
    bool make_room(const DaiTurnCtx &c, DaiManagerWrites &w);
    static bool gem_held(const DaiTurnCtx &c);
    void arrivals(const DaiTurnCtx &c, DaiManagerWrites &w);
    bool scavenge(const DaiTurnCtx &c, DaiManagerWrites &w);
    void weigh_scavenging(const DaiTurnCtx &c);
    Note *note(const DaiThingRef &r);

    std::vector<Note> m_notes;
    std::vector<DaiTurn> m_trips;      /**< the turns of the work trips of the last 1,000 turns */
    /** The imps carried to train lately (P9): at most IMP_TRIPS a 1,000 turns, each left there IMP_STAY turns. */
    struct ImpTrip { DaiThingRef ref; DaiTurn at = 0; };
    std::vector<ImpTrip> m_imp_trips;
    DaiTurn m_room_made = 0;           /**< when it last dismissed a creature to make room (P9) */
    // Scavenging: the kind chosen (0 none) and how many scavengers; when it was weighed; the pool draw per kind -- tried
    // since (0 not), its creatures then, dry until.
    DaiThingModel m_scav_kind = 0;
    int64_t m_scav_n = 0;
    bool m_scav_pool = false;
    DaiTurn m_scav_next = 0;
    std::array<DaiTurn, DAI_MAX_KINDS> m_pool_since{}, m_pool_dry{};
    std::array<int64_t, DAI_MAX_KINDS> m_pool_count{};
};

#endif
