/** @file DaiTypes.h
 *  Daimon's small shared types: the facade's enums (seen by C as int64_t, see Daimon.h), domains, results, and the
 *  references Daimon keeps to things and places (docs/plans/ai-plans/v2/02-library-architecture.md 0, 7). */
#ifndef DAIMON_DAITYPES_H
#define DAIMON_DAITYPES_H

#include <cstdint>
#include "globals.h"
#include "daimon_base_types.h"

/** What Daimon does with a keeper. */
enum class DaimonMode : int64_t {
    None = -1,       /**< not a Daimon keeper */
    Opponent = 0,    /**< plays the keeper: skirmish, scenario, drop-in, takeover (sets PlaF_Daimon) */
    Assistant = 1,   /**< helps the local human (Phase 7) */
    Lieutenant = 2,  /**< works under an External seat's agent (Phase 8) */
};

/** Who a keeper goes back to. */
enum class DaimonHandBackTo : int64_t {
    Computer2 = 0,   /**< the keeper's own built-in AI model */
    Human = 1,       /**< the local human (assistant off, a player reconnecting) */
};

/** Why Daimon took a keeper (logs, the save). */
enum class DaimonTakeReason : int64_t {
    Skirmish = 0, Script = 1, LevelKey = 2, DropIn = 3, HostLost = 4, SeatTakeover = 5, Assistant = 6,
    Lieutenant = 7, Console = 8, Load = 9, LocalSeat = 10,
};

/** The arbiter's domains, in the managers' order (02 4.10). */
enum class DaiDomain : uint8_t {
    Economy, Workforce, Excavation, Construction, Creatures, Custody, Defence, Offence, Magic, Tactics, Count
};
constexpr const char *DaiDomainName(DaiDomain d)
{
    constexpr const char *names[] = { "economy", "workforce", "excavation", "construction", "creatures", "custody",
                                      "defence", "offence", "magic", "tactics" };
    return (d < DaiDomain::Count) ? names[(size_t)d] : "?";
}

enum class DaiResult : uint8_t { Ok, Done, Wait, Fail, Full, BadInput };

/** A thing is the same thing only if index and creation turn both match (external_seat.c's ordered-creature rule). */
struct DaiThingRef {
    DaiThingIndex idx = 0;
    DaiTurn creation_turn = 0;
    bool operator==(const DaiThingRef &o) const { return (idx == o.idx) && (creation_turn == o.creation_turn); }
};

/** An inclusive rectangle of slabs. */
struct DaiRect {
    DaiSlabCoord x0 = 0, y0 = 0, x1 = 0, y1 = 0;
};

struct DaiSlabXY {
    DaiSlabCoord x = 0, y = 0;
};

/** What a slab is, as far as a keeper knows: 5 bits in the knowledge map (03 section 2.4a for abyss). */
enum class DaiTerrain : uint8_t {
    Unknown = 0,   /**< not revealed: never treated as rock (03 section 5) */
    Rock, Dirt, Gold, DenseGold, Gem, Floor, Room, Water, Lava, Abyss, Wall, Door, Impassable,
    Count
};

/** The keeper powers Daimon uses, by its own name (decisions never see the game's DaiPowerKind numbers): the reader maps
 *  each to the game's power, the verbs name it by its magic.cfg code name. Later phases add theirs. */
