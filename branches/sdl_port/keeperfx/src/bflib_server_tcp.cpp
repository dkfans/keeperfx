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

TCP_NetServer::TCP_NetServer(ushort port, int maxP, int localPid) : TCP_NetBase(localPid),
		socketSet(SDLNet_AllocSocketSet(maxP)),
		remoteMutex(SDL_CreateMutex()),
		maxPlayers(maxP),
		joinable(true),
		shouldExit(false)
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

	recvThread = SDL_CreateThread(reinterpret_cast<int (*)(void *)>(recvThreadFunc), this);
	if (recvThread == NULL) {
		NETMSG("Failed to initialize TCP client receive thread");
		setErrorFlag();
		return;
	}
}

TCP_NetServer::~TCP_NetServer()
{
	haltRecvThread();
	SDL_DestroyMutex(remoteMutex);
}

void TCP_NetServer::recvThreadFunc(TCP_NetServer * svr)
{
	SYNCDBG(4, "Starting");

	while (!svr->shouldExit) {
		if (SDLNet_CheckSockets(svr->socketSet, 100) <= 0) {
			continue;
		}

		SDL_LockMutex(svr->remoteMutex);

		for (int i = 0; i < svr->maxPlayers; ++i) {
			if (!SDLNet_SocketReady(svr->remote[i].socket)) {
				continue;
			}

			svr->readRemotePeer(svr->remote[i]);
		}

		SDL_UnlockMutex(svr->remoteMutex);
	}

	SYNCDBG(4, "Exiting");
}

void TCP_NetServer::readRemotePeer(RemotePeer & peer)
{
	SYNCDBG(7, "Starting");

	SDL_LockMutex(remoteMutex);

	// Read ===================================================================

	void * const destBuffer = peer.readState == HEADER?
			peer.currMsg->getHeaderPointer() + peer.currMsg->getHeaderSize() - peer.bytesToRead :
			peer.currMsg->getDataPointer() + peer.currMsg->getDataSize() - peer.bytesToRead;

	//common behavior for all states is to read as much as possible for current block
	int bytesRead = SDLNet_TCP_Recv(peer.socket, destBuffer, peer.bytesToRead);
	if (bytesRead < 0) {
		NETLOG("TCP receive error, will drop remote client");
		SDL_UnlockMutex(remoteMutex);
		removeRemoteSocket(peer.socket);
		return;
	}

	peer.bytesToRead -= bytesRead;

	// Finalize ===============================================================

	if (peer.bytesToRead <= 0) {
		switch (peer.readState) {
		case HEADER:
			peer.currMsg->inflate(); //TODO NET: add max allowed size sanity check
			peer.bytesToRead = peer.currMsg->getDataSize();

			if (peer.currMsg->getDestPlayer() == getLocalPlayerId()) {
				//next state
				peer.readState = CONSUME;
			}
			else {
				peer.destSockets = getRemoteSocketsByPlayer(peer.currMsg->getDestPlayer());

				//next state
				peer.readState = PASS_ON;
			}

			//recheck socket
			SDLNet_CheckSockets(socketSet, 0);
			if (SDLNet_SocketReady(peer.socket)) {
				readRemotePeer(peer);
			}

			break;
		case CONSUME:
			addIntMessage(peer.currMsg);
			peer.currMsg = new InternalMsg(0);

			//next state
			peer.readState = HEADER;
			peer.bytesToRead = TCP_HEADER_SIZE;
			break;
		case PASS_ON:
			if (peer.destSockets != NULL) {
				for (int i = 0; peer.destSockets[i] != NULL; ++i) {
					SDLNet_TCP_Send(peer.destSockets[i], peer.currMsg->getBufferPointer(), peer.currMsg->getSize());
				}

				free(peer.destSockets);
			}

			peer.currMsg->resizeData(0);

			//next state
			peer.readState = HEADER;
			peer.bytesToRead = TCP_HEADER_SIZE;
			break;
		}
	}

	SDL_UnlockMutex(remoteMutex);
}

void TCP_NetServer::haltRecvThread()
{
	if (recvThread != NULL) {
		shouldExit = true;
		SDL_WaitThread(recvThread, NULL);
		recvThread = NULL;
	}

	SDL_LockMutex(remoteMutex);
	for (int i = 0; i < maxPlayers - 1; ++i) {
		if (remote[i].socket == NULL) {
			continue;
		}
		SDLNet_TCP_Close(remote[i].socket);
		remote[i].socket = NULL;
		remote[i].playerId = 0;
	}
	SDL_UnlockMutex(remoteMutex);

	SDLNet_TCP_Close(mySocket);
	mySocket = NULL;

	SDLNet_FreeSocketSet(socketSet);
}

