/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsession.c
 *     Algorithms and data structures for network sessions.
 * @par Purpose:
 *     Sessions support code for network library.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     08 Mar 2009 - 12 Oct 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_netsession.h"
#include "bflib_basics.h"
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <json-dom.h>
#include "ver_defs.h"
#include "post_inc.h"

static const char *const session_phase_names[] = {"Unknown", "Lobby", "In-Game", "In-Landview"};

/******************************************************************************/
void net_copy_name_string(char *dst, const char *src, int32_t max_len)
{
    if (dst == NULL || max_len <= 0) {
        return;
    }
    memset(dst, 0, max_len);
    if (src != NULL) {
        snprintf(dst, max_len, "%s", src);
    }
}
/******************************************************************************/

TbBool net_json_escape(char *output, size_t output_size, const char *input)
{
    if (output_size == 0) {
        return false;
    }
    size_t used = 0;
    for (const unsigned char *c = (const unsigned char *)input; *c; c++) {
        size_t needed = 1;
        if (*c < 32) {
            needed = 6;
        } else if (*c == '"' || *c == '\\') {
            needed = 2;
        }
        if (used + needed >= output_size) {
            output[used] = '\0';
            return false;
        }
        if (*c < 32) {
            used += snprintf(output + used, output_size - used, "\\u%04x", *c);
        } else {
            if (needed == 2) {
                output[used++] = '\\';
            }
            output[used++] = *c;
        }
    }
    output[used] = '\0';
    return true;
}

int net_session_metadata_json(const struct TbNetworkSessionNameEntry *session, char *output, size_t size)
{
    if (size == 0 || session->player_count > SESSION_HUMANS_MAX || session->max_players > SESSION_HUMANS_MAX || session->phase < NetPhase_Unknown || session->phase > NetPhase_InLandview) {
        return -1;
    }
    const char *joinable = "false";
    if (session->joinable) {
        joinable = "true";
    }
    char version[sizeof(session->version) * 6];
    net_json_escape(version, sizeof(version), session->version);
    int used = snprintf(output, size, "\"version\":\"%s\",\"phase\":\"%s\",\"joinable\":%s,\"maxPlayers\":%d,\"players\":[", version, session_phase_names[session->phase], joinable, session->max_players);
    for (int i = 0; i < session->player_count; i++) {
        if (used < 0 || used >= size) {
            return -1;
        }
        char name[NETSP_PLAYER_NAME_MAX_LEN * 6];
        net_json_escape(name, sizeof(name), session->players[i]);
        const char *separator = "";
        if (i > 0) {
            separator = ",";
        }
        used += snprintf(output + used, size - used, "%s\"%s\"", separator, name);
    }
    if (used < 0 || used + 1 >= size) {
        return -1;
    }
    output[used++] = ']';
    output[used] = '\0';
    return used;
}

void net_session_parse_metadata(struct TbNetworkSessionNameEntry *session, const VALUE *root)
{
    session->phase = NetPhase_Unknown;
    session->joinable = 1;
    session->roster_known = 0;
    session->player_count = 0;
    session->max_players = 0;
    session->created_at = 0;
    session->version[0] = '\0';
    memset(session->players, 0, sizeof(session->players));
    if (value_type(root) != VALUE_DICT) {
        return;
    }
    VALUE *created_at = value_dict_get(root, "createdAt");
    if (value_is_compatible(created_at, VALUE_INT64) && value_int64(created_at) > 0) {
        session->created_at = value_int64(created_at);
    }
    const char *version = value_string(value_dict_get(root, "version"));
    if (version) {
        snprintf(session->version, sizeof(session->version), "%s", version);
        size_t length = strlen(session->version);
        while (length > 0 && isspace((unsigned char)session->version[length - 1])) {
            session->version[--length] = '\0';
        }
    }
    const char *phase = value_string(value_dict_get(root, "phase"));
    for (int i = NetPhase_Lobby; phase && i <= NetPhase_InLandview; i++) {
        if (strcmp(phase, session_phase_names[i]) == 0) {
            session->phase = i;
            break;
        }
    }
    VALUE *joinable = value_dict_get(root, "joinable");
    if (value_type(joinable) == VALUE_BOOL) {
        session->joinable = value_bool(joinable);
    }
    VALUE *capacity = value_dict_get(root, "maxPlayers");
    if (value_type(capacity) == VALUE_INT32 && value_int32(capacity) > 0 && value_int32(capacity) <= SESSION_HUMANS_MAX) {
        session->max_players = value_int32(capacity);
    }
    VALUE *players = value_dict_get(root, "players");
    if (value_type(players) != VALUE_ARRAY || value_array_size(players) > SESSION_HUMANS_MAX) {
        return;
    }
    for (int i = 0; i < value_array_size(players); i++) {
        VALUE *entry = value_array_get(players, i);
        const char *name = value_string(entry);
        if (!name || !*name || value_string_length(entry) >= NETSP_PLAYER_NAME_MAX_LEN || strlen(name) != value_string_length(entry)) {
            return;
        }
        for (const unsigned char *c = (const unsigned char *)name; *c; c++) {
            if (*c < 32) {
                return;
            }
        }
        snprintf(session->players[i], sizeof(session->players[i]), "%s", name);
    }
    session->player_count = value_array_size(players);
    session->roster_known = 1;
}

int net_session_incompatible(const struct TbNetworkSessionNameEntry *session)
{
    int32_t major;
    int32_t minor;
    int32_t release;
    int32_t build = 0;
    if (sscanf(session->version, "%d.%d.%d.%d", &major, &minor, &release, &build) < 3) {
        return 0;
    }
    return major != VER_MAJOR || minor != VER_MINOR || release != VER_RELEASE || build != VER_BUILD;
}

enum NetJoinRejection net_session_join_rejection(const struct TbNetworkSessionNameEntry *session)
{
    if (net_session_incompatible(session)) {
        return NetJoin_Version;
    }
    if (session->phase == NetPhase_InGame) {
        return NetJoin_InGame;
    }
    if (!session->joinable) {
        return NetJoin_Locked;
    }
    if (session->roster_known && session->max_players && session->player_count >= session->max_players) {
        return NetJoin_Full;
    }
    return NetJoin_Accepted;
}

enum NetJoinRejection net_join_rejection;
