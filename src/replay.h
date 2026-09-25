/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file replay.h
 *     Header file for replay.c.
 * @par Purpose:
 *     Replay recording and playback.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_REPLAY_H
#define DK_REPLAY_H

#include "bflib_basics.h"
#include "globals.h"
#include "net_main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct CatalogueEntry;

#pragma pack(1)

// save file header for .pck files.
// (Bump the version if this struct or the .pck format changes.)
#define PACKET_SAVE_HEAD_VER 2

enum PacketSaveHeadFlags {
    PSHF_Checksum   = 0x01,
    PSHF_Compressed = 0x02,
};

struct PacketSaveHead {
    unsigned short game_ver_major;
    unsigned short game_ver_minor;
    unsigned short game_ver_release;
    unsigned short game_ver_build;
    uint32_t level_num;
    PlayerBitFlags players_exist;
    PlayerBitFlags players_comp;
    uint32_t isometric_view_zoom_level;
    uint32_t frontview_zoom_level;
    int isometric_tilt;
    unsigned char video_rotate_mode;
    uint8_t flags; // PacketSaveHeadFlags
    uint32_t action_seed;
    TbBool default_imprison_tendency;
    TbBool default_flee_tendency;
    TbBool skip_heart_zoom;
    TbBool highlight_mode;
    signed char user_players[MAX_NET_USERS];
    signed char recording_user;
    char frontend_alliances;
    char user_names[MAX_NET_USERS][20];
};

#pragma pack()
/******************************************************************************/
extern unsigned long initial_replay_seed;

TbBigChecksum compute_replay_integrity(void);
void restore_users_from_packet_save(void);
TbBool setup_auto_replay_save(void);
TbBool open_new_packet_file_for_save(void);
void load_packets_for_turn(GameTurn nturn);
TbBool open_packet_file_for_load(char *fname, struct CatalogueEntry *centry);
short save_packets(void);
void close_packet_file(void);
TbBool reinit_packets_after_load(void);
void disable_packet_mode(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
