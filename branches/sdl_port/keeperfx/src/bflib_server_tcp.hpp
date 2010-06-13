/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_server_tcp.hpp
 *     Header file for bflib_server_tcp.cpp.
 * @par Purpose:
 *     TCP server class.
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

#ifndef BFLIB_SERVER_TCP_HPP
#define BFLIB_SERVER_TCP_HPP

#include "bflib_base_tcp.hpp"
#include "bflib_netsp.hpp"

class TCP_NetServer : public TCP_NetBase
{
	TCPsocket mySocket;
	struct
	{
		TCPsocket socket;
		int playerId;
		SDL_Thread * recvThread;
	} remote[NETSP_PLAYERS_COUNT]; //could really be NETSP_PLAYERS_COUNT-1 but it doesn't matter

	SDL_mutex * const remoteMutex;

	struct RecvThreadArg
	{
		TCP_NetServer * svr;
		int remoteIndex;
	};

	static void recvThreadFunc(RecvThreadArg * arg);
	void haltRecvThreads();

	void addRemoteSocket(int index, TCPsocket);
	TCPsocket getRemoteSocketByIndex(int index, ulong & playerId);
	TCPsocket getRemoteSocketByPlayer(int playerId);
	void removeRemoteSocket(TCPsocket sock);

	int maxPlayers;
	bool joinable;

public:
	explicit TCP_NetServer(ushort port);
	virtual ~TCP_NetServer();

	virtual void update();
	virtual bool sendDKMessage(unsigned long playerId, const char buffer[], size_t bufferLen);

	virtual void setServerOptions(int maxPlayers, bool joinable);
};

#endif //!BFLIB_SERVER_TCP_HPP
