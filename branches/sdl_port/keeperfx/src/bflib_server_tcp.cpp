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
		NETDBG(6, "Checking sockets");

		SDL_LockMutex(svr->remoteMutex);

		for (int i = 0; i < svr->maxPlayers - 1; ++i) {
			if (svr->remote[i].playerId == 0 || !SDLNet_SocketReady(svr->remote[i].socket)) {
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
		removeRemoteSocket(peer.socket);
		SDL_UnlockMutex(remoteMutex);
		return;
	}

	peer.bytesToRead -= bytesRead;

	// Finalize ===============================================================

	if (peer.bytesToRead <= 0) {
		switch (peer.readState) {
		case HEADER:
			peer.currMsg->inflate(); //TODO NET: add max allowed size sanity check
			peer.bytesToRead = peer.currMsg->getDataSize();

			//next state
			peer.readState = peer.currMsg->getDestPlayer() == getLocalPlayerId()? CONSUME : PASS_ON;

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
			RemotePeer ** const p = getPeersByPlayerId(peer.currMsg->getDestPlayer());
			if (p != NULL) {
				for (RemotePeer ** it = p; *it != NULL; ++it) {
					SDLNet_TCP_Send((*it)->socket, peer.currMsg->getBufferPointer(), peer.currMsg->getSize());
				}

				free(p);
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

	SDL_LockMutex(remoteMutex);

	RemotePeer ** const peers = getPeersByPlayerId(playerId);
	if (peers == NULL) {
		NETLOG("No socket to player with ID %d", playerId);
		retval = false;
	}
	else {
		InternalMsg * msg = buildTCPMessage(0, buffer, bufferLen);
		NETMSG("Begins sending");

		for (RemotePeer ** it = peers; *it != NULL; ++it) {

			NETMSG("Sending to socket for player %d", (*it)->playerId);
			msg->setDestPlayer((*it)->playerId);

			if (SDLNet_TCP_Send((*it)->socket, msg->getBufferPointer(), msg->getSize()) < msg->getDataSize()) {
				NETDBG(5, "Remote client with ID %d closed", (*it)->playerId);
				removeRemoteSocket((*it)->socket);
				retval = false;
			}
		}


		delete msg;
	}

	free(peers);

	SDL_UnlockMutex(remoteMutex);

	return retval;
}

void TCP_NetServer::addRemoteSocket(int index, TCPsocket socket)
{
	SDL_LockMutex(remoteMutex);

	remote[index].socket = socket;
	remote[index].playerId = PLAYERID_UNASSIGNED;
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

TCP_NetServer::RemotePeer ** TCP_NetServer::getPeersByPlayerId(int playerId)
{
	SDL_LockMutex(remoteMutex);

	bool everyone = playerId == PLAYERID_ALL || playerId == PLAYERID_ALLBUTSELF;
	int count = 0;
	for (int i = 0; i < maxPlayers - 1; ++i) {
		if ((everyone && remote[i].playerId != 0) || remote[i].playerId == playerId) {
			count += 1;
		}
	}

	if (count == 0) {
		return NULL;
	}

	count = 0;
	RemotePeer ** const peers = reinterpret_cast<RemotePeer **>(malloc(sizeof(RemotePeer*) * (count + 1)));
	for (int i = 0; i < maxPlayers - 1; ++i) {
		if ((everyone && remote[i].playerId != 0) || remote[i].playerId == playerId) {
			peers[count++] = &remote[i];
		}
	}
	peers[count] = NULL;

	SDL_UnlockMutex(remoteMutex);

	return peers;
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

