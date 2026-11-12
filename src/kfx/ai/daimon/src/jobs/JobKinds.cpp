#include "pre_inc.h"
#include "JobKinds.h"

#include <array>

#include "post_inc.h"

std::span<const IDaiJobKind *const> DaiJobKinds()
{
    static const std::array<const IDaiJobKind *, DAI_JOB_KINDS> kinds = {
        nullptr,                 // 0: none
        &JobCreateImpKind(),     // DAI_JOB_CREATE_IMP
        &JobDigRouteKind(),      // DAI_JOB_DIG_ROUTE
        &JobMineVeinKind(),      // DAI_JOB_MINE_VEIN
        &JobBuildBridgeKind(),   // DAI_JOB_BUILD_BRIDGE
        &JobDigAreaKind(),       // DAI_JOB_DIG_AREA
        &JobPlaceRoomKind(),     // DAI_JOB_PLACE_ROOM (Phase 1's build_treasury)
        &JobSlapImpsKind(),      // DAI_JOB_SLAP_IMPS
        &JobGrowRoomKind(),      // DAI_JOB_GROW_ROOM
        &JobRebuildRoomKind(),   // DAI_JOB_REBUILD_ROOM
        &JobPlaceRoomDoorKind(), // DAI_JOB_PLACE_ROOM_DOOR
        &JobSellRoomKind(),      // DAI_JOB_SELL_ROOM
        &JobMoveToJobKind(),     // DAI_JOB_MOVE_TO_JOB
        &JobTrainCreatureKind(), // DAI_JOB_TRAIN_CREATURE
        &JobSendToLairKind(),    // DAI_JOB_SEND_TO_LAIR
        &JobDismissCreatureKind(), // DAI_JOB_DISMISS_CREATURE
        &JobFeedCreatureKind(),  // DAI_JOB_FEED_CREATURE
        &JobPayCreatureKind(),   // DAI_JOB_PAY_CREATURE
        &JobTacticsLiftKind(),   // DAI_JOB_TACTICS_LIFT
        &JobTacticsCastKind(),   // DAI_JOB_TACTICS_CAST
        &JobTacticsBannerKind(), // DAI_JOB_TACTICS_BANNER
        &JobHoldAudienceKind(),  // DAI_JOB_HOLD_AUDIENCE
        &JobPlaceTrapKind(),     // DAI_JOB_PLACE_TRAP
        &JobPlaceDoorKind(),     // DAI_JOB_PLACE_DOOR
        &JobSetDoorLockKind(),   // DAI_JOB_SET_DOOR_LOCK
        &JobSetTendencyKind(),   // DAI_JOB_SET_TENDENCY
        &JobTacticsGroupKind(),  // DAI_JOB_TACTICS_GROUP
        &JobMagicCastKind(),     // DAI_JOB_MAGIC_CAST
        &JobCreepKind(),         // DAI_JOB_CREEP
        &JobSetAllianceKind(),   // DAI_JOB_SET_ALLIANCE
        &JobSendMessageKind(),   // DAI_JOB_SEND_MESSAGE
        &JobUnmarkAreaKind(),    // DAI_JOB_UNMARK_AREA
        &JobCustodyHandKind(),   // DAI_JOB_CUSTODY_HAND
        &JobJailCreatureKind(),  // DAI_JOB_JAIL_CREATURE
        &JobUseSpecialKind(),    // DAI_JOB_USE_SPECIAL
        &JobFortifyRoomKind(),   // DAI_JOB_FORTIFY_ROOM
    };
    return kinds;
}
