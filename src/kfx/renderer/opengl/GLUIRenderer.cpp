#include "pre_inc.h"
#include "kfx/renderer/opengl/GLUIRenderer.h"
#include "kfx/renderer/opengl/GLSpriteAtlas.h"
#include "kfx/renderer/opengl/GLTextRenderer.h"
#include "kfx/renderer/opengl/GLShaders.h"
#include "kfx/renderer/opengl/GLFunctions.h"
#include "kfx/renderer/opengl/GLResourceMapper.h"
#include "kfx/renderer/ir/UICommands.h"
#include "kfx/renderer/ir/TextCommands.h"
#include "kfx/renderer/RendererManager.h"
#include "bflib_basics.h"
#include "bflib_video.h" // Lb_SPRITE_TRANSPAR4/8, Lb_SPRITE_FLIP_HORIZ/VERTIC
#include "bflib_sprfnt.h" // LbDbcGetGlyphBits
#include <vector>
#include <algorithm>
#include <array>
#include <cstddef>
#include <unordered_set>
#include "post_inc.h"

namespace {

// Swap the U or V pair to mirror the sprite instead of touching vertex
// positions -- draw_textured_quad()'s quad geometry stays untouched, so the
// existing dst rect (x,y,w,h) is unaffected either way.
void apply_flip_flags(TbDrawFlagsMask draw_flags, float& u0, float& v0, float& u1, float& v1)
{
    if (draw_flags & Lb_SPRITE_FLIP_HORIZ) std::swap(u0, u1);
    if (draw_flags & Lb_SPRITE_FLIP_VERTIC) std::swap(v0, v1);
}

} // namespace

void GLUIRenderer::PaletteColour(unsigned char idx, float* r, float* g, float* b) const
{
    if (m_frame_palette == nullptr) { *r = *g = *b = 1.0f; return; }
    *r = (float)m_frame_palette[idx * 4 + 0] / 255.0f;
    *g = (float)m_frame_palette[idx * 4 + 1] / 255.0f;
    *b = (float)m_frame_palette[idx * 4 + 2] / 255.0f;
}

namespace {
// Shared attribute layout for both VAO/VBO pairs below: 9 floats/vertex --
// x,y (pos), u,v, r,g,b,a (color), z (NDC depth, used by depth-tested
// world-overlay draws so they sort against world geometry).
std::vector<GpuVertexAttribDesc> UIVertexAttribs()
{
    std::vector<GpuVertexAttribDesc> attribs(4);
    attribs[0] = { 0, 2, GpuVertexAttribType::Float, 0 };
    attribs[1] = { 1, 2, GpuVertexAttribType::Float, 2 * (uint32_t)sizeof(float) };
    attribs[2] = { 2, 4, GpuVertexAttribType::Float, 4 * (uint32_t)sizeof(float) };
    attribs[3] = { 3, 1, GpuVertexAttribType::Float, 8 * (uint32_t)sizeof(float) };
    return attribs;
}
} // namespace

unsigned int GLUIRenderer::ResolveShaderId(GpuResourceHandle handle) const
{
    if (!m_resource_mapper) return 0;
    const GLProgram* prog = m_resource_mapper->ResolveProgram(handle);
    return prog ? prog->id : 0;
}

bool GLUIRenderer::Init()
{
    if (m_resource_mapper == nullptr)
    {
        ERRORLOG("GLUIRenderer::Init -- no resource mapper set");
        return false;
    }

    GpuProgramDesc sprite_desc;
    sprite_desc.vertex_src = UI_VERTEX_SHADER;
    sprite_desc.fragment_src = UI_SPRITE_FRAGMENT_SHADER;
    sprite_desc.debug_name = "ui_sprite";
    m_shader_sprite_handle = m_resource_mapper->RequestCreateProgram(sprite_desc);

    GpuProgramDesc colored_desc;
    colored_desc.vertex_src = UI_VERTEX_SHADER;
    colored_desc.fragment_src = UI_SPRITE_COLORED_FRAGMENT_SHADER;
    colored_desc.debug_name = "ui_sprite_colored";
    m_shader_sprite_colored_handle = m_resource_mapper->RequestCreateProgram(colored_desc);

    GpuProgramDesc remap_desc;
    remap_desc.vertex_src = UI_VERTEX_SHADER;
    remap_desc.fragment_src = UI_REMAP_FRAGMENT_SHADER;
    remap_desc.debug_name = "ui_remap";
    m_shader_remap_handle = m_resource_mapper->RequestCreateProgram(remap_desc);

    GpuProgramDesc solid_desc;
    solid_desc.vertex_src = UI_VERTEX_SHADER;
    solid_desc.fragment_src = UI_SOLID_FRAGMENT_SHADER;
    solid_desc.debug_name = "ui_solid";
    m_shader_solid_handle = m_resource_mapper->RequestCreateProgram(solid_desc);

    if (!ResolveShaderId(m_shader_sprite_handle) || !ResolveShaderId(m_shader_sprite_colored_handle)
        || !ResolveShaderId(m_shader_remap_handle) || !ResolveShaderId(m_shader_solid_handle))
        return false;

    GpuGeometryBufferDesc geom_desc;
    geom_desc.vertex_stride = 9 * (uint32_t)sizeof(float);
    geom_desc.attribs = UIVertexAttribs();
    geom_desc.dynamic = true;
    geom_desc.initial_vertex_capacity = 6 * 9 * sizeof(float);
    geom_desc.debug_name = "ui_quad";
    m_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(geom_desc);
    if (m_resource_mapper->ResolveGeometryBuffer(m_geom_handle) == nullptr)
        return false;

    GpuGeometryBufferDesc batch_desc;
    batch_desc.vertex_stride = 9 * (uint32_t)sizeof(float);
    batch_desc.attribs = UIVertexAttribs();
    batch_desc.dynamic = true;
    batch_desc.initial_vertex_capacity = kBatchVertexCapacity * 9 * sizeof(float);
    batch_desc.debug_name = "ui_batch";
    m_batch_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(batch_desc);
    if (m_resource_mapper->ResolveGeometryBuffer(m_batch_geom_handle) == nullptr)
        return false;

    GpuTextureDesc remap_tex_desc;
    remap_tex_desc.width = 256;
    remap_tex_desc.height = 64;
    remap_tex_desc.format = GpuTextureFormat::R8;
    remap_tex_desc.min_filter = GpuTextureFilter::Nearest;
    remap_tex_desc.mag_filter = GpuTextureFilter::Nearest;
    remap_tex_desc.wrap = GpuTextureWrap::Clamp;
    remap_tex_desc.initial_pixels.resize(256 * 64);
    remap_tex_desc.debug_name = "ui_custom_remap";
    m_custom_remap_tex_handle = m_resource_mapper->RequestCreateTexture(remap_tex_desc);
    remap_tex_desc.debug_name = "ui_text_remap";
    m_text_remap_tex_handle = m_resource_mapper->RequestCreateTexture(remap_tex_desc);
    if (m_resource_mapper->ResolveTexture(m_custom_remap_tex_handle) == nullptr ||
        m_resource_mapper->ResolveTexture(m_text_remap_tex_handle) == nullptr)
        return false;

    return true;
}

