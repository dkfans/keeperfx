/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/**
 * @author   KeeperFX Team
 * @date     18 Oct 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "pre_inc.h"
#include "bflib_enet.h"
#include "bflib_datetm.h"
#include "bflib_network.h"
#include "front_network.h"
#include "bflib_math.h"
#include "net_portforward.h"
#include "net_holepunch.h"
#include "net_matchmaking.h"
#include "game_legacy.h"
#include "player_data.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif
#include <enet6/enet.h>
#include <cstddef>
#include <climits>

#include "post_inc.h"

#define NUM_CHANNELS 2
#define MAX_PEERS 2
#define PEER_TIMEOUT_MIN_MS 2000
#define PEER_TIMEOUT_MAX_MS 5000
#define HOLEPUNCH_CONNECT_DELAY_MS 1000
#define HOLEPUNCH_PRE_CONNECT_DELAY_MS 500
#define HAPPY_EYEBALLS_DELAY_MS 250
#define JOIN_CONNECT_POLL_DELAY_MS 16
#define ENET_ADDRESS_BUFFER_SIZE 128

uint16_t external_ipv4_port = 0;
int skip_holepunch = 0;

namespace
{
    NetDropCallback g_drop_callback = nullptr;
    ENetHost *host = nullptr;
    ENetPeer *client_peer = nullptr;
    int host_is_dual_stack = 0;

    // List
    ENetPacket *oldest_packet = nullptr;
    ENetPacket *newest_packet = nullptr;
    int incoming_queue_size = 0;

    ENetHost *create_ipv6_host(enet_uint16 port)
    {
        ENetAddress bind_address;
        enet_address_build_any(&bind_address, ENET_ADDRESS_TYPE_IPV6);
        bind_address.port = port;
        return enet_host_create(ENET_ADDRESS_TYPE_IPV6, &bind_address, MAX_PEERS, NUM_CHANNELS, 0, 0);
    }

    TbError bf_enet_init(NetDropCallback drop_callback)
    {
        if (enet_initialize())
            return Lb_FAIL;
        g_drop_callback = drop_callback;
        return Lb_OK;
    }

    void host_destroy()
    {
        if (oldest_packet)
        {
            for (ENetPacket *packet = oldest_packet; packet != nullptr;)
            {
                ENetPacket *current_packet = packet;
                packet = static_cast<ENetPacket *>(packet->userData);
                enet_packet_destroy(current_packet);
            }

            oldest_packet = nullptr;
            newest_packet = nullptr;
            incoming_queue_size = 0;
        }
        client_peer = nullptr;
        if (host)
        {
            for (ENetPeer *peer = host->peers; peer < &host->peers[host->peerCount]; peer++) {
                if (peer->state == ENET_PEER_STATE_CONNECTED) {
                    enet_peer_disconnect(peer, 0);
                }
            }
            enet_host_flush(host);
            enet_host_destroy(host);
            host = nullptr;
        }
        external_ipv4_port = 0;
        host_is_dual_stack = 0;
    }

    void bf_enet_exit()
    {
        port_forward_remove_mapping();
        host_destroy();
        g_drop_callback = nullptr;
        enet_deinitialize();
    }

    /**
     * Sets this service provider up as a host for a new network game.
     * @param session String representing the network game to be hosted.
     *  This could be a hostname:port pair for TCP for instance.
     * @param options
     * @return Lb_FAIL or Lb_OK
     */
    TbError bf_enet_host(const char *session, void *)
    {
        if (!*session)
            return Lb_FAIL;
        const char *port_string = session;
        if (*port_string == ':') port_string++;
        int port = atoi(port_string);
        enet_uint16 actual_port = ENET_DEFAULT_PORT;
        if (port > 0)
            actual_port = (enet_uint16)port;
        ENetAddress address;
        enet_address_build_any(&address, ENET_ADDRESS_TYPE_IPV6);
        address.port = actual_port;
        host = enet_host_create(ENET_ADDRESS_TYPE_ANY, &address, MAX_PEERS, NUM_CHANNELS, 0, 0);
        if (host) {
            host_is_dual_stack = 1;
            address = host->address;
            LbNetLog("ENet: host created (dual-stack IPv4+IPv6) on port %d\n", (int)address.port);
        } else {
            LbNetLog("ENet: dual-stack host creation failed, falling back to IPv4-only\n");
            enet_address_build_any(&address, ENET_ADDRESS_TYPE_IPV4);
            address.port = actual_port;
            host = enet_host_create(ENET_ADDRESS_TYPE_IPV4, &address, MAX_PEERS, NUM_CHANNELS, 0, 0);
            if (!host)
                return Lb_FAIL;
            host_is_dual_stack = 0;
            address = host->address;
            LbNetLog("ENet: host created (IPv4) on port %d\n", (int)address.port);
        }
        enet_host_compress_with_range_coder(host);
        port_forward_add_mapping(address.port);
        external_ipv4_port = holepunch_stun_query(host, NULL, 0);
        return Lb_OK;
    }


    enet_uint16 parse_session_address(const char *session, char *output_hostname, size_t hostname_buffer_size)
    {
        enet_uint16 port = ENET_DEFAULT_PORT;
        if (session[0] == '[') {
            const char *bracket_end = strchr(session, ']');
            if (!bracket_end) {
                return 0;
            }
            size_t address_length = bracket_end - session - 1;
            if (address_length >= hostname_buffer_size) {
                return 0;
            }
            strncpy(output_hostname, session + 1, address_length);
            output_hostname[address_length] = '\0';
            if (bracket_end[1] == ':') {
                port = strtoul(bracket_end + 2, NULL, 10);
                if (port == 0) {
                    return 0;
                }
            }
        } else {
            const char *first_colon = strchr(session, ':');
            const char *last_colon = strrchr(session, ':');
            if (first_colon && first_colon != last_colon) {
                strncpy(output_hostname, session, hostname_buffer_size - 1);
                output_hostname[hostname_buffer_size - 1] = '\0';
            } else if (first_colon) {
                size_t address_length = first_colon - session;
                if (address_length >= hostname_buffer_size) {
                    return 0;
                }
                strncpy(output_hostname, session, address_length);
                output_hostname[address_length] = '\0';
                port = strtoul(first_colon + 1, NULL, 10);
                if (port == 0) {
                    return 0;
                }
            } else {
                strncpy(output_hostname, session, hostname_buffer_size - 1);
                output_hostname[hostname_buffer_size - 1] = '\0';
            }
        }
        return port;
    }

    int resolve_punch_address(const char *address_string, ENetAddressType type, int port, ENetAddress *output)
    {
        if (!address_string[0]) return 0;
        if (enet_address_set_host(output, type, address_string) < 0) return 0;
        output->port = (enet_uint16)port;
        return 1;
    }

    void cleanup_join_host(ENetHost *network_host, ENetPeer *peer) {
        if (!network_host)
            return;
        if (network_host == host) {
            host_destroy();
            return;
        }
        if (peer) {
            enet_peer_disconnect_now(peer, 0);
            enet_host_flush(network_host);
        }
        enet_host_destroy(network_host);
    }

    TbError finish_join(ENetHost *next_host, ENetPeer *next_peer, ENetHost *old_host, ENetPeer *old_peer,
        const char *join_type, const char *ip_version) {
        LbNetLog("Join: connected successfully via %s (%s)\n", join_type, ip_version);
        enet_peer_timeout(next_peer, 0, PEER_TIMEOUT_MIN_MS, PEER_TIMEOUT_MAX_MS);
        cleanup_join_host(old_host, old_peer);
        host = next_host;
        client_peer = next_peer;
        return Lb_OK;
    }

    TbError create_join_host(ENetAddressType address_type)
    {
        host_destroy();
        host = enet_host_create(address_type, NULL, MAX_PEERS, NUM_CHANNELS, 0, 0);
        if (!host) {
            LbNetLog("Join: failed to create ENet host\n");
            return Lb_FAIL;
        }
        return Lb_OK;
    }

    TbError connect_to_current_join_host(const ENetAddress *connect_address, const char *join_type,
        TbClockMSec display_deadline, TbClockMSec timeout_ms)
    {
        if (timeout_ms <= 0)
            return Lb_FAIL;
        enet_host_compress_with_range_coder(host);
        client_peer = enet_host_connect(host, connect_address, NUM_CHANNELS, 0);
        if (!client_peer) {
            LbNetLog("Join: enet_host_connect returned NULL\n");
            host_destroy();
            return Lb_FAIL;
        }
        ENetEvent enet_event;
        TbClockMSec connection_deadline = LbTimerClock() + timeout_ms;
        while (LbTimerClock() < connection_deadline) {
            int service_result = enet_host_service(host, &enet_event, 0);
            if (service_result > 0 && enet_event.type == ENET_EVENT_TYPE_CONNECT) {
                LbNetLog("Join: connected successfully via %s\n", join_type);
                enet_peer_timeout(client_peer, 0, PEER_TIMEOUT_MIN_MS, PEER_TIMEOUT_MAX_MS);
                return Lb_OK;
            }
            if (service_result > 0 && (enet_event.type == ENET_EVENT_TYPE_DISCONNECT || enet_event.type == ENET_EVENT_TYPE_DISCONNECT_TIMEOUT)) {
                LbNetLog("Join: connection rejected by host\n");
                break;
            }
            if (service_result > 0) {
                LbNetLog("Join: unexpected event type=%d\n", (int)enet_event.type);
            } else if (service_result < 0) {
                LbNetLog("Join: enet_host_service error %d\n", service_result);
                ERRORLOG("Unable to connect: %d", service_result);
                break;
            }
            TbClockMSec time_remaining = connection_deadline - LbTimerClock();
            if (time_remaining <= 0)
                break;
            enet_uint32 wait_ms = (enet_uint32)min((TbClockMSec)JOIN_CONNECT_POLL_DELAY_MS, time_remaining);
            SDL_Delay(wait_ms);
            display_attempting_to_join_message((int)((display_deadline - LbTimerClock()) / 1000));
            if (attempting_to_join_cancel_requested()) {
                LbNetLog("Join: cancelled by user\n");
                host_destroy();
                return Lb_FAIL;
            }
        }
        LbNetLog("Join: connection timed out or failed\n");
        host_destroy();
        return Lb_FAIL;
    }

    TbError join_direct_session(const char *session, TbClockMSec display_deadline, TbClockMSec timeout_ms, const char *join_label)
    {
        ENetAddress connect_address;
        char address_string[ENET_ADDRESS_BUFFER_SIZE] = {0};
        enet_uint16 port = parse_session_address(session, address_string, sizeof(address_string));
        if (port == 0 || address_string[0] == '\0')
            return Lb_FAIL;
        ENetAddressType connect_type = ENET_ADDRESS_TYPE_IPV4;
        const char *ip_version = "IPv4";
        if (strchr(address_string, ':') != NULL) {
            connect_type = ENET_ADDRESS_TYPE_IPV6;
            ip_version = "IPv6";
        }
        char join_type[64];
        snprintf(join_type, sizeof(join_type), "%s (%s)", join_label, ip_version);
        LbNetLog("Join: connecting via %s\n", join_type);
        if (enet_address_set_host(&connect_address, connect_type, address_string) < 0)
            return Lb_FAIL;
        connect_address.port = port;
        if (create_join_host(connect_type) != Lb_OK)
            return Lb_FAIL;
        TbClockMSec stage_start = LbTimerClock();
        TbError result = connect_to_current_join_host(&connect_address, join_type, display_deadline, timeout_ms);
        const char *stage_name = (connect_type == ENET_ADDRESS_TYPE_IPV6) ? "TIMEOUT_CONNECT_DIRECT_IPV6" : "TIMEOUT_CONNECT_DIRECT_IPV4";
        LbNetLog("Join: %s took %d ms (%s)\n", stage_name, (int)(LbTimerClock() - stage_start), result == Lb_OK ? "connected" : "failed");
        return result;
    }

    TbError join_direct_fallback(const PunchAddresses *punch_addresses, TbClockMSec display_deadline) {
        const int has_ipv4 = (punch_addresses->ipv4[0] != '\0');
        const int has_ipv6 = (punch_addresses->ipv6[0] != '\0');
        if (!has_ipv4 && !has_ipv6) {
            LbNetLog("Join: direct-connect fallback has no usable address\n");
            return Lb_FAIL;
        }
        LbNetLog("Join: hole-punch phase timed out, retrying via direct connect\n");
        char session[ENET_ADDRESS_BUFFER_SIZE];
        if (has_ipv6) {
            snprintf(session, sizeof(session), "[%s]:%d", punch_addresses->ipv6, ENET_DEFAULT_PORT);
            if (join_direct_session(session, display_deadline, TIMEOUT_CONNECT_DIRECT_IPV6, "direct connect fallback") == Lb_OK)
                return Lb_OK;
        }
        if (!has_ipv4)
            return Lb_FAIL;
        snprintf(session, sizeof(session), "%s:%d", punch_addresses->ipv4, ENET_DEFAULT_PORT);
        return join_direct_session(session, display_deadline, TIMEOUT_CONNECT_DIRECT_IPV4, "direct connect fallback");
    }

    TbError join_via_holepunch(TbClockMSec join_start_ms) {
        LbNetLog("Join: connecting via matchmaking server (UDP hole punching)\n");
        if (create_join_host(ENET_ADDRESS_TYPE_IPV4) != Lb_OK)
            return Lb_FAIL;
        uint16_t my_external_ipv4_port = holepunch_stun_query(host, NULL, 0);
        if (my_external_ipv4_port == 0)
            LbNetLog("Join: STUN failed, proceeding with port 0\n");
        ENetHost *ipv6_host = create_ipv6_host(ENET_PORT_ANY);
        int my_ipv6_port = 0;
        if (ipv6_host) {
            enet_host_compress_with_range_coder(ipv6_host);
            my_ipv6_port = (int)ipv6_host->address.port;
        }
        PunchAddresses punch_addresses;
        if (matchmaking_punch(join_lobby_id, (int)my_external_ipv4_port, my_ipv6_port, &punch_addresses) != 0) {
            LbNetLog("Join: matchmaking_punch failed\n");
            cleanup_join_host(ipv6_host, nullptr);
            host_destroy();
            return Lb_FAIL;
        }
        ENetAddress ipv4_address = {};
        ENetAddress ipv6_address = {};
        int has_ipv4 = resolve_punch_address(punch_addresses.ipv4, ENET_ADDRESS_TYPE_IPV4, punch_addresses.ipv4_port, &ipv4_address);
        int has_ipv6 = (ipv6_host != nullptr) && resolve_punch_address(punch_addresses.ipv6, ENET_ADDRESS_TYPE_IPV6, punch_addresses.ipv6_port, &ipv6_address);
        if (!has_ipv4 && !has_ipv6) {
            LbNetLog("Join: failed to resolve any peer address from punch\n");
            cleanup_join_host(ipv6_host, nullptr);
            host_destroy();
            return Lb_FAIL;
        }
        join_lobby_id[0] = '\0';
        if (skip_holepunch) {
            LbNetLog("Join: skipping hole-punch phase\n");
            cleanup_join_host(ipv6_host, nullptr);
            host_destroy();
            TbClockMSec direct_display_ms = 0;
            if (punch_addresses.ipv6[0] != '\0') {
                direct_display_ms += TIMEOUT_CONNECT_DIRECT_IPV6;
            }
            if (punch_addresses.ipv4[0] != '\0') {
                direct_display_ms += TIMEOUT_CONNECT_DIRECT_IPV4;
            }
            return join_direct_fallback(&punch_addresses, LbTimerClock() + direct_display_ms);
        }
        enet_host_compress_with_range_coder(host);
        if (has_ipv6) holepunch_punch_to(ipv6_host, &ipv6_address);
        if (has_ipv4) holepunch_punch_to(host, &ipv4_address);
        SDL_Delay(HOLEPUNCH_PRE_CONNECT_DELAY_MS);
        ENetPeer *ipv4_peer = nullptr;
        ENetPeer *ipv6_peer = nullptr;
        if (has_ipv6) {
            ipv6_peer = enet_host_connect(ipv6_host, &ipv6_address, NUM_CHANNELS, 0);
        } else if (has_ipv4) {
            ipv4_peer = enet_host_connect(host, &ipv4_address, NUM_CHANNELS, 0);
        }
        TbClockMSec connection_start = LbTimerClock();
        TbClockMSec holepunch_stage_start = connection_start;
        TbClockMSec holepunch_deadline = connection_start + TIMEOUT_CONNECT_HOLEPUNCH;
        TbClockMSec connection_deadline = connection_start + TIMEOUT_CONNECT_HOLEPUNCH
            + (has_ipv6 ? TIMEOUT_CONNECT_DIRECT_IPV6 : 0)
            + (has_ipv4 ? TIMEOUT_CONNECT_DIRECT_IPV4 : 0);
        TbClockMSec ipv4_delay_end = LbTimerClock() + HAPPY_EYEBALLS_DELAY_MS;
        ENetEvent enet_event;
        while (LbTimerClock() < holepunch_deadline) {
            if (has_ipv4 && ipv4_peer == nullptr && LbTimerClock() >= ipv4_delay_end) {
                ipv4_peer = enet_host_connect(host, &ipv4_address, NUM_CHANNELS, 0);
            }
            if (ipv6_peer && enet_host_service(ipv6_host, &enet_event, 0) > 0 && enet_event.type == ENET_EVENT_TYPE_CONNECT) {
                LbNetLog("Join: TIMEOUT_CONNECT_HOLEPUNCH took %d ms (connected)\n", (int)(LbTimerClock() - holepunch_stage_start));
                return finish_join(ipv6_host, ipv6_peer, host, nullptr, "matchmaking server", "IPv6");
            }
            if (ipv4_peer && enet_host_service(host, &enet_event, 0) > 0 && enet_event.type == ENET_EVENT_TYPE_CONNECT) {
                LbNetLog("Join: TIMEOUT_CONNECT_HOLEPUNCH took %d ms (connected)\n", (int)(LbTimerClock() - holepunch_stage_start));
                if (ipv6_peer)
                    LbNetLog("Join: IPv4 connected first, continuing over IPv4.\n");
                return finish_join(host, ipv4_peer, ipv6_host, ipv6_peer, "matchmaking server", "IPv4");
            }
            if (ipv6_peer) holepunch_punch_to(ipv6_host, &ipv6_address);
            if (ipv4_peer) holepunch_punch_to(host, &ipv4_address);
            TbClockMSec remaining = holepunch_deadline - LbTimerClock();
            enet_uint32 wait_ms = (enet_uint32)min((TbClockMSec)HOLEPUNCH_CONNECT_DELAY_MS, remaining);
            if (has_ipv4 && ipv4_peer == nullptr) {
                TbClockMSec time_to_ipv4 = ipv4_delay_end - LbTimerClock();
                if (time_to_ipv4 > 0)
                    wait_ms = (enet_uint32)min((TbClockMSec)wait_ms, time_to_ipv4);
            }
            SDL_Delay(wait_ms);
            display_attempting_to_join_message((int)((connection_deadline - LbTimerClock()) / 1000));
            if (attempting_to_join_cancel_requested()) {
                LbNetLog("Join: cancelled by user during hole-punch\n");
                cleanup_join_host(ipv6_host, ipv6_peer);
                host_destroy();
                return Lb_FAIL;
            }
        }
        LbNetLog("Join: TIMEOUT_CONNECT_HOLEPUNCH took %d ms (timed out)\n", (int)(LbTimerClock() - holepunch_stage_start));
        cleanup_join_host(ipv6_host, ipv6_peer);
        host_destroy();
        return join_direct_fallback(&punch_addresses, connection_deadline);
    }

    TbError bf_enet_join(const char *session, void *)
    {
        ENetAddress connect_address;
        TbClockMSec join_start_ms = LbTimerClock();
        if (strncmp(join_lobby_id, "LAN:", 4) == 0) {
            LbNetLog("Join: connecting via LAN\n");
            char lan_peer_address[MATCHMAKING_IP_MAX];
            snprintf(lan_peer_address, sizeof(lan_peer_address), "%s", join_lobby_id + 4);
            join_lobby_id[0] = '\0';
            char *port_separator = strrchr(lan_peer_address, ':');
            if (!port_separator)
                return Lb_FAIL;
            *port_separator = '\0';
            int lan_game_port = atoi(port_separator + 1);
            if (enet_address_set_host(&connect_address, ENET_ADDRESS_TYPE_IPV4, lan_peer_address) < 0) {
                LbNetLog("Join: failed to resolve LAN peer address %s\n", lan_peer_address);
                return Lb_FAIL;
            }
            connect_address.port = (enet_uint16)lan_game_port;
            if (create_join_host(ENET_ADDRESS_TYPE_IPV4) != Lb_OK)
                return Lb_FAIL;
        } else if (join_lobby_id[0] != '\0') {
            return join_via_holepunch(join_start_ms);
        } else {
            return join_direct_session(session, LbTimerClock() + TIMEOUT_CONNECT_DIRECT_IPV4, TIMEOUT_CONNECT_DIRECT_IPV4, "direct connect");
        }
        return connect_to_current_join_host(&connect_address, "LAN", LbTimerClock() + TIMEOUT_CONNECT_DIRECT_IPV4, TIMEOUT_CONNECT_DIRECT_IPV4);
    }

    /*
     * @returns -1 if error, +1 if there is a packet, 0 if timeoout or no events
     */
    int bf_enet_read_event(NetNewUserCallback new_user, uint timeout)
    {
        if (!host)
            return -1;
        ENetEvent enet_event;
        NetUserId user_id;
        int service_result = enet_host_service(host, &enet_event, timeout);
        if (service_result == 0)
            return 0;
        if (service_result < 0)
        {
            NETDBG(1, "enet_host -> %d", service_result);
            return service_result;
        }
        switch (enet_event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                LbNetLog("ENet: incoming connection accepted\n");
                if (new_user(&user_id)) {
                    enet_peer_timeout(enet_event.peer, 0, PEER_TIMEOUT_MIN_MS, PEER_TIMEOUT_MAX_MS);
                    enet_event.peer->data = reinterpret_cast<void *>(user_id);
                } else {
                    LbNetLog("ENet: rejecting peer, no user slot available\n");
                    enet_peer_disconnect_now(enet_event.peer, 0);
                }
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                user_id = NetUserId(reinterpret_cast<ptrdiff_t>(enet_event.peer->data));
                const char *disconnect_reason = "timed out";
                if (enet_event.type == ENET_EVENT_TYPE_DISCONNECT)
                    disconnect_reason = "disconnected (clean)";
                LbNetLog("ENet: peer %d %s\n", (int)user_id, disconnect_reason);
                g_drop_callback(user_id, NETDROP_ERROR);
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE:
                if (oldest_packet == nullptr)
                {
                    newest_packet = enet_event.packet;
                    oldest_packet = newest_packet;
                    incoming_queue_size = 1;
                }
                else
                {
                    newest_packet->userData = enet_event.packet;
                    newest_packet = enet_event.packet;
                    newest_packet->userData = nullptr;
                    incoming_queue_size += 1;
                    if (incoming_queue_size > 50)
                    {
                        fprintf(stderr, "Too many packets %d\n", incoming_queue_size);
                        WARNLOG("Too many packets %d", incoming_queue_size);
                    }
                }
                return 1;
            case ENET_EVENT_TYPE_NONE:
                break;
        }
        return 0;
    }

    /**
     * Checks for new connections.
     * @param new_user Call back if a new user has connected.
     */
    void bf_enet_update(NetNewUserCallback new_user)
    {
        int packets_read = 0;
        const int MAX_PACKETS_PER_UPDATE = 100;
        while (packets_read < MAX_PACKETS_PER_UPDATE && bf_enet_read_event(new_user, 0))
        {
            packets_read++;
        }
    }

    /**
     * Sends a message buffer to a certain user.
     * @param destination Destination user.
     * @param buffer
     * @param size Must be > 0
     */
    void bf_enet_sendmsg_single(NetUserId destination, const char *buffer, size_t size)
    {
        ENetPacket *packet = enet_packet_create(buffer, size, ENET_PACKET_FLAG_RELIABLE);
        if (client_peer) // Just send to server
        {
            enet_peer_send(client_peer, ENET_CHANNEL_RELIABLE, packet);
        }
        else
        {
            for (ENetPeer *currentPeer = host->peers; currentPeer < &host->peers[host -> peerCount]; ++currentPeer)
            {
                if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
                    continue;
                if (NetUserId(reinterpret_cast<ptrdiff_t>(currentPeer->data)) == destination)
                {
                    enet_peer_send(currentPeer, ENET_CHANNEL_RELIABLE, packet);
                }
            }
        }
        enet_host_flush(host);
    }

    /**
     * Sends a message buffer to a certain user using unsequenced delivery.
     * @param destination Destination user.
     * @param buffer
     * @param size Must be > 0
     */
    void bf_enet_sendmsg_single_unsequenced(NetUserId destination, const char *buffer, size_t size)
    {
        ENetPacket *packet = enet_packet_create(buffer, size, ENET_PACKET_FLAG_UNSEQUENCED);
        if (client_peer) // Just send to server
        {
            enet_peer_send(client_peer, ENET_CHANNEL_UNSEQUENCED, packet);
        }
        else
        {
            for (ENetPeer *currentPeer = host->peers; currentPeer < &host->peers[host -> peerCount]; ++currentPeer)
            {
                if (currentPeer->state != ENET_PEER_STATE_CONNECTED)
                    continue;
                if (NetUserId(reinterpret_cast<ptrdiff_t>(currentPeer->data)) == destination)
                {
                    enet_peer_send(currentPeer, ENET_CHANNEL_UNSEQUENCED, packet);
                }
            }
        }
        enet_host_flush(host);
    }

    /**
     * Sends a message buffer to all remote users.
     * @param buffer
     * @param size Must be > 0
     */
    void bf_enet_sendmsg_all(const char *buffer, size_t size)
    {
        ENetPacket *packet = enet_packet_create(buffer, size, ENET_PACKET_FLAG_RELIABLE);
        enet_host_broadcast(host, ENET_CHANNEL_RELIABLE, packet);
        enet_host_flush(host);
    }

    TbBool not_expected_user(NetUserId *)
    {
        ERRORLOG("Unexpected connected user\n");
        fprintf(stderr, "Unexpected connected user\n");
        return false;
    }
    /**
     * Completely reads a message. Blocks until entire message has been read.
     * Will not block if msgready has returned > 0.
     * @param source The source user.
     * @param buffer
     * @param max_size The maximum size of the message to be received.
     * @return The actual size of the message received, <= max_size. If 0, an
     *  error occurred.
     */
    size_t bf_enet_readmsg(NetUserId, char *buffer, size_t max_size)
    {
        while (!oldest_packet)
        {
            bf_enet_read_event(not_expected_user, 0);
        }
        ENetPacket *packet = oldest_packet;
        oldest_packet = static_cast<ENetPacket *>(oldest_packet->userData);
        incoming_queue_size--;

        size_t copy_size = min(packet->dataLength, max_size);
        memcpy(buffer, packet->data, copy_size);
        enet_packet_destroy(packet);
        return copy_size;
    }

    /**
     * Asks if a message has finished reception and get be read through readmsg.
     * May block under some circumstances but shouldn't unless it can be presumed
     * a whole message is on the way.
     * TODO NET this definition is due to how SDL Net handles sockets.. see if it can
     *  be improved - ideally this function shouldn't block at all
     * @param source The source user.
     * @param timeout If non-zero, this method will wait this number of milliseconds
     *  for a message to arrive before returning.
     * @return The size of the message waiting if there is a message, otherwise 0.
     */
    size_t bf_enet_msgready(NetUserId, unsigned timeout)
    {
        if (!oldest_packet)
        {
            bf_enet_read_event(not_expected_user, timeout);
        }
        if (oldest_packet)
            return oldest_packet->dataLength;
        return 0;
    }

    /**
     * Disconnects a user.
     * @param id User to be dropped.
     */
    void bf_enet_drop_user(NetUserId)
    {
        fprintf(stderr, "enet_drop_user not implemented\n");
        ERRORLOG("enet_drop_user not implemented");
    }

}

