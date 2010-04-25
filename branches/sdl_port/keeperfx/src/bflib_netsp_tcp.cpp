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

#include "bflib_netconfig.hpp"
#include "bflib_network.h"
#include "bflib_nethost_udp.hpp"
#include "bflib_netlisten_udp.hpp"

/*
 * TCP frame format:
 * Header:
 * 0x00-0x04: destination player ID
 * 0x04-0x08: tcp data length
 * Data (size of DK message data + DK message header of 4 bytes):
 * 0x08-0x??
 */

const size_t TCP_HEADER_SIZE = 8;

TCPServiceProvider::TCPServiceProvider() :
		remoteMutex(SDL_CreateMutex()),
		host(NULL),
		listener(NULL),
		recvThread(NULL),
		recvCond()
{
	resetReceiveBuffer();
}

TCPServiceProvider::~TCPServiceProvider()
{
	SDL_DestroyMutex(remoteMutex);
}

void TCPServiceProvider::serverReceiveThreadFunc(TCPServiceProvider * sp)
{

}

void TCPServiceProvider::clientReceiveThreadFunc(TCPServiceProvider * sp)
{

}

TbError TCPServiceProvider::Start(struct TbNetworkSessionNameEntry * sessionName, char * playerName, void * options)
{
	SYNCDBG(7, "Starting");
	assert(!started);

	runningServer = false;

	ClearPlayers();

	IPaddress addr;
	SDLNet_ResolveHost(&addr, "localhost", HOST_PORT_NUMBER); //TODO: change from local host to real session later
	mySocket = SDLNet_TCP_Open(&addr);
	if (mySocket == NULL) {
		NETMSG("Failed to initialize TCP client socket");
		return Lb_FAIL;
	}

	//TODO: see if we're responsible for adding ourself to player list or not
	localPlayerId = 0; //for now...

	//TODO: see if local_id must be set

	started = true;

	recvCond.reset();
	recvThread = SDL_CreateThread(reinterpret_cast<int (*)(void *)>(clientReceiveThreadFunc), this);
	if (recvThread == NULL) {
		NETMSG("Failed to initialize TCP client receive thread");
		Stop();
		return Lb_FAIL;
	}

	return Lb_OK;
}

TbError TCPServiceProvider::Start(char * sessionName, char * playerName, unsigned long maxPlayers, void * options)
{
	SYNCDBG(7, "Starting");
	assert(!started);
	assert(maxPlayers >= 2);
	assert(maxPlayers <= NETSP_PLAYERS_COUNT);

	runningServer = true;

	memset(remote, 0, sizeof(remote));
	ClearPlayers();
	joinable = true;
	this->maxPlayers = maxPlayers;

	// Open server socket and socket set (as player 0's socket):
	IPaddress addr;
	addr.host = INADDR_ANY;
	SDLNet_Write16(HOST_PORT_NUMBER, &addr.port);
	mySocket = SDLNet_TCP_Open(&addr);
	if (mySocket == NULL) {
		NETMSG("Failed to initialize TCP server socket");
		return Lb_FAIL;
	}

	recvSocketSet = SDLNet_AllocSocketSet(maxPlayers - 1);
	if (recvSocketSet == NULL) {
		SDLNet_TCP_Close(mySocket);
		NETMSG("Failed to initialize TCP socket set");
		return Lb_FAIL;
	}

	// Fill in player info.
	localPlayerId = 1;
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

	recvCond.reset();
	recvThread = SDL_CreateThread(reinterpret_cast<int (*)(void *)>(serverReceiveThreadFunc), this);
	if (recvThread == NULL) {
		NETMSG("Failed to initialize TCP server receive thread");
		Stop();
		return Lb_FAIL;
	}

	return Lb_OK;
}

TbError TCPServiceProvider::Stop(void)
{
	SYNCDBG(7, "Starting");

	haltReceiveThread();

	if (runningServer) {
		SDLNet_FreeSocketSet(recvSocketSet);
	}

	for (int i = 0; i < NETSP_PLAYERS_COUNT; ++i) {
		if (remote[i].socket != NULL) {
			SDLNet_TCP_Close(remote[i].socket);
		}
	}

	SDLNet_TCP_Close(mySocket);

	/*int index = SessionIndex(local_id);
	if (index >= 0) {
		nsnames[index].in_use = false;
	}*/
	localPlayerId = 0;

	delete host;
	host = NULL;

	free(recvBuffer);
	resetReceiveBuffer();
	started = false;

	return Lb_OK;
}

