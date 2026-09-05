#include "pre_inc.h"
#include "kfx/renderer/opengl/GLSpriteAtlas.h"
#include "bflib_basics.h"
#include "bflib_sprite.h"
#include "kfx/renderer/opengl/GLFunctions.h"
#include <unordered_set>
#include <cstring>
#include "post_inc.h"

bool GLSpriteAtlas::Init()
{
    m_pixels.assign((size_t)k_atlas_w * k_atlas_h, 0u);
    m_cursor_x = 1;
    m_shelf_y  = 0;
    m_shelf_h  = 0;
    m_dirty_y_min = k_atlas_h;
    m_dirty_y_max = -1;
    m_gl_init_needed = true;
    m_uvs.clear();
    return true;
}

void GLSpriteAtlas::Free()
{
    if (m_texture) {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    m_gl_init_needed = false;
    m_pixels.clear();
    m_uvs.clear();
}

void GLSpriteAtlas::PackSprite(SpriteHandle handle, const struct TbSprite* spr)
{
    // Text/cursor sprites (GLTextRenderer/GLCursorLayer, via
    // GLUIRenderer::DrawGlyphQuad) go through the exact same ResolveSprite()
    // -> PackSprite() -> GetUV() path as regular UI sprites (GLUIRenderer.cpp's
    // K_Sprite case in DrawFromIR) -- if either of these two early-returns
    // fires for a glyph/cursor handle specifically, GetUV() later fails and
    // DrawGlyphQuad() silently no-ops, with nothing in the log to explain
    // why. Logged (once per handle -- see below) instead of silent, matching
    // the "atlas full" ERRORLOG a few lines down.
    if (handle == kInvalidSpriteHandle) return;

    // Diagnostic-only: PackSprite() is called every frame for every
    // submitted handle (ResolveSprite() calls it unconditionally, see
    // GLUIRenderer::ResolveSprite()), so an early-return that fired every
    // call would flood the log for a persistent failure -- log each
    // distinct broken handle once, not once per frame.
    static std::unordered_set<SpriteHandle> s_logged_no_data;
    static std::unordered_set<SpriteHandle> s_logged_bad_size;

    if (!spr || !spr->Data)
    {
        if (s_logged_no_data.insert(handle).second)
            WARNLOG("GLSpriteAtlas: PackSprite(handle=%u) -- sprite has no Data, cannot pack", (unsigned)handle);
        return;
    }

    std::lock_guard<std::mutex> guard(m_mutex);

    if (m_uvs.find(handle) != m_uvs.end()) return; // already packed

    const int w = spr->SWidth;
    const int h = spr->SHeight;
    if (w <= 0 || h <= 0)
    {
        if (s_logged_bad_size.insert(handle).second)
            WARNLOG("GLSpriteAtlas: PackSprite(handle=%u) -- degenerate size %dx%d, cannot pack", (unsigned)handle, w, h);
        return;
    }

    int x, y;
    if (!alloc_shelf_rect(w, h, &x, &y, "sprite")) return;

    uint8_t* dst = m_pixels.data() + (size_t)y * k_atlas_w + x;
    LbSpriteDecode(dst, k_atlas_w, spr->Data, w, h);

    SpriteUV uv;
    uv.u0 = (float)x / (float)k_atlas_w;
    uv.v0 = (float)y / (float)k_atlas_h;
    uv.u1 = (float)(x + w) / (float)k_atlas_w;
    uv.v1 = (float)(y + h) / (float)k_atlas_h;
    uv.pixel_w = (uint16_t)w;
    uv.pixel_h = (uint16_t)h;
    m_uvs[handle] = uv;

    if (y < m_dirty_y_min) m_dirty_y_min = y;
    if (y + h > m_dirty_y_max) m_dirty_y_max = y + h;
}

void GLSpriteAtlas::PackRaw(SpriteHandle handle, const uint8_t* pixels, int w, int h)
{
    if (handle == kInvalidSpriteHandle || !pixels || w <= 0 || h <= 0) return;

    std::lock_guard<std::mutex> guard(m_mutex);

    if (m_uvs.find(handle) != m_uvs.end()) return; // already packed

    int x, y;
    if (!alloc_shelf_rect(w, h, &x, &y, "raw glyph")) return;

    for (int row = 0; row < h; ++row)
    {
        uint8_t* dst = m_pixels.data() + (size_t)(y + row) * k_atlas_w + x;
        std::memcpy(dst, pixels + (size_t)row * w, (size_t)w);
    }

    SpriteUV uv;
    uv.u0 = (float)x / (float)k_atlas_w;
    uv.v0 = (float)y / (float)k_atlas_h;
    uv.u1 = (float)(x + w) / (float)k_atlas_w;
    uv.v1 = (float)(y + h) / (float)k_atlas_h;
    uv.pixel_w = (uint16_t)w;
    uv.pixel_h = (uint16_t)h;
    m_uvs[handle] = uv;

    if (y < m_dirty_y_min) m_dirty_y_min = y;
    if (y + h > m_dirty_y_max) m_dirty_y_max = y + h;
}

// Caller already holds m_mutex.
bool GLSpriteAtlas::alloc_shelf_rect(int w, int h, int* out_x, int* out_y, const char* what)
{
    const int alloc_w = w + 1; // 1px margin, avoids bleed between shelf entries
    const int alloc_h = h + 1;

    if (m_cursor_x + alloc_w > k_atlas_w) {
        m_shelf_y += m_shelf_h;
        m_cursor_x = 0;
        m_shelf_h  = 0;
    }
    if (m_shelf_y + alloc_h > k_atlas_h) {
        ERRORLOG("GLSpriteAtlas: atlas full -- cannot pack %dx%d %s", w, h, what);
        return false;
    }
    if (alloc_h > m_shelf_h) m_shelf_h = alloc_h;

    *out_x = m_cursor_x;
    *out_y = m_shelf_y;
    m_cursor_x += alloc_w;
    return true;
}

bool GLSpriteAtlas::GetUV(SpriteHandle handle, SpriteUV& out) const
{
    std::lock_guard<std::mutex> guard(m_mutex);
    auto it = m_uvs.find(handle);
    if (it == m_uvs.end()) return false;
    out = it->second;
    return true;
}

// Called with m_mutex already held (from FlushPendingGL()).
void GLSpriteAtlas::flush_dirty()
{
    if (m_dirty_y_min > m_dirty_y_max) return;
    int h = m_dirty_y_max - m_dirty_y_min;
    if (h <= 0) return;

    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_dirty_y_min, k_atlas_w, h,
                    GL_RED, GL_UNSIGNED_BYTE,
                    m_pixels.data() + (size_t)m_dirty_y_min * k_atlas_w);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_dirty_y_min = k_atlas_h;
    m_dirty_y_max = -1;
}

void GLSpriteAtlas::FlushPendingGL()
{
    std::lock_guard<std::mutex> guard(m_mutex);

    if (m_gl_init_needed && !m_pixels.empty())
    {
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, k_atlas_w, k_atlas_h, 0,
                     GL_RED, GL_UNSIGNED_BYTE, m_pixels.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        m_gl_init_needed = false;
        m_dirty_y_min = k_atlas_h;
        m_dirty_y_max = -1;
    }
    else
    {
        flush_dirty();
    }
}
