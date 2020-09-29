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
 * @date     11 Mar 2010 - 30 Aug 2020
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
#include "game_legacy.h"
#include "game_merge.h"
#include "keeperfx.hpp"
#include "lens_api.h"
#include "net_game.h"
#include "player_data.h"
#include "thing_serde.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

struct SyncDungeonInfo
{
    short creatr_list_start;
    short digger_list_start;
    short things_in_hand[MAX_THINGS_IN_HAND];
    // short sight_casted_thing_idx; // TODO:from things

//    short highest_task_number;
//    int total_money_owned;
//    int offmap_money_owned;
    unsigned char instance_num;
    unsigned long instance_remain_rurns;
    unsigned char allied_players;
};

struct SyncPartCommon
{
    unsigned long play_gameturn;
    unsigned long action_turn_rand_seed;
    int free_things_start_index;
    short nodungeon_creatr_list_start;
    struct SyncDungeonInfo dungeons[DUNGEONS_COUNT];

    int size_of_cctrl_part;
    int size_of_things_part;
    int size_of_rooms_part;
};

/******************************************************************************/
#define RESYNC_CCTRL ( (1 << CKS_Creatures_1) | (1 << CKS_Creatures_2) | (1 << CKS_Creatures_3) | \
    (1 << CKS_Creatures_4) | (1 << CKS_Creatures_5) | (1 << CKS_Creatures_6))
#define RESYNC_THINGS (RESYNC_CCTRL | (1 << CKS_Things) | (1 << CKS_Effects))
#define RESYNC_ROOMS (1 << CKS_Rooms)

static char desync_info[(2 * CKS_MAX ) + 1] = ".....................";
static const char desync_letters[CKS_MAX] = {
  'A', // CKS_Action
  'P', // CKS_Players
  '0', // CKS_Creatures_1
  '1', // CKS_Creatures_2
  '2', // CKS_Creatures_3
  '3', // CKS_Creatures_4
  '4', // CKS_Creatures_5
  '5', // CKS_Creatures_6
  'O', // CKS_Things
  'E', // CKS_Effects
  'R', // CKS_Rooms
};

static unsigned long resync_parts = 0;
struct ChecksumStorage player_checksum_storage[PLAYERS_EXT_COUNT] = {0};

#ifdef LOG_CHECKSUMS
TbBool log_checksums = 0;
#endif
/******************************************************************************/
static void update_desync_info(struct PacketEx* v1, struct PacketEx* v2)
{
    for (int i = 0; i < (int)CKS_MAX; i++)
    {
        if (v1->sums[i] != v2->sums[i])
        {
            desync_info[i*2] = desync_letters[i];
            resync_parts |= (1UL << i);
        }
        else
        {
            desync_info[i*2] = '.';
        }
    }
}

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


        serde_srv_things();

        struct SyncPartCommon part1 = {
            .play_gameturn = game.play_gameturn,
            .action_turn_rand_seed = gameadd.action_turn_rand_seed,
            .free_things_start_index = game.free_things_start_index,
            .nodungeon_creatr_list_start = game.nodungeon_creatr_list_start
        };

        for (int i = 0; i < DUNGEONS_COUNT; i++)
        {
            part1.dungeons[i].creatr_list_start = game.dungeon[i].creatr_list_start;
            part1.dungeons[i].digger_list_start = game.dungeon[i].digger_list_start;
            memcpy(
                part1.dungeons[i].things_in_hand,
                game.dungeon[i].things_in_hand,
                sizeof(part1.dungeons[i].things_in_hand)
                );
            if (i < PLAYERS_COUNT)
            {
                struct PlayerInfo* player = get_player(i);
                assert(player->id_number == i);
                part1.dungeons[i].instance_num = player->instance_num;
                part1.dungeons[i].instance_remain_rurns = player->instance_remain_rurns;
                part1.dungeons[i].allied_players = player->allied_players;
            }
        }

        int size_of_part1 = sizeof(part1);
        // If size of part is set to 0 - that part will be not synced
        part1.size_of_cctrl_part = (resync_parts & (RESYNC_CCTRL))? sizeof(game.cctrl_data) : 0;
        part1.size_of_things_part = (resync_parts & (RESYNC_THINGS))? sizeof(game.things_data) : 0;
        part1.size_of_rooms_part = (resync_parts & (RESYNC_ROOMS))? sizeof(game.rooms) : 0;

        struct SyncArrayItem data[] =
        {
            { &part1, &size_of_part1 },
            { &game.cctrl_data[0], &part1.size_of_cctrl_part },
            { &game.things_data[0], &part1.size_of_things_part },
            { &game.rooms[0], &part1.size_of_rooms_part },
            { NULL, 0 },
        };

