/** @file Daimon.cpp
 *  The C facade over Daimon's classes (Daimon.h): the keeper table, the take and hand-back, the level start, a load,
 *  the turn entry. Every entry point catches: a failure hands the keeper concerned back to the built-in AI and logs
 *  why, so Daimon can never take the game down with it (02 principle 9). */
#include "pre_inc.h"
#include "daimon/Daimon.h"
#include "DaimonInternal.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "config_daimon.h"
#include "dai_net.h"
#include "daimon_slots.h"
#include "daimon/DaiActorKeeperCtl.h"
#include "daimon/DaiDirector.h"
#include "daimon/DaiKeeper.h"
#include "daimon/DaiLog.h"
#include "daimon/DaiModeOpponent.h"
#include "daimon/DaiModeAssistant.h"
#include "daimon/DaiDirectives.h"
#include "daimon/DaiModeLieutenant.h"
#include "daimon/DaiSave.h"
#include "daimon/DaiWorker.h"
#include "daimon/DaiWorldReaderSim.h"
#include "keeper_ctl.h"
#include "game_legacy.h"
#include "game_legacy.h"
#include "packets.h"
#include "player_data.h"
#include "player_instances.h"
#include "bflib_basics.h"
#include "net_game.h"
#include "tasks_list.h"
#include "post_inc.h"

static_assert(DAI_MAX_PLAYERS == PLAYERS_COUNT, "DaiLimits.h's DAI_MAX_PLAYERS must be the game's PLAYERS_COUNT");
static_assert(DAI_MAX_MARKS == MAPTASKS_COUNT, "DaiLimits.h's DAI_MAX_MARKS must be the game's MAPTASKS_COUNT");
static_assert(DAIMON_SAVE_MAJOR == DAI_SAVE_MAJOR, "Daimon.h's DAIMON_SAVE_MAJOR must be DaiSave.h's DAI_SAVE_MAJOR");

