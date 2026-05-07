/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/**
 * @file net_holepunch.h
 *     UDP hole punching via STUN.
 * @par Purpose:
 *     Sends a STUN Binding Request from the ENet host socket to discover the
 *     external address and create a NAT mapping.  Also provides a helper to
 *     send pre-connect punch packets when joining.
 * @author   KeeperFX Team
 * @date     06 Mar 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef NET_HOLEPUNCH_H
#define NET_HOLEPUNCH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _ENetHost;
struct _ENetAddress;

uint16_t holepunch_stun_query(struct _ENetHost *host, char *output_ip, size_t output_ip_buffer_size);
void holepunch_punch_to(struct _ENetHost *host, const struct _ENetAddress *target);

#ifdef __cplusplus
}
#endif

#endif