void GLUIRenderer::Shutdown() { }

SpriteHandle GLUIRenderer::ResolveSprite(const struct TbSprite* spr)
{
    SpriteHandle h = IUIRenderer::ResolveSprite(spr);
    if (h != kInvalidSpriteHandle && m_atlas)
        m_atlas->PackSprite(h, spr);
    return h;
}

void GLUIRenderer::draw_textured_quad(GpuResourceHandle shader_handle, float x, float y, float w, float h,
                                      float u0, float v0, float u1, float v1,
                                      float r, float g, float b, float a,
                                      float remap_row, GpuResourceHandle remap_tex_handle)
{
    if (!m_resource_mapper) return;
    const GLGeometryBuffer* geom = m_resource_mapper->ResolveGeometryBuffer(m_geom_handle);
    const unsigned int shader = ResolveShaderId(shader_handle);
    if (!shader || !geom) return;

    const float verts[6 * 9] = {
        x,     y,     u0, v0,  r, g, b, a, 0.0f,
        x + w, y,     u1, v0,  r, g, b, a, 0.0f,
        x + w, y + h, u1, v1,  r, g, b, a, 0.0f,
        x,     y,     u0, v0,  r, g, b, a, 0.0f,
        x + w, y + h, u1, v1,  r, g, b, a, 0.0f,
        x,     y + h, u0, v1,  r, g, b, a, 0.0f,
    };

    glUseProgram(shader);
    glUniform2f(glGetUniformLocation(shader, "u_screen_size"), (float)m_screen_w, (float)m_screen_h);

    glActiveTexture(GL_TEXTURE0);
    {
        const GLTexture* atlas_tex = (m_atlas && m_resource_mapper)
            ? m_resource_mapper->ResolveTexture(m_atlas->GetTexture()) : nullptr;
        glBindTexture(GL_TEXTURE_2D, atlas_tex ? atlas_tex->id : 0);
    }
    glUniform1i(glGetUniformLocation(shader, "u_sprite_atlas"), 0);

    if (shader_handle == m_shader_sprite_handle || shader_handle == m_shader_remap_handle) {
        const GLTexture* pal_tex = m_resource_mapper->ResolveTexture(m_palette_tex_handle);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pal_tex ? pal_tex->id : 0);
        glUniform1i(glGetUniformLocation(shader, "u_palette"), 1);
    }
    if (shader_handle == m_shader_remap_handle) {
        const GLTexture* fade_tex = m_resource_mapper->ResolveTexture(
            remap_tex_handle != kInvalidGpuResource ? remap_tex_handle : m_fade_table_tex_handle);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, fade_tex ? fade_tex->id : 0);
        glUniform1i(glGetUniformLocation(shader, "u_fade_table"), 2);
        glUniform1f(glGetUniformLocation(shader, "u_remap_row"), remap_row);
    }

    glBindVertexArray(geom->vao);
    glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void GLUIRenderer::draw_solid_quad(float x, float y, float w, float h, float r, float g, float b, float a)
{
    if (!m_resource_mapper) return;
    const GLGeometryBuffer* geom = m_resource_mapper->ResolveGeometryBuffer(m_geom_handle);
    const unsigned int shader = ResolveShaderId(m_shader_solid_handle);
    if (!shader || !geom) return;

    const float verts[6 * 9] = {
        x,     y,     0, 0,  r, g, b, a, 0.0f,
        x + w, y,     0, 0,  r, g, b, a, 0.0f,
        x + w, y + h, 0, 0,  r, g, b, a, 0.0f,
        x,     y,     0, 0,  r, g, b, a, 0.0f,
        x + w, y + h, 0, 0,  r, g, b, a, 0.0f,
        x,     y + h, 0, 0,  r, g, b, a, 0.0f,
    };

    glUseProgram(shader);
    glUniform2f(glGetUniformLocation(shader, "u_screen_size"), (float)m_screen_w, (float)m_screen_h);
    glBindVertexArray(geom->vao);
    glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void GLUIRenderer::DrawGlyphQuad(SpriteHandle glyph, float x, float y, int units_per_px,
                                 float r, float g, float b, float a, bool sample_palette)
{
    SpriteUV uv;
    if (!m_atlas || !m_atlas->GetUV(glyph, uv))
    {
        static std::unordered_set<SpriteHandle> s_logged;
        if (m_atlas && s_logged.insert(glyph).second)
            WARNLOG("GLUIRenderer::DrawGlyphQuad: GetUV(handle=%u) failed -- glyph/cursor not in atlas", (unsigned)glyph);
        return;
    }
    float w = uv.pixel_w * units_per_px / 16.0f;
    float h = uv.pixel_h * units_per_px / 16.0f;
    draw_textured_quad(sample_palette ? m_shader_sprite_handle : m_shader_sprite_colored_handle,
                        x, y, w, h, uv.u0, uv.v0, uv.u1, uv.v1, r, g, b, a);
}

void GLUIRenderer::DrawGlyphRemapQuad(SpriteHandle glyph, float x, float y, int units_per_px,
                                      float a, const std::array<unsigned char, 256>& remap)
{
    SpriteUV uv;
    if (!m_atlas || !m_atlas->GetUV(glyph, uv) || !m_resource_mapper)
        return;
    const GLTexture* tex = m_resource_mapper->ResolveTexture(m_text_remap_tex_handle);
    if (!tex)
        return;
    if (!m_text_remap_cached || m_text_remap_cache != remap)
    {
        glBindTexture(GL_TEXTURE_2D, tex->id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RED, GL_UNSIGNED_BYTE, remap.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        m_text_remap_cache = remap;
        m_text_remap_cached = true;
    }
    const float w = uv.pixel_w * units_per_px / 16.0f;
    const float h = uv.pixel_h * units_per_px / 16.0f;
    draw_textured_quad(m_shader_remap_handle, x, y, w, h, uv.u0, uv.v0, uv.u1, uv.v1,
                       1.0f, 1.0f, 1.0f, a, 0.0f, m_text_remap_tex_handle);
}

int GLUIRenderer::FindCustomRemap(const std::array<unsigned char, 256>& remap)
{
    for (size_t i = 0; i < m_custom_remaps.size(); ++i)
        if (m_custom_remaps[i] == remap)
            return (int)i;
    if (m_custom_remaps.size() >= 64)
    {
        WARNLOG("GLUIRenderer: too many distinct UI remap tables in one frame");
        return 0;
    }
    m_custom_remaps.push_back(remap);
    return (int)m_custom_remaps.size() - 1;
}

void GLUIRenderer::UploadCustomRemaps()
{
    if (m_custom_remaps_uploaded || m_custom_remaps.empty() || !m_resource_mapper)
        return;
    const GLTexture* tex = m_resource_mapper->ResolveTexture(m_custom_remap_tex_handle);
    if (!tex)
        return;
    glBindTexture(GL_TEXTURE_2D, tex->id);
    for (size_t i = 0; i < m_custom_remaps.size(); ++i)
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, (int)i, 256, 1, GL_RED, GL_UNSIGNED_BYTE,
                        m_custom_remaps[i].data());
    glBindTexture(GL_TEXTURE_2D, 0);
    m_custom_remaps_uploaded = true;
}

SpriteHandle GLUIRenderer::ResolveDbcGlyph(const struct AsianFont* font, uint32_t codepoint)
{
    if (!font) return kInvalidSpriteHandle;

    const uint64_t key = ((uint64_t)(uintptr_t)font << 20) | (uint64_t)(codepoint & 0xFFFFFu);
    SpriteHandle handle = kInvalidSpriteHandle;
    auto it = m_dbc_glyph_handles.find(key);
    if (it != m_dbc_glyph_handles.end())
    {
        handle = it->second;
        if (!m_atlas || m_atlas->Contains(handle))
            return handle;
        // Dropped by an atlas reset; expand and pack it again below.
    }

    const unsigned char* data = nullptr;
    int scanline = 0, w = 0, h = 0, spacing = 0, voffset = 0;
    if (LbDbcGetGlyphBits(font, codepoint, &data, &scanline, &w, &h, &spacing, &voffset) != 0
        || !data || w <= 0 || h <= 0)
        return kInvalidSpriteHandle;

    if (handle == kInvalidSpriteHandle)
    {
        handle = m_next_dbc_handle++;
        m_dbc_glyph_handles[key] = handle;
    }

    // Expand the 1bpp bitmap (MSB-first per byte, matching bflib_sprfnt.c's
    // dbc_draw_font_sprite()) into 8-bit-per-pixel "palette index" bytes so
    // it can share GLSpriteAtlas's shelf packer and R8 texture: 0 stays
    // transparent (discarded by UI_SPRITE_COLORED_FRAGMENT_SHADER), any
    // nonzero index draws fully opaque, flat-tinted by DrawGlyphQuad's r/g/b/a.
    std::vector<uint8_t> expanded((size_t)w * (size_t)h);
    for (int y = 0; y < h; ++y)
    {
        const unsigned char* src_row = data + (size_t)y * scanline;
        uint8_t* dst_row = expanded.data() + (size_t)y * w;
        for (int x = 0; x < w; ++x)
            dst_row[x] = ((src_row[x >> 3] >> (7 - (x & 7))) & 1) ? 255 : 0;
    }

    if (m_atlas)
        m_atlas->PackRaw(handle, expanded.data(), w, h);
    return handle;
}

GLUIRenderer::PassType GLUIRenderer::classify(float mode)
{
    switch ((int)(mode + 0.5f))
    {
    case PASS_SOLID:   return PASS_SOLID;
    case PASS_SLAB:    return PASS_SLAB;
    case PASS_COLORED: return PASS_COLORED;
    case PASS_REMAP:   return PASS_REMAP;
    case PASS_CUSTOM_REMAP: return PASS_CUSTOM_REMAP;
    case PASS_MINIMAP: return PASS_MINIMAP;
    default:           return PASS_SPRITE;
    }
}

void GLUIRenderer::UpdateSlabTexture(const unsigned char* data, int dim)
{
    if (!data || dim <= 0) return;
    std::lock_guard<std::mutex> guard(m_slab_mutex);
    m_slab_pending.assign(data, data + (size_t)dim * (size_t)dim);
    m_slab_pending_dim = dim;
}

void GLUIRenderer::FlushPendingSlabUpload()
{
    std::vector<uint8_t> pending;
    int dim = 0;
    {
        std::lock_guard<std::mutex> guard(m_slab_mutex);
        if (m_slab_pending_dim <= 0) return;
        pending.swap(m_slab_pending);
        dim = m_slab_pending_dim;
        m_slab_pending_dim = 0;
    }
    const unsigned char* data = pending.data();

    if (!m_resource_mapper) return; 
    if (m_slab_tex_handle == kInvalidGpuResource)
    {
        GpuTextureDesc desc;
        desc.width = dim;
        desc.height = dim;
        desc.format = GpuTextureFormat::R8;
        desc.min_filter = GpuTextureFilter::Nearest;
        desc.mag_filter = GpuTextureFilter::Nearest;
        desc.wrap = GpuTextureWrap::Repeat;
        desc.debug_name = "slab_tile";
        m_slab_tex_handle = m_resource_mapper->RequestCreateTexture(desc);
        m_slab_dim = 0; // force the subimage-upload path below on first content
    }

    const GLTexture* tex = m_resource_mapper->ResolveTexture(m_slab_tex_handle);
    if (tex == nullptr) return;

    if (dim != m_slab_dim && m_slab_dim != 0)
    {
        WARNLOG("GLUIRenderer::FlushPendingSlabUpload: slab dimension changed at runtime "
                "(%d -> %d) -- not supported, dropping update", m_slab_dim, dim);
        return;
    }

    // Substitute index 0 -> 1: sprites treat palette index 0 as transparent
    // (UI_SPRITE_FRAGMENT_SHADER, reused for PASS_SLAB, discards it).
    const int total = dim * dim;
    std::vector<uint8_t> slab_buf((size_t)total);
    for (int k = 0; k < total; ++k)
        slab_buf[(size_t)k] = data[k] ? data[k] : 1;

    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dim, dim, GL_RED, GL_UNSIGNED_BYTE, slab_buf.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    m_slab_dim = dim;
}

uint8_t* GLUIRenderer::AcquireMinimapBuffer(int /*screen_x*/, int /*screen_y*/, int size)
{
    if (size <= 0) return nullptr;
    auto& buf = m_minimap_cpu_buf[m_minimap_write_idx];
    const size_t needed = (size_t)size * (size_t)size;
    if (buf.size() != needed)
        buf.assign(needed, 0);
    else
        std::fill(buf.begin(), buf.end(), (uint8_t)0);
    m_minimap_cpu_size = size;
    return buf.data();
}

void GLUIRenderer::SubmitMinimap(int screen_x, int screen_y, int size,
                                 const int32_t* /*shape_start*/, const int32_t* /*shape_end*/)
{
    // shape_start/shape_end are unused here: any buffer cell panel_map_draw_pixel()
    // never wrote stays index 0 and is discarded by the sprite shader like any
    // other transparent texel, letting the panel-background quad already drawn
    // underneath show through -- see BackendCapabilities::compositesMinimapBackground.
    if (size <= 0 || m_minimap_cpu_size != size || !m_ui_write_cmds) return;
    IRUIMinimapCmd cmd;
    cmd.layer = ComputeCurrentLayer();
    cmd.x = screen_x;
    cmd.y = screen_y;
    ApplyWindowOffset(cmd.layer, cmd.x, cmd.y);
    cmd.size = size;
    cmd.slot = m_minimap_write_idx;
    cmd.ndc_z = ComputeCurrentNdcZ();
    cmd.seq = m_ui_write_cmds->NextSeq();
    m_ui_write_cmds->minimaps.Append(cmd);
    // The render thread reads this slot while the next frame fills the other one.
    m_minimap_write_idx = 1 - m_minimap_write_idx;
}

bool GLUIRenderer::UploadMinimap(int slot, int size)
{
    if (slot < 0 || slot > 1 || size <= 0 || !m_resource_mapper) return false;
    const auto& buf = m_minimap_cpu_buf[slot];
    if (buf.size() != (size_t)size * (size_t)size) return false;

    if (m_minimap_tex_handle == kInvalidGpuResource)
    {
        GpuTextureDesc desc;
        desc.width = size;
        desc.height = size;
        desc.format = GpuTextureFormat::R8;
        desc.min_filter = GpuTextureFilter::Nearest;
        desc.mag_filter = GpuTextureFilter::Nearest;
        desc.wrap = GpuTextureWrap::Clamp;
        desc.debug_name = "minimap";
        m_minimap_tex_handle = m_resource_mapper->RequestCreateTexture(desc);
        m_minimap_tex_size = 0;
    }
    const GLTexture* tex = m_resource_mapper->ResolveTexture(m_minimap_tex_handle);
    if (tex == nullptr) return false;

    glBindTexture(GL_TEXTURE_2D, tex->id);
    if (size != m_minimap_tex_size)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, size, size, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        m_minimap_tex_size = size;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size, size, GL_RED, GL_UNSIGNED_BYTE, buf.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void GLUIRenderer::AppendQuadsFromIR(const UICommandBuffers& ui, std::vector<UIQuad> (&out)[kLayerCount])
{
    struct Ref { uint32_t seq; uint8_t kind; uint32_t idx; };
    enum { K_Sprite = 0, K_OneColour, K_Scaled, K_ScaledOneColour, K_ScaledRemap, K_Box, K_Slab, K_Minimap };

    std::vector<Ref> order;
    order.reserve(ui.sprites.Size() + ui.sprites_one_colour.Size() + ui.sprites_scaled.Size() +
                  ui.sprites_scaled_one_colour.Size() + ui.sprites_scaled_remap.Size() +
                  ui.solid_boxes.Size() + ui.slab_backgrounds.Size() + ui.minimaps.Size());

    for (uint32_t i = 0; i < (uint32_t)ui.sprites.Size(); ++i)
        order.push_back({ ui.sprites.Data()[i].seq, K_Sprite, i });
    for (uint32_t i = 0; i < (uint32_t)ui.sprites_one_colour.Size(); ++i)
        order.push_back({ ui.sprites_one_colour.Data()[i].seq, K_OneColour, i });
    for (uint32_t i = 0; i < (uint32_t)ui.sprites_scaled.Size(); ++i)
        order.push_back({ ui.sprites_scaled.Data()[i].seq, K_Scaled, i });
    for (uint32_t i = 0; i < (uint32_t)ui.sprites_scaled_one_colour.Size(); ++i)
        order.push_back({ ui.sprites_scaled_one_colour.Data()[i].seq, K_ScaledOneColour, i });
    for (uint32_t i = 0; i < (uint32_t)ui.sprites_scaled_remap.Size(); ++i)
        order.push_back({ ui.sprites_scaled_remap.Data()[i].seq, K_ScaledRemap, i });
    for (uint32_t i = 0; i < (uint32_t)ui.solid_boxes.Size(); ++i)
        order.push_back({ ui.solid_boxes.Data()[i].seq, K_Box, i });
    for (uint32_t i = 0; i < (uint32_t)ui.slab_backgrounds.Size(); ++i)
        order.push_back({ ui.slab_backgrounds.Data()[i].seq, K_Slab, i });
    for (uint32_t i = 0; i < (uint32_t)ui.minimaps.Size(); ++i)
        order.push_back({ ui.minimaps.Data()[i].seq, K_Minimap, i });

    std::sort(order.begin(), order.end(), [](const Ref& a, const Ref& b) { return a.seq < b.seq; });

    // Clip to the command's graphics window; UVs move with the edges cut away.
    auto push_clipped = [&](IRUILayer layer, UIQuad q, const IRUIClip& clip) {
        const float qw = q.x1 - q.x0;
        const float qh = q.y1 - q.y0;
        if (qw <= 0.0f || qh <= 0.0f) return;
        const float x0 = std::max(q.x0, (float)clip.x);
        const float y0 = std::max(q.y0, (float)clip.y);
        const float x1 = std::min(q.x1, (float)(clip.x + clip.w));
        const float y1 = std::min(q.y1, (float)(clip.y + clip.h));
        if (x1 <= x0 || y1 <= y0) return;
        const float du = (q.u1 - q.u0) / qw;
        const float dv = (q.v1 - q.v0) / qh;
        const float u0 = q.u0 + (x0 - q.x0) * du;
        const float u1 = q.u0 + (x1 - q.x0) * du;
        const float v0 = q.v0 + (y0 - q.y0) * dv;
        const float v1 = q.v0 + (y1 - q.y0) * dv;
        q.x0 = x0; q.y0 = y0; q.x1 = x1; q.y1 = y1;
        q.u0 = u0; q.u1 = u1; q.v0 = v0; q.v1 = v1;
        out[(int)layer].push_back(q);
    };

    for (const Ref& ref : order)
    {
        switch (ref.kind)
        {
        case K_Sprite: {
            const IRUISpriteCmd& c = ui.sprites.Data()[ref.idx];
            SpriteUV uv;
            if (!m_atlas || !m_atlas->GetUV(c.sprite, uv)) break;
            float u0 = uv.u0, v0 = uv.v0, u1 = uv.u1, v1 = uv.v1;
            apply_flip_flags(c.draw_flags, u0, v0, u1, v1);
            UIQuad q;
            q.x0 = (float)c.x; q.y0 = (float)c.y;
            q.x1 = q.x0 + uv.pixel_w; q.y1 = q.y0 + uv.pixel_h;
            q.u0 = u0; q.v0 = v0; q.u1 = u1; q.v1 = v1;
            q.a = draw_flags_source_weight(c.draw_flags);
            q.ndc_z = c.ndc_z; q.mode = (float)PASS_SPRITE; q.seq = c.seq;
            push_clipped(c.layer, q, c.clip);
            break;
        }
        case K_OneColour: {
            const IRUISpriteOneColourCmd& c = ui.sprites_one_colour.Data()[ref.idx];
            SpriteUV uv;
            if (!m_atlas || !m_atlas->GetUV(c.sprite, uv)) break;
            float u0 = uv.u0, v0 = uv.v0, u1 = uv.u1, v1 = uv.v1;
            apply_flip_flags(c.draw_flags, u0, v0, u1, v1);
            UIQuad q;
            q.x0 = (float)c.x; q.y0 = (float)c.y;
            q.x1 = q.x0 + uv.pixel_w; q.y1 = q.y0 + uv.pixel_h;
            q.u0 = u0; q.v0 = v0; q.u1 = u1; q.v1 = v1;
            PaletteColour(c.colour, &q.r, &q.g, &q.b);
            q.a = draw_flags_source_weight(c.draw_flags);
            q.ndc_z = c.ndc_z; q.mode = (float)PASS_COLORED; q.seq = c.seq;
            push_clipped(c.layer, q, c.clip);
            break;
        }
        case K_Scaled: {
            const IRUISpriteScaledCmd& c = ui.sprites_scaled.Data()[ref.idx];
            SpriteUV uv;
            if (!m_atlas || !m_atlas->GetUV(c.sprite, uv)) break;
            float u0 = uv.u0, v0 = uv.v0, u1 = uv.u1, v1 = uv.v1;
            apply_flip_flags(c.draw_flags, u0, v0, u1, v1);
            UIQuad q;
            q.x0 = (float)c.x; q.y0 = (float)c.y;
            q.x1 = q.x0 + (float)c.w; q.y1 = q.y0 + (float)c.h;
            q.u0 = u0; q.v0 = v0; q.u1 = u1; q.v1 = v1;
            q.a = draw_flags_source_weight(c.draw_flags);
            q.ndc_z = c.ndc_z; q.mode = (float)PASS_SPRITE; q.seq = c.seq;
            push_clipped(c.layer, q, c.clip);
            break;
        }
        case K_ScaledOneColour: {
            const IRUISpriteScaledOneColourCmd& c = ui.sprites_scaled_one_colour.Data()[ref.idx];
            SpriteUV uv;
            if (!m_atlas || !m_atlas->GetUV(c.sprite, uv)) break;
            float u0 = uv.u0, v0 = uv.v0, u1 = uv.u1, v1 = uv.v1;
            apply_flip_flags(c.draw_flags, u0, v0, u1, v1);
            UIQuad q;
            q.x0 = (float)c.x; q.y0 = (float)c.y;
            q.x1 = q.x0 + (float)c.w; q.y1 = q.y0 + (float)c.h;
            q.u0 = u0; q.v0 = v0; q.u1 = u1; q.v1 = v1;
            PaletteColour(c.colour, &q.r, &q.g, &q.b);
            q.a = draw_flags_source_weight(c.draw_flags);
            q.ndc_z = c.ndc_z; q.mode = (float)PASS_COLORED; q.seq = c.seq;
            push_clipped(c.layer, q, c.clip);
            break;
        }
        case K_ScaledRemap: {
            const IRUISpriteScaledRemapCmd& c = ui.sprites_scaled_remap.Data()[ref.idx];
            SpriteUV uv;
            if (!m_atlas || !m_atlas->GetUV(c.sprite, uv)) break;
            float u0 = uv.u0, v0 = uv.v0, u1 = uv.u1, v1 = uv.v1;
            apply_flip_flags(c.draw_flags, u0, v0, u1, v1);
            UIQuad q;
            q.x0 = (float)c.x; q.y0 = (float)c.y;
            q.x1 = q.x0 + (float)c.w; q.y1 = q.y0 + (float)c.h;
            q.u0 = u0; q.v0 = v0; q.u1 = u1; q.v1 = v1;
            q.a = draw_flags_source_weight(c.draw_flags);
            q.ndc_z = c.ndc_z; q.mode = (float)PASS_CUSTOM_REMAP;
            q.remap_row = FindCustomRemap(c.cmap);
            q.seq = c.seq;
            push_clipped(c.layer, q, c.clip);
            break;
        }
        case K_Box: {
            const IRUISolidBoxCmd& c = ui.solid_boxes.Data()[ref.idx];
            float r, g, b;
            PaletteColour(c.colour, &r, &g, &b);
            const float a = draw_flags_source_weight(c.draw_flags);
            if (c.draw_flags & Lb_SPRITE_OUTLINE)
            {
                // Hollow border, matching LbDrawBoxImmediate()'s CPU geometry
                // (bflib_vidraw.c): top + bottom 1px lines always; left/right
                // 1px lines only when height > 2 (leaves the corners to the
                // top/bottom lines, same as the CPU version's LbDrawHVLine
                // ranges).
                auto push_rect = [&](float x, float y, float w, float h) {
                    if (w <= 0.0f || h <= 0.0f) return;
                    UIQuad q;
                    q.x0 = x; q.y0 = y; q.x1 = x + w; q.y1 = y + h;
                    q.r = r; q.g = g; q.b = b; q.a = a;
                    q.ndc_z = c.ndc_z; q.mode = (float)PASS_SOLID; q.seq = c.seq;
                    push_clipped(c.layer, q, c.clip);
                };
                push_rect((float)c.x, (float)c.y, (float)c.w, 1.0f);
                push_rect((float)c.x, (float)(c.y + c.h - 1), (float)c.w, 1.0f);
                if (c.h > 2)
                {
                    push_rect((float)c.x, (float)(c.y + 1), 1.0f, (float)(c.h - 2));
                    push_rect((float)(c.x + c.w - 1), (float)(c.y + 1), 1.0f, (float)(c.h - 2));
                }
            }
            else
            {
                UIQuad q;
                q.x0 = (float)c.x; q.y0 = (float)c.y;
                q.x1 = q.x0 + (float)c.w; q.y1 = q.y0 + (float)c.h;
                q.r = r; q.g = g; q.b = b; q.a = a;
                q.ndc_z = c.ndc_z; q.mode = (float)PASS_SOLID; q.seq = c.seq;
                push_clipped(c.layer, q, c.clip);
            }
            break;
        }
        case K_Minimap: {
            const IRUIMinimapCmd& c = ui.minimaps.Data()[ref.idx];
            if (!UploadMinimap(c.slot, c.size)) break;
            UIQuad q;
            q.x0 = (float)c.x; q.y0 = (float)c.y;
            q.x1 = q.x0 + (float)c.size; q.y1 = q.y0 + (float)c.size;
            q.u0 = 0.0f; q.v0 = 0.0f; q.u1 = 1.0f; q.v1 = 1.0f;
            q.ndc_z = c.ndc_z; q.mode = (float)PASS_MINIMAP; q.seq = c.seq;
            out[(int)c.layer].push_back(q);
            break;
        }
        case K_Slab: {
            const IRUISlabBackgroundCmd& c = ui.slab_backgrounds.Data()[ref.idx];
            int dim = m_slab_dim;
            if (dim <= 0)
            {
                std::lock_guard<std::mutex> guard(m_slab_mutex);
                dim = m_slab_pending_dim;
            }
            if (dim <= 0) break; // no slab texture uploaded (or pending) yet -- nothing to tile
            const float u1 = (float)c.w / (float)dim;
            const float v1 = (float)c.h / (float)dim;
            // Opaque backing quad first (blocks world bleed-through under
            // WorldOverlayFlat; harmless no-op layering for GameUI) --
            // painter's order puts the tiled quad on top right after it.
            UIQuad qbg;
            qbg.x0 = (float)c.x; qbg.y0 = (float)c.y;
            qbg.x1 = qbg.x0 + (float)c.w; qbg.y1 = qbg.y0 + (float)c.h;
            qbg.r = 0.0f; qbg.g = 0.0f; qbg.b = 0.0f; qbg.a = 1.0f;
            qbg.ndc_z = c.ndc_z; qbg.mode = (float)PASS_SOLID; qbg.seq = c.seq;
            out[(int)c.layer].push_back(qbg);

            UIQuad qt;
            qt.x0 = (float)c.x; qt.y0 = (float)c.y;
            qt.x1 = qt.x0 + (float)c.w; qt.y1 = qt.y0 + (float)c.h;
            qt.u0 = 0.0f; qt.v0 = 0.0f; qt.u1 = u1; qt.v1 = v1;
            qt.ndc_z = c.ndc_z; qt.mode = (float)PASS_SLAB; qt.seq = c.seq;
            out[(int)c.layer].push_back(qt);
            break;
        }
        }
    }
}

void GLUIRenderer::FlushQuadRun(const std::vector<UIQuad>& run, PassType pass, int remap_row)
{
    if (run.empty()) return;
    if (!m_resource_mapper) return;

    unsigned int shader = 0;
    unsigned int tex0 = 0;
    bool bind_palette = false;
    bool bind_fade = false;
    auto resolve_atlas_tex = [this]() -> unsigned int {
        if (!m_atlas || !m_resource_mapper) return 0;
        const GLTexture* tex = m_resource_mapper->ResolveTexture(m_atlas->GetTexture());
        return tex ? tex->id : 0;
    };
    switch (pass)
    {
    case PASS_SPRITE:  shader = ResolveShaderId(m_shader_sprite_handle);         tex0 = resolve_atlas_tex(); bind_palette = true; break;
    case PASS_SOLID:   shader = ResolveShaderId(m_shader_solid_handle);                                                          break;
    case PASS_COLORED: shader = ResolveShaderId(m_shader_sprite_colored_handle); tex0 = resolve_atlas_tex();                     break;
    case PASS_REMAP:   shader = ResolveShaderId(m_shader_remap_handle);          tex0 = resolve_atlas_tex(); bind_palette = true; bind_fade = true; break;
    case PASS_CUSTOM_REMAP:
        UploadCustomRemaps();
        shader = ResolveShaderId(m_shader_remap_handle); tex0 = resolve_atlas_tex(); bind_palette = true; bind_fade = true;
        break;
    case PASS_SLAB:
    {
        FlushPendingSlabUpload();
        const GLTexture* slab_tex = m_resource_mapper->ResolveTexture(m_slab_tex_handle);
        if (!slab_tex) return;
        shader = ResolveShaderId(m_shader_sprite_handle); tex0 = slab_tex->id; bind_palette = true;
        break;
    }
    case PASS_MINIMAP:
    {
        const GLTexture* mm_tex = m_resource_mapper->ResolveTexture(m_minimap_tex_handle);
        if (!mm_tex) return;
        shader = ResolveShaderId(m_shader_sprite_handle); tex0 = mm_tex->id; bind_palette = true;
        break;
    }
    }
    if (!shader) return;

    std::vector<float> verts;
    verts.reserve(run.size() * 6 * 9);
    for (const UIQuad& q : run)
    {
        const float vv[6][9] = {
            { q.x0, q.y0, q.u0, q.v0, q.r, q.g, q.b, q.a, q.ndc_z },
            { q.x1, q.y0, q.u1, q.v0, q.r, q.g, q.b, q.a, q.ndc_z },
            { q.x1, q.y1, q.u1, q.v1, q.r, q.g, q.b, q.a, q.ndc_z },
            { q.x0, q.y0, q.u0, q.v0, q.r, q.g, q.b, q.a, q.ndc_z },
            { q.x1, q.y1, q.u1, q.v1, q.r, q.g, q.b, q.a, q.ndc_z },
            { q.x0, q.y1, q.u0, q.v1, q.r, q.g, q.b, q.a, q.ndc_z },
        };
        for (const auto& row : vv)
            for (float f : row)
                verts.push_back(f);
    }

    glUseProgram(shader);
    glUniform2f(glGetUniformLocation(shader, "u_screen_size"), (float)m_screen_w, (float)m_screen_h);
    if (tex0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0);
        glUniform1i(glGetUniformLocation(shader, "u_sprite_atlas"), 0);
    }
    if (bind_palette)
    {
        const GLTexture* pal_tex = m_resource_mapper ? m_resource_mapper->ResolveTexture(m_palette_tex_handle) : nullptr;
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pal_tex ? pal_tex->id : 0);
        glUniform1i(glGetUniformLocation(shader, "u_palette"), 1);
    }
    if (bind_fade)
    {
        const GpuResourceHandle remap_tex = (pass == PASS_CUSTOM_REMAP) ? m_custom_remap_tex_handle : m_fade_table_tex_handle;
        const GLTexture* fade_tex = m_resource_mapper ? m_resource_mapper->ResolveTexture(remap_tex) : nullptr;
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, fade_tex ? fade_tex->id : 0);
        glUniform1i(glGetUniformLocation(shader, "u_fade_table"), 2);
        glUniform1f(glGetUniformLocation(shader, "u_remap_row"), (float)remap_row);
    }

    const GLGeometryBuffer* batch_geom = m_resource_mapper->ResolveGeometryBuffer(m_batch_geom_handle);
    if (!batch_geom) return;
    glBindVertexArray(batch_geom->vao);
    glBindBuffer(GL_ARRAY_BUFFER, batch_geom->vbo);
    // Orphan + refill: avoids the driver stalling this frame's batch on last
    // frame's still-in-flight draw of the same buffer.
    glBufferData(GL_ARRAY_BUFFER, (long)(verts.size() * sizeof(float)), nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (long)(verts.size() * sizeof(float)), verts.data());
    glDrawArrays(GL_TRIANGLES, 0, (int)(run.size() * 6));
    glBindVertexArray(0);
    glUseProgram(0);
}

