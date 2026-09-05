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
#include "kfx/renderer/RendererManager.h"  // RendererPresentImageDesc, PRESENT_*
#include "kfx/renderer/RendererThread.h"   // ASSERT_GAME_THREAD/ASSERT_RENDER_THREAD
#include "bflib_basics.h"                  // ERRORLOG
#include <cstring>
#include "post_inc.h"

/******************************************************************************/

// File-local copy of GLWorldViewRenderer.cpp's/GLMapFadePass.cpp's own
// compile_shader_src() -- static, no shared header for it yet.
static GLuint compile_shader_src(GLenum type, const char* src, const char* debug_name)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[512];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        ERRORLOG("GLImagePresentPass: shader '%s' compile error: %s", debug_name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

static GLuint link_program(GLuint vs, const char* fs_src, const char* debug_name)
{
    GLuint fs = compile_shader_src(GL_FRAGMENT_SHADER, fs_src, debug_name);
    if (!fs)
        return 0;
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(fs);
    GLint linked = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[512];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        ERRORLOG("GLImagePresentPass: '%s' link error: %s", debug_name, log);
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

GLImagePresentPass::~GLImagePresentPass()
{
    // Shutdown() must be called explicitly on the render thread before
    // destruction -- see GLMapFadePass's own destructor comment.
}

bool GLImagePresentPass::CompileShaders()
{
    GLuint vs = compile_shader_src(GL_VERTEX_SHADER, RAWIMAGE_VERTEX_SHADER, "rawimage_vert.glsl");
    if (!vs)
        return false;
    if (!init_quad())
    {
        glDeleteShader(vs);
        return false;
    }

    m_shader             = link_program(vs, RAWIMAGE_BLIT_FRAGMENT_SHADER, "rawimage_blit_frag.glsl");
    m_transparent_shader = link_program(vs, RAWIMAGE_TRANSPARENT_FRAGMENT_SHADER, "rawimage_transparent_frag.glsl");
    m_zoom_shader        = link_program(vs, RAWIMAGE_ZOOM_FRAGMENT_SHADER, "rawimage_zoom_frag.glsl");
    glDeleteShader(vs);

    if (!m_shader || !m_transparent_shader || !m_zoom_shader)
        return false;

    glUseProgram(m_shader);
    glUniform1i(glGetUniformLocation(m_shader, "u_image"), 0);
    glUniform1i(glGetUniformLocation(m_shader, "u_palette"), 1);
    m_loc_screen_size = glGetUniformLocation(m_shader, "u_screen_size");

    glUseProgram(m_transparent_shader);
    glUniform1i(glGetUniformLocation(m_transparent_shader, "u_image"), 0);
    glUniform1i(glGetUniformLocation(m_transparent_shader, "u_palette"), 1);

    glUseProgram(m_zoom_shader);
    glUniform1i(glGetUniformLocation(m_zoom_shader, "u_image"), 0);
    glUniform1i(glGetUniformLocation(m_zoom_shader, "u_palette"), 1);
    m_loc_zoom_screen_size    = glGetUniformLocation(m_zoom_shader, "u_screen_size");
    m_loc_zoom_center_map     = glGetUniformLocation(m_zoom_shader, "u_zoom_center_map");
    m_loc_zoom_screen_center  = glGetUniformLocation(m_zoom_shader, "u_zoom_screen_center");
    m_loc_zoom_scale          = glGetUniformLocation(m_zoom_shader, "u_zoom_scale");
    m_loc_zoom_src_size       = glGetUniformLocation(m_zoom_shader, "u_src_size");

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
    glGenVertexArrays(1, &m_quad_vao);
    glGenBuffers(1, &m_quad_vbo);
    glBindVertexArray(m_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(k_quad), k_quad, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return true;
}

void GLImagePresentPass::Shutdown()
{
    if (m_quad_vbo) { glDeleteBuffers(1, &m_quad_vbo); m_quad_vbo = 0; }
    if (m_quad_vao) { glDeleteVertexArrays(1, &m_quad_vao); m_quad_vao = 0; }
    if (m_shader)             { glDeleteProgram(m_shader);             m_shader = 0; }
    if (m_transparent_shader) { glDeleteProgram(m_transparent_shader); m_transparent_shader = 0; }
    if (m_zoom_shader)        { glDeleteProgram(m_zoom_shader);        m_zoom_shader = 0; }
    if (m_tex)         { glDeleteTextures(1, &m_tex);         m_tex = 0; }
    if (m_overlay_tex) { glDeleteTextures(1, &m_overlay_tex); m_overlay_tex = 0; }
    if (m_zoom_tex)    { glDeleteTextures(1, &m_zoom_tex);    m_zoom_tex = 0; }
    if (m_embedded_palette_tex) { glDeleteTextures(1, &m_embedded_palette_tex); m_embedded_palette_tex = 0; }
    m_tex_w = m_tex_h = 0;
    m_overlay_tex_w = m_overlay_tex_h = 0;
    m_zoom_tex_w = m_zoom_tex_h = 0;
    m_zoom_tex_identity = nullptr;
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

void GLImagePresentPass::Submit(const struct RendererPresentImageDesc* desc)
{
    ASSERT_GAME_THREAD();
    if (desc == nullptr || desc->src == nullptr || desc->src_w <= 0 || desc->src_h <= 0)
        return;

    if (desc->kind == PRESENT_KIND_TRANSPARENT)
        fill_cmd(m_overlay_cmd, desc);
    else
        fill_cmd(m_cmd, desc);
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

// Shared upload helper: (re)allocate `tex`/`tex_w`/`tex_h` to hold an R8
// indexed8 image, uploading via glTexImage2D on a size change or
// glTexSubImage2D otherwise. Mirrors upload_texture()'s original body,
// factored out so the base/overlay/zoom textures can all use it.
static void upload_indexed8(GLuint& tex, int& tex_w, int& tex_h,
                            const unsigned char* pixels, int w, int h)
{
    if (tex == 0)
    {
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        // NEAREST, not LINEAR: this texture holds raw 8-bit palette
        // *indices*, not colour -- see the original comment this was copied
        // from (GLImagePresentPass::upload_texture(), P5.8-era).
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, tex);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (tex_w != w || tex_h != h)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
        tex_w = w;
        tex_h = h;
    }
    else
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, pixels);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLImagePresentPass::upload_texture()
{
    upload_indexed8(m_tex, m_tex_w, m_tex_h, m_rt_cmd.pixels.data(), m_rt_cmd.src_w, m_rt_cmd.src_h);
}

void GLImagePresentPass::upload_overlay_texture()
{
    upload_indexed8(m_overlay_tex, m_overlay_tex_w, m_overlay_tex_h,
                    m_rt_overlay_cmd.pixels.data(), m_rt_overlay_cmd.src_w, m_rt_overlay_cmd.src_h);
}

void GLImagePresentPass::upload_zoom_texture()
{
    // map_screen is session-static -- only re-upload when the pointer
    // identity actually changed (a new landview loaded), not every frame of
    // the zoom animation. See IRLandviewZoomCmd::src_buf's own comment.
    if (m_zoom_tex != 0 && m_zoom_tex_identity == m_rt_zoom_cmd.src_buf
        && m_zoom_tex_w == m_rt_zoom_cmd.src_w && m_zoom_tex_h == m_rt_zoom_cmd.src_h)
        return;
    upload_indexed8(m_zoom_tex, m_zoom_tex_w, m_zoom_tex_h, m_rt_zoom_cmd.src_buf,
                    m_rt_zoom_cmd.src_w, m_rt_zoom_cmd.src_h);
    m_zoom_tex_identity = m_rt_zoom_cmd.src_buf;
}

void GLImagePresentPass::upload_embedded_palette()
{
    if (m_embedded_palette_tex == 0)
    {
        glGenTextures(1, &m_embedded_palette_tex);
        glBindTexture(GL_TEXTURE_2D, m_embedded_palette_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, m_embedded_palette_tex);
    }
    if (m_rt_cmd.embedded_palette.size() >= 256u * 4u)
    {
        // GL_BGRA upload into an RGBA8 texture: the driver does the channel
        // swap, so the AVFrame's native BGRA layout needs no CPU-side
        // reordering (same trick develop's own equivalent upload uses).
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_BGRA, GL_UNSIGNED_BYTE,
                        m_rt_cmd.embedded_palette.data());
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLImagePresentPass::draw_quad(GLuint program, GLuint image_tex, GLuint palette_tex,
                                   int dst_x, int dst_y, int dst_w, int dst_h,
                                   int screen_w, int screen_h)
{
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
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glUseProgram(program);
    glUniform2f(glGetUniformLocation(program, "u_screen_size"), (float)screen_w, (float)screen_h);
    glBindVertexArray(m_quad_vao);

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
    if (!IsActiveRT() || !m_shader || m_palette_tex == 0 || screen_w <= 0 || screen_h <= 0)
        return;

    glViewport(0, 0, screen_w, screen_h);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);

    // Base layer: the zoom transition and a plain opaque present are both
    // "this frame's entire background" content and never active together
    // (frontzoom_to_point() is the landview screen's own background draw;
    // nothing else calls Submit() with PRESENT_KIND_OPAQUE while it's
    // running) -- zoom takes priority if somehow both fired the same frame.
    if (m_rt_zoom_cmd.active && m_zoom_shader)
    {
        upload_zoom_texture();
        if (m_zoom_tex != 0)
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
            glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

            glUseProgram(m_zoom_shader);
            glUniform2f(m_loc_zoom_screen_size, (float)screen_w, (float)screen_h);
            glUniform2f(m_loc_zoom_center_map, m_rt_zoom_cmd.center_map_x, m_rt_zoom_cmd.center_map_y);
            glUniform2f(m_loc_zoom_screen_center, m_rt_zoom_cmd.screen_cx, m_rt_zoom_cmd.screen_cy);
            glUniform1f(m_loc_zoom_scale, m_rt_zoom_cmd.scale);
            glUniform2f(m_loc_zoom_src_size, (float)m_rt_zoom_cmd.src_w, (float)m_rt_zoom_cmd.src_h);
            glBindVertexArray(m_quad_vao);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_zoom_tex);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_palette_tex);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            glBindVertexArray(0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    else if (m_rt_cmd.active)
    {
        upload_texture();
        if (m_tex != 0)
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            GLuint palette_tex = m_palette_tex;
            if (m_rt_cmd.palette == PRESENT_PALETTE_EMBEDDED)
            {
                upload_embedded_palette();
                if (m_embedded_palette_tex != 0)
                    palette_tex = m_embedded_palette_tex;
            }
            draw_quad(m_shader, m_tex, palette_tex,
                     m_rt_cmd.dst_x, m_rt_cmd.dst_y, m_rt_cmd.dst_w, m_rt_cmd.dst_h,
                     screen_w, screen_h);
        }
    }

    // Transparent overlay (window-frame): drawn over whatever's already on
    // screen this frame -- the base/zoom layer above, or (if neither was
    // active) whatever the rest of the frame already put there. No clear.
    if (m_rt_overlay_cmd.active && m_transparent_shader)
    {
        upload_overlay_texture();
        if (m_overlay_tex != 0)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            draw_quad(m_transparent_shader, m_overlay_tex, m_palette_tex,
                     m_rt_overlay_cmd.dst_x, m_rt_overlay_cmd.dst_y,
                     m_rt_overlay_cmd.dst_w, m_rt_overlay_cmd.dst_h,
                     screen_w, screen_h);
            glDisable(GL_BLEND);
        }
    }

    glUseProgram(0);
    glDepthMask(GL_TRUE);
}
