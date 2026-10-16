/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_archon.c
 *     Shared between-levels flow of an archon (shared keeper) co-op campaign.
 * @par Purpose:
 *     Keeps every user of an archon campaign session together between levels:
 *     the shared campaign map (network land view), the level briefing and the
 *     level statistics screen. The host's campaign progress decides which levels
 *     are available and the host makes every choice; joiners follow.
 * @par Comment:
 *     The frontend is not lockstep. The host opens the shared map with a frontend
 *     chat message "<campaign>:@<level>[,<level>...]" (the land view picture level,
 *     then the ensigns its progress shows); users report their screen in the
 *     ScreenPacket action_par2 flags (ArchonFe_*).
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "front_archon.h"

#include <string.h>
#include <stdlib.h>

#include "globals.h"
#include "bflib_basics.h"
#include "config.h"
#include "config_campaigns.h"
#include "front_landview.h"
#include "front_network.h"
#include "frontend.h"
#include "game_legacy.h"
#include "game_saves.h"
#include "net_exchange_common.h"
#include "net_game.h"
#include "net_lobby.h"
#include "net_main.h"
#include "net_spectator.h"
#include "player_data.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
char archon_campaign_fname[80] = "";

// local frontend state; this machine (host or joiner) is in the shared campaign flow
static TbBool archon_flow_on = false;
static TbBool archon_map_pending = false;
// the land view picture level, then the ensigns the host's progress shows
static LevelNumber archon_map_levels[ARCHON_MAP_LEVELS_MAX];
static int archon_map_levels_num = 0;
/******************************************************************************/
TbBool archon_flow_active(void)
{
    return archon_flow_on && network_is_active();
}

void archon_flow_reset(void)
{
    archon_flow_on = false;
    archon_map_pending = false;
    archon_map_levels_num = 0;
}

/**
 * Called when a level ends: after a level of an archon co-op campaign the session stays in the shared flow,
 * also when the level was started directly (e.g. with -level) rather than from the shared map.
 */
void archon_flow_level_ended(void)
{
    if (archon_campaign_active() && network_is_active())
        archon_flow_on = true;
}

LevelNumber archon_map_level(int idx)
{
    if ((idx < 0) || (idx >= archon_map_levels_num))
        return SINGLEPLAYER_NOTSTARTED;
    return archon_map_levels[idx];
}

int archon_map_levels_count(void)
{
    return archon_map_levels_num;
}

TbBool archon_map_level_visible(LevelNumber lvnum)
{
    if (lvnum <= 0)
        return false;
    for (int i = 1; i < archon_map_levels_num; i++)
    {
        if (archon_map_levels[i] == lvnum)
            return true;
    }
    return false;
}

static TbBool archon_load_campaign(const char *cmpgn_fname)
{
    char campaign_file[DISKPATH_SIZE];
    uint8_t pack = prepare_campaign_file_name(cmpgn_fname, campaign_file, sizeof(campaign_file));
    if (pack == CampgnT_Default)
        pack = CampgnT_Campaign;
    if (!change_campaign(pack, campaign_file) || (strcasecmp(campaign.fname, campaign_file) != 0))
    {
        ERRORLOG("Unable to load campaign '%s' for the shared campaign map", cmpgn_fname);
        return false;
    }
    return true;
}

/**
 * Tells whether every user playing in the session reports the given screen flag.
 */
TbBool archon_flow_users_report(unsigned char flag)
{
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        if (!network_user_active(user) || network_user_is_spectator(user))
            continue;
        const struct ScreenPacket *nspck = &net_screen_packet[user];
        if (((nspck->networkstatus_flags & NetStat_PlayerConnected) == 0) || ((nspck->action_par2 & flag) == 0))
            return false;
    }
    return true;
}

/**
 * Host: loads its campaign progress and takes every user of the session to the shared campaign map.
 */
