/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file DrawState.h
 *     Immutable per-call draw-state descriptor.
 * @par Purpose:
 *     Instructions for a 2D draw — transparency, flip, outline,
 *     text alignment, and the active draw colour.  
 */
/******************************************************************************/
#pragma once

#include "bflib_video.h" // Lb_SPRITE_TRANSPAR4/8

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int TbDrawFlagsMask;

typedef struct KfxDrawState {
    TbDrawFlagsMask flags;
    /** Active draw colour (palette index)*/
    unsigned char colour;
} KfxDrawState;

/** A default, no-modifiers draw state (opaque, no flip, colour 0). */
static inline KfxDrawState draw_state_default(void)
{
    KfxDrawState s;
    s.flags  = 0;
    s.colour = 0;
    return s;
}

/** Build a draw state from explicit flags + colour. */
static inline KfxDrawState draw_state_make(TbDrawFlagsMask flags, unsigned char colour)
{
    KfxDrawState s;
    s.flags  = flags;
    s.colour = colour;
    return s;
}

/** How much of the drawn colour a translucent draw keeps, with the rest from
 *  what is underneath. Software blends through the ghost table
 *  (compute_fade_tables): ghost[a][b] is the palette colour nearest to
 *  (a + 2*b) / 3. TRANSPAR4 looks up ghost[drawn][screen] and TRANSPAR8
 *  ghost[screen][drawn], and TRANSPAR4 wins when both are set. */
static inline float draw_flags_source_weight(TbDrawFlagsMask flags)
{
    // TODO : These could be configurable
    if (flags & Lb_SPRITE_TRANSPAR4)
        return 1.0f / 3.0f;
    if (flags & Lb_SPRITE_TRANSPAR8)
        return 2.0f / 3.0f;
    return 1.0f;
}

#ifdef __cplusplus
}
#endif

/******************************************************************************/
