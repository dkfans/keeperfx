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
#include "thing_data.h"

void net_remap_init(Thingid thing_num);
Thingid net_remap_thingid(int client_id, Thingid id);
void net_remap_update(int net_player_id, Thingid their, Thingid mine);

/*
    This function is called at start of processing each network packet.
    It should read addendum data from packet (if any) and setup remapping of thing_id
    Then it starts recording created creatures

    net_player_id is source of a packet
*/
void net_remap_start(int client_id, unsigned char packet_kind, void *data, short size);

// Used for things
void net_remap_thing_created(Thingid mine);
// Used for creatures (TODO: check is owner is player_id or net_id)
void net_remap_creature_created(int client_id, Thingid mine);

// Used to create a new remap packet
void net_remap_flush_things();

/* We may create a packet */
void net_remap_finish();

TbBool net_remap_packet_cb(unsigned long turn, int plyr_idx, unsigned char kind, void *data, short size);

void netremap_make_ghost_maybe(struct Thing *thing, int client_id);

/* return true if this side is responsible for something belonging to this player_idx */
TbBool netremap_is_mine(PlayerNumber plyr_id);

/*
 * Called before each packet exchange cycle to create one time packets
 */
void send_remap_packets();

/*
  Remap an object created by room (chickens, spellbooks, boxes etc)
 */
void netremap_room_object(struct Room *room, Thingid mine);