void GLUIRenderer::FlushQuadLayer(std::vector<UIQuad>& quads, bool depth_test)
{
    if (quads.empty()) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (depth_test) { glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); glDepthMask(GL_FALSE); }
    else             glDisable(GL_DEPTH_TEST);

    std::vector<UIQuad> run;
    PassType run_pass = PASS_SPRITE;
    int run_remap_row = -1;
    for (const UIQuad& q : quads)
    {
        PassType p = classify(q.mode);
        bool remap_changed = ((p == PASS_REMAP || p == PASS_CUSTOM_REMAP) && q.remap_row != run_remap_row);
        if (!run.empty() && (p != run_pass || remap_changed))
        {
            FlushQuadRun(run, run_pass, run_remap_row);
            run.clear();
        }
        if (run.empty()) { run_pass = p; run_remap_row = q.remap_row; }
        run.push_back(q);
    }
    if (!run.empty())
        FlushQuadRun(run, run_pass, run_remap_row);

    if (depth_test) { glDepthMask(GL_TRUE); glDisable(GL_DEPTH_TEST); }
    glDisable(GL_BLEND);
    quads.clear();
}

void GLUIRenderer::BuildQuadsFromIR(const UICommandBuffers& ui)
{
    for (auto& v : m_quads) v.clear();
    m_custom_remaps.clear();
    m_custom_remaps_uploaded = false;
    AppendQuadsFromIR(ui, m_quads);
    m_game_vp_x   = ui.game_vp.x;
    m_game_vp_y   = ui.game_vp.y;
    m_game_vp_w   = ui.game_vp.w;
    m_game_vp_h   = ui.game_vp.h;
    m_game_vp_set = ui.game_vp.set;
}

