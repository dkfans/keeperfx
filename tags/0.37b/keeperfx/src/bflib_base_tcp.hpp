/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_base_tcp.hpp
 *     Header file for bflib_base_tcp.cpp.
 * @par Purpose:
 *     Base class for TCP client and server.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     10 April 2010 - ?
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef BFLIB_BASE_TCP_HPP
#define BFLIB_BASE_TCP_HPP

#include <SDL_net.h>
#include <SDL_thread.h>

#include "bflib_basics.h"

/*
 * TCP frame format:
 * Header:
 * 0x00-0x04: destination player ID //TODO: potentially cut down on bit length, 8 should be enough... but that applies everywhere
 * 0x04-0x08: tcp data length
 * Data (size of DK message data + DK message header of 4 bytes):
 * 0x08-0x??
 */

const size_t TCP_HEADER_SIZE = 8;

/**
 * This class factors out all common functionality shared between TCP_Server and TCP_Client.
 */
class TCP_NetBase
{
protected:
	class InternalMsg
	{
	public:
		struct InternalMsg * next; //I would have preferred std::list... again, compilation error
		ulong playerId;
		size_t len;
		char * buffer;

		InternalMsg(size_t size, ulong player) : next(NULL), playerId(player), len(size),
				buffer((char *)calloc(1, size)) { }
		~InternalMsg() { free(buffer); }
	};

private:
	//message queue
	InternalMsg * msgHead;
	InternalMsg * msgTail;
	SDL_mutex * const msgMutex;

	bool errorFlag; //indicates constructor error, we don't have exception handling

protected:
	/**
	 * Constructs a TCP message from a DK message.
	 * @param playerId ID of player to be recipent of message.
	 * @param msg The DK message buffer.
	 * @param msgLen Initially, size of DK message buffer (including header). Contains size of returned
	 * buffer after function exits.
	 * @return The TCP message buffer. This must be free'd by caller.
	 */
	char * buildTCPMessageBuffer(ulong playerId, const char msg[], size_t & msgLen);
	static bool receiveOnSocket(TCPsocket sock, char buffer[], size_t count);

	//synchronized message queue
	void addIntMessage(InternalMsg * msg);
	InternalMsg * getIntMessage();
	InternalMsg * peekIntMessage();
	void clearIntMessages();

	void setErrorFlag() { errorFlag = true; }

public:
	TCP_NetBase();
	virtual ~TCP_NetBase();
	bool hadError() { return errorFlag; }

	virtual void update() = 0;
	bool fetchDKMessage(ulong & playerId, char buffer[], size_t & bufferLen, bool peek);
	virtual bool sendDKMessage(unsigned long playerId, const char buffer[], size_t bufferLen) = 0;
};

#endif //!BFLIB_BASE_TCP_HPP
