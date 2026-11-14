/** @file DaiJob.h
 *  A job: one piece of work a manager wants done, as a value in the keeper's DaiJobPool (02 section 4.6, 01 section
 *  8.2). The manager creates or refreshes it, the arbiter starts it, the controller's result for its verb moves it on,
 *  and the stall rule ends it if it stops making progress. Its kind (IDaiJobKind) is the behaviour; this is the data. */
#ifndef DAIMON_DAIJOB_H
#define DAIMON_DAIJOB_H

#include <cstdint>
#include <variant>
#include <vector>

#include "globals.h"
#include "daimon/DaiFix.h"
#include "daimon/DaiTypes.h"

enum class DaiJobState : uint8_t { Ready, Running, Blocked, Done, Failed, Cancelled };

/** Job kinds, by id: the index of IDaiJobKind in the kind table (DaiJobKinds(); 0 = none). Tests use their own tables. */
typedef uint16_t DaiJobKindId;
constexpr DaiJobKindId DAI_JOB_CREATE_IMP = 1;
constexpr DaiJobKindId DAI_JOB_DIG_ROUTE = 2;
constexpr DaiJobKindId DAI_JOB_MINE_VEIN = 3;
constexpr DaiJobKindId DAI_JOB_BUILD_BRIDGE = 4;
constexpr DaiJobKindId DAI_JOB_DIG_AREA = 5;
constexpr DaiJobKindId DAI_JOB_PLACE_ROOM = 6;       /**< was build_treasury (Phase 1): Construction's from Phase 2 */
constexpr DaiJobKindId DAI_JOB_SLAP_IMPS = 7;
constexpr DaiJobKindId DAI_JOB_GROW_ROOM = 8;
constexpr DaiJobKindId DAI_JOB_REBUILD_ROOM = 9;
constexpr DaiJobKindId DAI_JOB_PLACE_ROOM_DOOR = 10;
constexpr DaiJobKindId DAI_JOB_SELL_ROOM = 11;
constexpr DaiJobKindId DAI_JOB_MOVE_TO_JOB = 12;
constexpr DaiJobKindId DAI_JOB_TRAIN_CREATURE = 13;
constexpr DaiJobKindId DAI_JOB_SEND_TO_LAIR = 14;
constexpr DaiJobKindId DAI_JOB_DISMISS_CREATURE = 15;
constexpr DaiJobKindId DAI_JOB_FEED_CREATURE = 16;
constexpr DaiJobKindId DAI_JOB_PAY_CREATURE = 17;
constexpr DaiJobKindId DAI_JOB_TACTICS_LIFT = 18;       /**< Tactics' hand (06 section 5.5): evacuate, reinforce, lift wounded, recall */
constexpr DaiJobKindId DAI_JOB_TACTICS_CAST = 19;       /**< a fight rule's cast (06 section 7) */
constexpr DaiJobKindId DAI_JOB_TACTICS_BANNER = 20;     /**< the Call to Arms banner placed, moved or taken down */
constexpr DaiJobKindId DAI_JOB_HOLD_AUDIENCE = 21;
constexpr DaiJobKindId DAI_JOB_PLACE_TRAP = 22;         /**< Defence (06 section 4.2): done when the trap is armed */
constexpr DaiJobKindId DAI_JOB_PLACE_DOOR = 23;         /**< Defence: a door on a chokepoint */
constexpr DaiJobKindId DAI_JOB_SET_DOOR_LOCK = 24;      /**< Defence: a door locked or unlocked (Q6) */
constexpr DaiJobKindId DAI_JOB_SET_TENDENCY = 25;       /**< Tactics: the flee tendency set to the personality's (06 section 11) */
constexpr DaiJobKindId DAI_JOB_TACTICS_GROUP = 26;      /**< Tactics: the group formed and ordered, or released (06 section 5a) */
constexpr DaiJobKindId DAI_JOB_MAGIC_CAST = 27;         /**< DaiMagic: a requested Sight of Evil or Destroy Walls (07 section 4.2) */
constexpr DaiJobKindId DAI_JOB_CREEP = 28;              /**< Excavation: imps dropped by an enemy slab to convert it (07 section 2.1) */
constexpr DaiJobKindId DAI_JOB_SET_ALLIANCE = 29;       /**< Offence: an alliance declared or withdrawn (07 section 5a) */
constexpr DaiJobKindId DAI_JOB_SEND_MESSAGE = 30;       /**< Offence: a short message to every player (a proposal a human notices) */
constexpr DaiJobKindId DAI_JOB_UNMARK_AREA = 31;       /**< Excavation: the marks by a hot hazard taken off (07 P5.13) */
constexpr DaiJobKindId DAI_JOB_CUSTODY_HAND = 32;      /**< Custody: a prisoner carried to a rack, a prison or the portal, or fed (08 section 17) */
constexpr DaiJobKindId DAI_JOB_JAIL_CREATURE = 33;     /**< Creatures: a livid creature of its own jailed or racked (discipline, 08 section 8) */
constexpr DaiJobKindId DAI_JOB_USE_SPECIAL = 34;      /**< Magic: a dungeon special box used by the hand (P9) */
constexpr DaiJobKindId DAI_JOB_FORTIFY_ROOM = 35;     /**< Economy: an imp dropped by a room's earth to fortify it (P9) */
constexpr DaiJobKindId DAI_JOB_KINDS = 36;

