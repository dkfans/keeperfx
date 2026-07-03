/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_fmvids.h
 *     Header file for bflib_fmvids.c.
 * @par Purpose:
 *     Full Motion Videos (Smacker,FLIC) decode & play library.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     27 Nov 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_FMVIDS_H
#define BFLIB_FMVIDS_H

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

enum SmackerPlayFlags {
    SMK_NoSound            = 0x01,
    SMK_NoStopOnUserInput  = 0x02, // unused?
    SMK_PixelDoubleLine    = 0x04, // unused?
    SMK_InterlaceLine      = 0x08, // unused?
    SMK_FullscreenFit      = 0x10,
    SMK_FullscreenStretch  = 0x20,
    SMK_FullscreenCrop     = 0x40,
    SMK_PixelDoubleWidth   = 0x80, // unused?
    SMK_WriteStatusFile    = 0x100, // was 0x40, unused?
};

TbBool play_smk(const char * filename, int flags);
short anim_stop(void);
short anim_record(void);
TbBool anim_record_frame(unsigned char * screenbuf, unsigned char * palette);

#ifdef __cplusplus
}
#endif
#endif
