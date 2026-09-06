/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file IUIRenderer.h
 *     The UI drawing a backend has to provide.
 * @par Design:
 *     The engine's Lb* entry points route here, so this is where a backend
 *     decides how a draw is realised. The defaults are the software path, which
 *     calls the existing raster; a GPU backend overrides what it accelerates.
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/DrawState.h"
#include "bflib_basics.h"
#include <cstdint>

struct TbSprite;

/******************************************************************************/

class IUIRenderer {
public:
    virtual ~IUIRenderer() = default;

    virtual TbResult SubmitRawSprite(int32_t x, int32_t y, const struct TbSprite* spr,
                                     KfxDrawState state);
    virtual TbResult SubmitRawSpriteOneColour(int32_t x, int32_t y, const struct TbSprite* spr,
                                              unsigned char colour, KfxDrawState state);

    // Drawn at an explicit size rather than the sprite's own.
    virtual TbResult SubmitRawSpriteScaled(int32_t x, int32_t y, const struct TbSprite* spr,
                                           int32_t w, int32_t h, KfxDrawState state);
    virtual TbResult SubmitRawSpriteScaledOneColour(int32_t x, int32_t y, const struct TbSprite* spr,
                                                    int32_t w, int32_t h, unsigned char colour,
                                                    KfxDrawState state);
    virtual int      SubmitRawSpriteScaledRemap(int32_t x, int32_t y, const struct TbSprite* spr,
                                                int32_t w, int32_t h, const unsigned char* cmap,
                                                KfxDrawState state);

    virtual void SubmitSolidBox(int32_t x, int32_t y, int32_t w, int32_t h,
                                uint8_t colour_idx, KfxDrawState state);

    /** Tile the GUI slab texture over a rect. */
    virtual void SubmitSlabBackground(int32_t x, int32_t y, int32_t w, int32_t h);

    virtual const char* GetName() const { return "UI"; }
};

/******************************************************************************/
