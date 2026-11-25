#include "pre_inc.h"
#include "kfx/renderer/opengl/atlas/GLSpriteAtlas.h"
#include "kfx/renderer/opengl/core/GLResourceMapper.h"
#include "bflib_basics.h"
#include "bflib_sprite.h"
#include "kfx/renderer/opengl/core/GLFunctions.h"
#include "kfx/renderer/opengl/core/GLLog.h"
#include <algorithm>
#include <unordered_set>
#include <cstring>
#include <iterator>
#include <new>
#include "post_inc.h"

void GLSpriteAtlas::SetLivenessQuery(std::function<bool(SpriteHandle)> is_live)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    m_is_live = std::move(is_live);
}

bool GLSpriteAtlas::Init()
{
    if (m_max_dim < k_start_dim)
        m_max_dim = k_start_dim;
    m_w = m_h = m_tex_w = m_tex_h = k_start_dim;
    m_grow_failed = false;
    m_pixels.assign((size_t)m_w * m_h, 0u);
    m_cursor_x = 1;
    m_shelf_y  = 0;
    m_shelf_h  = 0;
    m_dirty_y_min = m_h;
    m_dirty_y_max = -1;
    m_entries.clear();

    if (m_resource_mapper != nullptr)
    {
        GpuTextureDesc desc;
        desc.width = m_w;
        desc.height = m_h;
        desc.format = GpuTextureFormat::R8;
        desc.min_filter = GpuTextureFilter::Nearest;
        desc.mag_filter = GpuTextureFilter::Nearest;
        desc.wrap = GpuTextureWrap::Clamp;
        desc.debug_name = "sprite_atlas";
        m_texture_handle = m_resource_mapper->RequestCreateTexture(desc);
        return m_resource_mapper->ResolveTexture(m_texture_handle) != nullptr;
    }

    return true;
}

void GLSpriteAtlas::Free() { }

void GLSpriteAtlas::PackSprite(SpriteHandle handle, const struct TbSprite* spr)
{
    if (handle == kInvalidSpriteHandle) return;

    static std::unordered_set<SpriteHandle> s_logged_no_data;
    static std::unordered_set<SpriteHandle> s_logged_bad_size;

    if (!spr)
        return;
    if (spr->SWidth == 0 || spr->SHeight == 0)
        return; // an empty slot in the sprite sheet: nothing to draw
    if (!spr->Data)
    {
        if (s_logged_no_data.insert(handle).second)
            GLWARN("Interface sprite %u has no pixel data, so it isn't drawn.", (unsigned)handle);
        return;
    }

    std::lock_guard<std::mutex> guard(m_mutex);

    if (m_entries.find(handle) != m_entries.end()) return; // already packed

    const int w = spr->SWidth;
    const int h = spr->SHeight;
    if (w <= 0 || h <= 0)
    {
        if (s_logged_bad_size.insert(handle).second)
            GLWARN("Interface sprite %u has an invalid size, so it isn't drawn. (%dx%d)", (unsigned)handle, w, h);
        return;
    }

    int x, y;
    if (!alloc_shelf_rect(w, h, &x, &y, "sprite")) return;

    uint8_t* dst = m_pixels.data() + (size_t)y * m_w + x;
    LbSpriteDecode(dst, m_w, spr->Data, w, h);

    AtlasEntry entry;
    entry.x = x;
    entry.y = y;
    entry.w = w;
    entry.h = h;
    m_entries[handle] = entry;

    if (y < m_dirty_y_min) m_dirty_y_min = y;
    if (y + h > m_dirty_y_max) m_dirty_y_max = y + h;
}

void GLSpriteAtlas::PackRaw(SpriteHandle handle, const uint8_t* pixels, int w, int h)
{
    if (handle == kInvalidSpriteHandle || !pixels || w <= 0 || h <= 0) return;

    std::lock_guard<std::mutex> guard(m_mutex);

    if (m_entries.find(handle) != m_entries.end()) return; // already packed

    int x, y;
    if (!alloc_shelf_rect(w, h, &x, &y, "raw glyph")) return;

    for (int row = 0; row < h; ++row)
    {
        uint8_t* dst = m_pixels.data() + (size_t)(y + row) * m_w + x;
        std::memcpy(dst, pixels + (size_t)row * w, (size_t)w);
    }

    AtlasEntry entry;
    entry.x = x;
    entry.y = y;
    entry.w = w;
    entry.h = h;
    m_entries[handle] = entry;

    if (y < m_dirty_y_min) m_dirty_y_min = y;
    if (y + h > m_dirty_y_max) m_dirty_y_max = y + h;
}