static bool IsPeerConnected(const ENetPeer *peer) {
    return peer && peer->state == ENET_PEER_STATE_CONNECTED;
}

static bool IsLocalPeer(NetUserId id) {
    return id == SERVER_ID || id == my_player_number;
}

static unsigned int ClampSizeToUInt(size_t value) {
    if (value > UINT_MAX) {
        return UINT_MAX;
    }
    return static_cast<unsigned int>(value);
}

unsigned long GetPing(NetUserId id) {
    const bool requesting_local_peer = IsLocalPeer(id);

    if (IsPeerConnected(client_peer)) {
        if (!requesting_local_peer) {
            return 0;
        }
        enet_uint32 value = client_peer->roundTripTime;
        if (value == 0) {
            value = client_peer->lastRoundTripTime;
        }
        return static_cast<unsigned long>(value);
    }

    if (!host) {
        return 0;
    }

    unsigned long best_value = 0;

    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }

        enet_uint32 peer_round_trip = peer->roundTripTime;
        if (peer_round_trip == 0) {
            peer_round_trip = peer->lastRoundTripTime;
        }
        unsigned long value = static_cast<unsigned long>(peer_round_trip);
        if (!requesting_local_peer) {
            NetUserId peer_id = NetUserId(reinterpret_cast<ptrdiff_t>(peer->data));
            if (peer_id == id) {
                return value;
            }
            continue;
        }
        if (value > best_value) {
            best_value = value;
        }
    }

    if (requesting_local_peer) {
        return best_value;
    }

    return 0;
}