namespace daimon_detail {
/** The game as every keeper reads it (one tracker per keeper inside). */
DaiWorldReaderSim s_world;
/** How every keeper acts: its slot's keeper controller. Declared before the keepers, which release through it. */
DaiActorKeeperCtl s_actor;
std::array<std::unique_ptr<DaiKeeper>, PLAYERS_COUNT> s_keepers;
/** The threads the keepers plan on (12 section 5), keyed by player; declared after the keepers so that at exit they
 *  finish and stop before any keeper goes. Started at the first turn a level plays, stopped when it ends. */
DaiWorkers s_workers;
bool s_workers_started = false;
/** The turn driver's counters; `last` keeps a turn from being played twice. */
DaimonDriverStats s_driver;
/** Network host: the turn of the last copy of Daimon's state sent to the clients, and whether one is due now (a keeper
 *  was taken or adopted). */
DaiTurn s_replicated_turn = 0;
bool s_replicate_now = false;
/** The level start has run (DaimonLevelStart, or a load): a script's take is a take now, not a queued one. */
bool s_level_started = false;

/** In a network game only the host plays Daimon's keepers. */
bool authors_keepers()
{
    return !network_is_active() || network_is_host();
}
StagedSave s_staged;
/** What the level start was asked for each keeper (DaimonQueueLevelStart). */
std::array<std::string, PLAYERS_COUNT> s_pending_personality;
std::array<std::string, PLAYERS_COUNT> s_pending_difficulty;
/** DAIMON_SET lines read before the level start: set once the keeper is adopted. */
std::array<std::vector<std::pair<std::string, std::string>>, PLAYERS_COUNT> s_pending_sets;

std::string lower(const char *s)
{
    std::string out = (s != nullptr) ? s : "";
    for (char &c : out) c = (char)tolower((unsigned char)c);
    return out;
}

std::array<std::vector<PendingGoal>, PLAYERS_COUNT> s_pending_goals;

/** Everything queued for the level start, forgotten. */
void clear_pending()
{
    for (auto &n : s_pending_personality) n.clear();
    for (auto &n : s_pending_difficulty) n.clear();
    for (auto &n : s_pending_sets) n.clear();
    for (auto &n : s_pending_goals) n.clear();
}

/** A pending goal onto the keeper: a Build goal's DaiRoom from the game's room kind; a rival of none drops the script's
 *  attacks. */
bool set_script_goal(PlayerNumber plyr, const PendingGoal &pg)
{
    DaiKeeper &k = *s_keepers[plyr];
    DaiGoal g = pg.goal;
    if (pg.drop_attacks) {
        std::vector<uint32_t> drop;
        k.ForEachGoal([&](const DaiGoal &o) { if ((o.kind == DaiGoalKind::Attack) && (o.source == DaiGoalSource::Script)) drop.push_back(o.id); });
        for (uint32_t id : drop) k.DropGoal(id);
        DaiLog::Info(plyr, "DAIMON_ATTACK: the script's attacks taken back");
        return true;
    }
    if (g.kind == DaiGoalKind::Build) {
        DaiRoom r = DaiRoom::Count;
        for (size_t i = 0; i < (size_t)DaiRoom::Count; i++)
            if (k.GetMainKnowledge().RoomKindOf((DaiRoom)i) == pg.room_kind) r = (DaiRoom)i;
        if (r == DaiRoom::Count) {
            DaiLog::Warn(plyr, "DAIMON_BUILD: room kind %d is not one Daimon builds", (int)pg.room_kind);
            return false;
        }
        const int64_t width = std::max<int64_t>(k.GetMainKnowledge().Width(), 1);
        g.p.key = DAI_SCRIPT_BUILD_KEY * (1 + g.p.at.y0 * width + g.p.at.x0) + (int64_t)r;
        g.p.count = (int64_t)(g.p.at.x1 - g.p.at.x0 + 1) * (g.p.at.y1 - g.p.at.y0 + 1);
    } else if ((g.kind == DaiGoalKind::ExpandTo) && (g.p.count == DAI_EXPAND_POINT)) {
        g.p.key = (int64_t)g.p.at.y0 * std::max<int64_t>(k.GetMainKnowledge().Width(), 1) + g.p.at.x0;
    }
    const uint32_t id = k.SetGoal(g, get_gameturn());
    DaiLog::Info(plyr, "script goal %d key %" PRId64 " count %" PRId64 " (%d,%d)-(%d,%d): id %u", (int)g.kind, g.p.key, g.p.count,
                 (int)g.p.at.x0, (int)g.p.at.y0, (int)g.p.at.x1, (int)g.p.at.y1, (unsigned)id);
    return id != 0;
}

bool set_key(PlayerNumber plyr, const std::string &key, const std::string &value)
{
    std::string err;
    int64_t was = 0, now = 0;
    if (!s_keepers[plyr]->SetPersonalityKey(key.c_str(), value.c_str(), &was, &now, &err)) {
        DaiLog::Warn(plyr, "DAIMON_SET %s %s: %s", key.c_str(), value.c_str(), err.c_str());
        return false;
    }
    DaiLog::Info(plyr, "DAIMON_SET %s %" PRId64 " -> %" PRId64, key.c_str(), was, now);
    return true;
}

bool player_ok(PlayerNumber plyr)
{
    return (plyr >= 0) && (plyr < PLAYERS_COUNT);
}


uint64_t keeper_seed(PlayerNumber plyr)
{
    return (uint64_t)game.action_random_seed ^ ((uint64_t)(plyr + 1) * 0x9E3779B97F4A7C15ULL);
}

DaiPersonality resolve_or_default(PlayerNumber plyr, const char *name)
{
    DaiPersonality p;
    std::string error;
    if (!DaiPersonality::Resolve(daimon_config, name, &p, &error)) {
        DaiLog::Warn(plyr, "%s; the default personality plays instead", error.c_str());
        DaiPersonality::Resolve(daimon_config, "default", &p, &error);
    }
    return p;
}

/** A difficulty as the log shows it: "hard (cheats: sees all, rich start)". */
std::string difficulty_label(const DaiDifficulty &d)
{
    const std::string cheats = d.CheatsText();
    return cheats.empty() ? d.GetId() : d.GetId() + " (cheats: " + cheats + ")";
}

/** A keeper's cheats that change the game reach every machine once, as the host's packet (05 section 7.3) -- not
 *  again for a keeper that already had them (a load, a take after a hand-back, a host lost). */
void request_sim_cheats(PlayerNumber plyr, const DaiDifficulty &d)
{
    const uint64_t sim = d.Cheats() & DAI_CHEATS_SIM;
    if ((sim == 0) || flag_is_set(get_player(plyr)->allocflags, PlaF_DaimonCheats)) return;
    const DaimonGlobalConfig &g = daimon_config.global;
    dai_net_request_cheats(plyr, sim, g.cheats_rich_start_gold, g.cheats_creature_bonus_pct, g.cheats_pay_discount_pct);
}

/** The difficulty `name`, or normal (logged) when there is none of that name. */
DaiDifficulty difficulty_or_normal(PlayerNumber plyr, const char *name)
{
    DaiDifficulty d;
    std::string error;
    if (!DaiDifficulty::Resolve(daimon_config, name, &d, &error)) {
        DaiLog::Warn(plyr, "%s; normal plays instead", error.c_str());
        DaiDifficulty::Resolve(daimon_config, "normal", &d, &error);
    }
    return d;
}

/** "random" (05 section 5): one of the personalities Skirmish may pick (skirmish_pool), drawn from the keeper's seed --
 *  the game's synced random at the take, the same on every machine; "default" when the pool is empty. Any other name
 *  as it is. */
std::string pick_personality(PlayerNumber plyr, const char *name, bool *random)
{
    *random = (name != nullptr) && (strcmp(name, "random") == 0);
    if (!*random) return (name != nullptr) ? name : "";
    DaimonPersonalityInfo list[DAIMON_MAX_PERSONALITIES];
    const size_t n = daimon_config_personality_list(list, DAIMON_MAX_PERSONALITIES);
    std::vector<std::string> pool;
    for (size_t i = 0; i < n; i++) if (list[i].skirmish_pool) pool.push_back(list[i].id);
    if (pool.empty()) return "default";
    return pool[(size_t)(keeper_seed(plyr) % pool.size())];
}

/** Builds the C++ keeper for a player whose slot is already Daimon's (the level start, a load, a take). */
void adopt(PlayerNumber plyr, const char *personality, DaimonTakeReason why, const char *difficulty)
{
    const NetUserId slot = daimon_slot_of(plyr);
    bool random = false;
    const std::string name = pick_personality(plyr, personality, &random);
    s_keepers[plyr] = std::make_unique<DaiKeeper>(plyr, slot, std::make_unique<DaiModeOpponent>(),
        resolve_or_default(plyr, name.empty() ? nullptr : name.c_str()), daimon_slot_prev_model(plyr), why, get_gameturn(),
        keeper_seed(plyr), s_world, s_actor);
    s_keepers[plyr]->SetRandomPersonality(random);
    s_keepers[plyr]->SetDifficulty(difficulty_or_normal(plyr, difficulty));
    request_sim_cheats(plyr, s_keepers[plyr]->GetDifficulty());
    s_actor.Attach(slot, s_keepers[plyr]->CtlWorld());
    DaiLog::Info(plyr, "Daimon plays this keeper from slot %" PRId64 " (%s, %s)", (int64_t)slot,
        s_keepers[plyr]->GetPersonality().GetId().c_str(), difficulty_label(s_keepers[plyr]->GetDifficulty()).c_str());
    for (const auto &kv : s_pending_sets[plyr]) set_key(plyr, kv.first, kv.second);
    s_pending_sets[plyr].clear();
    for (const PendingGoal &pg : s_pending_goals[plyr]) set_script_goal(plyr, pg);
    s_pending_goals[plyr].clear();
}

/** Keeper `plyr`'s planning turn has run and its output is taken: the main thread may touch the keeper again. "" or
 *  what the planning turn threw. */
std::string settle(PlayerNumber plyr)
{
    return s_keepers[plyr] ? s_keepers[plyr]->Settle() : std::string();
}

/** The turns a second the game runs as a player sees it: its speed times the fast-forward (0 as 1), which sets the
 *  planning delay D (12 section 13.8). */
int64_t turn_rate()
{
    return (int64_t)turns_per_second * std::max<int64_t>(local_state.fast_forward, 1);
}

void fail_keeper(PlayerNumber plyr, const char *where, const char *what);

/** Every keeper settled (anything outside the turn that reads or changes one comes here first); a planning turn that
 *  failed fails its keeper. */
void settle_all()
{
    for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) {
        const std::string err = settle(p);
        if (!err.empty() && s_keepers[p]) fail_keeper(p, "turn", err.c_str());
    }
}

