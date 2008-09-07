/******************************************************************************/
// keeperfx.h - Dungeon Keeper fan extension.
/******************************************************************************/
// Author:   Tomasz Lis
// Created:  27 May 2008

// Purpose:
//   Header file. Defines exported routines from keeperfx.dll

// Comment:
//   None.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/

#ifndef DK_KEEPERFX_H
#define DK_KEEPERFX_H

#include <windows.h>

#include "globals.h"

enum TbErrorCode {
        Lb_OK      =  0,
        Lb_FAIL    = -1,
};

typedef int TbError;

#ifdef __cplusplus
extern "C" {
#endif

//Functions - exported by the DLL

DLLIMPORT int __stdcall _DK_WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);

#ifdef __cplusplus
}
#endif

#endif // DK_KEEPERFX_H
