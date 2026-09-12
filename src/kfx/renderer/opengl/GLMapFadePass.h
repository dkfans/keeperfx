/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLMapFadePass.h
 *     Parchment-transition ("map-fade" in code identifiers)
 * @par Purpose:
 *     Captures the overhead parchment view and the live 3D world view into
 *     two native-resolution textures once per transition, then composites
 *     them every frame of the ~8-9 tick transition via a GLSL port of the
 *     CPU map_fade() warp+blend (engine_redraw.c).
 * 
 *   @par Design Rationale: possibly replace / use rendergraph instead of this ad-hoc pass.
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/opengl/GLFunctions.h"
#include "kfx/renderer/ir/WorldCommands.h"  // IRMapFadeCmd
#include "kfx/renderer/GpuResourceHandle.h"

class GLResourceMapper;

/******************************************************************************/

class GLMapFadePass {
public:
    GLMapFadePass() = default;
    ~GLMapFadePass();

    /** Must be called before CompileShaders(). Not owned; must outlive this. */
    void SetResourceMapper(GLResourceMapper* mapper) { m_resource_mapper = mapper; }

    /** Compile the composite shader + build the shared unit quad. Safe to
     *  call after construction; idempotent. */
    bool CompileShaders();

    /** Release all GL resources. Must be called on the render thread. */
    void Shutdown();

    /** True once CompileShaders() has succeeded -- gates
     *  MapFadeSupportsNativeResolution(), matching develop's
     *  GLMapFadePass::SupportsNativeResolution() (which returns its own
     *  m_initialized) exactly. */
    bool IsReady() const { return m_shader_handle != kInvalidGpuResource; }

    void SubmitStep(int tick_step, float display_step, bool fading_in);

    void FlipBuffers();


    bool IsActiveRT() const { return m_rt_cmd.active; }
    bool IsCapturePendingRT() const { return m_rt_cmd.active && m_rt_cmd.capture_pending; }

    /** Bind (creating/resizing if needed) the parchment capture FBO and
     *  clear it. Caller then issues whatever draws should land in it
     *  (GLUIRenderer::DrawFromIR() against the diverted parchment IR
     *  buffers) before calling EndParchmentCapture(). Only valid to call
     *  when IsCapturePendingRT() is true. */
    void BeginParchmentCapture(int w, int h);

    void EndParchmentCapture();

    void CaptureWorldFrame(int w, int h);

    /** Draw the full-screen wipe composite (MAPFADE_FRAGMENT_SHADER,
     *  sampling m_tex_parchment/m_tex_world) over whatever is currently in
     *  the default framebuffer. */
    void ResolveComposite(int screen_w, int screen_h);

private:
    bool init_quad();

    void EnsureCaptureResources(int w, int h);

    IRMapFadeCmd m_cmd;         // GT: written by SubmitStep(); reset after FlipBuffers()
    IRMapFadeCmd m_rt_cmd;      // RT: stable copy after FlipBuffers()
    bool m_was_active_gt = false;  // GT: was a transition active as of the end of the previous frame

    GLResourceMapper* m_resource_mapper = nullptr;

    GpuResourceHandle m_shader_handle = kInvalidGpuResource;
    GLint  m_loc_step = -1;
    GpuResourceHandle m_quad_geom_handle = kInvalidGpuResource;

    GpuResourceHandle m_parchment_rt_handle = kInvalidGpuResource;
    GpuResourceHandle m_tex_world_handle = kInvalidGpuResource;
    int    m_capture_gt_w = 0, m_capture_gt_h = 0;
};

/******************************************************************************/