void TCPServiceProvider::resetReceiveBuffer()
{
	recvBuffer = NULL;
	recvBufferSize = 0;
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

	if (sessionEntry == NULL) {
		for (int i = 0; i < players_count; ++i) {
			TbNetworkPlayerNameEntry entry;
			memset(&entry, 0, sizeof(entry));
			if (i == 0) {
				entry.isHost = runningServer;
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
	joinable = allow;
	return Lb_OK;
}

bool TCPServiceProvider::fetchMessage(ulong * playerId, void * msg, ulong * a3, bool peek) {
	/*if (recvBuffer == NULL) {
		//read TCP header first
		char header[TCP_HEADER_SIZE];
		int numLeft = TCP_HEADER_SIZE, numRead;
		do {
			if (runningServer) {

			}
			else {

			}
			numRead = SDLNet_TCP_Read()
		} while (numRead >= 1 && numLeft >= 0);

		if (numRead == 0) {
			//TODO: deal with closed socket here
		}
		else if (numRead == -1) {
			NETMSG("TCP receive error");
		}
	}*/

	return false;
}

bool TCPServiceProvider::ReadMessage(unsigned long * playerId, void * msg, unsigned long * a3) {
	return fetchMessage(playerId, msg, a3, false);
}

bool TCPServiceProvider::PeekMessage(unsigned long * playerId, void * msg, unsigned long * a3) {
	return fetchMessage(playerId, msg, a3, true);
}

TbError TCPServiceProvider::SendMessage(unsigned long playerId, void * msg, unsigned char i) {
	assert(started);

	//get length of DK message
	ulong msgDataLen;
	DecodeMessageStub(msg, &msgDataLen, NULL, NULL);
	msgDataLen += 4; //include DK msg header
	ulong totalMsgLen = msgDataLen + TCP_HEADER_SIZE;

	//build buffer
	uchar * const buffer = reinterpret_cast<uchar *>(malloc(totalMsgLen));
	if (buffer == NULL) {
		NETMSG("malloc failure");
		return Lb_FAIL;
	}
	uchar * bufferPtr = buffer;
	SDLNet_Write32(playerId, bufferPtr);
	bufferPtr += 4;
	SDLNet_Write32(msgDataLen, bufferPtr); //accounts for header
	bufferPtr += 4;
	memcpy(bufferPtr, msg, msgDataLen);

	//send to appropriate destination
	if (runningServer) {
		TCPsocket socket = getRemoteSocketByPlayer(playerId);
		if (socket == NULL) {
			NETLOG("No socket to player with ID %d", playerId);
		}
		else if (SDLNet_TCP_Send(socket, buffer, totalMsgLen) < totalMsgLen) {
			removeRemoteSocket(socket);
			NETMSG("Remote client with ID %d closed", playerId);
			//TODO: spread message that client has disconnected if necessary
			return Lb_FAIL;
		}
	}
	else {
		if (SDLNet_TCP_Send(mySocket, buffer, totalMsgLen) < totalMsgLen) {
			haltReceiveThread();
			SDLNet_TCP_Close(mySocket);
			mySocket = NULL;
			NETMSG("Remote server closed");
			//TODO: spread message that server has disconnected if necessary
			return Lb_FAIL;
		}
	}

	//clean up
	free(buffer);

	return Lb_OK;
}

void TCPServiceProvider::tick()
{
	//if this is a server, check for incoming connections
	if (runningServer && started) {
		TCPsocket newSocket;
		while ((newSocket = SDLNet_TCP_Accept(mySocket)) != NULL) { //does not block
			if (joinable) {
				bool notFound = true;
				for (int i = 0; i < maxPlayers - 1; ++i) {
					if (remote[i].socket == NULL) { //sockets will never be created by receive thread so no lock required
						addRemoteSocket(i, newSocket);
						notFound = false;
						break;
						//TODO: see if we should add player here or if protocol does this
					}
				}

				if (notFound) {
					SDLNet_TCP_Close(newSocket);
					NETMSG("Socket dropped because server is full");
				}
			}
			else {
				SDLNet_TCP_Close(newSocket);
			}
		}
	}
}

void TCPServiceProvider::haltReceiveThread()
{
	if (recvThread != NULL) {
		recvCond.signalExit();
		SDL_WaitThread(recvThread, NULL);
		recvThread = NULL;
	}
}

void TCPServiceProvider::addRemoteSocket(int index, TCPsocket socket)
{
	assert(runningServer);

	SDL_LockMutex(remoteMutex);

	remote[index].socket = socket;
	if (SDLNet_TCP_AddSocket(recvSocketSet, remote[index].socket) == -1) {
		NETMSG("Too many sockets in socket set... This will influence reception.");
	}

	SDL_UnlockMutex(remoteMutex);
}

TCPsocket TCPServiceProvider::getRemoteSocketByIndex(int index)
{
	SDL_LockMutex(remoteMutex);
	TCPsocket sock = remote[index].socket;
	SDL_UnlockMutex(remoteMutex);

	return sock;
}

TCPsocket TCPServiceProvider::getRemoteSocketByPlayer(int playerId)
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

void TCPServiceProvider::removeRemoteSocket(TCPsocket sock)
{
	assert(runningServer);

	SDL_LockMutex(remoteMutex);

	for (int i = 0; i < NETSP_PLAYERS_COUNT; ++i) {
		if (remote[i].socket == sock) {
			SDLNet_TCP_DelSocket(recvSocketSet, remote[i].socket);
			SDLNet_TCP_Close(remote[i].socket);
			remote[i].socket = NULL;
			remote[i].playerId = 0;
			break;
		}
	}

	SDL_UnlockMutex(remoteMutex);
}
