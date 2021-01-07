/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_remap.c
 *     Network remap.
 * @par Purpose:
 *     Functions to map remotes thing ids to locals.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     14 Nov 2020 -
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "globals.h"

#include "game_legacy.h"
#include "net_game.h"
#include "net_remap.h"
#include "player_data.h" // for my_player_number
#include "thing_list.h"  // for THINGS_COUNT

#define MAX_CREATURES_PER_PACKET    4

static Thingid thing_map[NET_PLAYERS_COUNT][THINGS_COUNT];

static struct
{
    unsigned short message_id;
    unsigned short message_data[MAX_CREATURES_PER_PACKET];

    // We are senders
    unsigned short *src_last;
    unsigned short *src_end;

    unsigned short dst_buf[MAX_CREATURES_PER_PACKET * 2];
    unsigned short *dst_last;
    unsigned short *dst_end;

    unsigned short net_player_id;
} addendum;

void net_remap_init(Thingid thing_num)
{
    for (int plyr = 0; plyr < NET_PLAYERS_COUNT; plyr++)
    {
        memset(&thing_map[plyr][0], 0, sizeof(Thingid) * THINGS_COUNT);
        for (int i = 1; i < THINGS_COUNT; i++)
        {
            struct Thing *thing = thing_get(i);
            if (thing_exists(thing))
            {
                thing_map[plyr][i] = i;
            }
        }
    }
}

Thingid net_remap_thingid(int client_id, Thingid id)
{
    Thingid mine;
    if (id == 0)
        return 0;
    //TODO: replace to "my_net_nmber"
    if (client_id == my_player_number)
        return id;
    mine = thing_map[client_id][id];
    if (mine == 0)
    {
        WARNMSG("not found their:%d", id);
    }
    return mine;
}

void net_remap_update(int net_player_id, Thingid their, Thingid mine)
{
    if (thing_map[net_player_id][their] != 0)
    {
        ERRORLOG("found unexpected id:%d", their);
    }
    if (mine == 0)
    {
        ERRORLOG("Remap from their:%d to ZERO?!");
        return;
    } else if (their == 0)
    {
        ERRORLOG("Remap from ZERO to mine:%d ?!", mine);
        return;
    }
    NETDBG(6, "Remap from their:%d to mine:%d", their, mine);
    thing_map[net_player_id][their] = mine;
}

void net_remap_start(int net_player_id, unsigned char packet_kind, void *data, short size)
{
    addendum.net_player_id = net_player_id;
    if (net_player_id == my_player_number)
    {
        // We are source of event
        switch (packet_kind)
        {
            case PckA_UsePower:
                addendum.src_last = (unsigned short *) (((unsigned char *) data) + sizeof(struct BigActionPacket));
                addendum.src_end = addendum.src_last + 1; //Only one creature
                break;
            default:
                addendum.src_last = addendum.message_data;
                addendum.src_end = addendum.src_last + MAX_CREATURES_PER_PACKET;
        }
        NETDBG(10, "reserving %p[%d]", addendum.src_last, addendum.src_end - addendum.src_last);
    } else
    {
        addendum.dst_last = addendum.dst_buf;
        addendum.dst_end = addendum.dst_buf + 2 * MAX_CREATURES_PER_PACKET;

        switch (packet_kind)
        {
            case PckA_UsePower:
                addendum.src_last = (unsigned short *) (((unsigned char *) data) + sizeof(struct BigActionPacket));
                addendum.src_end = addendum.src_last + 1; //Only one creature
            default:
                break;
        }
    }
}

