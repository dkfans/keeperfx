/** @file IDaiWorldReader.h
 *  The only way Daimon reads the game (02 section 4.2, principle 1): every answer is what a human in that keeper's
 *  seat could know (D2). DaiWorldReaderSim answers from the simulation, tests from a synthetic map, Phase 10's
 *  DaiSnapshot from a copy. Phase 0 builds the part knowledge and the imp-keeping manager need; later phases add the
 *  calls 02 section 4.2 lists (custody, stock, research, castability, the live exclusion...). */
#ifndef DAIMON_IDAIWORLDREADER_H
#define DAIMON_IDAIWORLDREADER_H

#include <cstdint>
#include <vector>

#include "globals.h"
#include "keeper_assist.h"
#include "keeper_facts_types.h"
#include "daimon/DaiContent.h"
#include "daimon/DaiFix.h"
#include "daimon/DaiLimits.h"
#include "daimon/DaiTypes.h"

/** One slab as the keeper may know it. An unrevealed slab is Unknown with no owner and no room. */
struct DaiSlabLook {
    DaiTerrain terrain = DaiTerrain::Unknown;
    PlayerNumber owner = -1;
    RoomKind room = 0;
    bool revealed = false;
    bool marked = false;        /**< one of the keeper's own dig marks (shown on unrevealed slabs too) */
    int64_t gold_left = 0;      /**< a revealed gold or dense gold slab: the gold still in it (03 section 3) */
};

/** The keeper's own dungeon: always exact. */
struct DaiOwnLook {
    int64_t gold = 0;
    int64_t imps = 0;           /**< its diggers: imps, and tunnellers working as imps (P9) */
    int64_t tunnellers = 0;     /**< of them, tunnellers */
    int64_t creatures = 0;
    int64_t wage_bill = 0;
    int64_t turns_to_payday = 0;
    bool has_heart = false;
    DaiSubtlCoord heart_x = 0, heart_y = 0;
    int64_t heart_health = 0, heart_max_health = 0;
    bool cta_active = false;    /**< Call to Arms is on (Phase 4: the combat assistant's dungeon facts) */
    bool cta_paying = false;    /**< ...and charged for: its banner is off the keeper's ground */
    bool obey_active = false;   /**< Must Obey is on */
    int64_t obey_upkeep_per_kturn = 0;  /**< what Must Obey costs each 1,000 turns while on (its price every duration turns) */
    // Armageddon (P9; any keeper's: the level has one at a time): cast by whom, when the creatures start going to its
    // caster's heart (the countdown's end), when it is over.
    bool armageddon = false;
    PlayerNumber armageddon_caster = -1;
    DaiTurn armageddon_teleport = 0, armageddon_over = 0;
    bool flee_tendency = false; /**< its creatures' flee tendency is on (the panel button, creature_tendencies) */
    bool imprison_tendency = false; /**< ...and the imprison tendency (knocked-out enemies taken prisoner, 08 section 6) */
    // Its one group (06 section 5a, keeper_group_state.h): members (0: none), the order (KeeperGroupOrder), the leader;
    // and the rules' most creatures in a group (BarrackMaxPartySize, at most 30).
    int64_t group_count = 0, group_order = 0;
    DaiThingIndex group_leader = 0;
    int64_t group_max = 0;
    // The economy (03 section 2.6): gold is total_money_owned (off-map gold and the treasuries' hoards).
    int64_t payday_progress = 0, payday_gap = 0, payday_speed = 0;
    int64_t offmap_gold = 0;
    int64_t treasury_capacity = 0, treasury_used = 0;     /**< gold its treasuries hold at most, and hold now */
    int64_t loose_gold = 0;     /**< gold piles on the keeper's own ground */
    int64_t gold_carried = 0;   /**< in its imps' pockets */
    int64_t gold_mined = 0;     /**< mined since the level started */
    int64_t imp_gold_hold = 0;  /**< gold an imp carries before it takes it to a treasury */
    int64_t food_generation_speed = 0;  /**< a hatchery of capacity C grows a chicken every this / C + 1 turns (rules.cfg) */
};

/** One of the keeper's own imps. */
struct DaiImpLook {
    DaiThingRef ref;
    DaiSubtlCoord x = 0, y = 0;
    bool working = false;       /**< digging, mining, claiming or reinforcing */
    bool sped_up = false;       /**< still running from a slap */
    bool sacrificing = false;   /**< going into a temple's pool */
    int64_t gold = 0;           /**< the gold it carries (its panel shows it): one carrying any takes it home first */
    int64_t force = 0;          /**< its kind's score at its level, unhurt (creature_force_score's scale; P5.13 rates
                                 *   the imps a trap took by it) */
    bool tunneller = false;     /**< a tunneller (POWER_TUNNELLER): on the creatures' list, digging as an imp (P9) */
    DaiThingModel model = 0;       /**< its kind */
    int64_t level = 1;          /**< 1-based: an imp learns speed at 3 and teleport at 10 (P9) */
    bool training = false;      /**< in one of its keeper's training rooms (or on its way there to train) */
    bool pickable = true;       /**< the hand may take it now (can_thing_be_picked_up_by_player) */
};