/** The powers Daimon casts: Workforce's Create Imp; Tactics' fight powers (Phase 4, 06 section 2.2). */
enum class DaiPower : uint8_t {
    CreateImp, CallToArms, Heal, Speed, Protect, Lightning, Slow, Freeze, Chicken, CaveIn, HoldAudience,
    DestroyWalls, Sight,        // Phase 5 (07 sections 4.2, 7)
    Obey, Conceal, Flight, Vision, Rebound, Disease, Tunneller, Armageddon,     // P9
    Count
};
/** Its code name in magic.cfg (power_model_id). */
constexpr const char *DaiPowerCode(DaiPower p)
{
    switch (p) {
    case DaiPower::CreateImp: return "POWER_IMP";
    case DaiPower::CallToArms: return "POWER_CALL_TO_ARMS";
    case DaiPower::Heal: return "POWER_HEAL_CREATURE";
    case DaiPower::Speed: return "POWER_SPEED";
    case DaiPower::Protect: return "POWER_PROTECT";
    case DaiPower::Lightning: return "POWER_LIGHTNING";
    case DaiPower::Slow: return "POWER_SLOW";
    case DaiPower::Freeze: return "POWER_FREEZE";
    case DaiPower::Chicken: return "POWER_CHICKEN";
    case DaiPower::CaveIn: return "POWER_CAVE_IN";
    case DaiPower::HoldAudience: return "POWER_HOLD_AUDIENCE";
    case DaiPower::DestroyWalls: return "POWER_DESTROY_WALLS";
    case DaiPower::Sight: return "POWER_SIGHT";
    case DaiPower::Obey: return "POWER_OBEY";
    case DaiPower::Conceal: return "POWER_CONCEAL";
    case DaiPower::Flight: return "POWER_FLIGHT";
    case DaiPower::Vision: return "POWER_VISION";
    case DaiPower::Rebound: return "POWER_REBOUND";
    case DaiPower::Disease: return "POWER_DISEASE";
    case DaiPower::Tunneller: return "POWER_TUNNELLER";
    case DaiPower::Armageddon: return "POWER_ARMAGEDDON";
    default: return "";
    }
}

/** A dungeon special box's kind (power_specials.h's SpcKind_*, the same numbers; P9). */
enum class DaiSpecial : uint8_t {
    None, RevealMap, Resurrect, Transfer, StealHero, Multiply, IncreaseLevel, MakeSafe, HiddenWorld, Custom, HealAll,
    GetGold, MakeAngry, MakeUnsafe, Count
};

/** The rooms Daimon builds or wants, by its own name: the reader maps each to the first of the game's room kinds with
 *  its role (DaiRoomRole; 04 section 2.1: rooms are found by role, so a mod's re-roled rooms work). */
enum class DaiRoom : uint8_t {
    Treasury, Bridge, Entrance,
    Lair, Hatchery, Library, Training, Workshop, Prison, Torture, Temple, Graveyard, Scavenger, Barracks, GuardPost,
    Count
};
/** Its code name in the shipped terrain.cfg, as build_room takes it. */
constexpr const char *DaiRoomCode(DaiRoom r)
{
    switch (r) {
    case DaiRoom::Treasury: return "TREASURE";
    case DaiRoom::Bridge: return "BRIDGE";
    case DaiRoom::Entrance: return "ENTRANCE";
    case DaiRoom::Lair: return "LAIR";
    case DaiRoom::Hatchery: return "GARDEN";
    case DaiRoom::Library: return "RESEARCH";
    case DaiRoom::Training: return "TRAINING";
    case DaiRoom::Workshop: return "WORKSHOP";
    case DaiRoom::Prison: return "PRISON";
    case DaiRoom::Torture: return "TORTURE";
    case DaiRoom::Temple: return "TEMPLE";
    case DaiRoom::Graveyard: return "GRAVEYARD";
    case DaiRoom::Scavenger: return "SCAVENGER";
    case DaiRoom::Barracks: return "BARRACKS";
    case DaiRoom::GuardPost: return "GUARD_POST";
    default: return "";
    }
}

/** What a room does: the game's room roles (terrain.cfg ROLES, 04 section 2.1), the same bits, by Daimon's names. Code
 *  finds rooms by role, not by kind, so a mod's re-roled rooms work. The sim reader checks each against the game's. */
constexpr uint64_t DAI_ROLE_HEART = 0x00000001;          /**< keeper storage */
constexpr uint64_t DAI_ROLE_LAIR = 0x00000002;
constexpr uint64_t DAI_ROLE_GOLD = 0x00000004;
constexpr uint64_t DAI_ROLE_FOOD = 0x00000008;           /**< food storage */
constexpr uint64_t DAI_ROLE_CRATES = 0x00000010;
constexpr uint64_t DAI_ROLE_SPELLBOOKS = 0x00000020;
constexpr uint64_t DAI_ROLE_PRISON = 0x00000040;
constexpr uint64_t DAI_ROLE_GRAVEYARD = 0x00000080;
constexpr uint64_t DAI_ROLE_PORTAL = 0x00000100;         /**< creatures from the pool arrive here */
constexpr uint64_t DAI_ROLE_SACRIFICE = 0x00000400;
constexpr uint64_t DAI_ROLE_FOOD_SPAWN = 0x00001000;
constexpr uint64_t DAI_ROLE_MANUFACTURE = 0x00002000;
constexpr uint64_t DAI_ROLE_RESEARCH = 0x00004000;
constexpr uint64_t DAI_ROLE_TORTURE = 0x00008000;
constexpr uint64_t DAI_ROLE_PRAY = 0x00010000;
constexpr uint64_t DAI_ROLE_HEAL_SLEEP = 0x00020000;
constexpr uint64_t DAI_ROLE_SCAVENGE = 0x00040000;
constexpr uint64_t DAI_ROLE_TRAIN = 0x00080000;
constexpr uint64_t DAI_ROLE_GROUP = 0x00100000;          /**< barracks */
constexpr uint64_t DAI_ROLE_GUARD = 0x00200000;
constexpr uint64_t DAI_ROLE_PASS_WATER = 0x00800000;
constexpr uint64_t DAI_ROLE_PASS_LAVA = 0x01000000;