/** The pool sized for the keepers Daimon plays now (12 section 5: one worker each, at most the cores less the
 *  reserved ones): started at a level's first turn, and started again, every keeper settled first, when the number of
 *  keepers changes (a take, a hand-back, a drop-in). */
void start_workers()
{
    int64_t keepers = 0;
    for (const auto &k : s_keepers) keepers += k ? 1 : 0;
    const DaimonGlobalConfig &g = daimon_config.global;
    const int64_t want = DaiWorkers::CountFor(g.threads_workers, g.threads_reserved_cores, keepers);
    if (s_workers_started && (want == s_workers.Count())) return;
    settle_all();
    s_workers_started = true;
    s_workers.Start(want, g.threads_jitter_us);
    JUSTMSG("DAI: planning on %" PRId64 " worker thread(s)%s for %" PRId64 " keeper(s); the game at %" PRId64 " turns a second x frame skip %" PRId64,
        s_workers.Count(), (s_workers.Count() == 0) ? " (inline)" : "", keepers, (int64_t)turns_per_second,
        (int64_t)local_state.fast_forward);
}

void stop_workers()
{
    s_workers.Stop();
    s_workers_started = false;
}

/** Drops a keeper: its claims and gestures go (DaiKeeper::Release), its slot's controller gets the default world. */
void drop_keeper(PlayerNumber plyr)
{
    if (!s_keepers[plyr]) return;
    settle(plyr);
    const NetUserId slot = s_keepers[plyr]->GetSlot();
    const bool lieutenant = (s_keepers[plyr]->GetMode().GetMode() == DaimonMode::Lieutenant);
    s_keepers[plyr]->Release();
    s_keepers[plyr].reset();
    // A lieutenant takes only its own lane's verbs away: the slot is the seat's, and the agent's verbs stay.
    if (lieutenant) s_actor.DetachLieutenant(slot);
    else s_actor.Detach(slot);
}

