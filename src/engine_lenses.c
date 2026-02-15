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
#include "pre_inc.h"
#include "engine_lenses.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_math.h"

#include "engine_render.h"
#include "engine_camera.h"
#include "post_inc.h"

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

// Lens buffer dimensions
unsigned int eye_lens_width = 0;
unsigned int eye_lens_height = 0;

// Shared lens buffers (allocated by LensManager)
uint32_t *eye_lens_memory = NULL;
TbPixel *eye_lens_spare_screen_memory = NULL;

long lens;
Perspect_Func perspective;
RotPers_Func rotpers;
unsigned char lens_mode;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void perspective_standard(struct XYZ *cor, struct PolyPoint *ppt)
{
  if (cor->z >= 32)
  {
      long i = (lens << 16) / (cor->z);
      ppt->X = view_width_over_2 + (i * cor->x >> 16);
      ppt->Y = view_height_over_2 - (i * cor->y >> 16);
  } else
  {
    ppt->X = view_width_over_2 + cor->x;
    ppt->Y = view_height_over_2 - cor->y;
  }
}

void perspective_fisheye(struct XYZ *cor, struct PolyPoint *ppt)
{ }

void pers_set_transform_matrix(struct EngineCoord *epos, const struct M33 *matx)
{
    long px = epos->x;
    long py = epos->y;
    long pz = epos->z;
    long long pxpy = px * py;
    long long pyr0 = py + matx->r[0].v[0];
    long long pxr1 = px + matx->r[0].v[1];
    long long pzr2 = pz * matx->r[0].v[2];
    epos->x = object_origin.x + ((pzr2 + pyr0 * pxr1 - matx->r[0].v[3] - pxpy) >> 14);
    pyr0 = py + matx->r[1].v[0];
    pxr1 = px + matx->r[1].v[1];
    pzr2 = pz * matx->r[1].v[2];
    epos->y = object_origin.y + ((pzr2 + pyr0 * pxr1 - matx->r[1].v[3] - pxpy) >> 14);
    pyr0 = py + matx->r[2].v[0];
    pxr1 = px + matx->r[2].v[1];
    pzr2 = pz * matx->r[2].v[2];
    epos->z = object_origin.z + ((pzr2 + pyr0 * pxr1 - matx->r[2].v[3] - pxpy) >> 14);
}

void flicker_fix(struct EngineCoord *epos) {
    // Set this value as low as possible without seeing flickers. Higher = more culling, lower = more flickers.
    int cull_nearby_z = 256;
    // Set this value as high as possible without seeing flickers. Lower = more culling, higher = more flickers.
    int cull_must_have_distant_xy = 256*3;

    if (epos->z-cull_nearby_z < 0 && abs(epos->x)+abs(epos->y) >= cull_must_have_distant_xy) {
        epos->clip_flags = 65535;
    }
}

void pers_set_view_width(struct EngineCoord *epos, long len)
{
    epos->view_width = len;
    if (epos->view_width < 0) {
        epos->clip_flags |= 0x0008;
    } else
    if (epos->view_width >= vec_window_width) {
        epos->clip_flags |= 0x0010;
    }
}

void pers_set_view_height(struct EngineCoord *epos, long len)
{
    epos->view_height = len;
    if (epos->view_height < 0) {
        epos->clip_flags |= 0x0020;
    } else
    if (epos->view_height >= vec_window_height) {
        epos->clip_flags |= 0x0040;
    }
}

void rotpers_parallel(struct EngineCoord *epos, const struct M33 *matx)
{
    pers_set_transform_matrix(epos, matx);
    long zoom = camera_zoom / pixel_size;
    long tx = view_width_over_2 + ((epos->x * zoom) >> 16);
    long ty = view_height_over_2 - ((epos->y * zoom) >> 16);
    long tz = (epos->z + (cells_away << 8)) / 2;
    epos->render_distance = COORD_PER_STL * 10;
    if (tz < 32) {
        tz = 0;
    } else
    if (tz >= Z_DRAW_DISTANCE_MAX) {
        tz = Z_DRAW_DISTANCE_MAX;
    }
    epos->view_width = tx;
    epos->view_height = ty;
    epos->z = tz;
    if (tx < 0) {
        epos->clip_flags |= 0x08;
    } else
    if (tx >= vec_window_width) {
        epos->clip_flags |= 0x10;
    }
    if (ty < 0) {
        epos->clip_flags |= 0x20;
    } else
    if (ty >= vec_window_height) {
        epos->clip_flags |= 0x40;
    }
}

