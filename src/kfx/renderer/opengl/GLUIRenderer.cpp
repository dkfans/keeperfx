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
#include "vidmode.h" // pixmap.fade_tables, for recovering a remap row from its pointer
#include <vector>
#include <algorithm>
#include <cstddef>
#include <unordered_set>
#include "post_inc.h"

namespace {

// TRANSPAR4/8 aren't a real alpha blend on the CPU path -- they dither
// through a palette-index blend table (pixmap.ghost, bflib_vidraw_spr_norm.c)
// with no GL equivalent -- so, matching develop's own approximation for this
// same flag pair, treat them as flat alpha instead. Values match develop's
// RendererSettings defaults (transpar4_alpha/transpar8_alpha) -- this branch
// doesn't have that config module yet (see the plan doc's config-module
// note), so these are fixed constants, not user-configurable, until it does.
constexpr float kTranspar4Alpha = 0.5f;
constexpr float kTranspar8Alpha = 0.25f;

// Priority matches the software renderer (bflib_vidraw.c): TRANSPAR4 checked first.
float alpha_from_draw_flags(TbDrawFlagsMask draw_flags)
{
    if (draw_flags & Lb_SPRITE_TRANSPAR4) return kTranspar4Alpha;
    if (draw_flags & Lb_SPRITE_TRANSPAR8) return kTranspar8Alpha;
    return 1.0f;
}

// Swap the U or V pair to mirror the sprite instead of touching vertex
// positions -- draw_textured_quad()'s quad geometry stays untouched, so the
// existing dst rect (x,y,w,h) is unaffected either way.
void apply_flip_flags(TbDrawFlagsMask draw_flags, float& u0, float& v0, float& u1, float& v1)
{
    if (draw_flags & Lb_SPRITE_FLIP_HORIZ) std::swap(u0, u1);
    if (draw_flags & Lb_SPRITE_FLIP_VERTIC) std::swap(v0, v1);
}

// palette channels are 6-bit (VGA); expand to 8-bit for display, then to [0,1].
inline float chan6_to_unit(unsigned char v) { return (float)((v * 255) / 63) / 255.0f; }

void palette_index_to_rgb(unsigned char idx, float* r, float* g, float* b)
{
    const unsigned char* pal = RendererGetActivePalette();
    if (!pal) { *r = *g = *b = 1.0f; return; }
    *r = chan6_to_unit(pal[idx * 3 + 0]);
    *g = chan6_to_unit(pal[idx * 3 + 1]);
    *b = chan6_to_unit(pal[idx * 3 + 2]);
}

} // namespace

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

    // Single-quad immediate path (text glyphs, cursor): exactly one quad
    // (6 verts) of dynamic storage.
    GpuGeometryBufferDesc geom_desc;
    geom_desc.vertex_stride = 9 * (uint32_t)sizeof(float);
    geom_desc.attribs = UIVertexAttribs();
    geom_desc.dynamic = true;
    geom_desc.initial_vertex_capacity = 6 * 9 * sizeof(float);
    geom_desc.debug_name = "ui_quad";
    m_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(geom_desc);
    if (m_resource_mapper->ResolveGeometryBuffer(m_geom_handle) == nullptr)
        return false;

    // Beat 5: separate, larger scratch VAO/VBO for batched layer draws (world
    // overlay / world overlay flat / game UI) -- m_geom_handle above stays
    // sized for exactly one quad, used by the unrelated single-quad
    // immediate path. Same attribute layout, bigger capacity.
    GpuGeometryBufferDesc batch_desc;
    batch_desc.vertex_stride = 9 * (uint32_t)sizeof(float);
    batch_desc.attribs = UIVertexAttribs();
    batch_desc.dynamic = true;
    batch_desc.initial_vertex_capacity = kBatchVertexCapacity * 9 * sizeof(float);
    batch_desc.debug_name = "ui_batch";
    m_batch_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(batch_desc);
    if (m_resource_mapper->ResolveGeometryBuffer(m_batch_geom_handle) == nullptr)
        return false;

    return true;
}