unsigned long GetPingVariance(NetUserId id) {
    const bool requesting_local_peer = IsLocalPeer(id);

    if (IsPeerConnected(client_peer)) {
        if (!requesting_local_peer) {
            return 0;
        }
        enet_uint32 value = client_peer->roundTripTimeVariance;
        if (value == 0) {
            value = client_peer->lastRoundTripTimeVariance;
        }
        return static_cast<unsigned long>(value);
    }

    if (!host) {
        return 0;
    }

    unsigned long best_value = 0;

    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }

        enet_uint32 peer_round_trip_variance = peer->roundTripTimeVariance;
        if (peer_round_trip_variance == 0) {
            peer_round_trip_variance = peer->lastRoundTripTimeVariance;
        }
        unsigned long value = static_cast<unsigned long>(peer_round_trip_variance);
        if (!requesting_local_peer) {
            NetUserId peer_id = NetUserId(reinterpret_cast<ptrdiff_t>(peer->data));
            if (peer_id == id) {
                return value;
            }
            continue;
        }
        if (value > best_value) {
            best_value = value;
        }
    }

    if (requesting_local_peer) {
        return best_value;
    }

    return 0;
}

unsigned int GetPacketLoss(NetUserId id) {
    const bool requesting_local_peer = IsLocalPeer(id);

    if (IsPeerConnected(client_peer)) {
        if (!requesting_local_peer) {
            return 0;
        }
        enet_uint32 value = client_peer->packetLoss;
        unsigned int percent = static_cast<unsigned int>((static_cast<unsigned long long>(value) * 100ULL) / ENET_PEER_PACKET_LOSS_SCALE);
        return percent;
    }

    if (!host) {
        return 0;
    }

    unsigned int best_value = 0;

    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }

        enet_uint32 peer_packet_loss = peer->packetLoss;
        unsigned int value = static_cast<unsigned int>((static_cast<unsigned long long>(peer_packet_loss) * 100ULL) / ENET_PEER_PACKET_LOSS_SCALE);
        if (!requesting_local_peer) {
            NetUserId peer_id = NetUserId(reinterpret_cast<ptrdiff_t>(peer->data));
            if (peer_id == id) {
                return value;
            }
            continue;
        }
        if (value > best_value) {
            best_value = value;
        }
    }

    if (requesting_local_peer) {
        return best_value;
    }

    return 0;
}

