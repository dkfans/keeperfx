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
#include "net_checksums.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct CatalogueEntry;
struct Packet;

#pragma pack(1)

// save file header for .pck files.
// (Bump the version if this struct or the .pck format changes.)
#define PACKET_SAVE_HEAD_VER 5

enum PacketSaveHeadFlags {
    PSHF_Checksum   = 0x01,
    PSHF_Compressed = 0x02,
    PSHF_MultiGame  = 0x04,
};

enum UserStartFlags {
    USF_CheatsEnabled = 0x01,
    USF_SkipHeartZoom = 0x02,
};

// a user's settings as they were at the start of a game.
struct UserStartSettings {
    uint8_t video_rotate_mode;
    uint8_t flags; // UserStartFlags
    uint16_t tendencies; // CrTend_* flags
    uint8_t highlight_mode;
    int32_t isometric_tilt;
    uint32_t isometric_view_zoom_level;
    uint32_t frontview_zoom_level;
    uint32_t zoom_distance;
    uint32_t frontview_zoom_distance;
};

// Replay file header.
struct PacketSaveHead {
    unsigned short game_ver_major;
    unsigned short game_ver_minor;
    unsigned short game_ver_release;
    unsigned short game_ver_build;
    uint32_t level_num;
    PlayerBitFlags players_exist;
    PlayerBitFlags players_comp;
    uint8_t flags; // PacketSaveHeadFlags
    uint32_t action_seed;
    TbBigChecksum map_checksums[NETWORK_STARTUP_MAP_FILE_COUNT];
    struct UserStartSettings user_start[MAX_NET_USERS];
    signed char user_players[MAX_NET_USERS];
    signed char recording_user;
    char frontend_alliances;
    char user_names[MAX_NET_USERS][20];
};

#pragma pack()
/******************************************************************************/

/*
 * State relating to watching and saving replays, including whether replay mode is enabled.
 * Not stored in the main game struct, since resyncs shouldn't overwrite this, and shouldn't
 * be part of game saves.
 */
struct ReplayState {
    unsigned char save_enable;
    unsigned char load_enable;
    char fname[150];
    char fopened;
    TbFileHandle fp;
    unsigned int file_pos;
    struct PacketSaveHead head;
    uint32_t turns_stored;
    uint32_t turns_fastforward;
    unsigned char loading_in_progress;
    unsigned char checksum_verify;
    uint32_t log_things_start_turn;
    uint32_t log_things_end_turn;
    uint32_t turns_packetoff;
    GameTurn pckt_gameturn;
};

extern struct ReplayState replay;

extern unsigned long initial_replay_seed;

TbBigChecksum compute_replay_integrity(void);
void restore_users_from_packet_save(void);
TbBool setup_auto_replay_save(void);
TbBool open_new_packet_file_for_save(void);
void load_packets_for_turn(GameTurn nturn);
void verify_replay_checksum(void);
TbBool open_packet_file_for_load(char *fname, struct CatalogueEntry *centry);
short save_packets(void);
void replay_forget_saved_turn(void);
void replay_apply_pending_resync(void);
void close_packet_file(void);
void stop_replay_recording(const char *reason);
void replay_record_chat_message(NetUserId user, const char *message, MapCoord cursor_x, MapCoord cursor_y);
void replay_record_paused_action(NetUserId user, const struct Packet *pckt);
void replay_record_resync(const void *message, size_t message_size);
void replay_record_network_stopped(void);
TbBool replay_playback_is_paused(void);
void set_replay_playback_paused(TbBool paused);
TbBool reinit_packets_after_load(void);
void disable_packet_mode(void);
TbBool verify_replay_map_checksums(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
