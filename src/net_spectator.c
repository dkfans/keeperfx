#include "pre_inc.h"
#include "net_spectator.h"
#include "observer.h"

#include "bflib_datetm.h"
#include "config_strings.h"
#include "game_legacy.h"
#include "front_network.h"
#include "frontend.h"
#include "gui_msgs.h"
#include "gui_topmsg.h"
#include "local_camera.h"
#include "net_exchange_common.h"
#include "net_exchange_gameplay.h"
#include "net_game.h"
#include "net_input_lag.h"
#include "net_lobby.h"
#include "net_resync.h"
#include "packets.h"
#include "player_data.h"
#include "player_instances.h"
#include "player_utils.h"
#include "sprites.h"
#include "keeperfx.hpp"
#include <stddef.h>
#include <SDL3/SDL.h>
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SpectatorTurn {
    GameTurn turn;
    TbBigChecksum checksum;
    uint8_t input_lag_turns;
    uint16_t skip_initial_input_turns;
    uint8_t operation_flags;
    uint8_t chat_count;
    struct Packet packets[MAX_NET_USERS];
    struct QueuedGameplayChat chat[GAMEPLAY_CHAT_QUEUE_LEN];
};

#define SPECTATOR_TURN_BUFFER_SIZE 256

static struct SpectatorTurn spectator_turns[SPECTATOR_TURN_BUFFER_SIZE];
static int32_t spectator_turn_index;
static TbBool spectator_catching_up;
int32_t network_spectator_turn_count;
static struct SpectatorTurn spectator_send_turn;
static int32_t spectator_send_turn_pending;
TbBool network_spectator_desynced;
static uint64_t network_spectators;
TbBool network_spectator_resync_pending;
TbBool network_spectator_chat_visible;

#define SPECTATOR_ICON_COUNT ((GPS_creatr_icon_tentc_std - GPS_creatr_icon_wizrd_std) / 2 + 2)

static uint32_t spectator_icon_seed;
static char spectator_icon_names[SPECTATOR_ICON_COUNT][sizeof(netstate.users[0].name)];

void network_spectator_init_icons(uint32_t seed)
{
    if (!network_is_host()) {
        return;
    }
    memset(spectator_icon_names, 0, sizeof(spectator_icon_names));
    spectator_icon_seed = seed;
}

static void assign_spectator_icon(NetUserId sender)
{
    const char *name = netstate.users[sender].name;
    uint32_t used = 0;
    for (NetUserId id = MAX_NET_USERS; id < MAX_NET_CONNECTIONS; id++) {
        used |= UINT32_C(1) << netstate.users[id].spectator_icon;
    }
    int32_t available = 0;
    int32_t available_count = 0;
    for (int32_t icon = 1; icon <= SPECTATOR_ICON_COUNT; icon++) {
        if ((used & (UINT32_C(1) << icon)) != 0) {
            continue;
        }
        if (strcmp(spectator_icon_names[icon - 1], name) == 0) {
            available = icon;
            break;
        }
        if (spectator_icon_names[icon - 1][0] == '\0' && LB_RANDOM(++available_count, &spectator_icon_seed) == 0) {
            available = icon;
        }
    }
    netstate.users[sender].spectator_icon = available;
    if (available != 0) {
        snprintf(spectator_icon_names[available - 1], sizeof(spectator_icon_names[0]), "%s", name);
    }
}

TbBool network_user_is_spectator(NetUserId user_id)
{
    return user_id >= MAX_NET_USERS && user_id < MAX_NET_CONNECTIONS;
}

