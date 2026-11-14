/******************************************************************************/
/** @file dai_net.c
 *     Daimon in a network game.
 * @par Purpose:
 *     See dai_net.h.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "dai_net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "bflib_basics.h"
#include "daimon_slots.h"
#include "keeper_ctl.h"
#include "game_legacy.h"
#include "net_exchange_common.h"
#include "net_exchange_gameplay.h"
#include "net_game.h"
#include "net_main.h"
#include "packets.h"
#include "player_data.h"
#include "player_instances.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

TbBool dai_net_slot_expected(NetUserId slot)
{
    if (!network_is_active() || (slot < MAX_NET_USERS) || (slot >= PACKETS_COUNT)) return false;
    const PlayerNumber p = get_daimon_slot_player(slot);
    return (p >= 0) && (daimon_slot_of(p) == slot);
}

void dai_net_host_author_slots(void)
{
    if (!network_is_active() || !network_is_host()) return;
    const struct Packet *mine = get_local_packet();
    // While the game takes no input (its first turns, a pause) a step written now would be dropped unprocessed and
    // the gesture lost: the controller waits. The slot's packet still goes out every turn -- the clients wait for it.
    const TbBool hold = (game.skip_initial_input_turns > 0) || flag_is_set(game.operation_flags, GOF_Paused);
    for (NetUserId slot = MAX_NET_USERS; slot < PACKETS_COUNT; slot++) {
        const PlayerNumber p = get_daimon_slot_player(slot);
        if ((p < 0) || (daimon_slot_of(p) != slot)) continue;
        struct Packet staging;
        memset(&staging, 0, sizeof(staging));
        if (!hold) {
            kctl_sweep(slot);
            kctl_tick(slot, p, &staging);
        }
        staging.turn = get_gameturn();
        staging.checksum = mine->checksum; // the host's for this turn, as its own packet carries
        store_packet_history(slot, &staging);
        exchange_slot_frame(slot, &staging);
    }
}

/******************************************************************************/
/* Adoption */

static TbBool s_adoption = false;
static PlayerNumber s_adopt_queue[PLAYERS_COUNT];
static int64_t s_adopt_count = 0;
struct DaiCheatRequest {
    PlayerNumber player;
    int64_t flags, gold, bonus_pct, discount_pct;
};
static struct DaiCheatRequest s_cheat_queue[PLAYERS_COUNT];
static int64_t s_cheat_count = 0;

void dai_net_set_adoption(TbBool on)
{
    s_adoption = on;
}

TbBool dai_net_adoption(void)
{
    return s_adoption;
}

static TbBool s_opponents = false;
static char s_opponents_personality[32];
static char s_opponents_difficulty[32];

void dai_net_set_opponents(TbBool on, const char *personality)
{
    if (s_opponents != on) NETLOG("Daimon: the lobby's AI keepers are %s", on ? "Daimon's" : "the built-in AI's");
    s_opponents = on;
    snprintf(s_opponents_personality, sizeof(s_opponents_personality), "%s", (personality != NULL) ? personality : "");
}

TbBool dai_net_opponents(void)
{
    return s_opponents;
}

const char *dai_net_opponents_personality(void)
{
    return s_opponents_personality;
}

void dai_net_set_opponents_difficulty(const char *difficulty)
{
    snprintf(s_opponents_difficulty, sizeof(s_opponents_difficulty), "%s", (difficulty != NULL) ? difficulty : "");
}

const char *dai_net_opponents_difficulty(void)
{
    return s_opponents_difficulty;
}

/* The host's choice: whether the players may have Daimon's assistant play beside them (co-op seats,
   docs/plans/hellhound-coop.md section 14). On by default; a host may turn it off for fairness. */
static TbBool s_no_assistants = false;

void dai_net_set_assistants_allowed(TbBool on)
{
    s_no_assistants = !on;
}

TbBool dai_net_assistants_allowed(void)
{
    return !s_no_assistants;
}

