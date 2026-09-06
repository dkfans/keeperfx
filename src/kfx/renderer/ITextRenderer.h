/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file ITextRenderer.h
 *     The text drawing a backend has to provide.
 */
/******************************************************************************/
#pragma once

#include "bflib_basics.h"
#include <cstdint>

/******************************************************************************/

class ITextRenderer {
public:
    virtual ~ITextRenderer() = default;

    /** Draw text laid out in the current text window, scaled by units_per_px. */
    virtual TbBool DrawTextResized(int32_t x, int32_t y, int32_t units_per_px, const char* text);

    virtual const char* GetName() const { return "TEXT"; }
};

/******************************************************************************/
