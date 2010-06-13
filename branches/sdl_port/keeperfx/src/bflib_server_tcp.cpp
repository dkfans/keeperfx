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

#include "bflib_server_tcp.hpp"

TCP_NetServer::TCP_NetServer(ushort port) : TCP_NetBase(),
		remoteMutex(SDL_CreateMutex()),
		maxPlayers(0),
		joinable(false)
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
		if (true /*playerId == sp->localPlayerId*/) { //TODO: handle this
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
				NETMSG("TCP send error, will drop remote client with ID %d", playerId);
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
	//TODO: deal with this, needs data presently in TCPServiceProvider
	TCPsocket newSocket;
	while ((newSocket = SDLNet_TCP_Accept(mySocket)) != NULL) { //does not block
		if (joinable) {
			bool notFound = true;
			for (int i = 0; i < maxPlayers - 1; ++i) {
				if (remote[i].socket == NULL) { //sockets will never be created by receive thread so no lock required
					addRemoteSocket(i, newSocket);
					notFound = false;

					//TODO: see if we should add player here or if protocol does this

					NETMSG("Remote socket accepted");

					break;
				}
			}

			if (notFound) {
				SDLNet_TCP_Close(newSocket);
				NETMSG("Socket dropped because server is full");
			}
		}
		else {
			SDLNet_TCP_Close(newSocket);
			NETMSG("Socket dropped because joining is disabled");
		}
	}
}

bool TCP_NetServer::sendDKMessage(unsigned long playerId, const char buffer[], size_t bufferLen)
{
	bool retval = true;

	TCPsocket socket = getRemoteSocketByPlayer(playerId);
	if (socket == NULL) {
		NETLOG("No socket to player with ID %d", playerId);
		retval = false;
	}
	else {
		size_t len = bufferLen;
		char * msg = buildTCPMessageBuffer(playerId, buffer, len);

		if (SDLNet_TCP_Send(socket, msg, len) < len) {
			removeRemoteSocket(socket);
			NETMSG("Remote client with ID %d closed", playerId);
			retval = false;
		}

		free(msg);
	}

	return retval;
}

void TCP_NetServer::addRemoteSocket(int index, TCPsocket socket)
{
	SDL_LockMutex(remoteMutex);

	remote[index].socket = socket;

	SDL_UnlockMutex(remoteMutex);
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

	//TODO: inform everyone about what happened

	SDL_UnlockMutex(remoteMutex);

	if (removedIndex >= 0 && remote[removedIndex].recvThread != NULL) {
		SDL_WaitThread(remote[removedIndex].recvThread, NULL);
		remote[removedIndex].recvThread = NULL;
	}
}

void TCP_NetServer::setServerOptions(int maxPlayers, bool joinable)
{
	this->maxPlayers = maxPlayers;
	this->joinable = joinable;
}

