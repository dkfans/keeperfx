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

#include <SDL2/SDL_net.h>
#include <SDL2/SDL_thread.h>

#include "bflib_basics.h"

/*
 * TCP frame format:
 * Header:
 * 0x00-0x04: destination player ID //TODO NET potentially cut down on bit length, 8 should be enough... but that applies everywhere
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
		class InternalMsg * next; //I would have preferred std::list... again, compilation error
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
	static bool receiveOnSocket(TCPsocket sock, char buffer[], size_t count);

	//synchronized message queue
	void addIntMessage(InternalMsg * msg);
	InternalMsg * getIntMessage();
	void clearIntMessages();

	void setErrorFlag() { errorFlag = true; }

public:
	TCP_NetBase();
	virtual ~TCP_NetBase();

	virtual void update() = 0;
};

#endif //!BFLIB_BASE_TCP_HPP