TbError process_network_spectator_chat(NetUserId source, const char *buffer, size_t size)
{
    if (size < 4) {
        return Lb_OK;
    }
    NetUserId sender = (uint8_t)buffer[0];
    if (sender < MAX_NET_USERS || sender >= MAX_NET_CONNECTIONS || (source != SERVER_ID && source != sender)) {
        return Lb_OK;
    }
    int32_t icon = (uint8_t)buffer[1];
    const char *message = buffer + 2;
    size_t remaining = size - 2;
    if (remaining < 2 || remaining > PLAYER_MP_MESSAGE_LEN || message[remaining - 1] != '\0' || memchr(message, '\0', remaining - 1) != NULL) {
        return Lb_OK;
    }
    if (network_is_host()) {
        if (!network_user_is_spectator(source) || netstate.users[source].spectator_state == NetSpectator_Loading) {
            return Lb_OK;
        }
        icon = netstate.users[source].spectator_icon;
        netstate.msg_buffer[2] = icon;
        uint64_t recipients = ((UINT64_C(1) << MAX_NET_CONNECTIONS) - 1) & ~ALL_NET_USERS_MASK;
        if (net_config_info.spectator_chat) {
            recipients |= ALL_NET_USERS_MASK;
        }
        send_to_active_peers(1, NetSend_Reliable, netstate.msg_buffer, size + 1, recipients);
    } else if (source != SERVER_ID) {
        return Lb_OK;
    }
    if (icon > SPECTATOR_ICON_COUNT) {
        return Lb_OK;
    }
    if (!network_is_host() || net_config_info.spectator_chat) {
        if (icon == 0) {
            message_add(MsgType_Blank, 0, message);
        } else {
            int32_t sprite = GPS_creatr_icon_wizrd_std + 2 * (icon - 1);
            if (icon == SPECTATOR_ICON_COUNT) {
                sprite = GPS_creatr_icon_orc_std;
            }
            message_add(MsgType_Spectator, sprite, message);
        }
    }
    return Lb_OK;
}

void network_spectator_update_roles(void)
{
    network_spectators = 0;
    for (NetUserId id = MAX_NET_USERS; id < MAX_NET_CONNECTIONS; id++) {
        if (IsUserActive(id) && netstate.users[id].spectator_state == NetSpectator_Streaming) {
            network_spectators |= UINT64_C(1) << id;
        }
    }
}

void network_spectator_clear_roles(void)
{
    if (network_user_is_spectator(netstate.my_id)) {
        return_to_player_camera();
    }
    network_spectators = 0;
    spectator_send_turn_pending = 0;
}

void network_spectator_reset(void)
{
    network_spectator_desynced = false;
    network_spectator_resync_pending = false;
    spectator_turn_index = 0;
    spectator_catching_up = false;
    network_spectator_turn_count = 0;
    spectator_send_turn_pending = 0;
}

TbError process_network_spectator_ready(NetUserId source, size_t size)
{
    if (netstate.my_id == SERVER_ID && source >= MAX_NET_USERS && source < MAX_NET_CONNECTIONS && IsUserActive(source) && size == 0 && netstate.users[source].spectator_state == NetSpectator_Loading) {
        enum NetJoinRejection reason = net_lobby_spectator_rejection(source);
        if (reason != NetJoin_Accepted) {
            netstate.sp->drop_user(source, reason);
            return Lb_OK;
        }
        netstate.users[source].spectator_state = NetSpectator_SnapshotPending;
        assign_spectator_icon(source);
    }
    return Lb_OK;
}

void network_spectator_service(void)
{
    if (netstate.my_id != SERVER_ID) {
        return;
    }
    for (NetUserId id = 1; id < MAX_NET_CONNECTIONS; id++) {
        struct NetUser *user = &netstate.users[id];
        if (user->progress == USER_UNUSED || (user->progress == USER_LOGGEDIN && (id < MAX_NET_USERS || user->spectator_state == NetSpectator_Streaming))) {
            continue;
        }
        if (LbTimerClock() - user->connected_at >= TIMEOUT_WAIT_FOR_ALL_PLAYERS) {
            netstate.sp->drop_user(id, NetJoin_Locked);
        }
    }
}

static void remove_spectator_turn(void)
{
    spectator_turn_index = (spectator_turn_index + 1) % SPECTATOR_TURN_BUFFER_SIZE;
    network_spectator_turn_count -= 1;
}

TbBool network_spectator_start(void)
{
    PlayerNumber camera_player_number = my_player_number;
    my_player_number = PLAYER_NEUTRAL;
    memset(&local_observer_player, 0, sizeof(local_observer_player));
    memset(&local_observer_user_state, 0, sizeof(local_observer_user_state));
    init_local_player_state();
    local_observer_player.id_number = my_player_number;
    local_observer_player.allocflags = PlaF_Allocated;
    local_observer_user_state.teleport_destination = 19;
    local_observer_user_state.battleid = 1;
    memset(get_local_packet(), 0, sizeof(struct Packet));
    network_spectator_reset();
    char *write_pos = begin_net_message(NETMSG_SPECTATOR_READY);
    send_message_buffer(SERVER_ID, write_pos);
    if (!network_spectator_wait(NETMSG_GAMEPLAY_UNSEQUENCED)) {
        return false;
    }
    spectator_catching_up = true;
    observer_init(camera_player_number);
    show_onscreen_msg(10 * turns_per_second, "%s", get_string(GUIStr_NetYouAreSpectating));
    return true;
}

