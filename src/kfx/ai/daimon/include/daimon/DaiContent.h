/** @file DaiContent.h
 *  The content catalogue (Phase 7, 09 section 13.3): every power, trap and door kind the level's configs define --
 *  a map pack's POWER_REAPER or HERO_SENTRY as much as Lightning -- with a role worked out from its config (its use
 *  function, spell, castability; a trap's shots, rating and health; a door's flags) and a worth, so that the managers
 *  can ask for a role, not a name. A role from daimon.toml's [content.power|trap|door.<NAME>] tables (role, worth)
 *  wins over the inferred one; those tables merge through the game's config chain (fxdata, mods, a campaign's or map
 *  pack's CONFIGS_LOCATION, the level's mapNNNNN.daimon.toml), so a pack sets its own.
 *  Built when a keeper starts (the configs are a level's for its whole run) and after a load; not saved. The pure
 *  half (the inference over spec structs) is here; the reader fills the specs (DaiWorldReaderSim). */
#ifndef DAIMON_DAICONTENT_H
#define DAIMON_DAICONTENT_H

#include "daimon_base_types.h"
#include <cstdint>
#include <vector>

#include "globals.h"

enum class DaiContentClass : uint8_t { Power, Trap, Door, Count };
enum class DaiContentRole : uint8_t {
    Ignore,         /**< never researched, made, placed or cast: a script marker, a power off the panel */
    // powers
    Summon,         /**< makes a creature (CreatureType, or a spell's summon) */
    Strike,         /**< damages the enemy where cast */
    Disable,        /**< a hostile spell on enemy creatures (slow, freeze, chicken, fear, disease, ...) */
    Boost,          /**< a friendly spell on own creatures (speed, armour, rebound, flight, invisibility, ...) */
    Heal,
    Utility,        /**< the keeper's other powers (hand, imp, sight, call to arms, cave-in, walls, obey, ...): each
                         its own manager's, by function */
    // traps
    Pressure,       /**< shoots or gasses what triggers it */
    Alarm,
    Boulder,        /**< rolls at its target */
    Terrain,        /**< changes the slab (lava) */
    Blocker,        /**< stands in the way with health and no damage (a barricade) */
    // doors
    Door,
    Secret,
    Midas,          /**< pays its owner's gold as it is hit */
    Count
};
const char *DaiContentRoleName(DaiContentRole r);
/** The role named (as daimon.toml writes it, lower case); Count if none. */
DaiContentRole DaiContentRoleNamed(const char *name);
const char *DaiContentClassName(DaiContentClass c);

/** The use functions magic.cfg names (magic_use_func_commands), Daimon's names for them. */
enum class DaiPowerFunc : uint8_t {
    None, Hand, ApplySpell, Slap, Possess, CallToArms, Lightning, Imp, Sight, CaveIn, DestroyWalls, Obey,
    HoldAudience, Armageddon, Tunneller, Other     /**< a Lua function or one this build does not name */
};

/** What the inference reads of a power kind. */
struct DaiPowerSpec {
    int64_t kind = 0;
    char name[32] = {};
    bool on_panel = false;          /**< PanelTabIndex > 0: a keeper can pick it */
    DaiPowerFunc func = DaiPowerFunc::None;
    bool enemy_creatures = false;   /**< castable on enemy creatures (ENEMY_CRTRS, ALL_CRTRS) */
    bool own_creatures = false;     /**< castable on own creatures (OWNED_CRTRS, ALL_CRTRS) */
    bool ground = false;            /**< castable on ground (own, enemy or all) */
    DaiThingModel creature = 0;        /**< CreatureType, or the spell's summon */
    int64_t creature_level = 0;     /**< the summon's level (the power's strength at its first level) */
    // Its spell (apply_spell), if any:
    bool spell_hostile = false;     /**< slows, freezes, turns to a chicken, frightens, diseases, maddens */
    bool spell_friendly = false;    /**< speeds, armours, rebounds, flies, hides, sees, lights */
    bool spell_heals = false;       /**< the HEAL flag */
    int64_t spell_damage = 0, spell_heal = 0;
    bool spell_shot = false;        /**< the spell fires a shot */
    int64_t cost = 0;               /**< at its first level */
    int64_t strength = 0;           /**< at its first level */
};

