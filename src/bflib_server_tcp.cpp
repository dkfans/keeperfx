/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_server_tcp.cpp
 *     TCP server class declaration.
 * @par Purpose:
 *     Defines TCP server class.
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
#include "pre_inc.h"
#include "bflib_server_tcp.hpp"
#include "post_inc.h"

TCP_NetServer::TCP_NetServer(ushort port) : TCP_NetBase(),
		remoteMutex(SDL_CreateMutex())
{
	memset(&remote, 0, sizeof(remote));

	IPaddress addr;
	addr.host = INADDR_ANY;
	SDLNet_Write16(port, &addr.port);
	mySocket = SDLNet_TCP_Open(&addr);
	if (mySocket == NULL) {
		NETMSG("Failed to initialize TCP server socket");
		setErrorFlag();
		return;
	}
}

TCP_NetServer::~TCP_NetServer()
{
	haltRecvThreads();
	SDL_DestroyMutex(remoteMutex);
}

void TCP_NetServer::recvThreadFunc(RecvThreadArg * arg)
{
	TCP_NetServer * svr = arg->svr;
	int remoteIndex = arg->remoteIndex;
	delete arg; //so we don't forget later

	for (;;) {
		ulong player;
		TCPsocket sock = svr->getRemoteSocketByIndex(remoteIndex, player);

		char header[TCP_HEADER_SIZE];
		if (!receiveOnSocket(sock, header, sizeof(header))) {
			NETMSG("TCP send error, will drop remote client");
			svr->removeRemoteSocket(sock);
			break;
		}

		ulong playerId = SDLNet_Read32(header);
		ulong msgDataLen = SDLNet_Read32(header);
		if (true /*playerId == sp->localPlayerId*/) { //TODO NET handle this
			//message is for us
		}
		else {
			TCPsocket sendSock = svr->getRemoteSocketByPlayer(playerId);

			//pass it on
			const size_t bufferLen = sizeof(header) + msgDataLen;
			char * const buffer = reinterpret_cast<char *> (malloc(bufferLen));
			if (buffer == NULL) {
				ERRORLOG("Memory alloc error");
				break;
			}

			memcpy(buffer, header, sizeof(header));
			if (!receiveOnSocket(sock, buffer + sizeof(header), msgDataLen)) {
				free(buffer);
				svr->removeRemoteSocket(sock);
				break;
			}

			if (SDLNet_TCP_Send(sendSock, buffer, bufferLen) < bufferLen) {
				svr->removeRemoteSocket(sendSock);
				NETMSG("TCP send error, will drop remote client with ID %lu", playerId);
				break;
			}
		}
	}
}

void TCP_NetServer::haltRecvThreads()
{
	SDL_LockMutex(remoteMutex);
	for (int i = 0; i < NETSP_PLAYERS_COUNT; ++i) {
		if (remote[i].socket == NULL) {
			continue;
		}
		SDLNet_TCP_Close(remote[i].socket);
		remote[i].socket = NULL;
		remote[i].playerId = 0;
	}
	SDL_UnlockMutex(remoteMutex);

	for (int i = 0; i < NETSP_PLAYERS_COUNT; ++i) {
		if (remote[i].recvThread != NULL) {
			SDL_WaitThread(remote[i].recvThread, NULL);
			remote[i].recvThread = NULL;
		}
	}
}

void TCP_NetServer::update()
{
}

TCPsocket TCP_NetServer::getRemoteSocketByIndex(int index, ulong & playerId)
{
	SDL_LockMutex(remoteMutex);
	TCPsocket sock = remote[index].socket;
	playerId = remote[index].playerId;
	SDL_UnlockMutex(remoteMutex);

	return sock;
}

TCPsocket TCP_NetServer::getRemoteSocketByPlayer(int playerId)
{
	SDL_LockMutex(remoteMutex);
	TCPsocket sock = NULL;
	for (int i = 0; i < NETSP_PLAYERS_COUNT; ++i) {
		if (remote[i].playerId == playerId) {
			sock = remote[i].socket;
			break;
		}
	}
	SDL_UnlockMutex(remoteMutex);

	return sock;
}

void TCP_NetServer::removeRemoteSocket(TCPsocket sock)
{
	SDL_LockMutex(remoteMutex);

	int removedIndex = -1;
	for (int i = 0; i < NETSP_PLAYERS_COUNT; ++i) {
		if (remote[i].socket == sock) {
			removedIndex = i;
			SDLNet_TCP_Close(remote[i].socket);
			remote[i].socket = NULL;
			remote[i].playerId = 0;
			break;
		}
	}

	//TODO NET inform everyone about what happened

	SDL_UnlockMutex(remoteMutex);

	if (removedIndex >= 0 && remote[removedIndex].recvThread != NULL) {
		SDL_WaitThread(remote[removedIndex].recvThread, NULL);
		remote[removedIndex].recvThread = NULL;
	}
}
