/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLImagePresentPass.h
 *     GPU-native present of a raw indexed8 image (FMV frame, splash
 *     bitmap) at an arbitrary destination rect -- the GL side of
 *     RendererOpenGL::PresentImage()/RendererPresentImageDesc 
 *     ToDo : replace with Rendergraph impl?
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/opengl/GLFunctions.h"
#include "kfx/renderer/GpuResourceHandle.h"
#include <vector>

class GLResourceMapper;

/******************************************************************************/

struct IRImagePresentCmd {
    bool active = false;
    int dst_x = 0, dst_y = 0, dst_w = 0, dst_h = 0;
    int src_w = 0, src_h = 0;
    std::vector<unsigned char> pixels; // tightly packed, src_w * src_h, indexed8

    int palette = 0;
    std::vector<unsigned char> embedded_palette; // 256*4 BGRA, PRESENT_PALETTE_EMBEDDED only

    // Overlay slot only: per-pixel opacity (255=opaque/0=transparent),
    std::vector<unsigned char> coverage;
};

struct IRLandviewZoomCmd {
    bool active = false;
    const unsigned char* src_buf = nullptr;
    int src_w = 0, src_h = 0;
    float center_map_x = 0.0f, center_map_y = 0.0f;
    float screen_cx = 0.0f, screen_cy = 0.0f;
    float scale = 1.0f;
};

class GLImagePresentPass {
public:
    GLImagePresentPass() = default;
    ~GLImagePresentPass();

    /** Compile the blit/transparent/zoom shaders + build the shared quad.
     *  Safe to call after construction; idempotent. */
    bool CompileShaders();

    /** Release all GL resources. Must be called on the render thread. */
    void Shutdown();

    /** True once CompileShaders() has succeeded -- gates
     *  RendererOpenGL::PresentImage()'s return value (false means the
     *  caller falls back to its own CPU blit). */
    bool IsReady() const { return m_shader_handle != kInvalidGpuResource; }

    /** Shared 256x1 RGBA8 palette texture (same one world/UI already bind)
     *  -- used for every present except PRESENT_PALETTE_EMBEDDED, which
     *  brings its own (see m_embedded_palette_tex_handle). This pass never
     *  owns a copy of the shared one: only the handle is stored, resolved
     *  fresh at each point of use rather than cached as a raw GL id, so it
     *  stays valid across a reload of the underlying texture. */
    void SetResourceMapper(GLResourceMapper* mapper) { m_resource_mapper = mapper; }
    void SetPaletteTexture(GpuResourceHandle tex) { m_palette_tex_handle = tex; }

    // -- Game thread --------------------------------------------------------

    /** Copies desc's source pixels now (see IRImagePresentCmd's comment).
     *  Routes to the base (PRESENT_KIND_OPAQUE) or overlay
     *  (PRESENT_KIND_TRANSPARENT) slot depending on desc->kind -- both can
     *  be active the same frame (e.g. landview background + window-frame),
     *  composited in Resolve() in that order. */
    void Submit(const struct RendererPresentImageDesc* desc);

    /** Submit this frame's landview zoom parameters. src_buf is cached by
     *  pointer identity (map_screen is a large, session-static buffer,
     *  reallocated only when a new landview loads) -- only re-copied into
     *  the upload staging buffer when the pointer changes from the last
     *  call, not every frame of the zoom animation. Mirrors
     *  RendererSubmitLandviewZoom()'s own contract (RendererManager.h). */
    void SubmitZoom(const unsigned char* src_buf, int src_w, int src_h,
                    float center_map_x, float center_map_y,
                    float screen_cx,    float screen_cy,
                    float scale);

    /** FlipBuffers()-style move into the render-thread-stable copies,
     *  called from RendererOpenGL::PresentFrame() alongside every other
     *  per-frame command. */
    void FlipBuffers();

    // -- Render thread --------------------------------------------------------

    bool IsActiveRT() const { return !m_rt_cmds.empty() || m_rt_overlay_cmd.active || m_rt_zoom_cmd.active; }

