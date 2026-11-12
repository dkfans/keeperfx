/** @file DaiFortify.h
 *  Where Defence puts traps and doors (06 section 4.1, 2.5), from the threat model's routes and chokepoints and the
 *  knowledge -- a pure planner (no world, no state), so its site rules are tested on their own:
 *   - only on the keeper's own claimed floor (never rooms), one thing a slab, none where one already stands;
 *   - a door on a one-wide way: closed ground on two opposite sides, or on three (a dead end);
 *   - an alarm at the outermost own slab of each route in, where it warns first;
 *   - a boulder at the inner end of a straight, one-wide run of at least DAI_BOULDER_RUN own slabs of a route, off the
 *     own-traffic map (it hits the keeper's own creatures too), rolling out along the run;
 *   - pressure traps on the chokepoints left, at least two slabs apart; poison gas (which hurts own creatures) only off
 *     the own-traffic map, Word of Power or lightning on it;
 *   - imp traps (07 P5.13): a pressure trap on own floor where rival imps keep coming to convert its ground, ranked with
 *     the route traps by the imps seen there (an imp lost costs its keeper a rising price);
 *   - landings (07 P5.14): a one-slab guard post on the first own slab a route enters from water -- no creature moved
 *     by route steps up onto one from water, so the route has to bridge or go round; ranked with the doors by the
 *     routes' weight and capped with them (one-slab posts beside water already standing count), never where the
 *     keeper's own creatures cross the water onto that slab;
 *   - moats (07 P5.14): a one-slab guard post on a chokepoint of a land route, one wide there and on the route's own
 *     slab just outward of it, with a lava trap on that slab once the post stands -- what steps on it falls into lava
 *     with only the way back, and the route has to bridge the lava from then on; a door's place made a moat while the
 *     trap cap has room, never where the keeper's own creatures walk out over the trap's slab (the lava stays);
 *   - heart traps (P9, the owner's last-ditch defence): defence heart_traps traps on its own floor nearest the heart
 *     (within DAI_HEART_TRAP_REACH slabs, 5, two slabs apart), of the best-rated kind it has (DaiTrapRating) but a
 *     boulder, an alarm or lava -- counted apart from the cap, and a heart's own traps already standing count;
 *  the busiest chokepoints first, as many as the personality's share of [defence] max_traps / max_doors;
 *   - a door on each of the perimeter's gates (plan 13 section 32, DaiKnowledge::Gates), first and outside the cap. What the plan
 *  names is a role; the kind is the stock's (DaiFortKind), by who comes (P5.13: no poison gas against a force mostly
 *  immune to it, no boulder where a creature that breaks boulders comes). In contest the rival's routes come first
 *  (06 section 11). */
#ifndef DAIMON_DAIFORTIFY_H
#define DAIMON_DAIFORTIFY_H

#include "daimon_base_types.h"
#include <cstdint>
#include <vector>

#include "config_daimon.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiPersonality.h"
#include "daimon/DaiThreatModel.h"

/** Straight one-wide own slabs a boulder needs to roll down. */
constexpr int64_t DAI_BOULDER_RUN = 5;
/** A perimeter gate's door (plan 13 section 32): first of all, outside the door cap. */
constexpr int64_t DAI_GATE_DOOR_SCORE = (int64_t)1 << 40;
/** An imp trap's score for each time rival imps were seen at its slab (a route's weight is a rival's force). */
constexpr int64_t DAI_IMP_TRAP_WEIGHT = 100;

/** Barricade (Phase 7): a pack's blocker trap -- health, no shot -- that whoever comes stops to break
 *  (check_for_object_to_fight): it holds no gap shut (creatures walk past a trap), it holds the attackers where a door
 *  or a trap works on them. */
enum class DaiFortRole : uint8_t { Door, Alarm, Boulder, Pressure, Imps, Landing, MoatPost, LavaMoat, Heart, Barricade, Count };
/** Heart traps (P9): the last ditch, within this many slabs of the heart -- Armageddon's check counts those within
 *  DaiMagic::ARMAGEDDON_TRAP_REACH, the same 5 (rooms packed about the heart leave little floor nearer). */
constexpr int64_t DAI_HEART_TRAP_REACH = 5;

/** Who comes (P5.13): of the rival keepers' creatures it sees (imps apart), mostly (by levels) ones poison gas does not
 *  hurt; any a boulder breaks on. */
struct DaiFortWho {
    bool gas_immune = false;
    bool boulder_immune = false;
    /** Of the enemies' shot damage (rival keepers' creatures and heroes seen, by level), the magical share (DaiFix);
     *  -1 none seen (DAI_DOOR_MAGIC_UNSEEN then). */
    DaiFix magic_share = -1;
};
/** The magical share of the enemies' damage assumed before any is seen (a quarter). */
constexpr DaiFix DAI_DOOR_MAGIC_UNSEEN = DAI_FIX_ONE / 4;
/** Slabs within which rival ground or an enemy seen makes a door site known to the enemy: no secret door there. */
constexpr int64_t DAI_SECRET_SEEN_REACH = 4;

