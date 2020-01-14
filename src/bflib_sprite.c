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
short LbSpriteSetup(struct TbSprite *start, const struct TbSprite *end, const unsigned char * data)
{
    int n = 0;
    struct TbSprite* sprt = start;
    while (sprt < end)
    {
      if ((unsigned long)sprt->Data < (unsigned long)data)
      {
        sprt->Data += (unsigned long)data;
        n++;
      }
      sprt++;
    }
#ifdef __DEBUG
    LbSyncLog("%s: initied %d of %d sprites\n",func_name,n,(sprt-start));
#endif
    return 1;
}

int LbSpriteSetupAll(struct TbSetupSprite t_setup[])
{
    int idx = 0;
    struct TbSetupSprite* stp_sprite = &t_setup[idx];
    while (stp_sprite->Data != NULL)
    {
      if ((stp_sprite->Start != NULL) && (stp_sprite->End != NULL))
        LbSpriteSetup(*(stp_sprite->Start), *(stp_sprite->End), (unsigned char *)*(stp_sprite->Data));
      idx++;
      stp_sprite=&t_setup[idx];
    }
#ifdef __DEBUG
    LbSyncLog("%s: Initiated %d SetupSprite lists\n",func_name,idx);
#endif
    return 1;
}

int LbSpriteClearAll(struct TbSetupSprite t_setup[])
{
    int idx = 0;
    struct TbSetupSprite* stp_sprite = &t_setup[idx];
    while (stp_sprite->Data != NULL)
    {
        if ((stp_sprite->Start != NULL) && (stp_sprite->End != NULL))
        {
            *(stp_sprite->Start) = NULL;
            *(stp_sprite->End) = NULL;
            *(stp_sprite->Data) = 0;
        }
        idx++;
        stp_sprite = &t_setup[idx];
  }
#ifdef __DEBUG
  LbSyncLog("%s: Cleaned %d SetupSprite lists\n",func_name,idx);
#endif
  return 1;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
