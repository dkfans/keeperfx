/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLMapFadePass.h
 *     Parchment-transition ("map-fade" in code identifiers elsewhere --
 *     see the P5.8b plan for why that name is avoided in prose) GL
 *     composite pass.
 * @par Purpose:
 *     Captures the overhead parchment view and the live 3D world view into
 *     two native-resolution textures once per transition, then composites
 *     them every frame of the ~8-9 tick transition via a GLSL port of the
 *     CPU map_fade() warp+blend (engine_redraw.c). New code, not a port of
 *     origin/develop's GLMapFadePass -- that class rides on a RenderGraph/
 *     ImagePresentBuffers/m_rt_overhead_map_cmds architecture this branch
 *     doesn't have (see P5.8a's own precedent for the same reasoning, and
 *     the P5.8b plan's "Parchment-view capture" section for the research
 *     that confirmed this branch's own architecture needed a different
 *     mechanism entirely). Owned by RendererOpenGL::Impl alongside
 *     world/ui/cursor/atlas -- not part of GLWorldViewRenderer, since this
 *     spans both the world and UI draw pipelines.
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/opengl/GLFunctions.h"
#include "kfx/renderer/ir/WorldCommands.h"  // IRMapFadeCmd

/******************************************************************************/

class GLMapFadePass {
public:
    GLMapFadePass() = default;
    ~GLMapFadePass();

    /** Compile the composite shader + build the shared unit quad. Safe to
     *  call after construction; idempotent. */
    bool CompileShaders();

    /** Release all GL resources. Must be called on the render thread. */
    void Shutdown();

    /** True once CompileShaders() has succeeded -- gates
     *  MapFadeSupportsNativeResolution(), matching develop's
     *  GLMapFadePass::SupportsNativeResolution() (which returns its own
     *  m_initialized) exactly. */
    bool IsReady() const { return m_shader != 0; }

    // ── Game thread ─────────────────────────────────────────────────────────

    /** Called once per frame a parchment transition is in progress, via the
     *  RendererSubmitMapFadeStep() bridge (redraw_display(), engine_redraw.c)
     *  -- never called on a frame with no active transition, so a frame
     *  with no call naturally means "not active" once FlipBuffers() resets
     *  the write-side command below. @p tick_step matches software's own
     *  palette_fade_step_map exactly (0..32, stepping by 4) -- used only to
     *  detect the transition's first frame ((fading_in && tick_step==0) ||
     *  (!fading_in && tick_step==32)), never for the actual composite.
     *  @p display_step is the value actually drawn with -- when Ft_DeltaTime
     *  is on, engine_redraw.c interpolates it between ticks via
     *  game.process_turn_time for a smooth, display-rate fade instead of the
     *  20fps tick-quantised one; otherwise it equals tick_step. @p fading_in
     *  distinguishes the two transition directions, needed for the
     *  first-frame check above. */
    void SubmitStep(int tick_step, float display_step, bool fading_in);

    /** FlipBuffers()-style move into the render-thread-stable copy, called
     *  from RendererOpenGL::PresentFrame() alongside GLWorldViewRenderer's
     *  own. Must run while the render thread is idle, same discipline as
     *  every other double-buffered command in this codebase. */
    void FlipBuffers();

    // ── Render thread ───────────────────────────────────────────────────────

    bool IsActiveRT() const { return m_rt_cmd.active; }
    bool IsCapturePendingRT() const { return m_rt_cmd.active && m_rt_cmd.capture_pending; }

    /** Bind (creating/resizing if needed) the parchment capture FBO and
     *  clear it. Caller then issues whatever draws should land in it
     *  (GLUIRenderer::DrawFromIR() against the diverted parchment IR
     *  buffers) before calling EndParchmentCapture(). Only valid to call
     *  when IsCapturePendingRT() is true. */
    void BeginParchmentCapture(int w, int h);

    /** Unbind back to the default framebuffer. The parchment FBO's colour
     *  attachment is m_tex_parchment, ready for ResolveComposite() to
     *  sample -- no CPU readback. */
    void EndParchmentCapture();

    /** Blit the default framebuffer's current contents (whatever this
     *  frame already rendered -- world + UI, drawn normally just before
     *  this call) into m_tex_world. No new draw submission -- mirrors
     *  origin/develop's CaptureWorldFrame(), which does the same for the
     *  identical reason (re-running draw_view() on the render thread would
     *  race the game thread building the next frame). Only valid to call
     *  when IsCapturePendingRT() is true, after the frame's normal draws. */
    void CaptureWorldFrame(int w, int h);

    /** Draw the full-screen wipe composite (MAPFADE_FRAGMENT_SHADER,
     *  sampling m_tex_parchment/m_tex_world) over whatever is currently in
     *  the default framebuffer. Called every frame IsActiveRT() is true,
     *  not just the capture frame -- on non-capture frames nothing else
     *  drew this frame (redraw_display()'s PVM_ParchFadeIn/Out dispatch
     *  submits nothing else), so this is the only visible content; on the
     *  capture frame it deliberately replaces the raw pre-blend frame just
     *  drawn, matching software's own behaviour (the capture frame already
     *  shows the blended result, not the untouched 3D/parchment view). */
    void ResolveComposite(int screen_w, int screen_h);

private:
    bool init_quad();
    void ensure_textures(int w, int h);

    IRMapFadeCmd m_cmd;         // GT: written by SubmitStep(); reset after FlipBuffers()
    IRMapFadeCmd m_rt_cmd;      // RT: stable copy after FlipBuffers()
    bool m_was_active_gt = false;  // GT: was a transition active as of the end of the previous frame

    GLuint m_shader = 0;
    GLint  m_loc_step = -1;
    GLuint m_quad_vao = 0, m_quad_vbo = 0;

    GLuint m_tex_parchment = 0, m_tex_world = 0;
    int    m_tex_w = 0, m_tex_h = 0;

    GLuint m_parchment_fbo = 0;  // reused/resized; bound only during BeginParchmentCapture()/EndParchmentCapture()
};

/******************************************************************************/