/** One of the rooms Daimon builds, as the keeper may build it now. */
struct DaiBuildLook {
    bool available = false;
    int64_t price = 0;          /**< a slab */
};

/** What the keeper's bridge crosses (its room roles: a mod may let it cross abyss, 03 section 2.4a). */
struct DaiBridgeRoles {
    bool water = false, lava = false, abyss = false;
};

struct DaiPowerLook {
    bool available = false;
    int64_t price = 0;
};

/** One of the keeper's own rooms, exact (04 section 3). */
struct DaiRoomLook {
    int64_t index = 0;          /**< the game's room index: changes when a room is sold or split (DaiKnowledge keeps
                                 *   Daimon's own identity, DaiRoomRecord) */
    RoomKind kind = 0;
    int64_t slabs = 0, capacity = 0, used = 0;
    DaiSlabCoord cx = 0, cy = 0;
    uint64_t roles = 0;         /**< DAI_ROLE_* */
    int64_t efficiency = 0;     /**< 0..256, as the room panel shows it */
    int64_t health = 0, max_health = 0;
    int64_t stored = 0;         /**< what it stores: gold in a treasury, chickens in a hatchery, crates, books */
    int64_t occupants = 0;      /**< creatures working (or held) in it; a lair: creatures housed */
    int64_t open_sides = 0;     /**< sides of its slabs that score nothing toward its efficiency (04 section 2.2) */
    DaiSlabCoord ax = 0, ay = 0;    /**< one of its slabs (the first of its slab list) */
    DaiRect box;                /**< the rectangle round its slabs */
};

/** One of the keeper's own creatures, imps apart (OwnImps): what the seat view shows, with the creature panel's anger
 *  band and experience progress (04 section 2.5, Q1). No raw counters. */
struct DaiCreatureLook {
    DaiThingRef ref;
    DaiThingModel model = 0;
    int64_t level = 0;          /**< 1-based */
    int64_t health = 0, health_pct = 100;   /**< of its maximum, as the health bar shows it */
    DaiSubtlCoord x = 0, y = 0;
    int64_t state = 0;          /**< the game's state (for logs; decisions read `activity`) */
    DaiActivity activity = DaiActivity::Idle;
    RoomKind in_room = 0;       /**< the kind of the keeper's own room it stands in; 0 none */
    bool homeless = false;      /**< no lair of its own yet: the game counts its lair size as taken (free lair space) */
    DaiAnger anger = DaiAnger::Calm;
    DaiAngerReason anger_reason = DaiAngerReason::None;
    bool fleeing = false;
    bool orderable = true;      /**< the Order mode takes it now: not near its hunger limit, not owed pay (the HUNGRY and
                                 *   OWED_PAY refusals a human meets by trying; P5.12: one such member refused a whole
                                 *   group's order) */
    int64_t exp_pct = -1;       /**< experience toward its next level, 0..99; -1: it can gain no more */
    int64_t upkeep_per_kturn = 0;   /**< what its training or scavenging costs each 1,000 turns while it does it (the
                                     *   creature panel shows the cost; the game takes it every few dozen turns) */
    int64_t force = 0;          /**< creature_force_score: its kind and level, times its health (Phase 4, 06 section 2.1) */
    int64_t battle = 0;         /**< the battle it fights in (0: none) */
    // Custody and care (Phase 6, 08): what its creature panel and the map show.
    PlayerNumber held_by = -1;  /**< the keeper whose prison or torture chamber holds it (itself: discipline, 08 section 8) */
    bool scavenged = false;     /**< a rival's scavenger room is luring it (08 section 7a.3) */
    bool diseased = false, chickened = false;   /**< praying in a temple cures both (08 section 7.1) */
    bool sped = false;          /**< under Speed Up: it works -- trains, researches, manufactures -- twice as fast (P9) */
    bool sacrificing = false;   /**< going into a temple's pool */
    // Scavenging (08 section 7a, as the creature panel shows it): the points it adds a turn in a scavenger room at full
    // efficiency, and what the keeper's pool draw of its kind needs (x256 points).
    int64_t scavenge_work = 0, scavenge_required = 0;
};

/** A creature kind: its rules (creature config, as the seat view's kind profile shows them, with the attraction
 *  data of 04 section 2.4) and what the keeper may have of it. */
