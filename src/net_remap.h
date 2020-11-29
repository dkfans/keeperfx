/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_remap.h
 *     Network remap. Headers
 * @par Purpose:
 *     Functions to map remotes thing ids to locals.
 * @par Comment:
 *     Interacts directly with bflib_network
 * @author   KeeperFX Team
 * @date     14 Nov 2020 -
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "packets.h"

typedef unsigned short Thingid;

void net_remap_init(Thingid thing_num);
Thingid net_remap_thingid(int client_id, Thingid id);
void net_remap_update(int net_player_id, Thingid their, Thingid mine);

/*
    This function is called at start of processing each network packet.
    It should read addendum data from packet (if any) and setup remapping of thing_id
    Then it starts recording created creatures

    net_player_id is source of a packet
*/
void net_remap_start(int net_player_id, unsigned char packet_kind, void *data, short size);

// Used for things
void net_remap_thing_created(Thingid mine);
// Used for creatures (TODO: check is owner is player_id or net_id)
void net_remap_creature_created(int owner, Thingid mine);

// Used to create a new remap packet
void net_remap_flush_things();

/* We may create a packet */
void net_remap_finish();

TbBool net_remap_packet_cb(unsigned long turn, int plyr_idx, unsigned char kind, void *data, short size);

void netremap_make_ghost_maybe(struct Thing *thing, int client_id);