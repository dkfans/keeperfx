/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_archon.h
 *     Header file for front_archon.c.
 * @par Purpose:
 *     Shared between-levels flow of an archon (shared keeper) co-op campaign.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 */
/******************************************************************************/
#ifndef DK_FRONT_ARCHON_H
#define DK_FRONT_ARCHON_H

#include "globals.h"
#include "bflib_basics.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define ARCHON_MAP_LEVELS_MAX 12

/** What a user of an archon campaign session reports in ScreenPacket action_par2. */
enum ArchonFrontendFlags {
    ArchonFe_OnMap   = 0x01, /**< the user is on the shared campaign map */
    ArchonFe_InStats = 0x02, /**< the user is on the level statistics screen */
};

/** Host only: campaign the session plays co-op (from -campaign with -archon, or the session option). Empty when off. */
extern char archon_campaign_fname[80];
/******************************************************************************/
TbBool archon_flow_active(void);
void archon_flow_reset(void);
void archon_flow_level_ended(void);
TbBool archon_host_open_campaign_map(void);
TbBool archon_process_map_message(NetUserId user, const char *message);
void archon_flow_update(void);
TbBool archon_flow_stats_leave(void);
TbBool archon_flow_users_report(unsigned char flag);
LevelNumber archon_map_level(int idx);
int archon_map_levels_count(void);
TbBool archon_map_level_visible(LevelNumber lvnum);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
