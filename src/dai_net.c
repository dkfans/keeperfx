/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file dai_net.c
 *     Daimon in network games, the host's requests: a Daimon keeper's difficulty cheats and a dropped player's adoption, sent once in the host's own packet (dai_net.h). The fork's dai_net.c, its "Adoption" section, as Daimon's port tools copy it (scripts/daimon_port/sync.py in the fork).
 */
/******************************************************************************/
#include "pre_inc.h"
#include "dai_net.h"
#include <stdio.h>
#include <string.h>
#include "bflib_basics.h"
#include "daimon_slots.h"
#include "net_main.h"
#include "net_game.h"
#include "packets.h"
#include "player_data.h"
#include "player_instances.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

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


#ifdef __cplusplus
}
#endif