/** create_imp: the imp count when the job was made, so "done" is the count rising past it. */
struct DaiCreateImpParams {
    int64_t imps_at_start = 0;
    DaiPower power = DaiPower::CreateImp;   /**< or Tunneller (P9) */
};
/** The marks a job's gestures asked for, and how many gestures running asked for the same: a mark the game does not
 *  take would otherwise be asked for again every gesture. */
struct DaiMarkRepeat {
    DaiRect pending;            /**< what the job's next gesture will ask for */
    DaiRect last;               /**< what its last gesture asked for */
    int32_t repeats = 0;
    int64_t seen_gestures = 0;  /**< DaiJob::gestures when last counted */
};
/** dig_route (03 section 7.3): where to, how, and the path once its search found it. */
struct DaiDigRouteParams {
    std::vector<DaiSlabXY> goals;
    bool claim = false;                 /**< ClaimRoute (the target must become own land), else OwnRoute */
    bool through = false;               /**< the goal slab is dug too (a Dig service: an earth run's far end, 07 4.2) */
    std::vector<DaiSlabXY> path;        /**< empty: not searched yet (or re-planning) */
    int64_t path_cost = 0;
    /** Each path slab's entry cost when the path was first ticked (by path index; [0], the start, unused): what is left
     *  is compared with its own planned cost, not with the plan's average (a route whose cheap slabs past a crossing
     *  stood finished was re-planned for ever). */
    std::vector<int32_t> plan_costs;
    int64_t done = 0;                   /**< path slabs seen finished, over every path: `finished`'s size */
    /** The slabs seen finished, on any of its paths, once each: progress is what is newly finished, so a re-plan that
     *  lays the same slabs out again counts nothing twice. */
    std::vector<DaiSlabXY> finished;
    int64_t replans = 0;
    /** Re-plans that found another way (progress, unlike a re-plan to the same path: that loop is a stall), and the
     *  last path's hash. */
    int64_t new_ways = 0;
    uint64_t path_hash = 0;
    /** The gold left on the path's slabs at the last look (-1: not looked at this path) and the gold seen mined from
     *  them since: progress too (a gold slab far from home takes longer than the stall window to wear through). */
    int64_t gold_seen = -1;
    int64_t gold_mined = 0;
    DaiMarkRepeat marks;
    /** A fetch (P5.12): an imp dropped on the keeper's land by the next marked slab the imps leave undug -- a dropped imp
     *  takes the work where it lands, and shows the ground about it (a slab never seen may be water or rock). The imp,
     *  where, the turn the last was asked for, the job's gestures then (the drop is the next; -1: none under way), how
     *  many. */
    DaiThingRef drop_imp;
    DaiSlabXY drop_at;
    DaiTurn drop_turn = 0;
    int64_t drop_gestures = -1;
    int64_t drops = 0;
};
/** mine_vein: the vein, the slab its route reached, and the most gold seen mined from it. */
struct DaiMineVeinParams {
    uint32_t vein = 0;
    DaiSlabXY entry;
    int64_t gold_start = 0;
    int64_t mined = 0;
    DaiMarkRepeat marks;
};
/** build_bridge: the slabs, in order from the keeper's land. */
struct DaiBuildBridgeParams {
    std::vector<DaiSlabXY> slabs;
};
/** dig_area and sell_room: the rectangle. */
struct DaiAreaParams {
    DaiRect area;
};
/** place_room, grow_room, rebuild_room (04 section 6): the room, its rectangle (a growth: the strip), the entrance
 *  slab it is reached by (-1: none), and the room record it grows or rebuilds (0: none). */
