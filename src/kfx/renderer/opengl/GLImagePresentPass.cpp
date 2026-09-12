/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLImagePresentPass.cpp
 *     See GLImagePresentPass.h for the design rationale.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/opengl/GLImagePresentPass.h"
#include "kfx/renderer/opengl/GLShaders.h"
#include "kfx/renderer/opengl/GLResourceMapper.h"
#include "kfx/renderer/RendererManager.h"  // RendererPresentImageDesc, PRESENT_*
#include "kfx/renderer/RendererThread.h"   // ASSERT_GAME_THREAD/ASSERT_RENDER_THREAD
#include "bflib_basics.h"                  // ERRORLOG
#include <cstring>
#include "post_inc.h"

GLuint GLImagePresentPass::ResolvePaletteTexId() const
{
    const GLTexture* tex = m_resource_mapper ? m_resource_mapper->ResolveTexture(m_palette_tex_handle) : nullptr;
    return tex ? tex->id : 0;
}

/******************************************************************************/

GLImagePresentPass::~GLImagePresentPass()
{
    // Shutdown() must be called explicitly on the render thread before
    // destruction -- see GLMapFadePass's own destructor comment.
}

bool GLImagePresentPass::CompileShaders()
{
    if (m_resource_mapper == nullptr)
    {
        ERRORLOG("GLImagePresentPass::CompileShaders -- no resource mapper set");
        return false;
    }

    if (!init_quad())
        return false;

    GpuProgramDesc blit_desc;
    blit_desc.vertex_src = RAWIMAGE_VERTEX_SHADER;
    blit_desc.fragment_src = RAWIMAGE_BLIT_FRAGMENT_SHADER;
    blit_desc.debug_name = "rawimage_blit";
    m_shader_handle = m_resource_mapper->RequestCreateProgram(blit_desc);

    GpuProgramDesc transparent_desc;
    transparent_desc.vertex_src = RAWIMAGE_VERTEX_SHADER;
    transparent_desc.fragment_src = RAWIMAGE_TRANSPARENT_FRAGMENT_SHADER;
    transparent_desc.debug_name = "rawimage_transparent";
    m_transparent_shader_handle = m_resource_mapper->RequestCreateProgram(transparent_desc);

    GpuProgramDesc zoom_desc;
    zoom_desc.vertex_src = RAWIMAGE_VERTEX_SHADER;
    zoom_desc.fragment_src = RAWIMAGE_ZOOM_FRAGMENT_SHADER;
    zoom_desc.debug_name = "rawimage_zoom";
    m_zoom_shader_handle = m_resource_mapper->RequestCreateProgram(zoom_desc);

    const GLProgram* shader = m_resource_mapper->ResolveProgram(m_shader_handle);
    const GLProgram* transparent_shader = m_resource_mapper->ResolveProgram(m_transparent_shader_handle);
    const GLProgram* zoom_shader = m_resource_mapper->ResolveProgram(m_zoom_shader_handle);
    if (!shader || !transparent_shader || !zoom_shader)
        return false;

    glUseProgram(shader->id);
    glUniform1i(glGetUniformLocation(shader->id, "u_image"), 0);
    glUniform1i(glGetUniformLocation(shader->id, "u_palette"), 1);
    m_loc_screen_size = glGetUniformLocation(shader->id, "u_screen_size");

    glUseProgram(transparent_shader->id);
    glUniform1i(glGetUniformLocation(transparent_shader->id, "u_image"), 0);
    glUniform1i(glGetUniformLocation(transparent_shader->id, "u_palette"), 1);

    glUseProgram(zoom_shader->id);
    glUniform1i(glGetUniformLocation(zoom_shader->id, "u_image"), 0);
    glUniform1i(glGetUniformLocation(zoom_shader->id, "u_palette"), 1);
    m_loc_zoom_screen_size    = glGetUniformLocation(zoom_shader->id, "u_screen_size");
    m_loc_zoom_center_map     = glGetUniformLocation(zoom_shader->id, "u_zoom_center_map");
    m_loc_zoom_screen_center  = glGetUniformLocation(zoom_shader->id, "u_zoom_screen_center");
    m_loc_zoom_scale          = glGetUniformLocation(zoom_shader->id, "u_zoom_scale");
    m_loc_zoom_src_size       = glGetUniformLocation(zoom_shader->id, "u_src_size");

    glUseProgram(0);
    return true;
}

