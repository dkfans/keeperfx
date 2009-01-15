/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file kjm_input.c
 *     Keyboard-Joypad-Mouse input routines.
 * @par Purpose:
 *     Allows reading state of input devices.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 30 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "kjm_input.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_mouse.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/*
 * Returns X position of mouse cursor on screen.
 */
long GetMouseX(void)
{
  return lbDisplay.MMouseX * pixel_size;
}

/*
 * Returns Y position of mouse cursor on screen.
 */
long GetMouseY(void)
{
  //return _DK_GetMouseY();
  long result;
  result = lbDisplay.MMouseY * pixel_size;
/*  if ((lbDisplay.ScreenMode == 13) && (!MinimalResolutionSetup))
  {
      result -= 40;
      if (result < 0)
        result = 0;
  }*/
  return result;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
