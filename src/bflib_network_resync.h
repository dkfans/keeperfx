/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_network_resync.h
 *     Network resynchronization support.
 * @par Purpose:
 *     Network resynchronization routines.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     31 Oct 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_NETWORK_RESYNC_H
#define BFLIB_NETWORK_RESYNC_H

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

TbBool LbNetwork_Resync(void *data_buffer, size_t buffer_length);
void LbNetwork_TimesyncBarrier(void);

void LbNetwork_ClientSendReadyMessage(void);
void LbNetwork_WaitUntilResumeTime(TbClockMSec resume_time);
TbBool LbNetwork_HostWaitForAllClientsReady(void);
TbClockMSec LbNetwork_HostSendResumeToAllClients(void);
TbBool LbNetwork_ClientWaitForResumeMessage(TbClockMSec *resume_time);

#ifdef __cplusplus
}
#endif

#endif