void GLUIRenderer::DrawWorldSpriteLayerRT()
{
    // Guards against bleeding into the overhead map / zoom box regions.
    bool scissor = m_game_vp_set && m_game_vp_w > 0 && m_game_vp_h > 0;
    if (scissor)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(m_game_vp_x, m_screen_h - m_game_vp_y - m_game_vp_h, m_game_vp_w, m_game_vp_h);
    }
    // Depth-tested against world geometry -- creature status should occlude behind walls
    FlushQuadLayer(m_quads[(int)IRUILayer::WorldOverlay], /*depth_test=*/true);
    if (scissor) glDisable(GL_SCISSOR_TEST);
}

void GLUIRenderer::DrawWorldOverlayFlatLayerRT()
{
    bool scissor = m_game_vp_set && m_game_vp_w > 0 && m_game_vp_h > 0;
    if (scissor)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(m_game_vp_x, m_screen_h - m_game_vp_y - m_game_vp_h, m_game_vp_w, m_game_vp_h);
    }
    // Not depth-tested -- room flags/floating text always draw over world
    // geometry (matches develop's WorldOverlayFlat semantics).
    FlushQuadLayer(m_quads[(int)IRUILayer::WorldOverlayFlat], /*depth_test=*/false);
    if (scissor) glDisable(GL_SCISSOR_TEST);
}

