/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLPaletteIndexLookup.h
 *     Turns a colour drawn from the frame palette back into its palette index,
 *     so passes over captured RGB images can use the game's index tables.
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/GpuResourceHandle.h"
#include <cstdint>
#include <vector>

class GLResourceMapper;

/** 512x512 R8 texture holding a palette index for every 6-bit RGB colour,
 *  at key = r<<12 | g<<6 | b, texel (key & 511, key >> 9).
 *  - A colour in the frame palette maps to its index; when several entries
 *    share a colour, the lowest index, as LbPaletteFindColour() returns.
 *  - Any other colour maps to the nearest engine palette colour, on the same
 *    16-step grid the game's own rgb-to-index table uses.
 *  Render thread only. */
class GLPaletteIndexLookup {
public:
    bool Init(GLResourceMapper* mapper);
    void Shutdown();

    /** Rebuild the nearest-colour fallback from @p engine_palette (768 bytes,
     *  6-bit). Call once the palette is loaded. */
    void RefreshBase(const unsigned char* engine_palette);

    /** Map the entries of @p rgba (256 RGBA8 entries, 6-bit values expanded
     *  with RendererPaletteChannel8()) to their indexes. */
    void SetPalette(const unsigned char* rgba);

    GpuResourceHandle GetTexture() const { return m_tex_handle; }

private:
    void apply_palette();
    void upload();

    GLResourceMapper* m_mapper = nullptr;
    GpuResourceHandle m_tex_handle = kInvalidGpuResource;
    std::vector<uint8_t> m_base;
    std::vector<uint8_t> m_table;
    unsigned char m_palette_rgba[256 * 4] = {};
    uint32_t m_palette_keys[256] = {};
    bool m_have_palette = false;
};

/******************************************************************************/