struct DaiPlaceRoomParams {
    DaiRoom room = DaiRoom::Count;
    DaiRect area;
    DaiSlabXY entrance{ -1, -1 };
    uint32_t record = 0;
    /** A fetch, as dig_route's: the imp, where, the turn the last was asked for, the job's gestures then (-1: none
     *  under way), how many. */
    DaiThingRef drop_imp;
    DaiSlabXY drop_at;
    DaiTurn drop_turn = 0;
    int64_t drop_gestures = -1;
    int64_t drops = 0;
};
/** place_room_door, Defence's place_door and place_trap: the slab and the door's or trap's kind (the game's, 1-based). */
struct DaiDoorParams {
    DaiSlabXY at;
    int64_t door_kind = 0;
};
/** slap_imps: the imp chosen for the next slap. */
struct DaiSlapParams {
    DaiThingRef imp;
};
/** Creatures' hand jobs (04 section 7): the creatures (or the chicken) the hand takes, the creature it is for (feed,
 *  pay), and the game's room it drops them in (move, train, lair, dismissal: the portal). One trip each. */
struct DaiHandParams {
    std::vector<DaiThingRef> things;
    DaiThingRef target;
    int64_t room = 0;
    DaiWork work = DaiWork::Count;
    bool torture = false;       /**< jail_creature: on a rack (the torture intent), else in prison (imprison) */
};
/** Tactics' lift: the creatures, and where they go -- a subtile, or the game's room `room` when it is > 0. `step` is
 *  the menu step that made it (06 section 5.5), `slot` the hand rule that fired (-1: recall). */
struct DaiLiftParams {
    std::vector<DaiThingRef> things;
    DaiSubtlCoord x = 0, y = 0;
    int64_t room = 0;
    uint8_t step = 0;
    int8_t slot = -1;
};
/** Tactics' cast: the power (Daimon's name for it) and overcharge level, on a creature (target) or at a subtile (an
 *  area power), or neither (Hold Audience), for the fight rule in `slot` at `price` (noted on the assistant state when
 *  it is cast). off: switch the lasting power off instead (a power_off rule). */
struct DaiCastParams {
    DaiPower power = DaiPower::Count;   /**< Count: a map pack's own power, by `kind` (Phase 7) */
    int64_t level = 0;
    DaiThingRef target;
    DaiSubtlCoord x = -1, y = -1;
    int64_t slot = -1;
    int64_t price = 0;
    bool off = false;
    DaiPowerKind kind = 0;                 /**< the game's power kind when not one of Daimon's named powers */
};
/** set_door_lock: the door's slab, locked or not. */
struct DaiLockParams {
    DaiSlabXY at;
    bool lock = false;
};
/** Tactics' group (06 section 5a): its members (the first leads), and what it is told -- a move or an attack at a
 *  subtile, or (release) to break up. */
enum class DaiGroupOrder : uint8_t { Release, Move, Attack, Count };
struct DaiGroupParams {
    std::vector<DaiThingRef> members;
    DaiGroupOrder order = DaiGroupOrder::Move;
    DaiSubtlCoord x = 0, y = 0;
};
/** creep: another keeper's slab (its claimed path or a room's) beside the keeper's own land, which the keeper's imps
 *  convert on their own (the engine's convert tasks); the job drops up to `imps` of them on the foothold, a hand-load at
 *  a time, again once the last drop is lift_gap turns old and fewer than that stand near. */
