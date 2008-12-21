/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 25 Nov 2008

// Purpose:
//    Definition of button, and common routines to handle it.

// Comment:
//   Sound and music routines to use in games.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#include "bflib_guibtns.h"

#include <string.h>
#include <stdio.h>

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

DLLIMPORT void _DK_do_button_click_actions(struct GuiButton *gbtn, unsigned char *, Gf_Btn_Callback callback);
DLLIMPORT void _DK_do_button_release_actions(struct GuiButton *gbtn, unsigned char *, Gf_Btn_Callback callback);
/******************************************************************************/
// Global variables
/******************************************************************************/
// Functions

void do_button_click_actions(struct GuiButton *gbtn, unsigned char *s, Gf_Btn_Callback callback)
{
  _DK_do_button_click_actions(gbtn, s, callback);
}

void do_button_release_actions(struct GuiButton *gbtn, unsigned char *s, Gf_Btn_Callback callback)
{
  _DK_do_button_release_actions(gbtn, s, callback);
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