uint16_t dai_net_lobby_bits(void)
{
    return (uint16_t)((s_opponents ? 1u : 0u) | (s_adoption ? 2u : 0u) | (s_no_assistants ? 4u : 0u));
}

void dai_net_read_lobby_bits(uint16_t bits)
{
    s_opponents = (bits & 1u) != 0;
    s_adoption = (bits & 2u) != 0;
    s_no_assistants = (bits & 4u) != 0;
}

void dai_net_queue_opponents(uint16_t human_slots)
{
    if (!s_opponents) return;
    NETLOG("Daimon: the lobby's AI keepers: every keeper but the humans' (0x%x)", (unsigned)human_slots);
    for (PlayerNumber p = 0; p < PLAYERS_COUNT; p++) {
        if ((p == PLAYER_NEUTRAL) || (p == PLAYER_GOOD) || ((human_slots & (1u << p)) != 0)) continue;
        daimon_pending_slots_add(p);
    }
}

void dai_net_request_adoption(PlayerNumber plyr_idx)
{
    if ((plyr_idx < 0) || (plyr_idx >= PLAYERS_COUNT) || (s_adopt_count >= PLAYERS_COUNT)) return;
    for (int64_t i = 0; i < s_adopt_count; i++) {
        if (s_adopt_queue[i] == plyr_idx) return;
    }
    s_adopt_queue[s_adopt_count++] = plyr_idx;
    NETLOG("Daimon: player %d will be adopted", (int)plyr_idx);
}

void dai_net_cancel_adoption(PlayerNumber plyr_idx)
{
    for (int64_t i = 0; i < s_adopt_count; i++) {
        if (s_adopt_queue[i] != plyr_idx) continue;
        memmove(&s_adopt_queue[i], &s_adopt_queue[i + 1], sizeof(s_adopt_queue[0]) * (size_t)(s_adopt_count - i - 1));
        s_adopt_count--;
        NETLOG("Daimon: player %d will not be adopted", (int)plyr_idx);
        return;
    }
}

void dai_net_request_cheats(PlayerNumber plyr_idx, uint64_t flags, int64_t gold, int64_t bonus_pct, int64_t discount_pct)
{
    if ((plyr_idx < 0) || (plyr_idx >= PLAYERS_COUNT) || (flags == 0) || (s_cheat_count >= PLAYERS_COUNT)) return;
    for (int64_t i = 0; i < s_cheat_count; i++) {
        if (s_cheat_queue[i].player == plyr_idx) return;
    }
    s_cheat_queue[s_cheat_count++] = (struct DaiCheatRequest){ plyr_idx, (int64_t)flags, gold, bonus_pct, discount_pct };
    NETLOG("Daimon: player %d's cheats (0x%" PRIx64 ") will be applied", (int)plyr_idx, (uint64_t)flags);
}

void dai_net_set_pending_host_action(struct Packet *pckt)
{
    // The host's own packet; once the last client is gone the game is a local one and the packet is still the host's
    // (user 0), applied and recorded like any other. Not while a network game skips its packets (its first turns,
    // a pause): the action would be dropped with them.
    if ((network_is_active() && !network_is_host()) || replay.load_enable || (pckt->action != PckA_None)) return;
    if (network_is_active() && ((game.skip_initial_input_turns > 0) || flag_is_set(game.operation_flags, GOF_Paused)))
        return;
    if (s_adopt_count > 0) {
        set_packet_action(pckt, PckA_AdoptKeeper, s_adopt_queue[0], 0, 0, 0);
        memmove(&s_adopt_queue[0], &s_adopt_queue[1], sizeof(s_adopt_queue[0]) * (size_t)(s_adopt_count - 1));
        s_adopt_count--;
    } else if (s_cheat_count > 0) {
        const struct DaiCheatRequest *q = &s_cheat_queue[0];
        // par3 and par4 are 16 bits: the flags with the discount above them, the bonus alone; the gold in par2.
        set_packet_action(pckt, PckA_DaimonCheats, q->player, q->gold, q->flags | (q->discount_pct << 4), q->bonus_pct);
        memmove(&s_cheat_queue[0], &s_cheat_queue[1], sizeof(s_cheat_queue[0]) * (size_t)(s_cheat_count - 1));
        s_cheat_count--;
    }
}

