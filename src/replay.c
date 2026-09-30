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
#include "net_exchange_gameplay.h"
#include "bflib_fileio.h"
#include "bflib_datetm.h"
#include "frontend.h"
#include "game_legacy.h"
#include "game_saves.h"
#include "gui_topmsg.h"
#include "config_settings.h"
#include "config_keeperfx.h"
#include "config.h"
#include "config_campaigns.h"
#include "version.h"
#include "player_utils.h"
#include "slab_data.h"
#include "dungeon_data.h"
#include "tasks_list.h"
#include "spdigger_stack.h"
#include "keeperfx.hpp"
#include "net_resync.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define PACKET_TURN_MAX_SIZE (MAX_NET_USERS*sizeof(struct Packet) + sizeof(TbBigChecksum))
struct ReplayState replay;
unsigned long initial_replay_seed;
static TbBool replay_playback_paused;
static char *pending_resync;
static size_t pending_resync_len;
static TbBigChecksum replay_expected_checksum;
static TbBool replay_checksum_pending;
extern TbBool IMPRISON_BUTTON_DEFAULT;
extern TbBool FLEE_BUTTON_DEFAULT;
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
static uint32_t packet_dec_raw_left;
static TbBool packet_dec_reserved;
static TbBool packet_file_ends_in_reserved;

// special RLEs for encoding long sequences of zeros
static const unsigned char packet_zero_runs[8] = {1, 3, 5, 7, 11, 15, 20, 40};

static int packet_saved_users(NetUserId *users)
{
    int n = 0;
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        if (replay.head.user_players[user] >= 0)
            users[n++] = user;
    }
    return n;
}

static TbBool packet_file_compressed(void)
{
    return flag_is_set(replay.head.flags, PSHF_Compressed);
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
    packet_dec_raw_left = 0;
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
            if (LbFileRead(replay.fp, &c, 1) != 1)
                return false;
            replay.file_pos++;
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
// '11bb'<bb+1 bytes, not all zero>: 1~4 bytes verbatim
// '1110'<3 zero-bytes>: pad to a byte boundary, then uint32 len, then len uncompressed bytes
// '11bb'<bb+1 zero-bytes> otherwise: reserved
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
        default:
            assert(zeros[i] < arg[i]);
            put_bits(0xC | (arg[i] - 1), 4);
            for (int k = 0; k < arg[i]; k++)
                put_bits(buf[i + k], 8);
            i += arg[i];
            break;
        }
    }
    if (on_heap)
    {
        free(zeros); free(cost); free(code); free(arg);
    }
    return true;
}

#define PACKET_RAW_BLOCK_BB 2

static TbBool decode_packet_byte(unsigned char *out)
{
    while ((packet_dec_raw_left == 0) && (packet_dec_zeros == 0) && (packet_dec_lit_pos >= packet_dec_lit_len))
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
                if (all_zero)
                {
                    if (bb != PACKET_RAW_BLOCK_BB)
                    {
                        packet_dec_reserved = true;
                        return false;
                    }
                    packet_bits_count = 0;
                    uint32_t raw_len;
                    if (LbFileRead(replay.fp, &raw_len, sizeof(raw_len)) != sizeof(raw_len))
                        return false;
                    replay.file_pos += sizeof(raw_len);
                    packet_dec_raw_left = raw_len;
                    continue;
                }
                packet_dec_lit_pos = 0;
                packet_dec_lit_len = bb + 1;
            }
        }
    }
    if (packet_dec_raw_left > 0)
    {
        if (LbFileRead(replay.fp, out, 1) != 1)
            return false;
        replay.file_pos++;
        packet_dec_raw_left--;
        return true;
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
    size_t i = 0;
    while (i < len)
    {
        if (packet_dec_raw_left > 0)
        {
            const uint32_t n = min((uint32_t)(len - i), packet_dec_raw_left);
            if (LbFileRead(replay.fp, &b[i], n) != (int)n)
                return false;
            replay.file_pos += n;
            packet_dec_raw_left -= n;
            i += n;
            continue;
        }
        if (!decode_packet_byte(&b[i]))
            return false;
        i++;
    }
    return true;
}

static TbBool packet_codec_at_code_boundary(void)
{
    return (packet_dec_zeros == 0) && (packet_dec_lit_pos >= packet_dec_lit_len) && (packet_dec_raw_left == 0);
}

static TbBool skip_packet_bytes(size_t len)
{
    if (!packet_file_compressed())
        return (LbFileSeek(replay.fp, len, Lb_FILE_SEEK_CURRENT) >= 0);
    while (len > 0)
    {
        if (packet_dec_raw_left > 0)
        {
            const uint32_t n = min((uint32_t)len, packet_dec_raw_left);
            if (LbFileSeek(replay.fp, n, Lb_FILE_SEEK_CURRENT) < 0)
                return false;
            replay.file_pos += n;
            packet_dec_raw_left -= n;
            len -= n;
            continue;
        }
        unsigned char c;
        if (!decode_packet_byte(&c))
            return false;
        len--;
    }
    return true;
}

