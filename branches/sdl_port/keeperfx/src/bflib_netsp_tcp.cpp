/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsp_tcp.cpp
 *     TCP network ServiceProvider subclass declaration.
 * @par Purpose:
 *     Defines ServiceProvider for TCP network with UDP LAN discovery.
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

#include "bflib_ipconfig.hpp"
#include "bflib_network.h"
#include "bflib_nethost_udp.hpp"
#include "bflib_netlisten_udp.hpp"

TCPServiceProvider::TCPServiceProvider() :
		host(NULL),
		listener(NULL)
{

}

TCPServiceProvider::~TCPServiceProvider()
{
}

TbError TCPServiceProvider::Start(struct TbNetworkSessionNameEntry * sessionName, char * playerName, void * options) {
	memset(sockets, 0, sizeof(sockets));
	ClearPlayers();
	isHost = false;

	IPaddress addr;
	SDLNet_ResolveHost(&addr, "localhost", HOST_PORT_NUMBER); //TODO: change from local host to real session later
	sockets[0] = SDLNet_TCP_Open(&addr);
	if (sockets[0] == NULL) {
		NETMSG("Failed to initialize TCP client socket");
		return Lb_FAIL;
	}

	//TODO: see if we're responsible for adding ourself to player list or not

	//TODO: see if local_id must be set

	return Lb_OK;
}

TbError TCPServiceProvider::Start(char * sessionName, char * playerName, unsigned long maxPlayers, void * options) {
	SYNCDBG(7, "Starting");

	assert(maxPlayers >= 1);

	memset(sockets, 0, sizeof(sockets));
	ClearPlayers();
	isHost = true;
	this->maxPlayers = maxPlayers;

	// Open server socket (as player 0's socket):
	IPaddress addr;
	addr.host = INADDR_ANY;
	SDLNet_Write16(HOST_PORT_NUMBER, &addr.port);
	sockets[0] = SDLNet_TCP_Open(&addr);
	if (sockets[0] == NULL) {
		NETMSG("Failed to initialize TCP server socket");
		return Lb_FAIL;
	}

	// Fill in other player info.
	AddPlayer(nextPlayerId++, playerName, 0, 0);

	//TODO: make broadcast addresses configurable
	std::vector<std::string> broadcastAddr;
	broadcastAddr.push_back("255.255.255.255"); //global LAN broadcast addr
	broadcastAddr.push_back("192.168.0.255"); //common LAN broadcast addr
	broadcastAddr.push_back("5.255.255.255"); //Hamachi LAN
	host = new UDP_NetHost(broadcastAddr);

	/*NETMSG("Starting new session with name %s", sessionName);
	TbNetworkSessionNameEntry * const session = AddSession(static_cast<unsigned long>(-1), sessionName);
	local_id = session->id;*/
	local_id = 1;

	return Lb_OK;
}

TbError TCPServiceProvider::Stop(void) {
	SYNCDBG(7, "Starting");

	for (int i = NETSP_PLAYERS_COUNT - 1; i >= 0; --i) {
		if (sockets[i] != NULL) {
			SDLNet_TCP_Close(sockets[i]);
			sockets[i] = NULL;
		}
	}

	/*int index = SessionIndex(local_id);
	if (index >= 0) {
		nsnames[index].in_use = false;
	}*/
	local_id = 0;

	delete host;
	host = NULL;

	return Lb_OK;
}


TbError TCPServiceProvider::Enumerate(TbNetworkCallbackFunc sessionCallback, void * ptr) {
	SYNCDBG(7, "Starting");

	// Check sessions list.

	/*SDL_LockMutex(sessionsMutex);

	for (int i = 0; i < SESSION_ENTRIES_COUNT; ++i) {
		TbNetworkSessionNameEntry & session = nsnames[i];

		if (session.in_use && session.id != local_id) {
			sessionCallback(reinterpret_cast<TbNetworkCallbackData *>(&session), ptr);
		}
	}

	SDL_UnlockMutex(sessionsMutex);*/

	//for now, fake 1 session for testing
	nsnames[0].in_use = true;
	nsnames[0].joinable = true;
	net_copy_name_string(nsnames[0].text, "TESTING", SESSION_NAME_MAX_LEN);
	sessionCallback(reinterpret_cast<TbNetworkCallbackData *>(&nsnames[0]), ptr);

	return Lb_OK;
}

TbError TCPServiceProvider::Enumerate(struct TbNetworkSessionNameEntry * sessionEntry,
		TbNetworkCallbackFunc playerCallback, void * ptr) {

	if (sessionEntry == NULL || sessionEntry->id == local_id) {
		for (int i = 0; i < players_count; ++i) {
			TbNetworkPlayerNameEntry entry;
			memset(&entry, 0, sizeof(entry));
			if (i == 0) {
				entry.isHost = isHost;
				entry.isLocal = true; //I hope
			}

			memcpy(entry.name, players[i].name, NET_PLAYER_NAME_LENGTH);
			entry.id = players[i].id;

			playerCallback(reinterpret_cast<TbNetworkCallbackData *>(&entry), ptr);
		}
	}
	else {
		//TODO: look up remote session
	}

	return Lb_OK;
}

TbError TCPServiceProvider::Init(struct _GUID a1, struct _GUID * a2, struct ReceiveCallbacks * recCb, void * a4) {
	SYNCDBG(7, "Starting");

	TbError retval = ServiceProvider::Initialise(recCb, a4);
	if (retval != Lb_OK) {
		return retval;
	}

	listener = new UDP_NetListener();

	return Lb_OK;
}

TbError TCPServiceProvider::Release(void)
{
	SYNCDBG(7, "Starting");

	delete host;
	host = NULL;
	delete listener;
	listener = NULL;

	return ServiceProvider::Release();
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
