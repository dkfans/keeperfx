/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file SoftwareUIRenderer.h
 *     Software UI renderer. The draw path is the IUIRenderer base; this exists
 *     so the software backend has a named type to instantiate.
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/IUIRenderer.h"

class SoftwareUIRenderer final : public IUIRenderer {
public:
    const char* GetName() const override { return "SOFTWARE_UI"; }
};

/******************************************************************************/
