/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_filecheck.c
 *     Checksums of loaded files for multiplayer startup verification.
 * @par Purpose:
 *     Tracks checksums of files loaded for a level and reports which differ from the host.
 * @par Comment:
 *     Files are identified by their path relative to the game directory.
 * @author   KeeperFX Team
 * @date     06 Nov 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_filecheck.h"
#include "bflib_dernc.h"
#include "bflib_fileio.h"
#include "config.h"
#include "config_keeperfx.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "net_game.h"
#include "kfx/platform/UserDirs.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
static struct LoadedFileChecksums loaded_file_checksums;
static size_t loaded_file_names_length;

TbBigChecksum calculate_file_checksum(const char *fname)
{
    int32_t file_size = (int32_t)LbFileLengthRnc(fname);
    TbBigChecksum checksum = 0;
    CHECKSUM_ADD(checksum, file_size);
    if (file_size <= 0) {
        return checksum;
    }
    unsigned char *file_buf = malloc(file_size);
    if (file_buf != NULL && LbFileLoadAt(fname, file_buf) == file_size) {
        CHECKSUM_ADD(checksum, rnc_crc(file_buf, file_size));
    }
    free(file_buf);
    return checksum;
}

void calculate_network_startup_map_checksums(TbBigChecksum checksums[NETWORK_STARTUP_MAP_FILE_COUNT])
{
    LevelNumber lvnum = get_loaded_level_number();
    short fgroup = get_level_fgroup(lvnum);
    for (int i = 0; i < NETWORK_STARTUP_MAP_FILE_COUNT; i++) {
        char* fname = prepare_file_fmtpath(fgroup, "map%05u.%s", lvnum, network_startup_compare_files[i]);
        checksums[i] = calculate_file_checksum(fname);
    }
}

static int find_loaded_file(const struct LoadedFileChecksums *files, const char *fname)
{
    const char *name = files->names;
    for (int i = 0; i < files->count; i++) {
        if (strcmp(name, fname) == 0) {
            return i;
        }
        name += strlen(name) + 1;
    }
    return -1;
}

void reset_loaded_file_checksums(void)
{
    memset(&loaded_file_checksums, 0, sizeof(loaded_file_checksums));
    loaded_file_names_length = 0;
}

const struct LoadedFileChecksums *get_loaded_file_checksums(void)
{
    return &loaded_file_checksums;
}

static void normalise_path(const char *path, char *out, size_t out_size)
{
    size_t length = 0;
    TbBool segment_start = true;
    for (const char *character = path; *character != '\0' && length + 1 < out_size; character++) {
        char normalised = (char)tolower((unsigned char)*character);
        if (normalised == '\\') {
            normalised = '/';
        }
        if (normalised == '/') {
            if (!segment_start) {
                out[length++] = '/';
            }
            segment_start = true;
            continue;
        }
        if (segment_start && normalised == '.' && (character[1] == '/' || character[1] == '\\' || character[1] == '\0')) {
            continue;
        }
        segment_start = false;
        out[length++] = normalised;
    }
    while (length > 0 && out[length - 1] == '/') {
        length--;
    }
    out[length] = '\0';
}

static void make_loaded_file_name(const char *fname, char *out, size_t out_size)
{
    normalise_path(fname, out, out_size);
    // A file the player added to their data folder is compared by the same name as the game's copy it replaces.
    // When one folder is in another (a data folder inside the game folder), the deepest one names the file.
    const char *base_directories[] = {keeper_runtime_directory, install_info.inst_path, userdirs_get(UsrDir_Data)};
    size_t best_length = 0;
    for (int i = 0; i < sizeof(base_directories) / sizeof(base_directories[0]); i++) {
        char base[FULLPATH_SIZE];
        normalise_path(base_directories[i], base, sizeof(base));
        size_t base_length = strlen(base);
        if (base_length > best_length && strncmp(out, base, base_length) == 0 && out[base_length] == '/') {
            best_length = base_length;
        }
    }
    if (best_length > 0) {
        memmove(out, out + best_length + 1, strlen(out + best_length + 1) + 1);
    }
}

void add_loaded_file_checksum(const char *fname)
{
    if (!network_is_active()) {
        return;
    }
    char relative_fname[FULLPATH_SIZE];
    make_loaded_file_name(fname, relative_fname, sizeof(relative_fname));
    if (!LbFileExists(fname) || find_loaded_file(&loaded_file_checksums, relative_fname) >= 0) {
        return;
    }
    TbBigChecksum checksum = calculate_file_checksum(fname);
    NETDBG(6, "Loaded file %s has checksum %08x", relative_fname, checksum);
    size_t name_size = strlen(relative_fname) + 1;
    if (loaded_file_checksums.count >= LOADED_FILE_TRACKED_COUNT || loaded_file_names_length + name_size > LOADED_FILE_NAMES_SIZE) {
        WARNLOG("Loaded file list is full, cannot checksum %s individually", relative_fname);
        CHECKSUM_ADD(loaded_file_checksums.untracked_checksum, checksum);
        return;
    }
    memcpy(loaded_file_checksums.names + loaded_file_names_length, relative_fname, name_size);
    loaded_file_names_length += name_size;
    loaded_file_checksums.checksums[loaded_file_checksums.count] = checksum;
    loaded_file_checksums.count++;
}

void sanitise_received_loaded_file_checksums(struct LoadedFileChecksums *files)
{
    files->names[LOADED_FILE_NAMES_SIZE - 1] = '\0';
    size_t names_length = 0;
    int count = 0;
    while (count < files->count && count < LOADED_FILE_TRACKED_COUNT && names_length < LOADED_FILE_NAMES_SIZE) {
        names_length += strlen(files->names + names_length) + 1;
        count++;
    }
    files->count = count;
}

static void report_differing_file(const char *user_name, const char *fname, const char *reason, char *list, size_t list_size)
{
    JUSTMSG("File differs between %s and host: %s%s", user_name, fname, reason);
    size_t length = strlen(list);
    const char *separator = "\n";
    if (length == 0) {
        separator = "";
    }
    snprintf(list + length, list_size - length, "%s%s", separator, fname);
}

TbBool list_files_differing_from_host(const struct LoadedFileChecksums *files, const struct LoadedFileChecksums *host, const char *user_name, char *list, size_t list_size)
{
    list[0] = '\0';
    const char *name = host->names;
    for (int i = 0; i < host->count; i++) {
        int index = find_loaded_file(files, name);
        if (index < 0) {
            report_differing_file(user_name, name, " (only loaded by host)", list, list_size);
        } else if (files->checksums[index] != host->checksums[i]) {
            report_differing_file(user_name, name, "", list, list_size);
        }
        name += strlen(name) + 1;
    }
    name = files->names;
    for (int i = 0; i < files->count; i++) {
        if (find_loaded_file(host, name) < 0) {
            report_differing_file(user_name, name, " (not loaded by host)", list, list_size);
        }
        name += strlen(name) + 1;
    }
    if (files->untracked_checksum != host->untracked_checksum) {
        report_differing_file(user_name, "(files beyond the tracked limit)", "", list, list_size);
    }
    return list[0] != '\0';
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
