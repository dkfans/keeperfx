/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_client_tcp.cpp
 *     TCP client class declaration.
 * @par Purpose:
 *     Defines TCP client class.
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

#include "bflib_client_tcp.hpp"

#include "globals.h"

TCP_NetClient::TCP_NetClient(const char hostname[], ushort port) : TCP_NetBase()
{
	IPaddress addr;
	SDLNet_ResolveHost(&addr, hostname, port); //TODO: change from local host to real session later
	mySocket = SDLNet_TCP_Open(&addr);
	if (mySocket == NULL) {
		NETMSG("Failed to initialize TCP client socket");
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

TCP_NetClient::~TCP_NetClient()
{
	haltRecvThread();
}

void TCP_NetClient::recvThreadFunc(TCP_NetClient * cli)
{
	for (;;) {
		char header[TCP_HEADER_SIZE];
		if (!receiveOnSocket(cli->mySocket, header, sizeof(header))) {
			break;
		}

		ulong playerId = SDLNet_Read32(header);
		ulong msgDataLen = SDLNet_Read32(header + 4);
		InternalMsg * msg = new InternalMsg(msgDataLen, playerId);
		if (msg == NULL || msg->buffer == NULL) {
			ERRORLOG("Failure to allocate memory for message");
			break;
		}

		if (!receiveOnSocket(cli->mySocket, msg->buffer, msgDataLen)) {
			delete msg;
			break;
		}

		cli->addIntMessage(msg);
	}
}

void TCP_NetClient::haltRecvThread()
{
	SYNCDBG(7, "Starting");

	//necessary to close socket because receive thread may be waiting for messages
	SDLNet_TCP_Close(mySocket);
	mySocket = NULL;

	if (recvThread != NULL) {
		SDL_WaitThread(recvThread, NULL);
		recvThread = NULL;
	}
}

void TCP_NetClient::update()
{
	//nothing needed for client yet
	return;
}

bool TCP_NetClient::sendDKMessage(unsigned long playerId, const char buffer[], size_t bufferLen)
{
	bool retval = true;

	size_t len = bufferLen;
	char * msg = buildTCPMessageBuffer(playerId, buffer, len);

	if (SDLNet_TCP_Send(mySocket, msg, len) < len) {
		haltRecvThread();
		NETMSG("Remote server closed");
		retval = false;
	}

	free(msg);

	return retval;
}
