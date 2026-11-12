/** @file DaiTurnCtx.h
 *  One turn of one keeper as its decisions see it (02 section 5): const views of everything a decision may read, the
 *  keeper's random stream and the turn's budgets. Write access is the narrow view each step is given
 *  (DaiManagerWrites to a manager; the job pool and the ledger to the arbiter). */
#ifndef DAIMON_DAITURNCTX_H
#define DAIMON_DAITURNCTX_H

#include "globals.h"
#include "config_daimon.h"
#include "daimon/DaiForecast.h"
#include "daimon/DaiGoals.h"
#include "daimon/DaiInbox.h"
#include "daimon/DaiJobPool.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiLedger.h"
#include "daimon/DaiObjectives.h"
#include "daimon/DaiPersonality.h"
#include "daimon/DaiRequests.h"
#include "daimon/DaiRivals.h"
#include "daimon/DaiRng.h"
#include "daimon/DaiSearchPool.h"
#include "daimon/DaiThreatModel.h"
#include "daimon/DaiTypes.h"
#include "daimon/IDaiMode.h"
#include "daimon/IDaiPlanWorld.h"

class DaiDifficulty;

class DaiLeases;
struct DaiTurnCtx {
    PlayerNumber player;
    DaiTurn now;
    const DaiKnowledge &know;
    const DaiInbox &inbox;
    const DaiGoals &goals;
    const DaiRequests &requests;
    const DaiJobPool &jobs;
    const DaiLedger &ledger;
    const DaiPersonality &personality;
    const DaimonGlobalConfig &cfg;
    const IDaiMode &mode;
    DaiRng &rng;
    DaiBudget &budget;
    DaiSearchPool &searches;
    /** This turn's economy forecast (03 section 6), the same for every manager. */
    const DaiForecastOut &forecast;
    /** How well it plays (05 section 7): `cfg` and `budget` are already scaled by it; the arbiter's noise and
     *  Creatures' care delay read it here. */
    const DaiDifficulty &difficulty;
    /** What the fights are for (06 section 5.2): Defence posts, Tactics serves and reports. */
    const DaiObjectives &objectives;
    /** Where the enemies are and how they would come (06 section 3). */
    const DaiThreatModel &threat;
    /** The rivals as targets of an attack (07 section 3). */
    const DaiRivals &rivals;
    /** What a decision may ask the game itself (06 section 7, 12 section 13.7): the powers' table as the gather read it,
     *  and whether a cast would take or could be made there, answered a gather late; everything else comes from the
     *  knowledge. */
    const IDaiPlanWorld &world;
    /** The human's decisions an assistant leaves alone (09 section 5.3, P7.11); nullptr where none are kept. */
    const DaiLeases *leases = nullptr;
};

#endif
