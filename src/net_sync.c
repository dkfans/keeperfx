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
#include "pre_inc.h"
#include "net_sync.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_network.h"

#include "config.h"
#include "config_effects.h"
#include "front_network.h"
#include "player_data.h"
#include "game_merge.h"
#include "net_game.h"
#include "lens_api.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "frontend.h"
#include "thing_effects.h"
#include "bflib_datetm.h"
#include "vidfade.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct Boing {
  char field_0;
  char comp_player_aggressive;
  char comp_player_defensive;
  char comp_player_construct;
  char comp_player_creatrsonly;
  unsigned char creatures_tend_imprison;
  unsigned char creatures_tend_flee;
  unsigned short hand_over_subtile_x;
  unsigned short hand_over_subtile_y;
  long chosen_room_kind;
  long chosen_room_spridx;
  long chosen_room_tooltip;
  long chosen_spell_type;
  long chosen_spell_spridx;
  long chosen_spell_tooltip;
  long manufactr_element;
  long manufactr_spridx;
  long manufactr_tooltip;
};
/******************************************************************************/
/** Structure used for storing 'localised parameters' when resyncing net game. */

enum
{
    RESYNC_CHUNK = 1024,
    RESYNC_PACKETS_PER_TURN = 8,
};

struct ResyncProgress
{
    struct Boing boing;
    TbBool is_finished;
    uint32_t phase;
    uint32_t max_phase;
};

struct ResyncPacket
{
    uint8_t flags; // 0 = PART, 1 = FIN, 2 = CONFIRM
    uint8_t part;
    uint16_t idx;
    uint8_t data[];
};

struct ResyncPart
{
    uint8_t *data;
    uint32_t total;
};

static struct ResyncPart resync_parts[] =
{
        {(uint8_t *) &game,    sizeof(game), },
        {(uint8_t *) &gameadd, sizeof(gameadd) },
};

static struct ResyncProgress resync_progress;
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

static uint32_t count_max_resync_phase(uint32_t current_phase)
{
    uint32_t ret = current_phase;
    for (int i = 0; i < sizeof(resync_parts) / sizeof(resync_parts[0]); i++)
    {
        struct ResyncPart *part = &resync_parts[i];
        ret += (part->total / RESYNC_CHUNK) + 1;
    }
    return ret;
}

static TbBool resync_server_cb(
        void *context, unsigned long turn, int net_player_idx, unsigned char kind, void *packet_data, short size)
{
    if (kind == PckA_Resync)
    {
        if (net_player_idx == SERVER_ID)
        {
            return false; // Ignore our own packets
        }

    }
    else if (kind == PckA_Frame)
    {
        NETLOG("PckA_Frame ignored while resyncing");
    }
    else
    {
        NETLOG("Unexpected packet kind %d", kind);
    }
    return false;
}

static TbBool send_resync_game(void)
{
    if (resync_progress.phase == 1) // Assuming lossless delivery
    {
        resync_progress.phase++;
        resync_progress.max_phase = 1 + count_max_resync_phase(resync_progress.phase);

        NETLOG("Initiating re-sync:%d/%d", resync_progress.phase, resync_progress.max_phase);
    }

    if (resync_progress.phase < resync_progress.max_phase - 1)
    {
        uint32_t p = 2;
        int cnt = 0;
        for (int i = 0; i < sizeof(resync_parts) / sizeof(resync_parts[0]); i++)
        {
            struct ResyncPart *part = &resync_parts[i];
            struct ResyncPacket *pckt;
            uint32_t tail = part->total;
            uint16_t idx = 0;
            while (tail != 0)
            {
                uint32_t part_size;
                part_size = tail > RESYNC_CHUNK ? RESYNC_CHUNK : tail;

                if ((p >= resync_progress.phase) && (cnt < RESYNC_PACKETS_PER_TURN))
                {
                    resync_progress.phase++;
                    pckt = LbNetwork_AddPacket(PckA_Resync, game.play_gameturn,
                                               sizeof(struct ResyncPacket) + part_size);
                    pckt->idx = idx;
                    pckt->part = i;
                    memcpy(pckt->data, part->data + (idx * RESYNC_CHUNK), part_size);
                    resync_progress.phase++;
                    cnt++;
                }
                idx++; // Assuming part->total < RESYNC_CHUNG * MAX_INT16
                tail -= part_size;
            }
        }
    }

    if (LbNetwork_Exchange(game.packets, resync_server_cb) != 0)
    {
        ERRORLOG("LbNetwork_Exchange failed");
        return false;
    }

    if (resync_progress.phase == resync_progress.max_phase)
    {
        resync_progress.is_finished = 1;
        // TODO wait for confirmation
    }
    return true;
}

