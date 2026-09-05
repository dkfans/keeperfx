/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file EngineVertex.h
 *  Neutral fixed-point vertex type shared between the engine bucket system
 *  and the renderer IR command layer.  Replaces struct PolyPoint in IR
 *  commands so that IR headers do not depend on the SW rasteriser header.
 */
/******************************************************************************/
#pragma once
#include <cstdint>

/** Fixed-point screen-space vertex produced by the engine polygon pipeline.
 *  Coordinates are in screen pixels; U/V are fixed-point texture coordinates.
 *  The 'S' (shininess) field from PolyPoint is intentionally omitted — shadow
 *  IR commands carry darkness as a separate scalar, not per-vertex. */
struct EnginePolyVertex {
    int32_t x;  /**< Screen X coordinate */
    int32_t y;  /**< Screen Y coordinate */
    int32_t u;  /**< Texture U (fixed-point) */
    int32_t v;  /**< Texture V (fixed-point) */
};

/** Copy a PolyPoint into an EnginePolyVertex with explicit field-by-field
 *  assignment.  Safe on all platforms regardless of sizeof(long). */
#ifdef __cplusplus
#include "bflib_render.h"  /* struct PolyPoint for EnginePolyVertexFrom() */
inline EnginePolyVertex EnginePolyVertexFrom(const struct PolyPoint& p)
{
    EnginePolyVertex v;
    v.x = (int32_t)p.X;
    v.y = (int32_t)p.Y;
    v.u = (int32_t)p.U;
    v.v = (int32_t)p.V;
    return v;
}
#endif
