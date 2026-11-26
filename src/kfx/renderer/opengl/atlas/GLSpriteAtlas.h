#ifndef RENDERER_OPENGL_GLSPRITEATLAS_H
#define RENDERER_OPENGL_GLSPRITEATLAS_H

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <functional>
#include <mutex>
#include "kfx/renderer/SpriteHandle.h"
#include "kfx/renderer/GpuResourceHandle.h"

struct TbSprite;
class GLResourceMapper;

struct SpriteUV {
    float u0, v0, u1, v1;
    uint16_t pixel_w, pixel_h;
};

// Packs decoded TbSprite pixel data (palette indices) into a single R8 GL
// texture using a shelf packer. Forced 8bit, in the future, should be open to any format.
class GLSpriteAtlas {
public:
    static constexpr int32_t k_start_dim = 2048;

    /** Must be called before Init(). Not owned; must outlive this. */
    void SetResourceMapper(GLResourceMapper* mapper) { m_resource_mapper = mapper; }

    /** Tells the atlas which handles are still in use. When it fills up it
     *  keeps those and drops the rest. Without one, every packed handle is
     *  treated as live. Called with a lock held by the atlas, so the query
     *  must not call back into it. */
    void SetLivenessQuery(std::function<bool(SpriteHandle)> is_live);

    /** Largest width or height the atlas may grow to. Before Init(). */
    void SetMaxSize(int32_t max_dim) { m_max_dim = max_dim; }

    bool Init();
    void Free();

    /** Decode and pack one sprite at its already-assigned handle. No-op if
     *  already packed or the atlas is full. */
    void PackSprite(SpriteHandle handle, const struct TbSprite* spr);

    /** Pack an already-decoded 8-bit-per-pixel buffer (e.g. an expanded DBC
     *  glyph bitmap) at its already-assigned handle.
     *  ideally, format is agnostic but 8bit it is for now.*/
    void PackRaw(SpriteHandle handle, const uint8_t* pixels, int w, int h);

    bool GetUV(SpriteHandle handle, SpriteUV& out) const;
    bool Contains(SpriteHandle handle) const;

    GpuResourceHandle GetTexture() const { return m_texture_handle; }

    void FlushPendingGL();

private:
    std::vector<uint8_t> m_pixels;
    GLResourceMapper* m_resource_mapper = nullptr;
    GpuResourceHandle m_texture_handle = kInvalidGpuResource;

    int m_cursor_x = 1; // reserve (0,0) as a safe fallback UV
    int m_shelf_y  = 0;
    int m_shelf_h  = 0;

    int32_t m_w = k_start_dim;       // CPU layout
    int32_t m_h = k_start_dim;
    int32_t m_tex_w = k_start_dim;   // texture the render thread last committed
    int32_t m_tex_h = k_start_dim;
    int32_t m_max_dim = k_start_dim;
    bool    m_grow_failed = false;

    int m_dirty_y_min = k_start_dim;
    int m_dirty_y_max = -1;

    struct AtlasEntry {
        int x = 0, y = 0; // top-left texel of the sprite's pixels
        int w = 0, h = 0;
    };
    std::unordered_map<SpriteHandle, AtlasEntry> m_entries;
    std::function<bool(SpriteHandle)> m_is_live;

    mutable std::mutex m_mutex;

    /** Shelf-allocate a w x h (+1px margin) rect, advancing the packer
     *  cursor. When the atlas is full it compacts first if a quarter or more
     *  of it is unused sprites, then grows, then compacts as a last resort.
     *  Compacting repacks live entries and drops dead ones, so a live handle
     *  never loses its pixels. Returns false (logging via `what`) only if
     *  the rect still doesn't fit.
     *  Caller already holds m_mutex. */
    bool alloc_shelf_rect(int w, int h, int* out_x, int* out_y, const char* what);

    /** True when unused sprites cover a quarter or more of the layout.
     *  Caller already holds m_mutex. */
    bool unused_share_is_large() const;
    bool try_alloc_shelf_rect(int alloc_w, int alloc_h, int* out_x, int* out_y);
    static SpriteUV make_sprite_uv(int x, int y, int w, int h, int tex_w, int tex_h);

    /** Doubles the CPU layout (width first, then height) up to m_max_dim.
     *  Caller already holds m_mutex. */
    bool grow_layout();

    /** Moves the CPU pixels into a new_w x new_h layout, dropping entries
     *  that no longer fit. False (nothing changed) if the memory isn't there.
     *  Caller already holds m_mutex. */
    bool relayout(int32_t new_w, int32_t new_h);

    /** Repacks the live entries into a zeroed target_w x target_h buffer,
     *  dropping the rest. Reads from the current layout, which may be larger.
     *  Caller already holds m_mutex. */
    void compact(int32_t target_w, int32_t target_h);

    /** Uploads the dirty region into the given (already-resolved) texture
     *  id. Caller already holds m_mutex. */
    void flush_dirty(unsigned int tex_id);
};

#endif // RENDERER_OPENGL_GLSPRITEATLAS_H