struct DaiKindLook {
    DaiThingModel model = 0;
    bool known = false;         /**< a kind the game has (the rest of the look is empty otherwise) */
    bool digger = false, spectator = false;
    bool tunneller = false;     /**< digs as an imp but is a creature, paid (DIGGING_CREATURE; POWER_TUNNELLER's, P9) */
    char code[32] = {};         /**< its code name (creature.cfg), as orders name a kind */
    bool immune_gas = false;    /**< poison clouds do not hurt it (SpellImmunity POISON_CLOUD; P5.13) */
    bool immune_boulder = false;    /**< a rolling boulder breaks on it (IMMUNE_TO_BOULDER; P5.13) */
    int64_t speed = 0;          /**< its base walking speed (creature config's BaseSpeed; Phase 4's threat radius) */
    bool allowed = false;       /**< the keeper may attract it */
    bool forced = false;        /**< a script lets it come without its entrance rooms (CREATURE_AVAILABLE's force) */
    int64_t in_pool = -1;       /**< left in the creature pool -- only with the knows_pool cheat (08 Q8: a human sees only
                                 *   that the kind may come, `allowed`); -1 unknown */
    int64_t pay = 0, lair_size = 0, annoy_level = 0;
    int64_t hunger_rate = 0;    /**< turns from a meal until it is hungry again (0: it never eats) */
    int64_t hunger_fill = 0;    /**< chickens a meal */
    int64_t heal_requirement = 0;   /**< /256 of its health: below it, it goes to heal */
    int64_t training_cost = 0, scavenger_cost = 0;
    int64_t research_value = 0, manufacture_value = 0, training_value = 0, scavenge_value = 0;
    uint64_t jobs_primary = 0, jobs_secondary = 0, jobs_not_do = 0, jobs_anger = 0, jobs_stress = 0;
    bool refuses[(size_t)DaiWork::Count] = {};     /**< the work is in its NotDoJobs: dropped there, it refuses, angry */
    bool stressful[(size_t)DaiWork::Count] = {};   /**< the work is in its StressfulJobs */
    bool anger_harmful = false;     /**< livid, it would hurt the dungeon: kill, destroy, steal, damage, join the enemy
                                     *   (04 Q4; leaving alone is not) */
    RoomKind entrance_rooms[3] = { 0, 0, 0 };   /**< it comes only when the keeper has entrance_slabs of each */
    int64_t entrance_slabs[3] = { 0, 0, 0 };
    int64_t entrance_score = 0;
    /** Its score at each level, 0-based (the engine's creature score table: creature_force_score's scale; a summoned
     *  creature's worth, 08 section 7.2). */
    static constexpr int64_t LEVELS = 10;
    int64_t score[LEVELS] = {};
    /** Its fight at each level, 0-based (DaiAbilities: damage-equivalent in 100 turns, its abilities and melee), and the
     *  share of it (DaiFix) the abilities of its next two levels add (P9). */
    int64_t output[LEVELS] = {};
    DaiFix ability_gain[LEVELS] = {};
    /** Of its shots' damage at level 5 (melee swings and missiles), the magical share (DaiFix): what a magic door
     *  takes in full, other hits an eighth (Phase 7, doors by effective health). */
    DaiFix magic_share = 0;
    int64_t rebirth = 0;        /**< reborn at its lair when killed from this level (the config's Rebirth; 0 never) */
    bool alluring = false;      /**< ALLURING_SCVNGR: scavenging with nothing of its kind to lure, it speeds the portal
                                 *   for any kind (portal_scavenge_boost, the manage score's attraction) */
};

/** Where one of the keeper's prisoners is (08 section 4). */
enum class DaiHeldAt : uint8_t { Moving, Prison, Torture, Hand };

/** Another keeper's (or a hero's) creature in the keeper's custody, as the seat's custody view shows it. */
struct DaiPrisonerLook {
    DaiThingRef ref;
    DaiThingModel model = 0;
    int64_t level = 0;          /**< 1-based */
    PlayerNumber owner = -1;
    int64_t health = 0, max_health = 0;
    bool hungry = false;
    DaiHeldAt at = DaiHeldAt::Moving;
    int64_t room = 0;           /**< the game's room it stands in (0: none) */
    DaiThingModel on_death = 0;    /**< what it becomes for the keeper if it dies where it is (0: a corpse) */
    int64_t force = 0;          /**< its kind's score at its level, unhurt (creature_force_score's scale) */
    DaiThingModel skeleton = 0, ghost = 0;     /**< what it would become starved in the keeper's prison, killed on its rack */
    int64_t skeleton_force = 0, ghost_force = 0;    /**< their kinds' scores at its level */
    int64_t torture_turns = 0;  /**< on the rack this long */
    int64_t break_time = 0;     /**< its kind's TortureTime: past it, each turn may break it */
    DaiSubtlCoord x = 0, y = 0;
};

/** The keeper's custody and its special rooms (08 sections 2.1-2.4): its prisoners, its free torture devices, the
 *  graveyard's progress to a vampire, a temple pool to drop on, and what it has offered since a recipe last fired. */
