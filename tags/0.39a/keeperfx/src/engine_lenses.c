/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_lenses.c
 *     Defines and processes camera lense effects.
 * @par Purpose:
 *     Support of camera lense effect.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 30 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engine_lenses.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"

#include "engine_render.h"
#include "engine_camera.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
Perspect_Func perspective_routines[] = {
  perspective_standard,
  perspective_standard,
  perspective_standard,
  perspective_fisheye,
};

RotPers_Func rotpers_routines[] = {
  rotpers_parallel,
  rotpers_standard,
  rotpers_circular,
  rotpers_fisheye,
};

unsigned int eye_lens_width = 0;
unsigned int eye_lens_height = 0;

/******************************************************************************/
DLLIMPORT void _DK_perspective_standard(struct XYZ *cor, struct PolyPoint *ppt);
DLLIMPORT void _DK_perspective_fisheye(struct XYZ *cor, struct PolyPoint *ppt);
DLLIMPORT void _DK_rotpers_parallel(struct EngineCoord *epos, struct M33 *matx);
DLLIMPORT void _DK_rotpers_standard(struct EngineCoord *epos, struct M33 *matx);
DLLIMPORT void _DK_rotpers_circular(struct EngineCoord *epos, struct M33 *matx);
DLLIMPORT void _DK_rotpers_fisheye(struct EngineCoord *epos, struct M33 *matx);

/******************************************************************************/
void perspective_standard(struct XYZ *cor, struct PolyPoint *ppt)
{
  long i;
  if (cor->z >= 32)
  {
    i = (lens<<16)/(cor->z);
    ppt->field_0 = view_width_over_2 + (i * cor->x >> 16);
    ppt->field_4 = view_height_over_2 - (i * cor->y >> 16);
  } else
  {
    ppt->field_0 = view_width_over_2 + cor->x;
    ppt->field_4 = view_height_over_2 - cor->y;
  }
}

void perspective_fisheye(struct XYZ *cor, struct PolyPoint *ppt)
{ }

void rotpers_parallel(struct EngineCoord *epos, struct M33 *matx)
{
  _DK_rotpers_parallel(epos, matx);
}

void rotpers_standard(struct EngineCoord *epos, struct M33 *matx)
{
  _DK_rotpers_standard(epos, matx);
}

void rotpers_circular(struct EngineCoord *epos, struct M33 *matx)
{
  _DK_rotpers_circular(epos, matx);
}

void rotpers_fisheye(struct EngineCoord *epos, struct M33 *matx)
{
  _DK_rotpers_fisheye(epos, matx);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
