/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file replay.c
 *     Replay recording and playback.
 * @par Purpose:
 *     Creating, compressing, and loading replay (.pck) files.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "replay.h"
#include "packets.h"
#include "net_game.h"
#include "bflib_fileio.h"
#include "bflib_datetm.h"
#include "frontend.h"
#include "game_legacy.h"
#include "game_saves.h"
#include "gui_topmsg.h"
#include "config_settings.h"
#include "config_keeperfx.h"
#include "config_campaigns.h"
#include "version.h"
#include "player_utils.h"
#include "slab_data.h"
#include "dungeon_data.h"
#include "tasks_list.h"
#include "spdigger_stack.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define PACKET_TURN_MAX_SIZE (MAX_NET_USERS*sizeof(struct Packet) + sizeof(TbBigChecksum))
unsigned long initial_replay_seed;
extern TbBool IMPRISON_BUTTON_DEFAULT;
extern TbBool FLEE_BUTTON_DEFAULT;
extern TbBool get_skip_heart_zoom_feature(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif

// -- compression/decompression state --
static struct Packet packet_codec_prev[MAX_NET_USERS];
static uint32_t packet_bits_acc;
static int packet_bits_count;
static TbBool packet_bits_pending;
// long-turn runs use the heap instead
static unsigned char packet_turn_small[PACKET_TURN_MAX_SIZE];
static unsigned char *packet_turn_buf = packet_turn_small;
static size_t packet_turn_len;
static size_t packet_turn_cap = PACKET_TURN_MAX_SIZE;
static unsigned char *packet_bits_out;
static size_t packet_bits_out_len;
// decoder position within the current code
static uint32_t packet_dec_zeros;
static unsigned char packet_dec_lit[4];
static int packet_dec_lit_pos;
static int packet_dec_lit_len;
static TbBool packet_dec_reserved;
static TbBool packet_file_ends_in_reserved;

// special RLEs for encoding long sequences of zeros
static const unsigned char packet_zero_runs[8] = {1, 3, 5, 7, 11, 15, 20, 40};
#define PACKET_LONG_ZERO_RUN 256

static int packet_saved_users(NetUserId *users)
{
    int n = 0;
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        if (game.packet_save_head.user_players[user] >= 0)
            users[n++] = user;
    }
    return n;
}

static TbBool packet_file_compressed(void)
{
    return flag_is_set(game.packet_save_head.flags, PSHF_Compressed);
}


static void release_turn_buffer(void)
{
    if (packet_turn_buf != packet_turn_small)
        free(packet_turn_buf);
    packet_turn_buf = packet_turn_small;
    packet_turn_cap = PACKET_TURN_MAX_SIZE;
    packet_turn_len = 0;
}

static void reset_packet_codec(void)
{
    memset(packet_codec_prev, 0, sizeof(packet_codec_prev));
    packet_bits_acc = 0;
    packet_bits_count = 0;
    packet_bits_pending = false;
    release_turn_buffer();
    packet_dec_zeros = 0;
    packet_dec_lit_pos = 0;
    packet_dec_lit_len = 0;
    packet_dec_reserved = false;
}

static TbBool reserve_turn_buffer(size_t needed)
{
    if (needed <= packet_turn_cap)
        return true;
    size_t ncap = max(needed, packet_turn_cap * 2);
    unsigned char *nbuf = malloc(ncap);
    if (nbuf == NULL)
        return false;
    memcpy(nbuf, packet_turn_buf, packet_turn_len);
    if (packet_turn_buf != packet_turn_small)
        free(packet_turn_buf);
    packet_turn_buf = nbuf;
    packet_turn_cap = ncap;
    return true;
}

// the output buffer is sized up front for the worst case (12 bits per byte)
static void put_bits(uint32_t value, int nbits)
{
    for (int i = nbits - 1; i >= 0; i--)
    {
        packet_bits_acc = (packet_bits_acc << 1) | ((value >> i) & 1);
        if (++packet_bits_count == 8)
        {
            packet_bits_out[packet_bits_out_len++] = packet_bits_acc & 0xFF;
            packet_bits_acc = 0;
            packet_bits_count = 0;
        }
    }
}

static TbBool get_bits(uint32_t *value, int nbits)
{
    *value = 0;
    for (int i = 0; i < nbits; i++)
    {
        if (packet_bits_count == 0)
        {
            unsigned char c;
            if (LbFileRead(game.packet_save_fp, &c, 1) != 1)
                return false;
            game.packet_file_pos++;
            packet_bits_acc = c;
            packet_bits_count = 8;
        }
        packet_bits_count--;
        *value = (*value << 1) | ((packet_bits_acc >> packet_bits_count) & 1);
    }
    return true;
}

