#include "pre_inc.h"
#include "net_spectator.h"

#include "bflib_datetm.h"
#include "bflib_math.h"
#include "dungeon_data.h"
#include "engine_render.h"
#include "game_legacy.h"
#include "front_network.h"
#include "frontmenu_ingame_map.h"
#include "gui_msgs.h"
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

static struct SpectatorTurn spectator_turns[PACKET_HISTORY_SIZE];
static int32_t spectator_turn_index;
int32_t network_spectator_turn_count;
static struct SpectatorTurn spectator_send_turns[SPECTATOR_TURN_BUNDLE_SIZE];
static int32_t spectator_send_turn_index;
static int32_t spectator_send_turn_count[MAX_NET_SPECTATORS];
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
        } else {
            spectator_send_turn_count[id - MAX_NET_USERS] = 0;
        }
    }
}

void network_spectator_clear_roles(void)
{
    if (network_user_is_spectator(netstate.my_id)) {
        return_to_player_camera();
    }
    network_spectators = 0;
    memset(spectator_send_turn_count, 0, sizeof(spectator_send_turn_count));
}

void network_spectator_reset(void)
{
    network_spectator_desynced = false;
    network_spectator_resync_pending = false;
    spectator_turn_index = 0;
    network_spectator_turn_count = 0;
    spectator_send_turn_index = 0;
    memset(spectator_send_turn_count, 0, sizeof(spectator_send_turn_count));
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
    spectator_turn_index = (spectator_turn_index + 1) % PACKET_HISTORY_SIZE;
    network_spectator_turn_count -= 1;
}

static void share_human_player_vision_with_spectator(void)
{
    local_observer_player.allied_players = 0;
    for (PlayerNumber i = 0; i < PLAYERS_COUNT; i += 1) {
        const struct PlayerInfo *player = get_player(i);
        if (is_active_keeper(player) && !flag_is_set(player->allocflags, PlaF_CompCtrl)) {
            set_flag(local_observer_player.allied_players, to_flag(i));
        }
    }
    panel_map_update(0, 0, game.map_subtiles_x + 1, game.map_subtiles_y + 1);
}

static void position_spectator_camera_at_random_heart(void)
{
    struct Thing *starting_heart = NULL;
    int32_t heart_count = 0;
    for (PlayerNumber i = 0; i < PLAYERS_COUNT; i += 1) {
        if (!flag_is_set(local_observer_player.allied_players, to_flag(i))) {
            continue;
        }
        struct Thing *heart = get_player_soul_container(i);
        if (!thing_exists(heart)) {
            continue;
        }
        heart_count += 1;
        if (UNSYNC_RANDOM(heart_count) == 0) {
            starting_heart = heart;
        }
    }
    if (starting_heart != NULL) {
        local_observer_user_state.dungeon_camera.x = starting_heart->mappos.x.val;
        local_observer_user_state.dungeon_camera.y = starting_heart->mappos.y.val;
    }
}