void GLUIRenderer::Shutdown()
{
    // GPU Resource Mapper: this runs inside RendererOpenGL::
    // render_thread_cleanup() on the render thread -- RequestRelease() is
    // game-thread-only, so none of the mapper-owned resources above are
    // released here. ShutdownAll() destroys them unconditionally instead.
}

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
                                      float remap_row)
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
        const GLTexture* fade_tex = m_resource_mapper->ResolveTexture(m_fade_table_tex_handle);
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
        // Diagnostic-only: text glyphs and the cursor both draw through this
        // function (GLTextRenderer/GLCursorLayer), unlike regular UI sprites
        // (GLUIRenderer::DrawFromIR's K_Sprite case, which uses the same
        // GetUV() and is confirmed working) -- if glyph/cursor handles are
        // failing this lookup specifically, this is the one place that would
        // otherwise silently swallow it every single frame. Logged once per
        // handle, not once per call.
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

SpriteHandle GLUIRenderer::ResolveDbcGlyph(const struct AsianFont* font, uint32_t codepoint)
{
    if (!font) return kInvalidSpriteHandle;

    const uint64_t key = ((uint64_t)(uintptr_t)font << 20) | (uint64_t)(codepoint & 0xFFFFFu);
    auto it = m_dbc_glyph_handles.find(key);
    if (it != m_dbc_glyph_handles.end())
        return it->second;

    const unsigned char* data = nullptr;
    int scanline = 0, w = 0, h = 0, spacing = 0, voffset = 0;
    if (LbDbcGetGlyphBits(font, codepoint, &data, &scanline, &w, &h, &spacing, &voffset) != 0
        || !data || w <= 0 || h <= 0)
        return kInvalidSpriteHandle;

    SpriteHandle handle = m_next_dbc_handle++;
    m_dbc_glyph_handles[key] = handle;

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
    if (mode < 0.5f)  return PASS_SPRITE;
    if (mode < 9.5f)  return PASS_SOLID;
    if (mode < 19.5f) return PASS_SLAB;
    if (mode < 29.5f) return PASS_COLORED;
    return PASS_REMAP;
}

void GLUIRenderer::UpdateSlabTexture(const unsigned char* data, int dim)
{
    if (!data || dim <= 0) return;
    // Store dim first (relaxed), then data (release) so the render thread's
    // acquire-load on data is guaranteed to see dim too -- same ordering
    // develop's own UpdateSlabTexture() uses.
    m_slab_pending_dim.store(dim, std::memory_order_relaxed);
    m_slab_pending_data.store(data, std::memory_order_release);
}

void GLUIRenderer::FlushPendingSlabUpload()
{
    const unsigned char* data = m_slab_pending_data.load(std::memory_order_acquire);
    if (!data) return;
    int dim = m_slab_pending_dim.load(std::memory_order_relaxed);
    if (dim <= 0) return;
    m_slab_pending_data.store(nullptr, std::memory_order_relaxed);
    m_slab_pending_dim.store(0, std::memory_order_relaxed);

    if (!m_resource_mapper) return;

    // GPU Resource Mapper: the slab tile's dimension is a fixed compile-time
    // constant in practice (GUI_SLAB_DIMENSION, gui_draw.h) -- this is a
    // content update, not a resize, matching the mapper spec's mist-texture
    // guidance (Part 6.7): create the slot once (render thread; RequestCreate*
    // has no thread assert), then only ever resolve + subimage-upload on
    // later calls. If dim ever genuinely changed at runtime, that would need
    // a game-thread-requested RequestReloadTexture instead -- not built here
    // since nothing exercises that path today.
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
    // (UI_SPRITE_FRAGMENT_SHADER, reused for PASS_SLAB, discards it), but the
    // slab tile must render fully opaque -- matches the CPU path's
    // draw_slab64k_background_immediate() (gui_draw.c), which memcpy's every
    // byte of gui_slab verbatim with no transparency concept at all.
    const int total = dim * dim;
    std::vector<uint8_t> slab_buf((size_t)total);
    for (int k = 0; k < total; ++k)
        slab_buf[(size_t)k] = data[k] ? data[k] : 1;

    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dim, dim, GL_RED, GL_UNSIGNED_BYTE, slab_buf.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    m_slab_dim = dim;
}

