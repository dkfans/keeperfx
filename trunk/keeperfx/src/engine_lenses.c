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
#include "bflib_math.h"

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
#ifdef __cplusplus
}
#endif
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

void pers_set_transform_matrix(struct EngineCoord *epos, const struct M33 *matx)
{
    long px, py, pz;
    px = epos->x;
    py = epos->y;
    pz = epos->z;
    long long pxpy, pyr0, pxr1, pzr2;
    pxpy = px * py;
    pyr0 = py + matx->r0[0];
    pxr1 = px + matx->r0[1];
    pzr2 = pz * matx->r0[2];
    epos->x = object_origin.x + ((pzr2 + pyr0 * pxr1 - matx->r0[3] - pxpy) >> 14);
    pyr0 = py + matx->r1[0];
    pxr1 = px + matx->r1[1];
    pzr2 = pz * matx->r1[2];
    epos->y = object_origin.y + ((pzr2 + pyr0 * pxr1 - matx->r1[3] - pxpy) >> 14);
    pyr0 = py + matx->r2[0];
    pxr1 = px + matx->r2[1];
    pzr2 = pz * matx->r2[2];
    epos->z = object_origin.z + ((pzr2 + pyr0 * pxr1 - matx->r2[3] - pxpy) >> 14);
}

void pers_set_view_width(struct EngineCoord *epos, long len)
{
    epos->view_width = len;
    if (epos->view_width < 0) {
        epos->field_8 |= 0x0008;
    } else
    if (epos->view_width >= vec_window_width) {
        epos->field_8 |= 0x0010;
    }
}

void pers_set_view_height(struct EngineCoord *epos, long len)
{
    epos->view_height = len;
    if (epos->view_height < 0) {
        epos->field_8 |= 0x0020;
    } else
    if (epos->view_height >= vec_window_height) {
        epos->field_8 |= 0x0040;
    }
}

void rotpers_parallel(struct EngineCoord *epos, const struct M33 *matx)
{
    long tx,ty,tz;
    long zoom;
    //_DK_rotpers_parallel(epos, matx);
    pers_set_transform_matrix(epos, matx);
    zoom = camera_zoom / pixel_size;
    tx = view_width_over_2 + ((epos->x * zoom) >> 16);
    ty = view_height_over_2 - ((epos->y * zoom) >> 16);
    tz = (epos->z + (cells_away << 8)) / 2;
    if (tz < 32) {
        tz = 0;
    } else
    if (tz >= 11232) {
        tz = 11232;
    }
    epos->view_width = tx;
    epos->view_height = ty;
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

void rotpers_standard(struct EngineCoord *epos, const struct M33 *matx)
{
    long tx,ty,tz;
    //_DK_rotpers_standard(epos, matx);
    pers_set_transform_matrix(epos, matx);
    tx = epos->x;
    ty = epos->y;
    tz = epos->z;
    epos->field_C = tz;
    if (tz > fade_max) {
      epos->field_8 |= 0x0080;
    }
    long long wx, wy;
    if (tz < 32)
    {
        epos->field_8 |= 0x0100;
        wx = tx * (1 << 16);
        wy = ty * (1 << 16);
        epos->field_8 |= 0x0001;
        epos->field_8 |= 0x0002;
    } else
    {
        wx = tx * (lens << 16) / tz;
        wy = ty * (lens << 16) / tz;
        if (tz < split_1)
        {
            epos->field_8 |= 0x0001;
            if (tz < split_2) {
                epos->field_8 |= 0x0002;
            }
        }
    }
    pers_set_view_width(epos, view_width_over_2 + (wx >> 16));
    pers_set_view_height(epos, view_height_over_2 - (wy >> 16));
    epos->field_8 |= 0x0400;
}

void rotpers_circular(struct EngineCoord *epos, const struct M33 *matx)
{
    //_DK_rotpers_circular(epos, matx);
    pers_set_transform_matrix(epos, matx);
    long tx, ty, tz;
    tx = epos->x;
    ty = epos->y;
    tz = epos->z;
    epos->field_C = abs(tx) + abs(ty) + tz;
    if (tz > fade_max) {
      epos->field_8 |= 0x0080;
    }
    long long wx, wy;
    if (tz < 32)
    {
        epos->field_8 |= 0x0100;
        wx = tx * (8 << 16);
        wy = ty * (8 << 16);
        epos->field_8 |= 0x0001;
        epos->field_8 |= 0x0002;
    } else
    {
        long adheight;
        adheight = (lens << 16) / tz;
        if (tz < split_1)
        {
            epos->field_8 |= 0x0001;
            if (tz < split_2) {
              epos->field_8 |= 0x0002;
            }
        }
        wx = tx * adheight;
        wy = ty * adheight;
    }
    pers_set_view_width(epos, view_width_over_2 + (wx >> 16));
    pers_set_view_height(epos, view_height_over_2 - (wy >> 16));
    epos->field_8 |= 0x0400;
}

void rotpers_fisheye(struct EngineCoord *epos, const struct M33 *matx)
{
    //_DK_rotpers_fisheye(epos, matx);
    pers_set_transform_matrix(epos, matx);
    long tx, ty, tz;
    tx = epos->x;
    ty = epos->y;
    tz = epos->z;
    long txz;
    txz = LbDiagonalLength(abs(tx), abs(tz));
    epos->field_C = abs(LbDiagonalLength(abs(txz), abs(ty)));
    if (epos->field_C > fade_max) {
        epos->field_8 |= 0x0080;
    }
    long long wx, wy;
    if ((tz < 32) || (epos->field_C < 32))
    {
        epos->field_8 |= 0x0100;
        wx = tx * (1 << 16);
        wy = ty * (1 << 16);
    } else
    {
        wx = tx * (lens << 16) / epos->field_C;
        wy = ty * (lens << 16) / epos->field_C;
    }
    pers_set_view_width(epos, view_width_over_2 + (wx >> 16));
    pers_set_view_height(epos, view_height_over_2 - (wy >> 16));
    if (tz < split_1) {
        epos->field_8 |= 0x0001;
        if (tz < split_2) {
            epos->field_8 |= 0x0002;
        }
    }
    epos->field_8 |= 0x0400;
}
/******************************************************************************/
