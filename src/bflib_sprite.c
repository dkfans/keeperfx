/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_guibtns.c
 *     Graphics sprites support library.
 * @par Purpose:
 *     Functions for reading/writing, decoding/encodeing of sprites.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Nov 2008 - 09 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_sprite.h"

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
DLLIMPORT int _DK_LbSpriteSetupAll(struct TbSetupSprite t_setup[]);
DLLIMPORT int _DK_LbSpriteSetup(TbSprite *start, const TbSprite *end, const char *data);

/******************************************************************************/
short LbSpriteSetup(struct TbSprite *start, const struct TbSprite *end, const char *data)
{
  static const char *func_name="LbSpriteSetup";
  //return _DK_LbSpriteSetup(start, end, data);
  TbSprite *sprt=start;
  while ( sprt < end )
  {
    if ( sprt->Data < data )
      sprt->Data += (unsigned long)data;
    sprt++;
  }
#ifdef __DEBUG
  LbSyncLog("%s: initied %d sprites\n",func_name,(sprt-start));
#endif
  return 1;
}

int LbSpriteSetupAll(struct TbSetupSprite t_setup[])
{
  return _DK_LbSpriteSetupAll(t_setup);
}

/*
int __fastcall LbSpriteSetupAll(struct TbSetupSprite t_setup[])
{
  int idx=0;
  TbSetupSprite *stp_sprite;
  stp_sprite=&t_setup[idx];
  while ( stp_sprite->Data != NULL )
  {
    LbSpriteSetup(*(stp_sprite->Start), *(stp_sprite->End), *(stp_sprite->Data));
    idx++;
    stp_sprite=&t_setup[idx];
  }
#ifdef __DEBUG
  LbSyncLog("LbSpriteSetupAll: initied %d SetupSprite lists\n",idx);
#endif
  return 1;
}
*/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
