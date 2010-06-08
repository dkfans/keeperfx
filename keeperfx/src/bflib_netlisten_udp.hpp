/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_sessions_udp.hpp
 *     Header file for bflib_netlisten_udp.cpp.
 * @par Purpose:
 *     UDP_NetListener: Maintains a list of known hosts and periodically
 *     updates them. Hosts can be added automatically via UDP LAN broadcasting
 *     or manually.
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

#ifndef BFLIB_NETLISTEN_UDP_HPP
#define BFLIB_NETLISTEN_UDP_HPP

#include <SDL_net.h>
//TODO: remove reliance on string and vector, MinGW 3.4.5 compiler is FUBAR
#include <string>
#include <vector>

#include "bflib_threadcond.hpp"
#include "bflib_network.h" //try get rid of this dependency

class UDP_NetListener {
private:
	typedef std::vector<std::string> StringVector;

	SDL_Thread * thread;
	ThreadCond cond;

	static void threadFunc(UDP_NetListener * sh);

	struct Session
	{
		IPaddress addr;
		std::string name;
		StringVector players;
		unsigned long lastUpdateReq; //time of last update request
		unsigned long lastUpdateAns; //time of last update answer
		bool joinable; //is there any point in trying to join?
		bool removable; //if true, used may not be set to false (used for manually added IP addresses)
		bool used; //if false, this can be overwritten (it is not deleted from vector to preserve indices)

		Session(const IPaddress & a, const std::string n) :
			addr(a), name(n), players(), lastUpdateReq(0), lastUpdateAns(0),
			joinable(true), removable(true), used(true)
		{ }
	};

	typedef std::vector<Session> SessionVector;

	SessionVector sessions;
	SDL_mutex * const sessionsMutex; //guards sessions

	TbNetworkSessionNameEntry * reportSession(const IPaddress & addr, const char *namestr);
	TbNetworkSessionNameEntry * findSessionByAddress(const IPaddress & addr);

	bool criticalError;

public:
	UDP_NetListener();
	~UDP_NetListener();

	bool hadCriticalError() { return criticalError; }

	void addHost(const IPaddress & addr);
	void removeHost(const IPaddress & addr);

	IPaddress getSessionAddress(int sessionId);
};

#endif //!BFLIB_NETLISTEN_UDP_HPP
