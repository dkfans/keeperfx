#include "pre_inc.h"
#include "kfx/renderer/opengl/world/GLKeeperSpriteCache.h"
#include "kfx/renderer/opengl/world/KeeperSpriteDecode.h"
#include "kfx/renderer/opengl/core/GLCaps.h"
#include "kfx/renderer/opengl/core/GLFunctions.h"
#include "kfx/renderer/opengl/core/GLLog.h"
#include "kfx/renderer/opengl/core/GLResourceMapper.h"
#include "kfx/profiling/KfxProfiling.h"
#include <algorithm>
#include <cstring>
#include <vector>
#include "post_inc.h"

static GpuTextureDesc cache_desc(int32_t layers)
{
    GpuTextureDesc desc;
    desc.width = GLKeeperSpriteCache::k_layer_dim;
    desc.height = GLKeeperSpriteCache::k_layer_dim;
    desc.array_layers = layers;
    desc.format = GpuTextureFormat::RG8;
    desc.min_filter = GpuTextureFilter::Nearest;
    desc.mag_filter = GpuTextureFilter::Nearest;
    desc.wrap = GpuTextureWrap::Clamp;
    desc.debug_name = "kspr_sprite_array";
    return desc;
}

void GLKeeperSpriteCache::SetLimits(int32_t max_layers, int32_t budget_layers)
{
    m_max_layers = std::max<int32_t>(1, std::min(max_layers, budget_layers));
}

bool GLKeeperSpriteCache::Create(GLResourceMapper* mapper)
{
    m_mapper = mapper;
    const int32_t start = std::min(k_start_layers, std::max<int32_t>(1, m_max_layers / 2));
    m_handle = m_mapper->RequestCreateTexture(cache_desc(start));
    if (m_mapper->ResolveTexture(m_handle) == nullptr)
    {
        GLWARN("Could not create the creature sprite cache. Creatures still draw, but more slowly.");
        m_handle = kInvalidGpuResource;
        return false;
    }
    m_layers.SetCeiling(m_max_layers);
    m_layers.SetCapacity(start);
    GLLOG("Creature sprite cache ready: %d layers of %dx%d, up to %d.",
          (int)start, (int)k_layer_dim, (int)k_layer_dim, (int)m_max_layers);
    return true;
}

bool GLKeeperSpriteCache::Lookup(int32_t sprite_id, const unsigned char* data, int src_w, int src_h, Ref* out)
{
    if (!m_mapper || m_handle == kInvalidGpuResource || sprite_id < 0)
        return false;

    SpriteCacheLayers::Entry e;
    if (m_layers.Find(sprite_id, &e))
    {
        m_layers.MarkUsed(e.layer, m_frame);
        ++m_hits;
        *out = { e.layer, e.x, e.y, e.darkens };
        return true;
    }

    SpriteCacheLayers::PlaceResult r = m_layers.Place(sprite_id, src_w, src_h, m_frame, &e);
    if (r == SpriteCacheLayers::NeedsGrow)
    {
        const int32_t next = AtlasNextCapacity(m_layers.Capacity(), m_layers.Capacity() + 1, m_max_layers);
        if (!grow(next))
            m_layers.SetCeiling(m_layers.Capacity()); // can't grow: evict from now on
        r = m_layers.Place(sprite_id, src_w, src_h, m_frame, &e);
    }
    if (r != SpriteCacheLayers::Placed && r != SpriteCacheLayers::PlacedAfterEvict)
    {
        if (!m_full_logged)
        {
            GLWARN("Creature sprite cache has no free space this frame; some creatures draw the slow way. (%d layers, all in use)",
                   (int)m_layers.Capacity());
            m_full_logged = true;
        }
        return false;
    }

    const GLTexture* tex = m_mapper->ResolveTexture(m_handle);
    if (tex == nullptr)
        return false;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex->id);
    if (e.fresh_layer)
    {
        static std::vector<uint8_t> s_zero((size_t)k_layer_dim * k_layer_dim * 2, 0);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, e.layer, k_layer_dim, k_layer_dim, 1,
                        GL_RG, GL_UNSIGNED_BYTE, s_zero.data());
    }
    memset(kspr_decode_buf, 0, sizeof(kspr_decode_buf));
    decode_keeper_rle(kspr_decode_buf, data, src_w, src_h);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, k_kspr_decode_dim);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, e.x, e.y, e.layer, src_w, src_h, 1,
                    GL_RG, GL_UNSIGNED_BYTE, kspr_decode_buf);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    const bool darkens = decoded_sprite_darkens(src_w, src_h);
    m_layers.SetDarkens(sprite_id, darkens);
    ++m_misses;
    *out = { e.layer, e.x, e.y, darkens };
    return true;
}

bool GLKeeperSpriteCache::grow(int32_t layers)
{
    KFX_ZONE("GLKeeperSpriteCache::grow");
    if (layers <= m_layers.Capacity())
        return false;
    const int32_t old_layers = m_layers.Capacity();
    if (!m_mapper->ReplaceTexture(m_handle, cache_desc(layers),
            [old_layers](const GLTexture& from, const GLTexture& to) {
                return GLResourceMapper::CopyArrayLayers(from, to, old_layers);
            }))
        return false;
    m_layers.SetCapacity(layers);
    ++m_grows;
    return true;
}

// Logged when a level ends and when the game closes.
void GLKeeperSpriteCache::log_stats() const
{
    const uint64_t lookups = m_hits + m_misses;
    if (lookups > 0)
        GLLOG("Creature sprite cache: %d of %d layers used (peak %d), %u grows, %u evictions, %.1f%% hits.",
              (int)m_layers.UsedLayers(), (int)m_layers.Capacity(), (int)m_layers.PeakLayers(),
              (unsigned)m_grows, (unsigned)m_layers.Evictions(),
              100.0 * (double)m_hits / (double)lookups);
}

void GLKeeperSpriteCache::Clear()
{
    log_stats();
    m_layers.Clear();
    m_hits = 0;
    m_misses = 0;
    m_full_logged = false;
}
