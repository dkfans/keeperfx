// The game's job kinds, one per file in this directory, gathered into the table DaiJobKinds() returns.
#ifndef DAIMON_JOBS_JOBKINDS_H
#define DAIMON_JOBS_JOBKINDS_H

#include "daimon/IDaiJobKind.h"

const IDaiJobKind &JobCreateImpKind();
const IDaiJobKind &JobDigRouteKind();
const IDaiJobKind &JobMineVeinKind();
const IDaiJobKind &JobBuildBridgeKind();
const IDaiJobKind &JobDigAreaKind();
const IDaiJobKind &JobUnmarkAreaKind();
const IDaiJobKind &JobCustodyHandKind();
const IDaiJobKind &JobJailCreatureKind();
const IDaiJobKind &JobPlaceRoomKind();
const IDaiJobKind &JobGrowRoomKind();
const IDaiJobKind &JobRebuildRoomKind();
const IDaiJobKind &JobPlaceRoomDoorKind();
const IDaiJobKind &JobSellRoomKind();
const IDaiJobKind &JobMoveToJobKind();
const IDaiJobKind &JobTrainCreatureKind();
const IDaiJobKind &JobSendToLairKind();
const IDaiJobKind &JobDismissCreatureKind();
const IDaiJobKind &JobFeedCreatureKind();
const IDaiJobKind &JobPayCreatureKind();
const IDaiJobKind &JobSlapImpsKind();
const IDaiJobKind &JobTacticsLiftKind();
const IDaiJobKind &JobTacticsCastKind();
const IDaiJobKind &JobTacticsBannerKind();
/** hold_audience: a cast (DaiCastParams) of Hold Audience, no target. */
const IDaiJobKind &JobHoldAudienceKind();
const IDaiJobKind &JobSetTendencyKind();
const IDaiJobKind &JobTacticsGroupKind();
const IDaiJobKind &JobMagicCastKind();
const IDaiJobKind &JobCreepKind();
const IDaiJobKind &JobSetAllianceKind();
const IDaiJobKind &JobSendMessageKind();
const IDaiJobKind &JobPlaceTrapKind();
const IDaiJobKind &JobPlaceDoorKind();
const IDaiJobKind &JobUseSpecialKind();
const IDaiJobKind &JobFortifyRoomKind();
const IDaiJobKind &JobSetDoorLockKind();
/** Aims a slap_imps job at a working imp not still running from the last slap (the hand and the imp claimed); false if
 *  there is none. Workforce aims the job when it makes it: the arbiter runs a new job in the turn it is made, before
 *  the job's Tick. */
bool JobSlapAim(const DaiKnowledge &know, DaiJob &j);

#endif
