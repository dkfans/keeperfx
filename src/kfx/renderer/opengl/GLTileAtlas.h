/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLTileAtlas.h
 *     Desktop OpenGL tile texture atlas (GL_TEXTURE_2D_ARRAY).
 * @par Purpose:
 *     Inherits TileAtlasPacker (shared pixel-decode + UV logic) and
 *     ITileAtlas (platform-neutral interface).  Provides desktop OpenGL 3.3
 *     texture allocation and upload implementations.
 *
 *     All 32 variations are stored as layers of a single GL_TEXTURE_2D_ARRAY
 *     (2048×1024×32, GL_R8).  This allows the world renderer to draw all
 *     tile geometry in one draw call regardless of how many variations are
 *     visible, passing the variation index as a per-vertex attribute.
 *     Called by GLWorldViewRenderer and RendererOpenGL.
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/opengl/GLFunctions.h"
#include "kfx/renderer/TileAtlasPacker.h"
#include "kfx/renderer/ITileAtlas.h"
#include "kfx/renderer/GpuResourceHandle.h"
#include "engine_textures.h"
#include "kfx/renderer/GpuResourceDesc.h"

class GLResourceMapper;

class GLTileAtlas : public TileAtlasPacker, public ITileAtlas {
public:
    GLTileAtlas() { for (int32_t& l : m_set_layer) l = -1; }
    ~GLTileAtlas() override { Free(); }

    GLTileAtlas(const GLTileAtlas&)            = delete;
    GLTileAtlas& operator=(const GLTileAtlas&) = delete;

    /** Must be called before Init(). Not owned; must outlive this. */
    void SetResourceMapper(GLResourceMapper* mapper) { m_resource_mapper = mapper; }

    // ITileAtlas
    bool              Init() override;
    /** Allocates the array texture. Needs no level data; called during renderer init. */
    bool              CreateTexture();
    void              Free() override;
    /** Re-uploads every set with a layer whose tile data changed since it was last built. */
    void              RebuildChanged();
    /** Gives every set in set_mask a layer, growing the texture as needed, and
     *  builds it. Render thread, before tiles are drawn. */
    void              EnsureSets(uint32_t set_mask);
    /** Forgets which set has which layer (level change). Logs how many were used. */
    void              ResetSets();
    /** Layer of each set, -1 for none. TEXTURE_VARIATIONS_COUNT entries. */
    const int32_t*    SetLayerTable() const { return m_set_layer; }
    int32_t           LayerOf(int variation) const { return m_set_layer[variation]; }
    void              UpdateAnimatedTiles() override;
    GpuResourceHandle GetAtlasTextureArray() const override;

protected:
    // TileAtlasPacker build overrides — use R8 raw index copy instead of RGBA8 decode
    void BuildVariation(int variation) override;
    void BuildAnimatedStrip(int variation) override;

    // TileAtlasPacker upload hooks
    void UploadFull(int variation) override;
    void UploadAnimatedStrip(int variation, int y_offset, int h_pixels) override;

private:
    GLResourceMapper* m_resource_mapper = nullptr;
    GpuResourceHandle m_texture_handle = kInvalidGpuResource; // GL_TEXTURE_2D_ARRAY: 2048x1024 R8, 2 to 32 layers
    uint8_t* m_r8_scratch = nullptr;  // R8 (1 byte/pixel) CPU scratch; replaces m_rgba_scratch
    uint32_t m_built_generation[TEXTURE_VARIATIONS_COUNT] = {}; // texture_set_generation() last uploaded
    int32_t  m_set_layer[TEXTURE_VARIATIONS_COUNT];             // layer per set, -1 = none yet
    int32_t  m_layer_count = 0;
    int32_t  m_layer_capacity = 0;
    int32_t  m_layer_peak = 0;
    bool     m_grow_failed = false;

    GpuTextureDesc make_desc(int32_t layers) const;
};