// encoding scheme:
// '0': 2 zeroes;
// '10xxx': packet_zero_runs[x] zeroes
// '11bb'<bb+1 bytes, not all zero...>: 1~4 bytes verbatim
// '11zz'<zz+1 zero-bytes>: 
//    zz == 0: reserved
//    zz == 1: 256 zeroes
//    zz == 2: reserved
//    zz == 3: reserved
static TbBool encode_packet_bits(const unsigned char *buf, size_t len)
{
    uint32_t zeros_small[PACKET_TURN_MAX_SIZE + 1];
    uint32_t cost_small[PACKET_TURN_MAX_SIZE + 1];
    unsigned char code_small[PACKET_TURN_MAX_SIZE + 1];
    unsigned char arg_small[PACKET_TURN_MAX_SIZE + 1];
    uint32_t *zeros = zeros_small;
    uint32_t *cost = cost_small;
    unsigned char *code = code_small;
    unsigned char *arg = arg_small;
    const TbBool on_heap = (len > PACKET_TURN_MAX_SIZE);
    if (on_heap)
    {
        zeros = malloc((len + 1) * sizeof(uint32_t));
        cost = malloc((len + 1) * sizeof(uint32_t));
        code = malloc(len + 1);
        arg = malloc(len + 1);
        if ((zeros == NULL) || (cost == NULL) || (code == NULL) || (arg == NULL))
        {
            free(zeros); free(cost); free(code); free(arg);
            return false;
        }
    }
    zeros[len] = 0;
    cost[len] = 0;
    for (size_t i = len; i-- > 0; )
    {
        zeros[i] = (buf[i] == 0) ? zeros[i + 1] + 1 : 0;
        cost[i] = UINT32_MAX;
        if ((zeros[i] >= 2) && (1 + cost[i + 2] < cost[i]))
        {
            cost[i] = 1 + cost[i + 2];
            code[i] = 0;
        }
        for (int x = 0; x < 8; x++)
        {
            uint32_t n = packet_zero_runs[x];
            if ((zeros[i] >= n) && (5 + cost[i + n] < cost[i]))
            {
                cost[i] = 5 + cost[i + n];
                code[i] = 1;
                arg[i] = x;
            }
        }
        if ((zeros[i] >= PACKET_LONG_ZERO_RUN) && (20 + cost[i + PACKET_LONG_ZERO_RUN] < cost[i]))
        {
            cost[i] = 20 + cost[i + PACKET_LONG_ZERO_RUN];
            code[i] = 3;
        }
        for (uint32_t n = 1; (n <= 4) && (i + n <= len); n++)
        {
            if (zeros[i] >= n)
                continue;
            if (4 + 8 * n + cost[i + n] < cost[i])
            {
                cost[i] = 4 + 8 * n + cost[i + n];
                code[i] = 2;
                arg[i] = n;
            }
        }
    }
    for (size_t i = 0; i < len; )
    {
        switch (code[i])
        {
        case 0:
            put_bits(0, 1);
            i += 2;
            break;
        case 1:
            put_bits(0x10 | arg[i], 5);
            i += packet_zero_runs[arg[i]];
            break;
        case 2:
            if (zeros[i] >= arg[i])
                assert(false);
            put_bits(0xC | (arg[i] - 1), 4);
            for (int k = 0; k < arg[i]; k++)
                put_bits(buf[i + k], 8);
            i += arg[i];
            break;
        default:
            put_bits(0xD, 4);
            put_bits(0, 16);
            i += PACKET_LONG_ZERO_RUN;
            break;
        }
    }
    if (on_heap)
    {
        free(zeros); free(cost); free(code); free(arg);
    }
    return true;
}

static TbBool decode_packet_byte(unsigned char *out)
{
    if (packet_dec_zeros == 0 && packet_dec_lit_pos >= packet_dec_lit_len)
    {
        uint32_t v;
        if (!get_bits(&v, 1))
            return false;
        if (v == 0)
        {
            packet_dec_zeros = 2;
        } else
        {
            if (!get_bits(&v, 1))
                return false;
            if (v == 0)
            {
                if (!get_bits(&v, 3))
                    return false;
                packet_dec_zeros = packet_zero_runs[v];
            } else
            {
                if (!get_bits(&v, 2))
                    return false;
                int bb = v;
                TbBool all_zero = true;
                for (int k = 0; k <= bb; k++)
                {
                    if (!get_bits(&v, 8))
                        return false;
                    packet_dec_lit[k] = v;
                    all_zero &= (v == 0);
                }
                packet_dec_lit_pos = 0;
                packet_dec_lit_len = bb + 1;
                if (all_zero)
                {
                    if (bb != 1)
                    {
                        packet_dec_reserved = true;
                        return false;
                    }
                    packet_dec_lit_len = 0;
                    packet_dec_zeros = PACKET_LONG_ZERO_RUN;
                }
            }
        }
    }
    if (packet_dec_zeros > 0)
    {
        packet_dec_zeros--;
        *out = 0;
    } else
    {
        *out = packet_dec_lit[packet_dec_lit_pos++];
    }
    return true;
}