void GLUIRenderer::DrawFrontOverlay()
{
    FlushQuadLayer(m_quads[(int)IRUILayer::Overlay], /*depth_test=*/false);
}

// Dodge, Duck, Dip, Dive and Dodge
void GLUIRenderer::DrawGameUIQuadsInterleaved(std::vector<UIQuad>& quads,
                                              const TextCommandBuffers& text,
                                              GLTextRenderer* text_renderer)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    const size_t qn = quads.size();
    const size_t tn = (text_renderer ? (size_t)text.draws.Size() : 0);
    size_t qi = 0, ti = 0;
    std::vector<UIQuad> run;
    PassType run_pass = PASS_SPRITE;
    int run_remap_row = -1;

    auto flush_run = [&]() {
        if (!run.empty()) { FlushQuadRun(run, run_pass, run_remap_row); run.clear(); }
    };

    while (qi < qn || ti < tn)
    {
        const bool take_quad = (ti >= tn) || (qi < qn && quads[qi].seq <= text.draws.Data()[ti].seq);
        if (take_quad)
        {
            const UIQuad& q = quads[qi++];
            PassType p = classify(q.mode);
            bool remap_changed = ((p == PASS_REMAP || p == PASS_CUSTOM_REMAP) && q.remap_row != run_remap_row);
            if (!run.empty() && (p != run_pass || remap_changed))
                flush_run();
            if (run.empty()) { run_pass = p; run_remap_row = q.remap_row; }
            run.push_back(q);
        }
        else
        {
            flush_run();
            text_renderer->DrawGlyphs(text.draws.Data()[ti++], text);
        }
    }
    flush_run();

    glDisable(GL_BLEND);
    quads.clear();
}

void GLUIRenderer::DrawGameUILayerRT(const TextCommandBuffers& text, GLTextRenderer* text_renderer)
{
    DrawGameUIQuadsInterleaved(m_quads[(int)IRUILayer::GameUI], text, text_renderer);
}

void GLUIRenderer::DrawFromIR(const UICommandBuffers& ui, const TextCommandBuffers& text,
                              GLTextRenderer* text_renderer)
{
    std::vector<UIQuad> local[kLayerCount];
    AppendQuadsFromIR(ui, local);

    FlushQuadLayer(local[(int)IRUILayer::WorldOverlay], /*depth_test=*/true);
    FlushQuadLayer(local[(int)IRUILayer::WorldOverlayFlat], /*depth_test=*/false);
    DrawGameUIQuadsInterleaved(local[(int)IRUILayer::GameUI], text, text_renderer);
    FlushQuadLayer(local[(int)IRUILayer::Overlay], /*depth_test=*/false);
}
