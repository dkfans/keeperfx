/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_packets.h
 *     The packet actions Daimon's keeper controller sends (daimon_packets.c).
 */
/******************************************************************************/
#ifndef DK_DAIMON_PACKETS_H
#define DK_DAIMON_PACKETS_H

#include "globals.h"
#include "bflib_basics.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Handles a user's PckA_Dai* or keeper group packet, the same on every machine. */
TbBool daimon_process_packet_action(NetUserId user);

#ifdef __cplusplus
}
#endif
#endif