struct PacketDecoderState {
    int32_t file_pos;
    uint32_t bits_acc;
    int bits_count;
    uint32_t dec_zeros;
    unsigned char dec_lit[4];
    int dec_lit_pos;
    int dec_lit_len;
    uint32_t dec_raw_left;
};

static void save_packet_decoder_state(struct PacketDecoderState *st)
{
    st->file_pos = LbFilePosition(replay.fp);
    st->bits_acc = packet_bits_acc;
    st->bits_count = packet_bits_count;
    st->dec_zeros = packet_dec_zeros;
    memcpy(st->dec_lit, packet_dec_lit, sizeof(st->dec_lit));
    st->dec_lit_pos = packet_dec_lit_pos;
    st->dec_lit_len = packet_dec_lit_len;
    st->dec_raw_left = packet_dec_raw_left;
}

static TbBool restore_packet_decoder_state(const struct PacketDecoderState *st)
{
    packet_bits_acc = st->bits_acc;
    packet_bits_count = st->bits_count;
    packet_dec_zeros = st->dec_zeros;
    memcpy(packet_dec_lit, st->dec_lit, sizeof(packet_dec_lit));
    packet_dec_lit_pos = st->dec_lit_pos;
    packet_dec_lit_len = st->dec_lit_len;
    packet_dec_raw_left = st->dec_raw_left;
    return (LbFileSeek(replay.fp, st->file_pos, Lb_FILE_SEEK_BEGINNING) >= 0);
}

static TbBool write_packet_bytes(const void *buf, size_t len)
{
    if (!packet_file_compressed())
        return (LbFileWrite(replay.fp, buf, len) == (long)len);
    if (!reserve_turn_buffer(packet_turn_len + len))
        return false;
    memcpy(&packet_turn_buf[packet_turn_len], buf, len);
    packet_turn_len += len;
    return true;
}

static TbBool read_packet_bytes(void *buf, size_t len)
{
    if (!packet_file_compressed())
        return (LbFileRead(replay.fp, buf, len) == (int)len);
    return decode_packet_bytes(buf, len);
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
    const size_t turn_data_size = nusers * sizeof(struct Packet);
    if (!packet_file_compressed())
        return write_packet_bytes(pckt_buf, turn_data_size);
    unsigned char res_buf[PACKET_TURN_MAX_SIZE];
    const struct Packet *raw = (const struct Packet *)pckt_buf;
    for (int i = 0; i < nusers; i++)
        packet_to_residual((struct Packet *)&res_buf[i * sizeof(struct Packet)], &raw[i], &raw[0], i);
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
    if (packet_bits_pending && (LbFileSeek(replay.fp, -1, Lb_FILE_SEEK_CURRENT) < 0))
        ok = false;
    packet_bits_pending = (packet_bits_count > 0);
    if (packet_bits_pending)
        packet_bits_out[packet_bits_out_len++] = (packet_bits_acc << (8 - packet_bits_count)) & 0xFF;
    if (ok && (LbFileWrite(replay.fp, packet_bits_out, packet_bits_out_len) != (long)packet_bits_out_len))
        ok = false;
    if (packet_bits_out != out_small)
        free(packet_bits_out);
    packet_bits_out = NULL;
    return ok;
}

static TbBool read_turn_packets(unsigned char *pckt_buf, int nusers)
{
    const size_t turn_data_size = nusers * sizeof(struct Packet);
    if (!packet_file_compressed())
    {
        if (LbFileRead(replay.fp, pckt_buf, turn_data_size) != (int)turn_data_size)
            return false;
        replay.file_pos += turn_data_size;
        return true;
    }
    unsigned char res_buf[PACKET_TURN_MAX_SIZE];
    if (!decode_packet_bytes(res_buf, turn_data_size))
        return false;
    struct Packet *raw = (struct Packet *)pckt_buf;
    for (int i = 0; i < nusers; i++)
        packet_from_residual(&raw[i], (const struct Packet *)&res_buf[i * sizeof(struct Packet)], &raw[0], i);
    return true;
}

