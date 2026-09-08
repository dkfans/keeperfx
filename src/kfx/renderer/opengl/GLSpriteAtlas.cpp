#include "pre_inc.h"
#include "kfx/renderer/opengl/GLSpriteAtlas.h"
#include "kfx/renderer/opengl/GLResourceMapper.h"
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
    m_uvs.clear();

    if (m_resource_mapper != nullptr)
    {
        GpuTextureDesc desc;
        desc.width = k_atlas_w;
        desc.height = k_atlas_h;
        desc.format = GpuTextureFormat::R8;
        desc.min_filter = GpuTextureFilter::Nearest;
        desc.mag_filter = GpuTextureFilter::Nearest;
        desc.wrap = GpuTextureWrap::Clamp;
        desc.debug_name = "sprite_atlas";
        m_texture_handle = m_resource_mapper->RequestCreateTexture(desc);
    }

    return true;
}

void GLSpriteAtlas::Free() { }

void GLSpriteAtlas::PackSprite(SpriteHandle handle, const struct TbSprite* spr)
{
    if (handle == kInvalidSpriteHandle) return;

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
void GLSpriteAtlas::flush_dirty(unsigned int tex_id)
{
    if (m_dirty_y_min > m_dirty_y_max) return;
    int h = m_dirty_y_max - m_dirty_y_min;
    if (h <= 0) return;

    glBindTexture(GL_TEXTURE_2D, tex_id);
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

    if (m_resource_mapper == nullptr) return;
    const GLTexture* const tex = m_resource_mapper->ResolveTexture(m_texture_handle);
    if (tex == nullptr) return;

    flush_dirty(tex->id);
}