bool GLImagePresentPass::init_quad()
{
    // Positions are placeholders (screen pixels), rewritten every draw via
    // glBufferSubData once the destination rect for that draw is known.
    const float k_quad[] = {
        //  x,    y,    u,    v
        0.0f, 0.0f,  0.0f, 0.0f,
        0.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 0.0f,  1.0f, 1.0f,
        0.0f, 0.0f,  0.0f, 0.0f,
        0.0f, 0.0f,  1.0f, 1.0f,
        0.0f, 0.0f,  0.0f, 1.0f,
    };

    GpuGeometryBufferDesc geom_desc;
    geom_desc.vertex_stride = 4 * (uint32_t)sizeof(float);
    geom_desc.attribs = {
        { 0, 2, GpuVertexAttribType::Float, 0 },
        { 1, 2, GpuVertexAttribType::Float, 2 * (uint32_t)sizeof(float) },
    };
    geom_desc.dynamic = true;
    geom_desc.initial_vertex_capacity = sizeof(k_quad);
    geom_desc.debug_name = "rawimage_quad";
    m_quad_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(geom_desc);

    const GLGeometryBuffer* geom = m_resource_mapper->ResolveGeometryBuffer(m_quad_geom_handle);
    if (!geom) return false;

    // Seed with the placeholder quad -- RealizeGeometryBuffer() only
    // allocates storage (nullptr data), the actual initial content upload
    // happens here, same as the original glBufferData(..., k_quad, ...) did.
    glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(k_quad), k_quad);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

void GLImagePresentPass::Shutdown()
{
    // This runs inside RendererOpenGL::render_thread_cleanup() on the
    // render thread -- RequestRelease() is game-thread-only, so none of
    // the mapper-owned resources above are released here. ShutdownAll()
    // destroys them unconditionally instead.
    m_tex_gt_w = m_tex_gt_h = 0;
    m_overlay_tex_gt_w = m_overlay_tex_gt_h = 0;
    m_zoom_tex_gt_w = m_zoom_tex_gt_h = 0;
    m_zoom_tex_identity = nullptr;
    m_zoom_tex_rt_w = m_zoom_tex_rt_h = 0;
    m_cmd = IRImagePresentCmd{};
    m_rt_cmd = IRImagePresentCmd{};
    m_overlay_cmd = IRImagePresentCmd{};
    m_rt_overlay_cmd = IRImagePresentCmd{};
    m_zoom_cmd = IRLandviewZoomCmd{};
    m_rt_zoom_cmd = IRLandviewZoomCmd{};
}

// Shared by Submit()'s two destinations (base/overlay) -- copies desc's
// fields and pixels into `dst`. See IRImagePresentCmd's comment for why the
// pixels are copied now rather than aliased.
static void fill_cmd(IRImagePresentCmd& dst, const struct RendererPresentImageDesc* desc)
{
    dst.active = true;
    dst.dst_x = desc->dst_x;
    dst.dst_y = desc->dst_y;
    dst.dst_w = desc->dst_w;
    dst.dst_h = desc->dst_h;
    dst.src_w = desc->src_w;
    dst.src_h = desc->src_h;
    dst.palette = desc->palette;

    dst.pixels.resize((size_t)desc->src_w * (size_t)desc->src_h);
    const int pitch = (desc->src_pitch > 0) ? desc->src_pitch : desc->src_w;
    for (int row = 0; row < desc->src_h; ++row)
    {
        std::memcpy(&dst.pixels[(size_t)row * desc->src_w],
                     desc->src + (size_t)row * pitch,
                     (size_t)desc->src_w);
    }

    if (desc->palette == PRESENT_PALETTE_EMBEDDED && desc->embedded_palette != nullptr)
        dst.embedded_palette.assign(desc->embedded_palette, desc->embedded_palette + 256 * 4);
    else
        dst.embedded_palette.clear();
}