unsigned int GetClientDataInTransit() {
    if (IsPeerConnected(client_peer)) {
        return client_peer->reliableDataInTransit;
    }

    if (!host) {
        return 0;
    }

    unsigned int result = 0;
    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }
        if (peer->reliableDataInTransit > result) {
            result = peer->reliableDataInTransit;
        }
    }
    return result;
}

unsigned int GetIncomingPacketQueueSize() {
    if (incoming_queue_size <= 0) {
        return 0;
    }
    return static_cast<unsigned int>(incoming_queue_size);
}

unsigned int GetClientPacketsLost() {
    if (IsPeerConnected(client_peer)) {
        return static_cast<unsigned int>(client_peer->packetsLost);
    }
    if (!host) {
        return 0;
    }
    unsigned long long total = 0;
    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }
        total += static_cast<unsigned long long>(peer->packetsLost);
        if (total > UINT_MAX) {
            return UINT_MAX;
        }
    }
    return static_cast<unsigned int>(total);
}

unsigned int GetClientOutgoingDataTotal() {
    if (IsPeerConnected(client_peer)) {
        return static_cast<unsigned int>(client_peer->outgoingDataTotal);
    }
    if (!host) {
        return 0;
    }
    unsigned long long total = 0;
    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }
        total += static_cast<unsigned long long>(peer->outgoingDataTotal);
        if (total > UINT_MAX) {
            return UINT_MAX;
        }
    }
    return static_cast<unsigned int>(total);
}