/** The turn the assistant last wrote an action into a user's packet (DaimonAssistantWroteAction: the touch log leaves
 *  it out). */
std::array<DaiTurn, PACKETS_COUNT> s_assist_wrote{};
/** A seat's delegation verb queued at this turn (0: none in flight), sent again after DELEGATE_RETRY_TURNS. */
std::array<DaiTurn, PLAYERS_COUNT> s_delegate_sent{};
/** Between DaimonBeforeSeatClaim and DaimonAfterSeatClaim (one claim_seat): the takeover keeper's return record (its
 *  mode's, saved with it, 14 F2) and whether that keeper was handed back. */
std::array<DaiSeatReturn, PLAYERS_COUNT> s_claim_return;
bool s_before_claim_handed[PLAYERS_COUNT] = {};

/** Something went wrong with a keeper: give it back to the built-in AI and forget it. In a network game the keeper's
 *  flags are every machine's: it only stops (its slot's packets stay empty), as a hand-back on one machine would split
 *  the game. */
void fail_keeper(PlayerNumber plyr, const char *where, const char *what)
{
    if (network_is_active()) {
        DaiLog::Warn(plyr, "%s failed (%s); the keeper stops (a network game: no hand-back)", where, what);
        if (player_ok(plyr)) drop_keeper(plyr);
        return;
    }
    if (player_ok(plyr) && s_keepers[plyr] && ((s_keepers[plyr]->GetMode().GetMode() == DaimonMode::Assistant)
                                               || (s_keepers[plyr]->GetMode().GetMode() == DaimonMode::Lieutenant))) {
        // The human's own keeper, or a seat's: the assistant (or lieutenant) stops, its owner plays on.
        DaiLog::Warn(plyr, "%s failed (%s); the %s stops", where, what, s_keepers[plyr]->GetMode().GetName());
        drop_keeper(plyr);
        return;
    }
    DaiLog::Warn(plyr, "%s failed (%s); the keeper goes back to the built-in AI", where, what);
    if (!player_ok(plyr)) return;
    const int64_t model = s_keepers[plyr] ? s_keepers[plyr]->GetPrevModel() : daimon_slot_prev_model(plyr);
    try {
        drop_keeper(plyr);
        daimon_release_slot(plyr, model);
    } catch (...) {
    }
}

void forget_all()
{
    s_driver = DaimonDriverStats{};
    s_replicated_turn = 0;
    s_replicate_now = false;
    // The facade's own per-seat records go with the keepers (14 R6): a new level or a load starts them afresh.
    s_assist_wrote.fill(0);
    s_delegate_sent.fill(0);
    for (DaiSeatReturn &r : s_claim_return) r = DaiSeatReturn();
    for (bool &h : s_before_claim_handed) h = false;
    for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) drop_keeper(p);
    stop_workers();
    s_world.Reset();
}

/** Network host: Daimon's state, as a saved game has it, to every client. */
void replicate(DaiTurn now)
{
    settle_all();
    std::vector<const DaiKeeper *> keepers;
    for (const auto &k : s_keepers) if (k) keepers.push_back(k.get());
    s_replicated_turn = now;
    s_replicate_now = false;
    if (keepers.empty()) return;
    const std::vector<uint8_t> blob = DaiSave::Write(keepers, s_actor);
    dai_net_send_state(now, blob.data(), blob.size());
}

/** A client that lost the host (now a local game, its Daimon keepers still on their slots): each keeper comes back
 *  from the newest copy of the host's state that reads, reconciled with the game as it is (01 section 10.6); one
 *  without a copy goes to the built-in AI. */