/******************************************************************************/
/* Replication */

#pragma pack(1)
struct DaiStatePartHeader {
    uint64_t turn;
    uint16_t part;
    uint16_t parts;
    uint32_t compressed;    /**< bytes of the whole compressed state */
    uint32_t original;      /**< bytes once decompressed */
};
#pragma pack()

static struct {
    TbBool active;
    DaiTurn turn;
    uint16_t parts;
    uint32_t compressed;
    uint32_t original;
    uint16_t received;
    uint8_t got[DAI_NET_MAX_PARTS];
    uint8_t *data;
} s_pending;

static struct {
    uint8_t *data;
    size_t len;
    DaiTurn turn;
} s_snap[DAI_NET_SNAPSHOTS];

static void pending_clear(void)
{
    free(s_pending.data);
    memset(&s_pending, 0, sizeof(s_pending));
}

void dai_net_reset(void)
{
    s_adopt_count = 0;
    s_cheat_count = 0;
    pending_clear();
    for (int64_t i = 0; i < DAI_NET_SNAPSHOTS; i++) {
        free(s_snap[i].data);
        memset(&s_snap[i], 0, sizeof(s_snap[i]));
    }
}

TbBool dai_net_snapshot(int64_t i, const uint8_t **data, size_t *len, DaiTurn *turn)
{
    if ((i < 0) || (i >= DAI_NET_SNAPSHOTS) || (s_snap[i].data == NULL)) return false;
    *data = s_snap[i].data;
    *len = s_snap[i].len;
    *turn = s_snap[i].turn;
    return true;
}

/** The whole state has arrived: decompressed, it becomes the newest snapshot. */
static void pending_complete(void)
{
    uint8_t *state = (uint8_t *)malloc(s_pending.original);
    uLongf size = s_pending.original;
    if ((state == NULL) || (uncompress(state, &size, s_pending.data, s_pending.compressed) != Z_OK) || (size != s_pending.original)) {
        WARNLOG("Daimon: the host's state for turn %" PRIu64 " does not decompress; the previous one is kept", (uint64_t)s_pending.turn);
        free(state);
        pending_clear();
        return;
    }
    free(s_snap[DAI_NET_SNAPSHOTS - 1].data);
    memmove(&s_snap[1], &s_snap[0], sizeof(s_snap[0]) * (DAI_NET_SNAPSHOTS - 1));
    s_snap[0].data = state;
    s_snap[0].len = size;
    s_snap[0].turn = s_pending.turn;
    SYNCDBG(7, "Daimon: the host's state for turn %" PRIu64 " (%" PRIu64 " bytes) received", (uint64_t)s_snap[0].turn, (uint64_t)size);
    pending_clear();
}