unsigned int GetClientIncomingDataTotal() {
    if (IsPeerConnected(client_peer)) {
        return static_cast<unsigned int>(client_peer->incomingDataTotal);
    }
    if (!host) {
        return 0;
    }
    unsigned long long total = 0;
    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }
        total += static_cast<unsigned long long>(peer->incomingDataTotal);
        if (total > UINT_MAX) {
            return UINT_MAX;
        }
    }
    return static_cast<unsigned int>(total);
}

unsigned int GetClientReliableCommandsInFlight() {
    if (IsPeerConnected(client_peer)) {
        size_t value = enet_list_size(&client_peer->sentReliableCommands);
        return ClampSizeToUInt(value);
    }
    if (!host) {
        return 0;
    }
    size_t best_value = 0;
    for (size_t peer_index = 0; peer_index < host->peerCount; ++peer_index) {
        ENetPeer *peer = &host->peers[peer_index];
        if (!IsPeerConnected(peer)) {
            continue;
        }
        size_t current = enet_list_size(&peer->sentReliableCommands);
        if (current > best_value) {
            best_value = current;
            if (best_value > UINT_MAX) {
                return UINT_MAX;
            }
        }
    }
    return ClampSizeToUInt(best_value);
}

uint16_t enet_get_bound_ipv6_port(void)
{
    if (!host_is_dual_stack || !host) {
        return 0;
    }
    return host->address.port;
}

