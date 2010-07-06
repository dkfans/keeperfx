/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_threadcond.cpp
 *     ThreadCond class implementation.
 * @par Purpose:
 *     For controlling execution of a child thread from a parent thread.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 April 2010 - ?
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "bflib_threadcond.hpp"

#include <cassert>

ThreadCond::ThreadCond() :
		mutex(SDL_CreateMutex()),
		cond(SDL_CreateCond()),
		exit(false),
		locked(false)
{
}

ThreadCond::~ThreadCond()
{
	SDL_DestroyCond(cond);
	SDL_DestroyMutex(mutex);
}

void ThreadCond::reset()
{
	assert(!locked);
	exit = false;
}

void ThreadCond::signalExit()
{
	SDL_LockMutex(mutex);
	exit = true;
	SDL_UnlockMutex(mutex);
}
void ThreadCond::waitMs(unsigned long ms)
{
	assert(locked);
	SDL_CondWaitTimeout(cond, mutex, ms);
}

void ThreadCond::lock()
{
	assert(!locked);
	locked = true;
	SDL_LockMutex(mutex);
}

void ThreadCond::unlock()
{
	assert(locked);
	locked = false;
	SDL_UnlockMutex(mutex);
}

bool ThreadCond::shouldExit()
{
	assert(locked);
	return exit;
}