static TbBool decode_packet_bytes(void *buf, size_t len)
{
    unsigned char *b = buf;
    for (size_t i = 0; i < len; i++)
    {
        if (!decode_packet_byte(&b[i]))
            return false;
    }
    return true;
}

static TbBool packet_codec_at_code_boundary(void)
{
    return (packet_dec_zeros == 0) && (packet_dec_lit_pos >= packet_dec_lit_len);
}

static TbBool write_packet_bytes(const void *buf, size_t len)
{
    if (!packet_file_compressed())
        return (LbFileWrite(game.packet_save_fp, buf, len) == (long)len);
    if (!reserve_turn_buffer(packet_turn_len + len))
        return false;
    memcpy(&packet_turn_buf[packet_turn_len], buf, len);
    packet_turn_len += len;
    return true;
}

static TbBool read_packet_bytes(void *buf, size_t len)
{
    if (!packet_file_compressed())
        return (LbFileRead(game.packet_save_fp, buf, len) == (int)len);
    return decode_packet_bytes(buf, len);
}

static TbBool skip_packet_bytes(size_t len)
{
    if (!packet_file_compressed())
        return (LbFileSeek(game.packet_save_fp, len, Lb_FILE_SEEK_CURRENT) >= 0);
    unsigned char c;
    for (size_t i = 0; i < len; i++)
    {
        if (!decode_packet_byte(&c))
            return false;
    }
    return true;
}

static void xor_bytes(void *dst, const void *src, size_t len)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    for (size_t i = 0; i < len; i++)
        d[i] ^= s[i];
}

// turn and checksum are shared by all users, so later users' are xor'd against the first's
static void packet_to_residual(struct Packet *res, const struct Packet *raw, const struct Packet *first, int idx)
{
    *res = *raw;
    xor_bytes(res, &packet_codec_prev[idx], sizeof(struct Packet));
    if (idx == 0)
    {
        res->turn = raw->turn - packet_codec_prev[idx].turn - 1;
    } else
    {
        res->turn = raw->turn ^ first->turn;
        res->checksum = raw->checksum ^ first->checksum;
    }
    packet_codec_prev[idx] = *raw;
}

static void packet_from_residual(struct Packet *raw, const struct Packet *res, const struct Packet *first, int idx)
{
    *raw = *res;
    xor_bytes(raw, &packet_codec_prev[idx], sizeof(struct Packet));
    if (idx == 0)
    {
        raw->turn = res->turn + packet_codec_prev[idx].turn + 1;
    } else
    {
        raw->turn = res->turn ^ first->turn;
        raw->checksum = res->checksum ^ first->checksum;
    }
    packet_codec_prev[idx] = *raw;
}

static TbBool write_turn_packets(const unsigned char *pckt_buf, int nusers)
{
    const size_t turn_data_size = nusers * sizeof(struct Packet) + sizeof(TbBigChecksum);
    if (!packet_file_compressed())
        return write_packet_bytes(pckt_buf, turn_data_size);
    unsigned char res_buf[PACKET_TURN_MAX_SIZE];
    const struct Packet *raw = (const struct Packet *)pckt_buf;
    for (int i = 0; i < nusers; i++)
        packet_to_residual((struct Packet *)&res_buf[i * sizeof(struct Packet)], &raw[i], &raw[0], i);
    memcpy(&res_buf[nusers * sizeof(struct Packet)], &pckt_buf[nusers * sizeof(struct Packet)], sizeof(TbBigChecksum));
    release_turn_buffer();
    return write_packet_bytes(res_buf, turn_data_size);
}

static TbBool finish_turn_write(void)
{
    if (!packet_file_compressed())
        return true;
    unsigned char out_small[PACKET_TURN_MAX_SIZE * 3 / 2 + 2];
    const size_t out_cap = packet_turn_len * 3 / 2 + 2;
    packet_bits_out = (out_cap <= sizeof(out_small)) ? out_small : malloc(out_cap);
    if (packet_bits_out == NULL)
    {
        release_turn_buffer();
        return false;
    }
    packet_bits_out_len = 0;
    TbBool ok = encode_packet_bits(packet_turn_buf, packet_turn_len);
    release_turn_buffer();
    // the trailing partial byte is written padded and rewritten by the next turn
    if (packet_bits_pending && (LbFileSeek(game.packet_save_fp, -1, Lb_FILE_SEEK_CURRENT) < 0))
        ok = false;
    packet_bits_pending = (packet_bits_count > 0);
    if (packet_bits_pending)
        packet_bits_out[packet_bits_out_len++] = (packet_bits_acc << (8 - packet_bits_count)) & 0xFF;
    if (ok && (LbFileWrite(game.packet_save_fp, packet_bits_out, packet_bits_out_len) != (long)packet_bits_out_len))
        ok = false;
    if (packet_bits_out != out_small)
        free(packet_bits_out);
    packet_bits_out = NULL;
    return ok;
}

