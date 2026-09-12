/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file ITileAtlas.h
 *     Abstract interface for the GPU tile texture atlas.
 * @par Purpose:
 *     Platform-independent contract for initialising, managing, and querying
 *     the dungeon tile texture atlas.  Implementations (GLTileAtlas,
 *     VitaTileAtlas) inherit this alongside TileAtlasPacker, which supplies
 *     the shared pixel-decoding and UV logic.
 *
 *     This header intentionally does NOT include any GL headers so it can be
 *     included in any translation unit regardless of platform.
 */
/******************************************************************************/
#pragma once

// Pure typedef/handle header -- no GL types, so including it here doesn't
// violate this file's own "no GL headers" contract (see file header above).
#include "kfx/renderer/GpuResourceHandle.h"

/******************************************************************************/

class ITileAtlas {
public:
    virtual ~ITileAtlas() = default;

    /** Build all variation atlases from block_ptrs[] and lbPalette.
     *  Must be called after setup_texture_block_mem() and lbPalette is set.
     *  Returns false if texture allocation fails. */
    virtual bool Init() = 0;

    /** Release all GPU resources.  Safe to call before Init(). */
    virtual void Free() = 0;

    /** Re-upload only the animated tile rows for all variations.
     *  Call once per game tick after update_animating_texture_maps(). */
    virtual void UpdateAnimatedTiles() = 0;

    /** Return a GPU resource handle for a GL_TEXTURE_2D_ARRAY covering all
     *  variations as layers. kInvalidGpuResource when the platform uses
     *  individual textures instead. */
    virtual GpuResourceHandle GetAtlasTextureArray() const { return kInvalidGpuResource; }

    bool IsInitialized() const { return m_initialized; }

protected:
    bool m_initialized = false;
};

/******************************************************************************/