TbBool network_spectator_wait(enum NetMessageType message_type)
{
    TbClockMSec timeout = TIMEOUT_WAIT_FOR_ALL_PLAYERS;
    if (message_type == NETMSG_SPECTATOR_BOOTSTRAP) {
        netstate.phase = NetPhase_Unknown;
        timeout = TIMEOUT_JOIN_LOBBY;
    }
    TbClockMSec started = LbTimerClock();
    while (LbTimerClock() - started < timeout && net_join_rejection == NetJoin_Accepted) {
        netstate.sp->update(NULL);
        if (attempting_to_join_cancel_requested() || netstate.users[SERVER_ID].progress == USER_UNUSED) {
            return false;
        }
        if (netstate.sp->msgready(SERVER_ID, 0)) {
            if (process_network_message(SERVER_ID, NULL, 0, message_type, NULL) != Lb_OK) {
                return false;
            }
        } else {
            SDL_Delay(1);
        }
        if ((message_type == NETMSG_SPECTATOR_BOOTSTRAP && netstate.phase == NetPhase_InGame) || (message_type == NETMSG_GAMEPLAY_UNSEQUENCED && network_spectator_resync_pending)) {
            return true;
        }
    }
    return false;
}

TbBool network_spectator_is_catching_up(void)
{
    if (network_spectator_turn_count > 100) {
        spectator_catching_up = true;
    } else if (network_spectator_turn_count == 0 && !network_spectator_resync_pending && !netstate.sp->msgready(SERVER_ID, 0)) {
        spectator_catching_up = false;
    }
    return spectator_catching_up;
}

enum PacketExchangeResult network_spectator_load_turn(TbBigChecksum checksum)
{
    if (network_spectator_resync_pending) {
        network_spectator_resync_pending = false;
        clear_packets();
        return PExR_Advance;
    }
    while (network_spectator_turn_count > 0 && (GameTurnDelta)(spectator_turns[spectator_turn_index].turn - get_gameturn()) < 0) {
        remove_spectator_turn();
    }
    if (network_spectator_desynced || network_spectator_turn_count == 0) {
        return PExR_Wait;
    }
    const struct SpectatorTurn *snapshot = &spectator_turns[spectator_turn_index];
    if (snapshot->turn != get_gameturn() || snapshot->checksum != checksum) {
        ERRORLOG("Spectator desync: host turn=%u checksum=%08x local turn=%u checksum=%08x", (uint32_t)snapshot->turn, (uint32_t)snapshot->checksum, (uint32_t)get_gameturn(), (uint32_t)checksum);
        network_spectator_desynced = true;
        network_spectator_turn_count = 0;
        return PExR_Wait;
    }
    if ((game.operation_flags & GOF_Paused) != (snapshot->operation_flags & GOF_Paused)) {
        process_pause_packet(snapshot->operation_flags & GOF_Paused, snapshot->operation_flags & GOF_WorldInfluence);
    }
    game.input_lag_turns = snapshot->input_lag_turns;
    game.skip_initial_input_turns = snapshot->skip_initial_input_turns;
    clear_packets();
    memcpy(game.packets, snapshot->packets, sizeof(snapshot->packets));
    load_gameplay_chat_messages(snapshot->chat, snapshot->chat_count);
    remove_spectator_turn();
    return PExR_Advance;
}

void network_spectator_send_pending_turn(const struct QueuedGameplayChat *messages, int32_t count)
{
    if (netstate.my_id != SERVER_ID || spectator_send_turn_pending == 0) {
        return;
    }
    struct SpectatorTurn *snapshot = &spectator_send_turn;
    snapshot->chat_count = count;
    memcpy(snapshot->chat, messages, count * sizeof(messages[0]));
    char *write_pos = begin_net_message(NETMSG_SPECTATOR_TURN);
    size_t snapshot_size = offsetof(struct SpectatorTurn, chat) + snapshot->chat_count * sizeof(snapshot->chat[0]);
    memcpy(write_pos, snapshot, snapshot_size);
    write_pos += snapshot_size;
    send_to_active_peers(1, NetSend_Reliable, netstate.msg_buffer, write_pos - netstate.msg_buffer, network_spectators);
    spectator_send_turn_pending = 0;
}

