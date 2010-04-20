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
#include <SDL_net.h>
#include <string>

#include "bflib_network.h"

#define BROADCAST_ADDRESS				"255.255.255.255"
#define BROADCAST_PORT_NUMBER			17777 //replace later
#define HOST_PORT_NUMBER				17778 //replace later, dynamic ports are slightly more complicated but it can be done
#define MAX_BROADCAST_PACKET_SIZE		100
#define SESSION_BROADCAST_PERIOD		4000 //2 s
#define SESSION_IDLE_LISTEN_PERIOD		2000

const static std::string broadcastMsgPrefix("KEEPERFX_SESSION:");

TCPServiceProvider::ThreadCond::ThreadCond() :
		mutex(SDL_CreateMutex()),
		cond(SDL_CreateCond()),
		exit(false),
		locked(false)
{
}

TCPServiceProvider::ThreadCond::~ThreadCond()
{
	SDL_DestroyCond(cond);
	SDL_DestroyMutex(mutex);
}

void TCPServiceProvider::ThreadCond::reset()
{
	assert(!locked);
	exit = false;
}

void TCPServiceProvider::ThreadCond::signalExit()
{
	SDL_LockMutex(mutex);
	exit = true;
	SDL_UnlockMutex(mutex);
}
void TCPServiceProvider::ThreadCond::waitMs(unsigned long ms)
{
	assert(locked);
	SDL_CondWaitTimeout(cond, mutex, ms);
}

void TCPServiceProvider::ThreadCond::lock()
{
	assert(!locked);
	locked = true;
	SDL_LockMutex(mutex);
}

void TCPServiceProvider::ThreadCond::unlock()
{
	assert(locked);
	locked = false;
	SDL_UnlockMutex(mutex);
}

bool TCPServiceProvider::ThreadCond::shouldExit()
{
	assert(locked);
	return exit;
}

int TCPServiceProvider::listenForSessions(TCPServiceProvider * sp)
{
	// Create UDP socket.
	UDPsocket socket = SDLNet_UDP_Open(BROADCAST_PORT_NUMBER);
	if (socket == NULL) {
		NETMSG("Failed to open UDP socket: %s", SDLNet_GetError());
		return Lb_FAIL;
	}

	// Create packet.

	UDPpacket * const packet = SDLNet_AllocPacket(MAX_BROADCAST_PACKET_SIZE);
	if (packet == NULL) {
		NETMSG("Failed to create UDP packet: %s", SDLNet_GetError());
		SDLNet_UDP_Close(socket);
		return Lb_FAIL;
	}

	sp->sessionListenCond.lock();

	while (!sp->sessionListenCond.shouldExit()) {
		int result = SDLNet_UDP_Recv(socket, packet);
		if (result == 1) {
			//unlock to give signalExit() a chance in case we're receiving a large number of packets
			sp->sessionListenCond.unlock();

			if (packet->len > broadcastMsgPrefix.size() &&
					memcmp(packet->data, broadcastMsgPrefix.c_str(), broadcastMsgPrefix.size()) == 0) {

				IPaddress addr;
				addr.host = packet->address.host;
				SDLNet_Write16(HOST_PORT_NUMBER, &addr.port);
				packet->data[packet->len - 1] = 0; //prevent buffer overflow on invalid data

				if (strlen(reinterpret_cast<char *>(packet->data)) > broadcastMsgPrefix.size()) {
					SDL_LockMutex(sp->sessionsMutex);
					sp->reportSession(addr, reinterpret_cast<char *>(packet->data) + broadcastMsgPrefix.size());
					SDL_UnlockMutex(sp->sessionsMutex);
				}
			}

			sp->sessionListenCond.lock();
		}
		else {
			if (result != 0) {
				NETMSG("UDP receive error");
			}

			sp->sessionListenCond.waitMs(SESSION_IDLE_LISTEN_PERIOD);
		}
	}

	sp->sessionListenCond.unlock();

	SDLNet_FreePacket(packet);
	SDLNet_UDP_Close(socket);

	return 0;
}

int TCPServiceProvider::broadcastSession(TCPServiceProvider * sp)
{
	// Create UDP socket.

	UDPsocket socket = SDLNet_UDP_Open(0);
	if (socket == NULL) {
		NETMSG("Failed to open UDP socket: %s", SDLNet_GetError());
		return Lb_FAIL;
	}

	// Create packet.

	UDPpacket * const packet = SDLNet_AllocPacket(MAX_BROADCAST_PACKET_SIZE);
	if (packet == NULL) {
		NETMSG("Failed to create UDP packet: %s", SDLNet_GetError());
		SDLNet_UDP_Close(socket);
		return Lb_FAIL;
	}

	sp->sessionBroadcastCond.lock();

	while (!sp->sessionBroadcastCond.shouldExit()) {
		//don't bother unlocking sessionBroadcastCond.mutex... not an issue here given code should return quickly

		// Get local session name.

		SDL_LockMutex(sp->sessionsMutex);

		int mySessionIndex = sp->SessionIndex(sp->local_id);
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
		}

		sp->sessionBroadcastCond.waitMs(SESSION_BROADCAST_PERIOD);
	}

	sp->sessionBroadcastCond.unlock();

	SDLNet_FreePacket(packet);
	SDLNet_UDP_Close(socket);

	return 0;
}

