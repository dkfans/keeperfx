/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 16 Nov 2008

// Purpose:
//    Graphics drawing support class.

// Comment:
//   A link between game engine and the DirectDraw library.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#include "bflib_drawcls.h"

#include <string.h>
#include <stdio.h>

#include "bflib_basics.h"

/******************************************************************************/
// Global variables
long lbDDRval;
volatile int lbWait;
/******************************************************************************/
// Methods

TDDrawSdk::TDDrawSdk(void) : TDDrawBaseClass()
{
  this->field_190 = 0;
  this->field_194 = 0;
  this->field_20 = 0;
  this->field_24 = 0;
  this->field_28 = 0;
  this->field_2C = 0;
  this->field_174 = 0;
  this->field_178 = 0;
  this->field_17C = 0;
  this->field_180 = 0;
  this->numfield_18 = 0;
  this->field_184 = 0;
  this->unkvar4 = 1;
  this->field_188 = 0;
  this->flags = 0;
}

virtual TDDrawSdk::~TDDrawSdk(void)
{
  if ( this->field_190 )
  {
    if (this->flags & 0x01)
    {
      if ( this->field_28 )
      {
        while ( (*(int (__stdcall **)(int))(*(_DWORD *)this->field_28 + 8))(this->field_28) )
          ;
        this->field_28 = 0;
      }
      v3 = this->field_2C;
      if ( v3 )
      {
        (*(int (__stdcall **)(int))(*(_DWORD *)v3 + 8))(this->field_2C);
        this->field_2C = 0;
      }
    }
    if ( this->numfield_1C )
    {
      v5 = this->flags & 0xFFFFFFFD;
      this->flags &= 0xFFFFFFFDu;
      if ( v5 & 1 )
      {
        if ( this->field_194 )
        {
          while ( (*(int (__stdcall **)(int))(*(_DWORD *)this->field_194 + 8))(this->field_194) )
            ;
          this->field_194 = 0;
        }
        if ( this->field_24 )
        {
          while ( (*(int (__stdcall **)(int))(*(_DWORD *)this->field_24 + 8))(this->field_24) )
            ;
          this->field_24 = 0;
        }
      }
    }
    (*(int (__stdcall **)(int, int, signed int))(*(_DWORD *)this->field_190 + 80))(
      this->field_190,
      this->numfield_18,
      8);
    (*(int (__stdcall **)(int))(*(_DWORD *)this->field_190 + 8))(this->field_190);
    this->field_190 = 0;
  }
  if (this->field_184)
  {
    SendMessageA(this->numfield_18, 2, 0, 0);
    LbWarnLog("closing down Sdk Window.\n");
    if (WaitForSingleObject(this->field_188, 10000) == -1)
    {
      LbWarnLog(" Timed out waiting for Sdk thread to terminate.\n");
      TerminateThread(this->field_188, 0);
    }
    CloseHandle(this->field_188);
    this->field_188 = 0;
    this->field_184 = 0;
    this->numfield_18 = 0;
  }
  lpDDC = 0;
}

//TODO: check with beta

int TDDrawSdk::reset_direct_draw(void)
{
  LPARAM v8; // esi@35
  int v9; // ecx@36
  int v10; // edi@37
  int v11; // ST04_4@37
  int v12; // ST08_4@37
  int v13; // eax@42
  int v14; // eax@45


    v8 = lParam;
    if ( *(_BYTE *)(lParam + 16) & 1 )
    {
      v9 = *(_DWORD *)(lParam + 400);
      if ( v9 )
      {
        v12 = *(_DWORD *)(lParam + 24);
        v10 = *(_DWORD *)v9;
        v11 = *(_DWORD *)(lParam + 400);
        if ( *(_BYTE *)(lParam + 370) & 0x20 )
        {
          (*(int (__stdcall **)(int, int, signed int))(v10 + 80))(v11, v12, 8);
          if ( *(_BYTE *)(v8 + 16) & 1 )
          {
            if ( *(_DWORD *)(v8 + 40) )
            {
              while ( (*(int (__stdcall **)(_DWORD))(**(_DWORD **)(v8 + 40) + 8))(*(_DWORD *)(v8 + 40)) )
                ;
              *(_DWORD *)(v8 + 40) = 0;
            }
            v13 = *(_DWORD *)(v8 + 44);
            if ( v13 )
            {
              (*(int (__stdcall **)(_DWORD))(*(_DWORD *)v13 + 8))(*(_DWORD *)(v8 + 44));
              *(_DWORD *)(v8 + 44) = 0;
            }
          }
          if ( *(_DWORD *)(v8 + 28) )
          {
            v14 = *(_DWORD *)(v8 + 16) & 0xFFFFFFFD;
            *(_DWORD *)(v8 + 16) &= 0xFFFFFFFDu;
            if ( v14 & 1 )
            {
              if ( *(_DWORD *)(v8 + 404) )
              {
                while ( (*(int (__stdcall **)(_DWORD))(**(_DWORD **)(v8 + 404) + 8))(*(_DWORD *)(v8 + 404)) )
                  ;
                *(_DWORD *)(v8 + 404) = 0;
              }
              if ( *(_DWORD *)(v8 + 36) )
              {
                while ( (*(int (__stdcall **)(_DWORD))(**(_DWORD **)(v8 + 36) + 8))(*(_DWORD *)(v8 + 36)) )
                  ;
                *(_DWORD *)(v8 + 36) = 0;
              }
            }
          }
          (*(int (__stdcall **)(_DWORD))(**(_DWORD **)(v8 + 400) + 8))(*(_DWORD *)(v8 + 400));
          *(_DWORD *)(v8 + 400) = 0;
        }
        else
        {
          (*(int (__stdcall **)(int, int, signed int))(v10 + 80))(v11, v12, 8);
        }
      }
      *(_DWORD *)(v8 + 16) &= 0xFFFFFFFEu;
    }
}