SpriteUV GLSpriteAtlas::make_sprite_uv(int x, int y, int w, int h, int tex_w, int tex_h)
{
    constexpr float k_inset = 1.0f / 32.0f;
    SpriteUV uv;
    uv.u0 = ((float)x + k_inset) / (float)tex_w;
    uv.v0 = ((float)y + k_inset) / (float)tex_h;
    uv.u1 = ((float)(x + w) - k_inset) / (float)tex_w;
    uv.v1 = ((float)(y + h) - k_inset) / (float)tex_h;
    uv.pixel_w = (uint16_t)w;
    uv.pixel_h = (uint16_t)h;
    return uv;
}

// Caller already holds m_mutex.
bool GLSpriteAtlas::try_alloc_shelf_rect(int alloc_w, int alloc_h, int* out_x, int* out_y)
{
    int cursor_x = m_cursor_x;
    int shelf_y  = m_shelf_y;
    int shelf_h  = m_shelf_h;
    if (cursor_x + alloc_w > m_w) {
        shelf_y += shelf_h;
        cursor_x = 0;
        shelf_h  = 0;
    }
    if (alloc_w > m_w || shelf_y + alloc_h > m_h)
        return false;
    if (alloc_h > shelf_h) shelf_h = alloc_h;

    *out_x = cursor_x;
    *out_y = shelf_y;
    m_cursor_x = cursor_x + alloc_w;
    m_shelf_y  = shelf_y;
    m_shelf_h  = shelf_h;
    return true;
}

// Caller already holds m_mutex.
bool GLSpriteAtlas::alloc_shelf_rect(int w, int h, int* out_x, int* out_y, const char* what)
{
    const int alloc_w = w + 1; // 1px margin, avoids bleed between shelf entries
    const int alloc_h = h + 1;

    if (try_alloc_shelf_rect(alloc_w, alloc_h, out_x, out_y))
        return true;

    while (grow_layout())
    {
        if (try_alloc_shelf_rect(alloc_w, alloc_h, out_x, out_y))
            return true;
    }

    compact(m_w, m_h);

    if (try_alloc_shelf_rect(alloc_w, alloc_h, out_x, out_y))
        return true;
    GLERR("Interface sprite atlas is full of sprites in use; a new %s can't be added and won't draw. (%dx%d)", what, w, h);
    return false;
}

// Caller already holds m_mutex.
void GLSpriteAtlas::compact(int32_t target_w, int32_t target_h)
{
    const int32_t src_pitch = m_w;
    struct Live { SpriteHandle handle; AtlasEntry entry; };
    std::vector<Live> live;
    live.reserve(m_entries.size());
    for (const auto& kv : m_entries)
    {
        if (!m_is_live || m_is_live(kv.first))
            live.push_back({ kv.first, kv.second });
    }
    const size_t dropped = m_entries.size() - live.size();

    // Tallest first packs shelves tightly; the handle breaks ties so the layout is repeatable.
    std::sort(live.begin(), live.end(), [](const Live& a, const Live& b) {
        if (a.entry.h != b.entry.h) return a.entry.h > b.entry.h;
        if (a.entry.w != b.entry.w) return a.entry.w > b.entry.w;
        return a.handle < b.handle;
    });

    std::vector<uint8_t> old_pixels;
    old_pixels.swap(m_pixels);
    m_w = target_w;
    m_h = target_h;
    m_pixels.assign((size_t)m_w * m_h, 0u);
    m_entries.clear();
    m_cursor_x = 1; // reserve (0,0) as a safe fallback UV
    m_shelf_y  = 0;
    m_shelf_h  = 0;

    size_t lost = 0;
    for (const Live& item : live)
    {
        const int w = item.entry.w;
        const int h = item.entry.h;
        int x, y;
        if (!try_alloc_shelf_rect(w + 1, h + 1, &x, &y))
        {
            ++lost; // its next draw fails to find it and logs
            continue;
        }
        for (int row = 0; row < h; ++row)
        {
            std::memcpy(m_pixels.data() + (size_t)(y + row) * m_w + x,
                        old_pixels.data() + (size_t)(item.entry.y + row) * src_pitch + item.entry.x,
                        (size_t)w);
        }
        AtlasEntry entry;
        entry.x = x;
        entry.y = y;
        entry.w = w;
        entry.h = h;
        m_entries[item.handle] = entry;
    }

    m_dirty_y_min = 0;
    m_dirty_y_max = m_h;
    GLWARN("Interface sprite atlas is full; made room by dropping %d unused sprites. (kept %d)",
           (int)dropped, (int)m_entries.size());
    if (lost > 0)
        GLERR("Interface sprite atlas: %d sprites in use no longer fit and won't draw until space frees up.", (int)lost);
}