struct DaiCustodyLook {
    static constexpr int64_t MAX_PRISONERS = 64;
    int64_t prisoners = 0;
    DaiPrisonerLook prisoner[MAX_PRISONERS];
    int64_t devices_free = 0;   /**< torture devices with no victim, in all its torture chambers */
    int64_t bodies_rotting = 0, bodies_per_vampire = 0;     /**< toward the next vampire, and how many make one */
    DaiThingModel vampire_model = 0;   /**< what a graveyard raises from them (its room's CreatureCreation; 0: none) */
    int64_t bodies_lying = 0;   /**< corpses on revealed ground outside graveyards */
    bool pool = false;          /**< a temple pool subtile it can drop on */
    DaiSubtlCoord pool_x = 0, pool_y = 0;
    int64_t praying = 0;        /**< creatures in its temples */
    int64_t offered[DAI_MAX_KINDS] = {};    /**< by kind, since the last recipe fired */
    int64_t offered_levels[DAI_MAX_KINDS] = {};     /**< their levels (1-based) summed: a summoned creature's level */
    /** Creatures going into its pool now (any owner's: the pool's keeper gets them), by kind, and their levels: the game
     *  counts each into `offered` only once it has sunk, some 50 turns after the drop. */
    int64_t sinking[DAI_MAX_KINDS] = {};
    int64_t sinking_levels[DAI_MAX_KINDS] = {};
    int64_t imps_sacrificed = 0;    /**< imps offered for cheaper imps, net (the imp price counts them off) */
    // The keeper's rules (rules.cfg [rooms], [gameplay]), %: a break converts the victim, kills it; a prisoner starved
    // rises a skeleton, a victim killed on a rack a ghost; pay for a kind with a victim of it on a rack.
    int64_t convert_chance = 0, death_chance = 0, skeleton_chance = 0, ghost_chance = 0, torture_payday = 100;
};

/** A temple recipe as the rules table has it (rules[0], the table the engine checks for every keeper; 08 section 7.2). */
enum class DaiRecipeAction : uint8_t {
    None, MakeCreature, MakeHero, CurseAll, BlessAll, Punish, Boon, CustomReward, CustomPunish
};
enum class DaiBoon : uint8_t { None, CompleteResearch, CompleteManufacture, CheaperImps, AllHappy };
struct DaiRecipeLook {
    static constexpr int64_t MAX_VICTIMS = 6;
    DaiThingModel victims[MAX_VICTIMS] = {};   /**< the kinds offered (0: none), a kind once per victim */
    DaiRecipeAction action = DaiRecipeAction::None;
    DaiThingModel creature = 0;    /**< MakeCreature, MakeHero */
    int64_t spell = 0;          /**< CurseAll, BlessAll: the game's spell kind */
    DaiBoon boon = DaiBoon::None;
    int64_t index = 0;          /**< its slot in the rules' table (KF_Sacrifice names a recipe by it) */
};

/** A mature chicken in one of the keeper's hatcheries (food the hand may take to a hungry creature). */
struct DaiFoodLook {
    DaiThingRef ref;
    DaiSubtlCoord x = 0, y = 0;
};

/** What the keeper's library will research, in order: the items still needed (04 section 2.4, research cannot be
 *  ordered). The first is being researched. */
struct DaiResearchItem {
    DaiResearchKind kind = DaiResearchKind::None;
    int64_t which = 0;          /**< the game's power, room or creature kind */
    int64_t amount = 0;         /**< research points it needs */
};
struct DaiResearchLook {
    static constexpr int64_t MAX = 32;
    int64_t count = 0;          /**< items still needed, at most MAX listed */
    DaiResearchItem items[MAX];
    int64_t progress = 0;       /**< points done on the first item */
    int64_t max_creatures = 0;  /**< creatures the keeper may attract at most */
};

/** The workshop and what it made (04 section 9). Kinds are the game's trap and door kinds, 1-based. Kinds 0-31 sit at
 *  their own index (trap_kinds / door_kinds of them); a map pack's own kinds numbered 32 and up (dk2maps' from 1001;
 *  trapdoor.cfg allows to 2,000) in the extra slots from MAX_KINDS on, their kinds in trap_extra_kind / door_extra_kind
 *  (Phase 7, 09 section 13.3). The arrays are by slot; the accessors take a kind. */
