/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file packets_upating.h
 *     Header file for packets_upating.c.
 * @par Purpose:
 *     Continious updating of game state on different routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     10 Jan 2021 -
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef PACKETS_UPDATING_H
#define PACKETS_UPDATING_H

#include "globals.h"
#include "config.h"
#include "thing_data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Thing;
struct BigActionPacket;
struct SmallActionPacket;

void send_update_job(struct Thing *thing);
void process_update_job(struct BigActionPacket *big);

TbBool send_update_land(struct Thing *thing, MapSlabCoord slb_x, MapSlabCoord slb_y, SlabKind nslab);
void process_update_land(int client_id, struct BigActionPacket *big);
void process_updating_packets();

void remove_update_thing(struct Thing *to_delete);

#ifdef __cplusplus
}
#endif

#endif