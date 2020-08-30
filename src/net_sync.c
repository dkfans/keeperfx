/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_sync.c
 *     Network game synchronization for Dungeon Keeper.
 * @par Purpose:
 *     Functions to keep network games synchronized.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     11 Mar 2010 - 09 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "net_sync.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_network.h"

#include "config.h"
#include "front_network.h"
#include "player_data.h"
#include "game_merge.h"
#include "net_game.h"
#include "lens_api.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
long get_resync_sender(void)
{
    for (int i = 0; i < NET_PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0))
            return i;
  }
  return -1;
}

// Return true when we have finished sending sync data
static TbBool send_resync_game(TbBool first_resync)
{
    TbBool ret;
    if (first_resync)
    {
        NETLOG("Initiating resync turn: %ld", game.play_gameturn);
        //TODO NET see if it is necessary to dump to file... probably superfluous
        char* fname = prepare_file_path(FGrp_Save, "resync.dat");
        TbFileHandle fh = LbFileOpen(fname, Lb_FILE_MODE_NEW);
        if (fh == -1)
        {
            ERRORLOG("Can't open resync file.");
            return true;
        }

        LbFileWrite(fh, &game, sizeof(game));
        LbFileClose(fh);
    }
    ret = LbNetwork_Resync(first_resync, game.play_gameturn, &game, sizeof(game));
    if (ret)
    {
        NETLOG("Done syncing");
    }
    return ret;
}

static TbBool receive_resync_game(TbBool first_resync)
{
    TbBool ret;
    if (first_resync)
    {
        NETLOG("%s: Initiating re-synchronization of network game", __func__);
    }
    ret = LbNetwork_Resync(first_resync, game.play_gameturn, &game, sizeof(game));
    if (ret)
    {
        NETLOG("%s: Done syncing", __func__);
    }
    return ret;
}

TbBool resync_game(TbBool first_resync)
{
    SYNCDBG(2,"Starting");
    reset_eye_lenses();
    int i = get_resync_sender();
    if (is_my_player_number(i))
    {
        return send_resync_game(first_resync);
    } else
    {
        return receive_resync_game(first_resync);
    }
}

/**
 * Exchanges verification packets between all players, making sure level data is identical.
 * @return Returns true if all players return same checksum.
 */
void perform_checksum_verification(void)
{
    unsigned long checksum_mem = 0;
    for (int i = 1; i < THINGS_COUNT; i++)
    {
        struct Thing* thing = thing_get(i);
        if (thing_exists(thing)) {
            checksum_mem += thing->mappos.z.val + thing->mappos.y.val + thing->mappos.x.val;
        }
    }
    clear_packets();
    struct Packet* pckt = get_packet(my_player_number);
    set_packet_action(pckt, PckA_LevelExactCheck, 0, 0, 0, 0);
    pckt->chksum = checksum_mem ^ game.action_rand_seed;
    if (LbNetwork_Exchange(pckt))
    {
        ERRORLOG("Network exchange failed on level checksum verification");
    }
    if ( checksums_different() )
    {
        ERRORLOG("Level checksums different for network players");
    }
}
/**
 * Checks if all active players packets have same checksums.
 * @return Returns false if all checksums are same; true if there's mismatch.
 */
TbBool checksums_different(void)
{
    TbChecksum checksum = 0;
    TbBool is_set = false;
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0))
        {
            struct PacketEx* pckt = get_packet_ex_direct(player->packet_num);
            if (!is_set)
            {
                checksum = pckt->packet.chksum;
                is_set = true;
            }
            else if (checksum != pckt->packet.chksum)
            {
                return true;
            }
        }
  }
  return false;
}

static TbBigChecksum compute_player_checksum(struct PlayerInfo *player)
{
    TbBigChecksum sum = 0;
    if (((player->allocflags & PlaF_CompCtrl) == 0) && (player->acamera != NULL))
    {
        struct Coord3d* mappos = &(player->acamera->mappos);
        sum ^= (TbBigChecksum)player->instance_remain_rurns + (TbBigChecksum)player->instance_num;
        SHIFT_CHECKSUM(sum);
        sum ^= (TbBigChecksum)mappos->x.val + (TbBigChecksum)mappos->z.val + (TbBigChecksum)mappos->y.val;
    }
    return sum;
}

/**
 * Computes checksum of current state of all existing players.
 * @return The checksum value.
 */
TbBigChecksum compute_players_checksum(void)
{
    TbBigChecksum sum = 0;
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player))
        {
            SHIFT_CHECKSUM(sum);
            sum ^= compute_player_checksum(player);
      }
    }
    return sum;
}

/**
 * Adds given value to checksum at current game turn stored in packet file.
 *
 * @param plyr_idx The player whose checksum is computed.
 * @param sum Checksum increase.
 * @param area_name Name of the area from which the checksum increase comes, for logging purposes.
 */
void player_packet_checksum_add(PlayerNumber plyr_idx, TbBigChecksum sum, enum ChecksumKind kind)
{
    struct PacketEx* pckt = get_packet_ex(plyr_idx);
    pckt->packet.chksum ^= sum;
    pckt->sums[(int)kind] ^= sum;
    SYNCDBG(9,"Checksum updated kind:%d amount:%06lX", kind,(unsigned long)sum);
}
/******************************************************************************/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
