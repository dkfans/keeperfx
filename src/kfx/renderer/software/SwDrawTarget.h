/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file SwDrawTarget.h
 *     The surface and clip window the software raster draws into.
 ******************************************************************************/
#pragma once

#include "bflib_video.h"   /* TbPixel */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Base of the target surface. */
TbPixel* SwTargetWScreen(void);
/** Surface base advanced to the clip window origin. */
TbPixel* SwTargetGraphicsWindowPtr(void);
/** Surface pitch, in bytes per row. */
int32_t SwTargetScanline(void);
/** Full surface height, in rows. */
int32_t SwTargetScreenHeight(void);

/** The clip window the primitives draw within. */
int32_t SwTargetWindowX(void);
int32_t SwTargetWindowY(void);
int32_t SwTargetWindowWidth(void);
int32_t SwTargetWindowHeight(void);

#ifdef __cplusplus
}
#endif