#ifdef DUMP_THINGS
        for (int i = 0; data[i].buf != NULL; i++)
        {
            char buf[64];
            sprintf(buf, "dump/s_%d", i);
            FILE *F = fopen(buf, "w");
            if (F)
            {
                fwrite(data[i].buf, *data[i].size, 1, F);
                fclose(F);
            }
        }
#endif

        ret = LbNetwork_Resync(first_resync, game.play_gameturn, data);
    }
    else
    {
        ret = LbNetwork_Resync(first_resync, game.play_gameturn, NULL);
    }

    if (ret)
    {
        serde_fin_things();
        clear_packets();
        game.action_rand_seed = gameadd.action_turn_rand_seed;
        NETLOG("Done syncing");
#ifdef LOG_CHECKSUMS
        log_checksums = true;
#endif
    }
    return ret;
}

static TbBool receive_resync_game(TbBool first_resync)
{
    TbBool ret;
    if (first_resync)
    {
        NETLOG("Initiating resync turn:%ld", game.play_gameturn);
        serde_cli_things();
    }
    static struct SyncPartCommon part1;
    int size_of_part1 = sizeof(part1);

    struct SyncArrayItem data[] =
    {
        { &part1, &size_of_part1 },
        { &game.cctrl_data[0], &part1.size_of_cctrl_part },
        { &game.things_data[0], &part1.size_of_things_part },
        { &game.rooms[0], &part1.size_of_rooms_part },
        { NULL, 0 },
    };
    ret = LbNetwork_Resync(first_resync, game.play_gameturn, data);
    if (ret)
    {
        NETDBG(6, "free_things %d -> %d", game.free_things_start_index, part1.free_things_start_index);
        game.play_gameturn = part1.play_gameturn;
        gameadd.action_turn_rand_seed = part1.action_turn_rand_seed;
        game.nodungeon_creatr_list_start = part1.nodungeon_creatr_list_start;

        for (int i = 0; i < DUNGEONS_COUNT; i++)
        {
            game.dungeon[i].creatr_list_start = part1.dungeons[i].creatr_list_start;
            game.dungeon[i].digger_list_start = part1.dungeons[i].digger_list_start;
            memcpy(
                game.dungeon[i].things_in_hand,
                part1.dungeons[i].things_in_hand,
                sizeof(game.dungeon[i].things_in_hand)
                );
            game.dungeon[i].num_things_in_hand = 0;
            for (int j = 0; j < MAX_THINGS_IN_HAND; j++)
            {
                if (game.dungeon[i].things_in_hand[j] != 0)
                {
                    game.dungeon[i].num_things_in_hand++;
                }
                else
                    break;
            }

            if (i < PLAYERS_COUNT)
            {
                struct PlayerInfo* player = get_player(i);

                assert(player->id_number == i);
                if (player->instance_num != part1.dungeons[i].instance_num)
                {
                    JUSTLOG("instance_num from %d to %d for player %d",
                        player->instance_num,
                        part1.dungeons[i].instance_num,
                        i);
                    player->instance_num = part1.dungeons[i].instance_num;
                }
                player->instance_remain_rurns = part1.dungeons[i].instance_remain_rurns;
                player->allied_players = part1.dungeons[i].allied_players;
            }
        }

#ifdef DUMP_THINGS
    for (int i = 0; data[i].buf != NULL; i++)
    {
        char buf[64];
        sprintf(buf, "dump/r_%d", i);
        F = fopen(buf, "w");
        if (F)
        {
            fwrite(data[i].buf, *data[i].size, 1, F);
            fclose(F);
        }
    }
#endif
        serde_fin_things();
        clear_packets();
        game.action_rand_seed = gameadd.action_turn_rand_seed;
        NETLOG("Done syncing");
#ifdef LOG_CHECKSUMS
        log_checksums = true;
#endif
    }
    return ret;
}

