/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netlisten_udp.cpp
 *     UDP_NetListener class implementation.
 * @par Purpose:
 *     UDP_NetListener: Maintains a list of known hosts and periodically
 *     updates them. Hosts can be added automatically via UDP LAN broadcasting
 *     or manually.
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

#include "bflib_netlisten_udp.hpp"
#include "bflib_netconfig.hpp"

const int MAX_PACKET_SIZE = 400;

UDP_NetListener::UDP_NetListener() :
	thread(NULL),
	cond(),
	sessions(),
	sessionsMutex(SDL_CreateMutex()),
	criticalError(false)
{
	thread = SDL_CreateThread(reinterpret_cast<int (*)(void *)>(threadFunc), this);
	if (thread == NULL) {
		ERRORLOG("Failure to create session listener thread");
		criticalError = true; //would be better with exception handling but...
	}
}

UDP_NetListener::~UDP_NetListener()
{
	cond.signalExit();
	SDL_WaitThread(thread, NULL);
	SDL_DestroyMutex(sessionsMutex);
}

void UDP_NetListener::threadFunc(UDP_NetListener * sh)
{
	// Create UDP socket.
	UDPsocket socket = SDLNet_UDP_Open(LISTENER_PORT_NUMBER);
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
		int result = SDLNet_UDP_Recv(socket, packet);
		if (result == 1) {
			//unlock to give signalExit() a chance in case we're receiving a large number of packets
			sh->cond.unlock();

			/*if (packet->len > broadcastMsgPrefix.size() &&
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
			}*/

			sh->cond.lock();
		}
		else {
			if (result != 0) {
				NETMSG("UDP receive error");
			}

			sh->cond.waitMs(SESSION_LISTENER_PERIOD);
		}
	}

	sh->cond.unlock();

	SDLNet_FreePacket(packet);
	SDLNet_UDP_Close(socket);
}

TbNetworkSessionNameEntry * UDP_NetListener::reportSession(const IPaddress & addr, const char * namestr)
{
	/*//assumes sessionsMutex is held

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

		memcpy(&sessions[index].addr, &addr, sizeof(addr));
	}
	else {
		index = SessionIndex(session->id);
		assert(index >= 0);
	}

	sessions[index].lastUpdate = SDL_GetTicks();

	return session;*/
	return NULL;
}

TbNetworkSessionNameEntry * UDP_NetListener::findSessionByAddress(const IPaddress & addr)
{
	/*//assumes sessionsMutex is held

	SYNCDBG(7, "Starting");

	for (int i = 0; i < SESSION_ENTRIES_COUNT; ++i) {
		if (i == local_id) {
			continue;
		}

		if (sessions[i].addr.host == addr.host && sessions[i].addr.port == addr.port) {
			int index = SessionIndex(sessions[i].sessionId);
			if (index >= 0) {
				return &nsnames[index];
			}
			else {
				return NULL;
			}
		}
	}

	SYNCDBG(7, "No matching session entry found");*/

	return NULL;
}

void UDP_NetListener::addHost(const IPaddress & addr)
{
	SDL_LockMutex(sessionsMutex);

	//prefer using an unused entry prior to starting a new one; also see if host already is in list
	SessionVector::iterator unusedIt = sessions.end();
	bool alreadyExists = false;
	for (SessionVector::iterator it = sessions.begin(), end = unusedIt; it != end; ++it) {
		if (it->used) {
			if (it->addr.host = addr.host && it->addr.port == addr.port) {
				alreadyExists = true;
				unusedIt = it;
				break;
			}
		}
		else {
			unusedIt = it;
		}
	}

	if (!alreadyExists) {
		if (unusedIt == sessions.end()) {
			sessions.push_back(Session(addr, "???"));
			unusedIt = --sessions.end(); //I think it already does but better avoid a potential bug if I'm incorrect...
		}
		else {
			*unusedIt = Session(addr, "???");
		}
	}

	unusedIt->removable = false;

	SDL_UnlockMutex(sessionsMutex);
}

void UDP_NetListener::removeHost(const IPaddress & addr)
{
	SDL_LockMutex(sessionsMutex);

	for (SessionVector::iterator it = sessions.begin(), end = sessions.end(); it != end; ++it) {
		if (it->addr.host = addr.host && it->addr.port == addr.port) {
			it->removable = true;
			break;
		}
	}

	SDL_UnlockMutex(sessionsMutex);
}

IPaddress UDP_NetListener::getSessionAddress(int sessionId)
{
	IPaddress addr;
	SDLNet_ResolveHost(&addr, "localhost", HOST_PORT_NUMBER); //for now return localhost until class is finshed
	return addr;
}
