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

class GLTileAtlas : public TileAtlasPacker, public ITileAtlas {
public:
    GLTileAtlas() = default;
    ~GLTileAtlas() override { Free(); }

    GLTileAtlas(const GLTileAtlas&)            = delete;
    GLTileAtlas& operator=(const GLTileAtlas&) = delete;

    // ITileAtlas
    bool         Init() override;
    void         Free() override;
    void         UpdateAnimatedTiles() override;
    unsigned int GetAtlasTexture(int variation) const override;
    unsigned int GetAtlasTextureArray() const override;

protected:
    // TileAtlasPacker build overrides — use R8 raw index copy instead of RGBA8 decode
    void BuildVariation(int variation) override;
    void BuildAnimatedStrip(int variation) override;

    // TileAtlasPacker upload hooks
    void UploadFull(int variation) override;
    void UploadAnimatedStrip(int variation, int y_offset, int h_pixels) override;

private:
    GLuint   m_texture_array = 0;     // GL_TEXTURE_2D_ARRAY: 2048×1024×32 R8
    uint8_t* m_r8_scratch = nullptr;  // R8 (1 byte/pixel) CPU scratch; replaces m_rgba_scratch
};