void network_spectator_prepare_turn(TbBigChecksum checksum)
{
    if (netstate.my_id != SERVER_ID || network_spectators == 0) {
        return;
    }
    struct SpectatorTurn *snapshot = &spectator_send_turn;
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->turn = get_gameturn();
    snapshot->checksum = checksum;
    snapshot->input_lag_turns = game.input_lag_turns;
    snapshot->skip_initial_input_turns = game.skip_initial_input_turns;
    snapshot->operation_flags = game.operation_flags;
    memcpy(snapshot->packets, game.packets, sizeof(snapshot->packets));
    spectator_send_turn_pending = 1;
}

void network_spectator_send_snapshots(void)
{
    if (netstate.my_id != SERVER_ID) {
        return;
    }
    uint64_t resync_recipients = 0;
    for (NetUserId id = MAX_NET_USERS; id < MAX_NET_CONNECTIONS; id++) {
        if (IsUserActive(id) && netstate.users[id].spectator_state == NetSpectator_SnapshotPending) {
            resync_recipients |= UINT64_C(1) << id;
        }
    }
    if (resync_recipients == 0) {
        return;
    }
    if (send_spectator_resync(resync_recipients)) {
        for (NetUserId id = MAX_NET_USERS; id < MAX_NET_CONNECTIONS; id++) {
            if (resync_recipients & (UINT64_C(1) << id)) {
                netstate.users[id].spectator_state = NetSpectator_Streaming;
            }
        }
        network_spectator_update_roles();
    }
}

void network_spectator_receive_turns(void *server_buf, size_t frame_size)
{
    for (int32_t messages = 0; messages < 32; messages++) {
        const char *message_buffer;
        size_t message_size = netstate.sp->peekmsg(SERVER_ID, &message_buffer);
        if (message_size == 0) {
            break;
        }
        if (message_buffer[0] == NETMSG_SPECTATOR_TURN && network_spectator_turn_count >= SPECTATOR_TURN_BUFFER_SIZE) {
            break;
        }
        if (process_network_message(SERVER_ID, server_buf, frame_size, NETMSG_GAMEPLAY_UNSEQUENCED, NULL) != Lb_OK) {
            break;
        }
        if (network_spectator_resync_pending) {
            break;
        }
    }
}

TbError process_network_spectator_turn_message(NetUserId source, const char *buffer, size_t buffer_size)
{
    if (source != SERVER_ID || netstate.phase != NetPhase_InGame || buffer_size == 0 || buffer_size >= sizeof(netstate.msg_buffer) || network_spectator_desynced) {
        return Lb_OK;
    }
    size_t header_size = offsetof(struct SpectatorTurn, chat);
    struct SpectatorTurn snapshot = {0};
    if (buffer_size < header_size || buffer_size > sizeof(snapshot)) {
        return Lb_OK;
    }
    memcpy(&snapshot, buffer, buffer_size);
    size_t snapshot_size = header_size + snapshot.chat_count * sizeof(snapshot.chat[0]);
    if (snapshot.input_lag_turns > MAXIMUM_INPUT_LAG_TURNS || snapshot.chat_count > GAMEPLAY_CHAT_QUEUE_LEN || snapshot_size != buffer_size) {
        return Lb_OK;
    }
    for (int32_t i = 0; i < snapshot.chat_count; i += 1) {
        const struct QueuedGameplayChat *chat = &snapshot.chat[i];
        if (chat->user < 0 || chat->user >= MAX_NET_USERS || memchr(chat->message, '\0', sizeof(chat->message)) == NULL) {
            return Lb_OK;
        }
    }
    if (network_spectator_turn_count >= SPECTATOR_TURN_BUFFER_SIZE) {
        network_spectator_desynced = true;
        network_spectator_turn_count = 0;
        return Lb_OK;
    }
    int32_t index = (spectator_turn_index + network_spectator_turn_count) % SPECTATOR_TURN_BUFFER_SIZE;
    spectator_turns[index] = snapshot;
    network_spectator_turn_count += 1;
    for (NetUserId user = 0; user < MAX_NET_USERS; user += 1) {
        store_packet_history(user, &snapshot.packets[user]);
    }
    return Lb_OK;
}

#ifdef __cplusplus
}
#endif
