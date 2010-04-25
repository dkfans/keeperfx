/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsp_tcp.hpp
 *     Header file for bflib_netsp_tcp.cpp.
 * @par Purpose:
 *     IP network ServiceProvider subclass declaration.
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

#ifndef BFLIB_NETSP_TCP_HPP
#define BFLIB_NETSP_TCP_HPP

#include <SDL_net.h>

#include "bflib_netsp.hpp"
#include "bflib_threadcond.hpp"


class UDP_NetHost;
class UDP_NetListener;

class TCPServiceProvider : public ServiceProvider {
private:
	TCPsocket mySocket;
	struct
	{
		TCPsocket socket;
		int playerId;
	} remote[NETSP_PLAYERS_COUNT]; //could really be NETSP_PLAYERS_COUNT-1 but it doesn't matter
	SDL_mutex * const remoteMutex;
	int maxPlayers;
	bool runningServer;
	bool joinable;

	UDP_NetHost * host;
	UDP_NetListener * listener;

	//we must buffer messages because of PeekMessage
	uchar * recvBuffer; //contains DK message (including DK header)
	size_t recvBufferSize;
	ulong recvPlayer;

	SDLNet_SocketSet recvSocketSet;
	SDL_Thread * recvThread;
	ThreadCond recvCond;

	static void serverReceiveThreadFunc(TCPServiceProvider * sp);
	static void clientReceiveThreadFunc(TCPServiceProvider * sp);
	void haltReceiveThread();

	bool fetchMessage(ulong * playerId, void * msg, ulong * a3, bool peek);
	void resetReceiveBuffer();

	void addRemoteSocket(int index, TCPsocket);
	TCPsocket getRemoteSocketByIndex(int index);
	TCPsocket getRemoteSocketByPlayer(int playerId);
	void removeRemoteSocket(TCPsocket sock);

public:
	TCPServiceProvider();
	virtual ~TCPServiceProvider();
	virtual TbError Start(struct TbNetworkSessionNameEntry * sessionName, char * playerName, void * options);
	virtual TbError Start(char * sessionName, char * playerName, unsigned long maxPlayers, void * options);
	virtual TbError Stop(void);
	virtual TbError Enumerate(TbNetworkCallbackFunc callback, void * ptr);
	virtual TbError Enumerate(struct TbNetworkSessionNameEntry * sessionEntry,
			TbNetworkCallbackFunc playerCb, void * ptr);
	virtual TbError Init(struct _GUID a1, struct _GUID * a2, struct ReceiveCallbacks * recCb, void * a4);
	virtual TbError Release(void);
	virtual TbError ChangeSettings(unsigned long, void *);
	virtual TbError EnableNewPlayers(TbBool allow);
	virtual bool ReadMessage(unsigned long * playerId, void * msg, unsigned long *);
	virtual bool PeekMessage(unsigned long *, void *, unsigned long *);
	virtual TbError SendMessage(unsigned long playerId, void * msg, unsigned char i);
	virtual void tick();
};

#endif //BFLIB_NETSP_TCP_HPP
