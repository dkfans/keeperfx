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

    /** Return the platform texture handle for the given variation (0–31).
     *  Returns 0 if not initialised or out of range. */
    virtual unsigned int GetAtlasTexture(int variation) const = 0;

    /** Return a GL_TEXTURE_2D_ARRAY handle covering all variations as layers.
     *  Returns 0 when the platform uses individual textures instead. */
    virtual unsigned int GetAtlasTextureArray() const { return 0; }

    bool IsInitialized() const { return m_initialized; }

protected:
    bool m_initialized = false;
};

/******************************************************************************/
