/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file packets_misc.c
 *     Processing packets with cheat commands.
 * @par Purpose:
 *     Functions for creating and executing packets.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     20 Sep 2020 - 20 Sep 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "net_game.h"
#include "pre_inc.h"
#include "kfx/renderer/RendererManager.h"
#include "packets.h"
#include "net_exchange_gameplay.h"
#include "bflib_datetm.h"
#include "front_landview.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define MULTIPLAYER_PAUSE_COOLDOWN_MS 500
struct Packet bad_packet;
unsigned long last_pause_toggle_time = 0;
extern TbBool force_player_num;
extern TbBool keeper_screen_redraw(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif

NetUserId get_local_user(void)
{
    if (game.packet_load_enable)
    {
        for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
            if (get_net_user_player_number(user) == my_player_number)
                return user;
        }
    }
    if (game.packet_load_enable && !force_player_num)
    {
        NetUserId rec_user = game.packet_save_head.recording_user;
        if ((rec_user >= 0) && (rec_user < MAX_NET_USERS)
         && (get_net_user_player_number(rec_user) >= 0))
            return rec_user;
    }

    if (!network_is_active())
    {
        return SOLO_HUMAN_ID;
    }

    if (netstate.my_id >= 0 && netstate.my_id < MAX_NET_USERS)
    {
        return netstate.my_id;
    }

    assert(false);
    return 0;
}

struct Packet *get_local_packet(void)
{
    return get_packet(get_local_user());
}

void set_players_packet_action(struct PlayerInfo *player, unsigned char pcktype,
        unsigned long par1, unsigned long par2, unsigned short par3, unsigned short par4)
{
    struct Packet* pckt = get_packet(player->user_id);
    pckt->actn_par1 = par1;
    pckt->actn_par2 = par2;
    pckt->actn_par3 = par3;
    pckt->actn_par4 = par4;
    pckt->action = pcktype;
}

unsigned char get_players_packet_action(struct PlayerInfo *player)
{
    struct Packet* pckt = get_packet(player->user_id);
    return pckt->action;
}

void set_packet_control(struct Packet *pckt, unsigned long flag)
{
  pckt->control_flags |= flag;
}

void set_players_packet_control(struct PlayerInfo *player, unsigned long flag)
{
    struct Packet* pckt = get_packet(player->user_id);
    pckt->control_flags |= flag;
}

void unset_packet_control(struct Packet *pckt, unsigned long flag)
{
    pckt->control_flags &= ~flag;
}

void unset_players_packet_control(struct PlayerInfo *player, unsigned long flag)
{
    struct Packet* pckt = get_packet(player->user_id);
    pckt->control_flags &= ~flag;
}

void set_players_packet_position(struct Packet *pckt, long x, long y, unsigned char context)
{
    pckt->pos_x = x;
    pckt->pos_y = y;
    pckt->control_flags |= PCtr_MapCoordsValid;
    pckt->additional_packet_values &= ~PCAdV_ContextMask;
    pckt->additional_packet_values |= (context << 1);
}

/**
 * Gives a pointer to the packet of a given network user.
 * @param user Network user id. Note that it may differ from the player index.
 * @return Returns Packet pointer. On error, returns a dummy structure.
 */
struct Packet *get_packet(NetUserId user)
{
    if ((user < 0) || (user >= PACKETS_COUNT))
        return INVALID_PACKET;
    return &game.packets[user];
}

void clear_packets(void)
{
    for (int i = 0; i < PACKETS_COUNT; i++)
    {
        memset(&game.packets[i], 0, sizeof(struct Packet));
    }
}

void post_init_packets(void)
{
    SYNCDBG(6,"Starting");
    initialize_packet_history();
    clear_packets();
}

void dump_memory_to_file(const char * fname, const char * buf, size_t len)
{
    FILE* file = fopen(fname, "w");
    fwrite(buf, 1, len, file);
    fflush(file);
    fclose(file);
}

void write_debug_packets(void)
{
    //note, changed this to be more general and to handle multiplayer where there can
    //be several players writing to same directory if testing on local machine
    char filename[32];
    snprintf(filename, sizeof(filename), "%s%u.%s", "keeperd", my_player_number, "pck");
    dump_memory_to_file(filename, (char*) game.packets, sizeof(game.packets));
}

void write_debug_screenpackets(void)
{
    char filename[32];
    snprintf(filename, sizeof(filename), "%s%u.%s", "keeperd", my_player_number, "spck");
    dump_memory_to_file(filename, (char*) net_screen_packet, sizeof(net_screen_packet));
}

void set_packet_pause_toggle()
{
    struct PlayerInfo* player = get_my_player();
    if (player_invalid(player))
        return;
    if (player->user_id >= PACKETS_COUNT)
        return;
    if (game.game_kind != GKind_LocalGame) {
        unsigned long current_time = LbTimerClock();
        if (current_time - last_pause_toggle_time < MULTIPLAYER_PAUSE_COOLDOWN_MS) {
            MULTIPLAYER_LOG("set_packet_pause_toggle: cooldown active, ignoring");
            return;
        }
        last_pause_toggle_time = current_time;
    }
    if ((game.operation_flags & GOF_Paused) == 0) {
        set_players_packet_action(player, PckA_TogglePause, 1, 0, 0, 0);
        return;
    }
    if (game.game_kind != GKind_LocalGame) {
        MULTIPLAYER_LOG("set_packet_pause_toggle: broadcasting unpause");
        unpausing_in_progress = 1;
        keeper_screen_redraw();
        RendererPresentFrame();
        LbNetwork_BroadcastUnpause();
        if (network_is_host()) {
            process_pause_packet(0, 0);
        }
        unpausing_in_progress = 0;
        return;
    }
    process_pause_packet(0, 0);
}
