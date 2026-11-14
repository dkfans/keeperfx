/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file DaimonGlue.cpp
 *     Where KeeperFX calls into Daimon: the configuration loaded.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_daimon.h"

#include <vector>

#include "daimon/DaiDifficulty.h"
#include "daimon/DaiLog.h"
#include "daimon/DaiPersonality.h"
#include "post_inc.h"

/** Keys of the personalities and difficulties this build does not know: logged. */
extern "C" void daimon_on_config_loaded(void)
{
    std::vector<DaiPersonality::KeyIssue> issues = DaiPersonality::CheckKeys(daimon_config);
    for (DaiPersonality::KeyIssue &i : DaiDifficulty::CheckKeys(daimon_config)) issues.push_back(i);
    for (const DaiPersonality::KeyIssue &issue : issues)
        DaiLog::Warn(-1, "daimon.toml: %s is not %s this build knows; ignored", issue.what.c_str(), issue.value ? "a name" : "a key");
}
