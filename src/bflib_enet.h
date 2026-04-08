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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ENET_DEFAULT_PORT 5556

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
void enet_matchmaking_host_update(void);
extern uint16_t external_ipv4_port;
extern int skip_holepunch;
uint16_t enet_get_bound_ipv6_port(void);

#ifdef __cplusplus
}
#endif

#endif //GIT_BFLIB_ENET_H