struct DaiStockLook {
    static constexpr int64_t MAX_KINDS = 32;    /**< kinds below this at their own index */
    static constexpr int64_t MAX_EXTRA = 32;    /**< kinds from MAX_KINDS up, in slots MAX_KINDS .. + MAX_EXTRA */
    static constexpr int64_t MAX_SLOTS = MAX_KINDS + MAX_EXTRA;
    DaiWorkshopMode mode = DaiWorkshopMode::Auto;
    int64_t manufacture_level = 0;
    bool making_door = false;   /**< what is being made: a door or a trap */
    int64_t making_kind = 0;    /**< 0 nothing */
    int64_t progress = 0;
    int64_t trap_kinds = 0, door_kinds = 0;     /**< kinds 0 .. this - 1 at their own index (at most MAX_KINDS) */
    int64_t trap_extra = 0, door_extra = 0;     /**< extra slots used */
    int64_t trap_extra_kind[MAX_EXTRA] = {}, door_extra_kind[MAX_EXTRA] = {};
    int64_t traps_placeable[MAX_SLOTS] = {}, traps_stored[MAX_SLOTS] = {};
    int64_t doors_placeable[MAX_SLOTS] = {}, doors_stored[MAX_SLOTS] = {};
    bool traps_buildable[MAX_SLOTS] = {}, doors_buildable[MAX_SLOTS] = {};
    /** Each trap kind's worth in 100 turns while triggered (DaiTrapRating: a creature's abilities' scale, P9). */
    int64_t trap_ratings[MAX_SLOTS] = {};
    /** The manufacture level each kind needs (trapdoor.cfg's ManufactureLevel): the keeper's workshop slabs make the
     *  level -- above 9 slabs 1, above 16 2, above 25 3, above 36 4 (calculate_manufacture_level). */
    int64_t trap_levels[MAX_SLOTS] = {}, door_levels[MAX_SLOTS] = {};
    char door_names[MAX_SLOTS][24] = {};    /**< each door kind's code name, as place_door takes it */
    char trap_names[MAX_SLOTS][24] = {};    /**< each trap kind's code name, as place_trap takes it (Phase 4) */

    /** The slot of a kind; -1 if the stock has no such kind (or kind 0). */
    int64_t Slot(bool door, int64_t kind) const
    {
        if (kind <= 0) return -1;
        if (kind < MAX_KINDS) return (kind < (door ? door_kinds : trap_kinds)) ? kind : -1;
        const int64_t n = door ? door_extra : trap_extra;
        for (int64_t i = 0; i < n; i++) if ((door ? door_extra_kind[i] : trap_extra_kind[i]) == kind) return MAX_KINDS + i;
        return -1;
    }
    /** The kind in a slot. */
    int64_t KindAt(bool door, int64_t slot) const
    {
        return (slot < MAX_KINDS) ? slot : (door ? door_extra_kind[slot - MAX_KINDS] : trap_extra_kind[slot - MAX_KINDS]);
    }
    /** Every kind's slot from kind 1 on: f(slot, kind). */
    template <typename F> void ForEachKind(bool door, F f) const
    {
        const int64_t dense = door ? door_kinds : trap_kinds, extra = door ? door_extra : trap_extra;
        for (int64_t s = 1; s < dense; s++) f(s, s);
        for (int64_t i = 0; i < extra; i++) f(MAX_KINDS + i, door ? door_extra_kind[i] : trap_extra_kind[i]);
    }
    const char *Name(bool door, int64_t kind) const
    {
        const int64_t s = Slot(door, kind);
        return (s < 0) ? "" : (door ? door_names[s] : trap_names[s]);
    }
    int64_t Placeable(bool door, int64_t kind) const
    {
        const int64_t s = Slot(door, kind);
        return (s < 0) ? 0 : (door ? doors_placeable[s] : traps_placeable[s]);
    }
    bool Buildable(bool door, int64_t kind) const
    {
        const int64_t s = Slot(door, kind);
        return (s >= 0) && (door ? doors_buildable[s] : traps_buildable[s]);
    }
};

/** The keeper's human's combat-assistant rules (09 section 5.4, P7.12): the live set, as the simulation's runner would
 *  run them, and whether the keeper's Daimon assistant runs them instead (PckA_AssistDelegate). */
struct DaiHumanFightRules {
    bool enabled = false;               /**< the human has the combat assistant on */
    bool delegated = false;             /**< executed_by_daimon: the runner skips the keeper */
    KeeperAssistGlobals globals{};
    KeeperAssistRule rules[KEEPER_ASSIST_RULES] = {};
};

/** What the keeper's human is doing now (09 section 5.2a, Daimon P7.10; keeper_exclusion.h): the things never to touch
 *  -- the possessed creature and the group it leads, the hand's things, the thing under the cursor -- and the possessed
 *  creature's place, no cast or drop within `radius` subtiles of it. Empty for a keeper no human plays. */
