/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLMapFadePass.cpp
 *     See GLMapFadePass.h for the design rationale.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/opengl/GLMapFadePass.h"
#include "kfx/renderer/opengl/GLShaders.h"
#include "kfx/renderer/RendererThread.h"   // ASSERT_GAME_THREAD/ASSERT_RENDER_THREAD
#include "bflib_basics.h"                  // ERRORLOG
#include "post_inc.h"

/******************************************************************************/

// File-local copy of GLWorldViewRenderer.cpp's own compile_shader_src() --
// that one has internal linkage (static), can't be shared across
// translation units without a new shared header, and this is the only
// other file that needs it so far.
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
        ERRORLOG("GLMapFadePass: shader '%s' compile error: %s", debug_name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLMapFadePass::~GLMapFadePass()
{
    // Shutdown() must be called explicitly on the render thread before
    // destruction (mirrors GLWorldViewRenderer/GLUIRenderer -- by the time
    // this destructs, on the game thread after the render thread has
    // already been joined and the context destroyed, GL calls here would
    // run with no current context on any thread).
}

bool GLMapFadePass::CompileShaders()
{
    GLuint vs = compile_shader_src(GL_VERTEX_SHADER, LENS_COMPOSITE_VERTEX_SHADER, "mapfade_composite_vert.glsl");
    if (!vs)
        return false;

    if (!init_quad())
    {
        glDeleteShader(vs);
        return false;
    }

    GLuint fs = compile_shader_src(GL_FRAGMENT_SHADER, MAPFADE_FRAGMENT_SHADER, "mapfade_frag.glsl");
    if (!fs)
    {
        glDeleteShader(vs);
        return false;
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[512];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        ERRORLOG("GLMapFadePass: shader link error: %s", log);
        glDeleteProgram(prog);
        return false;
    }

    m_shader = prog;
    glUseProgram(prog);
    glUniform1i(glGetUniformLocation(prog, "u_parchment"), 0);
    glUniform1i(glGetUniformLocation(prog, "u_world"), 1);
    m_loc_step = glGetUniformLocation(prog, "u_step");
    glUseProgram(0);
    return true;
}

bool GLMapFadePass::init_quad()
{
    // Same static NDC unit quad shape as P5.8a's lens composite pass --
    // glViewport is set to the full screen before ResolveComposite()'s
    // draw call, so a_pos's -1..1 range covers the whole backbuffer.
    static const float k_quad[] = {
        //  x,     y,    u,    v
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
    };
    glGenVertexArrays(1, &m_quad_vao);
    glGenBuffers(1, &m_quad_vbo);
    glBindVertexArray(m_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(k_quad), k_quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    return true;
}

void GLMapFadePass::Shutdown()
{
    if (m_quad_vbo) { glDeleteBuffers(1, &m_quad_vbo); m_quad_vbo = 0; }
    if (m_quad_vao) { glDeleteVertexArrays(1, &m_quad_vao); m_quad_vao = 0; }
    if (m_shader)   { glDeleteProgram(m_shader); m_shader = 0; }
    if (m_tex_parchment) { glDeleteTextures(1, &m_tex_parchment); m_tex_parchment = 0; }
    if (m_tex_world)     { glDeleteTextures(1, &m_tex_world);     m_tex_world = 0; }
    if (m_parchment_fbo) { glDeleteFramebuffers(1, &m_parchment_fbo); m_parchment_fbo = 0; }
    m_tex_w = m_tex_h = 0;
    m_cmd = IRMapFadeCmd{};
    m_rt_cmd = IRMapFadeCmd{};
    m_was_active_gt = false;
}

void GLMapFadePass::SubmitStep(int tick_step, float display_step, bool fading_in)
{
    ASSERT_GAME_THREAD();
    const bool is_start = (fading_in && tick_step == 0) || (!fading_in && tick_step == 32);
    m_cmd.active = true;
    m_cmd.step = display_step;
    m_cmd.capture_pending = is_start && !m_was_active_gt;
}

void GLMapFadePass::FlipBuffers()
{
    ASSERT_GAME_THREAD();
    m_was_active_gt = m_cmd.active;
    m_rt_cmd = std::move(m_cmd);
    // IRMapFadeCmd::active is a scalar -- std::move leaves it unchanged in
    // the moved-from object (same hazard IRWorldLensCmd's own comment
    // documents). Reset explicitly so a frame with no SubmitStep() call
    // (transition ended) starts clean rather than replaying stale state.
    m_cmd = IRMapFadeCmd{};
}

void GLMapFadePass::ensure_textures(int w, int h)
{
    if (w <= 0 || h <= 0)
        return;
    if (m_tex_parchment != 0 && m_tex_world != 0 && m_tex_w == w && m_tex_h == h)
        return;

    if (m_tex_parchment == 0) glGenTextures(1, &m_tex_parchment);
    if (m_tex_world == 0)     glGenTextures(1, &m_tex_world);

    GLuint textures[2] = { m_tex_parchment, m_tex_world };
    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    m_tex_w = w;
    m_tex_h = h;
}

void GLMapFadePass::BeginParchmentCapture(int w, int h)
{
    ASSERT_RENDER_THREAD();
    ensure_textures(w, h);
    if (m_tex_parchment == 0)
        return;

    if (m_parchment_fbo == 0)
        glGenFramebuffers(1, &m_parchment_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_parchment_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex_parchment, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        ERRORLOG("GLMapFadePass: parchment FBO incomplete (0x%x) -- transition will show a blank parchment side this run", (unsigned)status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glViewport(0, 0, w, h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLMapFadePass::EndParchmentCapture()
{
    ASSERT_RENDER_THREAD();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLMapFadePass::CaptureWorldFrame(int w, int h)
{
    ASSERT_RENDER_THREAD();
    ensure_textures(w, h);
    if (m_tex_world == 0 || w <= 0 || h <= 0)
        return;

    // Blit the already-rendered default framebuffer (world + UI, whatever
    // this frame's normal draw calls already produced) into m_tex_world --
    // no new draw submission, matching origin/develop's CaptureWorldFrame()
    // for the same reason: re-running the 3D/UI draw on the render thread
    // would race the game thread already building the next frame.
    GLuint tmp_fbo = 0;
    glGenFramebuffers(1, &tmp_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tmp_fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex_world, 0);

    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
    else
    {
        ERRORLOG("GLMapFadePass: world-capture FBO incomplete -- transition will show a blank world side this run");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &tmp_fbo);
}

void GLMapFadePass::ResolveComposite(int screen_w, int screen_h)
{
    ASSERT_RENDER_THREAD();
    if (!m_shader || m_tex_parchment == 0 || m_tex_world == 0)
        return;

    glViewport(0, 0, screen_w, screen_h);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glUseProgram(m_shader);
    glBindVertexArray(m_quad_vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex_parchment);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_tex_world);

    glUniform1f(m_loc_step, m_rt_cmd.step);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDepthMask(GL_TRUE);
}