    /** Upload this frame's pixels (if any) and draw whatever's active this
     *  frame: the zoom background OR the base opaque image (mutually
     *  exclusive -- both are full-screen "this frame's entire background"
     *  content), then the transparent overlay on top if also active.
     *  Clears to black first only for the base/zoom layer -- matches the
     *  CPU raster's own letterbox behaviour (copy_to_screen()/
     *  copy_to_screen_scaled() memset the surrounding area to palette
     *  index 0) since nothing else draws on an FMV/splash/landview frame.
     *  No-op if IsActiveRT() is false. */
    void Resolve(int screen_w, int screen_h);

private:
    bool init_quad();
    void upload_overlay_texture();  // overlay image -> m_overlay_tex_handle
    void upload_overlay_coverage_texture(); // overlay coverage -> m_coverage_tex_handle
    void upload_opaque_texture(size_t idx); // m_rt_cmds[idx].pixels -> m_opaque_slots[idx].tex_handle
    void upload_zoom_texture();     // cached landview bitmap -> m_zoom_tex_handle (only on identity change)
    void upload_embedded_palette(const std::vector<unsigned char>& embedded_palette); // -> m_embedded_palette_tex_handle
    void draw_quad(GLuint program, GLuint image_tex, GLuint palette_tex,
                  int dst_x, int dst_y, int dst_w, int dst_h, int screen_w, int screen_h,
                  GLuint coverage_tex = 0);

    /** Game-thread-only. Creates `handle` on first call, or reloads it. */
    void EnsureImageTextureHandle(GpuResourceHandle& handle, int& gt_w, int& gt_h, int new_w, int new_h);

    std::vector<IRImagePresentCmd> m_cmds;      // GT: appended by Submit() (kind == OPAQUE)
    std::vector<IRImagePresentCmd> m_rt_cmds;   // RT: stable copy after FlipBuffers()
    IRImagePresentCmd m_overlay_cmd;     // GT: written by Submit() (kind == TRANSPARENT)
    IRImagePresentCmd m_rt_overlay_cmd;  // RT: stable copy after FlipBuffers()
    IRLandviewZoomCmd m_zoom_cmd;        // GT: written by SubmitZoom()
    IRLandviewZoomCmd m_rt_zoom_cmd;     // RT: stable copy after FlipBuffers()

    GLResourceMapper* m_resource_mapper = nullptr;

    GpuResourceHandle m_shader_handle             = kInvalidGpuResource; // RAWIMAGE_BLIT_FRAGMENT_SHADER (opaque)
    GpuResourceHandle m_transparent_shader_handle = kInvalidGpuResource; // RAWIMAGE_TRANSPARENT_FRAGMENT_SHADER (overlay)
    GpuResourceHandle m_coverage_shader_handle    = kInvalidGpuResource; // RAWIMAGE_TRANSPARENT_COVERAGE_FRAGMENT_SHADER (overlay w/ coverage)
    GpuResourceHandle m_zoom_shader_handle        = kInvalidGpuResource; // RAWIMAGE_ZOOM_FRAGMENT_SHADER
    GLint  m_loc_screen_size = -1;         // m_shader_handle / m_transparent_shader_handle
    GLint  m_loc_zoom_screen_size = -1;
    GLint  m_loc_zoom_center_map  = -1;
    GLint  m_loc_zoom_screen_center = -1;
    GLint  m_loc_zoom_scale = -1;
    GLint  m_loc_zoom_src_size = -1;
    GpuResourceHandle m_quad_geom_handle = kInvalidGpuResource;

    struct OpaqueSlot {
        GpuResourceHandle tex_handle = kInvalidGpuResource;
        int tex_gt_w = 0, tex_gt_h = 0;
    };
    std::vector<OpaqueSlot> m_opaque_slots;

    GpuResourceHandle m_overlay_tex_handle = kInvalidGpuResource;
    int    m_overlay_tex_gt_w = 0, m_overlay_tex_gt_h = 0;
    GpuResourceHandle m_coverage_tex_handle = kInvalidGpuResource;
    int    m_coverage_tex_gt_w = 0, m_coverage_tex_gt_h = 0;
    GpuResourceHandle m_zoom_tex_handle = kInvalidGpuResource;
    int    m_zoom_tex_gt_w = 0, m_zoom_tex_gt_h = 0;
    const unsigned char* m_zoom_tex_identity = nullptr;
    int    m_zoom_tex_rt_w = 0, m_zoom_tex_rt_h = 0;

    GpuResourceHandle m_palette_tex_handle = kInvalidGpuResource; // not owned (shared game palette)
    GpuResourceHandle m_embedded_palette_tex_handle = kInvalidGpuResource; // owned -- FMV's own per-frame palette

    /** Resolves the shared palette handle to a raw GLuint. Zero if the
     *  handle is unset or resolution fails. */
    GLuint ResolvePaletteTexId() const;
};

/******************************************************************************/