bool GLSpriteAtlas::Contains(SpriteHandle handle) const
{
    std::lock_guard<std::mutex> guard(m_mutex);
    return m_entries.find(handle) != m_entries.end();
}

bool GLSpriteAtlas::GetUV(SpriteHandle handle, SpriteUV& out) const
{
    std::lock_guard<std::mutex> guard(m_mutex);
    auto it = m_entries.find(handle);
    if (it == m_entries.end()) return false;
    const AtlasEntry& e = it->second;
    if (e.x + e.w > m_tex_w || e.y + e.h > m_tex_h)
        return false;   // packed into space the texture doesn't have yet; drawn next frame
    out = make_sprite_uv(e.x, e.y, e.w, e.h, m_tex_w, m_tex_h);
    return true;
}

// Called with m_mutex already held (from FlushPendingGL()).
void GLSpriteAtlas::flush_dirty(unsigned int tex_id)
{
    if (m_dirty_y_min > m_dirty_y_max) return;
    int h = m_dirty_y_max - m_dirty_y_min;
    if (h <= 0) return;

    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_dirty_y_min, m_w, h,
                    GL_RED, GL_UNSIGNED_BYTE,
                    m_pixels.data() + (size_t)m_dirty_y_min * m_w);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_dirty_y_min = m_h;
    m_dirty_y_max = -1;
}

void GLSpriteAtlas::FlushPendingGL()
{
    std::lock_guard<std::mutex> guard(m_mutex);

    if (m_resource_mapper == nullptr) return;
    const GLTexture* const tex = m_resource_mapper->ResolveTexture(m_texture_handle);
    if (tex == nullptr) return;

    if (m_w != m_tex_w || m_h != m_tex_h)
    {
        GpuTextureDesc desc;
        desc.width = m_w;
        desc.height = m_h;
        desc.format = GpuTextureFormat::R8;
        desc.min_filter = GpuTextureFilter::Nearest;
        desc.mag_filter = GpuTextureFilter::Nearest;
        desc.wrap = GpuTextureWrap::Clamp;
        desc.debug_name = "sprite_atlas";
        if (m_resource_mapper->ReplaceTexture(m_texture_handle, desc, nullptr))
        {
            m_tex_w = m_w;
            m_tex_h = m_h;
        }
        else
        {
            GLWARN("Interface sprite atlas could not grow; it makes room by dropping unused sprites instead.");
            m_grow_failed = true;
            compact(m_tex_w, m_tex_h); // repacks in-use sprites from the bigger layout into the texture's size
        }
        m_dirty_y_min = 0;
        m_dirty_y_max = m_h;
    }

    flush_dirty(tex->id);
}

// Caller already holds m_mutex.
bool GLSpriteAtlas::grow_layout()
{
    if (m_grow_failed)
        return false;
    int32_t new_w = m_w;
    int32_t new_h = m_h;
    if (m_w <= m_h && m_w * 2 <= m_max_dim)
        new_w = m_w * 2;
    else if (m_h * 2 <= m_max_dim)
        new_h = m_h * 2;
    else if (m_w * 2 <= m_max_dim)
        new_w = m_w * 2;
    else
        return false;
    if (!relayout(new_w, new_h))
    {
        GLWARN("Not enough memory to grow the interface sprite atlas to %dx%d; it makes room by dropping unused sprites instead.",
               (int)new_w, (int)new_h);
        m_grow_failed = true;
        return false;
    }
    GLLOG("Interface sprite atlas grew to %dx%d.", (int)new_w, (int)new_h);
    return true;
}

// Caller already holds m_mutex.
bool GLSpriteAtlas::relayout(int32_t new_w, int32_t new_h)
{
    std::vector<uint8_t> pixels;
    try
    {
        pixels.assign((size_t)new_w * new_h, 0u);
    }
    catch (const std::bad_alloc&)
    {
        return false;
    }
    const int32_t rows = std::min(m_h, new_h);
    const int32_t cols = std::min(m_w, new_w);
    for (int32_t row = 0; row < rows; ++row)
        std::memcpy(pixels.data() + (size_t)row * new_w, m_pixels.data() + (size_t)row * m_w, (size_t)cols);
    for (auto it = m_entries.begin(); it != m_entries.end(); )
    {
        const AtlasEntry& e = it->second;
        it = (e.x + e.w > new_w || e.y + e.h > new_h) ? m_entries.erase(it) : std::next(it);
    }
    m_pixels.swap(pixels);
    m_w = new_w;
    m_h = new_h;
    m_dirty_y_min = 0;
    m_dirty_y_max = m_h;
    return true;
}
