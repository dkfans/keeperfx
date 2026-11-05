/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_filecheck.c
 *     Checksums of loaded files for multiplayer startup verification.
 * @par Purpose:
 *     Tracks checksums of files loaded for a level and reports which differ from the host.
 * @par Comment:
 *     Files are identified by a hash of their path relative to the game directory.
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
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define PATH_HASH_OFFSET_BASIS 2166136261u
#define PATH_HASH_PRIME 16777619u
#define ATTEMPTED_FILE_NAME_COUNT 1024

struct AttemptedFileNames {
    int count;
    TbBigChecksum path_hashes[ATTEMPTED_FILE_NAME_COUNT];
    char names[ATTEMPTED_FILE_NAME_COUNT][DISKPATH_SIZE];
};

static struct LoadedFileChecksums loaded_file_checksums;
static struct AttemptedFileNames attempted_file_names;

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

static TbBigChecksum calculate_path_hash(const char *path)
{
    TbBigChecksum hash = PATH_HASH_OFFSET_BASIS;
    for (const unsigned char *character = (const unsigned char *)path; *character != '\0'; character++) {
        hash = (hash ^ *character) * PATH_HASH_PRIME;
    }
    return hash;
}

static int find_path_hash(const TbBigChecksum *path_hashes, int count, TbBigChecksum path_hash)
{
    for (int i = 0; i < count; i++) {
        if (path_hashes[i] == path_hash) {
            return i;
        }
    }
    return -1;
}

static void remember_attempted_file_name(TbBigChecksum path_hash, const char *relative_fname)
{
    if (find_path_hash(attempted_file_names.path_hashes, attempted_file_names.count, path_hash) >= 0 || attempted_file_names.count >= ATTEMPTED_FILE_NAME_COUNT) {
        return;
    }
    attempted_file_names.path_hashes[attempted_file_names.count] = path_hash;
    snprintf(attempted_file_names.names[attempted_file_names.count], DISKPATH_SIZE, "%s", relative_fname);
    attempted_file_names.count++;
}

static const char *loaded_file_name(TbBigChecksum path_hash)
{
    int index = find_path_hash(attempted_file_names.path_hashes, attempted_file_names.count, path_hash);
    if (index < 0) {
        return "(file name unknown locally)";
    }
    return attempted_file_names.names[index];
}

void reset_loaded_file_checksums(void)
{
    memset(&loaded_file_checksums, 0, sizeof(loaded_file_checksums));
    attempted_file_names.count = 0;
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
    const char *base_directories[] = {keeper_runtime_directory, install_info.inst_path};
    for (int i = 0; i < sizeof(base_directories) / sizeof(base_directories[0]); i++) {
        char base[FULLPATH_SIZE];
        normalise_path(base_directories[i], base, sizeof(base));
        size_t base_length = strlen(base);
        if (base_length > 0 && strncmp(out, base, base_length) == 0 && out[base_length] == '/') {
            memmove(out, out + base_length + 1, strlen(out + base_length + 1) + 1);
            return;
        }
    }
}

void add_loaded_file_checksum(const char *fname)
{
    if (!network_is_active()) {
        return;
    }
    char relative_fname[FULLPATH_SIZE];
    make_loaded_file_name(fname, relative_fname, sizeof(relative_fname));
    TbBigChecksum path_hash = calculate_path_hash(relative_fname);
    remember_attempted_file_name(path_hash, relative_fname);
    if (!LbFileExists(fname) || find_path_hash(loaded_file_checksums.path_hashes, loaded_file_checksums.count, path_hash) >= 0) {
        return;
    }
    TbBigChecksum checksum = calculate_file_checksum(fname);
    NETDBG(6, "Loaded file %s has checksum %08x", relative_fname, checksum);
    if (loaded_file_checksums.count >= LOADED_FILE_TRACKED_COUNT) {
        WARNLOG("Too many loaded files to checksum %s individually", relative_fname);
        CHECKSUM_ADD(loaded_file_checksums.untracked_checksum, checksum);
        return;
    }
    loaded_file_checksums.path_hashes[loaded_file_checksums.count] = path_hash;
    loaded_file_checksums.checksums[loaded_file_checksums.count] = checksum;
    loaded_file_checksums.count++;
}

static void report_differing_file(NetUserId user, const char *fname, const char *reason, char *list, size_t list_size)
{
    JUSTMSG("File differs between %s and host: %s%s", network_user_name(user), fname, reason);
    size_t length = strlen(list);
    snprintf(list + length, list_size - length, "\n%s", fname);
}

TbBool list_files_differing_from_host(const struct LoadedFileChecksums *files, const struct LoadedFileChecksums *host, NetUserId user, char *list, size_t list_size)
{
    list[0] = '\0';
    for (int i = 0; i < host->count; i++) {
        int index = find_path_hash(files->path_hashes, files->count, host->path_hashes[i]);
        if (index < 0) {
            report_differing_file(user, loaded_file_name(host->path_hashes[i]), " (only loaded by host)", list, list_size);
        } else if (files->checksums[index] != host->checksums[i]) {
            report_differing_file(user, loaded_file_name(host->path_hashes[i]), "", list, list_size);
        }
    }
    for (int i = 0; i < files->count; i++) {
        if (find_path_hash(host->path_hashes, host->count, files->path_hashes[i]) < 0) {
            report_differing_file(user, loaded_file_name(files->path_hashes[i]), " (not loaded by host)", list, list_size);
        }
    }
    if (files->untracked_checksum != host->untracked_checksum) {
        report_differing_file(user, "(files beyond the tracked limit)", "", list, list_size);
    }
    return list[0] != '\0';
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
