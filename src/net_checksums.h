/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_checksums.h
 *     Header file for net_checksums.c.
 * @par Purpose:
 *     Network checksum computation and desync analysis for multiplayer games.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     03 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_NET_CHECKSUMS_H
#define DK_NET_CHECKSUMS_H

#include "globals.h"
#include "bflib_basics.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
struct PlayerInfo;
struct Thing;

static const char * const network_startup_compare_files[] = {
    "slb", "dat", "clm", "own", "wib", "inf", "flg", "wlb", "slx",
    "lgtfx", "lgt", "aptfx", "apt", "tngfx", "tng", "txt", "lua"
};

#define NETWORK_STARTUP_MAP_FILE_COUNT (sizeof(network_startup_compare_files) / sizeof(network_startup_compare_files[0]))

#define LOADED_FILE_TRACKED_COUNT 384

#pragma pack(1)
struct LoadedFileChecksums {
    TbBigChecksum untracked_checksum;
    uint16_t count;
    TbBigChecksum path_hashes[LOADED_FILE_TRACKED_COUNT];
    TbBigChecksum checksums[LOADED_FILE_TRACKED_COUNT];
};
#pragma pack()

void update_turn_checksums(void);
void pack_desync_history_for_resync(void);
void compare_desync_history_from_host(void);
TbBigChecksum get_thing_checksum(const struct Thing *thing);
short checksums_different(void);
TbBigChecksum calculate_file_checksum(const char *fname);
void calculate_network_startup_map_checksums(TbBigChecksum checksums[NETWORK_STARTUP_MAP_FILE_COUNT]);
void reset_loaded_file_checksums(void);
const struct LoadedFileChecksums *get_loaded_file_checksums(void);
void add_loaded_file_checksum(const char *fname);
const char *find_file_differing_from_host(const struct LoadedFileChecksums *files, const struct LoadedFileChecksums *host, NetUserId user);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