TbBool resync_game(TbBool first_resync)
{
    SYNCDBG(2, "Starting");
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

TbBool checksum_packet_callback(
    void *context_, unsigned long turn, int plyr_idx, unsigned char kind, void *packet_data, short size)
{
    struct ChecksumContext *context = (struct ChecksumContext *) context_;
    struct PlayerInfo* player = get_player(plyr_idx);
    if (!player_exists(player) || ((player->allocflags & PlaF_CompCtrl) != 0))
    {
        ERRORLOG("unexpected player:%d", plyr_idx);
        return true;
    }
    context->checked_players |= plyr_idx;
    struct PacketEx* pckt = get_packet_ex_direct(player->packet_num);
    if (!context->base)
    {
        context->checksum = pckt->packet.chksum;
        context->base = pckt;

#ifdef DUMP_THINGS
        char buf[64];
        sprintf(buf, "dump/pl_%d", i);
        FILE *F = fopen(buf, "w");
        if (F)
        {
            fwrite(pckt, sizeof(struct PacketEx), 1, F);
            fclose(F);
        }
#endif

    }
    else if (context->checksum != pckt->packet.chksum)
    {
        update_desync_info(context->base, pckt);
        NETDBG(3, "different checksums at %lu player_id:%d", game.play_gameturn, i);

#ifdef DUMP_THINGS
        char buf[64];
        sprintf(buf, "dump/pl_%d", i);
        FILE *F = fopen(buf, "w");
        if (F)
        {
            fwrite(pckt, sizeof(struct PacketEx), 1, F);
            fclose(F);
        }
#endif

        return true;
    }

    return true;
}

/**
 * Exchanges verification packets between all players, making sure level data is identical.
 * @return Returns true if all players return same checksum.
 */
void perform_checksum_verification(void)
{
    struct ChecksumContext context = {0};

    unsigned long checksum_mem = 0;
    for (int i = 1; i < THINGS_COUNT; i++)
    {
        struct Thing* thing = thing_get(i);
        if (thing_exists(thing)) {
            SHIFT_CHECKSUM(checksum_mem);
            checksum_mem ^= (thing->mappos.z.val << 16)
                ^ (thing->mappos.y.val << 8)
                ^ thing->mappos.x.val
                ^ thing->model;
        }
    }
    clear_packets();
    struct PacketEx* pckt = get_packet_ex(my_player_number);
    set_players_packet_action(get_player(my_player_number), PckA_LevelExactCheck, 0, 0, 0, 0);
    pckt->packet.chksum = checksum_mem ^ game.action_rand_seed;
    if (LbNetwork_Exchange(&context, &checksum_packet_callback) != NR_OK)
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
    resync_parts = 0;

    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
    }
#ifdef LOG_CHECKSUMS
    log_checksums = false;
#endif
    return false;
}

const char *get_desync_info()
{
    return desync_info;
}

static TbBigChecksum compute_player_checksum(struct PlayerInfo *player)
{
    TbBigChecksum sum = 0;
    if (((player->allocflags & PlaF_CompCtrl) == 0) && (player->acamera != NULL))
    {
        struct Coord3d* mappos = &(player->acamera->mappos);
        sum ^= (TbBigChecksum)player->instance_remain_rurns + (TbBigChecksum)player->instance_num;
        SHIFT_CHECKSUM(sum);
        sum ^= player->allied_players;
        // This value should not be important
        // This value is dependant on unsync random i.e. at **make_camera_deviations**
        // sum ^= (TbBigChecksum)mappos->x.val + (TbBigChecksum)mappos->z.val + (TbBigChecksum)mappos->y.val;
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

TbBigChecksum get_thing_checksum(const struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    if (!thing_exists(thing))
        return 0;
    TbBigChecksum csum = ((ulong)thing->class_id << 24);
    csum ^= (((ulong)thing->mappos.z.val) << 16) ^
            (((ulong)thing->mappos.x.val) << 8) ^
            (ulong)thing->mappos.y.val;
    csum ^= (ulong)thing->health ^
            (((ulong)thing->model) << 4) ^
            (((ulong)thing->owner) << 12);
    csum ^= thing->index;
    if (thing->class_id == TCls_Creature)
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        SHIFT_CHECKSUM(csum);
        csum ^= (ulong)cctrl->inst_turn ^
            (((ulong)cctrl->instance_id) << 4) ^
            (((ulong)thing->field_49) << 12) ^
            (ulong)thing->field_48;

        player_checksum_storage[thing->owner].checksum_creatures[thing->index] = (thing->index << 16) | ((csum & 0xFFFF) ^ (csum >> 16));
    }
    return csum;
}

void resync_reset_storage()
{
    memset(player_checksum_storage, 0, sizeof(player_checksum_storage));
    resync_parts = 0;
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
    EVM_GLOBAL_EVENT("mp.checksum,kind=%d val=%ld", kind, (unsigned long)sum);
    SYNCDBG(9, "Checksum updated kind:%d amount:%06lX", kind,(unsigned long)sum);
#ifdef LOG_CHECKSUMS
    if (log_checksums)
    {
        JUSTLOG("Checksum type:%d sum:%06lx sums[type]:%06lx delta:%06lx",
            kind,
            pckt->packet.chksum,
            pckt->sums[(int)kind],
            sum);
    }
#endif
}
/******************************************************************************/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
