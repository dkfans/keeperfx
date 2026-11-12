/** @file DaiDifficulty.h
 *  How well a keeper plays, apart from what it is like (05 section 7): its intervals and search budgets scaled, a
 *  deterministic decision noise, creatures' needs seen to late. Resolved at the take from daimon.toml's
 *  [difficulty.<id>] over the built-in easy, normal and hard, and saved with the keeper (a later edit of the file does
 *  not change a saved one). The pace cap (P3.5) and the cheats (P3.6) join it later. */
#ifndef DAIMON_DAIDIFFICULTY_H
#define DAIMON_DAIDIFFICULTY_H

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "daimon/DaiPersonality.h"
#include "daimon/DaiTypes.h"

struct DaimonConfig;
struct DaimonGlobalConfig;

/** The cheats (05 section 7.3): named, shown, never part of a fair level. Sees all and knows the pool (08 Q8: the
 *  creature pool's counts, which a human does not see) live only in Daimon's reader; rich start and stronger creatures
 *  change the game, so the host sends them once as PckA_DaimonCheats. */
constexpr uint64_t DAI_CHEAT_SEES_ALL = 1, DAI_CHEAT_RICH_START = 2, DAI_CHEAT_STRONGER = 4, DAI_CHEAT_KNOWS_POOL = 8;
/** The cheats that change the simulation (the packet's flags). */
constexpr uint64_t DAI_CHEATS_SIM = DAI_CHEAT_RICH_START | DAI_CHEAT_STRONGER;

class DaiDifficulty {
public:
    /** One key of [difficulty.<id>]: its member, range, reader; `scale` DAI_FIX_ONE for a decimal (as DaiPersonality). */
    struct Field {
        const char *key;
        int64_t DaiDifficulty::*member;
        int64_t min, max;
        const char *reader;
        int64_t scale = 1;
        /** A name field: the file gives one of these names (nullptr-terminated), stored as its index. */
        const char *const *names = nullptr;
    };

    /** The director's, the programme's and Excavation's rescore intervals times this (DaiFix: 2.0 reacts half as
     *  often), Creatures' work trips a 1000 turns divided by it. */
    int64_t interval_scale = 1024;
    /** The search budgets (reach visits, dig search nodes, layout sites) times this (DaiFix). */
    int64_t budget_scale = 1024;
    /** The share of the arbiter's choices (DaiFix) that go to a domain's second-best job, drawn from Daimon's random. */
    int64_t noise = 0;
    /** Turns a creature's need (anger, wounds) waits before Creatures sees to it. */
    int64_t care_delay = 0;
    /** The arbiter's pace cap (05 section 7.2): 0 relaxed, 1 human, 2 unlimited -- [pace]'s gestures a 1000 turns. */
    int64_t pace = 1;

    static std::span<const Field> Fields();
    /** The difficulty `name` (NULL or empty: "normal"; any case): the built-in values of easy, normal or hard, then the
     *  file's [difficulty.<name>] keys, clamped and logged; cheats follow the level's name after '+' ("hard+sees_all+
     *  rich_start+stronger_creatures"). False with a reason in *error for a level that is neither built in nor in the
     *  file, or a cheat it does not know. */
    static bool Resolve(const DaimonConfig &cfg, const char *name, DaiDifficulty *out, std::string *error);
    /** Keys of the file's difficulty tables this build does not know (as DaiPersonality::CheckKeys). */
    static std::vector<DaiPersonality::KeyIssue> CheckKeys(const DaimonConfig &cfg);
    /** This keeper's config and budgets for a turn: the global ones scaled (05 section 15, P3.4's intervals). */
    void Apply(DaimonGlobalConfig *cfg, DaiBudget *budget) const;
    /** World gestures this keeper may make in any 1000 turns. */
    int64_t PaceCap(const DaimonGlobalConfig &cfg) const;

    const std::string &GetId() const { return m_id; }
    void SetId(const std::string &id) { m_id = id; }
    /** DAI_CHEAT_* (not a file key: chosen with the level, saved with it). */
    uint64_t Cheats() const { return m_cheats; }
    void SetCheats(uint64_t c) { m_cheats = c & (DAI_CHEAT_SEES_ALL | DAI_CHEAT_KNOWS_POOL | DAI_CHEATS_SIM); }
    /** "sees all, rich start" (empty without cheats). */
    std::string CheatsText() const;
    /** The name Resolve reads back: the level, then "+cheat" for each cheat ("hard+rich_start"). For every
     *  round-trip (a takeover's return record, own.ai, a same-settings test); CheatsText is for display. */
    std::string Spec() const;
private:
    std::string m_id = "normal";
    uint64_t m_cheats = 0;
};

#endif