void enet_matchmaking_host_update(void)
{
    if (!host)
        return;
    static ENetAddress pending_ipv4;
    static ENetAddress pending_ipv6;
    static int has_pending_ipv4 = 0;
    static int has_pending_ipv6 = 0;
    static TbClockMSec next_punch_time = 0;
    PunchAddresses punch_addresses;
    int new_punch = 0;
    if (matchmaking_poll_punch(&punch_addresses)) {
        new_punch = 1;
        has_pending_ipv4 = resolve_punch_address(punch_addresses.ipv4, ENET_ADDRESS_TYPE_IPV4, punch_addresses.ipv4_port, &pending_ipv4);
        has_pending_ipv6 = host_is_dual_stack && resolve_punch_address(punch_addresses.ipv6, ENET_ADDRESS_TYPE_IPV6, punch_addresses.ipv6_port, &pending_ipv6);
        next_punch_time = 0;
        if (has_pending_ipv4 || has_pending_ipv6)
            LbNetLog("Host: received punch ipv4=%s ipv6=%s ipv4_port=%d ipv6_port=%d\n", punch_addresses.ipv4, punch_addresses.ipv6, punch_addresses.ipv4_port, punch_addresses.ipv6_port);
        if (!has_pending_ipv6 && punch_addresses.ipv6[0] != '\0')
            LbNetLog("Host: IPv6 punch skipped (host is not dual-stack)\n");
    }
    if (!has_pending_ipv4 && !has_pending_ipv6) {
        next_punch_time = 0;
        return;
    }
    if (!new_punch) {
        for (ENetPeer *peer = host->peers; peer < &host->peers[host->peerCount]; peer++) {
            if (peer->state == ENET_PEER_STATE_CONNECTED)
                return;
        }
    }
    TbClockMSec now = LbTimerClock();
    if (!new_punch && now < next_punch_time)
        return;
    if (has_pending_ipv6)
        holepunch_punch_to(host, &pending_ipv6);
    if (has_pending_ipv4)
        holepunch_punch_to(host, &pending_ipv4);
    next_punch_time = now + HOLEPUNCH_CONNECT_DELAY_MS;
}

struct NetSP *InitEnetSP()
{
    static struct NetSP ret =
    {
            .init = &bf_enet_init,
            .exit = &bf_enet_exit,
            .host = &bf_enet_host,
            .join = &bf_enet_join,
            .update = &bf_enet_update,
            .sendmsg_single = &bf_enet_sendmsg_single,
            .sendmsg_single_unsequenced = &bf_enet_sendmsg_single_unsequenced,
            .sendmsg_all = &bf_enet_sendmsg_all,
            .msgready = &bf_enet_msgready,
            .readmsg = &bf_enet_readmsg,
            .drop_user = &bf_enet_drop_user,
    };
    return &ret;
}
