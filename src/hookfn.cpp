/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file hookfn.cpp
 * @author   KeeperFX Team
 * @date     01 Aug 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "pre_inc.h"
#include <assert.h>
#include <windows.h>
#include <stdint.h>
#include "post_inc.h"

extern "C" void replaceFn(void* oldFn, void* newFn)
{
	// E9 00000000   jmp rel  displacement relative to next instruction
	unsigned char codeBytes[5] = {0xE9, 0x00, 0x00, 0x00, 0x00};
	uintptr_t p = (uintptr_t)newFn - (uintptr_t)oldFn - sizeof(codeBytes);
	memcpy(&codeBytes[1], &p, sizeof(p));

	SIZE_T bytesWritten = 0;
	BOOL res = WriteProcessMemory(GetCurrentProcess(),
	    oldFn, codeBytes, sizeof(codeBytes), &bytesWritten);
	assert(res);
	assert(bytesWritten == sizeof(codeBytes));
}