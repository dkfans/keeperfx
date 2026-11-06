/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_filecheck.h
 *     Header file for net_filecheck.c.
 * @par Purpose:
 *     Checksums of loaded files, compared between multiplayer users at startup.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     06 Nov 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_NET_FILECHECK_H
#define DK_NET_FILECHECK_H

#include "globals.h"
#include "bflib_basics.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
static const char * const network_startup_compare_files[] = {
    "slb", "dat", "clm", "own", "wib", "inf", "flg", "wlb", "slx",
    "lgtfx", "lgt", "aptfx", "apt", "tngfx", "tng", "txt", "lua"
};

#define NETWORK_STARTUP_MAP_FILE_COUNT (sizeof(network_startup_compare_files) / sizeof(network_startup_compare_files[0]))

#define LOADED_FILE_TRACKED_COUNT 384
#define LOADED_FILE_NAMES_SIZE 16384

#pragma pack(1)
struct LoadedFileChecksums {
    TbBigChecksum untracked_checksum;
    uint16_t count;
    TbBigChecksum checksums[LOADED_FILE_TRACKED_COUNT];
    char names[LOADED_FILE_NAMES_SIZE];
};
#pragma pack()

TbBigChecksum calculate_file_checksum(const char *fname);
void calculate_network_startup_map_checksums(TbBigChecksum checksums[NETWORK_STARTUP_MAP_FILE_COUNT]);
void reset_loaded_file_checksums(void);
const struct LoadedFileChecksums *get_loaded_file_checksums(void);
void add_loaded_file_checksum(const char *fname);
void sanitise_received_loaded_file_checksums(struct LoadedFileChecksums *files);
TbBool list_files_differing_from_host(const struct LoadedFileChecksums *files, const struct LoadedFileChecksums *host, const char *user_name, char *list, size_t list_size);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
