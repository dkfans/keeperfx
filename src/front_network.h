/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_network.h
 *     Header file for front_network.c.
 * @par Purpose:
 *     Front-end menus for network games.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     11 Mar 2010 - 09 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONTNET_H
#define DK_FRONTNET_H

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_coroutine.h"
#include "bflib_netsession.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define NET_SERVICE_LEN 64

enum FrontendNetService {
    FrontendNetSvc_Skirmish = -1,
    FrontendNetSvc_Online = 0,
    FrontendNetSvc_LAN = 1,
};

#pragma pack(1)

/******************************************************************************/

struct ConfigInfo {
    char str_join[20];
    char net_player_name[20];
};

struct TbNetworkPlayerName {
    char name[20];
};

extern int fe_network_active;
extern int net_service_index_selected;
extern struct TbNetworkSessionNameEntry *net_session[SESSION_ENTRIES_COUNT];
extern int32_t net_number_of_sessions;
extern int32_t net_session_index_active;
extern struct TbNetworkPlayerName net_player[MAX_NET_USERS];
extern struct ConfigInfo net_config_info;
extern char net_service[16][NET_SERVICE_LEN];
extern char net_player_name[20];
extern char tmp_net_player_name[24];

#pragma pack()
/******************************************************************************/
void process_network_error(int32_t errcode);
void draw_out_of_sync_box(int32_t a1, int32_t a2, int32_t box_width);
void display_attempting_to_join_message(int remaining_s);
void reset_attempting_to_join_cancel(void);
TbBool attempting_to_join_cancel_requested(void);
void setup_alliances(void);
void frontnet_service_setup(void);
void frontnet_session_setup(void);
void frontnet_start_setup(void);
void frontnet_service_update(void);
void frontnet_session_update(void);
void frontnet_start_update(void);
TbBool frontnet_start_level(const char *campaign_fname, LevelNumber lvnum);
void process_frontend_chat_message(int player_id, const char *message);
TbBool frontnet_service_selected(enum FrontendNetService service);
void enum_sessions_callback(struct TbNetworkCallbackData *netcdat, void *ptr);

void net_load_config_file(void);
void net_write_config_file(void);

void frontnet_send_campaign_change_message(const char *campaign_fname);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