static TbBool read_turn_packets(unsigned char *pckt_buf, int nusers)
{
    const size_t turn_data_size = nusers * sizeof(struct Packet) + sizeof(TbBigChecksum);
    if (!packet_file_compressed())
    {
        if (LbFileRead(game.packet_save_fp, pckt_buf, turn_data_size) != (int)turn_data_size)
            return false;
        game.packet_file_pos += turn_data_size;
        return true;
    }
    unsigned char res_buf[PACKET_TURN_MAX_SIZE];
    if (!decode_packet_bytes(res_buf, turn_data_size))
        return false;
    struct Packet *raw = (struct Packet *)pckt_buf;
    for (int i = 0; i < nusers; i++)
        packet_from_residual(&raw[i], (const struct Packet *)&res_buf[i * sizeof(struct Packet)], &raw[0], i);
    memcpy(&pckt_buf[nusers * sizeof(struct Packet)], &res_buf[nusers * sizeof(struct Packet)], sizeof(TbBigChecksum));
    return true;
}

// If the turn-end checksum is LONG_TURN_MARKER, additional data follows, for what can't
// be expressed just in regular packets. (The real checksum comes right after the long turn marker.)
#define LONG_TURN_MARKER ((TbBigChecksum)0x80000000)
enum LongTurnReplayRecordKind {
    LTK_End = 0,
    LTK_ChatMessage = 1, // payload: uint16_t user, char message[PLAYER_MP_MESSAGE_LEN]
    // TODO: resyncs / state transfers?
};

static TbBool chat_messages_recorded(void)
{
    // Chat messages are sent out-of-band, so they break the requirements
    // for determinism if they don't arrive the same turn they are sent.
    // (Also, it's nice to preserve privacy too, in case any couples send a bug report...)
    return !network_is_active() && (game.input_lag_turns == 0);
}

static TbBool write_long_turn_record(unsigned char kind, uint32_t len, const void *payload)
{
    return write_packet_bytes(&kind, sizeof(kind))
        && write_packet_bytes(&len, sizeof(len))
        && write_packet_bytes(payload, len);
}

static TbBool read_long_turn_data(TbBigChecksum *chksum, TbBool apply, int32_t *consumed)
{
    if (!read_packet_bytes(chksum, sizeof(*chksum)))
        return false;
    *consumed += sizeof(*chksum);
    while (true)
    {
        unsigned char kind;
        if (!read_packet_bytes(&kind, sizeof(kind)))
            return false;
        *consumed += sizeof(kind);
        if (kind == LTK_End)
            return true;
        uint32_t len;
        if (!read_packet_bytes(&len, sizeof(len)))
            return false;
        *consumed += sizeof(len);
        uint32_t used = 0;
        if (apply && (kind == LTK_ChatMessage) && (len >= sizeof(uint16_t)))
        {
            uint16_t user;
            if (!read_packet_bytes(&user, sizeof(user)))
                return false;
            used += sizeof(user);
            const uint32_t msg_len = min(len - used, (uint32_t)PLAYER_MP_MESSAGE_LEN);
            char message[PLAYER_MP_MESSAGE_LEN] = {0};
            if (!read_packet_bytes(message, msg_len))
                return false;
            used += msg_len;
            message[PLAYER_MP_MESSAGE_LEN - 1] = '\0';
            PlayerNumber plyr_idx = (user < MAX_NET_USERS) ? get_net_user_player_number(user) : -1;
            if (plyr_idx >= 0)
                memcpy(get_player(plyr_idx)->mp_pending_message, message, PLAYER_MP_MESSAGE_LEN);
            else
                WARNLOG("Chat message for invalid user %u in Packet File", (unsigned)user);
        }
        if ((len > used) && !skip_packet_bytes(len - used))
            return false;
        *consumed += len;
    }
}

// reads one whole turn from replay data
static TbBool read_turn(unsigned char *pckt_buf, int nusers, TbBool apply, TbBigChecksum *chksum)
{
    if (!read_turn_packets(pckt_buf, nusers))
        return false;
    *chksum = llong(&pckt_buf[nusers * sizeof(struct Packet)]);
    if (*chksum == LONG_TURN_MARKER)
    {
        int32_t consumed = 0;
        if (!read_long_turn_data(chksum, apply, &consumed))
            return false;
        if (!packet_file_compressed())
            game.packet_file_pos += consumed;
    }
    return !packet_file_compressed() || packet_codec_at_code_boundary();
}