TbBool archon_host_open_campaign_map(void)
{
    if (!network_is_host())
        return false;
    const char *cmpgn_fname = (archon_campaign_fname[0] != '\0') ? archon_campaign_fname : campaign.fname;
    if (!archon_load_campaign(cmpgn_fname))
        return false;
    // the host's own progress decides the land view and the available levels
    load_host_campaign_progress();
    update_ensigns_visibility();
    char base_name[64];
    snprintf(base_name, sizeof(base_name), "%s", campaign.fname);
    char* dot = strrchr(base_name, '.');
    if (dot != NULL)
        *dot = '\0';
    char message[PLAYER_MP_MESSAGE_LEN];
    LevelNumber continue_lvnum = get_continue_level_number();
    int len = snprintf(message, sizeof(message), "%s:@%d", base_name, (int)continue_lvnum);
    int count = 1;
    for (struct LevelInformation* lvinfo = get_first_level_info(); lvinfo != NULL; lvinfo = get_next_level_info(lvinfo))
    {
        if ((lvinfo->lvnum <= 0) || (lvinfo->state != LvSt_Visible))
            continue;
        char item[16];
        int item_len = snprintf(item, sizeof(item), ",%d", (int)lvinfo->lvnum);
        if ((count >= ARCHON_MAP_LEVELS_MAX) || (len + item_len >= (int)sizeof(message)))
        {
            WARNLOG("Too many levels on the shared campaign map, level %d not shown", (int)lvinfo->lvnum);
            break;
        }
        memcpy(message + len, item, item_len + 1);
        len += item_len;
        count++;
    }
    SYNCLOG("Opening shared campaign map: %s", message);
    send_network_chat_message(netstate.my_id, message);
    archon_process_map_message(netstate.my_id, message);
    return true;
}

/**
 * Handles the host's "<campaign>:@<levels>" frontend chat message, which opens the shared campaign map.
 * @return True if the message was a shared campaign map message.
 */
TbBool archon_process_map_message(NetUserId user, const char *message)
{
    const char *separator = strstr(message, ":@");
    if ((separator == NULL) || (separator == message) || (separator - message >= 64))
        return false;
    if (user != SERVER_ID)
        return false;
    char cmpgn_fname[64];
    snprintf(cmpgn_fname, sizeof(cmpgn_fname), "%.*s", (int)(separator - message), message);
    if (!archon_load_campaign(cmpgn_fname))
        return true;
    archon_map_levels_num = 0;
    const char *pos = separator + 2;
    while ((*pos != '\0') && (archon_map_levels_num < ARCHON_MAP_LEVELS_MAX))
    {
        char *end;
        long lvnum = strtol(pos, &end, 10);
        if (end == pos)
            break;
        archon_map_levels[archon_map_levels_num++] = lvnum;
        pos = (*end == ',') ? end + 1 : end;
    }
    if (archon_map_levels_num < 1)
    {
        ERRORLOG("Invalid shared campaign map message \"%s\"", message);
        return true;
    }
    archon_flow_on = true;
    // switched from archon_flow_update(), not while inside the network exchange
    archon_map_pending = true;
    fe_network_active = 1;
    return true;
}

static void archon_stats_exchange(void)
{
    if (!network_is_host() && (netstate.users[SERVER_ID].progress == USER_UNUSED))
    {
        // the host has left; the shared campaign cannot continue
        WARNLOG("Host left the shared campaign");
        LbNetwork_Stop();
        archon_flow_reset();
        frontend_set_state(FeSt_MAIN_MENU);
        return;
    }
    // a level start left in the packets from the shared map would end the exchange before anything is received
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        screen_packet_set_action(&net_screen_packet[user], NetAct_None);
    }
    struct ScreenPacket *nspck = &net_screen_packet[netstate.my_id];
    memset(nspck, 0, sizeof(struct ScreenPacket));
    nspck->networkstatus_flags = NetStat_PlayerConnected;
    nspck->action_par2 = ArchonFe_InStats;
    if (LbNetwork_ExchangeFrontend(nspck, &net_screen_packet, sizeof(struct ScreenPacket)))
    {
        ERRORLOG("LbNetwork_Exchange failed");
    }
}

/**
 * Per frontend frame: switches to the shared map once the host opened it, keeps the
 * session exchanging on the level statistics screen.
 */
void archon_flow_update(void)
{
    if (!archon_flow_on)
        return;
    if (!network_is_active())
    {
        archon_flow_reset();
        return;
    }
    if (archon_map_pending)
    {
        archon_map_pending = false;
        frontend_set_state(FeSt_NETLAND_VIEW);
        return;
    }
    if (frontend_menu_state == FeSt_LEVEL_STATS)
    {
        archon_stats_exchange();
    }
}

/**
 * Leaving the level statistics screen. Only the host continues, and only once every
 * user has reached the screen; everyone then goes to the shared campaign map together.
 * @return True if handled (the session is in the shared campaign flow).
 */
TbBool archon_flow_stats_leave(void)
{
    if (!archon_flow_active())
        return false;
    if (network_is_host() && archon_flow_users_report(ArchonFe_InStats))
    {
        archon_host_open_campaign_map();
    }
    return true;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
