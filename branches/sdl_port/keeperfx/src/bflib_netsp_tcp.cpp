/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsp_tcp.cpp
 *     TCP network ServiceProvider subclass declaration.
 * @par Purpose:
 *     Defines ServiceProvider for TCP network.
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

#include "bflib_netsp_tcp.hpp"

#include <cassert>
#include <SDL_net.h>
#include <string>

#define BROADCAST_PORT_NUMBER 7777 //replace later
#define MAX_PACKET_SIZE 200

const static std::string broadcastMsg("KEEPERFX ENUMHOSTS");

TCPServiceProvider::TCPServiceProvider() {

}

TCPServiceProvider::~TCPServiceProvider() {

}

TbError TCPServiceProvider::Start(struct TbNetworkSessionNameEntry * sessionName, char * playerName, void * options) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::Start(char * sessionName, char * playerName, unsigned long maxPlayers, void * options) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::Stop(void) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::Enumerate(TbNetworkCallbackFunc sessionCallback, void * ptr) {
	SYNCDBG(7, "Starting");

	ClearSessions();

	// Create UDP socket.

	UDPsocket socket = SDLNet_UDP_Open(0);
	if (socket == NULL) {
		NETMSG("Failed to open UDP socket: %s", SDLNet_GetError());
		return Lb_FAIL;
	}

	// Create packet.

	UDPpacket * const packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
	if (packet == NULL) {
		NETMSG("Failed to create UDP packet: %s", SDLNet_GetError());
		SDLNet_UDP_Close(socket);
		return Lb_FAIL;
	}

	// Fill in ENUMHOSTS packet.

	packet->len = broadcastMsg.size() + 1;
	assert(packet->len <= MAX_PACKET_SIZE);

	packet->address.host = INADDR_BROADCAST;
	packet->address.port = BROADCAST_PORT_NUMBER;
	packet->channel = -1;
	strcpy((char*) packet->data, broadcastMsg.c_str());

	// Send ENUMHOSTS packet.

	if (SDLNet_UDP_Send(socket, -1, packet) <= 0) {
		NETMSG("Failed to send ENUMHOSTS packet");
	}

	//TODO: listen for replies

	// Clean up.

	SDLNet_FreePacket(packet);
	SDLNet_UDP_Close(socket);

	return Lb_OK;
}

TbError TCPServiceProvider::Enumerate(struct TbNetworkSessionNameEntry * sessionEntry,
		TbNetworkCallbackFunc playerCallback, void * ptr) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::Init(struct _GUID a1, struct _GUID * a2, struct ReceiveCallbacks * recCb, void * a4) {
	Initialise(recCb, a4);
	return Lb_OK;
}

TbError TCPServiceProvider::Release(void) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::ChangeSettings(unsigned long, void *) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::EnableNewPlayers(TbBool allow) {
	return Lb_FAIL;
}

unsigned long TCPServiceProvider::ReadMessage(unsigned long *, void *, unsigned long *) {
	return 0;
}

unsigned long TCPServiceProvider::PeekMessage(unsigned long *, void *, unsigned long *) {
	return 0;
}

TbError TCPServiceProvider::SendMessage(unsigned long, void *, unsigned char) {
	return Lb_FAIL;
}
