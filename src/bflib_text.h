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

size_t convert_codepage_to_utf8_buffer(const char *src, size_t src_size, char *dst, size_t dst_size, uint8_t lang_id);
#define read_utf_8_codepoint(text, out_seq_len) read_utf_8_codepoint_f(text, out_seq_len,__func__)
uint32_t read_utf_8_codepoint_f(const char *text, size_t *out_seq_len, const char *func_name);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