struct DaiLiveExclusion {
    static constexpr int64_t MAX = 32;
    DaiThingIndex possessed = 0;
    DaiSubtlCoord px = -1, py = -1;
    int64_t radius = 0;
    DaiThingIndex things[MAX] = {};
    int64_t n = 0;
    bool Excludes(DaiThingIndex t) const
    {
        if (t == 0) return false;
        for (int64_t i = 0; i < n; i++) if (things[i] == t) return true;
        return false;
    }
    bool NearPossessed(DaiSubtlCoord x, DaiSubtlCoord y) const
    {
        if (possessed == 0) return false;
        const int64_t dx = (x > px) ? x - px : px - x, dy = (y > py) ? y - py : py - y;
        return ((dx > dy) ? dx : dy) <= radius;
    }
};

/** A thing the keeper is shown: another keeper's creature it can see, a dungeon heart on revealed ground. */
struct DaiThingLook {
    DaiThingRef ref;
    bool heart = false;         /**< a dungeon heart; otherwise a creature */
    DaiThingModel model = 0;
    PlayerNumber owner = -1;
    DaiSubtlCoord x = 0, y = 0;
    int64_t level = 0;          /**< creatures, 1-based */
    int64_t force = 0;          /**< creatures: creature_force_score (Phase 4) */
    int64_t health_pct = 100;   /**< creatures: of its maximum */
    bool digger = false;        /**< an imp (or another special digger) */
    int64_t battle = 0;         /**< the battle it fights in (0: none) */
    int64_t scavenge_required = 0;  /**< another keeper's creature the keeper's scavengers may lure: the points it
                                     *   needs (x256); 0 none (in custody, in a hand, a hero, an imp) (08 section 7a) */
};

/** A battle with some of the keeper's creatures in it (Phase 4, 06 section 3.1): its own side, and the enemies in it the
 *  keeper can see. */
struct DaiBattleLook {
    int64_t index = 0;          /**< the game's battle (changes as battles end and begin) */
    DaiSubtlCoord x = 0, y = 0; /**< where its own fighters are, on average */
    int64_t own_count = 0, own_force = 0;
    int64_t enemy_count = 0, enemy_force = 0;   /**< visible ones only */
};

/** One of the keeper's traps (Phase 4). A placed trap is a blueprint until an imp brings its crate. */
struct DaiTrapLook {
    DaiThingRef ref;
    DaiThingModel model = 0;
    DaiSubtlCoord x = 0, y = 0;
    int64_t shots = 0;          /**< shots left: 0 until armed, and once spent */
    bool armed = false;         /**< it can fire now (shots and not rearming) */
};

/** One of the keeper's doors (Phase 4). */
struct DaiDoorLook {
    DaiThingRef ref;
    DaiThingModel model = 0;
    DaiSubtlCoord x = 0, y = 0;
    int64_t health = 0, max_health = 0;
    bool locked = false;
};

/** The keeper's own Sight of Evil (Phase 5, 07 section 7): open or not, where, and how many subtiles it has opened.
 *  The game reveals that ground only while drawing it (and restores the reveal bits after), so what the sight shows is
 *  read from the sight's own flags (power_sight_explored), never by revealing. */
struct DaiSightLook {
    bool active = false;
    DaiSubtlCoord x = 0, y = 0;
    int64_t open = 0;
};

/** Another keeper's (or the heroes') trap or door the keeper sees (Phase 5, 07 section 3.1), and a neutral trap (P5.13:
 *  a hazard): a hidden trap only once it is revealed, a secret door only once it is not hidden from the keeper; on
 *  revealed ground or in its Sight of Evil. */
struct DaiEnemyDefenceLook {
    DaiThingRef ref;
    PlayerNumber owner = -1;
    bool door = false;
    DaiThingModel model = 0;
    DaiSubtlCoord x = 0, y = 0;
    int64_t health = 0;
    bool locked = false;
    bool spent = false;         /**< a trap with no shots left: drawn transparent, as a human sees it (P5.13) */
};

/** A room of another keeper's (or the heroes') the keeper has seen some of (Phase 5): only its slabs the keeper sees
 *  count, on revealed ground or in its Sight of Evil. */
/** Another player as the alliance panel shows it to every player (07 section 5a): public, not under the fog. */
/** A dungeon special box the keeper's hand could pick up and use now (P9). */
struct DaiSpecialLook {
    DaiThingRef ref;
    DaiSpecial kind = DaiSpecial::None;
    DaiSubtlCoord x = 0, y = 0;
};

/** A dead creature of the keeper's that a Resurrect box could bring back (the dungeon's list of the dead). */
struct DaiDeadLook {
    DaiThingModel model = 0;
    int64_t level = 1;          /**< 1-based */
};

struct DaiAllianceLook {
    bool keeper = false;        /**< another keeper still in the game (an ally too) */
    bool ours = false;          /**< p has declared alliance with it */
    bool theirs = false;        /**< it has declared alliance with p */
    bool locked = false;        /**< the level forbids changing it (ALLIANCE_LOCKED) */
    int64_t creatures = 0;      /**< its creatures, imps not counted, and its rooms: the query panel's row (P9) */
    int64_t rooms = 0;
};