// Each turn starts with its checksum, then the packets. A checksum of LONG_TURN_MARKER means
// records follow first, for what can't be expressed just in regular packets, terminated by LTK_End;
// then come the real checksum and the packets.
#define LONG_TURN_MARKER ((TbBigChecksum)0x80000000)
enum LongTurnReplayRecordKind {
    LTK_End = 0,
    LTK_ChatMessage = 1, // payload: uint16_t user, message text
    LTK_ChatCommand = 2, // payload: uint16_t user, int32_t cursor_x, int32_t cursor_y, command text
    LTK_PausedAction = 3, // payload: uint16_t user, uint8_t action, int32_t par1, int32_t par2, int16_t par3, int16_t par4
    LTK_Resync = 4, // payload: the resync message as sent by the host (its header, then zlib-compressed data)
    LTK_NetworkStopped = 5, // no payload: lost the host, continued locally
    // TODO: resyncs / state transfers?
    // Could also carry cheat menu buttons, API actions, etc.
};

static TbBool chat_message_recorded(const char *message)
{
    // for privacy, on multiplayer, only record messages if they are commands
    if (message[0] == cmd_char)
        return true;
    return !network_is_active() && (game.input_lag_turns == 0);
}

static TbBool replay_long_turn_open;

static TbBool write_raw_bytes(const void *buf, size_t len)
{
    return (LbFileWrite(replay.fp, buf, len) == (long)len);
}

static TbBool begin_raw_block(uint32_t len)
{
    if (!packet_file_compressed())
        return true;
    if (!finish_turn_write())
        return false;
    unsigned char out[8];
    packet_bits_out = out;
    packet_bits_out_len = 0;
    put_bits(0xC | PACKET_RAW_BLOCK_BB, 4);
    put_bits(0, 24);
    if (packet_bits_count > 0)
        packet_bits_out[packet_bits_out_len++] = (packet_bits_acc << (8 - packet_bits_count)) & 0xFF;
    packet_bits_out = NULL;
    TbBool ok = !(packet_bits_pending && (LbFileSeek(replay.fp, -1, Lb_FILE_SEEK_CURRENT) < 0));
    ok = ok && write_raw_bytes(out, packet_bits_out_len);
    packet_bits_pending = false;
    packet_bits_acc = 0;
    packet_bits_count = 0;
    return ok && write_raw_bytes(&len, sizeof(len));
}

static TbBool write_long_turn_record(unsigned char kind, uint32_t len, const void *payload)
{
    return begin_raw_block(sizeof(kind) + sizeof(len) + len)
        && write_raw_bytes(&kind, sizeof(kind))
        && write_raw_bytes(&len, sizeof(len))
        && write_raw_bytes(payload, len);
}

static TbBool replay_write_fits(size_t raw_len)
{
    if (packetsave_max_kb == 0)
        return true;
    const int pos = LbFilePosition(replay.fp);
    if (pos < 0)
        return true;
    const size_t bound = packet_file_compressed() ? (packet_turn_len + raw_len) * 3 / 2 + 2 : raw_len;
    if ((uint32_t)pos + bound <= packetsave_max_kb * 1024)
        return true;
    WARNLOG("Replay reached the %u KB limit at turn %u; recording stopped", packetsave_max_kb, (unsigned)get_gameturn());
    close_packet_file();
    replay.save_enable = false;
    return false;
}

static TbBool begin_long_turn(void)
{
    if (replay_long_turn_open)
        return true;
    const TbBigChecksum marker = LONG_TURN_MARKER;
    replay_long_turn_open = write_packet_bytes(&marker, sizeof(marker));
    return replay_long_turn_open;
}

#define LONG_TURN_RECORD_MAX_LEN (1 << 28)