static void net_remap_thing_created_internal(int owner, Thingid mine)
{
    if (game.play_gameturn == 0)
    {
        NETDBG(6, "loading level thing:%d", mine);
        return;
    }
    if (addendum.src_last >= addendum.src_end)
    {
        ERRORLOG("Too many creatures per packet created! last:%d %p[%d]", mine, addendum.src_last,
                 addendum.src_end - addendum.src_last);
        return;
    }
    if (addendum.net_player_id == my_player_number)
    {
        NETDBG(6, "master mine:%d", mine);
        *addendum.src_last = mine;
        addendum.src_last++;
    } else
    {
        Thingid their = *addendum.src_last;
        net_remap_update(addendum.net_player_id, their, mine);
        if (addendum.dst_last >= addendum.dst_end)
        {
            //TODO: When is it possible?
            ERRORLOG("Too many creatures per packet created! last:%d", mine);
            return;
        }
        *addendum.dst_last = their;
        addendum.dst_last++;
        *addendum.dst_last = mine;
        addendum.dst_last++;
        NETDBG(6, "slave mine:%d their:%d", mine, their);
        addendum.src_last++;
    }
}

static void net_remap_pre_create_packet()
{
    addendum.src_last = addendum.message_data;
    addendum.src_end = addendum.src_last + MAX_CREATURES_PER_PACKET;
    NETDBG(6, "%p[%d]", addendum.src_last, addendum.src_end - addendum.src_last);
}

void net_remap_creature_created(int owner, Thingid mine)
{
    if (addendum.src_last == NULL)
    {
        net_remap_pre_create_packet();
    }
    net_remap_thing_created_internal(owner, mine);
}

void net_remap_thing_created(Thingid mine)
{
    if (addendum.src_last == NULL)
    {
        net_remap_pre_create_packet();
    }
    net_remap_thing_created_internal(my_player_number, mine);
}

void net_remap_finish()
{
    if ((addendum.net_player_id != my_player_number) && (addendum.dst_last != addendum.dst_buf))
    {
        if (LbNetwork_IsServer())
        {
            // TODO spawn new message near LbNetwork_Packetid()
            // We should alter original packet for others
        }

        NETDBG(7, "sending remap notification");
        size_t size = sizeof(Thingid) * (addendum.dst_last - addendum.dst_buf);
        Thingid *packet_data = LbNetwork_AddPacket(PckA_RemapNotify, 0, size);

        LbNetwork_SetDestination(packet_data, addendum.net_player_id); // Only for original sender
        LbNetwork_MoveToOutgoingQueue(packet_data); // We dont want to get this message ourself

        int i = 0;
        for (unsigned short *val = addendum.dst_buf; val <= addendum.dst_last;)
        {
            packet_data[i++] = *val;
            val++; // other's
            packet_data[i++] = *val;
            val++; // mine
        }
    }
    addendum.src_last = NULL;
    addendum.src_end = NULL;
    addendum.dst_last = NULL;
    addendum.dst_end = NULL;
}

void net_remap_flush_things()
{
}

/*
    TODO: also update position of units etc. so it shoud be moved out of this file
*/
TbBool net_remap_packet_cb(unsigned long turn, int net_idx, unsigned char kind, void *data_ptr, short size)
{
    NETDBG(6, "net_idx:%d size:%d", net_idx, size);
    unsigned short *src = (unsigned short *) data_ptr;
    unsigned short *src_end = src + (size / sizeof(short));
    while (src < src_end)
    {
        unsigned short mine = *(src++);
        unsigned short their = *(src++);
        net_remap_update(net_idx, their, mine);
    }
    return true;
}

void netremap_make_ghost_maybe(struct Thing *thing, int client_id)
{
    struct ThingAdd *thingadd = get_thingadd(thing->index);
    // TODO: use some kind of my_client_id
    if (client_id != my_player_number)
    {
        NETDBG(3, "ghost:%d", thing->index);
        thingadd->flags |= TA_NetGhost;


        //TODO: add to list and remove sometimes
        if (gameadd.first_ghost != 0)
        {
            thingadd->next_ghost = gameadd.first_ghost;
        }
        gameadd.first_ghost = thing->index;
    }
}

TbBool netremap_is_mine(PlayerNumber plyr_id)
{
    if (plyr_id == my_player_number)
        return true;
        // TODO: Computers?
    else if (my_player_number == 0)
        return (plyr_id == game.neutral_player_num) || (plyr_id == game.hero_player_num);
    return false;
}

void send_remap_packets()
{
}

