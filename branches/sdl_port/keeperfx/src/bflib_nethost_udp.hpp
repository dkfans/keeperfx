/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sessions_udp.hpp
 *     Header file for bflib_nethost_udp.cpp.
 * @par Purpose:
 *     UDP_NetHost: Handles LAN discovery of server through UDP broadcasts and
 *     answers requests for server info.
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

#ifndef BFLIB_NETHOST_UDP_HPP
#define BFLIB_NETHOST_UDP_HPP

#include <SDL_net.h>
#include <string>
#include <vector>

#include "bflib_threadcond.hpp"
#include "bflib_network.h" //try get rid of this dependency

class UDP_NetHost {
public:
	typedef std::vector<std::string> StringVector;
private:
	SDL_Thread * thread;
	ThreadCond cond;

	static void threadFunc(UDP_NetHost * sh);

	StringVector broadcastAddr;

	bool criticalError;

public:
	explicit UDP_NetHost(const StringVector & broadcastAddresses);
	~UDP_NetHost();

	bool hadCriticalError() { return criticalError; }
};

#endif //!BFLIB_NETHOST_UDP_HPP
