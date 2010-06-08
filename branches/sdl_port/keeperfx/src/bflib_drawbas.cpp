/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_drawbas.cpp
 *     Graphics drawing support base class.
 * @par Purpose:
 *     A link between game engine and the DirectDraw library.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     16 Nov 2008 - 21 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_drawbas.hpp"

#include <cstring>
#include <cstdio>

#include "bflib_basics.h"

/******************************************************************************/
// Global variables
class TDDrawBaseClass *lpDDC;
char lbDrawAreaTitle[128] = "Bullfrog Shell";
volatile HINSTANCE lbhInstance;
volatile TbBool lbInteruptMouse;
volatile unsigned long lbIconIndex = 0;
/******************************************************************************/
// Base class methods

TDDrawBaseClass::TDDrawBaseClass(void) :
		active(true)
{
}

TDDrawBaseClass::~TDDrawBaseClass(void)
{
  lpDDC = NULL;
}

bool TDDrawBaseClass::isActive(void)
{
  return active;
}

/******************************************************************************/