static TbBool read_long_turn_records(TbBool apply)
{
    while (true)
    {
        unsigned char kind;
        if (!read_packet_bytes(&kind, sizeof(kind)))
            return false;
        if (kind == LTK_End)
            return true;
        uint32_t len;
        if (!read_packet_bytes(&len, sizeof(len)))
            return false;
        const int32_t remaining = LbFileLengthHandle(replay.fp) - LbFilePosition(replay.fp);
        if ((len > LONG_TURN_RECORD_MAX_LEN) || (!packet_file_compressed() && ((remaining < 0) || (len > (uint32_t)remaining))))
        {
            ERRORLOG("Long turn record kind %u length %u is invalid (%d bytes left)", (unsigned)kind, (unsigned)len, (int)remaining);
            return false;
        }
        uint32_t used = 0;
        const TbBool is_command = (kind == LTK_ChatCommand);
        if (apply && ((kind == LTK_ChatMessage) || is_command) && (len >= sizeof(uint16_t) + (is_command ? 2 * sizeof(int32_t) : 0)))
        {
            uint16_t user;
            if (!read_packet_bytes(&user, sizeof(user)))
                return false;
            used += sizeof(user);
            int32_t pos[2] = {-1, -1};
            if (is_command)
            {
                if (!read_packet_bytes(pos, sizeof(pos)))
                    return false;
                used += sizeof(pos);
            }
            char message[PLAYER_MP_MESSAGE_LEN] = {0};
            const uint32_t msg_len = min(len - used, (uint32_t)PLAYER_MP_MESSAGE_LEN - 1);
            if (!read_packet_bytes(message, msg_len))
                return false;
            used += msg_len;
            PlayerNumber plyr_idx = (user < MAX_NET_USERS) ? get_net_user_player_number(user) : -1;
            if (plyr_idx >= 0)
                process_gameplay_chat_message(user, message, pos[0], pos[1]);
            else
                WARNLOG("Chat message for invalid user %u in Packet File", (unsigned)user);
        } else
        if (apply && (kind == LTK_Resync))
        {
            char *message = (char *)malloc(len > 0 ? len : 1);
            if (message == NULL)
                return false;
            if (!read_packet_bytes(message, len))
            {
                free(message);
                return false;
            }
            used += len;
            if (pending_resync != NULL)
            {
                WARNLOG("Replay turn has more than one resync; using the last");
                free(pending_resync);
            }
            pending_resync = message;
            pending_resync_len = len;
        } else
        if (apply && (kind == LTK_PausedAction) && (len >= sizeof(uint16_t) + sizeof(uint8_t) + 2 * sizeof(int32_t) + 2 * sizeof(int16_t)))
        {
            uint16_t user;
            uint8_t action;
            int32_t par12[2];
            int16_t par34[2];
            if (!read_packet_bytes(&user, sizeof(user)) || !read_packet_bytes(&action, sizeof(action))
             || !read_packet_bytes(par12, sizeof(par12)) || !read_packet_bytes(par34, sizeof(par34)))
                return false;
            used += sizeof(user) + sizeof(action) + sizeof(par12) + sizeof(par34);
            if ((user < MAX_NET_USERS) && (get_net_user_player_number(user) >= 0))
            {
                struct Packet saved = game.packets[user];
                memset(&game.packets[user], 0, sizeof(struct Packet));
                set_packet_action(&game.packets[user], action, par12[0], par12[1], par34[0], par34[1]);
                process_user_global_packet_action(user);
                game.packets[user] = saved;
            } else
                WARNLOG("Paused action for invalid user %u in Packet File", (unsigned)user);
        } else
        if (apply && (kind == LTK_NetworkStopped))
        {
            apply_recorded_network_stop();
        }
        if ((len > used) && !skip_packet_bytes(len - used))
            return false;
    }
}

// reads one whole turn from replay data
static TbBool read_turn(unsigned char *pckt_buf, int nusers, TbBigChecksum *chksum, TbBool *has_records, struct PacketDecoderState *records_state)
{
    *has_records = false;
    if (!read_packet_bytes(chksum, sizeof(*chksum)))
        return false;
    if (*chksum == LONG_TURN_MARKER)
    {
        *has_records = true;
        save_packet_decoder_state(records_state);
        if (!read_long_turn_records(false))
            return false;
        if (!read_packet_bytes(chksum, sizeof(*chksum)))
            return false;
    }
    if (!read_turn_packets(pckt_buf, nusers))
        return false;
    return !packet_file_compressed() || packet_codec_at_code_boundary();
}

static TbBool apply_long_turn_records(const struct PacketDecoderState *records_state)
{
    struct PacketDecoderState resume;
    save_packet_decoder_state(&resume);
    TbBool ok = restore_packet_decoder_state(records_state) && read_long_turn_records(true);
    if (!restore_packet_decoder_state(&resume))
        ok = false;
    return ok;
}

static GameTurn count_stored_turns(void)
{
    NetUserId users[MAX_NET_USERS];
    const int nusers = packet_saved_users(users);
    unsigned char pckt_buf[PACKET_TURN_MAX_SIZE+4];
    const unsigned int start_pos = replay.file_pos;
    const int32_t file_len = LbFileLengthHandle(replay.fp);
    GameTurn turns = 0;
    TbBigChecksum chksum;
    TbBool has_records;
    struct PacketDecoderState records_state;
    LbFileSeek(replay.fp, start_pos, Lb_FILE_SEEK_BEGINNING);
    int32_t end_pos = start_pos;
    while (read_turn(pckt_buf, nusers, &chksum, &has_records, &records_state))
    {
        turns++;
        end_pos = LbFilePosition(replay.fp);
    }
    packet_file_ends_in_reserved = packet_dec_reserved;
    if ((end_pos != file_len) && !packet_file_ends_in_reserved)
        ERRORLOG("Packet File unreadable at offset %d of %d; replay ends after %u turns", (int)end_pos, (int)file_len, (unsigned)turns);
    LbFileSeek(replay.fp, start_pos, Lb_FILE_SEEK_BEGINNING);
    replay.file_pos = start_pos;
    reset_packet_codec();
    return turns;
}

