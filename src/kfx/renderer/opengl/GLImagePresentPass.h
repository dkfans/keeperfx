/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLImagePresentPass.h
 *     GPU-native present of a raw indexed8 image (FMV frame, splash
 *     bitmap) at an arbitrary destination rect -- the GL side of
 *     RendererOpenGL::PresentImage()/RendererPresentImageDesc (see the
 *     BeginFrame/EndFrame/PresentImage plan doc). Same shape as
 *     GLMapFadePass (own shader, own texture, own small double-buffered
 *     command) -- new code, not a port of origin/develop's
 *     ImagePresentBuffers/ExecuteImagePresentsFromIR, which rides on a
 *     RenderGraph this branch doesn't have.
 *
 *     Beats 9/10 extended this with:
 *      - an embedded per-present palette (FMV, PRESENT_PALETTE_EMBEDDED) --
 *        own small palette texture, independent of the shared game-palette
 *        one so an FMV frame's colours never leak into anything else;
 *      - a second, non-clearing "overlay" slot (PRESENT_KIND_TRANSPARENT,
 *        index 0 = see-through) for content that draws over the base image
 *        instead of replacing it -- the landview window-frame
 *        (compressed_window_draw(), front_landview.c) is the only current
 *        user;
 *      - a dedicated landview zoom-in/out path (SubmitZoom()), with its own
 *        cached source texture (map_screen is a large, session-static
 *        buffer -- re-copying/re-uploading it every frame of the zoom
 *        animation would be wasteful) and its own sampling shader.
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/opengl/GLFunctions.h"
#include "kfx/renderer/GpuResourceHandle.h"
#include <vector>

class GLResourceMapper;

/******************************************************************************/

// One base image-present submission (PRESENT_KIND_OPAQUE, the FMV/splash/
// zoom-background case). GT builds it in Submit(), copying the source
// pixels immediately (the caller's buffer -- an AVFrame's decode buffer, a
// bitmap load buffer -- isn't guaranteed to outlive this call, same reason
// every other GT->RT command in this codebase copies rather than aliases).
// FlipBuffers() moves it to the RT-stable copy.
struct IRImagePresentCmd {
    bool active = false;
    int dst_x = 0, dst_y = 0, dst_w = 0, dst_h = 0;
    int src_w = 0, src_h = 0;
    std::vector<unsigned char> pixels; // tightly packed, src_w * src_h, indexed8

    int palette = 0; // PRESENT_PALETTE_* (RendererManager.h)
    std::vector<unsigned char> embedded_palette; // 256*4 BGRA, PRESENT_PALETTE_EMBEDDED only
};

// One landview zoom submission (frontzoom_to_point(), front_landview.c).
// Unlike IRImagePresentCmd, the source pixels (map_screen) are cached by
// pointer identity rather than copied every call -- see SubmitZoom()'s
// comment.
struct IRLandviewZoomCmd {
    bool active = false;
    // map_screen itself -- session-static (allocated at landview load, freed
    // only at unload), so unlike every other buffer this codebase's GT->RT
    // commands copy defensively, this one is safe to alias by pointer across
    // the handoff. Used only as an identity check (upload_zoom_texture()
    // re-copies/re-uploads only when this differs from the last frame's) and
    // as the actual source when it does.
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

    bool IsActiveRT() const { return m_rt_cmd.active || m_rt_overlay_cmd.active || m_rt_zoom_cmd.active; }

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
    void upload_texture();          // base image -> m_tex_handle
    void upload_overlay_texture();  // overlay image -> m_overlay_tex_handle
    void upload_zoom_texture();     // cached landview bitmap -> m_zoom_tex_handle (only on identity change)
    void upload_embedded_palette(); // m_rt_cmd.embedded_palette -> m_embedded_palette_tex_handle
    void draw_quad(GLuint program, GLuint image_tex, GLuint palette_tex,
                  int dst_x, int dst_y, int dst_w, int dst_h, int screen_w, int screen_h);

