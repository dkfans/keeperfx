/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLTileAtlas.cpp
 *     Desktop OpenGL tile texture atlas — GL upload implementation.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/opengl/GLTileAtlas.h"
#include "kfx/renderer/opengl/GLResourceMapper.h"

#include "engine_textures.h"   // TEXTURE_VARIATIONS_COUNT
#include "bflib_basics.h"      // SYNCLOG / ERRORLOG

#include <cstdlib>
#include <cstring>
#include "kfx/renderer/opengl/GLLog.h"
#include "kfx/renderer/opengl/GLCaps.h"
#include "post_inc.h"

/******************************************************************************/

bool GLTileAtlas::Init()
{
    if (m_initialized)
        return true;

    if (block_ptrs[0] == nullptr || !level_textures_ready)
    {
        SYNCDBG(7, "GLTileAtlas::Init — level texture data not ready yet");
        return false;
    }

    if (!m_r8_scratch)
    {
        m_r8_scratch = (uint8_t*)malloc((size_t)k_atlas_w * k_atlas_h);
        if (!m_r8_scratch)
        {
            ERRORLOG("GLTileAtlas::Init — failed to allocate R8 scratch buffer");
            return false;
        }
    }

    if (!CreateTexture())
        return false;

    for (int v = 0; v < TEXTURE_VARIATIONS_COUNT; v++)
        m_set_layer[v] = -1;
    m_layer_count = 0;

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    m_initialized = true;
    return true;
}

bool GLTileAtlas::CreateTexture()
{
    if (m_texture_handle != kInvalidGpuResource)
        return true;
    if (m_resource_mapper == nullptr)
    {
        ERRORLOG("GLTileAtlas::CreateTexture — no resource mapper set");
        return false;
    }

    m_layer_capacity = 2;
    m_texture_handle = m_resource_mapper->RequestCreateTexture(make_desc(m_layer_capacity));
    if (m_resource_mapper->ResolveTexture(m_texture_handle) == nullptr)
    {
        GLERR("Could not create the tile atlas. Switching to software.");
        m_texture_handle = kInvalidGpuResource;
        return false;
    }
    return true;
}

GpuTextureDesc GLTileAtlas::make_desc(int32_t layers) const
{
    GpuTextureDesc desc;
    desc.width = k_atlas_w;
    desc.height = k_atlas_h;
    desc.array_layers = layers;
    desc.format = GpuTextureFormat::R8;
    desc.min_filter = GpuTextureFilter::Nearest;
    desc.mag_filter = GpuTextureFilter::Nearest;
    desc.wrap = GpuTextureWrap::Clamp;
    desc.debug_name = "tile_atlas_array";
    return desc;
}