static GameTurn count_stored_turns(void)
{
    NetUserId users[MAX_NET_USERS];
    const int nusers = packet_saved_users(users);
    unsigned char pckt_buf[PACKET_TURN_MAX_SIZE+4];
    const unsigned int start_pos = game.packet_file_pos;
    GameTurn turns = 0;
    TbBigChecksum chksum;
    LbFileSeek(game.packet_save_fp, start_pos, Lb_FILE_SEEK_BEGINNING);
    while (read_turn(pckt_buf, nusers, false, &chksum))
        turns++;
    packet_file_ends_in_reserved = packet_dec_reserved;
    LbFileSeek(game.packet_save_fp, start_pos, Lb_FILE_SEEK_BEGINNING);
    game.packet_file_pos = start_pos;
    reset_packet_codec();
    return turns;
}

TbBool open_packet_file_for_load(char *fname, struct CatalogueEntry *centry)
{
    memset(centry, 0, sizeof(struct CatalogueEntry));
    strcpy(game.packet_fname, fname);
    game.packet_save_fp = LbFileOpen(game.packet_fname, Lb_FILE_MODE_READ_ONLY);
    if (!game.packet_save_fp)
    {
        ERRORLOG("Cannot open keeper packet file for load");
        game.packet_fopened = 0;
        return false;
    }
    int i = load_game_chunks(game.packet_save_fp, centry);
    if ((i != GLoad_PacketStart) && (i != GLoad_PacketContinue))
    {
        LbFileClose(game.packet_save_fp);
        game.packet_save_fp = NULL;
        game.packet_fopened = 0;
        WARNMSG("Couldn't correctly read packet file \"%s\" header.",fname);
        return false;
    }
    game.packet_file_pos = LbFilePosition(game.packet_save_fp);
    reset_packet_codec();
    game.turns_stored = count_stored_turns();
    if ((game.packet_checksum_verify) && !flag_is_set(game.packet_save_head.flags, PSHF_Checksum))
    {
        WARNMSG("PacketSave checksum not available, checking disabled.");
        game.packet_checksum_verify = false;
    }
    if (game.log_things_start_turn == -1)
    {
        game.log_things_start_turn = 0;
        game.log_things_end_turn = game.turns_stored + 1;
    }
    game.packet_fopened = 1;
    return true;
}

void restore_users_from_packet_save(void)
{
    TbBool local_mapped = false;
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        set_net_user_player_number(user, -1);
    }
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        PlayerNumber plyr_idx = game.packet_save_head.user_players[user];
        if (plyr_idx < 0)
            continue;
        if ((plyr_idx >= PLAYERS_COUNT)
         || !flag_is_set(game.packet_save_head.players_exist, to_flag(plyr_idx)))
        {
            WARNLOG("Packet file maps user %d to player %d, which the file says does not exist",
                (int)user, (int)plyr_idx);
            continue;
        }
        set_net_user_player_number(user, plyr_idx);
        struct PlayerInfo *player = get_player(plyr_idx);
        player->user_id = user;
        snprintf(player->player_name, sizeof(player->player_name), "%s",
            game.packet_save_head.user_names[user]);
        init_user_state(user);
        local_mapped |= (plyr_idx == my_player_number);
        SYNCLOG("Replay user %d -> player %d", (int)user, (int)plyr_idx);
    }
    if (!local_mapped)
    {
        set_net_user_player_number(SOLO_HUMAN_ID, my_player_number);
        get_player(my_player_number)->user_id = SOLO_HUMAN_ID;
        init_user_state(SOLO_HUMAN_ID);
        SYNCLOG("Replay local user %d -> player %d (not in the recorded map)",
            (int)SOLO_HUMAN_ID, (int)my_player_number);
    }
}

/**
 * Computes verification checksum for -packetsave/-packetload replay files.
 * NOT used for multiplayer - only for single-player replay integrity checking.
 * Sums things, terrain, and each player's pending map tasks.
 *
 * @return Checksum value for detecting replay file corruption
 */
TbBigChecksum compute_replay_integrity(void)
{
    TbBigChecksum sum = 0;
    for (long tng_idx = 0; tng_idx < THINGS_COUNT; tng_idx++)
    {
        struct Thing* tng = thing_get(tng_idx);
        if ((tng->alloc_flags & TAlF_Exists) != 0)
        {
            // It would be nice to completely ignore effects, but since
            // thing indices are used in packets, lack of effect may cause desync too.
            if (!is_non_synchronized_thing_class(tng->class_id))
            {
                sum += (ulong)tng->mappos.x.val + (ulong)tng->mappos.y.val + (ulong)tng->mappos.z.val
                     + (ulong)tng->move_angle_xy + (ulong)tng->owner;
            }
        }
    }
    for (MapSlabCoord slb_y = 0; slb_y < game.map_tiles_y; slb_y++)
    {
        for (MapSlabCoord slb_x = 0; slb_x < game.map_tiles_x; slb_x++)
        {
            const struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
            sum += (ulong)slb->kind + (ulong)slb->owner + (ulong)slb->health;
        }
    }
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        const struct PlayerInfo* player = get_player(plyr_idx);
        if (!player_exists(player))
            continue;
        const struct Dungeon* dungeon = get_players_dungeon(player);
        if (dungeon_invalid(dungeon))
            continue;
        for (size_t i = 0; i < MAPTASKS_COUNT; i++)
        {
            const struct MapTask* task = &dungeon->task_list[i];
            if (task->kind != SDDigTask_None)
                sum += (ulong)task->kind + (ulong)task->coords;
        }
    }
    return sum;
}

