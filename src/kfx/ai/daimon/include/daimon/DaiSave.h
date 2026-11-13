/** @file DaiSave.h
 *  daimon.bin, Daimon's entry in a saved game (02 section 4.15, 01 section 9): every keeper this machine plays, with
 *  what cannot be rebuilt from the game -- identity, personality values, random stream, the knowledge (map, scan
 *  place, rivals), inbox, goals, jobs, claims, the arbiter, each manager's own state and the slot controller's
 *  gesture in progress. Not kept: the keeper-facts trackers (the world reader's; a load takes a new baseline) and the
 *  profile counters.
 *
 *  Format: header {'DAIM', major, minor, keepers}, then one 'KEEP' section a keeper, whose body holds the keeper's
 *  fields and its own sections. A different major is refused; within the major an unknown section is skipped and a
 *  missing one leaves its part as a new keeper has it. Every count, length and enum is checked: the bytes come from a
 *  file. */
#ifndef DAIMON_DAISAVE_H
#define DAIMON_DAISAVE_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "config_daimon.h"
#include "daimon/DaiKeeper.h"
#include "daimon/IDaiActor.h"
#include "daimon/IDaiWorldReader.h"

/** 2 (the pre-merge review, 14 F1-F2): every section at version 1, a version per manager and per job kind, the mode's
 *  state in its own MODE section. A save of another major hands its keepers to the built-in AI. */
constexpr uint16_t DAI_SAVE_MAJOR = 2;
constexpr uint16_t DAI_SAVE_MINOR = 0;

/** A keeper read back, with its slot's controller queue to put back once its slot is attached. */
struct DaiLoadedKeeper {
    std::unique_ptr<DaiKeeper> keeper;
    bool has_queue = false;
    std::vector<uint8_t> queue;
};

class DaiSave {
public:
    /** The blob for these keepers (their slots' queues through `actor`). */
    static std::vector<uint8_t> Write(const std::vector<const DaiKeeper *> &keepers, const IDaiActor &actor);
    /** The keepers of a blob, built over `world` and `actor` (nothing is installed or attached); false with the reason
     *  in *why on any error, and *out is then empty. */
    static bool Read(const uint8_t *data, size_t len, const DaimonConfig &cfg, IDaiWorldReader &world, IDaiActor &actor,
                     std::vector<DaiLoadedKeeper> *out, std::string *why);
    /** A keeper read from the network host's last copy, made fit for the game as it is now (01 section 10.6): every
     *  running job is ready again (its gesture, planned for a game up to an interval old, is not resumed), every claim
     *  goes with it (claims last a gesture), and the results the copy expected are forgotten. Its knowledge stays: the
     *  rolling scan brings it up to date within a pass, the facts tracker takes a new baseline. */
    static void ReconcileAfterHostLost(DaiKeeper &k, DaiTurn now);
private:
    static void write_keeper(DaiBlobWriter &w, const DaiKeeper &k, const IDaiActor &actor);
    static bool read_keeper(DaiBlobReader &r, const DaimonConfig &cfg, IDaiWorldReader &world, IDaiActor &actor,
                            DaiLoadedKeeper *out, std::string *why);
    static void write_goals(DaiBlobWriter &w, const DaiGoals &g);
    static bool read_goals(DaiBlobReader &r, DaiGoals &g);
    static void write_jobs(DaiBlobWriter &w, const DaiJobPool &jobs);
    static bool read_jobs(DaiBlobReader &r, DaiJobPool &jobs);
    static void write_ledger(DaiBlobWriter &w, const DaiLedger &l);
    static void write_pipeline(DaiBlobWriter &w, const DaiKeeper &k);
    static bool read_pipeline(DaiBlobReader &r, DaiKeeper &k);
    static bool read_ledger(DaiBlobReader &r, DaiLedger &l);
};

#endif