/** A trap's activation type: trapdoor.cfg's numbers (content, the same in every tree; the engine's TrpAcT_*). */
enum DaiTrapActivation : int64_t {
    DaiTrapAct_None = 0, DaiTrapAct_HeadforTarget90 = 1, DaiTrapAct_EffectonTrap = 2, DaiTrapAct_ShotonTrap = 3,
    DaiTrapAct_SlabChange = 4, DaiTrapAct_CreatureShot = 5, DaiTrapAct_CreatureSpawn = 6, DaiTrapAct_Power = 7,
};

struct DaiTrapSpec {
    int64_t kind = 0;
    char name[32] = {};
    bool on_panel = false;
    int64_t activation = 0;         /**< a DaiTrapActivation */
    int64_t shots = 0;              /**< 0: never triggers; 255: without end */
    int64_t rating = 0;             /**< DaiTrapRating */
    int64_t health = 0;
    bool destructible = false;
    bool gas = false;               /**< a gas cloud: hurts the keeper's own creatures too */
};

struct DaiDoorSpec {
    int64_t kind = 0;
    char name[32] = {};
    bool on_panel = false;
    int64_t health = 0;
    bool secret = false, midas = false;
    bool resist_non_magic = false;  /**< RESIST_NON_MAGIC: non-magical damage an eighth */
};

struct DaiContentEntry {
    DaiContentClass cls = DaiContentClass::Power;
    int64_t kind = 0;
    char name[32] = {};
    bool base = false;              /**< one of KeeperFX 1.4's own kinds (by name: IsBaseName) */
    DaiContentRole role = DaiContentRole::Ignore;
    bool overridden = false;        /**< role (or worth) from daimon.toml */
    int64_t worth = 0;              /**< power: strength at its first level; trap: rating (a blocker: health); door: health;
                                         or the override */
    int64_t cost = 0;               /**< power: at its first level */
    DaiThingModel creature = 0;        /**< a summon's kind */
    int64_t creature_level = 0;
    bool gas = false;               /**< trap: a gas cloud */
    // trap and door:
    int64_t health = 0;
    bool secret = false, midas = false, resist_non_magic = false;
};

/** daimon.toml's override for a kind: role Count keeps the inferred one; worth < 0 keeps the inferred one. */
struct DaiContentOverride {
    DaiContentClass cls = DaiContentClass::Power;
    char name[32] = {};
    DaiContentRole role = DaiContentRole::Count;
    int64_t worth = -1;
};

class DaiContent {
public:
    /** One of KeeperFX 1.4's own kinds, by its code name (the base fxdata's magic.cfg and trapdoor.cfg). */
    static bool IsBaseName(DaiContentClass c, const char *name);
    static DaiContentRole InferPower(const DaiPowerSpec &s);
    static DaiContentRole InferTrap(const DaiTrapSpec &s);
    static DaiContentRole InferDoor(const DaiDoorSpec &s);
    /** The overrides in the merged daimon.toml ([content.power|trap|door.<NAME>]), unknown roles warned and skipped. */
    static std::vector<DaiContentOverride> ReadOverrides();

    void Build(const std::vector<DaiPowerSpec> &powers, const std::vector<DaiTrapSpec> &traps,
               const std::vector<DaiDoorSpec> &doors, const std::vector<DaiContentOverride> &overrides);
    bool Built() const { return m_built; }
    void Clear() { *this = DaiContent(); }
    /** The entry for a game kind; nullptr if the configs define no such kind. */
    const DaiContentEntry *Find(DaiContentClass c, int64_t kind) const;
    const DaiContentEntry *FindNamed(DaiContentClass c, const char *name) const;
    const std::vector<DaiContentEntry> &Entries(DaiContentClass c) const { return m_entries[(size_t)c]; }
    /** The entries of a role, in kind order. */
    std::vector<const DaiContentEntry *> OfRole(DaiContentClass c, DaiContentRole r) const;
    /** One log line per kind that is not KeeperFX's own or whose role daimon.toml set (09 section 13.3). */
    void Log(PlayerNumber plyr) const;
private:
    bool m_built = false;
    std::vector<DaiContentEntry> m_entries[(size_t)DaiContentClass::Count];
};

#endif
