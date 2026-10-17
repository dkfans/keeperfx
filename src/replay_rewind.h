/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file replay_rewind.h
 *     Header file for replay_rewind.c.
 * @par Purpose:
 *     Rewinding Efficiently rewinding a replay while watching it.
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_REPLAY_REWIND_H
#define DK_REPLAY_REWIND_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

// kfx_rewindable: marks a variable as belonging to a memory region which
// should be stored and unstored when rewinding a replay.
// On some platforms, kfx_rewindable can force a variable to not be in .bss,
// so to avoid inflating the binary don't use this on extremely large sections of memory (on the order of megabytes).
#if defined(_MSC_VER)
#pragma section(".rwd$a", read, write)
#pragma section(".rwd$u", read, write)
#pragma section(".rwd$z", read, write)
#define REWIND_SECTION(name) __declspec(allocate(name))
#define kfx_rewindable REWIND_SECTION(".rwd$u")
#elif defined(_WIN32)
#define REWIND_SECTION(name) __attribute__((section(name)))
#define kfx_rewindable REWIND_SECTION(".rwd$u")
#else
#define kfx_rewindable __attribute__((section("kfx_rwd")))
#endif

// kfx_align_to_page: variable placed at multiple of 4096;
// makes memory tracking more efficient
#define KFX_PAGE_ALIGN 4096
#if defined(_MSC_VER)
#define kfx_align_to_page __declspec(align(KFX_PAGE_ALIGN))
#else
#define kfx_align_to_page __attribute__((aligned(KFX_PAGE_ALIGN)))
#endif

void replay_rewind_capture(void);
TbBool replay_rewind_available(void);
void replay_rewind_step(void);
TbBool replay_rewind_at_limit(void);
size_t replay_rewind_buffer_used(void);
void replay_rewind_reset(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