void GLUIRenderer::AppendQuadsFromIR(const UICommandBuffers& ui, std::vector<UIQuad> (&out)[kLayerCount])
{
    struct Ref { uint32_t seq; uint8_t kind; uint32_t idx; };
    enum { K_Sprite = 0, K_OneColour, K_Scaled, K_ScaledOneColour, K_ScaledRemap, K_Box, K_Slab };

    std::vector<Ref> order;
    order.reserve(ui.sprites.Size() + ui.sprites_one_colour.Size() + ui.sprites_scaled.Size() +
                  ui.sprites_scaled_one_colour.Size() + ui.sprites_scaled_remap.Size() +
                  ui.solid_boxes.Size() + ui.slab_backgrounds.Size());

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

    std::sort(order.begin(), order.end(), [](const Ref& a, const Ref& b) { return a.seq < b.seq; });

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
            q.a = alpha_from_draw_flags(c.draw_flags);
            q.ndc_z = c.ndc_z; q.mode = (float)PASS_SPRITE; q.seq = c.seq;
            out[(int)c.layer].push_back(q);
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
            palette_index_to_rgb(c.colour, &q.r, &q.g, &q.b);
            q.a = alpha_from_draw_flags(c.draw_flags);
            q.ndc_z = c.ndc_z; q.mode = (float)PASS_COLORED; q.seq = c.seq;
            out[(int)c.layer].push_back(q);
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
            q.a = alpha_from_draw_flags(c.draw_flags);
            q.ndc_z = c.ndc_z; q.mode = (float)PASS_SPRITE; q.seq = c.seq;
            out[(int)c.layer].push_back(q);
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
            palette_index_to_rgb(c.colour, &q.r, &q.g, &q.b);
            q.a = alpha_from_draw_flags(c.draw_flags);
            q.ndc_z = c.ndc_z; q.mode = (float)PASS_COLORED; q.seq = c.seq;
            out[(int)c.layer].push_back(q);
            break;
        }
        case K_ScaledRemap: {
            const IRUISpriteScaledRemapCmd& c = ui.sprites_scaled_remap.Data()[ref.idx];
            SpriteUV uv;
            if (!m_atlas || !m_atlas->GetUV(c.sprite, uv)) break;
            float u0 = uv.u0, v0 = uv.v0, u1 = uv.u1, v1 = uv.v1;
            apply_flip_flags(c.draw_flags, u0, v0, u1, v1);
            // cmap points into pixmap.fade_tables (64 rows x 256 cols);
            // recover the row index from the pointer offset.
            ptrdiff_t offset = c.cmap - pixmap.fade_tables;
            UIQuad q;
            q.x0 = (float)c.x; q.y0 = (float)c.y;
            q.x1 = q.x0 + (float)c.w; q.y1 = q.y0 + (float)c.h;
            q.u0 = u0; q.v0 = v0; q.u1 = u1; q.v1 = v1;
            q.a = alpha_from_draw_flags(c.draw_flags);
            q.ndc_z = c.ndc_z; q.mode = (float)PASS_REMAP;
            q.remap_row = (offset >= 0) ? (int)(offset / 256) : 0;
            q.seq = c.seq;
            out[(int)c.layer].push_back(q);
            break;
        }
        case K_Box: {
            const IRUISolidBoxCmd& c = ui.solid_boxes.Data()[ref.idx];
            float r, g, b;
            palette_index_to_rgb(c.colour, &r, &g, &b);
            const float a = alpha_from_draw_flags(c.draw_flags);
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
                    out[(int)c.layer].push_back(q);
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
                out[(int)c.layer].push_back(q);
            }
            break;
        }
        case K_Slab: {
            const IRUISlabBackgroundCmd& c = ui.slab_backgrounds.Data()[ref.idx];
            if (m_slab_dim <= 0 && !m_slab_pending_data.load(std::memory_order_relaxed))
                break; // no slab texture uploaded (or pending) yet -- nothing to tile
            const int dim = (m_slab_dim > 0) ? m_slab_dim
                                              : m_slab_pending_dim.load(std::memory_order_relaxed);
            if (dim <= 0) break;
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
    case PASS_SLAB:
    {
        FlushPendingSlabUpload();
        const GLTexture* slab_tex = m_resource_mapper->ResolveTexture(m_slab_tex_handle);
        if (!slab_tex) return;
        shader = ResolveShaderId(m_shader_sprite_handle); tex0 = slab_tex->id; bind_palette = true;
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
        const GLTexture* fade_tex = m_resource_mapper ? m_resource_mapper->ResolveTexture(m_fade_table_tex_handle) : nullptr;
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
        bool remap_changed = (p == PASS_REMAP && q.remap_row != run_remap_row);
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
    AppendQuadsFromIR(ui, m_quads);
}

void GLUIRenderer::DrawWorldSpriteLayerRT()
{
    // Depth-tested against world geometry -- creature status should occlude
    // behind walls (matches develop's WorldOverlay semantics).
    FlushQuadLayer(m_quads[(int)IRUILayer::WorldOverlay], /*depth_test=*/true);
}

void GLUIRenderer::DrawWorldOverlayFlatLayerRT()
{
    // Not depth-tested -- room flags/floating text always draw over world
    // geometry (matches develop's WorldOverlayFlat semantics).
    FlushQuadLayer(m_quads[(int)IRUILayer::WorldOverlayFlat], /*depth_test=*/false);
}

// Shared by DrawGameUILayerRT() (the real per-frame m_quads[] path) and
// DrawFromIR() (a local, one-shot quad vector for standalone buffers) --
// merges already seq-ordered quads with text draws by seq (same cross-
// family interleaving this renderer always did), batching consecutive
// same-pass quads instead of issuing one draw call per quad. A text draw
// always breaks the current run (text isn't part of the UIQuad/mode
// system, see the class comment). Clears `quads` when done.
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
            bool remap_changed = (p == PASS_REMAP && q.remap_row != run_remap_row);
            if (!run.empty() && (p != run_pass || remap_changed))
                flush_run();
            if (run.empty()) { run_pass = p; run_remap_row = q.remap_row; }
            run.push_back(q);
        }
        else
        {
            flush_run();
            text_renderer->DrawGlyphs(text.draws.Data()[ti++]);
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
    // Self-contained buffer (currently only fd.swipe_cmds -- see the class
    // comment): local quad arrays, never touching the per-frame m_quads[]
    // state, which may be mid-frame (built but not yet fully flushed) when
    // this runs -- FGFlushSwipeOverlay() fires between FGClearFrame()'s
    // BuildQuadsFromIR() and the 3 Draw*LayerRT() calls that drain it.
    std::vector<UIQuad> local[kLayerCount];
    AppendQuadsFromIR(ui, local);

    // Swipe (and any other standalone buffer) never sets ambient world-
    // overlay state, so local[WorldOverlay]/[WorldOverlayFlat] are always
    // empty in practice -- flushed anyway for correctness if that ever
    // changes, in the same relative order the frame graph uses.
    FlushQuadLayer(local[(int)IRUILayer::WorldOverlay], /*depth_test=*/true);
    FlushQuadLayer(local[(int)IRUILayer::WorldOverlayFlat], /*depth_test=*/false);
    DrawGameUIQuadsInterleaved(local[(int)IRUILayer::GameUI], text, text_renderer);
}
