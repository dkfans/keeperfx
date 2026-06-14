/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_text.h
 *     Text routines header.
 * @par Purpose:
 *     Text conversion helper declarations.
 * @par Comment:
 *     Provides UTF-8 to internal codepage conversion for text rendering.
 * @author   Tomasz Lis
 * @date     2026-06-13
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/*******************************************************************************/
#ifndef BFLIB_TEXT_H
#define BFLIB_TEXT_H

#include "bflib_basics.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

uint32_t read_utf_8_codepoint(const char *text, size_t *out_seq_len);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