void GLImagePresentPass::EnsureImageTextureHandle(GpuResourceHandle& handle, int& gt_w, int& gt_h, int new_w, int new_h)
{
    if (!m_resource_mapper) return;

    GpuTextureDesc desc;
    desc.width = new_w;
    desc.height = new_h;
    desc.format = GpuTextureFormat::R8;
    desc.min_filter = GpuTextureFilter::Nearest;
    desc.mag_filter = GpuTextureFilter::Nearest;
    desc.wrap = GpuTextureWrap::Clamp;
    desc.debug_name = "image_present";

    if (handle == kInvalidGpuResource)
    {
        handle = m_resource_mapper->RequestCreateTexture(desc);
        gt_w = new_w;
        gt_h = new_h;
    }
    else if (gt_w != new_w || gt_h != new_h)
    {
        handle = m_resource_mapper->RequestReloadTexture(handle, desc);
        gt_w = new_w;
        gt_h = new_h;
    }
}

void GLImagePresentPass::Submit(const struct RendererPresentImageDesc* desc)
{
    ASSERT_GAME_THREAD();
    if (desc == nullptr || desc->src == nullptr || desc->src_w <= 0 || desc->src_h <= 0)
        return;

    if (desc->kind == PRESENT_KIND_TRANSPARENT)
    {
        fill_cmd(m_overlay_cmd, desc);
        EnsureImageTextureHandle(m_overlay_tex_handle, m_overlay_tex_gt_w, m_overlay_tex_gt_h, desc->src_w, desc->src_h);
    }
    else
    {
        fill_cmd(m_cmd, desc);
        EnsureImageTextureHandle(m_tex_handle, m_tex_gt_w, m_tex_gt_h, desc->src_w, desc->src_h);
    }
}

void GLImagePresentPass::SubmitZoom(const unsigned char* src_buf, int src_w, int src_h,
                                    float center_map_x, float center_map_y,
                                    float screen_cx,    float screen_cy,
                                    float scale)
{
    ASSERT_GAME_THREAD();
    if (src_buf == nullptr || src_w <= 0 || src_h <= 0)
        return;

    m_zoom_cmd.active = true;
    m_zoom_cmd.src_buf = src_buf;
    m_zoom_cmd.src_w = src_w;
    m_zoom_cmd.src_h = src_h;
    m_zoom_cmd.center_map_x = center_map_x;
    m_zoom_cmd.center_map_y = center_map_y;
    m_zoom_cmd.screen_cx = screen_cx;
    m_zoom_cmd.screen_cy = screen_cy;
    m_zoom_cmd.scale = scale;

    EnsureImageTextureHandle(m_zoom_tex_handle, m_zoom_tex_gt_w, m_zoom_tex_gt_h, src_w, src_h);
}

void GLImagePresentPass::FlipBuffers()
{
    ASSERT_GAME_THREAD();
    m_rt_cmd = std::move(m_cmd);
    m_rt_overlay_cmd = std::move(m_overlay_cmd);
    m_rt_zoom_cmd = std::move(m_zoom_cmd);
    // `active` is a scalar -- std::move leaves it unchanged in the moved-from
    // object (same hazard IRMapFadeCmd/IRWorldLensCmd's own comments
    // document). Reset explicitly so a frame with no Submit()/SubmitZoom()
    // call starts clean rather than replaying stale state.
    m_cmd = IRImagePresentCmd{};
    m_overlay_cmd = IRImagePresentCmd{};
    m_zoom_cmd = IRLandviewZoomCmd{};
}

