/** @file DaimonScript.cpp
 *  The facade for a level's scripts (14 K6): DAIMON_PLAYER, DAIMON_SET, DAIMON_BUILD, DAIMON_ATTACK, DAIMON_DIG
 *  and their Lua twins -- queued before the level start, set on the keeper after it. */
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
#include "daimon/DaiDirector.h"
#include "daimon/DaiLog.h"
#include "daimon/DaiModeAssistant.h"
#include "daimon/DaiDirectives.h"
#include "daimon/DaiModeLieutenant.h"
#include "daimon/DaiSave.h"
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

using namespace daimon_detail;

namespace {
/** At the level start (pending) or now. */
TbBool script_goal(PlayerNumber plyr, const PendingGoal &pg)
{
    if (!player_ok(plyr)) return false;
    if (!s_level_started) {
        try {
            s_pending_goals[plyr].push_back(pg);
        } catch (...) {
            return false;
        }
        return true;
    }
    settle_all();
    if (daimon_slot_of(plyr) < 0) return false;
    if (!s_keepers[plyr]) return true;
    return set_script_goal(plyr, pg);
}

DaiGoal script_goal_of(DaiGoalKind kind)
{
    DaiGoal g;
    g.kind = kind;
    g.weight = DAI_FIX_ONE;
    g.source = DaiGoalSource::Script;
    return g;
}

}

extern "C" {

TbBool DaimonScriptPlayer(PlayerNumber plyr, const char *personality_in, const char *difficulty)
{
    if (!player_ok(plyr)) return false;
    // daimon.toml's personalities are lower case: DAIMON_PLAYER's reader upper-cases the name, Lua passes it as written
    // (both callers the same, 14 S7).
    const std::string personality_lc = lower(personality_in);
    const char *personality = (personality_in != nullptr) ? personality_lc.c_str() : nullptr;
    // A level's script never hands the human's own seat to Daimon (the human does: DaimonTakeSeat, the Seat control).
    if (!network_is_active() && (plyr == my_player_number)) return false;
    if (!s_level_started) {
        DaimonQueueLevelStart(plyr, personality, difficulty);
        return true;
    }
    if (daimon_slot_of(plyr) >= 0) return true;   // Daimon's already
    const NetUserId slot = daimon_adopt_slot(plyr);
    if (slot < 0) return false;
    if (replay.load_enable || !authors_keepers() || s_keepers[plyr]) return true;
    try {
        adopt(plyr, (personality != nullptr) && (personality[0] != '\0') ? personality : nullptr, DaimonTakeReason::Script,
            (difficulty != nullptr) && (difficulty[0] != '\0') ? difficulty : nullptr);
        s_replicate_now = true;
    } catch (const std::exception &e) {
        fail_keeper(plyr, "script take", e.what());
    } catch (...) {
        fail_keeper(plyr, "script take", "unknown");
    }
    return true;
}

TbBool DaimonCheckKey(const char *key, const char *value, char *why, size_t why_len)
{
    const std::string k = lower(key);
    const DaiPersonality::Field *f = DaiPersonality::FindField(k.c_str());
    if (f == nullptr) {
        snprintf(why, why_len, "no personality key '%s'", k.c_str());
        return false;
    }
    int64_t v = 0;
    std::string err;
    if (!DaiPersonality::ParseValue(*f, value, &v, &err)) {
        snprintf(why, why_len, "%s: %s", k.c_str(), err.c_str());
        return false;
    }
    return true;
}

TbBool DaimonScriptSet(PlayerNumber plyr, const char *key, const char *value)
{
    if (!player_ok(plyr)) return false;
    const std::string k = lower(key), v = (value != nullptr) ? value : "";
    if (!s_level_started) {
        try {
            s_pending_sets[plyr].emplace_back(k, v);
        } catch (...) {
            return false;
        }
        return true;
    }
    settle_all();
    if (daimon_slot_of(plyr) < 0) return false;            // not Daimon's
    if (!s_keepers[plyr]) return true;                  // another machine authors it (or a replay's playback)
    return set_key(plyr, k, v);
}

TbBool DaimonScriptBuild(PlayerNumber plyr, RoomKind room, DaiSlabCoord x, DaiSlabCoord y, int64_t w, int64_t h)
{
    if ((room <= 0) || (w < 1) || (h < 1) || (x < 0) || (y < 0)) return false;
    PendingGoal pg;
    pg.goal = script_goal_of(DaiGoalKind::Build);
    pg.goal.p.at = DaiRect{ x, y, (DaiSlabCoord)(x + w - 1), (DaiSlabCoord)(y + h - 1) };
    pg.room_kind = room;
    return script_goal(plyr, pg);
}

TbBool DaimonScriptAttack(PlayerNumber plyr, PlayerNumber rival, const char *objective)
{
    PendingGoal pg;
    if (rival < 0) {
        pg.drop_attacks = true;
        return script_goal(plyr, pg);
    }
    if (!player_ok(rival) || (rival == plyr)) return false;
    const std::string obj = lower(objective);
    const char *first = (obj == "heart") ? "heart" : (obj == "portal") ? "portal" : (obj == "rooms") ? "room" : nullptr;
    if (!obj.empty() && (obj != "any") && (first == nullptr)) return false;
    if ((first != nullptr) && !DaimonScriptSet(plyr, "offence.first_target", first)) return false;
    pg.goal = script_goal_of(DaiGoalKind::Attack);
    pg.goal.p.key = (int64_t)rival + 1;
    return script_goal(plyr, pg);
}

TbBool DaimonScriptDig(PlayerNumber plyr, DaiSlabCoord x, DaiSlabCoord y)
{
    if ((x < 0) || (y < 0)) return false;
    PendingGoal pg;
    pg.goal = script_goal_of(DaiGoalKind::ExpandTo);
    pg.goal.p.count = DAI_EXPAND_POINT;
    pg.goal.p.at = DaiRect{ x, y, x, y };
    return script_goal(plyr, pg);
}

void DaimonScriptHandBack(PlayerNumber plyr)
{
    settle_all();
    if (!player_ok(plyr) || !s_level_started || (daimon_slot_of(plyr) < 0)) return;
    try {
        // The keeper's saved model (dai_slots' copy is not saved: after a load it is 0, 14 R8).
        const int64_t model = s_keepers[plyr] ? s_keepers[plyr]->GetPrevModel() : daimon_slot_prev_model(plyr);
        drop_keeper(plyr);
        daimon_release_slot(plyr, model);
        s_replicate_now = true;
        DaiLog::Info(plyr, "handed back to the built-in AI by the level script");
    } catch (const std::exception &e) {
        fail_keeper(plyr, "script hand back", e.what());
    } catch (...) {
        fail_keeper(plyr, "script hand back", "unknown");
    }
}

}
