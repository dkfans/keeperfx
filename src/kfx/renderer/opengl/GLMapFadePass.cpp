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
#include "kfx/renderer/opengl/GLResourceMapper.h"
#include "kfx/renderer/RendererThread.h"   // ASSERT_GAME_THREAD/ASSERT_RENDER_THREAD
#include "kfx/renderer/RendererManager.h"  // RendererPhysicalWidth
#include "bflib_basics.h"                  // ERRORLOG
#include "bflib_video.h"                   // lbDisplay
#include "post_inc.h"

/******************************************************************************/

GLMapFadePass::~GLMapFadePass() { }

bool GLMapFadePass::CompileShaders()
{
    if (m_resource_mapper == nullptr)
    {
        ERRORLOG("GLMapFadePass::CompileShaders -- no resource mapper set");
        return false;
    }

    if (!init_quad())
        return false;

    GpuProgramDesc desc;
    desc.vertex_src = LENS_COMPOSITE_VERTEX_SHADER;
    desc.fragment_src = MAPFADE_FRAGMENT_SHADER;
    desc.debug_name = "mapfade_composite";
    m_shader_handle = m_resource_mapper->RequestCreateProgram(desc);

    const GLProgram* prog = m_resource_mapper->ResolveProgram(m_shader_handle);
    if (!prog)
        return false;

    glUseProgram(prog->id);
    glUniform1i(glGetUniformLocation(prog->id, "u_parchment"), 0);
    glUniform1i(glGetUniformLocation(prog->id, "u_world"), 1);
    m_loc_step = glGetUniformLocation(prog->id, "u_step");
    glUseProgram(0);
    return true;
}

bool GLMapFadePass::init_quad()
{
    static const float k_quad[] = {
        //  x,     y,    u,    v
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
    };

    GpuGeometryBufferDesc desc;
    desc.vertex_stride = 4 * (uint32_t)sizeof(float);
    desc.attribs = {
        { 0, 2, GpuVertexAttribType::Float, 0 },
        { 1, 2, GpuVertexAttribType::Float, 2 * (uint32_t)sizeof(float) },
    };
    desc.dynamic = false; // static unit quad, content never changes after this
    desc.initial_vertex_capacity = sizeof(k_quad);
    desc.debug_name = "mapfade_quad";
    m_quad_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(desc);

    const GLGeometryBuffer* geom = m_resource_mapper->ResolveGeometryBuffer(m_quad_geom_handle);
    if (!geom) return false;

    glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(k_quad), k_quad);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

void GLMapFadePass::Shutdown()
{
    m_capture_gt_w = m_capture_gt_h = 0;
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

    if (m_cmd.capture_pending)
        EnsureCaptureResources((int)RendererPhysicalWidth(), (int)lbDisplay.PhysicalScreenHeight);
}

void GLMapFadePass::FlipBuffers()
{
    ASSERT_GAME_THREAD();
    m_was_active_gt = m_cmd.active;
    m_rt_cmd = std::move(m_cmd);
    m_cmd = IRMapFadeCmd{};
}

void GLMapFadePass::EnsureCaptureResources(int w, int h)
{
    if (!m_resource_mapper || w <= 0 || h <= 0)
        return;
    if (m_parchment_rt_handle != kInvalidGpuResource && m_tex_world_handle != kInvalidGpuResource
        && m_capture_gt_w == w && m_capture_gt_h == h)
        return;

    GpuRenderTargetDesc rt_desc;
    rt_desc.width = w;
    rt_desc.height = h;
    rt_desc.attachments = { { GpuTextureFormat::RGBA8, false } };
    rt_desc.debug_name = "mapfade_parchment";

    GpuTextureDesc world_desc;
    world_desc.width = w;
    world_desc.height = h;
    world_desc.format = GpuTextureFormat::RGBA8;
    world_desc.min_filter = GpuTextureFilter::Linear;
    world_desc.mag_filter = GpuTextureFilter::Linear;
    world_desc.wrap = GpuTextureWrap::Clamp;
    world_desc.debug_name = "mapfade_world";

    if (m_parchment_rt_handle == kInvalidGpuResource)
        m_parchment_rt_handle = m_resource_mapper->RequestCreateRenderTarget(rt_desc);
    else
        m_parchment_rt_handle = m_resource_mapper->RequestReloadRenderTarget(m_parchment_rt_handle, rt_desc);

    if (m_tex_world_handle == kInvalidGpuResource)
        m_tex_world_handle = m_resource_mapper->RequestCreateTexture(world_desc);
    else
        m_tex_world_handle = m_resource_mapper->RequestReloadTexture(m_tex_world_handle, world_desc);

    m_capture_gt_w = w;
    m_capture_gt_h = h;
}

void GLMapFadePass::BeginParchmentCapture(int w, int h)
{
    ASSERT_RENDER_THREAD();
    if (!m_resource_mapper) return;
    const GLRenderTarget* rt = m_resource_mapper->ResolveRenderTarget(m_parchment_rt_handle);
    if (!rt) return;

    glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo);

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
    if (!m_resource_mapper) return;
    const GLTexture* world_tex = m_resource_mapper->ResolveTexture(m_tex_world_handle);
    if (!world_tex || w <= 0 || h <= 0)
        return;
        
    GLuint tmp_fbo = 0;
    glGenFramebuffers(1, &tmp_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tmp_fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, world_tex->id, 0);

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
    if (!m_resource_mapper) return;

    const GLProgram* prog = m_resource_mapper->ResolveProgram(m_shader_handle);
    const GLRenderTarget* parchment_rt = m_resource_mapper->ResolveRenderTarget(m_parchment_rt_handle);
    const GLTexture* world_tex = m_resource_mapper->ResolveTexture(m_tex_world_handle);
    const GLGeometryBuffer* geom = m_resource_mapper->ResolveGeometryBuffer(m_quad_geom_handle);
    if (!prog || !parchment_rt || parchment_rt->color_attachments.empty() || !world_tex || !geom)
        return;

    glViewport(0, 0, screen_w, screen_h);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glUseProgram(prog->id);
    glBindVertexArray(geom->vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, parchment_rt->color_attachments[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, world_tex->id);

    glUniform1f(m_loc_step, m_rt_cmd.step);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDepthMask(GL_TRUE);
}
