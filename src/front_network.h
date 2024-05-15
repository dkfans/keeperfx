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

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

/******************************************************************************/
extern int fe_network_active;
extern int net_service_index_selected;
extern char tmp_net_player_name[24];

#pragma pack()
/******************************************************************************/
void process_network_error(long errcode);
void draw_out_of_sync_box();
void display_attempting_to_join_message(void);
CoroutineLoopState setup_alliances(CoroutineLoop *con);
void frontnet_service_setup(void);
void frontnet_session_setup(void);
void frontnet_start_setup(void);
void frontnet_service_update(void);
void frontnet_session_update(void);
void frontnet_start_update(CoroutineLoop *context);

void net_load_config_file(void);
void net_write_config_file(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