int TDDrawSdk::WindowProc(struct TDDrawSdk *this, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  struct tagPOINT mouse_pos;
  switch (Msg)
  {
  case 16:
      lbUserQuit = 1;
      return 0;
  case 2:
      this->field_184 = 0;
      PostQuitMessage(0);
      return DefWindowProcA(hWnd, Msg, wParam, lParam);
  case 28:
      this->numfield_1C = (wParam) && (GetForegroundWindow() == hWnd);
      return DefWindowProcA(hWnd, Msg, wParam, lParam);
  case 32:
      if (this->numfield_1C)
      {
        SetCursor(0);
        return 1;
      }
      return DefWindowProcA(hWnd, Msg, wParam, lParam);
  case 256:
  case 257:
  case 260:
  case 261:
      KeyboardProc(0, 0, lParam);
      return 0;
  case 1124:
      if (!setup_direct_draw())
        lbDDRval = -2147467259;
      lbWait = 0;
      return 0;
  case 512:
  case 513:
  case 514:
  case 515:
  case 516:
  case 517:
  case 518:
  case 519:
  case 520:
  case 521:
      mouse_pos.x = lParam & 0xFFFF;
      mouse_pos.y = lParam >> 16;
      mouseControl(Msg, &mouse_pos);
      return DefWindowProcA(hWnd, Msg, wParam, lParam);
  case 1126:
      if ( !setup_surfaces(
                (__int16)this,
                *(&lbScreenModeInfo_0.Width + 19 * *(_DWORD *)wParam),
                *(&lbScreenModeInfo_0.Height + 19 * *(_DWORD *)wParam)) )
      lbDDRval = -2147467259;
      lbWait = 0;
      return DefWindowProcA(hWnd, Msg, wParam, lParam);
  case 1125:
      reset_direct_draw();
    lbWait = 0;
    return 0;
  default:
      return DefWindowProcA(hWnd, Msg, wParam, lParam);
  }
}

/******************************************************************************/

// Base class methods

TDDrawBaseClass::TDDrawBaseClass()
{
  this->flags = 0;
  this->numfield_14 = 0;
  this->flags &= 0xFFFFFFFBu;
  this->numfield_18 = 0;
  this->numfield_1C = 1;
  this->flags &= 0xFFFFFFF7u;
  this->flags &= 0xFFFFFBFFu;
  this->flags |= 0x40u;
  this->flags |= 0x80u;
  this->flags |= 0x100u;
  this->flags |= 0x200u;
  this->textname = window_class_name;
  this->textn2 = window_class_name;
  WndClass.style = 11;
  WndClass.cbClsExtra = 0;
  this->flags &= 0xFFFFF7FFu;
  WndClass.cbWndExtra = 0;
  WndClass.lpfnWndProc = WndProcDrawClassWrapper;
  this->flags &= 0xFFFFEFFF;
  WndClass.hInstance = hInstance;
  WndClass.hIcon = LoadIconA(hInstance, (LPCSTR)0x7F00);
  WndClass.hCursor = LoadCursorA(0, (LPCSTR)0x7F00);
  WndClass.hbrBackground = GetStockObject(4);
  WndClass.lpszMenuName = this->textname;
  WndClass.lpszClassName = this->textname;
  RegisterClassA(&WndClass);
  lpDDC = this;
}

virtual TDDrawBaseClass::~TDDrawBaseClass()
{
  lpDDC = NULL;
}



/******************************************************************************/
