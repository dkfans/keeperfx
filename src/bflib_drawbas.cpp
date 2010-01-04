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

#include <string.h>
#include <stdio.h>

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

TDDrawBaseClass::TDDrawBaseClass(void)
{
  WNDCLASSA WndClass;
  this->flags = 0;
  set_double_buffering_video(false);
  this->hWindow = NULL;
  this->active = true;
  set_wscreen_in_video(false);
  this->flags &= 0xFFFFFBFFu;
  this->flags |= 0x40u;
  this->flags |= 0x80u;
  this->flags |= 0x100u;
  this->flags |= 0x200u;
  this->appName = lbDrawAreaTitle;
  this->appTitle = lbDrawAreaTitle;
  WndClass.style = 11;
  WndClass.cbClsExtra = 0;
  this->flags &= 0xFFFFF7FFu;
  WndClass.cbWndExtra = 0;
  WndClass.lpfnWndProc = WndProc;
  this->flags &= 0xFFFFEFFF;
  WndClass.hInstance = lbhInstance;
  WndClass.hIcon = LoadIconA(lbhInstance, (LPCSTR)0x7F00);
  WndClass.hCursor = LoadCursorA(0, (LPCSTR)0x7F00);
  WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  WndClass.lpszMenuName = this->appName;
  WndClass.lpszClassName = this->appName;
  RegisterClass(&WndClass);
  lpDDC = this;
}

TDDrawBaseClass::~TDDrawBaseClass(void)
{
  lpDDC = NULL;
}

void TDDrawBaseClass::LoresEmulation(bool nstate)
{
}

/**
 *  Static callback, used only as a wrapper to call virtual WindowProc() method.
 */
long CALLBACK TDDrawBaseClass::WndProc(HWND hWnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
  try {
    return lpDDC->WindowProc(hWnd, message, wParam, lParam);
  } catch (...)
  {
    return 0;
  }		
}

bool TDDrawBaseClass::is_double_buffering_video(void)
{
  return ((flags & 0x0004) != 0);
}

bool TDDrawBaseClass::set_double_buffering_video(bool nstate)
{
  if (nstate)
    flags |= 0x0004;
  else
    flags ^= flags & 0x0004;
  return ((flags & 0x0004) != 0);
}

bool TDDrawBaseClass::is_wscreen_in_video(void)
{
  return ((flags & 0x0008) != 0);
}

bool TDDrawBaseClass::set_wscreen_in_video(bool nstate)
{
  if (nstate)
    flags |= 0x0008;
  else
    flags ^= flags & 0x0008;
  return ((flags & 0x0008) != 0);
}

bool TDDrawBaseClass::IsActive(void)
{
  return this->active;
}

void TDDrawBaseClass::SetIcon(void)
{
}
/******************************************************************************/