// Shared content-upload helper: uploads an R8 indexed8 image into an
// already-correctly-sized, already-resolved texture id. Resizing (create/
// reload) happens on the game thread now (EnsureImageTextureHandle(),
// called from Submit()/SubmitZoom()) -- gpu-resource-mapper-spec.md Part
// 6.7's remap-texture pattern, since RequestReloadTexture is game-thread-
// only and this runs on the render thread.
static void upload_indexed8_content(GLuint tex_id, const unsigned char* pixels, int w, int h)
{
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, pixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLImagePresentPass::upload_texture()
{
    if (!m_resource_mapper) return;
    const GLTexture* tex = m_resource_mapper->ResolveTexture(m_tex_handle);
    if (!tex) return;
    upload_indexed8_content(tex->id, m_rt_cmd.pixels.data(), m_rt_cmd.src_w, m_rt_cmd.src_h);
}

void GLImagePresentPass::upload_overlay_texture()
{
    if (!m_resource_mapper) return;
    const GLTexture* tex = m_resource_mapper->ResolveTexture(m_overlay_tex_handle);
    if (!tex) return;
    upload_indexed8_content(tex->id, m_rt_overlay_cmd.pixels.data(), m_rt_overlay_cmd.src_w, m_rt_overlay_cmd.src_h);
}

void GLImagePresentPass::upload_zoom_texture()
{
    if (!m_resource_mapper) return;
    // map_screen is session-static -- only re-upload when the pointer
    // identity actually changed (a new landview loaded), not every frame of
    // the zoom animation. See IRLandviewZoomCmd::src_buf's own comment. This
    // is a pure content-reupload-avoidance cache, separate from the handle
    // sizing already settled on the game thread (EnsureImageTextureHandle()).
    if (m_zoom_tex_identity == m_rt_zoom_cmd.src_buf
        && m_zoom_tex_rt_w == m_rt_zoom_cmd.src_w && m_zoom_tex_rt_h == m_rt_zoom_cmd.src_h)
        return;
    const GLTexture* tex = m_resource_mapper->ResolveTexture(m_zoom_tex_handle);
    if (!tex) return;
    upload_indexed8_content(tex->id, m_rt_zoom_cmd.src_buf, m_rt_zoom_cmd.src_w, m_rt_zoom_cmd.src_h);
    m_zoom_tex_identity = m_rt_zoom_cmd.src_buf;
    m_zoom_tex_rt_w = m_rt_zoom_cmd.src_w;
    m_zoom_tex_rt_h = m_rt_zoom_cmd.src_h;
}

void GLImagePresentPass::upload_embedded_palette()
{
    if (!m_resource_mapper) return;

    // Fixed 256x1 RGBA8, so this is a pure content update: never needs
    // RequestReloadTexture. RequestCreateTexture has no thread assert, so
    // creating it here (render thread, first use) is safe.
    if (m_embedded_palette_tex_handle == kInvalidGpuResource)
    {
        GpuTextureDesc desc;
        desc.width = 256;
        desc.height = 1;
        desc.format = GpuTextureFormat::RGBA8;
        desc.min_filter = GpuTextureFilter::Nearest;
        desc.mag_filter = GpuTextureFilter::Nearest;
        desc.wrap = GpuTextureWrap::Clamp;
        desc.debug_name = "fmv_embedded_palette";
        m_embedded_palette_tex_handle = m_resource_mapper->RequestCreateTexture(desc);
    }

    const GLTexture* tex = m_resource_mapper->ResolveTexture(m_embedded_palette_tex_handle);
    if (!tex) return;

    if (m_rt_cmd.embedded_palette.size() >= 256u * 4u)
    {
        // GL_BGRA upload into an RGBA8 texture: the driver does the channel
        // swap, so the AVFrame's native BGRA layout needs no CPU-side
        // reordering (same trick develop's own equivalent upload uses).
        glBindTexture(GL_TEXTURE_2D, tex->id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_BGRA, GL_UNSIGNED_BYTE,
                        m_rt_cmd.embedded_palette.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void GLImagePresentPass::draw_quad(GLuint program, GLuint image_tex, GLuint palette_tex,
                                   int dst_x, int dst_y, int dst_w, int dst_h,
                                   int screen_w, int screen_h)
{
    if (!m_resource_mapper) return;
    const GLGeometryBuffer* geom = m_resource_mapper->ResolveGeometryBuffer(m_quad_geom_handle);
    if (!geom) return;

    const float x0 = (float)dst_x;
    const float y0 = (float)dst_y;
    const float x1 = (float)(dst_x + dst_w);
    const float y1 = (float)(dst_y + dst_h);
    const float verts[] = {
        x0, y0,  0.0f, 0.0f,
        x1, y0,  1.0f, 0.0f,
        x1, y1,  1.0f, 1.0f,
        x0, y0,  0.0f, 0.0f,
        x1, y1,  1.0f, 1.0f,
        x0, y1,  0.0f, 1.0f,
    };
    glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glUseProgram(program);
    glUniform2f(glGetUniformLocation(program, "u_screen_size"), (float)screen_w, (float)screen_h);
    glBindVertexArray(geom->vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, palette_tex);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLImagePresentPass::Resolve(int screen_w, int screen_h)
{
    ASSERT_RENDER_THREAD();
    if (!m_resource_mapper) return;

    const GLuint palette_tex_id = ResolvePaletteTexId();
    const GLProgram* shader_prog = m_resource_mapper->ResolveProgram(m_shader_handle);
    if (!IsActiveRT() || !shader_prog || palette_tex_id == 0 || screen_w <= 0 || screen_h <= 0)
        return;
    const GLuint shader = shader_prog->id;

    glViewport(0, 0, screen_w, screen_h);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);

    const GLProgram* zoom_shader_prog = m_resource_mapper->ResolveProgram(m_zoom_shader_handle);

    // Base layer: the zoom transition and a plain opaque present are both
    // "this frame's entire background" content and never active together
    // (frontzoom_to_point() is the landview screen's own background draw;
    // nothing else calls Submit() with PRESENT_KIND_OPAQUE while it's
    // running) -- zoom takes priority if somehow both fired the same frame.
    if (m_rt_zoom_cmd.active && zoom_shader_prog)
    {
        upload_zoom_texture();
        const GLTexture* zoom_tex = m_resource_mapper->ResolveTexture(m_zoom_tex_handle);
        const GLGeometryBuffer* geom = m_resource_mapper->ResolveGeometryBuffer(m_quad_geom_handle);
        if (zoom_tex != nullptr && geom != nullptr)
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            const float verts[] = {
                0.0f,            0.0f,             0.0f, 0.0f,
                (float)screen_w, 0.0f,             1.0f, 0.0f,
                (float)screen_w, (float)screen_h,  1.0f, 1.0f,
                0.0f,            0.0f,             0.0f, 0.0f,
                (float)screen_w, (float)screen_h,  1.0f, 1.0f,
                0.0f,            (float)screen_h,  0.0f, 1.0f,
            };
            glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

            glUseProgram(zoom_shader_prog->id);
            glUniform2f(m_loc_zoom_screen_size, (float)screen_w, (float)screen_h);
            glUniform2f(m_loc_zoom_center_map, m_rt_zoom_cmd.center_map_x, m_rt_zoom_cmd.center_map_y);
            glUniform2f(m_loc_zoom_screen_center, m_rt_zoom_cmd.screen_cx, m_rt_zoom_cmd.screen_cy);
            glUniform1f(m_loc_zoom_scale, m_rt_zoom_cmd.scale);
            glUniform2f(m_loc_zoom_src_size, (float)m_rt_zoom_cmd.src_w, (float)m_rt_zoom_cmd.src_h);
            glBindVertexArray(geom->vao);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, zoom_tex->id);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, palette_tex_id);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            glBindVertexArray(0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    else if (m_rt_cmd.active)
    {
        upload_texture();
        const GLTexture* tex = m_resource_mapper->ResolveTexture(m_tex_handle);
        if (tex != nullptr)
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            GLuint palette_tex = palette_tex_id;
            if (m_rt_cmd.palette == PRESENT_PALETTE_EMBEDDED)
            {
                upload_embedded_palette();
                const GLTexture* embedded_tex = m_resource_mapper->ResolveTexture(m_embedded_palette_tex_handle);
                if (embedded_tex != nullptr)
                    palette_tex = embedded_tex->id;
            }
            draw_quad(shader, tex->id, palette_tex,
                     m_rt_cmd.dst_x, m_rt_cmd.dst_y, m_rt_cmd.dst_w, m_rt_cmd.dst_h,
                     screen_w, screen_h);
        }
    }

    // Transparent overlay (window-frame): drawn over whatever's already on
    // screen this frame -- the base/zoom layer above, or (if neither was
    // active) whatever the rest of the frame already put there. No clear.
    const GLProgram* transparent_shader_prog = m_resource_mapper->ResolveProgram(m_transparent_shader_handle);
    if (m_rt_overlay_cmd.active && transparent_shader_prog)
    {
        upload_overlay_texture();
        const GLTexture* overlay_tex = m_resource_mapper->ResolveTexture(m_overlay_tex_handle);
        if (overlay_tex != nullptr)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            draw_quad(transparent_shader_prog->id, overlay_tex->id, palette_tex_id,
                     m_rt_overlay_cmd.dst_x, m_rt_overlay_cmd.dst_y,
                     m_rt_overlay_cmd.dst_w, m_rt_overlay_cmd.dst_h,
                     screen_w, screen_h);
            glDisable(GL_BLEND);
        }
    }

    glUseProgram(0);
    glDepthMask(GL_TRUE);
}