short save_packets(void)
{
    NetUserId users[MAX_NET_USERS];
    const int nusers = packet_saved_users(users);
    unsigned char pckt_buf[PACKET_TURN_MAX_SIZE+4];
    TbBigChecksum chksum;
    SYNCDBG(6,"Starting");
    if (game.packet_checksum_verify)
        chksum = compute_replay_integrity();
    else
        chksum = 0;
    LbFileSeek(game.packet_save_fp, 0, Lb_FILE_SEEK_END);
    // Prepare data in the buffer
    for (int i = 0; i < nusers; i++)
        memcpy(&pckt_buf[i*sizeof(struct Packet)], &game.packets[users[i]], sizeof(struct Packet));
    TbBool has_chat = false;
    if (chat_messages_recorded()) {
        for (int i = 0; i < nusers; i++)
            has_chat |= (game.packets[users[i]].action == PckA_PlyrMsgEnd);
    }
    const TbBool long_turn = has_chat || (chksum == LONG_TURN_MARKER);
    const TbBigChecksum chksum_slot = long_turn ? LONG_TURN_MARKER : chksum;
    memcpy(&pckt_buf[nusers*sizeof(struct Packet)], &chksum_slot, sizeof(TbBigChecksum));
    // Write buffer into file
    TbBool ok = write_turn_packets(pckt_buf, nusers);
    if (long_turn)
    {
        ok &= write_packet_bytes(&chksum, sizeof(chksum));
        // write a null-terminated sequence of additional records
        for (int i = 0; has_chat && (i < nusers); i++) {
            if (game.packets[users[i]].action != PckA_PlyrMsgEnd)
                continue;
            unsigned char payload[sizeof(uint16_t) + PLAYER_MP_MESSAGE_LEN];
            const uint16_t user = users[i];
            memcpy(payload, &user, sizeof(user));
            memcpy(payload + sizeof(user), get_player(get_net_user_player_number(users[i]))->mp_pending_message, PLAYER_MP_MESSAGE_LEN);
            ok &= write_long_turn_record(LTK_ChatMessage, sizeof(payload), payload);
        }
        const unsigned char end = LTK_End;
        ok &= write_packet_bytes(&end, sizeof(end));
    }
    ok &= finish_turn_write();
    if (!ok)
        ERRORLOG("Packet file write error");
    if ( !LbFileFlush(game.packet_save_fp) )
    {
        ERRORLOG("Unable to flush PacketSave File");
        return false;
    }
    if (packetsave_max_kb > 0)
    {
        int pos = LbFilePosition(game.packet_save_fp);
        if ((pos >= 0) && ((uint32_t)pos >= packetsave_max_kb * 1024))
        {
            WARNLOG("PacketSave reached the %u KB limit at turn %u; recording stopped",
                packetsave_max_kb, get_gameturn());
            close_packet_file();
            game.packet_save_enable = false;
        }
    }
    return true;
}

void close_packet_file(void)
{
    if ( game.packet_fopened )
    {
        LbFileClose(game.packet_save_fp);
        game.packet_fopened = 0;
        game.packet_save_fp = NULL;
    }
}

TbBool reinit_packets_after_load(void)
{
    game.packet_save_enable = false;
    game.packet_load_enable = false;
    game.packet_save_fp = NULL;
    game.packet_fopened = 0;
    return true;
}

static const char replay_type_chars[ReplTyp_Count] = {'c', 'f', 'm'};

static int compare_replay_names(const void *a, const void *b)
{
    return strcmp(*(char *const *)a, *(char *const *)b);
}

// names start with a 15-char timestamp, then '_' and the map type char [cfm]
#define REPLAY_TYPE_CHAR_POS 16