void take_over_after_host_lost()
{
    forget_all();
    s_world.Reset(); // the facts trackers take a new baseline
    std::vector<DaiLoadedKeeper> loaded;
    DaiTurn turn = 0;
    for (int64_t i = 0; i < DAI_NET_SNAPSHOTS; i++) {
        const uint8_t *data = nullptr;
        size_t len = 0;
        if (!dai_net_snapshot(i, &data, &len, &turn)) break;
        std::string why;
        if (DaiSave::Read(data, len, daimon_config, s_world, s_actor, &loaded, &why)) break;
        WARNLOG("DAI: the host's state of turn %" PRIu64 " is refused (%s); trying the one before", (uint64_t)turn, why.c_str());
        loaded.clear();
    }
    dai_net_reset();
    for (DaiLoadedKeeper &l : loaded) {
        const PlayerNumber p = l.keeper->GetPlayer();
        if ((daimon_slot_of(p) < 0) || (daimon_slot_of(p) != l.keeper->GetSlot()) || s_keepers[p]) {
            WARNLOG("DAI: the host's copy names player %d on slot %" PRId64 ", which is not this game's Daimon keeper there", (int)p,
                (int64_t)l.keeper->GetSlot());
            continue;
        }
        const NetUserId slot = l.keeper->GetSlot();
        DaiSave::ReconcileAfterHostLost(*l.keeper, get_gameturn());
        s_keepers[p] = std::move(l.keeper);
        DaiActorKeeperCtl::ResetSlot(slot);       // the copy's gesture in progress was planned for a game up to an interval old: dropped
        s_actor.Attach(slot, s_keepers[p]->CtlWorld());
        DaiLog::Info(p, "the host is gone: Daimon plays this keeper here from its state of turn %" PRIu64, (uint64_t)turn);
    }
    for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) {
        if ((daimon_slot_of(p) >= 0) && !s_keepers[p]) fail_keeper(p, "host loss", "no copy of its state from the host");
    }
}

/** Every PlaF_Daimon player gets its keeper (those that have none yet). */
void adopt_flagged(DaimonTakeReason why)
{
    for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) {
        if (s_keepers[p] || (daimon_slot_of(p) < 0)) continue;
        // A name the level start was given (Skirmish, a script), else in a network game the host's lobby choice.
        std::string name = s_pending_personality[p];
        if (name.empty() && network_is_active()) name = dai_net_opponents_personality();
        std::string diff = s_pending_difficulty[p];
        if (diff.empty() && network_is_active()) diff = dai_net_opponents_difficulty();
        adopt(p, name.empty() ? nullptr : name.c_str(), why, diff.empty() ? nullptr : diff.c_str());
    }
}
}

using namespace daimon_detail;

DaiWorldReaderSim &DaimonWorldReader()
{
    return s_world;
}

const DaiActorKeeperCtl &DaimonActor()
{
    return s_actor;
}

const DaimonDriverStats &DaimonDriver()
{
    return s_driver;
}

const DaiKeeper *DaimonKeeper(PlayerNumber plyr)
{
    settle_all();
    return player_ok(plyr) ? s_keepers[plyr].get() : nullptr;
}

uint32_t DaimonPostRequest(PlayerNumber plyr, DaiDomain from, const DaiRequest &r)
{
    settle_all();
    DaiKeeper *k = player_ok(plyr) ? s_keepers[plyr].get() : nullptr;
    return (k != nullptr) ? k->PostRequest(from, r, get_gameturn()) : 0;
}

uint32_t DaimonSetGoal(PlayerNumber plyr, const DaiGoal &g)
{
    settle_all();
    DaiKeeper *k = player_ok(plyr) ? s_keepers[plyr].get() : nullptr;
    return (k != nullptr) ? k->SetGoal(g, get_gameturn()) : 0;
}

extern "C" {

void DaimonInit(void)
{
    forget_all();
    s_level_started = false;
    s_staged = StagedSave{};
    clear_pending();
}

void DaimonQueueLevelStart(PlayerNumber plyr, const char *personality, const char *difficulty)
{
    if (!player_ok(plyr)) return;
    try {
        s_pending_personality[plyr] = (personality != nullptr) ? personality : "";
        s_pending_difficulty[plyr] = (difficulty != nullptr) ? difficulty : "";
        daimon_pending_slots_add(plyr);
    } catch (const std::exception &e) {
        DaiLog::Warn(plyr, "DaimonQueueLevelStart failed (%s)", e.what());
    } catch (...) {
        DaiLog::Warn(plyr, "DaimonQueueLevelStart failed");
    }
}

void DaimonQueueClear(void)
{
    daimon_pending_slots_clear();
    clear_pending();
}

void DaimonLevelStart(void)
{
    s_staged = StagedSave{}; // a load that did not finish leaves nothing for a new level
    s_level_started = true;
    if (replay.load_enable) {   // the recorded packets play the keepers; the header's authors claimed the slots
        DaimonQueueClear();
        return;
    }
    if (!authors_keepers()) {
        clear_pending();      // a client: the host plays them (it may take them over, 01 10.6)
        return;
    }
    s_replicate_now = true;
    try {
        adopt_flagged(DaimonTakeReason::Skirmish);
    } catch (const std::exception &e) {
        ERRORLOG("DAI: level start failed (%s); Daimon's keepers go back to the built-in AI", e.what());
        for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) if (daimon_slot_of(p) >= 0) fail_keeper(p, "level start", e.what());
    } catch (...) {
        ERRORLOG("DAI: level start failed; Daimon's keepers go back to the built-in AI");
        for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) if (daimon_slot_of(p) >= 0) fail_keeper(p, "level start", "unknown");
    }
    clear_pending();
}

