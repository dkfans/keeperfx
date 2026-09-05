#ifndef RENDERER_OPENGL_GLSPRITEATLAS_H
#define RENDERER_OPENGL_GLSPRITEATLAS_H

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <mutex>
#include "kfx/renderer/SpriteHandle.h"

struct TbSprite;

struct SpriteUV {
    float u0, v0, u1, v1;
    uint16_t pixel_w, pixel_h;
};

// Packs decoded TbSprite pixel data (palette indices) into a single R8 GL
// texture using a shelf packer. Keyed by the SpriteHandle IUIRenderer already
// assigned -- this class owns no handle-allocation of its own.
//
// Locked (P5.6): PackSprite() runs on the game thread for UI sprites (at
// Submit* time) but on the render thread for text glyphs (GLTextRenderer::
// DrawGlyphs resolves them lazily during DrawFromIR replay) -- the two can
// run concurrently once threaded, and GetUV()/FlushPendingGL() (always
// render-thread) read the same staging buffer and map. develop uses a
// shared_mutex; a plain mutex is simpler and correctness-first here, matching
// the "correctness over performance" stance already taken for this backend.
class GLSpriteAtlas {
public:
    static constexpr int k_atlas_w = 4096;
    static constexpr int k_atlas_h = 2048;

    bool Init();
    void Free();

    /** Decode and pack one sprite at its already-assigned handle. No-op if
     *  already packed or the atlas is full. */
    void PackSprite(SpriteHandle handle, const struct TbSprite* spr);

    /** Pack an already-decoded 8-bit-per-pixel buffer (e.g. an expanded DBC
     *  glyph bitmap) at its already-assigned handle. Shares this atlas's
     *  shelf packer and single R8 texture with PackSprite() -- callers that
     *  only ever need a mask (any nonzero byte = opaque) rather than a real
     *  palette index can use pixel value 255, matching how
     *  UI_SPRITE_COLORED_FRAGMENT_SHADER already treats index 0 as
     *  transparent and anything else as fully opaque, flat-tinted. No-op if
     *  already packed or the atlas is full. */
    void PackRaw(SpriteHandle handle, const uint8_t* pixels, int w, int h);

    bool GetUV(SpriteHandle handle, SpriteUV& out) const;

    unsigned int GetTexture() const { return m_texture; }

    /** Create/upload the GL texture for whatever PackSprite() has queued.
     *  Must run on the thread owning the GL context. */
    void FlushPendingGL();

private:
    std::vector<uint8_t> m_pixels;
    unsigned int m_texture = 0;
    bool m_gl_init_needed = false;

    int m_cursor_x = 1; // reserve (0,0) as a safe fallback UV
    int m_shelf_y  = 0;
    int m_shelf_h  = 0;

    int m_dirty_y_min = k_atlas_h;
    int m_dirty_y_max = -1;

    std::unordered_map<SpriteHandle, SpriteUV> m_uvs;

    mutable std::mutex m_mutex;

    /** Shelf-allocate a w x h (+1px margin) rect, advancing the packer
     *  cursor. Returns false (logging via `what`) if the atlas is full.
     *  Caller already holds m_mutex. */
    bool alloc_shelf_rect(int w, int h, int* out_x, int* out_y, const char* what);

    void flush_dirty();
};

#endif // RENDERER_OPENGL_GLSPRITEATLAS_H