struct DaiRivalRoomLook {
    int64_t index = 0;          /**< the game's room */
    PlayerNumber owner = -1;
    RoomKind kind = 0;
    int64_t slabs_seen = 0;
    DaiRect box;                /**< of the slabs seen */
    DaiSlabCoord x = 0, y = 0;  /**< a slab of it seen (the first) */
};

class IDaiWorldReader {
public:
    virtual ~IDaiWorldReader() = default;
    virtual void MapSize(DaiSlabCoord *w, DaiSlabCoord *h) const = 0;
    virtual void Slab(PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y, DaiSlabLook *out) const = 0;
    virtual void Own(PlayerNumber p, DaiOwnLook *out) const = 0;
    virtual int64_t OwnRooms(PlayerNumber p, DaiRoomLook *out, int64_t max) const = 0;
    /** The game's index of the keeper's own room on that slab; 0 if none. */
    virtual int64_t OwnRoomAt(PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y) const = 0;
    /** The keeper's own creatures, imps apart, from the `from`-th on; how many were written. */
    virtual int64_t OwnCreatures(PlayerNumber p, DaiCreatureLook *out, int64_t max, int64_t from) const = 0;
    /** Creature kinds the game has (1 .. KindCount()). */
    virtual int64_t KindCount() const = 0;
    virtual void Kind(PlayerNumber p, DaiThingModel m, DaiKindLook *out) const = 0;
    virtual void Research(PlayerNumber p, DaiResearchLook *out) const = 0;
    /** The slabs of the keeper's own libraries (rooms storing spellbooks) that hold one of its spellbooks, as a human
     *  sees them on the floor; how many were written. Selling such a slab moves its book or loses its spell. */
    virtual int64_t OwnBookSlabs(PlayerNumber p, DaiSlabXY *out, int64_t max) const = 0;
    /** The keeper's mature chickens in its own hatcheries; how many were written. */
    virtual int64_t OwnFood(PlayerNumber p, DaiFoodLook *out, int64_t max) const = 0;
    virtual void Stock(PlayerNumber p, DaiStockLook *out) const = 0;
    /** The keeper's custody and special rooms (Phase 6). */
    virtual void Custody(PlayerNumber p, DaiCustodyLook *out) const = 0;
    /** The temple recipes, in the table's order (the order the engine tries them); at most `max`. */
    virtual int64_t Recipes(DaiRecipeLook *out, int64_t max) const = 0;
    /** The job a hand drop of the keeper's creature `c` on slab (x, y) would give it (the game's CreatureJob bits);
     *  0 none: refused, the wrong place, no capacity (04 section 2.4, predict_hand_drop_job). */
    virtual uint64_t PredictHandDropJob(PlayerNumber p, const DaiThingRef &c, DaiSlabCoord x, DaiSlabCoord y) const = 0;
    /** Other keepers' things the keeper is shown in the slab rectangle (its own are in Own / OwnRooms). */
    virtual int64_t VisibleThings(PlayerNumber p, const DaiRect &area, DaiThingLook *out, int64_t max) const = 0;
    /** The battles the keeper's creatures fight in (enemies counted only when the keeper sees them; Phase 4). */
    virtual int64_t Battles(PlayerNumber p, DaiBattleLook *out, int64_t max) const = 0;
    /** The hero gates the keeper sees (revealed, not hidden by a script), as slabs; how many were written (Phase 4,
     *  06 section 3.2: where heroes come from). */
    virtual int64_t HeroGates(PlayerNumber p, DaiSlabXY *out, int64_t max) const = 0;
    /** The keeper's traps and doors (Phase 4). */
    virtual int64_t OwnTraps(PlayerNumber p, DaiTrapLook *out, int64_t max) const = 0;
    virtual int64_t OwnDoors(PlayerNumber p, DaiDoorLook *out, int64_t max) const = 0;
    /** The keeper's own Sight of Evil, and whether it shows a subtile now (Phase 5, 07 section 7). */
    virtual bool SightOfEvil(PlayerNumber p, DaiSightLook *out) const = 0;
    virtual bool SightShows(PlayerNumber p, DaiSubtlCoord x, DaiSubtlCoord y) const = 0;
    /** Other keepers' (and the heroes') traps and doors the keeper sees; how many were written (Phase 5). */
    virtual int64_t EnemyDefences(PlayerNumber p, DaiEnemyDefenceLook *out, int64_t max) const = 0;
    /** Other keepers' (and the heroes') rooms the keeper sees some of; how many were written (Phase 5). */
    virtual int64_t RivalRooms(PlayerNumber p, DaiRivalRoomLook *out, int64_t max) const = 0;
    /** Whether the keeper could cast the power now: on the thing when `target` names one, else at the subtile (the
     *  game's read-only checks, can_cast_power_on_thing / can_cast_power_at_xy; Phase 4, 06 section 7). */
    virtual bool PowerCastable(PlayerNumber p, DaiPowerKind k, DaiSubtlCoord x, DaiSubtlCoord y, const DaiThingRef *target) const = 0;
    /** Whether the power cast now would take on the creature `target`: castable there and on it, and the creature not
     *  dying, not already under the power's spell and not immune to it (the combat assistant's can_target,
     *  keeper_assist_power_takes; Phase 4, 06 section 7). False for a creature the keeper cannot see. */
    virtual bool PowerTakes(PlayerNumber p, DaiPowerKind k, const DaiThingRef &target) const = 0;
    /** An area power's reach from its target at `level`, map units (0: not an area power); *own_too: it hurts the
     *  caster's own creatures too, not only its allies' (keeper_assist_area_range). The game's config, no fog. */
    virtual int64_t PowerAreaRange(DaiPowerKind k, int64_t level, bool *own_too) const = 0;
    /** The game's power for one of Daimon's (0 if the game has none of that name). */
    virtual DaiPowerKind PowerOf(DaiPower d) const = 0;
    /** What the keeper's human is doing now (Daimon P7.10). Fakes may leave it empty. */
    virtual void LiveExclusion(PlayerNumber p, DaiLiveExclusion *out) const { (void)p; *out = DaiLiveExclusion(); }
    /** The keeper's combat-assistant rules (P7.12). Fakes may leave them off. */
    virtual void HumanFightRules(PlayerNumber p, DaiHumanFightRules *out) const { (void)p; *out = DaiHumanFightRules(); }
    /** What the content catalogue reads of every power, trap and door kind the level's configs define (Phase 7,
     *  DaiContent): the game's config, no fog. Fakes may leave them empty. */
    virtual void ContentSpecs(std::vector<DaiPowerSpec> *powers, std::vector<DaiTrapSpec> *traps,
                              std::vector<DaiDoorSpec> *doors) const { (void)powers; (void)traps; (void)doors; }
    /** One past the highest power kind (magic.cfg's count): kinds 1 .. this - 1. */
    virtual int64_t PowerKindCount() const = 0;
    virtual bool PowerAvailable(PlayerNumber p, DaiPowerKind k) const = 0;
    virtual int64_t PowerPrice(PlayerNumber p, DaiPowerKind k, int64_t level) const = 0;
    /** The keeper's own dig marks (slabs, in its task list's order); how many were written. */
    virtual int64_t OwnMarks(PlayerNumber p, DaiSlabXY *out, int64_t max) const = 0;
    /** The keeper's own imps, in its digger list's order; how many were written. */
    /** The special boxes p's hand can reach (P9), and p's dead a Resurrect box could bring back. */
    virtual int64_t SpecialBoxes(PlayerNumber p, DaiSpecialLook *out, int64_t max) const = 0;
    virtual int64_t DeadCreatures(PlayerNumber p, DaiDeadLook *out, int64_t max) const = 0;
    virtual int64_t OwnImps(PlayerNumber p, DaiImpLook *out, int64_t max) const = 0;
    /** The game's room for one of Daimon's (0 if none). */
    virtual RoomKind RoomOf(DaiRoom d) const = 0;
    virtual DaiBridgeRoles BridgeRoles() const = 0;
    virtual bool RoomAvailable(PlayerNumber p, RoomKind k) const = 0;
    virtual int64_t RoomPrice(PlayerNumber p, RoomKind k) const = 0;
    /** `r` is another keeper still in the game and not `p`'s ally: what the scoreboard tells every player (05 section
     *  6.3: whose ground and strength count for contest; heroes, neutral and idle seats do not). */
    virtual bool RivalKeeper(PlayerNumber p, PlayerNumber r) const = 0;
    /** `r` as `p`'s alliance panel shows it (07 section 5a). */
    virtual void Alliance(PlayerNumber p, PlayerNumber r, DaiAllianceLook *out) const = 0;
    /** The sees-all cheat (05 section 7.3): `p`'s reads ignore the fog, and the fog audit leaves them alone. Set by its
     *  keeper every turn from its difficulty; no simulation change. */
    virtual void SetSeesAll(PlayerNumber p, bool on) = 0;
    /** The knows-pool cheat (08 Q8): `p`'s kind looks carry the creature pool's counts. Set with sees all. */
    virtual void SetKnowsPool(PlayerNumber p, bool on) = 0;
    /** What changed for the keeper since the last call (the keeper-facts tracker, owned by the reader per keeper). */
    virtual int64_t Facts(PlayerNumber p, DaiTurn now, KeeperFact *out, int64_t max) = 0;
    /** The next Facts() takes a new baseline (a load, a new level). */
    virtual void Rebaseline(PlayerNumber p) = 0;
};

#endif