TbNetworkSessionNameEntry * TCPServiceProvider::reportSession(const IPaddress & addr, const char * namestr)
{
	//assumes sessionsMutex is held

	SYNCDBG(7, "Starting with session name %s", namestr);

	TbNetworkSessionNameEntry * session = findSessionByAddress(addr);
	int index;

	if (session == NULL) {
		session = AddSession(static_cast<unsigned long>(-1), namestr);

		if (session == NULL) {
			return NULL;
		}

		index = SessionIndex(session->id);
		assert(index >= 0);

		memcpy(&sessionAddrTable[index].addr, &addr, sizeof(addr));
	}
	else {
		index = SessionIndex(session->id);
		assert(index >= 0);
	}

	sessionAddrTable[index].lastUpdate = SDL_GetTicks();

	return session;
}

TbNetworkSessionNameEntry * TCPServiceProvider::findSessionByAddress(const IPaddress & addr)
{
	//assumes sessionsMutex is held

	SYNCDBG(7, "Starting");

	for (int i = 0; i < SESSION_ENTRIES_COUNT; ++i) {
		if (i == local_id) {
			continue;
		}

		if (sessionAddrTable[i].addr.host == addr.host && sessionAddrTable[i].addr.port == addr.port) {
			int index = SessionIndex(sessionAddrTable[i].sessionId);
			if (index >= 0) {
				return &nsnames[index];
			}
			else {
				return NULL;
			}
		}
	}

	SYNCDBG(7, "No matching session entry found");

	return NULL;
}

TCPServiceProvider::TCPServiceProvider() :
	sessionsMutex(SDL_CreateMutex()),
	sessionListenThread(NULL),
	sessionBroadcastThread(NULL),
	sessionListenCond(),
	sessionBroadcastCond()
{

}

TCPServiceProvider::~TCPServiceProvider()
{
	assert(sessionListenThread == NULL);
	assert(sessionBroadcastThread == NULL);

	SDL_DestroyMutex(sessionsMutex);
}

TbError TCPServiceProvider::Start(struct TbNetworkSessionNameEntry * sessionName, char * playerName, void * options) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::Start(char * sessionName, char * playerName, unsigned long maxPlayers, void * options) {
	SYNCDBG(7, "Starting");

	SDL_LockMutex(sessionsMutex);

	NETMSG("Starting new session with name %s", sessionName);
	TbNetworkSessionNameEntry * const session = AddSession(static_cast<unsigned long>(-1), sessionName);
	local_id = session->id;

	SDL_UnlockMutex(sessionsMutex);

	return Lb_OK;
}

TbError TCPServiceProvider::Stop(void) {
	SYNCDBG(7, "Starting");

	SDL_LockMutex(sessionsMutex);

	int index = SessionIndex(local_id);
	if (index >= 0) {
		nsnames[index].in_use = false;
	}
	local_id = 0;

	SDL_UnlockMutex(sessionsMutex);

	return Lb_OK;
}


TbError TCPServiceProvider::Enumerate(TbNetworkCallbackFunc sessionCallback, void * ptr) {
	SYNCDBG(7, "Starting");

	// Check sessions list.

	SDL_LockMutex(sessionsMutex);

	for (int i = 0; i < SESSION_ENTRIES_COUNT; ++i) {
		TbNetworkSessionNameEntry & session = nsnames[i];

		if (session.in_use && session.id != local_id) {
			sessionCallback(reinterpret_cast<TbNetworkCallbackData *>(&session), NULL);
		}
	}

	SDL_UnlockMutex(sessionsMutex);

	return Lb_OK;
}

TbError TCPServiceProvider::Enumerate(struct TbNetworkSessionNameEntry * sessionEntry,
		TbNetworkCallbackFunc playerCallback, void * ptr) {
	//need to fool game this is working to test
	TbNetworkPlayerNameEntry entry;
	memset(&entry, 0, sizeof(entry));
	strcpy(entry.name, "goatman");
	entry.id = 44;
	entry.isHost = true;
	entry.isLocal = true;
	playerCallback((TbNetworkCallbackData*) &entry, 0);
	return Lb_OK;
}

TbError TCPServiceProvider::Init(struct _GUID a1, struct _GUID * a2, struct ReceiveCallbacks * recCb, void * a4) {
	SYNCDBG(7, "Starting");

	TbError retval = ServiceProvider::Initialise(recCb, a4);
	if (retval != Lb_OK) {
		return retval;
	}

	for (int i = 0; i < SESSION_ENTRIES_COUNT; ++i) {
		sessionAddrTable[i].sessionId = 0;
	}

	sessionListenCond.reset();
	sessionListenThread = SDL_CreateThread(reinterpret_cast<int (*)(void *)>(listenForSessions), this);

	sessionBroadcastCond.reset();
	sessionBroadcastThread = SDL_CreateThread(reinterpret_cast<int (*)(void *)>(broadcastSession), this);

	if (sessionListenThread == NULL || sessionBroadcastThread == NULL) {
		ERRORLOG("Failure to create session threads");
		Release();
		return Lb_FAIL;
	}

	return Lb_OK;
}

TbError TCPServiceProvider::Release(void)
{
	SYNCDBG(7, "Starting");

	sessionBroadcastCond.signalExit();
	sessionListenCond.signalExit();
	SDL_WaitThread(sessionBroadcastThread, NULL);
	SDL_WaitThread(sessionListenThread, NULL);
	sessionBroadcastThread = NULL;
	sessionListenThread = NULL;

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