TbBool open_packet_file_for_load(char *fname, struct CatalogueEntry *centry)
{
    memset(centry, 0, sizeof(struct CatalogueEntry));
    strcpy(replay.fname, fname);
    replay.fp = LbFileOpen(replay.fname, Lb_FILE_MODE_READ_ONLY);
    if (!replay.fp)
    {
        ERRORLOG("Cannot open keeper packet file for load");
        replay.fopened = 0;
        return false;
    }
    int i = load_game_chunks(replay.fp, centry);
    if ((i != GLoad_PacketStart) && (i != GLoad_PacketContinue))
    {
        LbFileClose(replay.fp);
        replay.fp = NULL;
        replay.fopened = 0;
        WARNMSG("Couldn't correctly read packet file \"%s\" header.",fname);
        return false;
    }
    replay.file_pos = LbFilePosition(replay.fp);
    reset_packet_codec();
    replay.turns_stored = count_stored_turns();
    replay_playback_paused = false;
    if ((replay.checksum_verify) && !flag_is_set(replay.head.flags, PSHF_Checksum))
    {
        WARNMSG("PacketSave checksum not available, checking disabled.");
        replay.checksum_verify = false;
    }
    if (replay.log_things_start_turn == -1)
    {
        replay.log_things_start_turn = 0;
        replay.log_things_end_turn = replay.turns_stored + 1;
    }
    replay.fopened = 1;
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
        PlayerNumber plyr_idx = replay.head.user_players[user];
        if (plyr_idx < 0)
            continue;
        if ((plyr_idx >= PLAYERS_COUNT)
         || !flag_is_set(replay.head.players_exist, to_flag(plyr_idx)))
        {
            WARNLOG("Packet file maps user %d to player %d, which the file says does not exist",
                (int)user, (int)plyr_idx);
            continue;
        }
        set_net_user_player_number(user, plyr_idx);
        struct PlayerInfo *player = get_player(plyr_idx);
        player->user_id = user;
        snprintf(player->player_name, sizeof(player->player_name), "%s",
            replay.head.user_names[user]);
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

// where a resync record can still be inserted into the turn just written
struct TurnRewritePoint {
    TbBool valid;
    TbBool was_long_turn;
    int32_t file_pos;
    uint32_t bits_acc;
    int bits_count;
    TbBool bits_pending;
    struct Packet codec_prev[MAX_NET_USERS];
    TbBigChecksum chksum;
    int nusers;
    unsigned char pckt_buf[PACKET_TURN_MAX_SIZE+4];
};
static struct TurnRewritePoint turn_rewrite;

static void save_turn_rewrite_point(TbBool was_long_turn)
{
    turn_rewrite.was_long_turn = was_long_turn;
    turn_rewrite.file_pos = LbFilePosition(replay.fp);
    turn_rewrite.bits_acc = packet_bits_acc;
    turn_rewrite.bits_count = packet_bits_count;
    turn_rewrite.bits_pending = packet_bits_pending;
    memcpy(turn_rewrite.codec_prev, packet_codec_prev, sizeof(turn_rewrite.codec_prev));
    turn_rewrite.valid = (turn_rewrite.file_pos >= 0);
}

static TbBool rewind_to_turn_rewrite_point(void)
{
    release_turn_buffer();
    packet_bits_acc = turn_rewrite.bits_acc;
    packet_bits_count = turn_rewrite.bits_count;
    packet_bits_pending = turn_rewrite.bits_pending;
    memcpy(packet_codec_prev, turn_rewrite.codec_prev, sizeof(packet_codec_prev));
    replay_long_turn_open = turn_rewrite.was_long_turn;
    return (LbFileSeek(replay.fp, turn_rewrite.file_pos, Lb_FILE_SEEK_BEGINNING) >= 0);
}

static TbBool write_turn_end(TbBigChecksum chksum, const unsigned char *pckt_buf, int nusers)
{
    TbBool ok = true;
    if (replay_long_turn_open || (chksum == LONG_TURN_MARKER))
    {
        ok &= begin_long_turn() && finish_turn_write();
        save_turn_rewrite_point(true);
        const unsigned char end = LTK_End;
        ok &= write_packet_bytes(&end, sizeof(end));
        replay_long_turn_open = false;
    }
    ok &= write_packet_bytes(&chksum, sizeof(chksum));
    ok &= write_turn_packets(pckt_buf, nusers);
    ok &= finish_turn_write();
    return ok;
}

static void replay_write_record(unsigned char kind, const void *payload, uint32_t payload_len)
{
    turn_rewrite.valid = false;
    LbFileSeek(replay.fp, 0, Lb_FILE_SEEK_END);
    const size_t raw_len = (replay_long_turn_open ? 0 : sizeof(TbBigChecksum)) + 5 + sizeof(uint32_t) + sizeof(unsigned char) + sizeof(uint32_t) + payload_len;
    if (!replay_write_fits(raw_len))
        return;
    if (!begin_long_turn() || !write_long_turn_record(kind, payload_len, payload))
    {
        ERRORLOG("Packet file write error at turn %u; recording stopped", (unsigned)get_gameturn());
        close_packet_file();
        replay.save_enable = false;
        return;
    }
    LbFileFlush(replay.fp);
}

void replay_record_chat_message(NetUserId user, const char *message, MapCoord cursor_x, MapCoord cursor_y)
{
    if (!replay.save_enable || !replay.fopened || !chat_message_recorded(message))
        return;
    unsigned char payload[sizeof(uint16_t) + 2 * sizeof(int32_t) + PLAYER_MP_MESSAGE_LEN];
    uint32_t len = 0;
    const uint16_t user16 = user;
    memcpy(payload + len, &user16, sizeof(user16));
    len += sizeof(user16);
    const TbBool is_command = (message[0] == cmd_char);
    if (is_command)
    {
        const int32_t pos[2] = {cursor_x, cursor_y};
        memcpy(payload + len, pos, sizeof(pos));
        len += sizeof(pos);
    }
    const size_t msg_len = strnlen(message, PLAYER_MP_MESSAGE_LEN - 1);
    memcpy(payload + len, message, msg_len);
    len += msg_len;
    replay_write_record(is_command ? LTK_ChatCommand : LTK_ChatMessage, payload, len);
}

static TbBool packet_recorded_while_paused(const struct Packet *pckt)
{
    switch (pckt->action)
    {
    case PckA_SetViewType:
    case PckA_SaveViewType:
    case PckA_LoadViewType:
    case PckA_SwitchView:
    case PckA_ZoomFromMap:
    case PckA_SetComputerKind:
        return true;
    default:
        return false;
    }
}

void replay_record_paused_action(NetUserId user, const struct Packet *pckt)
{
    if (!replay.save_enable || !replay.fopened || network_is_active())
        return;
    if (!packet_recorded_while_paused(pckt))
        return;
    unsigned char payload[sizeof(uint16_t) + sizeof(uint8_t) + 2 * sizeof(int32_t) + 2 * sizeof(int16_t)];
    uint32_t len = 0;
    const uint16_t user16 = user;
    const int32_t par12[2] = {pckt->actn_par1, pckt->actn_par2};
    const int16_t par34[2] = {pckt->actn_par3, pckt->actn_par4};
    memcpy(payload + len, &user16, sizeof(user16));
    len += sizeof(user16);
    payload[len++] = pckt->action;
    memcpy(payload + len, par12, sizeof(par12));
    len += sizeof(par12);
    memcpy(payload + len, par34, sizeof(par34));
    len += sizeof(par34);
    replay_write_record(LTK_PausedAction, payload, len);
}

void replay_record_network_stopped(void)
{
    if (!replay.save_enable || !replay.fopened)
        return;
    static const unsigned char no_payload = 0;
    replay_write_record(LTK_NetworkStopped, &no_payload, 0);
}

void replay_record_resync(const void *message, size_t message_size)
{
    if (!replay.save_enable || !replay.fopened)
        return;
    if (message_size > LONG_TURN_RECORD_MAX_LEN)
    {
        stop_replay_recording("resync too large to record");
        return;
    }
    if (!turn_rewrite.valid)
    {
        replay_write_record(LTK_Resync, message, message_size);
        return;
    }
    const size_t raw_len = sizeof(TbBigChecksum) + 5 + 2 * sizeof(uint32_t) + sizeof(unsigned char) + message_size
        + sizeof(unsigned char) + sizeof(TbBigChecksum) + turn_rewrite.nusers * sizeof(struct Packet);
    TbBool ok = rewind_to_turn_rewrite_point();
    if (ok && !replay_write_fits(raw_len))
        return;
    if (ok && !turn_rewrite.was_long_turn)
        ok = begin_long_turn();
    ok = ok && write_long_turn_record(LTK_Resync, message_size, message);
    ok = ok && write_turn_end(turn_rewrite.chksum, turn_rewrite.pckt_buf, turn_rewrite.nusers);
    if (!ok)
    {
        turn_rewrite.valid = false;
        ERRORLOG("Packet file write error at turn %u; recording stopped", (unsigned)get_gameturn());
        close_packet_file();
        replay.save_enable = false;
        return;
    }
    LbFileFlush(replay.fp);
}

// a recorded resync is applied at the end of its turn, where the live game resynced
void replay_apply_pending_resync(void)
{
    if (pending_resync == NULL)
        return;
    if (!apply_recorded_resync(pending_resync, pending_resync_len))
        WARNLOG("Recorded resync could not be applied");
    free(pending_resync);
    pending_resync = NULL;
    pending_resync_len = 0;
}


void replay_forget_saved_turn(void)
{
    turn_rewrite.valid = false;
}

short save_packets(void)
{
    NetUserId users[MAX_NET_USERS];
    const int nusers = packet_saved_users(users);
    SYNCDBG(6,"Starting");
    const TbBigChecksum chksum = replay.checksum_verify ? compute_replay_integrity() : 0;
    LbFileSeek(replay.fp, 0, Lb_FILE_SEEK_END);
    for (int i = 0; i < nusers; i++)
        memcpy(&turn_rewrite.pckt_buf[i*sizeof(struct Packet)], &game.packets[users[i]], sizeof(struct Packet));
    turn_rewrite.chksum = chksum;
    turn_rewrite.nusers = nusers;
    const TbBool close_long_turn = replay_long_turn_open || (chksum == LONG_TURN_MARKER);
    const size_t raw_len = (close_long_turn ? ((replay_long_turn_open ? 0 : sizeof(TbBigChecksum)) + sizeof(unsigned char)) : 0)
        + sizeof(TbBigChecksum) + nusers * sizeof(struct Packet);
    turn_rewrite.valid = false;
    if (!replay_write_fits(raw_len))
        return true;
    save_turn_rewrite_point(false);
    if (!write_turn_end(chksum, turn_rewrite.pckt_buf, nusers))
    {
        turn_rewrite.valid = false;
        ERRORLOG("Packet file write error at turn %u; recording stopped", (unsigned)get_gameturn());
        close_packet_file();
        replay.save_enable = false;
        return false;
    }
    if (!LbFileFlush(replay.fp))
    {
        ERRORLOG("Unable to flush PacketSave File");
        return false;
    }
    return true;
}

TbBool replay_playback_is_paused(void)
{
    return replay.load_enable && replay_playback_paused;
}

void set_replay_playback_paused(TbBool paused)
{
    if (paused == replay_playback_paused)
        return;
    replay_playback_paused = paused;
    process_pause_packet(paused, 0);
    set_flag_value(game.operation_flags, GOF_Paused, paused);
}

void stop_replay_recording(const char *reason)
{
    if (!replay.save_enable || !replay.fopened)
        return;
    WARNLOG("Replay recording stopped: %s", reason);
    close_packet_file();
    replay.save_enable = false;
}

void close_packet_file(void)
{
    if ( replay.fopened )
    {
        LbFileClose(replay.fp);
        replay.fopened = 0;
        replay.fp = NULL;
    }
}

TbBool reinit_packets_after_load(void)
{
    close_packet_file();
    replay.save_enable = false;
    replay.load_enable = false;
    return true;
}

static const char replay_type_chars[ReplTyp_Count] = {'c', 'f', 'm'};
static const char *const replay_type_dirs[ReplTyp_Count] = {"campaign", "freeplay", "multiplayer"};

static int compare_replay_names(const void *a, const void *b)
{
    return strcmp(*(char *const *)a, *(char *const *)b);
}

// names start with a 15-char timestamp, then '_' and the map type char [cfm]
#define REPLAY_TYPE_CHAR_POS 16

static void evict_old_replays(int type, uint32_t keep)
{
    const char type_chr = replay_type_chars[type];
    char **names = NULL;
    size_t count = 0;
    size_t cap = 0;
    struct TbFileEntry fe;
    char spec[DISKPATH_SIZE];
    char rel[DISKPATH_SIZE];
    snprintf(rel, sizeof(rel), "%s/*.pck", replay_type_dirs[type]);
    prepare_file_path_buf(spec, sizeof(spec), FGrp_Replays, rel);
    if (spec[0] == '\0')
        return;
    struct TbFileFind *ff = LbFileFindFirst(spec, &fe);
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
        snprintf(rel, sizeof(rel), "%s/%s", replay_type_dirs[type], names[i]);
        prepare_file_path_buf(fname, sizeof(fname), FGrp_Replays, rel);
        if (fname[0] != '\0')
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
    if (replays_enabled == false)
        return false;
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
    evict_old_replays(type, max_replays[type] - 1);

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
    char fname[sizeof(replay.fname)];
    snprintf(fname, sizeof(fname), "%s/%04d%02d%02dT%02d%02d%02d_%c%d",
        replay_type_dirs[type], year, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec,
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
    prepare_file_path_buf(replay.fname, sizeof(replay.fname), FGrp_Replays, fname);
    if (replay.fname[0] == '\0')
    {
        ERRORLOG("Replay path for \"%s\" is too long; not recording", fname);
        return false;
    }
    replay.save_enable = true;
    return true;
}

TbBool open_new_packet_file_for_save(void)
{
    // Filling the header
    SYNCMSG("Starting packet saving, turn %lu",(unsigned long)get_gameturn());
    replay.head.game_ver_major = VER_MAJOR;
    replay.head.game_ver_minor = VER_MINOR;
    replay.head.game_ver_release = VER_RELEASE;
    replay.head.game_ver_build = VER_BUILD;
    replay.head.level_num = get_loaded_level_number();
    replay.head.players_exist = 0;
    replay.head.players_comp = 0;
    replay_long_turn_open = false;
    reset_packet_codec();
    replay.head.flags = PSHF_Compressed;
    if (replay.checksum_verify)
        set_flag(replay.head.flags, PSHF_Checksum);
    if (game.game_kind == GKind_MultiGame)
        set_flag(replay.head.flags, PSHF_MultiGame);
    replay.head.action_seed = initial_replay_seed;
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        struct UserStartSettings *us = &replay.head.user_start[user];
        if (!get_startup_user_settings(user, us))
            build_local_user_start_settings(us);
    }
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
        replay.head.user_players[user] = get_net_user_player_number(user);
    replay.head.recording_user = get_local_user();
    replay.head.frontend_alliances = frontend_alliances;
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        const char *name = network_user_name(user);
        snprintf(replay.head.user_names[user],
            sizeof(replay.head.user_names[user]), "%s", (name != NULL) ? name : "");
    }
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player))
        {
            set_flag(replay.head.players_exist, to_flag(i));
            if ((player->allocflags & PlaF_CompCtrl) != 0)
              set_flag(replay.head.players_comp, to_flag(i));
        }
    }
    LbFileDelete(replay.fname);
    replay.fp = LbFileOpen(replay.fname, Lb_FILE_MODE_NEW);
    if (!replay.fp)
    {
        ERRORLOG("Cannot open keeper packet file for save, \"%s\".",replay.fname);
        replay.fopened = 0;
        return false;
    }
    struct CatalogueEntry centry;
    fill_game_catalogue_entry(&centry, "Packet file");
    if (!save_packet_chunks(replay.fp,&centry))
    {
        WARNMSG("Cannot write to packet file, \"%s\".",replay.fname);
        LbFileClose(replay.fp);
        replay.fopened = 0;
        replay.fp = NULL;
        return false;
    }
    replay.fopened = 1;
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
    if ((nturn >= replay.turns_stored) && packet_file_ends_in_reserved)
    {
        ERRORLOG("Packet File uses an unrecognized encoding at turn %u; stopping replay", (unsigned)nturn);
        packet_file_ends_in_reserved = false;
        disable_packet_mode();
        return;
    }
    if (nturn >= replay.turns_stored)
    {
        ERRORDBG(18,"Out of turns to load from Packet File");
        erstat_inc(ESE_CantReadPackets);
        return;
    }

    TbBigChecksum tot_chksum;
    TbBool has_records;
    struct PacketDecoderState records_state;
    if (!read_turn(pckt_buf, nusers, &tot_chksum, &has_records, &records_state))
    {
        ERRORLOG("Cannot read turn data from Packet File; replay aborted at turn %u", (unsigned)get_gameturn());
        erstat_inc(ESE_CantReadPackets);
        disable_packet_mode();
        return;
    }
    for (int i = 0; i < nusers; i++)
        memcpy(&game.packets[users[i]], &pckt_buf[i * sizeof(struct Packet)], sizeof(struct Packet));
    if (has_records && !apply_long_turn_records(&records_state))
    {
        ERRORLOG("Cannot read long turn records from Packet File; replay aborted at turn %u", (unsigned)get_gameturn());
        erstat_inc(ESE_CantReadPackets);
        disable_packet_mode();
        return;
    }
    if (replay.turns_fastforward > 0)
        replay.turns_fastforward--;
    replay_expected_checksum = tot_chksum;
    replay_checksum_pending = replay.checksum_verify && !turn_has_quit_packet();
}

void verify_replay_checksum(void)
{
    if (!replay_checksum_pending)
        return;
    replay_checksum_pending = false;
    if (compute_replay_integrity() != replay_expected_checksum)
    {
        ERRORLOG("PacketSave checksum - Out of sync (GameTurn %u)", get_gameturn());
        if (!is_onscreen_msg_visible())
            show_onscreen_msg(turns_per_second, "Out of sync");
    }
}

void disable_packet_mode(void)
{
    close_packet_file();
    replay.load_enable = false;
    replay.save_enable = false;
    get_my_player()->cheats_allowed = game.easter_eggs_enabled;
    remap_local_user_to_solo();
    show_onscreen_msg(2*turns_per_second, "Packet mode disabled");
    set_gui_visible(true);
}
