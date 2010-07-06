/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_threadcond.hpp
 *     Header file for bflib_threadcond.cpp.
 * @par Purpose:
 *     For controlling execution of a child thread from a parent thread.
 * thread.
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

#ifndef BFLIB_THREADCOND_HPP
#define BFLIB_THREADCOND_HPP

#include <SDL_thread.h>

class ThreadCond
{
private:
	SDL_mutex * const mutex;
	SDL_cond * const cond;
	bool exit; //shared
	bool locked; //child thread
public:
	ThreadCond();
	~ThreadCond();

	void reset(); //parent thread
	void signalExit(); //parent thread
	void waitMs(unsigned long ms); //child thread
	void lock(); //child thread
	void unlock(); //child thread
	bool shouldExit(); //child thread
};

#endif //BFLIB_THREADCOND_HPP