/** A slab of its own floor where rival imps were seen converting its ground, and how often (Defence counts them). */
struct DaiFortImpSite {
    DaiSlabXY at;
    int64_t seen = 0;
};

struct DaiFortPlacement {
    DaiSlabXY at;
    DaiFortRole role = DaiFortRole::Pressure;
    bool traffic = false;           /**< on the own-traffic map */
    bool held = false;              /**< wanted (the workshop makes it) but not placed yet: a lava moat waits for its post */
    int64_t score = 0;
};

class DaiFortify {
public:
    /** The placements wanted now, busiest first, already-standing ones left out. `contested` (bit r: rival r, the
     *  director's contest Defend goals): their routes count double, and the caps are half as much again. */
    static std::vector<DaiFortPlacement> Plan(const DaiKnowledge &k, const DaiThreatModel &t, PlayerNumber p,
                                              const DaimonGlobalConfig &cfg, const DaiPersonality &per, uint32_t contested = 0,
                                              const std::vector<DaiFortImpSite> &imps = {});
    /** Who comes, from the enemies the keeper sees now. */
    static DaiFortWho Who(const DaiKnowledge &k);
    /** The keeper's own claimed floor with nothing on it: where a trap or door may go. */
    static bool FreeClaimed(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y);
    /** Closed ground (rock, earth, walls, valuables) for a door's sides and a corridor's. */
    static bool Closed(const DaiKnowledge &k, DaiSlabCoord x, DaiSlabCoord y);
    /** Where a route first sets foot on the keeper's ground out of water (its own free floor), or false. */
    static bool LandingSite(const DaiKnowledge &k, PlayerNumber p, const std::vector<DaiSlabXY> &path, DaiSlabXY *at);
    /** One-slab guard posts of the keeper's beside water: the landings standing. */
    static int64_t LandingsStanding(const DaiKnowledge &k);
    /** A door may stand here: free claimed floor, closed on two opposite sides or on three. */
    static bool DoorSite(const DaiKnowledge &k, PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y);
    /** The slab a moat post at `post` (one wide) keeps its lava on: the heaviest route's own slab just before it, one
     *  wide across the way, off the own-traffic map, and the keeper's free floor, its lava trap, or lava (`weight`: that
     *  route's); false if none. */
    static bool MoatSite(const DaiKnowledge &k, const DaiThreatModel &t, PlayerNumber p, DaiSlabXY post, DaiSlabXY *lava,
                         int64_t *weight = nullptr);
    /** One-slab guard posts of the keeper's beside lava or a lava trap (none beside water): the moats standing. */
    static int64_t MoatsStanding(const DaiKnowledge &k);
    /** The inner end of the first straight one-wide run of DAI_BOULDER_RUN free claimed slabs off the own-traffic map
     *  along `path` (source first); false if it has none. */
    static bool BoulderSite(const DaiKnowledge &k, const DaiThreatModel &t, PlayerNumber p, const std::vector<DaiSlabXY> &path,
                            DaiSlabXY *at);
    /** The trap kind (the game's, 1-based) for a role and a place, by the stock's code names; 0 none. With `in_stock`
     *  only a kind the keeper may place now, else also one its workshop can make. */
    /** With the content catalogue (Phase 7, 09 section 13.3), a map pack's own kinds take part: a `pressure` kind for
     *  pressure and imp sites when it outrates the named one (never gas on the keeper's own ways); at the heart only a
     *  pack's `pressure` kinds (its markers, blockers and the like never). Without it, KeeperFX's kinds alone. */
    static int64_t TrapKind(const DaiStockLook &st, DaiFortRole role, bool traffic, bool in_stock, const DaiFortWho &who = DaiFortWho(),
                            const DaiContent *content = nullptr);
    /** The strongest door kind placeable now (`in_stock`) or makeable; 0 none. Never Midas; the secret door only with
     *  `secret` (the personality's secret_doors), and then first: the enemy does not see it until it finds it. */
    /** With the catalogue (Phase 7): the door of the best effective health against who comes (DoorWorth) -- KeeperFX's
     *  and a pack's own alike; with `secret`, a secret door first (a site the enemy has not seen: SecretSite). Without
     *  it, the named order (STEEL, MAGIC, BRACED, WOOD; never Midas). */
    static int64_t DoorKind(const DaiStockLook &st, bool in_stock, bool secret = false, const DaiContent *content = nullptr,
                            const DaiFortWho &who = DaiFortWho(), int64_t spare_gold = 0);
    /** A door's effective health: its health; against non-magical hits eight times (RESIST_NON_MAGIC), by the enemies'
     *  magical share; a Midas door's hits paid from `spare_gold` (each gold piece takes its health's worth of damage). */
    static int64_t DoorWorth(const DaiContentEntry &e, const DaiFortWho &who, int64_t spare_gold);
    /** No rival ground and no enemy in sight within DAI_SECRET_SEEN_REACH slabs: a door there is one the enemy has
     *  not seen the way to (a secret door's place, with the personality's secret_doors). */
    static bool SecretSite(const DaiKnowledge &k, DaiSlabXY at);
};

#endif
