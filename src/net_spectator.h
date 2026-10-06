#ifndef DK_NET_SPECTATOR_H
#define DK_NET_SPECTATOR_H

#include "bflib_basics.h"
#include "globals.h"
#include "net_main.h"
#include "packets.h"

#define SPECTATOR_TURN_BUNDLE_SIZE 20

struct QueuedGameplayChat;

#ifdef __cplusplus
extern "C" {
#endif

extern TbBool network_spectator_desynced;
extern TbBool network_spectator_resync_pending;
extern TbBool network_spectator_chat_visible;
extern int32_t network_spectator_turn_count;
void network_spectator_send_bootstrap(NetUserId user_id);
TbBool network_spectator_wait(enum NetMessageType message_type);
TbBool network_spectator_start(void);
TbError process_network_spectator_bootstrap(NetUserId source, const char *buffer, size_t size);
TbError process_network_spectator_ready(NetUserId source, size_t size);
TbError process_network_spectator_chat(NetUserId source, const char *buffer, size_t size);
void network_spectator_service(void);

TbBool network_user_is_spectator(NetUserId user_id);
void network_spectator_update_roles(void);
void network_spectator_clear_roles(void);
void network_spectator_reset(void);
void network_spectator_init_icons(uint32_t seed);
enum PacketExchangeResult network_spectator_load_turn(TbBigChecksum checksum);
void network_spectator_send_turn(TbBigChecksum checksum);
void network_spectator_store_chat_messages(const struct QueuedGameplayChat *messages, int32_t count);
void network_spectator_flush_turns(int32_t minimum_turn_count);
void network_spectator_send_snapshots(void);
void network_spectator_receive_turns(void *server_buf, size_t frame_size);
TbError process_network_spectator_turn_bundle_message(NetUserId source, const char *buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
