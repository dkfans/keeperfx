/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file kjm_input.h
 *     Header file for kjm_input.c.
 * @par Purpose:
 *     Keyboard-Joypad-Mouse input routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 30 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_KJMINPUT_H
#define DK_KJMINPUT_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

DLLIMPORT extern unsigned long _DK_key_modifiers;
#define key_modifiers _DK_key_modifiers
DLLIMPORT extern int _DK_defining_a_key;
#define defining_a_key _DK_defining_a_key
DLLIMPORT extern long _DK_defining_a_key_id;
#define defining_a_key_id _DK_defining_a_key_id
/******************************************************************************/

#pragma pack()
/******************************************************************************/
long GetMouseX(void);
long GetMouseY(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
