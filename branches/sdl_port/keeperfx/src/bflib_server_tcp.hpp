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
	enum ReadState
	{
		HEADER = 0,
		PASS_ON = 1,
		CONSUME = 2
	};

	struct RemotePeer
	{
		TCPsocket socket;

		ReadState readState;
		size_t bytesToRead;
		ulong playerId;

		InternalMsg * currMsg; //for CONSUME and PASS_ON states
	};

	TCPsocket mySocket; //= Server socket
	RemotePeer remote[NETSP_PLAYERS_COUNT];
	SDLNet_SocketSet socketSet;
	SDL_mutex * const remoteMutex;
	SDL_Thread * recvThread;
	int maxPlayers;
	bool joinable;
	bool shouldExit; //TODO NET: signal... perhaps ThreadCond..

	static void recvThreadFunc(TCP_NetServer * svr);
	void haltRecvThread();

	void addRemoteSocket(int index, TCPsocket);
	TCPsocket * getRemoteSocketByIndex(int index, ulong & playerId);
	RemotePeer ** getPeersByPlayerId(int playerId);
	void removeRemoteSocket(TCPsocket sock);
	void readRemotePeer(RemotePeer & peer);

public:
	TCP_NetServer(ushort port, int maxPlayers, int localPlayerId);
	virtual ~TCP_NetServer();

	virtual void update();
	virtual bool sendDKMessage(unsigned long playerId, const char buffer[], size_t bufferLen);

	virtual void setServerOptions(bool joinable);
};

#endif //!BFLIB_SERVER_TCP_HPP