/** The role that names one of Daimon's rooms (the game's first room kind with it is that room). */
constexpr uint64_t DaiRoomRole(DaiRoom r)
{
    switch (r) {
    case DaiRoom::Treasury: return DAI_ROLE_GOLD;
    case DaiRoom::Bridge: return DAI_ROLE_PASS_WATER;
    case DaiRoom::Entrance: return DAI_ROLE_PORTAL;
    case DaiRoom::Lair: return DAI_ROLE_LAIR;
    case DaiRoom::Hatchery: return DAI_ROLE_FOOD;
    case DaiRoom::Library: return DAI_ROLE_RESEARCH;
    case DaiRoom::Training: return DAI_ROLE_TRAIN;
    case DaiRoom::Workshop: return DAI_ROLE_MANUFACTURE;
    case DaiRoom::Prison: return DAI_ROLE_PRISON;
    case DaiRoom::Torture: return DAI_ROLE_TORTURE;
    case DaiRoom::Temple: return DAI_ROLE_SACRIFICE;
    case DaiRoom::Graveyard: return DAI_ROLE_GRAVEYARD;
    case DaiRoom::Scavenger: return DAI_ROLE_SCAVENGE;
    case DaiRoom::Barracks: return DAI_ROLE_GROUP;
    case DaiRoom::GuardPost: return DAI_ROLE_GUARD;
    default: return 0;
    }
}

/** What one of the keeper's creatures is doing, as a human reads it from what it does (04 section 2.5, Q1): the game's
 *  creature state types, the same numbers. Hunger shows as Feed, a wish for pay as Salary. */
enum class DaiActivity : uint8_t {
    Idle, Work, OwnNeeds, Sleep, Feed, Fight, Move, Salary, Escape, Unconscious, AngerJob, FightDoor, FightObject,
    CalledToArms, Follow, DeepWork, Count
};
/** Work the keeper puts its creatures to by hand (04 section 7.3), and the role of the room it is done in. */
enum class DaiWork : uint8_t { Research, Manufacture, Train, Count };
constexpr uint64_t DaiWorkRole(DaiWork w)
{
    return (w == DaiWork::Research) ? DAI_ROLE_RESEARCH : (w == DaiWork::Manufacture) ? DAI_ROLE_MANUFACTURE
         : (w == DaiWork::Train) ? DAI_ROLE_TRAIN : 0;
}

/** Anger as the creature panel shows it (04 Q1): a band, not the counters. */
enum class DaiAnger : uint8_t { Calm, Angry, Livid, Count };
/** Why a creature is angry (its worst motive, as the panel shows it). */
enum class DaiAngerReason : uint8_t { None, NotPaid, Hungry, NoLair, Other, Count };
/** What the workshop takes (04 section 9): only `Auto` exists today; requests and purchases come with that feature. */
enum class DaiWorkshopMode : uint8_t { Auto, Requests, Purchase, Both, Count };
/** A research item's kind (the game's research categories, the same numbers). */
enum class DaiResearchKind : uint8_t { None, Power, Room, Creature, Count };

/** Work a keeper's turn may do, decremented by the class that spends it (02 section 5). */
struct DaiBudget {
    int64_t slab_scan = 0;      /**< slabs the knowledge's rolling scan may read */
    int64_t reach_visits = 0;   /**< slabs the reach map's flood may visit (Phase 1) */
    int64_t search_nodes = 0;   /**< planner nodes (Phase 1) */
    int64_t layout_candidates = 0;  /**< sites the layout planner weighs (Phase 2) */
};

#endif