static TbBool resync_client_cb(
        void *context, unsigned long turn, int net_player_idx, unsigned char kind, void *packet_data, short size)
{
    if (kind == PckA_Resync)
    {
        if (net_player_idx != SERVER_ID)
        {
            return false; // Our own confirmation is not 
        }
        struct ResyncPacket *pckt = packet_data;
        struct ResyncPart *part = &resync_parts[pckt->part];
        assert (pckt->part < (sizeof(resync_parts) / sizeof(resync_parts[0])));
        assert (pckt->idx <= (part->total / RESYNC_CHUNK));
        size_t part_size = size - sizeof(struct ResyncPacket);

        memcpy(part->data + (pckt->idx * RESYNC_CHUNK), pckt->data, part_size);

        NETLOG("Resync %d/%d", resync_progress.phase, resync_progress.max_phase);
        resync_progress.phase++;
    }
    else if (kind == PckA_FrameSrv)
    {
        NETLOG("PckA_FrameSrv ignored while resyncing");
    }
    else
    {
        NETLOG("Unexpected packet kind %d", kind);
    }
    return false;
}

static TbBool receive_resync_game(void)
{
    if (resync_progress.phase == 1) // Assuming lossless delivery
    {
        resync_progress.phase++;
        resync_progress.max_phase = count_max_resync_phase(resync_progress.phase);
        NETLOG("Initiating re-sync:%d/%d", resync_progress.phase, resync_progress.max_phase);
    }

    if (LbNetwork_Exchange(NULL, resync_client_cb) != 0)
    {
        ERRORLOG("LbNetwork_Exchange failed");
        return false;
    }

    if (resync_progress.phase == resync_progress.max_phase)
    {
        NETLOG("Resync done");
        resync_progress.is_finished = 1;
    }
    return true;
}

TbBool resync_callback(void *context, unsigned long turn, int net_player_idx, unsigned char kind, void *packet_data, short size)
{
    if (my_player_number == SERVER_ID) // TODO: my_net_id?
    {
        return resync_server_cb(NULL, turn, net_player_idx, kind, packet_data, size);
    }
    else
    {
        return resync_client_cb(NULL, turn, net_player_idx, kind, packet_data, size);
    }
}

static void store_localised_game_structure(void)
{
    resync_progress.boing.field_0 = game.active_panel_mnu_idx;
    resync_progress.boing.comp_player_aggressive = game.comp_player_aggressive;
    resync_progress.boing.comp_player_defensive = game.comp_player_defensive;
    resync_progress.boing.comp_player_construct = game.comp_player_construct;
    resync_progress.boing.comp_player_creatrsonly = game.comp_player_creatrsonly;
    resync_progress.boing.creatures_tend_imprison = game.creatures_tend_imprison;
    resync_progress.boing.creatures_tend_flee = game.creatures_tend_flee;
    resync_progress.boing.hand_over_subtile_x = game.hand_over_subtile_x;
    resync_progress.boing.hand_over_subtile_y = game.hand_over_subtile_y;
    resync_progress.boing.chosen_room_kind = game.chosen_room_kind;
    resync_progress.boing.chosen_room_spridx = game.chosen_room_spridx;
    resync_progress.boing.chosen_room_tooltip = game.chosen_room_tooltip;
    resync_progress.boing.chosen_spell_type = game.chosen_spell_type;
    resync_progress.boing.chosen_spell_spridx = game.chosen_spell_spridx;
    resync_progress.boing.chosen_spell_tooltip = game.chosen_spell_tooltip;
    resync_progress.boing.manufactr_element = game.manufactr_element;
    resync_progress.boing.manufactr_spridx = game.manufactr_spridx;
    resync_progress.boing.manufactr_tooltip = game.manufactr_tooltip;
}

