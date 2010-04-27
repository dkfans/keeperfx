/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_client_tcp.hpp
 *     Header file for bflib_client_tcp.cpp.
 * @par Purpose:
 *     TCP client class.
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

#ifndef BFLIB_CLIENT_TCP_HPP
#define BFLIB_CLIENT_TCP_HPP

#include "bflib_base_tcp.hpp"

class TCP_NetClient : public TCP_NetBase
{
	TCPsocket mySocket; //potentially mutex is needed for this

	SDL_Thread * recvThread;

	static void recvThreadFunc(TCP_NetClient * cli);
	void haltRecvThread();
public:
	TCP_NetClient(const char hostname[], ushort port);
	virtual ~TCP_NetClient();

	virtual void update();
	virtual bool sendDKMessage(unsigned long playerId, const char buffer[], size_t bufferLen);
};

#endif //!BFLIB_CLIENT_TCP_HPP
