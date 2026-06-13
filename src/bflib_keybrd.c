/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_keybrd.c
 *     Keyboard related routines - reading keyboard.
 * @par Purpose:
 *     Wrapper for keyboard support.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Feb 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_keybrd.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "globals.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

static unsigned char lbInkeyFlags;
static unsigned char lbIInkeyFlags;
static unsigned char lbIInkey;
static unsigned char lbExtendedKeyPress;
unsigned char lbKeyOn[KC_LIST_END];
TbKeyCode lbInkey;

/******************************************************************************/
extern void init_inputcontrol(void);
/******************************************************************************/
/******************************************************************************/
short LbIKeyboardClose(void)
{
  return 1;
}

short LbIKeyboardOpen(void)
{
  init_inputcontrol();
    return 1;
}

void keyboardControl(unsigned int action, TbKeyCode code, TbKeyMods modifiers, int ScanCode)
{
    // Set the key code action value
    switch ( action )
    {
    case KActn_KEYDOWN:
        lbKeyOn[code] = 1;
        lbInkey = code;
        break;
    case KActn_KEYUP:
    default:
        lbKeyOn[code] = 0;
        lbExtendedKeyPress = 0;
        break;
    }
    // Check for undetected/incorrectly maintained modifiers
    if (modifiers != KMod_DONTCARE)
    {
        // If modifiers were supplied, make sure they are correctly set in lbKeyOn[]
        if (modifiers & KMod_SHIFT)
        {
            if (!lbKeyOn[KC_RSHIFT] && !lbKeyOn[KC_LSHIFT])
                lbKeyOn[KC_LSHIFT] = 1;
        } else
        {
            lbKeyOn[KC_LSHIFT] = 0;
            lbKeyOn[KC_RSHIFT] = 0;
        }
        if (modifiers & KMod_CONTROL)
        {
            if (!lbKeyOn[KC_RCONTROL] && !lbKeyOn[KC_LCONTROL])
                lbKeyOn[KC_LCONTROL] = 1;
        } else
        {
            lbKeyOn[KC_LCONTROL] = 0;
            lbKeyOn[KC_RCONTROL] = 0;
        }
        if (modifiers & KMod_ALT)
        {
            if (!lbKeyOn[KC_RALT] && !lbKeyOn[KC_LALT])
                lbKeyOn[KC_LALT] = 1;
        } else
        {
            lbKeyOn[KC_LALT] = 0;
            lbKeyOn[KC_RALT] = 0;
        }
    }
    // Update modifiers flags
    lbInkeyFlags = 0;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        lbInkeyFlags |= KMod_SHIFT;
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL])
        lbInkeyFlags |= KMod_CONTROL;
    if (lbKeyOn[KC_LALT] || lbKeyOn[KC_RALT])
        lbInkeyFlags |= KMod_ALT;
    if (lbKeyOn[code] != 0)
        lbKeyOn[code] |= lbInkeyFlags;
    if (lbInkey < 0x80)
    {
        if (lbIInkey == 0)
        {
            lbIInkey = ScanCode;
            lbIInkeyFlags = lbInkeyFlags;
        }
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