static void evict_old_replays(char type_chr, uint32_t keep)
{
    char **names = NULL;
    size_t count = 0;
    size_t cap = 0;
    struct TbFileEntry fe;
    struct TbFileFind *ff = LbFileFindFirst("replays/*.pck", &fe);
    if (ff != NULL)
    {
        do {
            if ((strlen(fe.Filename) <= REPLAY_TYPE_CHAR_POS) || (fe.Filename[REPLAY_TYPE_CHAR_POS - 1] != '_')
                || (fe.Filename[REPLAY_TYPE_CHAR_POS] != type_chr))
                continue;
            if (count == cap)
            {
                cap = cap ? cap * 2 : 16;
                char **grown = realloc(names, cap * sizeof(char *));
                if (grown == NULL)
                    break;
                names = grown;
            }
            names[count] = strdup(fe.Filename);
            if (names[count] != NULL)
                count++;
        } while (LbFileFindNext(ff, &fe) >= 0);
        LbFileFindEnd(ff);
    }
    if (count > 0)
        qsort(names, count, sizeof(char *), compare_replay_names);
    for (size_t i = 0; i + keep < count; i++)
    {
        char fname[DISKPATH_SIZE];
        snprintf(fname, sizeof(fname), "replays/%s", names[i]);
        LbFileDelete(fname);
    }
    for (size_t i = 0; i < count; i++)
        free(names[i]);
    free(names);
}

static void append_git_sha(char *buf, size_t buflen)
{
    const char *sha = GIT_REVISION;
    const char *g = strstr(sha, "-g");
    while (g != NULL)
    {
        sha = g + 2;
        g = strstr(sha, "-g");
    }
    size_t n = strspn(sha, "0123456789abcdef");
    if ((n < 6) || (sha[n] != '\0'))
        return;
    size_t len = strlen(buf);
    if (len + 7 >= buflen)
        return;
    buf[len++] = '_';
    for (int i = 0; i < 6; i++)
        buf[len++] = toupper((unsigned char)sha[i]);
    buf[len] = '\0';
}

TbBool setup_auto_replay_save(void)
{
    LevelNumber lvnum = get_loaded_level_number();
    int type;
    if (is_multiplayer_level(lvnum))
        type = ReplTyp_Multiplayer;
    else if (is_freeplay_level(lvnum))
        type = ReplTyp_Freeplay;
    else
        type = ReplTyp_Campaign;
    if (max_replays[type] == 0)
        return false;
    evict_old_replays(replay_type_chars[type], max_replays[type] - 1);

    int humans = 0;
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0))
            humans++;
    }
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    if (lt == NULL)
        return false;
    int year = ((lt->tm_year + 1900) % 10000 + 10000) % 10000;
    char fname[sizeof(game.packet_fname)];
    snprintf(fname, sizeof(fname), "replays/%04d%02d%02dT%02d%02d%02d_%c%d",
        year, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec,
        replay_type_chars[type], humans);
    size_t len = strlen(fname);
    if (humans > 1)
    {
        snprintf(fname + len, sizeof(fname) - len, "u%d", (int)get_local_user() + 1);
        len = strlen(fname);
    }
    if (campaign.fname[0] != '\0')
    {
        char cmpgn_id[64];
        get_campaign_sanitized_id(campaign.fname, cmpgn_id, sizeof(cmpgn_id));
        snprintf(fname + len, sizeof(fname) - len, "_%s", cmpgn_id);
        len = strlen(fname);
    }
    snprintf(fname + len, sizeof(fname) - len, "_map%05d_v%d%d%d", (int)lvnum, VER_MAJOR, VER_MINOR, VER_RELEASE);
    len = strlen(fname);
    if (VER_BUILD != 0)
    {
        snprintf(fname + len, sizeof(fname) - len, "b%d", VER_BUILD);
        len = strlen(fname);
    }
    append_git_sha(fname, sizeof(fname));
    len = strlen(fname);
    snprintf(fname + len, sizeof(fname) - len, ".pck");
    snprintf(game.packet_fname, sizeof(game.packet_fname), "%s", fname);
    game.packet_save_enable = true;
    return true;
}