void recall_localised_game_structure(void)
{
    game.active_panel_mnu_idx = resync_progress.boing.field_0;
    game.comp_player_aggressive = resync_progress.boing.comp_player_aggressive;
    game.comp_player_defensive = resync_progress.boing.comp_player_defensive;
    game.comp_player_construct = resync_progress.boing.comp_player_construct;
    game.comp_player_creatrsonly = resync_progress.boing.comp_player_creatrsonly;
    game.creatures_tend_imprison = resync_progress.boing.creatures_tend_imprison;
    game.creatures_tend_flee = resync_progress.boing.creatures_tend_flee;
    game.hand_over_subtile_x = resync_progress.boing.hand_over_subtile_x;
    game.hand_over_subtile_y = resync_progress.boing.hand_over_subtile_y;
    game.chosen_room_kind = resync_progress.boing.chosen_room_kind;
    game.chosen_room_spridx = resync_progress.boing.chosen_room_spridx;
    game.chosen_room_tooltip = resync_progress.boing.chosen_room_tooltip;
    game.chosen_spell_type = resync_progress.boing.chosen_spell_type;
    game.chosen_spell_spridx = resync_progress.boing.chosen_spell_spridx;
    game.chosen_spell_tooltip = resync_progress.boing.chosen_spell_tooltip;
    game.manufactr_element = resync_progress.boing.manufactr_element;
    game.manufactr_spridx = resync_progress.boing.manufactr_spridx;
    game.manufactr_tooltip = resync_progress.boing.manufactr_tooltip;
}

void resync_game(void)
{
    SYNCDBG(2,"Starting");
    if (resync_progress.phase == 0)
    {
        reset_eye_lenses();
        store_localised_game_structure();
        resync_progress.phase++;
    }
    int i = get_resync_sender(); // TODO: Fix this
    NETLOG("Resync sender is player%d", i);
    if (is_my_player_number(i))
    {
        send_resync_game();
    } else
    {
        receive_resync_game();
    }

    if (resync_progress.is_finished)
    {
        recall_localised_game_structure();
        reinit_level_after_load();
        clear_flag(game.system_flags, GSF_NetGameNoSync);
    }
}

static TbBool perform_checksum_verification_cb(void *context, unsigned long turn, int net_player_idx, unsigned char kind, void *packet_data, short size)
{
    if (kind == PckA_LevelExactCheck)
    {
        NETLOG("Got %d/%d", net_player_idx, game.active_players_count);
        struct Packet *pckt = ((struct Packet*) context) + net_player_idx;
        game.action_rand_seed = pckt->actn_par1;
        pckt->net_flags = PACKET_IS_NEW;
        memcpy(pckt, packet_data, size);
    }
    else
    {
        NETLOG("Unexpected packet %d from %d", kind, net_player_idx);
    }
    return false;
}

/**
 * Exchanges verification packets between all players, making sure level data is identical.
 * @return Returns true if all players return same checksum.
 */
CoroutineLoopState perform_checksum_verification(CoroutineLoop *con)
{
    short result = true;
    unsigned long checksum_mem = 0;
    for (int i = 1; i < THINGS_COUNT; i++)
    {
        struct Thing* thing = thing_get(i);
        if (thing_exists(thing)) {
            checksum_mem += thing->mappos.z.val + thing->mappos.y.val + thing->mappos.x.val;
        }
    }
    TbClockMSec now = LbTimerClock();
    if (my_player_number != SERVER_ID) // Everyone have to wait for a server
    {
        if (now > coroutine_vars(con)[0])
        {
            coroutine_vars(con)[0] = now + 500; // two times per second
            clear_packets();
            struct Packet *pckt = LbNetwork_AddPacket(PckA_LevelExactCheck, 0, sizeof(struct Packet));
            set_packet_action(pckt, PckA_LevelExactCheck, 0, 0, 0, 0);
            pckt->chksum = checksum_mem + game.action_rand_seed;
            NETLOG("Sending a checksum from %d", my_player_number);
        }
    }
    else
    {
        get_packet(my_player_number)->action = PckA_LevelExactCheck;
        get_packet(my_player_number)->chksum = checksum_mem + game.action_rand_seed;
    }
    if (LbNetwork_Exchange(game.packets, &perform_checksum_verification_cb))
    {
        ERRORLOG("Network exchange failed on level checksum verification");
        return CLS_ERROR;
    }
    for (int i = 0; i < game.active_players_count; i++)
    {
        if (get_packet(i)->action != PckA_LevelExactCheck)
        {
            if (now > coroutine_vars(con)[1])
            {
                coroutine_vars(con)[1] = now + 750;
                NETLOG("Waiting for a checksum from %d", i);
            }
            // Wait for message from all sides
            return CLS_REPEAT;
        }
    }
    if ( checksums_different() )
    {
        ERRORLOG("Level checksums different for network players");
        result = false;
    }
    NETLOG("Checksums are %s", result? "ok" :"bad");
    if (!result)
    {
        create_frontend_error_box(5000, get_string(GUIStr_NetUnsyncedMap));
        return CLS_ERROR;
    }
    if (my_player_number == SERVER_ID)
    {   // Last chance for clients to catch up
        struct Packet* pckt = LbNetwork_AddPacket(PckA_LevelExactCheck, 0, sizeof(struct Packet));
        set_packet_action(pckt, PckA_LevelExactCheck, (long)game.action_rand_seed, 0, 0, 0);
        pckt->chksum = checksum_mem + game.action_rand_seed;
        NETLOG("Sending a checksum from %d", my_player_number);
    }
    NETLOG("Done");
    return CLS_CONTINUE;
}

