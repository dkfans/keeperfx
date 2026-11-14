/** @file DaimonInternal.h
 *  What the facade's translation units share (14 K6): Daimon.cpp (the keeper table, takes and hand-backs, the level,
 *  saves, the turn), DaimonAssist.cpp, DaimonLieutenant.cpp and DaimonScript.cpp. Main-thread state, cleared with the
 *  keepers (forget_all); defined in Daimon.cpp. Root group only. */
#ifndef DAIMON_DAIMONINTERNAL_H
#define DAIMON_DAIMONINTERNAL_H

#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "daimon/Daimon.h"
#include "daimon/DaiActorKeeperCtl.h"
#include "daimon/DaiKeeper.h"
#include "daimon/DaiModeOpponent.h"
#include "daimon/DaiWorker.h"
#include "daimon/DaiWorldReaderSim.h"
#include "net_game.h"
#include "packets.h"
#include "player_data.h"

namespace daimon_detail {
/** The saved game's daimon.bin, waiting for DaimonAfterLoad (the players' slots are restored in between). */
struct StagedSave {
    bool present = false;       /**< the save had a daimon.bin */
    bool foreign = false;       /**< ... of a major this version does not read */
    std::vector<uint8_t> data;
};

/** A level script's goals (DAIMON_BUILD, DAIMON_ATTACK, DAIMON_DIG) read before the level start: set once the keeper
 *  is adopted. A Build goal carries the game's room kind in its key until then (`room_kind`). */
struct PendingGoal {
    DaiGoal goal;
    RoomKind room_kind = 0;
    bool drop_attacks = false;
};

constexpr DaiTurn DELEGATE_RETRY_TURNS = 200;
extern DaiWorldReaderSim s_world;
extern DaiActorKeeperCtl s_actor;
extern std::array<std::unique_ptr<DaiKeeper>, PLAYERS_COUNT> s_keepers;
extern DaiWorkers s_workers;
extern bool s_workers_started;
extern DaimonDriverStats s_driver;
extern DaiTurn s_replicated_turn;
extern bool s_replicate_now;
extern bool s_level_started;
bool authors_keepers();
extern StagedSave s_staged;
extern std::array<std::string, PLAYERS_COUNT> s_pending_personality;
extern std::array<std::string, PLAYERS_COUNT> s_pending_difficulty;
extern std::array<std::vector<std::pair<std::string, std::string>>, PLAYERS_COUNT> s_pending_sets;
std::string lower(const char *s);
extern std::array<std::vector<PendingGoal>, PLAYERS_COUNT> s_pending_goals;
void clear_pending();
bool set_script_goal(PlayerNumber plyr, const PendingGoal &pg);
bool set_key(PlayerNumber plyr, const std::string &key, const std::string &value);
bool player_ok(PlayerNumber plyr);
uint64_t keeper_seed(PlayerNumber plyr);
DaiPersonality resolve_or_default(PlayerNumber plyr, const char *name);
std::string difficulty_label(const DaiDifficulty &d);
void request_sim_cheats(PlayerNumber plyr, const DaiDifficulty &d);
DaiDifficulty difficulty_or_normal(PlayerNumber plyr, const char *name);
std::string pick_personality(PlayerNumber plyr, const char *name, bool *random);
void adopt(PlayerNumber plyr, const char *personality, DaimonTakeReason why, const char *difficulty = nullptr);
std::string settle(PlayerNumber plyr);
int64_t turn_rate();
void settle_all();
void start_workers();
void stop_workers();
void drop_keeper(PlayerNumber plyr);
extern std::array<DaiTurn, PACKETS_COUNT> s_assist_wrote;
extern std::array<DaiTurn, PLAYERS_COUNT> s_delegate_sent;
extern std::array<DaiSeatReturn, PLAYERS_COUNT> s_claim_return;
extern bool s_before_claim_handed[PLAYERS_COUNT];
void fail_keeper(PlayerNumber plyr, const char *where, const char *what);
void forget_all();
void replicate(DaiTurn now);
void take_over_after_host_lost();
void adopt_flagged(DaimonTakeReason why);
}

#endif
