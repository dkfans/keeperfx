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
#include <stdio>

#include "bflib_netconfig.hpp"
#include "bflib_network.h"
#include "bflib_nethost_udp.hpp"
#include "bflib_netlisten_udp.hpp"
#include "bflib_client_tcp.hpp"
#include "bflib_server_tcp.hpp"

#define SORC(x) (x? "Server" : "Client") //"server or client"

TCPServiceProvider::TCPServiceProvider() :
		host(NULL),
		listener(NULL),
		base(NULL)
{
}

TCPServiceProvider::~TCPServiceProvider()
{
}

TbError TCPServiceProvider::Start(struct TbNetworkSessionNameEntry * sessionName, char * playerName, void * options)
{
	SYNCDBG(7, "Starting");
	assert(!started);

	isServer = false;

	ClearPlayers();

	base = new TCP_NetClient("localhost", HOST_PORT_NUMBER);

	if (base == NULL || base->hadError()) {
		delete base;
		base = NULL;
		return Lb_FAIL;
	}

	//TODO: see if we're responsible for adding ourself to player list or not
	localPlayerId = PLAYERID_UNASSIGNED; //until assigned other id

	started = true;

	return Lb_OK;
}

TbError TCPServiceProvider::Start(char * sessionName, char * playerName, unsigned long maxPlayers, void * options)
{
	SYNCDBG(7, "Starting");
	assert(!started);
	assert(maxPlayers >= 2);
	assert(maxPlayers <= NETSP_PLAYERS_COUNT);

	isServer = true;
	localPlayerId = PLAYERID_HOSTDEFAULT;

	ClearPlayers();
	joinable = true;
	this->maxPlayers = maxPlayers;

	base = new TCP_NetServer(HOST_PORT_NUMBER, maxPlayers, localPlayerId);

	if (base == NULL || base->hadError()) {
		delete base;
		base = NULL;
		return Lb_FAIL;
	}

	// Fill in player info
	AddPlayer(localPlayerId++, playerName, 0, 0);

	//TODO: make broadcast addresses configurable
	std::vector<std::string> broadcastAddr;
	broadcastAddr.push_back("255.255.255.255"); //global LAN broadcast addr
	broadcastAddr.push_back("192.168.0.255"); //common LAN broadcast addr
	broadcastAddr.push_back("5.255.255.255"); //Hamachi LAN
	host = new UDP_NetHost(broadcastAddr);

	/*NETMSG("Starting new session with name %s", sessionName);
	TbNetworkSessionNameEntry * const session = AddSession(static_cast<unsigned long>(-1), sessionName);*/

	started = true;

	return Lb_OK;
}

TbError TCPServiceProvider::Stop(void)
{
	SYNCDBG(7, "Starting");

	/*int index = SessionIndex(local_id);
	if (index >= 0) {
		nsnames[index].in_use = false;
	}*/

	delete host;
	host = NULL;

	delete base;
	base = NULL;

	started = false;

	return Lb_OK;
}

TbError TCPServiceProvider::Enumerate(TbNetworkCallbackFunc sessionCallback, void * ptr)
{
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

	SYNCDBG(7, "Starting");

	if (sessionEntry == NULL) {
		for (int i = 0; i < players_count; ++i) {
			TbNetworkPlayerNameEntry entry;
			memset(&entry, 0, sizeof(entry));
			if (i == 0) {
				entry.isHost = isServer; //TODO: check correctness of this... was written for old code
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

	if (started) {
		Stop();
	}

	delete listener;
	listener = NULL;

	return ServiceProvider::Release();
}

TbError TCPServiceProvider::ChangeSettings(unsigned long, void *) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::EnableNewPlayers(TbBool allow) {
	joinable = allow;
	if (base != NULL) {
		base->setServerOptions(joinable);
	}
	return Lb_OK;
}

bool TCPServiceProvider::ReadMessage(ulong * playerId, void * msg, ulong * len) {
	assert(started);

	size_t len2 = *len;
	bool retval = base->fetchDKMessage(*playerId, reinterpret_cast<char *>(msg), len2, false);
	*len = len2;

	if (retval) {
		NETDBG(6, "%s: Message read from player %d: %d bytes", SORC(isServer), *playerId, *len);
	}
	else {
		NETDBG(6, "%s: No message read", SORC(isServer));
	}

	return retval;
}

bool TCPServiceProvider::PeekMessage(ulong * playerId, void * msg, ulong * len) {
	assert(started);

	size_t len2 = *len;
	bool retval = base->fetchDKMessage(*playerId, reinterpret_cast<char *>(msg), len2, true);
	*len = len2;

	if (retval) {
		NETDBG(6, "%s: Peek at message from player %d: %d bytes", SORC(isServer), *playerId, *len);

		/*if (*len <= 16) {
			char buf[1024];
			char * next = buf;
			for (int i = 0; i < *len; ++i) {
				next += sprintf(next, "%x ", ((char*) msg)[i]);
			}
			NETDBG(6, "Bytes: %s", buf);
		}*/
	}
	//failure to peek is common so don't display a message

	return retval;
}

TbError TCPServiceProvider::SendMessage(ulong playerId, void * msg, uchar i) {
	assert(started);

	NETDBG(6, "%s: Sending message to player %d", SORC(isServer), playerId);

	ulong totalMsgLen;
	ServiceProvider::DecodeMessageStub(msg, &totalMsgLen, NULL, NULL);
	totalMsgLen += 4; //include header

	if (!base->sendDKMessage(playerId, reinterpret_cast<char *>(msg), totalMsgLen)) {
		NETDBG(6, "%s: Failure to send message", SORC(isServer));
		return Lb_FAIL;
	}

	return Lb_OK;
}

void TCPServiceProvider::update()
{
	if (started) {
		base->update();
	}
}
