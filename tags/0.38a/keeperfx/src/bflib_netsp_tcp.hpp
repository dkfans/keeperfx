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

#include "bflib_netsp.hpp"
#include "bflib_base_tcp.hpp"

class UDP_NetHost;
class UDP_NetListener;
class TCP_Base;

class TCPServiceProvider : public ServiceProvider {
private:
	int maxPlayers;
	bool isServer;
	bool joinable;

	UDP_NetHost * host;
	UDP_NetListener * listener;
	TCP_NetBase * base;

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
	virtual void update();
};

#endif //BFLIB_NETSP_TCP_HPP
