/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file keeper_group_state.h
 *     A keeper's one group of creatures, as the group packets made it (keeper_group.h, docs/plans/ai-plans/v2/06
 *     section 5a.3): its members and the order it is under. Kept in kfx_sim_state so a save, a resync and a replay all
 *     carry it; the members' group links are the engine's own (creature_groups.c).
 */
/******************************************************************************/
#ifndef DK_KEEPER_GROUP_STATE_H
#define DK_KEEPER_GROUP_STATE_H

#include "globals.h"
#include "daimon_base_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Most creatures in a group: the engine's GROUP_MEMBERS_COUNT (creature_groups.h; checked in keeper_group.c). */
#define KEEPER_GROUP_MAX 30

/** What a group is doing (KeeperGroupState.order). */
enum KeeperGroupOrder {
    KGO_None = 0,   /**< formed, no order: its members follow the leader about its own business */
    KGO_Move,       /**< walking to the goal, then held there until the order ends */
    KGO_Attack,     /**< walking to the goal, then fighting what is there (the order ends on arrival) */
    KGO_Count,
};

#pragma pack(1)

struct KeeperGroupState {
    uint8_t count;                  /**< members, the leader included; 0 = no group */
    uint8_t order;                  /**< enum KeeperGroupOrder */
    uint8_t reserved[6];
    DaiThingIndex member[KEEPER_GROUP_MAX];        /**< [0] the leader */
    DaiTurn member_creation[KEEPER_GROUP_MAX]; /**< each member's creation turn: a reused thing slot is not it */
    DaiSubtlCoord goal_x, goal_y;
    DaiTurn ordered_turn;
    DaiTurn hold_until;
};

#pragma pack()

#ifdef __cplusplus
}
#endif
#endif