void TCP_NetServer::update()
{
	TCPsocket newSocket;
	while ((newSocket = SDLNet_TCP_Accept(mySocket)) != NULL) { //does not block
		SDL_LockMutex(remoteMutex);

		if (joinable) {
			bool notFound = true;
			for (int i = 0; i < maxPlayers - 1; ++i) {
				if (remote[i].socket == NULL) { //sockets will never be created by receive thread so no lock required
					addRemoteSocket(i, newSocket);
					notFound = false;

					//TODO: see if we should add player here or if protocol does this

					NETDBG(5, "Remote socket accepted");

					break;
				}
			}

			if (notFound) {
				SDLNet_TCP_Close(newSocket);
				NETDBG(5, "Socket dropped because server is full");
			}
		}
		else {
			SDLNet_TCP_Close(newSocket);
			NETDBG(5, "Socket dropped because joining is disabled");
		}

		SDL_UnlockMutex(remoteMutex);
	}
}

bool TCP_NetServer::sendDKMessage(unsigned long playerId, const char buffer[], size_t bufferLen)
{
	bool retval = true;

	TCPsocket * const sockets = getRemoteSocketsByPlayer(playerId);
	if (sockets == NULL) {
		NETLOG("No socket to player with ID %d", playerId);
		retval = false;
	}
	else {
		for (int i = 0; sockets[i] != NULL; ++i) {
			InternalMsg * msg = buildTCPMessage(playerId, buffer, bufferLen);

			if (SDLNet_TCP_Send(sockets[i], msg->getBufferPointer(), msg->getSize()) < msg->getDataSize()) {
				removeRemoteSocket(sockets[i]);
				NETDBG(5, "Remote client with ID %d closed", playerId);
				retval = false;
			}

			delete msg;
		}
	}

	free(sockets);

	return retval;
}

void TCP_NetServer::addRemoteSocket(int index, TCPsocket socket)
{
	SDL_LockMutex(remoteMutex);

	remote[index].socket = socket;
	remote[index].playerId = 0;
	remote[index].readState = HEADER;
	remote[index].bytesToRead = TCP_HEADER_SIZE;
	remote[index].currMsg = new InternalMsg(0);

	SDLNet_TCP_AddSocket(socketSet, socket);

	SDL_UnlockMutex(remoteMutex);
}

TCPsocket * TCP_NetServer::getRemoteSocketByIndex(int index, ulong & playerId)
{
	if (index >= maxPlayers) {
		return NULL;
	}

	SDL_LockMutex(remoteMutex);

	TCPsocket * sock = &remote[index].socket;
	playerId = remote[index].playerId;

	SDL_UnlockMutex(remoteMutex);

	return sock;
}

TCPsocket * TCP_NetServer::getRemoteSocketsByPlayer(int playerId)
{
	SDL_LockMutex(remoteMutex);

	int count = 0;
	for (int i = 0; i < maxPlayers - 1; ++i) {
		if (playerId == PLAYERID_ALL || playerId == PLAYERID_ALLBUTSELF || remote[i].playerId == playerId) {
			count += 1;
		}
	}

	TCPsocket * sockets = reinterpret_cast<TCPsocket *>(malloc(sizeof(TCPsocket) * (count + 1)));
	for (int i = 0; i < maxPlayers - 1; ++i) {
		if (playerId == PLAYERID_ALL || playerId == PLAYERID_ALLBUTSELF || remote[i].playerId == playerId) {
			sockets[i] = remote[i].socket;
		}
	}
	sockets[count] = NULL;

	SDL_UnlockMutex(remoteMutex);

	return sockets;
}

void TCP_NetServer::removeRemoteSocket(TCPsocket sock)
{
	SDL_LockMutex(remoteMutex);

	int removedIndex = -1;
	for (int i = 0; i < maxPlayers - 1; ++i) {
		if (remote[i].socket == sock) {
			SDLNet_TCP_DelSocket(socketSet, sock);

			removedIndex = i;
			SDLNet_TCP_Close(remote[i].socket);
			remote[i].socket = NULL;
			remote[i].playerId = 0;
			free(remote[i].destSockets);
			remote[i].destSockets = NULL,
			delete remote[i].currMsg;
			remote[i].currMsg = NULL;
			break;
		}
	}

	//TODO: inform everyone about what happened

	SDL_UnlockMutex(remoteMutex);
}

void TCP_NetServer::setServerOptions( bool joinable)
{
	this->joinable = joinable;
}

