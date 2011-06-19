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
    long sx,sy,sz;
    long tx,ty,tz;
    long long val;
    long zoom;
    //_DK_rotpers_parallel(epos, matx);
    zoom = camera_zoom / pixel_size;
    sx = epos->y;
    sy = epos->x;
    sz = epos->z;
    val = sz * matx->r0[2] + (sx + matx->r0[0]) * (sy + matx->r0[1]) - matx->r0[3] - (sy * sx);
    tx = (val >> 14);
    val = sz * matx->r1[2] + (sy + matx->r1[1]) * (sx + matx->r1[0]) - matx->r1[3] - (sy * sx);
    ty = (val >> 14);
    val = sz * matx->r2[2] + (sy + matx->r2[1]) * (sx + matx->r2[0]) - matx->r2[3] - (sy * sx);
    tz = (val >> 14);
    tx += object_origin.x;
    ty += object_origin.y;
    tz += object_origin.z;
    epos->x = tx;
    epos->y = ty;
    epos->z = tz;
    tx = view_width_over_2 + ((tx * zoom) >> 16);
    ty = view_height_over_2 - ((ty * zoom) >> 16);
    tz = (tz + (cells_away << 8)) / 2;
    if (tz < 32) {
        tz = 0;
    } else
    if (tz >= 11232) {
        tz = 11232;
    }
    epos->field_0 = tx;
    epos->field_4 = ty;
    epos->z = tz;
    if (tx < 0) {
        epos->field_8 |= 0x08;
    } else
    if (tx >= vec_window_width) {
        epos->field_8 |= 0x10;
    }
    if (ty < 0) {
        epos->field_8 |= 0x20;
    } else
    if (ty >= vec_window_height) {
        epos->field_8 |= 0x40;
    }
}

void rotpers_standard(struct EngineCoord *epos, struct M33 *matx)
{
    long sx,sy,sz;
    long tx,ty,tz;
    long long val,mval;
    long zoom;
    //_DK_rotpers_standard(epos, matx);
    zoom = camera_zoom / pixel_size;
    sx = epos->y;
    sy = epos->x;
    sz = epos->z;
    mval = sy * epos->x;
    val = sz * matx->r0[2] + (sy + matx->r0[0]) * (sx + matx->r0[1]) - matx->r0[3] - mval;
    tx = (val >> 14);
    val = sz * matx->r1[2] + (sy + matx->r1[0]) * (sx + matx->r1[1]) - matx->r1[3] - mval;
    ty = (val >> 14);
    val = sz * matx->r2[2] + (sy + matx->r2[0]) * (sx + matx->r2[1]) - matx->r2[3] - mval;
    tz = (val >> 14);
    tx += object_origin.x;
    ty += object_origin.y;
    tz += object_origin.z;
    epos->x = tx;
    epos->y = ty;
    epos->z = tz;
    epos->field_C = tz;
    if (tz > fade_max) {
      epos->field_8 |= 0x80;
    }
    if (tz < 32)
    {
        epos->field_8 |= 0x0100;
        tx = view_width_over_2 + tx;
        ty = view_height_over_2 - ty;
        epos->field_8 |= 0x01;
        epos->field_8 |= 0x02;
    } else
    {
        sx = tx * (lens << 16) / tz >> 16;
        sy = ty * (lens << 16) / tz >> 16;
        tx = view_width_over_2 + sx;
        ty = view_height_over_2 - sy;
        if (tz < split_1) {
          epos->field_8 |= 0x01;
          if (tz < split_2) {
              epos->field_8 |= 0x02;
          }
        }
    }
    epos->field_0 = tx;
    epos->field_4 = ty;
    epos->z = tz;
    if (tx < 0) {
        epos->field_8 |= 0x08;
    } else
    if (tx >= vec_window_width) {
        epos->field_8 |= 0x10;
    }
    if (ty < 0) {
        epos->field_8 |= 0x20;
    } else
    if (ty >= vec_window_height) {
        epos->field_8 |= 0x40;
    }
    epos->field_8 |= 0x04;
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
