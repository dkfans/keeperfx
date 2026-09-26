#include "pre_inc.h"
#include "kfx/renderer/opengl/GLSpriteAtlas.h"
#include "kfx/renderer/opengl/GLResourceMapper.h"
#include "bflib_basics.h"
#include "bflib_sprite.h"
#include "kfx/renderer/opengl/GLFunctions.h"
#include <algorithm>
#include <unordered_set>
#include <cstring>
#include "post_inc.h"

void GLSpriteAtlas::SetLivenessQuery(std::function<bool(SpriteHandle)> is_live)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    m_is_live = std::move(is_live);
}

bool GLSpriteAtlas::Init()
{
    m_pixels.assign((size_t)k_atlas_w * k_atlas_h, 0u);
    m_cursor_x = 1;
    m_shelf_y  = 0;
    m_shelf_h  = 0;
    m_dirty_y_min = k_atlas_h;
    m_dirty_y_max = -1;
    m_entries.clear();

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

    if (m_entries.find(handle) != m_entries.end()) return; // already packed

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

    AtlasEntry entry;
    entry.uv = make_sprite_uv(x, y, w, h);
    entry.x = x;
    entry.y = y;
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
        uint8_t* dst = m_pixels.data() + (size_t)(y + row) * k_atlas_w + x;
        std::memcpy(dst, pixels + (size_t)row * w, (size_t)w);
    }

    AtlasEntry entry;
    entry.uv = make_sprite_uv(x, y, w, h);
    entry.x = x;
    entry.y = y;
    m_entries[handle] = entry;

    if (y < m_dirty_y_min) m_dirty_y_min = y;
    if (y + h > m_dirty_y_max) m_dirty_y_max = y + h;
}

SpriteUV GLSpriteAtlas::make_sprite_uv(int x, int y, int w, int h)
{
    constexpr float k_inset = 1.0f / 32.0f;
    SpriteUV uv;
    uv.u0 = ((float)x + k_inset) / (float)k_atlas_w;
    uv.v0 = ((float)y + k_inset) / (float)k_atlas_h;
    uv.u1 = ((float)(x + w) - k_inset) / (float)k_atlas_w;
    uv.v1 = ((float)(y + h) - k_inset) / (float)k_atlas_h;
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
    if (cursor_x + alloc_w > k_atlas_w) {
        shelf_y += shelf_h;
        cursor_x = 0;
        shelf_h  = 0;
    }
    if (alloc_w > k_atlas_w || shelf_y + alloc_h > k_atlas_h)
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

    compact();

    if (try_alloc_shelf_rect(alloc_w, alloc_h, out_x, out_y))
        return true;
    ERRORLOG("GLSpriteAtlas: cannot pack %dx%d %s, the atlas is full of live sprites", w, h, what);
    return false;
}

// Caller already holds m_mutex.
void GLSpriteAtlas::compact()
{
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
        if (a.entry.uv.pixel_h != b.entry.uv.pixel_h) return a.entry.uv.pixel_h > b.entry.uv.pixel_h;
        if (a.entry.uv.pixel_w != b.entry.uv.pixel_w) return a.entry.uv.pixel_w > b.entry.uv.pixel_w;
        return a.handle < b.handle;
    });

    std::vector<uint8_t> old_pixels;
    old_pixels.swap(m_pixels);
    m_pixels.assign((size_t)k_atlas_w * k_atlas_h, 0u);
    m_entries.clear();
    m_cursor_x = 1; // reserve (0,0) as a safe fallback UV
    m_shelf_y  = 0;
    m_shelf_h  = 0;

    size_t lost = 0;
    for (const Live& item : live)
    {
        const int w = item.entry.uv.pixel_w;
        const int h = item.entry.uv.pixel_h;
        int x, y;
        if (!try_alloc_shelf_rect(w + 1, h + 1, &x, &y))
        {
            ++lost; // its next draw fails to find it and logs
            continue;
        }
        for (int row = 0; row < h; ++row)
        {
            std::memcpy(m_pixels.data() + (size_t)(y + row) * k_atlas_w + x,
                        old_pixels.data() + (size_t)(item.entry.y + row) * k_atlas_w + item.entry.x,
                        (size_t)w);
        }
        AtlasEntry entry;
        entry.uv = make_sprite_uv(x, y, w, h);
        entry.x = x;
        entry.y = y;
        m_entries[item.handle] = entry;
    }

    m_dirty_y_min = 0;
    m_dirty_y_max = k_atlas_h;
    WARNLOG("GLSpriteAtlas: atlas full -- compacted, kept %d sprites, dropped %d unused",
            (int)m_entries.size(), (int)dropped);
    if (lost > 0)
        ERRORLOG("GLSpriteAtlas: %d live sprites no longer fit after compacting", (int)lost);
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
    out = it->second.uv;
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
