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
    /** Shared textures the blend reads (not owned): the frame palette, the
     *  fade table and the palette index lookup. */
    void SetPaletteTexture(GpuResourceHandle tex) { m_palette_tex_handle = tex; }
    void SetFadeTableTexture(GpuResourceHandle tex) { m_fade_table_tex_handle = tex; }
    void SetPaletteIndexTexture(GpuResourceHandle tex) { m_palette_index_tex_handle = tex; }

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

    /** @p ghost_table is the 256x256 map-fade ghost table; it's copied on the
     *  transition's first frame. */
    void SubmitStep(int tick_step, float display_step, bool fading_in, const unsigned char* ghost_table);

    void FlipBuffers();


    bool IsActiveRT() const { return m_rt_cmd.active; }
    bool IsCapturePendingRT() const { return m_rt_cmd.active && m_rt_cmd.capture_pending; }

    /** Copy the screen target into the world side of the transition. Call
     *  once the world is drawn and before anything of the parchment is. */
    void CaptureWorldFrame(int w, int h);

    /** Copy the screen target into the parchment side of the transition.
     *  Call once the whole parchment view is drawn. */
    void CaptureParchmentFrame(int w, int h);

    /** Draw the full-screen wipe composite (MAPFADE_FRAGMENT_SHADER,
     *  sampling the parchment and world snapshots) over the screen target. */
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
    GpuResourceHandle m_ghost_tex_handle = kInvalidGpuResource;   // owned, 256x256 R8
    GpuResourceHandle m_palette_tex_handle = kInvalidGpuResource;
    GpuResourceHandle m_fade_table_tex_handle = kInvalidGpuResource;
    GpuResourceHandle m_palette_index_tex_handle = kInvalidGpuResource;
    int    m_capture_gt_w = 0, m_capture_gt_h = 0;
};

/******************************************************************************/