void DaimonLevelEnd(void)
{
    settle_all();
    s_level_started = false;
    forget_all();
    s_staged = StagedSave{};
}

void DaimonAfterLoad(void)
{
    settle_all();
    s_level_started = true;
    forget_all();
    StagedSave staged = std::move(s_staged);
    s_staged = StagedSave{};
    if (replay.load_enable) return;
    try {
        if (!staged.present) {
            // A save without Daimon's data (made before it was saved): the keepers start afresh.
            adopt_flagged(DaimonTakeReason::Load);
            return;
        }
        std::string why = "a format this version does not read";
        std::vector<DaiLoadedKeeper> loaded;
        bool ok = !staged.foreign && DaiSave::Read(staged.data.data(), staged.data.size(), daimon_config, s_world, s_actor,
            &loaded, &why);
        // An assistant or a lieutenant (a save of another build) is not one of the game's Daimon keepers: left out.
        if (ok) {
            for (auto it = loaded.begin(); it != loaded.end();) {
                const DaimonMode m = it->keeper->GetMode().GetMode();
                if ((m != DaimonMode::Assistant) && (m != DaimonMode::Lieutenant)) { ++it; continue; }
                DaiLog::Warn(it->keeper->GetPlayer(), "the saved %s is not played in this build; it stays off",
                    (m == DaimonMode::Assistant) ? "assistant" : "lieutenant");
                it = loaded.erase(it);
            }
        }
        // Every saved keeper must be a Daimon keeper of the loaded game, on the slot the game gives it, and every
        // Daimon keeper must have been saved: the two halves of one save, or neither is used.
        int64_t flagged = 0;
        for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) flagged += (daimon_slot_of(p) >= 0);
        if (ok && ((int64_t)loaded.size() != flagged)) {
            ok = false;
            why = "it does not have the game's Daimon keepers";
        }
        for (const DaiLoadedKeeper &l : loaded) {
            if (ok && (daimon_slot_of(l.keeper->GetPlayer()) != l.keeper->GetSlot())) {
                ok = false;
                why = "a keeper's slot is not the game's";
            }
        }
        if (!ok) {
            ERRORLOG("DAI: the saved game's Daimon data is refused (%s); Daimon's keepers go back to the built-in AI", why.c_str());
            for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) {
                if (daimon_slot_of(p) >= 0) fail_keeper(p, "load", why.c_str());
            }
            return;
        }
        for (DaiLoadedKeeper &l : loaded) {
            const PlayerNumber p = l.keeper->GetPlayer();
            const NetUserId slot = l.keeper->GetSlot();
            s_keepers[p] = std::move(l.keeper);
            DaiActorKeeperCtl::ResetSlot(slot);
            s_actor.Attach(slot, s_keepers[p]->CtlWorld());
            if (l.has_queue && !s_actor.LoadQueue(slot, l.queue.data(), l.queue.size()))
                DaiLog::Warn(p, "the saved gesture in progress is malformed and dropped");
            DaiLog::Info(p, "Daimon plays this keeper again from slot %" PRId64 " (%s)", (int64_t)slot,
                s_keepers[p]->GetPersonality().GetId().c_str());
        }
    } catch (const std::exception &e) {
        ERRORLOG("DAI: rebuilding the keepers after a load failed (%s)", e.what());
        for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) if (daimon_slot_of(p) >= 0) fail_keeper(p, "load", e.what());
    } catch (...) {
        ERRORLOG("DAI: rebuilding the keepers after a load failed");
        for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) if (daimon_slot_of(p) >= 0) fail_keeper(p, "load", "unknown");
    }
}

size_t DaimonSaveBlob(char **out)
{
    settle_all();
    *out = nullptr;
    try {
        std::vector<const DaiKeeper *> keepers;
        for (const auto &k : s_keepers) if (k) keepers.push_back(k.get());
        if (keepers.empty()) return 0;
        const std::vector<uint8_t> blob = DaiSave::Write(keepers, s_actor);
        *out = (char *)malloc(blob.size());
        if (*out == nullptr) return 0;
        memcpy(*out, blob.data(), blob.size());
        return blob.size();
    } catch (...) {
        ERRORLOG("DAI: Daimon's state could not be saved; a load of this save gives its keepers to the built-in AI");
        free(*out);
        *out = nullptr;
        return 0;
    }
}

