/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLTileAtlas.cpp
 *     Desktop OpenGL tile texture atlas — GL upload implementation.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/opengl/GLTileAtlas.h"

#include "engine_textures.h"   // TEXTURE_VARIATIONS_COUNT
#include "bflib_basics.h"      // SYNCLOG / ERRORLOG

#include <cstdlib>
#include <cstring>
#include "post_inc.h"

/******************************************************************************/

bool GLTileAtlas::Init()
{
    if (m_initialized)
        return true;

    // block_mem is a fixed-size array (always non-null); block_ptrs[0] is
    // the real "textures loaded yet" signal.
    if (block_ptrs[0] == nullptr)
    {
        SYNCDBG(7, "GLTileAtlas::Init — block_mem not ready (deferred until setup_stuff)");
        return false;
    }

    // Allocate our own R8 scratch (1 byte/pixel — palette indices only).
    // We do NOT call InitPacker() because we don't use the base-class RGBA8 decode.
    if (!m_r8_scratch)
    {
        m_r8_scratch = (uint8_t*)malloc((size_t)k_atlas_w * k_atlas_h);
        if (!m_r8_scratch)
        {
            ERRORLOG("GLTileAtlas::Init — failed to allocate R8 scratch buffer");
            return false;
        }
    }

    // Single GL_TEXTURE_2D_ARRAY: width × height × variations, GL_R8.
    glGenTextures(1, &m_texture_array);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture_array);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    // Allocate storage for all layers at once (immutable-ish via glTexImage3D).
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8,
                 k_atlas_w, k_atlas_h, TEXTURE_VARIATIONS_COUNT,
                 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    for (int v = 0; v < TEXTURE_VARIATIONS_COUNT; v++)
        BuildVariation(v);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    m_initialized = true;
    SYNCLOG("GLTileAtlas: loaded %d variations (%d×%d×%d R8 texture array)",
            TEXTURE_VARIATIONS_COUNT, k_atlas_w, k_atlas_h, TEXTURE_VARIATIONS_COUNT);
    return true;
}

void GLTileAtlas::Free()
{
    if (m_r8_scratch)
    {
        free(m_r8_scratch);
        m_r8_scratch = nullptr;
    }
    if (m_initialized)
    {
        glDeleteTextures(1, &m_texture_array);
        m_texture_array = 0;
        m_initialized = false;
    }
}

void GLTileAtlas::UpdateAnimatedTiles()
{
    if (!m_initialized) return;
    for (int v = 0; v < TEXTURE_VARIATIONS_COUNT; v++)
        BuildAnimatedStrip(v);
}

unsigned int GLTileAtlas::GetAtlasTexture(int variation) const
{
    // Legacy per-variation query — returns the shared array texture.
    // The consumer should prefer GetAtlasTextureArray() and pass variation
    // as a vertex attribute / uniform instead.
    if (!m_initialized || variation < 0 || variation >= k_max_variations)
        return 0;
    return (unsigned int)m_texture_array;
}

unsigned int GLTileAtlas::GetAtlasTextureArray() const
{
    return m_initialized ? (unsigned int)m_texture_array : 0;
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
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture_array);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0, 0, variation,
                    k_atlas_w, k_atlas_h, 1,
                    GL_RED, GL_UNSIGNED_BYTE,
                    m_r8_scratch);
}

void GLTileAtlas::UploadAnimatedStrip(int variation, int y_offset, int h_pixels)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture_array);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0, y_offset, variation,
                    k_atlas_w, h_pixels, 1,
                    GL_RED, GL_UNSIGNED_BYTE,
                    m_r8_scratch);
}
