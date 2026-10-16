/******************************************************************************/
// Free implementation of the Dungeon Keeper strategy game.
/******************************************************************************/
/** @file archon_test.h
 *     Automated archon (shared keeper) co-op test bot, enabled by -archontest.
 * @par Purpose:
 *     Drives the local user by filling the local packet like human input would,
 *     and logs checks to archontest_<user>.log.
 */
/******************************************************************************/
#ifndef DK_ARCHON_TEST_H
#define DK_ARCHON_TEST_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Packet;

/** Seconds of gameplay for the test bot; 0 when disabled. */
extern int32_t archontest_seconds;
/** Levels the bot plays in a row through the shared campaign flow (-archonflow); 0 when disabled. */
extern int32_t archontest_flow_levels;
/** Campaign mode (-archontestcampaign): after the checks win the level and continue the campaign. */
extern TbBool archontest_campaign;

TbBool archontest_active(void);
/** Replaces the human input for this frame: fills the local packet. */
void archontest_get_inputs(struct Packet *pckt);
/** Called after the turn checksum was computed for the local packet. */
void archontest_note_checksum(GameTurn turn, TbBigChecksum checksum);
/** Called whenever a checksum mismatch between users is detected. */
void archontest_note_desync(GameTurn turn);
/** Frontend part of the shared campaign flow test; called every frontend frame. */
void archontest_frontend_update(void);
/** Called after a level ended and its campaign progress was saved. */
void archontest_level_ended(void);

#ifdef __cplusplus
}
#endif
#endif