void DaimonStageLoad(const char *data, size_t len, int64_t version)
{
    s_staged = StagedSave{};
    try {
        s_staged.present = true;
        s_staged.foreign = (version != DAI_SAVE_MAJOR) || (data == nullptr);
        if (!s_staged.foreign) s_staged.data.assign((const uint8_t *)data, (const uint8_t *)data + len);
    } catch (...) {
        s_staged.foreign = true;
        s_staged.data.clear();
    }
}

TbBool DaimonTake(PlayerNumber plyr, const char *personality, const char *difficulty, DaimonMode mode, DaimonTakeReason why)
{
    settle_all();
    if (!player_ok(plyr) || s_keepers[plyr]) return false;
    if (mode != DaimonMode::Opponent) {
        DaiLog::Warn(plyr, "a %s starts through its own entry, not a take", (mode == DaimonMode::Assistant) ? "assistant" : "lieutenant");
        return false;
    }
    try {
        // Refuse an unknown personality or difficulty before the keeper is touched.
        DaiPersonality p;
        DaiDifficulty d;
        std::string error;
        bool random = false;
        const std::string name = pick_personality(plyr, personality, &random);
        if (!DaiPersonality::Resolve(daimon_config, name.empty() ? nullptr : name.c_str(), &p, &error)
            || !DaiDifficulty::Resolve(daimon_config, difficulty, &d, &error)) {
            DaiLog::Warn(plyr, "not taken: %s", error.c_str());
            return false;
        }
        int64_t prev_model = 0;
        const NetUserId slot = daimon_assign_slot(plyr, &prev_model);
        if (slot < 0) return false;
        s_keepers[plyr] = std::make_unique<DaiKeeper>(plyr, slot, std::make_unique<DaiModeOpponent>(), p, prev_model, why, get_gameturn(),
            keeper_seed(plyr), s_world, s_actor);
        s_keepers[plyr]->SetDifficulty(d);
        s_keepers[plyr]->SetRandomPersonality(random);
        request_sim_cheats(plyr, d);
        s_actor.Attach(slot, s_keepers[plyr]->CtlWorld());
        DaiLog::Info(plyr, "taken into slot %" PRId64 " (%s, %s, reason %" PRId64 ")", (int64_t)slot, p.GetId().c_str(),
            difficulty_label(d).c_str(), (int64_t)why);
        return true;
    } catch (const std::exception &e) {
        fail_keeper(plyr, "take", e.what());
    } catch (...) {
        fail_keeper(plyr, "take", "unknown");
    }
    return false;
}

TbBool DaimonHandBack(PlayerNumber plyr, DaimonHandBackTo to)
{
    settle_all();
    if (!player_ok(plyr) || !s_keepers[plyr]) return false;
    if (network_is_active()) {
        DaiLog::Warn(plyr, "a network game's keeper is not handed back on one machine");
        return false;
    }
    if (to != DaimonHandBackTo::Computer2) {
        DaiLog::Warn(plyr, "not handed back: a keeper goes back to the built-in AI only");
        return false;
    }
    try {
        const int64_t model = s_keepers[plyr]->GetPrevModel();
        drop_keeper(plyr);
        const TbBool ok = daimon_release_slot(plyr, model);
        DaiLog::Info(plyr, "handed back to the built-in AI (model %" PRId64 ")", model);
        return ok;
    } catch (const std::exception &e) {
        fail_keeper(plyr, "hand back", e.what());
    } catch (...) {
        fail_keeper(plyr, "hand back", "unknown");
    }
    return false;
}

TbBool DaimonControls(PlayerNumber plyr)
{
    return DaimonKeeper(plyr) != nullptr;
}

TbBool DaimonDescribe(PlayerNumber plyr, char *out, size_t len)
{
    settle_all();
    const DaiKeeper *k = DaimonKeeper(plyr);
    if ((k == nullptr) || (out == nullptr) || (len == 0)) return false;
    DaimonPersonalityInfo list[DAIMON_MAX_PERSONALITIES];
    const size_t n = daimon_config_personality_list(list, DAIMON_MAX_PERSONALITIES);
    std::string shown = k->GetPersonality().GetId();
    for (size_t i = 0; i < n; i++)
        if (shown == list[i].id) shown = daimon_personality_display_name(&list[i]);
    snprintf(out, len, "%s%s, %s", shown.c_str(), k->RandomPersonality() ? " (random)" : "", difficulty_label(k->GetDifficulty()).c_str());
    return true;
}

DaimonMode DaimonModeOf(PlayerNumber plyr)
{
    const DaiKeeper *k = DaimonKeeper(plyr);
    return (k != nullptr) ? k->GetMode().GetMode() : DaimonMode::None;
}

void DaimonOnKeeperDefeated(PlayerNumber plyr)
{
    settle_all();
    if (player_ok(plyr) && s_keepers[plyr]) {
        s_keepers[plyr]->SetDefeated();
        DaiLog::Info(plyr, "defeated: Daimon stops playing this keeper");
    }
}

