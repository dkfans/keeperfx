/******************************************************************************/
/** @file GLKeeperSpriteCache.h
 *     Creature sprite cache: a GL_TEXTURE_2D_ARRAY of RG8 layers that frames
 *     are shelf-packed into on first use. It grows by doubling and, at its
 *     ceiling, evicts whole layers (see SpriteCacheLayers). Render thread only.
 */
/******************************************************************************/
#ifndef KFX_GLKEEPERSPRITECACHE_H
#define KFX_GLKEEPERSPRITECACHE_H

#include "kfx/renderer/GpuResourceHandle.h"
#include "kfx/renderer/opengl/atlas/SpriteCacheLayers.h"
#include <cstdint>
#include <string>

class GLResourceMapper;

class GLKeeperSpriteCache {
public:
    static constexpr int32_t k_layer_dim = 1024;     // RG8 layer, 2 MiB
    static constexpr int32_t k_start_layers = 16;    // 32 MiB
    static constexpr int32_t k_budget_layers = 128;  // 256 MiB, the old fixed size

    /** Where a cached frame lives: layer and its top-left texel in it. */
    struct Ref { int32_t layer; int32_t x, y; bool darkens; };

    /** Largest the cache may grow: min(driver layers, budget). Before Create(). */
    void SetLimits(int32_t max_layers, int32_t budget_layers);

    /** Creates the texture at its start size. False if it can't be made. */
    bool Create(GLResourceMapper* mapper);
    GpuResourceHandle Texture() const { return m_handle; }

    /** Finds a frame, or decodes and packs it, growing or evicting as needed.
     *  False means use the per-sprite path. */
    bool Lookup(int32_t sprite_id, const unsigned char* data, int src_w, int src_h, Ref* out);

    /** Starts a render frame; layers used in it are never evicted. */
    void BeginFrame() { ++m_frame; }

    /** Empties the cache; the texture keeps its size. */
    void Clear();

    /** One sentence with this level's numbers, for the video memory line. */
    std::string Summary() const;

private:
    GLResourceMapper* m_mapper = nullptr;
    GpuResourceHandle m_handle = kInvalidGpuResource;
    SpriteCacheLayers m_layers{ k_layer_dim, 1 };
    int32_t  m_max_layers = k_budget_layers;
    uint32_t m_grows = 0;
    uint64_t m_hits = 0;
    uint64_t m_misses = 0;
    uint64_t m_frame = 0;
    bool     m_full_logged = false;

    bool grow(int32_t layers);
};

#endif