void rotpers_standard(struct EngineCoord *epos, const struct M33 *matx)
{
    pers_set_transform_matrix(epos, matx);
    long tx = epos->x;
    long ty = epos->y;
    long tz = epos->z;
    epos->render_distance = tz;
    if (tz > fade_max) {
      epos->clip_flags |= 0x0080;
    }
    long long wx;
    long long wy;
    if (tz < 32)
    {
        epos->clip_flags |= 0x0100;
        wx = tx * (1 << 16);
        wy = ty * (1 << 16);
        epos->clip_flags |= 0x0001;
        epos->clip_flags |= 0x0002;
    } else
    {
        wx = tx * (lens << 16) / tz;
        wy = ty * (lens << 16) / tz;
        if (tz < z_threshold_near)
        {
            epos->clip_flags |= 0x0001;
            if (tz < split_2) {
                epos->clip_flags |= 0x0002;
            }
        }
    }
    pers_set_view_width(epos, view_width_over_2 + (wx >> 16));
    pers_set_view_height(epos, view_height_over_2 - (wy >> 16));
    epos->clip_flags |= 0x0400;
    flicker_fix(epos);
}

void rotpers_circular(struct EngineCoord *epos, const struct M33 *matx)
{
    pers_set_transform_matrix(epos, matx);
    long tx = epos->x;
    long ty = epos->y;
    long tz = epos->z;
    epos->render_distance = abs(tx) + abs(ty) + tz;
    if (tz > fade_max) {
      epos->clip_flags |= 0x0080;
    }
    long long wx;
    long long wy;
    if (tz < 32)
    {
        epos->clip_flags |= 0x0100;
        wx = tx * (8 << 16);
        wy = ty * (8 << 16);
        epos->clip_flags |= 0x0001;
        epos->clip_flags |= 0x0002;
    } else
    {
        long adheight = (lens << 16) / tz;
        if (tz < z_threshold_near)
        {
            epos->clip_flags |= 0x0001;
            if (tz < split_2) {
              epos->clip_flags |= 0x0002;
            }
        }
        wx = tx * adheight;
        wy = ty * adheight;
    }
    pers_set_view_width(epos, view_width_over_2 + (wx >> 16));
    pers_set_view_height(epos, view_height_over_2 - (wy >> 16));
    epos->clip_flags |= 0x0400;
    flicker_fix(epos);
}

void rotpers_fisheye(struct EngineCoord *epos, const struct M33 *matx)
{
    pers_set_transform_matrix(epos, matx);
    long tx = epos->x;
    long ty = epos->y;
    long tz = epos->z;
    long txz = LbDiagonalLength(abs(tx), abs(tz));
    epos->render_distance = abs(LbDiagonalLength(abs(txz), abs(ty)));
    if (epos->render_distance > fade_max) {
        epos->clip_flags |= 0x0080;
    }
    long long wx;
    long long wy;
    if ((tz < 32) || (epos->render_distance < 32))
    {
        epos->clip_flags |= 0x0100;
        wx = tx * (1 << 16);
        wy = ty * (1 << 16);
    } else
    {
        wx = tx * (lens << 16) / epos->render_distance;
        wy = ty * (lens << 16) / epos->render_distance;
    }
    pers_set_view_width(epos, view_width_over_2 + (wx >> 16));
    pers_set_view_height(epos, view_height_over_2 - (wy >> 16));
    if (tz < z_threshold_near) {
        epos->clip_flags |= 0x0001;
        if (tz < split_2) {
            epos->clip_flags |= 0x0002;
        }
    }
    epos->clip_flags |= 0x0400;
    flicker_fix(epos);
}
/******************************************************************************/