void DaimonTickAuthors(DaiTurn now)
{
    // Once a turn: a keeper writes at most one packet a turn, however often the loop comes round (a test may play
    // the turn itself before the driver does).
    if (s_driver.ticks != 0 && now == s_driver.last) {
        s_driver.repeats++;
        return;
    }
    s_driver.ticks++;
    s_driver.last = now;
    DaiBudget budgets;
    budgets.slab_scan = daimon_config.global.budgets_slab_scan;
    budgets.reach_visits = daimon_config.global.budgets_reach_visits;
    budgets.search_nodes = daimon_config.global.budgets_search_nodes;
    budgets.layout_candidates = daimon_config.global.budgets_layout_candidates;
    start_workers();
    for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) {
        if (!s_keepers[p]) continue;
        // The keeper went to someone else (a script, the built-in AI, a human): its slot is gone, and so is Daimon.
        if (daimon_slot_of(p) != s_keepers[p]->GetSlot()) {
            drop_keeper(p);
            DaiLog::Info(p, "taken over: Daimon stops playing this keeper");
            continue;
        }
        try {
            // The main thread's half (the outputs landing now applied, the gather); the planning half goes to the keeper's
            // worker (or runs now, inline), its output landing D turns on.
            DaiKeeper &k = *s_keepers[p];
            k.Turn(now, budgets, daimon_config.global, turn_rate(), &s_workers);
            if (k.IsDefeated()) continue;
            // The slot's controller writes this turn's step of the running gesture into the slot's packet, before the
            // packets are exchanged and processed (02 section 5). A network host's are written with its own packet,
            // into the slot's history, and sent (dai_net_host_author_slots in exchange_packets).
            if (!network_is_active()) {
                DaiActorKeeperCtl::Tick(k.GetSlot(), p);
            }
            s_driver.keeper_turns++;
        } catch (const std::exception &e) {
            fail_keeper(p, "turn", e.what());
        } catch (...) {
            fail_keeper(p, "turn", "unknown");
        }
    }
    // A network host keeps the clients supplied with a copy of Daimon's state (01 section 10.6).
    if (network_is_active() && network_is_host()) {
        const DaiTurn interval = (DaiTurn)daimon_config.global.net_replicate_interval;
        if (s_replicate_now || (now >= s_replicated_turn + interval)) replicate(now);
    }
}

void DaimonOnDropIn(PlayerNumber plyr)
{
    if (!player_ok(plyr) || replay.load_enable || !authors_keepers() || s_keepers[plyr] || (daimon_slot_of(plyr) < 0)) return;
    try {
        adopt(plyr, "steady", DaimonTakeReason::DropIn);   // the caretaker (05 section 5); default if a mod removed it
        s_replicate_now = true;
    } catch (const std::exception &e) {
        fail_keeper(plyr, "adoption", e.what());
    } catch (...) {
        fail_keeper(plyr, "adoption", "unknown");
    }
}

void DaimonOnHostLost(void)
{
    settle_all();
    if (replay.load_enable) return; // a replay's recorded packets go on playing the keepers
    for (const auto &k : s_keepers) {
        if (k) return; // this machine plays keepers already: it was the host, and they go on
    }
    try {
        take_over_after_host_lost();
    } catch (const std::exception &e) {
        ERRORLOG("DAI: taking Daimon's keepers over after the host was lost failed (%s)", e.what());
        for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) if ((daimon_slot_of(p) >= 0) && !s_keepers[p]) fail_keeper(p, "host loss", e.what());
    } catch (...) {
        ERRORLOG("DAI: taking Daimon's keepers over after the host was lost failed");
        for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) if ((daimon_slot_of(p) >= 0) && !s_keepers[p]) fail_keeper(p, "host loss", "unknown");
    }
}

void DaimonDump(PlayerNumber plyr, const char *what, void (*line)(const char *))
{
    settle_all();
    if (line == nullptr) return;
    const DaiKeeper *k = DaimonKeeper(plyr);
    if (k == nullptr) {
        char buf[64];
        snprintf(buf, sizeof(buf), "p%d: no Daimon keeper", (int)plyr);
        line(buf);
        return;
    }
    try {
        const std::string view = (what != nullptr) ? what : "status";
        if (view == "dump") {
            for (const char *v : { "status", "posture", "know", "goals", "objectives", "requests", "jobs", "claims", "inbox" }) k->Dump(v, line);
        } else if (!k->Dump(view.c_str(), line)) {
            line("Daimon: no such view (status, posture, know, goals, objectives, requests, jobs, claims, inbox, dump)");
        }
    } catch (...) {
        line("Daimon: the dump failed");
    }
}

}
