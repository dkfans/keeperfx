/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file TileAtlasPacker.h
 *     Platform-independent dungeon tile atlas packer (shared base class).
 * @par Purpose:
 *     Owns the CPU-side scratch buffer, atlas layout constants, palette-decode
 *     logic, and UV computation.  Platform subclasses (GLTileAtlas,
 *     VitaTileAtlas) inherit this and provide the two pure-virtual upload
 *     hooks that perform the actual GPU texture upload.
 *
 *     This header has zero GL/vitaGL dependency and compiles on all targets.
 *
 * @par Usage
 *     Subclass together with ITileAtlas:
 * @code
 *     class GLTileAtlas : public TileAtlasPacker, public ITileAtlas { ... };
 * @endcode
 *     In Init():  call InitPacker(), allocate GPU textures, then BuildVariation().
 *     In Free():  call FreePacker() after releasing GPU textures.
 */
/******************************************************************************/
#pragma once

#include <stdint.h>

/******************************************************************************/

class TileAtlasPacker {
public:
    virtual ~TileAtlasPacker();

    // ── Atlas layout (public so callers can compute UVs independently) ───────

    static const int k_tile_dim   = 32;           // block_dimension (always 32)
    static const int k_atlas_w    = 2048;
    static const int k_atlas_h    = 1024;
    static const int k_atlas_cols = k_atlas_w / k_tile_dim;  // 64
    static const int k_max_variations = 32;

    // ── UV helper ─────────────────────────────────────────────────────────────

    /** Compute the normalised UV rectangle [u0,v0]–[u1,v1] for tile_id. */
    static void GetTileUV(int tile_id,
                          float* u0, float* v0,
                          float* u1, float* v1);

protected:
    // ── Lifecycle helpers called by platform Init / Free ─────────────────────

    /** Allocate the CPU scratch buffer (k_atlas_w × k_atlas_h × 4 bytes).
     *  Returns false on allocation failure. */
    bool InitPacker();

    /** Free the CPU scratch buffer. */
    void FreePacker();

    // ── Pixel build helpers ──────────────────────────────────────────────────

    /** Build atlas pixel data for one variation and call UploadFull(variation).
     *  Default implementation decodes palette indices to RGBA8 via lbPalette.
     *  Platform subclasses may override to use a different pixel format. */
    virtual void BuildVariation(int variation);

    /** Build animated tile strip pixel data for one variation and call
     *  UploadAnimatedStrip(variation, y_offset, h_pixels).
     *  Default implementation decodes palette indices to RGBA8 via lbPalette.
     *  Platform subclasses may override to use a different pixel format. */
    virtual void BuildAnimatedStrip(int variation);

    // ── GPU upload hooks — platform must implement ───────────────────────────

    /** Upload m_rgba_scratch (full atlas, k_atlas_w × k_atlas_h) for
     *  the given variation index. */
    virtual void UploadFull(int variation) = 0;

    /** Upload a horizontal strip of m_rgba_scratch for the given variation.
     *  @param variation   0 .. k_max_variations-1
     *  @param y_offset    Top pixel row of the strip in the full atlas.
     *  @param h_pixels    Height of the strip in pixels. */
    virtual void UploadAnimatedStrip(int variation, int y_offset, int h_pixels) = 0;

    // ── Scratch buffer (shared with subclasses for the upload calls) ─────────

    uint8_t* m_rgba_scratch = nullptr;

private:
    static inline int tile_row(int id) { return id / k_atlas_cols; }
    static inline int tile_col(int id) { return id % k_atlas_cols; }

    /** Palette-expand one 32×32 8-bit indexed tile into m_rgba_scratch at
     *  the grid position corresponding to tile_id. */
    void DecodeTile(const uint8_t* src_indexed, int tile_id);
};

/******************************************************************************/
