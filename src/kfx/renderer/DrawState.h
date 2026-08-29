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

#ifdef __cplusplus
}
#endif

/******************************************************************************/
