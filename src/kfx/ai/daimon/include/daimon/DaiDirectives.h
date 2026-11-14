/** @file DaiDirectives.h
 *  An agent's directives to its lieutenant (docs/plans/ai-plans/v2/10 section 4, P8.4; 14 K5): each checked into the
 *  goal it becomes (source Directive, above the director's and a script's), set, updated by id, expired, read back as
 *  a status, and set again on a takeover keeper and on the lieutenant back (P8.8). The list itself is the lieutenant
 *  mode's (DaiModeLieutenant::Directives, saved with it); this is what is done with it, apart from the game: a check
 *  needs only the map's size, the game's room kinds and the keeper's own number (DaiDirectiveMap). */
#ifndef DAIMON_DAIDIRECTIVES_H
#define DAIMON_DAIDIRECTIVES_H

#include <array>
#include <cstdint>
#include <vector>

#include "daimon/DaiBlob.h"
#include "daimon/DaiGoals.h"
#include "daimon/DaiTypes.h"

/** A directive as it arrived and what it became -- a goal (`goal_kind`, `goal_key`; source Directive) or a setting
 *  carried out at once (`goal_kind` Count) -- and when it came and lapses. Its status is read from its goal: open or in
 *  progress active, met done, blocked blocked. */
struct DaiDirective {
    static constexpr size_t ID_LEN = 32, KIND_LEN = 16;
    char id[ID_LEN] = {};
    char kind[KIND_LEN] = {};
    DaiGoalKind goal_kind = DaiGoalKind::Count;
    int64_t goal_key = 0;
    DaiGoalParams params;               /**< the goal's whole parameters and weight: set again after a takeover (P8.8) */
    DaiFix weight = 0;
    DaiTurn arrived = 0, expires = 0;  /**< expires 0: never */
    bool expired = false;
};

/** A directive as the agent sent it (the facade's DaimonDirectiveSpec). */
struct DaiDirectiveIn {
    const char *id = "", *kind = "", *objective = "", *action = "";
    int64_t room_kind = 0, count = 0, x = 0, y = 0, rival = 0, imps = 0, paydays = 0, priority = 0, expires_turn = 0;
};

/** What a check needs of the game. */
struct DaiDirectiveMap {
    PlayerNumber self = 0;
    int64_t width = 0, height = 0;              /**< in slabs */
    std::array<int64_t, (size_t)DaiRoom::Count> rooms{};   /**< the game's room kind of each of Daimon's rooms (0: none) */
};

/** A setting a directive makes at once (no goal). */
enum class DaiDirectiveEffect : uint8_t { None, PerimeterOpen, PerimeterClose };

struct DaiDirectiveState {
    const DaiDirective *d = nullptr;
    const char *status = "";            /**< active, done, blocked, expired */
    const char *why = "";               /**< its goal's last word */
};

class DaiDirectives {
public:
    static constexpr size_t MAX = 32;
    /** A directive's goal weight: priority 1-4 (0: 2), over the director's own (a few tenths to one). */
    static DaiFix Weight(int64_t priority);
    /** One directive checked: its goal into *g (kind Count: none, *effect the setting). NULL or an error code
     *  (MISSING_ID, UNKNOWN_KIND, NOT_BUILT_YET, BAD_ROOM, BAD_POSITION, BAD_RIVAL, BAD_OBJECTIVE, BAD_ACTION, BAD_COUNT). */
    static const char *Check(const DaiDirectiveMap &m, const DaiDirectiveIn &d, DaiGoal *g, DaiDirectiveEffect *effect);
    /** The agent's set: all checked first, nothing changed unless all pass (*bad the index of the first that fails);
     *  then added or updated by id (replace: the list swapped), their goals set in `goals` and an old one's dropped.
     *  The effects to carry out, in order, into *effects. Each arrival is logged (directives are not packets). */
    static const char *Set(std::vector<DaiDirective> &list, const DaiDirectiveMap &m, const DaiDirectiveIn *d, int64_t n,
                           bool replace, bool dry_run, DaiTurn now, DaiGoals &goals, int64_t *bad,
                           std::vector<DaiDirectiveEffect> *effects);
    /** One with an expires_turn come (nothing to do otherwise: no settle needed). */
    static bool Due(const std::vector<DaiDirective> &list, DaiTurn now);
    /** Those past their expires_turn: marked expired, their goals dropped, logged. */
    static void Expire(std::vector<DaiDirective> &list, PlayerNumber self, DaiTurn now, DaiGoals &goals);
    /** Their goals set again (a takeover keeper, the lieutenant back): the live ones only. */
    static void Restore(const std::vector<DaiDirective> &list, DaiTurn now, DaiGoals &goals);
    /** Each in order with its status, read from its goal. */
    static std::vector<DaiDirectiveState> Statuses(const std::vector<DaiDirective> &list, const DaiGoals &goals);
    /** A list in a save (the lieutenant's MODE section, a takeover keeper's return record). */
    static void Save(const std::vector<DaiDirective> &list, DaiBlobWriter &w);
    static bool Load(std::vector<DaiDirective> *list, DaiBlobReader &r);
    /** True while the directive stands (not expired, its turn not come). */
    static bool Live(const DaiDirective &d, DaiTurn now);
private:
    static void drop_goal(const DaiDirective &o, DaiGoals &goals);
};

#endif
