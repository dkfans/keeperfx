/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_base_tcp.cpp
 *     TCP base class declaration.
 * @par Purpose:
 *     Defines TCP base class that specifies common functionality shared between
 *     TCP client and server classes.
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

#include "bflib_base_tcp.hpp"

#include "bflib_netsp.hpp"

TCP_NetBase::TCP_NetBase(int localPid) :
		msgHead(NULL),
		msgTail(NULL),
		msgMutex(SDL_CreateMutex()),
		localPlayerId(localPid),
		errorFlag(false)
{
}

TCP_NetBase::~TCP_NetBase()
{
	clearIntMessages();
	SDL_DestroyMutex(msgMutex);
}

bool TCP_NetBase::fetchDKMessage(ulong & playerId, char buffer[], size_t & bufferLen, bool peek)
{
	InternalMsg * msg = peek? peekIntMessage() : getIntMessage();

	if (msg == NULL) {
		return false;
	}

	playerId = msg->getDestPlayer();
	if (bufferLen < msg->getDataSize()) {
		bufferLen = 0;
	}
	else {
		bufferLen = msg->getDataSize();
		memcpy(buffer, msg->getDataPointer(), msg->getDataSize());
	}

	if (!peek) {
		delete msg;
	}

	return true;
}

TCP_NetBase::InternalMsg * TCP_NetBase::buildTCPMessage(ulong playerId, const char msg[], size_t & msgLen)
{
	InternalMsg * m = new InternalMsg(msgLen, playerId);
	msgLen = m->getSize();
	memcpy(m->getDataPointer(), msg, m->getDataSize());

	return m;
}

bool TCP_NetBase::receiveOnSocket(TCPsocket sock, char buffer[], size_t count)
{
	size_t numRead = 0;
	char * bufferPtr = buffer;

	if (sock == NULL) {
		return false;
	}

	do {
		const size_t bytesLeft = count - numRead;
		int result = SDLNet_TCP_Recv(sock, bufferPtr, bytesLeft);
		if (result <= 0) {
			return false;
		}
		else {
			bufferPtr += result;
			numRead += result;
		}
	} while (numRead < count);

	return true;
}

void TCP_NetBase::addIntMessage(InternalMsg * msg)
{
	msg->setNext(NULL);

	SDL_LockMutex(msgMutex);

	if (msgHead == NULL) {
		msgHead = msgTail = msg;
	}
	else {
		msgTail->setNext(msg);
		msgTail = msg;
	}

	SDL_UnlockMutex(msgMutex);
}

TCP_NetBase::InternalMsg * TCP_NetBase::getIntMessage()
{
	InternalMsg * retval = NULL;
	SDL_LockMutex(msgMutex);

	if (msgHead != NULL) {
		retval = msgHead;
		msgHead = msgHead->getNext();
		if (msgHead == NULL) {
			msgTail = NULL;
		}
	}

	SDL_UnlockMutex(msgMutex);

	return retval;
}

TCP_NetBase::InternalMsg * TCP_NetBase::peekIntMessage()
{
	InternalMsg * retval = NULL;
	SDL_LockMutex(msgMutex);

	if (msgHead != NULL) {
		retval = msgHead;
	}

	SDL_UnlockMutex(msgMutex);

	return retval;
}

void TCP_NetBase::clearIntMessages()
{
	SYNCDBG(7, "Starting");

	SDL_LockMutex(msgMutex);

	InternalMsg * msg;
	while ((msg = getIntMessage()) != NULL) {
		delete msg;
	}

	SDL_UnlockMutex(msgMutex);
}
