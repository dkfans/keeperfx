/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/**
 * @author   KeeperFX Team
 * @date     18 Oct 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef GIT_BFLIB_ENET_H
#define GIT_BFLIB_ENET_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ENET_CHANNEL_RELIABLE = 0,
    ENET_CHANNEL_UNSEQUENCED = 1
};

struct NetSP;
struct NetSP* InitEnetSP();
unsigned long GetPing(int id);
unsigned long GetPingVariance(int id);
unsigned int GetPacketLoss(int id);
unsigned int GetClientDataInTransit();
unsigned int GetIncomingPacketQueueSize();
unsigned int GetClientPacketsLost();
unsigned int GetClientOutgoingDataTotal();
unsigned int GetClientIncomingDataTotal();
unsigned int GetClientReliableCommandsInFlight();

#ifdef __cplusplus
}
#endif

#endif //GIT_BFLIB_ENET_H
