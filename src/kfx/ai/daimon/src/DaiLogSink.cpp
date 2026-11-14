/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file DaiLogSink.cpp
 *     Daimon's log, on KeeperFX's: where its lines go, whether debug lines are wanted, the turn a warning carries.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "daimon/DaiLog.h"

#include <cinttypes>

#include "bflib_basics.h"
#include "game_legacy.h"
#include "post_inc.h"

bool DaiLog::DebugOn()
{
    return (BFDEBUG_LEVEL > 7);
}

DaiTurn DaiLog::EngineTurn()
{
    return get_gameturn();
}

void DaiLog::Write(const DaiLogLine &l)
{
    switch (l.level) {
    case DaiLogLevel::Info: LbJustLog("DAI: %s\n", l.text.c_str()); break;
    case DaiLogLevel::Debug: if (DebugOn()) LbSyncLog("Debug: DAI: %s\n", l.text.c_str()); break;
    case DaiLogLevel::Warn: LbWarnLog("[%" PRIu64 "] Warn: DAI: %s\n", (uint64_t)l.turn, l.text.c_str()); break;
    }
}
