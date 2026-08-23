/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file SoftwareTextRenderer.h
 *     Software text renderer. The draw path is the ITextRenderer base; this
 *     exists so the software backend has a named type to instantiate.
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/ITextRenderer.h"

class SoftwareTextRenderer final : public ITextRenderer {
public:
    const char* GetName() const override { return "SOFTWARE_TEXT"; }
};

/******************************************************************************/