void GLTileAtlas::EnsureSets(uint32_t set_mask)
{
    if (!m_initialized || m_resource_mapper == nullptr || block_ptrs[0] == nullptr)
        return;
    bool any = false;
    for (int v = 0; v < TEXTURE_VARIATIONS_COUNT; v++)
    {
        if (!(set_mask & (1u << v)) || m_set_layer[v] >= 0)
            continue;
        if (m_layer_count == m_layer_capacity)
        {
            if (m_grow_failed)
                break;
            const int32_t next = AtlasNextCapacity(m_layer_capacity, m_layer_count + 1, TEXTURE_VARIATIONS_COUNT);
            const int32_t old = m_layer_capacity;
            if (next <= old || !m_resource_mapper->ReplaceTexture(m_texture_handle, make_desc(next),
                    [old](const GLTexture& from, const GLTexture& to) { return GLResourceMapper::CopyArrayLayers(from, to, old); }))
            {
                m_grow_failed = true; // logged once by ReplaceTexture; extra sets draw black
                break;
            }
            m_layer_capacity = next;
        }
        m_set_layer[v] = m_layer_count++;
        if (m_layer_count > m_layer_peak)
            m_layer_peak = m_layer_count;
        BuildVariation(v);
        m_built_generation[v] = texture_set_generation(v);
        any = true;
    }
    if (any)
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void GLTileAtlas::ResetSets()
{
    if (m_layer_count > 0)
        GLLOG("Tile sets: %d of %d layers used (peak %d).", (int)m_layer_count, (int)m_layer_capacity, (int)m_layer_peak);
    for (int v = 0; v < TEXTURE_VARIATIONS_COUNT; v++)
        m_set_layer[v] = -1;
    m_layer_count = 0;
}

void GLTileAtlas::Free()
{
    if (m_r8_scratch)
    {
        free(m_r8_scratch);
        m_r8_scratch = nullptr;
    }
    ResetSets(); // logs the layers used, as at level end
    m_initialized = false;
}

void GLTileAtlas::RebuildChanged()
{
    if (!m_initialized || block_ptrs[0] == nullptr) return;
    bool any = false;
    for (int v = 0; v < TEXTURE_VARIATIONS_COUNT; v++)
    {
        if (m_set_layer[v] < 0)
            continue;
        const uint32_t gen = texture_set_generation(v);
        if (gen == m_built_generation[v])
            continue;
        BuildVariation(v);
        m_built_generation[v] = gen;
        any = true;
    }
    if (any)
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void GLTileAtlas::UpdateAnimatedTiles()
{
    if (!m_initialized) return;
    for (int v = 0; v < TEXTURE_VARIATIONS_COUNT; v++)
        if (m_set_layer[v] >= 0)
            BuildAnimatedStrip(v);
}

GpuResourceHandle GLTileAtlas::GetAtlasTextureArray() const
{
    return m_initialized ? m_texture_handle : kInvalidGpuResource;
}

/******************************************************************************/
// TileAtlasPacker build overrides (R8 raw palette-index copy)

void GLTileAtlas::BuildVariation(int variation)
{
    memset(m_r8_scratch, 0, (size_t)k_atlas_w * k_atlas_h);

    const int total_tiles = TEXTURE_BLOCKS_COUNT;
    const int src_stride  = (int)(block_count_per_row * k_tile_dim);

    for (int tile_id = 0; tile_id < total_tiles; tile_id++)
    {
        const uint8_t* src = block_ptrs[variation * total_tiles + tile_id];
        if (!src) continue;

        const int col      = tile_id % k_atlas_cols;
        const int row      = tile_id / k_atlas_cols;
        uint8_t* dst_row0  = m_r8_scratch + (size_t)(row * k_tile_dim) * k_atlas_w
                                          + (size_t)(col * k_tile_dim);

        for (int y = 0; y < k_tile_dim; y++)
            memcpy(dst_row0 + (size_t)y * k_atlas_w, src + (size_t)y * src_stride, k_tile_dim);
    }

    UploadFull(variation);
}

void GLTileAtlas::BuildAnimatedStrip(int variation)
{
    const int first_tile  = TEXTURE_BLOCKS_STAT_COUNT_A;  // 544
    const int last_tile   = TEX_B_START_POINT;             // 999 (exclusive end)
    const int total_tiles = TEXTURE_BLOCKS_COUNT;

    const int first_row = first_tile / k_atlas_cols;
    const int last_row  = (last_tile - 1) / k_atlas_cols;
    const int y_offset  = first_row * k_tile_dim;
    const int h_pixels  = (last_row - first_row + 1) * k_tile_dim;

    memset(m_r8_scratch, 0, (size_t)k_atlas_w * h_pixels);

    const int src_stride = (int)(block_count_per_row * k_tile_dim);

    // The animated tile range (first_tile..last_tile-1) shares atlas rows with
    // static tiles at both edges: static-A tiles first_row*k_atlas_cols..(first_tile-1)
    // occupy the beginning of first_row, and static-B tiles last_tile..(last_row+1)*k_atlas_cols-1
    // occupy the tail of last_row.  The memset above zeroed those positions, so we
    // must re-fill them here alongside the animated tiles to avoid overwriting them
    // with zeros on every animation tick.
    const int strip_start = first_row * k_atlas_cols;
    const int strip_end   = (last_row + 1) * k_atlas_cols;

    for (int tile_id = strip_start; tile_id < strip_end && tile_id < total_tiles; tile_id++)
    {
        const uint8_t* src = block_ptrs[variation * total_tiles + tile_id];
        if (!src) continue;

        const int col       = tile_id % k_atlas_cols;
        const int strip_row = tile_id / k_atlas_cols - first_row;
        uint8_t* dst_row0   = m_r8_scratch + (size_t)(strip_row * k_tile_dim) * k_atlas_w
                                           + (size_t)(col * k_tile_dim);

        for (int y = 0; y < k_tile_dim; y++)
            memcpy(dst_row0 + (size_t)y * k_atlas_w, src + (size_t)y * src_stride, k_tile_dim);
    }

    UploadAnimatedStrip(variation, y_offset, h_pixels);
}

/******************************************************************************/
// TileAtlasPacker upload hooks

void GLTileAtlas::UploadFull(int variation)
{
    if (m_resource_mapper == nullptr) return;
    const GLTexture* const tex = m_resource_mapper->ResolveTexture(m_texture_handle);
    if (tex == nullptr || m_set_layer[variation] < 0) return;

    glBindTexture(GL_TEXTURE_2D_ARRAY, tex->id);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0, 0, m_set_layer[variation],
                    k_atlas_w, k_atlas_h, 1,
                    GL_RED, GL_UNSIGNED_BYTE,
                    m_r8_scratch);
}

void GLTileAtlas::UploadAnimatedStrip(int variation, int y_offset, int h_pixels)
{
    if (m_resource_mapper == nullptr) return;
    const GLTexture* const tex = m_resource_mapper->ResolveTexture(m_texture_handle);
    if (tex == nullptr || m_set_layer[variation] < 0) return;

    glBindTexture(GL_TEXTURE_2D_ARRAY, tex->id);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0, y_offset, m_set_layer[variation],
                    k_atlas_w, h_pixels, 1,
                    GL_RED, GL_UNSIGNED_BYTE,
                    m_r8_scratch);
}