TbBigChecksum compute_player_checksum(struct PlayerInfo *player)
{
    TbBigChecksum sum = 0;
    if (((player->allocflags & PlaF_CompCtrl) == 0) && (player->acamera != NULL))
    {
        struct Coord3d* mappos = &(player->acamera->mappos);
        sum += (TbBigChecksum)player->instance_remain_rurns + (TbBigChecksum)player->instance_num;
        sum += (TbBigChecksum)mappos->x.val + (TbBigChecksum)mappos->z.val + (TbBigChecksum)mappos->y.val;
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
            sum += compute_player_checksum(player);
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
void player_packet_checksum_add(PlayerNumber plyr_idx, TbBigChecksum sum, const char *area_name)
{
    struct Packet* pckt = get_packet(plyr_idx);
    pckt->chksum += sum;
    SYNCDBG(9,"Checksum increase from %s is %06lX",area_name,(unsigned long)sum);
}

/**
 * Checks if all active players packets have same checksums.
 * @return Returns false if all checksums are same; true if there's mismatch.
 */
short checksums_different()
{
    TbChecksum checksum = 0;
    unsigned short is_set = false;
    int plyr = -1;
    int checked = 0;
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (is_human_player(player))
        {
            struct Packet* pckt = get_packet_direct(player->packet_num);
            if ((pckt->net_flags & PACKET_IS_NEW) == 0)
            {
                continue;
            }
            checked++;
            if (!is_set)
            {
                checksum = pckt->chksum;
                is_set = true;
                plyr = i;
            }
            else if (checksum != pckt->chksum)
            {
                ERRORLOG("Checksums %08x(%d) != %08x(%d) turn: %ld", checksum, plyr, pckt->chksum, i, game.play_gameturn);

                return true;
            }
        }
    }
    NETLOG("checked:%d", checked);
    return false;
}

TbBigChecksum get_thing_checksum(const struct Thing* thing)
{
    SYNCDBG(18, "Starting");
    if (!thing_exists(thing))
        return 0;
    TbBigChecksum csum = (ulong)thing->class_id + ((ulong)thing->model << 4) + (ulong)thing->owner;
    if (thing->class_id == TCls_Creature)
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        csum += (ulong)cctrl->inst_turn + (ulong)cctrl->instance_id
            + (ulong)thing->max_frames + (ulong)thing->current_frame;
    }
    else if ((thing->class_id == TCls_EffectElem) || (thing->class_id == TCls_AmbientSnd))
    {
        // No syncing on Effect Elements or Sounds
    }
    else if (thing->class_id == TCls_Effect)
    {
        const struct EffectConfigStats* effcst = get_effect_model_stats(thing->model);
        if (effcst->area_affect_type != AAffT_None)
        {
            csum += (ulong)thing->mappos.z.val +
                (ulong)thing->mappos.x.val +
                (ulong)thing->mappos.y.val +
                (ulong)thing->health;
        }
        //else: No syncing on Effects that do not affect the area around them
    }
    else
    {
        csum += (ulong)thing->mappos.z.val +
            (ulong)thing->mappos.x.val +
            (ulong)thing->mappos.y.val +
            (ulong)thing->health;
    }
    return csum * thing->index;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
