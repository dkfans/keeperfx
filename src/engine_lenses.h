/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_lenses.h
 *     Header file for engine_lenses.c.
 * @par Purpose:
 *     Support of camera lense effect.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 30 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_ENGNLENS_H
#define DK_ENGNLENS_H

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_video.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define PERS_ROUTINES_COUNT    4

struct XYZ;
struct PolyPoint;
struct EngineCoord;
struct M33;

typedef void (*RotPers_Func)(struct EngineCoord *epos, const struct M33 *matx);
typedef void (*Perspect_Func)(struct XYZ *cor, struct PolyPoint *ppt);

struct XYZ { // sizeof = 12
    long x;
    long y;
    long z;
};

/******************************************************************************/
DLLIMPORT unsigned char _DK_lens_mode;
#define lens_mode _DK_lens_mode
DLLIMPORT long _DK_lens;
#define lens _DK_lens
//DLLIMPORT RotPers_Func _DK_rotpers_routines[];
//#define rotpers_routines _DK_rotpers_routines
//DLLIMPORT Perspect_Func _DK_perspective_routines[];
//#define perspective_routines _DK_perspective_routines
DLLIMPORT Perspect_Func _DK_perspective;
#define perspective _DK_perspective
DLLIMPORT RotPers_Func _DK_rotpers;
#define rotpers _DK_rotpers

#pragma pack()
/******************************************************************************/
extern Perspect_Func perspective_routines[];
extern RotPers_Func rotpers_routines[];
/******************************************************************************/
void perspective_standard(struct XYZ *cor, struct PolyPoint *ppt);
void perspective_fisheye(struct XYZ *cor, struct PolyPoint *ppt);
void rotpers_parallel(struct EngineCoord *epos, const struct M33 *matx);
void rotpers_standard(struct EngineCoord *epos, const struct M33 *matx);
void rotpers_circular(struct EngineCoord *epos, const struct M33 *matx);
void rotpers_fisheye(struct EngineCoord *epos, const struct M33 *matx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
