/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/**
 * @file net_portforward.h
 *     Port forwarding support using NAT-PMP and UPnP.
 * @par Purpose:
 *     Automatic port forwarding for multiplayer hosting.
 *     Tries NAT-PMP first (simpler/faster), falls back to UPnP.
 * @author   KeeperFX Team
 * @date     01 Jan 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef NET_PORTFORWARD_H
#define NET_PORTFORWARD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int port_forward_add_mapping(uint16_t port);
void port_forward_remove_mapping(void);

#ifdef __cplusplus
}
#endif

#endif
