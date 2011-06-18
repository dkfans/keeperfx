/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_nethost_udp.cpp
 *     UDP_NetHost class implementation.
 * @par Purpose:
 *     UDP_NetHost: Handles LAN discovery of server through UDP broadcasts and
 *     answers requests for server info.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 April 2010 - ?
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "bflib_nethost_udp.hpp"
#include "bflib_netconfig.hpp"

const int MAX_PACKET_SIZE = 400;

UDP_NetHost::UDP_NetHost(const StringVector & broadcastAddresses) :
	thread(NULL),
	cond(),
	broadcastAddr(broadcastAddresses),
	errorFlag(false)
{
	thread = SDL_CreateThread(reinterpret_cast<int (*)(void *)>(threadFunc), this);
	if (thread == NULL) {
		ERRORLOG("Failure to create session host thread");
		errorFlag = true; //would be better with exception handling but...
	}
}

UDP_NetHost::~UDP_NetHost()
{
	cond.signalExit();
	SDL_WaitThread(thread, NULL);
}

void UDP_NetHost::threadFunc(UDP_NetHost * sh)
{
	// Create UDP socket.

	UDPsocket socket = SDLNet_UDP_Open(HOST_PORT_NUMBER);
	if (socket == NULL) {
		NETMSG("Failed to open UDP socket: %s", SDLNet_GetError());
		return;
	}

	// Create packet.

	UDPpacket * const packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
	if (packet == NULL) {
		NETMSG("Failed to create UDP packet: %s", SDLNet_GetError());
		SDLNet_UDP_Close(socket);
		return;
	}

	sh->cond.lock();

	while (!sh->cond.shouldExit()) {
		//don't bother unlocking sessionBroadcastCond.mutex... not an issue here given code should return quickly

		// Get local session name.

		/*SDL_LockMutex(sh->sessionsMutex);

		int mySessionIndex = sh->SessionIndex(sh->local_id);
		std::string mySessionName;

		if (mySessionIndex >= 0) {
			mySessionName = sp->nsnames[mySessionIndex].text;
		}

		SDL_UnlockMutex(sp->sessionsMutex);

		// We can now send session broadcast packet.

		if (mySessionIndex >= 0) {

			// Fill in broadcast packet.

			std::string msg = broadcastMsgPrefix + mySessionName;
			packet->len = msg.size() + 1;
			assert(packet->len <= MAX_BROADCAST_PACKET_SIZE);

			SDLNet_ResolveHost(&packet->address, BROADCAST_ADDRESS, BROADCAST_PORT_NUMBER);
			packet->channel = -1;
			strcpy(reinterpret_cast<char *>(packet->data), msg.c_str());

			// Send packet.

			if (SDLNet_UDP_Send(socket, -1, packet) <= 0) {
				NETMSG("Failed to send broadcast packet");
			}
		}*/

		sh->cond.waitMs(SESSION_HOST_PERIOD);
	}

	sh->cond.unlock();

	SDLNet_FreePacket(packet);
	SDLNet_UDP_Close(socket);
}