    /** Game-thread-only. Creates `handle` on first call, or reloads it
     *  (RequestReloadTexture) if `gt_w`/`gt_h` -- the dimensions this same
     *  call last committed the handle at -- differ from `new_w`/`new_h`.
     *  The resize decision has to happen here, not in Resolve() (render
     *  thread): RequestReloadTexture is game-thread-only. Content itself
     *  is uploaded separately, later, on the render thread. */
    void EnsureImageTextureHandle(GpuResourceHandle& handle, int& gt_w, int& gt_h, int new_w, int new_h);

    IRImagePresentCmd m_cmd;             // GT: written by Submit() (kind == OPAQUE)
    IRImagePresentCmd m_rt_cmd;          // RT: stable copy after FlipBuffers()
    IRImagePresentCmd m_overlay_cmd;     // GT: written by Submit() (kind == TRANSPARENT)
    IRImagePresentCmd m_rt_overlay_cmd;  // RT: stable copy after FlipBuffers()
    IRLandviewZoomCmd m_zoom_cmd;        // GT: written by SubmitZoom()
    IRLandviewZoomCmd m_rt_zoom_cmd;     // RT: stable copy after FlipBuffers()

    GLResourceMapper* m_resource_mapper = nullptr;

    GpuResourceHandle m_shader_handle             = kInvalidGpuResource; // RAWIMAGE_BLIT_FRAGMENT_SHADER (opaque)
    GpuResourceHandle m_transparent_shader_handle = kInvalidGpuResource; // RAWIMAGE_TRANSPARENT_FRAGMENT_SHADER (overlay)
    GpuResourceHandle m_zoom_shader_handle        = kInvalidGpuResource; // RAWIMAGE_ZOOM_FRAGMENT_SHADER
    GLint  m_loc_screen_size = -1;         // m_shader_handle / m_transparent_shader_handle
    GLint  m_loc_zoom_screen_size = -1;
    GLint  m_loc_zoom_center_map  = -1;
    GLint  m_loc_zoom_screen_center = -1;
    GLint  m_loc_zoom_scale = -1;
    GLint  m_loc_zoom_src_size = -1;
    GpuResourceHandle m_quad_geom_handle = kInvalidGpuResource;

    // Base/overlay/zoom textures: game-thread-owned handle + the dimensions
    // that handle was last created/reloaded at (gt_w/gt_h; compared against
    // each new Submit()/SubmitZoom() to decide create vs. reload vs. no-op).
    GpuResourceHandle m_tex_handle = kInvalidGpuResource;
    int    m_tex_gt_w = 0, m_tex_gt_h = 0;
    GpuResourceHandle m_overlay_tex_handle = kInvalidGpuResource;
    int    m_overlay_tex_gt_w = 0, m_overlay_tex_gt_h = 0;
    GpuResourceHandle m_zoom_tex_handle = kInvalidGpuResource;
    int    m_zoom_tex_gt_w = 0, m_zoom_tex_gt_h = 0;
    // Render-thread-only: last-uploaded src_buf pointer + dimensions, purely
    // a content-reupload-avoidance cache (distinct from gt_w/gt_h above,
    // which gate handle creation/reload).
    const unsigned char* m_zoom_tex_identity = nullptr;
    int    m_zoom_tex_rt_w = 0, m_zoom_tex_rt_h = 0;

    GpuResourceHandle m_palette_tex_handle = kInvalidGpuResource; // not owned (shared game palette)
    // Fixed 256x1 RGBA8 -- a content-update case like the mapper spec's
    // mist-texture guidance (Part 6.7), never needs RequestReloadTexture.
    GpuResourceHandle m_embedded_palette_tex_handle = kInvalidGpuResource; // owned -- FMV's own per-frame palette

    /** Resolves the shared palette handle to a raw GLuint. Zero if the
     *  handle is unset or resolution fails. */
    GLuint ResolvePaletteTexId() const;
};

/******************************************************************************/