struct DaiCreepParams {
    DaiSlabXY at{ -1, -1 };
    int64_t imps = 1;
    DaiTurn lifted = 0;                /**< the last hand-load's turn */
    std::vector<DaiThingRef> things;    /**< the hand-load being made */
    DaiSubtlCoord x = 0, y = 0;         /**< where it lands */
    /** Reinforce (plan 13 section 32): `at` is earth to fortify, the imp dropped on the foothold slab itself -- a
     *  dropped imp reinforces the earth beside the slab it lands on, and goes on along the wall. */
    bool reinforce = false;
};
/** set_alliance: the declaration towards `other` set to `enabled`; send_message: `message` to every player. */
/** use_special: the box, its kind, and Resurrect's choice (the dead creature's kind, code name and level). */
struct DaiSpecialParams {
    DaiThingRef box;
    DaiSpecial kind = DaiSpecial::None;
    DaiThingModel model = 0;
    char code[32] = {};
    int64_t level = 0;
};
/** fortify_room: the imp, and the subtile of the room's edge it is dropped on (beside the earth it fortifies first). */
struct DaiFortifyParams {
    DaiThingRef imp;
    DaiSubtlCoord stl_x = 0, stl_y = 0;
    DaiSlabCoord wall_x = 0, wall_y = 0;    /**< the earth slab beside it */
};
struct DaiDiplomacyParams {
    PlayerNumber other = -1;
    bool enabled = false;
    char message[64] = {};
};
/** The banner: placed at a subtile, or (off) taken down. */
struct DaiBannerParams {
    bool off = false;
    DaiSubtlCoord x = 0, y = 0;
};
/** set_tendency: which and to what (no params: the flee tendency to the personality's, as Tactics sets it). */
enum class DaiTendency : uint8_t { Flee, Imprison, Count };
struct DaiTendencyParams {
    DaiTendency which = DaiTendency::Flee;
    bool on = false;
};
/** custody_hand (08 section 17): a prisoner of the keeper's carried to its room `room` with that room's intent (a rack,
 *  a prison, the portal: dismissed), or a chicken `food` dropped on it; or (Sacrifice, 08 section 7.2) a prisoner, one
 *  of its creatures or an imp dropped on its temple pool. */
enum class DaiCustodyTrip : uint8_t { Torture, Prison, Dismiss, Feed, Sacrifice, Count };
struct DaiCustodyHandParams {
    DaiThingRef prisoner;
    DaiCustodyTrip trip = DaiCustodyTrip::Prison;
    int64_t room = 0;
    DaiThingRef food;
};
typedef std::variant<std::monostate, DaiCreateImpParams, DaiDigRouteParams, DaiMineVeinParams, DaiBuildBridgeParams,
                     DaiAreaParams, DaiSlapParams, DaiPlaceRoomParams, DaiDoorParams, DaiHandParams, DaiLiftParams,
                     DaiCastParams, DaiBannerParams, DaiLockParams, DaiGroupParams, DaiCreepParams, DaiDiplomacyParams,
                     DaiTendencyParams, DaiCustodyHandParams, DaiSpecialParams, DaiFortifyParams> DaiJobParams;

/** What the job's gesture needs claimed while it runs (01 section 8.3): taken by the arbiter before the submit,
 *  released when the gesture's result comes back, or with the job. */
struct DaiJobNeeds {
    int64_t gold = 0;
    bool hand = false;
    DaiThingRef thing;          /**< idx 0: none */
};

struct DaiJob {
    uint32_t id = 0;
    DaiJobKindId kind = 0;
    DaiDomain domain = DaiDomain::Count;
    uint64_t key = 0;           /**< the manager's key for "the same job": refreshed, not duplicated */
    uint32_t goal_id = 0, request_id = 0;
    DaiFix utility = 0;
    DaiTurn created = 0;
    DaiTurn state_turn = 0;            /**< when it entered its state */
    DaiTurn last_progress_turn = 0;
    DaiTurn stall_window = 0;
    DaiTurn retry_after = 0;           /**< not started before this turn */
    int64_t last_progress = 0;          /**< the kind's monotone metric (imps owned, slabs dug) */
    DaiJobState state = DaiJobState::Ready;
    int64_t verb_id = 0;                /**< the controller verb in flight (0: none) */
    int64_t gestures = 0;               /**< gestures submitted for it (the arbiter counts) */
    DaiJobNeeds needs;
    bool end_seen = false;              /**< its manager has seen it end (Done, Failed, Blocked, Cancelled) */
    DaiJobParams p;
    char why[32] = {};                  /**< why it is Blocked or Failed (the controller's code kept verbatim) */
};

const char *DaiJobStateName(DaiJobState s);
inline bool DaiJobLive(DaiJobState s) { return (s == DaiJobState::Ready) || (s == DaiJobState::Running); }
inline bool DaiJobEnded(DaiJobState s) { return !DaiJobLive(s); }

#endif
