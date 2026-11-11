/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLPaletteIndexLookup.cpp
 *     See GLPaletteIndexLookup.h.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/opengl/GLPaletteIndexLookup.h"
#include "kfx/renderer/opengl/GLFunctions.h"
#include "kfx/renderer/opengl/GLResourceMapper.h"
#include "bflib_video.h"   // LbPaletteFindColour
#include <cstring>
#include "post_inc.h"

namespace {
constexpr int k_dim = 512;
constexpr int k_keys = k_dim * k_dim;

inline uint32_t key6(unsigned r, unsigned g, unsigned b) { return (r << 12) | (g << 6) | b; }

// Inverse of RendererPaletteChannel8(); exact for every 6-bit value.
inline unsigned channel6(unsigned char c8) { return (unsigned)((c8 * 63 + 127) / 255); }
}

bool GLPaletteIndexLookup::Init(GLResourceMapper* mapper)
{
    m_mapper = mapper;
    m_base.assign(k_keys, 0);
    m_table.assign(k_keys, 0);
    m_have_palette = false;

    GpuTextureDesc desc;
    desc.width = k_dim;
    desc.height = k_dim;
    desc.format = GpuTextureFormat::R8;
    desc.min_filter = GpuTextureFilter::Nearest;
    desc.mag_filter = GpuTextureFilter::Nearest;
    desc.wrap = GpuTextureWrap::Clamp;
    desc.initial_pixels = m_table;
    desc.debug_name = "palette_index_lookup";
    m_tex_handle = m_mapper->RequestCreateTexture(desc);
    return m_mapper->ResolveTexture(m_tex_handle) != nullptr;
}

void GLPaletteIndexLookup::Shutdown()
{
    m_base.clear();
    m_table.clear();
    m_have_palette = false;
}

void GLPaletteIndexLookup::RefreshBase(const unsigned char* engine_palette)
{
    if (engine_palette == nullptr || m_base.empty())
        return;
    // Same sample points as compute_rgb2idx_table().
    uint8_t grid[16 * 16 * 16];
    for (unsigned r = 0; r < 16; r++)
        for (unsigned g = 0; g < 16; g++)
            for (unsigned b = 0; b < 16; b++)
                grid[(r << 8) | (g << 4) | b] = LbPaletteFindColour(engine_palette, 4 * r + 3, 4 * g + 3, 4 * b + 3);

    for (unsigned r = 0; r < 64; r++)
        for (unsigned g = 0; g < 64; g++)
            for (unsigned b = 0; b < 64; b++)
                m_base[key6(r, g, b)] = grid[((r >> 2) << 8) | ((g >> 2) << 4) | (b >> 2)];

    m_table = m_base;
    if (m_have_palette)
        apply_palette();
    upload();
}

void GLPaletteIndexLookup::SetPalette(const unsigned char* rgba)
{
    if (rgba == nullptr || m_table.empty())
        return;
    if (m_have_palette && std::memcmp(m_palette_rgba, rgba, sizeof(m_palette_rgba)) == 0)
        return;

    if (m_have_palette)
    {
        for (int i = 0; i < 256; i++)
            m_table[m_palette_keys[i]] = m_base[m_palette_keys[i]];
    }
    std::memcpy(m_palette_rgba, rgba, sizeof(m_palette_rgba));
    m_have_palette = true;
    apply_palette();
    upload();
}

void GLPaletteIndexLookup::apply_palette()
{
    // Highest index first, so the lowest index wins a shared colour.
    for (int i = 255; i >= 0; i--)
    {
        const uint32_t key = key6(channel6(m_palette_rgba[i * 4 + 0]),
                                  channel6(m_palette_rgba[i * 4 + 1]),
                                  channel6(m_palette_rgba[i * 4 + 2]));
        m_palette_keys[i] = key;
        m_table[key] = (uint8_t)i;
    }
}

void GLPaletteIndexLookup::upload()
{
    const GLTexture* tex = m_mapper ? m_mapper->ResolveTexture(m_tex_handle) : nullptr;
    if (tex == nullptr)
        return;
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, k_dim, k_dim, GL_RED, GL_UNSIGNED_BYTE, m_table.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/******************************************************************************/