TbBool open_new_packet_file_for_save(void)
{
    // Filling the header
    SYNCMSG("Starting packet saving, turn %lu",(unsigned long)get_gameturn());
    game.packet_save_head.game_ver_major = VER_MAJOR;
    game.packet_save_head.game_ver_minor = VER_MINOR;
    game.packet_save_head.game_ver_release = VER_RELEASE;
    game.packet_save_head.game_ver_build = VER_BUILD;
    game.packet_save_head.level_num = get_loaded_level_number();
    game.packet_save_head.players_exist = 0;
    game.packet_save_head.players_comp = 0;
    reset_packet_codec();
    game.packet_save_head.flags = PSHF_Compressed;
    if (game.packet_checksum_verify)
        set_flag(game.packet_save_head.flags, PSHF_Checksum);
    game.packet_save_head.isometric_view_zoom_level = settings.isometric_view_zoom_level;
    game.packet_save_head.frontview_zoom_level = settings.frontview_zoom_level;
    game.packet_save_head.isometric_tilt = settings.isometric_tilt;
    game.packet_save_head.video_rotate_mode = settings.video_rotate_mode;
    game.packet_save_head.action_seed = initial_replay_seed;
    game.packet_save_head.skip_heart_zoom = get_skip_heart_zoom_feature();
    game.packet_save_head.default_imprison_tendency = IMPRISON_BUTTON_DEFAULT;
    game.packet_save_head.default_flee_tendency = FLEE_BUTTON_DEFAULT;
    game.packet_save_head.highlight_mode = settings.highlight_mode;
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
        game.packet_save_head.user_players[user] = get_net_user_player_number(user);
    game.packet_save_head.recording_user = get_local_user();
    game.packet_save_head.frontend_alliances = frontend_alliances;
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        const char *name = network_user_name(user);
        snprintf(game.packet_save_head.user_names[user],
            sizeof(game.packet_save_head.user_names[user]), "%s", (name != NULL) ? name : "");
    }
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player))
        {
            set_flag(game.packet_save_head.players_exist, to_flag(i));
            if ((player->allocflags & PlaF_CompCtrl) != 0)
              set_flag(game.packet_save_head.players_comp, to_flag(i));
        }
    }
    LbFileDelete(game.packet_fname);
    game.packet_save_fp = LbFileOpen(game.packet_fname, Lb_FILE_MODE_NEW);
    if (!game.packet_save_fp)
    {
        ERRORLOG("Cannot open keeper packet file for save, \"%s\".",game.packet_fname);
        game.packet_fopened = 0;
        return false;
    }
    struct CatalogueEntry centry;
    fill_game_catalogue_entry(&centry, "Packet file");
    if (!save_packet_chunks(game.packet_save_fp,&centry))
    {
        WARNMSG("Cannot write to packet file, \"%s\".",game.packet_fname);
        LbFileClose(game.packet_save_fp);
        game.packet_fopened = 0;
        game.packet_save_fp = NULL;
        return false;
    }
    game.packet_fopened = 1;
    return true;
}

static TbBool turn_has_quit_packet(void)
{
    for (NetUserId i = 0; i < MAX_NET_USERS; i++)
    {
        switch (game.packets[i].action)
        {
        case PckA_QuitToMainMenu:
        case PckA_ForceApplicationClose:
            return true;
        default:
            break;
        }
    }
    return false;
}

void load_packets_for_turn(GameTurn nturn)
{
    SYNCDBG(19,"Starting");
    NetUserId users[MAX_NET_USERS];
    const int nusers = packet_saved_users(users);
    unsigned char pckt_buf[PACKET_TURN_MAX_SIZE+4];
    if ((nturn >= game.turns_stored) && packet_file_ends_in_reserved)
    {
        ERRORLOG("Packet File uses an unrecognized encoding at turn %u; stopping replay", (unsigned)nturn);
        packet_file_ends_in_reserved = false;
        disable_packet_mode();
        return;
    }
    if (nturn >= game.turns_stored)
    {
        ERRORDBG(18,"Out of turns to load from Packet File");
        erstat_inc(ESE_CantReadPackets);
        return;
    }

    if (!read_turn_packets(pckt_buf, nusers))
    {
        ERRORDBG(18,"Cannot read turn data from Packet File");
        erstat_inc(ESE_CantReadPackets);
        return;
    }
    for (int i = 0; i < nusers; i++)
        memcpy(&game.packets[users[i]], &pckt_buf[i * sizeof(struct Packet)], sizeof(struct Packet));
    for (int i = 0; i < nusers; i++) {
        if (game.packets[users[i]].action == PckA_PlyrMsgEnd)
            memset(get_player(get_net_user_player_number(users[i]))->mp_pending_message, 0, PLAYER_MP_MESSAGE_LEN);
    }
    TbBigChecksum tot_chksum = llong(&pckt_buf[nusers * sizeof(struct Packet)]);
    if (tot_chksum == LONG_TURN_MARKER)
    {
        int32_t consumed = 0;
        if (!read_long_turn_data(&tot_chksum, true, &consumed)) {
            ERRORDBG(18,"Cannot read long turn data from Packet File");
        }
        if (!packet_file_compressed())
            game.packet_file_pos += consumed;
    }
    if (packet_file_compressed() && !packet_codec_at_code_boundary()) {
        ERRORDBG(18,"Packet File turn does not end on a code boundary");
    }
    if (game.turns_fastforward > 0)
        game.turns_fastforward--;
    if (game.packet_checksum_verify && !turn_has_quit_packet())
    {
        if (compute_replay_integrity() != tot_chksum)
        {
            ERRORLOG("PacketSave checksum - Out of sync (GameTurn %u)", get_gameturn());
            if (!is_onscreen_msg_visible())
                show_onscreen_msg(turns_per_second, "Out of sync");
        }
    }
}

void disable_packet_mode(void)
{
    close_packet_file();
    game.packet_load_enable = false;
    game.packet_save_enable = false;
    remap_local_user_to_solo();
    show_onscreen_msg(2*turns_per_second, "Packet mode disabled");
    set_gui_visible(true);
}