TbBool network_spectator_start(void)
{
    PlayerNumber camera_player_number = my_player_number;
    my_player_number = PLAYER_NEUTRAL;
    memset(&local_observer_player, 0, sizeof(local_observer_player));
    memset(&local_observer_user_state, 0, sizeof(local_observer_user_state));
    init_local_player_state();
    local_observer_player.id_number = my_player_number;
    local_observer_player.user_id = netstate.my_id;
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
    local_observer_player = *get_player(camera_player_number);
    const struct UserState *camera_state = get_player_user_state(get_player(camera_player_number));
    if (!user_state_invalid(camera_state)) {
        local_observer_user_state.dungeon_camera = camera_state->dungeon_camera;
        local_observer_user_state.dungeon_wibble = camera_state->dungeon_wibble;
    }
    local_observer_player.id_number = my_player_number;
    local_observer_player.user_id = netstate.my_id;
    local_observer_player.allocflags = PlaF_Allocated;
    local_observer_player.cheats_allowed = true;
    local_observer_player.victory_state = VicS_LostLevel;
    local_observer_user_state.view_type = PVT_DungeonTop;
    local_observer_player.controlled_thing_idx = 0;
    local_observer_player.influenced_thing_idx = 0;
    local_observer_player.instance_num = 0;
    local_observer_player.instance_remain_turns = 0;
    share_human_player_vision_with_spectator();
    memset(local_observer_player.mp_pending_message, 0, sizeof(local_observer_player.mp_pending_message));
    memset(local_observer_player.mp_message_text, 0, sizeof(local_observer_player.mp_message_text));
    memset(local_observer_player.mp_message_text_last, 0, sizeof(local_observer_player.mp_message_text_last));
    position_spectator_camera_at_random_heart();
    init_local_cameras(&local_observer_player);
    enter_observer_camera();
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

void network_spectator_store_chat_messages(const struct QueuedGameplayChat *messages, int32_t count)
{
    if (netstate.my_id != SERVER_ID || network_spectators == 0) {
        return;
    }
    int32_t index = (spectator_send_turn_index + SPECTATOR_TURN_BUNDLE_SIZE - 1) % SPECTATOR_TURN_BUNDLE_SIZE;
    struct SpectatorTurn *snapshot = &spectator_send_turns[index];
    snapshot->chat_count = count;
    memcpy(snapshot->chat, messages, count * sizeof(messages[0]));
}

void network_spectator_flush_turns(int32_t minimum_turn_count)
{
    if (netstate.my_id != SERVER_ID || network_spectators == 0) {
        return;
    }
    for (int32_t spectator = 0; spectator < MAX_NET_SPECTATORS; spectator += 1) {
        uint64_t recipient = UINT64_C(1) << (MAX_NET_USERS + spectator);
        int32_t turn_count = spectator_send_turn_count[spectator];
        if ((network_spectators & recipient) == 0 || turn_count == 0) {
            continue;
        }
        if (minimum_turn_count > 1 && spectator % SPECTATOR_TURN_BUNDLE_SIZE != spectator_send_turn_index) {
            continue;
        }
        char *write_pos = begin_net_message(NETMSG_SPECTATOR_TURN_BUNDLE);
        for (int32_t i = 0; i < turn_count; i += 1) {
            int32_t index = (spectator_send_turn_index + SPECTATOR_TURN_BUNDLE_SIZE - turn_count + i) % SPECTATOR_TURN_BUNDLE_SIZE;
            const struct SpectatorTurn *snapshot = &spectator_send_turns[index];
            size_t snapshot_size = offsetof(struct SpectatorTurn, chat) + snapshot->chat_count * sizeof(snapshot->chat[0]);
            if ((size_t)(write_pos - netstate.msg_buffer) + snapshot_size > sizeof(netstate.msg_buffer)) {
                send_to_active_peers(1, NetSend_Reliable, netstate.msg_buffer, write_pos - netstate.msg_buffer, recipient);
                write_pos = begin_net_message(NETMSG_SPECTATOR_TURN_BUNDLE);
            }
            memcpy(write_pos, snapshot, snapshot_size);
            write_pos += snapshot_size;
        }
        send_to_active_peers(1, NetSend_Reliable, netstate.msg_buffer, write_pos - netstate.msg_buffer, recipient);
        spectator_send_turn_count[spectator] = 0;
    }
}

void network_spectator_send_turn(TbBigChecksum checksum)
{
    if (netstate.my_id != SERVER_ID || network_spectators == 0) {
        return;
    }
    struct SpectatorTurn *snapshot = &spectator_send_turns[spectator_send_turn_index];
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->turn = get_gameturn();
    snapshot->checksum = checksum;
    snapshot->input_lag_turns = game.input_lag_turns;
    snapshot->skip_initial_input_turns = game.skip_initial_input_turns;
    snapshot->operation_flags = game.operation_flags;
    memcpy(snapshot->packets, game.packets, sizeof(snapshot->packets));
    spectator_send_turn_index = (spectator_send_turn_index + 1) % SPECTATOR_TURN_BUNDLE_SIZE;
    for (int32_t spectator = 0; spectator < MAX_NET_SPECTATORS; spectator += 1) {
        if (network_spectators & (UINT64_C(1) << (MAX_NET_USERS + spectator))) {
            spectator_send_turn_count[spectator] += 1;
        }
    }
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
    network_spectator_flush_turns(1);
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
        if (message_buffer[0] == NETMSG_SPECTATOR_TURN_BUNDLE && network_spectator_turn_count > PACKET_HISTORY_SIZE - SPECTATOR_TURN_BUNDLE_SIZE) {
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

TbError process_network_spectator_turn_bundle_message(NetUserId source, const char *buffer, size_t buffer_size)
{
    if (source != SERVER_ID || netstate.phase != NetPhase_InGame || buffer_size == 0 || buffer_size >= sizeof(netstate.msg_buffer) || network_spectator_desynced) {
        return Lb_OK;
    }
    size_t offset = 0;
    int32_t turn_count = 0;
    struct SpectatorTurn bundle[SPECTATOR_TURN_BUNDLE_SIZE] = {0};
    while (offset < buffer_size) {
        size_t header_size = offsetof(struct SpectatorTurn, chat);
        if (turn_count >= SPECTATOR_TURN_BUNDLE_SIZE || buffer_size - offset < header_size) {
            return Lb_OK;
        }
        struct SpectatorTurn *snapshot = &bundle[turn_count];
        memcpy(snapshot, buffer + offset, header_size);
        size_t snapshot_size = header_size + snapshot->chat_count * sizeof(snapshot->chat[0]);
        if (snapshot->input_lag_turns > MAXIMUM_INPUT_LAG_TURNS || snapshot->chat_count > GAMEPLAY_CHAT_QUEUE_LEN || snapshot_size > buffer_size - offset) {
            return Lb_OK;
        }
        memcpy(snapshot->chat, buffer + offset + header_size, snapshot_size - header_size);
        for (int32_t i = 0; i < snapshot->chat_count; i += 1) {
            const struct QueuedGameplayChat *chat = &snapshot->chat[i];
            if (chat->user < 0 || chat->user >= MAX_NET_USERS || memchr(chat->message, '\0', sizeof(chat->message)) == NULL) {
                return Lb_OK;
            }
        }
        offset += snapshot_size;
        turn_count += 1;
    }
    if (turn_count > PACKET_HISTORY_SIZE - network_spectator_turn_count) {
        network_spectator_desynced = true;
        network_spectator_turn_count = 0;
        return Lb_OK;
    }
    for (int32_t turn_index = 0; turn_index < turn_count; turn_index += 1) {
        const struct SpectatorTurn *snapshot = &bundle[turn_index];
        int32_t index = (spectator_turn_index + network_spectator_turn_count) % PACKET_HISTORY_SIZE;
        spectator_turns[index] = *snapshot;
        network_spectator_turn_count += 1;
        for (NetUserId user = 0; user < MAX_NET_USERS; user += 1) {
            store_packet_history(user, &snapshot->packets[user]);
        }
    }
    return Lb_OK;
}

#ifdef __cplusplus
}
#endif