TbBool dai_net_state_part(const uint8_t *payload, size_t len)
{
    struct DaiStatePartHeader h;
    if (len < sizeof(h)) return false;
    memcpy(&h, payload, sizeof(h));
    const uint8_t *bytes = payload + sizeof(h);
    const size_t n = len - sizeof(h);
    if ((h.original == 0) || (h.original > DAI_NET_MAX_STATE) || (h.compressed == 0) || (h.compressed > DAI_NET_MAX_STATE)
     || (h.parts == 0) || (h.parts != (h.compressed + DAI_NET_PART_BYTES - 1) / DAI_NET_PART_BYTES) || (h.part >= h.parts))
        return false;
    const size_t at = (size_t)h.part * DAI_NET_PART_BYTES;
    const size_t want = ((size_t)h.compressed - at < DAI_NET_PART_BYTES) ? (size_t)h.compressed - at : DAI_NET_PART_BYTES;
    if (n != want) return false;
    // Older than the newest complete snapshot, or than the state being gathered: not wanted.
    if ((s_snap[0].data != NULL) && ((GameTurnDelta)(h.turn - s_snap[0].turn) <= 0)) return true;
    if (s_pending.active && ((GameTurnDelta)(h.turn - s_pending.turn) < 0)) return true;
    if (!s_pending.active || (h.turn != s_pending.turn)) {
        // A newer state supersedes one still incomplete.
        pending_clear();
        s_pending.data = (uint8_t *)malloc(h.compressed);
        if (s_pending.data == NULL) return false;
        s_pending.active = true;
        s_pending.turn = (DaiTurn)h.turn;
        s_pending.parts = h.parts;
        s_pending.compressed = h.compressed;
        s_pending.original = h.original;
    } else if ((h.parts != s_pending.parts) || (h.compressed != s_pending.compressed) || (h.original != s_pending.original)) {
        return false; // the same turn told two ways: keep what came first
    }
    if (s_pending.got[h.part]) return true; // a duplicate
    memcpy(s_pending.data + at, bytes, n);
    s_pending.got[h.part] = 1;
    if (++s_pending.received == s_pending.parts) pending_complete();
    return true;
}

TbBool dai_net_state_parts(DaiTurn turn, const uint8_t *blob, size_t len, void (*part)(void *ctx, const uint8_t *payload, size_t len), void *ctx)
{
    if ((len == 0) || (len > DAI_NET_MAX_STATE)) return false;
    uLongf csize = compressBound(len);
    uint8_t *packed = (uint8_t *)malloc(csize);
    if ((packed == NULL) || (compress(packed, &csize, blob, len) != Z_OK) || (csize > DAI_NET_MAX_STATE)) {
        free(packed);
        return false;
    }
    uint8_t buf[sizeof(struct DaiStatePartHeader) + DAI_NET_PART_BYTES];
    struct DaiStatePartHeader h;
    h.turn = turn;
    h.parts = (uint16_t)((csize + DAI_NET_PART_BYTES - 1) / DAI_NET_PART_BYTES);
    h.compressed = (uint32_t)csize;
    h.original = (uint32_t)len;
    for (uint16_t i = 0; i < h.parts; i++) {
        h.part = i;
        const size_t at = (size_t)i * DAI_NET_PART_BYTES;
        const size_t n = (csize - at < DAI_NET_PART_BYTES) ? csize - at : DAI_NET_PART_BYTES;
        memcpy(buf, &h, sizeof(h));
        memcpy(buf + sizeof(h), packed + at, n);
        part(ctx, buf, sizeof(h) + n);
    }
    free(packed);
    return true;
}

static void send_part(void *ctx, const uint8_t *payload, size_t len)
{
    (void)ctx;
    char *write_pos = begin_net_message(NETMSG_DAIMON_STATE);
    memcpy(write_pos, payload, len);
    write_pos += len;
    send_to_active_peers(1, NetSend_Reliable, netstate.msg_buffer, (size_t)(write_pos - netstate.msg_buffer),
        ALL_NET_USERS_MASK & ~(UINT64_C(1) << netstate.my_id));
}

void dai_net_send_state(DaiTurn turn, const uint8_t *blob, size_t len)
{
    if (!network_is_active() || !network_is_host()) return;
    if (!dai_net_state_parts(turn, blob, len, send_part, NULL))
        WARNLOG("Daimon: its state (%" PRIu64 " bytes) could not be sent to the clients", (uint64_t)len);
}

TbError dai_net_process_state_message(NetUserId source, const char *buf, size_t len)
{
    if ((source != SERVER_ID) || network_is_host()) {
        WARNLOG("Ignoring Daimon's state from peer %" PRId64 ": only the host sends it", (int64_t)source);
        return Lb_OK;
    }
    if (!dai_net_state_part((const uint8_t *)buf, len))
        WARNLOG("Ignoring a malformed part of Daimon's state from the host (%" PRIu64 " bytes)", (uint64_t)len);
    return Lb_OK;
}

#ifdef __cplusplus
}
#endif
