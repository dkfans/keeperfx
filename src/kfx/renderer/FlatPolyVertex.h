/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file FlatPolyVertex.h
 *     Shared flat-colour polygon vertex type for world rendering.
 */
/******************************************************************************/
#pragma once

/** A single vertex for the flat-colour polygon (QK_PolyMode0 / BasicPolygon) pass.
 *  X/Y are raw screen-pixel floats (FLATPOLY_VERTEX_SHADER does its own NDC
 *  conversion via u_viewport, matching WorldVertex's append_triangle()
 *  convention of pre-converting to NDC being the exception, not the rule);
 *  Z is NDC depth [-1,1]; R/G/B are linear colour [0..1]. */
struct FlatPolyVertex
{
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
};

/******************************************************************************/
