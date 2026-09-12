/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLWorldViewRenderer.h
 *     Desktop OpenGL world-geometry renderer. P5.7.2 ported tile +
 *     flat-colour polygon geometry (wired into RendererOpenGL in P5.7.2b).
 *     P5.7.3a added keeper-sprite (creature/object) rendering: instanced
 *     atlas + CLUT remap, core path only -- no depth-fail outline, no
 *     additive-glow shader variant, no PiP/cursor-sprite capture. P5.7.3b
 *     wires the CPU side up: DrawIsometricView()/DrawFrontView()'s own
 *     bucket walk now dispatches QK_JontySprite/QK_JontyISOSprite into the
 *     same draw_jonty_mapwho()/draw_fastview_mapwho() resolution code
 *     software uses -- see the class-level note in the .cpp for the full
 *     rationale (including why fill-time submission wasn't needed after
 *     all) and the two disclosed visual gaps (water clipping, true alpha).
 * @par Purpose:
 *     Implements IWorldViewRenderer for the desktop OpenGL path.
 *     DrawIsometricView()/DrawFrontView() walk the engine bucket list,
 *     convert each PolyPoint triangle to WorldVertex (float NDC + UV +
 *     shade) or FlatPolyVertex (float NDC + linear RGB), batch the geometry
 *     into one of two VBOs (tiles vs. flat-colour polys), and issue one
 *     glDrawArrays call per batch. Keeper sprites are submitted separately,
 *     via SubmitKeeperSprite() -- see its own doc comment.
 *
 *     The fragment shader (WORLD_FRAGMENT_SHADER in GLShaders.h) replaces
 *     the entire CPU inner loop for tiles: palette-index lookup +
 *     fade-table lighting = two texture samples.
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/opengl/GLFunctions.h"
#include "kfx/renderer/IWorldViewRenderer.h"
#include "kfx/renderer/WorldVertex.h"
#include "kfx/renderer/FlatPolyVertex.h"
#include "kfx/renderer/ir/WorldCommands.h"
#include "kfx/renderer/ir/IRCommandBuffer.h"
#include "kfx/renderer/GpuResourceHandle.h"

class GLResourceMapper;

#include <vector>
#include <unordered_map>
#include <array>

struct PolyPoint;
struct BucketKindTexturedQuad;
struct BucketKindCreatureShadow;
class ITileAtlas;

/******************************************************************************/

class GLWorldViewRenderer : public IWorldViewRenderer {
public:
    // Deferred draw command (built during DrawIsometricView/DrawFrontView,
    // executed by GPURenderNow).
    struct DrawCmd {
        enum Type {
            CMD_TILES,
            CMD_FLAT_POLYS,
            CMD_IR_KEEPER_SPRITES,   // range into m_rt_kspr_ir (P5.7.3a)
            CMD_PRELOAD_KSPR_ATLAS,  // one-shot: bulk decode+upload on render thread
            CMD_CLEAR_KSPR_ATLAS,    // one-shot: reset the atlas/CLUT caches (P5.7.6)
        } type;
        int vert_start = 0;       // CMD_TILES / CMD_FLAT_POLYS
        int vert_count = 0;       // CMD_TILES / CMD_FLAT_POLYS
        int sprite_ir_start = 0;  // CMD_IR_KEEPER_SPRITES
        int sprite_ir_count = 0;  // CMD_IR_KEEPER_SPRITES
    };

    /** Per-instance record for the instanced keeper-sprite pass. */
    struct KsprInstance {
        float    rect[4];   // dst x, y, w, h (screen px)
        float    uvext[2];  // src_w/decode_dim, src_h/decode_dim
        float    layer;     // atlas layer
        float    clut_v;    // CLUT row V coord (row 0 = identity)
        float    alpha;     // 1.0 / transpar4 / transpar8
        float    z_ndc;     // pre-computed NDC depth
        uint32_t flags;     // bit0 = flip_h, bit1 = additive glow
    };

    /** Per-instance record for the instanced depth-fail outline pass (Beat 4). */
    struct KsprOutlineInstance {
        float rect[4];
        float uvext[2];
        float layer;
        float z_ndc;        // sprite z + outline bias
        float flip;         // 0/1
        float color[4];     // owner colour + outline alpha
    };

    /** One creature-shadow quad captured at DrawIsometricView() time
     *  (P5.7.4). Geometry is already fully resolved by create_shadows()
     *  (engine_render.c, fill time) -- this just carries it across to the
     *  render thread, where the atlas layer for (draw_idx/data) is resolved
     *  via the existing resolve_atlas_layer() (shared with real sprite
     *  draws of the same frame -- same cache, same layer). x/y are screen
     *  px, u/v are content-pixel offsets within the sprite (0..src_w/src_h),
     *  normalized against k_kspr_decode_dim at draw time, matching
     *  KsprInstance::uvext's convention. */
    struct ShadowSubmission {
        float x[4], y[4], u[4], v[4];
        float z_ndc;
        float darken;         // approximate distance-based intensity, see .cpp
        int32_t draw_idx;     // atlas cache key -- shared with real sprite draws
        const unsigned char* data;  // raw RLE data, resolved at capture time
        int src_w, src_h;
    };

    // Trivial constructor -- like every other GL component in
    // RendererOpenGL::Impl (GLSpriteAtlas, GLUIRenderer, GLTileAtlas), this
    // must not touch GL: Impl is `new`'d on the game thread before the
    // render thread exists / any context is current. Injected resources are
    // supplied later via the setters below, from render_thread_init(),
    // mirroring GLUIRenderer::SetAtlas()/SetPaletteTexture()/
    // SetFadeTableTexture() exactly (P5.7.2b fix -- P5.7.2a's constructor
    // took these as params and called init_gl_resources() directly, which
    // only happened to compile because nothing ever constructed one).
    GLWorldViewRenderer() = default;
    ~GLWorldViewRenderer() override;

    /** Tile atlas providing GL texture handles. Owned externally
     *  (RendererOpenGL); must outlive this. */
    void SetAtlas(ITileAtlas* atlas) { m_atlas = atlas; }
    // Palette/fade textures: only the handle is stored, resolved fresh at
    // each point of use rather than cached as a raw GLuint, so it stays
    // valid across a reload of the underlying texture.
    void SetResourceMapper(GLResourceMapper* mapper) { m_resource_mapper = mapper; }
    /** GPU resource handle for the 256x256 fade/lighting LUT. */
    void SetFadeTexture(GpuResourceHandle tex) { m_fade_tex_handle = tex; }
    /** GPU resource handle for the 256x1 RGBA palette. */
    void SetPaletteTexture(GpuResourceHandle tex) { m_palette_tex_handle = tex; }

    // IWorldViewRenderer
    void BeginWorldPass(int w, int h, int vp_x, int vp_y) override;
    void DrawIsometricView() override;
    void DrawFrontView(struct Camera* cam) override;
    const char* GetName() const override { return "GLWorldViewRenderer"; }

    /** Compile all GLSL programs owned by this renderer. Idempotent (safe to
     *  call after construction). Not part of any interface -- this branch's
     *  RendererOpenGL calls Init()-style methods on its GL components
     *  directly (see GLSpriteAtlas::Init()/GLUIRenderer::Init()), not
     *  through a shared shader-compile interface; the P5.7.2b wiring calls
     *  this the same way, from render_thread_init(). */
    bool CompileShaders() { return init_gl_resources(); }

    /** Release all GL resources. Must be called on the render thread (the
     *  only thread where the context is current) before the context itself
     *  is destroyed -- mirrors GLUIRenderer::Shutdown(), called explicitly
     *  from RendererOpenGL::render_thread_cleanup() rather than relied on
     *  via the destructor (by the time Impl's members are destructed, on
     *  the game thread after the render thread has already been joined and
     *  the context destroyed, GL calls here would run with no current
     *  context on any thread). */
    void Shutdown() { free_gl_resources(); }

    /** Notify of the full OS-window dimensions (not the world viewport). */
    void SetScreenSize(int w, int h) override { m_full_screen_w = w; m_full_screen_h = h; }

    // Called by the eventual render-thread integration (P5.7.2b) to issue
    // the accumulated draw list after glClear().
    void GPURenderNow(const WorldCommandBuffers& cmds);

    /** Swap game-thread <-> render-thread command buffers. Must be called
     *  while the render thread is idle, same discipline as GLFrameData's
     *  double buffer (see RendererOpenGL::PresentFrame()). */
    void FlipBuffers();

    bool HasPendingCommands() const
    {
        return !m_draw_cmds.empty()
            || (m_world_write_cmds != nullptr && !m_world_write_cmds->flat_poly_verts.empty())
            || (m_vert_count > m_cmd_vert_start)
            || !m_kspr_ir.empty()
            || !m_shadow_cmds.empty();
    }

    // ── IR (Intermediate Representation) path ─────────────────────────────────

    /** Set the IR write target for this frame. Tile/flat-poly vertex data is
     *  written into @p cmds by append_triangle()/append_flatpoly_triangle();
     *  ordering is recorded separately in m_draw_cmds. Call with nullptr to
     *  close the write window. */
    void SetWorldCommandBuffers(WorldCommandBuffers* cmds) override { m_world_write_cmds = cmds; }

    /** Replay the captured tile/flat-poly geometry on the render thread. */
    void ExecuteFromIR(const WorldCommandBuffers& cmds) override { GPURenderNow(cmds); }

    /** Attempt to initialise GL resources outside of a world pass, e.g. from
     *  RendererOpenGL::BeginFrame(). No-op when already initialised.
     *  Returns true when initialisation is complete. */
    bool TryEarlyInit() { return init_gl_resources(); }

    // ── Keeper sprites (P5.7.3a) ───────────────────────────────────────────────

    /** Supply the active 256-colour VGA palette (768 bytes: R,G,B x 256).
     *  Pointer copy only, read at FlipBuffers() time into m_rt_palette --
     *  needed for the CLUT (palette-remap) system. Nothing calls this yet
     *  (no producer exists in this branch -- see the .cpp file header); the
     *  base no-op default already made this safe before this override
     *  existed, so m_palette_data simply stays null and m_rt_palette stays
     *  zeroed until a future beat wires a real caller. */
    void SetPaletteSource(const uint8_t* palette) override { m_palette_data = palette; }

    /** Clear the keeper-sprite atlas + CLUT caches (called between levels). */
    void ClearKeeperSpriteAtlas() override;

    /** Queue a bulk decode+upload of every currently-RAM-resident keeper
     *  sprite into the atlas, executed on the render thread. */
    void PreloadKeeperSpriteAtlas() override;

    // ── Possession lens (P5.8a) ─────────────────────────────────────────────

    /** Game thread: stash this frame's active lens state (built by
     *  LensManager::BuildActiveGPULensCmd() via the RendererManager.cpp
     *  bridge). Copied, not moved -- the bridge's local IRWorldLensCmd goes
     *  out of scope right after this call. */
    void SubmitPossessionLens(const IRWorldLensCmd& cmd) override { m_lens_cmd = cmd; }

    /** Render thread: bind the lens scene FBO (creating/resizing it to the
     *  current full-screen capture resolution if needed) and clear it, iff
     *  m_rt_lens_cmd has an active pixel effect. A palette-only lens (no
     *  pixel effect) does not redirect -- world renders straight to the
     *  backbuffer as normal; ApplyLensPaletteIfActive() handles that case.
     *  Returns true iff the capture was begun (caller must then call
     *  ResolveLensComposite() after the world pass). */
    bool BeginLensCapture();

    /** Render thread: unbind back to the default framebuffer and run the one
     *  shader variant matching m_rt_lens_cmd.type, compositing the captured
     *  scene texture into the real viewport sub-rect of the backbuffer.
     *  Only valid to call when BeginLensCapture() just returned true. */
    void ResolveLensComposite();

    /** Render thread: true iff m_rt_lens_cmd carries a palette override this
     *  frame (LCF_HasPalette) -- independent of whether a pixel effect is
     *  also active. */
    bool HasActiveLensPalette() const { return m_rt_lens_cmd.active && m_rt_lens_cmd.has_palette; }

    /** Render thread: the active lens's palette, expanded to RGBA8 (matching
     *  GLFrameData::palette_rgba's layout) into @p out_rgba (>= 1024 bytes).
     *  Only valid when HasActiveLensPalette() is true. */
    void GetActiveLensPaletteRGBA(unsigned char* out_rgba) const;

    /** Submit one keeper-sprite (creature/object) for GPU rendering.
     *  Game-thread only, pure CPU struct-building -- see IWorldViewRenderer's
     *  doc comment for the parameter shapes. Always returns 1 (claims the
     *  sprite) once GL resources are up. Called from draw_keepersprite()
     *  (engine_render.c, P5.7.3b) right before its own CPU blit -- the
     *  caller skips that blit whenever this returns 1. */
    int SubmitKeeperSprite(int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
                           const unsigned char* data, int src_w, int src_h, int32_t content_h,
                           unsigned int draw_flags, const unsigned char* remap,
                           int32_t sprite_id) override;

    /** Begin IR capture for one world-sprite entry (one thing) at the given
     *  depth bucket -- sets the NDC depth and composite sort key that
     *  subsequent SubmitKeeperSprite() calls inherit. Despite the base
     *  interface's "fill-time" doc comment, this branch calls it at WALK
     *  time (from draw_jonty_mapwho()/draw_fastview_mapwho(), P5.7.3b) --
     *  buckets are never skipped here, this call only tags the sort key. */
    int BeginWorldSpriteCapture(int32_t bucket_idx) override;

    /** Vestigial after P5.7.3b: nothing branches on this return value
     *  anymore (draw_keepersprite() just checks SubmitKeeperSprite()'s own
     *  result). Kept for now in case a future caller needs it; candidate
     *  for removal if none turns up. */
    int UsesFillTimeWorldSubmit() const override
    {
        return (m_initialized && m_world_write_cmds != nullptr) ? 1 : 0;
    }

    // ── Cursor keeper-sprite (P5.7.5) ───────────────────────────────────────────

    /** Game thread: redirect SubmitKeeperSprite()'s append target from the
     *  normal per-bucket world list (m_kspr_ir) to a dedicated cursor list
     *  (m_cursor_kspr_ir) for the duration of the bracket, and pin the
     *  z-depth every appended sprite inherits to the nearest legal NDC value
     *  -- GL_LEQUAL against the world pass's own [-1,1] range means cursor
     *  sprites are never occluded by world geometry, without needing to
     *  disable depth testing. Ported from develop's GLCursorLayer::
     *  SubmitKeeperHandSprite() (BeginCursorCapture()/process_keeper_sprite_ex()/
     *  EndCursorCapture()) -- here the bracket lives on GLWorldViewRenderer
     *  since that's where m_kspr_ir/SubmitKeeperSprite() already live on this
     *  branch, rather than GLCursorLayer owning a second copy of the
     *  accumulating-list state. */
    void BeginCursorCapture();

    /** Game thread: end the redirect started by BeginCursorCapture(). */
    void EndCursorCapture();

    /** Render thread: draw every keeper sprite captured into
     *  m_rt_cursor_kspr_ir this frame -- the power-hand cursor's held-thing
     *  and hand-graphic sprites, now both preserved instead of the second
     *  submission overwriting the first. Reuses append_keeper_sprite_instance()
     *  (the same per-sprite atlas/CLUT resolution + instanced-batch path
     *  normal world sprites use) followed by one flush_keeper_sprite_instances()
     *  call -- so cursor sprites benefit from the same atlas caching and
     *  (already-existing) additive-glow instance flag the normal sprite path
     *  has. Called from RendererOpenGL::FGExecuteCursor(), after the world
     *  pass. */
    void DrawCursorKeeperSprites();

private:
    bool init_gl_resources();
    void free_gl_resources();
    bool compile_world_shaders();
    bool init_flatpoly_shader();

    // Append one triangle (3 PolyPoint vertices, integer screen pixels) to the staging array.
    // tile_id is the flat block_ptrs[] index from p->block;
    // variation = tile_id / TEXTURE_BLOCKS_COUNT, tile_local = tile_id % TEXTURE_BLOCKS_COUNT.
    // cam_z0/1/2: camera-space Z for each vertex (for perspective-correct interpolation).
    //             Pass 0 for unknown/no-correction (defaults to 1.0).
    bool append_triangle(int tile_id,
                         const struct PolyPoint* p0,
                         const struct PolyPoint* p1,
                         const struct PolyPoint* p2,
                         int32_t cam_z0 = 0, int32_t cam_z1 = 0, int32_t cam_z2 = 0,
                         int32_t wx0 = 0, int32_t wy0 = 0, int32_t wz0 = 0,
                         int32_t wx1 = 0, int32_t wy1 = 0, int32_t wz1 = 0,
                         int32_t wx2 = 0, int32_t wy2 = 0, int32_t wz2 = 0);

    // Append one triangle from compact-format fields (unsigned short xy, unsigned char uv/shade)
    bool append_triangle_compact(int sx0, int sy0, int u0, int v0, int shade0,
                                 int sx1, int sy1, int u1, int v1, int shade1,
                                 int sx2, int sy2, int u2, int v2, int shade2);

    // Append a front-view textured quad (2 triangles = 6 vertices) from a BucketKindTexturedQuad.
    // Converts the axis-aligned screen quad to WorldVertex format using the tile atlas.
    bool append_frontview_quad(const struct BucketKindTexturedQuad* txquad);

    // Append one flat-colour triangle (QK_PolyMode0/QK_PolyMode4/QK_BasicPolygon).
    // colour_index is an 8-bit palette index, resolved to linear RGB here --
    // new code, not ported: the reference never actually implements this
    // conversion (see the .cpp class-level note).
    bool append_flatpoly_triangle(uint8_t colour_index,
                                  const struct PolyPoint* p0,
                                  const struct PolyPoint* p1,
                                  const struct PolyPoint* p2);

    // Record the current tile batch as a deferred draw command; advances the
    // batch start pointer. No GL calls are issued -- everything is replayed
    // in GPURenderNow().
    void gpu_flush();

    /** Core GL draw pass: uploads the vertex buffers, executes the tile and
     *  flat-poly draw commands in order, then resets the viewport to the
     *  full screen and clears the draw-command list. */
    void gpu_execute_passes(int vp_x, int vp_y_gl, int screen_w, int screen_h,
                            const std::vector<WorldVertex>& tile_verts,
                            const std::vector<FlatPolyVertex>& fp_verts);

    // ── Keeper sprites (P5.7.3a) ───────────────────────────────────────────────
    bool init_keeper_sprite_shader();
    bool init_keeper_sprite_instancing();
    void free_keeper_sprite_resources();  // called from free_gl_resources()

    /** Render-thread: bulk decode+upload of every currently-RAM-resident
     *  keeper sprite, capped at k_kspr_atlas_preload_max. */
    void execute_preload_atlas();

    /** Render-thread: the actual atlas/CLUT cache reset. ClearKeeperSpriteAtlas()
     *  (game thread) only queues a CMD_CLEAR_KSPR_ATLAS command -- this is
     *  what used to run directly on the game thread before P5.7.6, racing
     *  resolve_atlas_layer()/resolve_clut_v() (render thread) on the same
     *  m_kspr_atlas_map/m_kspr_clut_remaps. */
    void execute_clear_atlas();

    /** Rebuild CLUT row 0 (identity) when the palette has changed since the
     *  last check; invalidates every cached remap row (they bake in RGB
     *  values from the old palette). Called at the top of gpu_execute_passes(). */
    void ensure_clut_valid();

    /** Look up (or decode + upload) the atlas layer for a sprite. Keyed by
     *  @p sprite_id (stable across sprite-heap reload, unlike a raw data
     *  pointer -- see the .cpp file header). Returns -1 when the atlas is
     *  absent/full or sprite_id is unknown (< 0). */
    int resolve_atlas_layer(int32_t sprite_id, const unsigned char* data, int src_w, int src_h);

    /** Look up (or lazily build) the CLUT row for a remap table. Returns the
     *  row's V texcoord; identity row 0 when the CLUT is full. */
    float resolve_clut_v(const unsigned char* remap);

    void append_keeper_sprite_instance(const IRWorldKeeperSpriteCmd& cmd);
    void flush_keeper_sprite_instances();

    /** Render-thread: decode + GL-draw one keeper sprite via the
     *  non-instanced fallback path (atlas full, unknown sprite id, or
     *  instancing unavailable). */
    int render_keepersprite_gpu(int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
                                const unsigned char* data, int src_w, int src_h, int32_t content_h,
                                unsigned int draw_flags, const unsigned char* remap,
                                float z_ndc, int sprite_owner, int sprite_wants_outline,
                                int32_t sprite_id);
    void DrawKeeperSpriteGL(const IRWorldKeeperSpriteCmd& cmd);

    // ── Possession lens (P5.8a) ─────────────────────────────────────────────
    bool init_lens_shaders();
    void free_lens_resources();  // called from free_gl_resources()
    void ensure_lens_fbo(int w, int h);
    void upload_lens_textures_if_dirty();

    // ── Creature shadows (P5.7.4) ───────────────────────────────────────────────
    bool init_shadow_shader();
    void free_shadow_resources();  // called from free_gl_resources()

    /** Game thread: capture one shadow quad from a resolved
     *  BucketKindCreatureShadow bucket item into m_shadow_cmds. */
    void append_shadow_quad(const struct BucketKindCreatureShadow* sh,
                            int32_t draw_idx, const unsigned char* data,
                            int src_w, int src_h);

    /** Render thread: draw every captured shadow this frame. Resolves each
     *  entry's atlas layer via resolve_atlas_layer() (shared with the
     *  keeper-sprite atlas -- same cache, no separate decode pipeline) and
     *  issues one draw call for the whole batch. */
    void draw_shadows_gpu();

    // Resolved fresh at each point of use rather than cached as a raw
    // GLuint, so the id stays valid across a reload of the texture.
    GLuint ResolvePaletteTexId() const;
    GLuint ResolveFadeTexId() const;

    // Injected resources (not owned)
    ITileAtlas* m_atlas       = nullptr;
    GLResourceMapper* m_resource_mapper = nullptr;
    GpuResourceHandle m_fade_tex_handle    = kInvalidGpuResource;
    GpuResourceHandle m_palette_tex_handle = kInvalidGpuResource;

    // Tile GL objects
    GLuint m_vao    = 0;
    GLuint m_vbo    = 0;
    GLuint m_shader = 0;

    // Flat-colour polygon GL objects (QK_PolyMode0/4/BasicPolygon)
    GLuint m_flatpoly_shader        = 0;
    GLuint m_flatpoly_vao           = 0;
    GLuint m_flatpoly_vbo           = 0;
    GLint  m_flatpoly_loc_viewport  = -1;

    // Uniform locations (cached at shader compile time)
    GLint  m_loc_tile_atlas  = -1;
    GLint  m_loc_palette     = -1;   // sampler2D u_palette (unit 1)
    // Shade / lighting uniforms (world fragment shader)
    GLint  m_loc_fullbright    = -1;  // u_fullbright
    GLint  m_loc_ambient       = -1;  // u_ambient
    GLint  m_loc_shade_scale   = -1;  // u_shade_scale
    GLint  m_loc_shade_gamma   = -1;  // u_shade_gamma
    GLint  m_loc_lighting_mode = -1;  // u_lighting_mode (0=software-accurate, 1=modern)
    GLint  m_loc_darkness_mode = -1;  // u_darkness_mode (0=linear, 1=palette LUT, 2=fog)
    GLint  m_loc_fade_table    = -1;  // sampler2D u_fade_table (unit 3)
    GLint  m_loc_time          = -1;  // u_time (seconds, fog animation)
    GLint  m_loc_fog_speed     = -1;  // u_fog_speed
    GLint  m_loc_fog_density   = -1;  // u_fog_density
    GLint  m_loc_lightmap      = -1;  // usampler2D u_lightmap (unit 2)
    GLint  m_loc_tile_filter   = -1;  // u_tile_filter (0=nearest, 1=palette-correct bilinear)
    GLint  m_loc_missing_tile  = -1;  // u_missing_tile -- diagnostic checkerboard when atlas absent
    int    m_tile_filter_applied = -1; // last GL filter mode applied; -1 = force re-apply on next flush
    // Lightmap texture (unit 2): mirrors game.lish.subtile_lightness[] as GL_R16UI
    GLuint m_tex_lightmap      = 0;

    // Full OS-window dimensions -- set by SetScreenSize(), used for the
    // full-screen viewport reset in gpu_execute_passes().
    int m_full_screen_w = 0;  // GT:
    int m_full_screen_h = 0;  // GT:

    // Per-frame state -- game-thread write, snapshotted at FlipBuffers
    int m_screen_w     = 0;   // GT:
    int m_screen_h     = 0;   // GT:
    int m_vp_x         = 0;   // GT: viewport left edge in screen pixels
    int m_vp_y         = 0;   // GT: viewport top edge in screen pixels
    int m_current_bucket = 0; // GT: bucket index being processed
    int m_draw_screen_w = 0;  // RT: active viewport width for GL draw calls
    int m_draw_screen_h = 0;  // RT: active viewport height for GL draw calls

    // CPU-side vertex staging buffer (dynamic VBO)
    static const int k_max_verts = 65536;   // ~21000 triangles per frame
    int m_vert_count     = 0;   // GT: vertex count in WorldCommandBuffers::tile_verts
    int m_cmd_vert_start = 0;   // GT: start index of current accumulating tile batch

    // GT: Deferred draw list -- built during DrawIsometricView()/DrawFrontView(),
    // executed in GPURenderNow().
    std::vector<DrawCmd> m_draw_cmds;

    // ── Double-buffer render copies ───────────────────────────────────────────
    // FlipBuffers() std::moves the game-thread vectors here so the game
    // thread can start the next frame while the render thread reads stable
    // copies -- same discipline as RendererOpenGL::GLFrameData's frames[2].
    std::vector<DrawCmd> m_rt_draw_cmds;  // RT:
    int m_rt_screen_w = 0;   // RT:
    int m_rt_screen_h = 0;   // RT:
    int m_rt_vp_x     = 0;   // RT:
    int m_rt_vp_y     = 0;   // RT:

    // ── Lightmap shadow copy ──────────────────────────────────────────────────
    // FlipBuffers() snapshots game.lish.subtile_lightness[] here so the render
    // thread never touches the live game array.  511×511×2 bytes ~= 0.5 MB.
    static constexpr int k_lightmap_w = 511;
    static constexpr int k_lightmap_h = 511;
    uint16_t m_rt_lightmap[k_lightmap_w * k_lightmap_h] = {};

    bool m_initialized = false;
    // Set to true in BeginWorldPass(); not currently consumed by anything in
    // this slice, kept for parity with the reference's frame-activity tracking.
    bool m_world_pass_active = false;  // GT:

    // IR write target -- set by SetWorldCommandBuffers(); used as sentinel.
    WorldCommandBuffers* m_world_write_cmds = nullptr;

    // ── Keeper sprites (P5.7.3a) ───────────────────────────────────────────────

    // Non-instanced fallback: single-texture, palette+CLUT shader.
    GLuint m_kspr_shader        = 0;
    GLuint m_kspr_sprite_tex    = 0;  // 256x256 GL_R8, overwritten per sprite
    GLuint m_kspr_vao           = 0;
    GLuint m_kspr_vbo           = 0;
    GLint  m_kspr_loc_viewport  = -1;
    GLint  m_kspr_loc_sprite    = -1;
    GLint  m_kspr_loc_palette   = -1;
    GLint  m_kspr_loc_alpha     = -1;
    GLint  m_kspr_loc_z_ndc     = -1;

    // Additive-glow variant of the non-instanced shader (Beat 3), ported
    // from develop -- used instead of m_kspr_shader when Lb_SPRITE_ALPHA_ADDITIVE
    // is set. No palette/alpha uniform: the glow colour is computed purely
    // from the sprite's own DK glow-encoding pixel indices (see
    // KSPR_GLOW_FRAGMENT_SHADER).
    GLuint m_kspr_glow_shader       = 0;
    GLint  m_kspr_glow_loc_viewport = -1;
    GLint  m_kspr_glow_loc_sprite   = -1;
    GLint  m_kspr_glow_loc_z_ndc    = -1;

    // Depth-fail outline shaders (Beat 4), ported from develop. Single-texture
    // variant (fallback/remapped sprites):
    GLuint m_kspr_outline_shader       = 0;
    GLint  m_kspr_outline_loc_viewport = -1;
    GLint  m_kspr_outline_loc_sprite   = -1;
    GLint  m_kspr_outline_loc_z_ndc    = -1;
    GLint  m_kspr_outline_loc_color    = -1;
    // Array-atlas variant (normal sprites using GL_TEXTURE_2D_ARRAY):
    GLuint m_kspr_atlas_outline_shader       = 0;
    GLint  m_kspr_atlas_outline_loc_viewport = -1;
    GLint  m_kspr_atlas_outline_loc_sprite   = -1;
    GLint  m_kspr_atlas_outline_loc_z_ndc    = -1;
    GLint  m_kspr_atlas_outline_loc_color    = -1;
    GLint  m_kspr_atlas_outline_loc_layer    = -1;

    // Edge-detect outline shaders (Beat 4) -- same uniforms as the silhouette
    // variants above. Single-texture variant:
    GLuint m_kspr_edge_shader           = 0;
    GLint  m_kspr_edge_loc_viewport     = -1;
    GLint  m_kspr_edge_loc_sprite       = -1;
    GLint  m_kspr_edge_loc_z_ndc        = -1;
    GLint  m_kspr_edge_loc_color        = -1;
    // Array-atlas variant:
    GLuint m_kspr_atlas_edge_shader     = 0;
    GLint  m_kspr_atlas_edge_loc_viewport = -1;
    GLint  m_kspr_atlas_edge_loc_sprite   = -1;
    GLint  m_kspr_atlas_edge_loc_z_ndc    = -1;
    GLint  m_kspr_atlas_edge_loc_color    = -1;
    GLint  m_kspr_atlas_edge_loc_layer    = -1;

    // Sprite decode atlas: GL_TEXTURE_2D_ARRAY where each layer holds one
    // pre-decoded sprite (populated on first use, persists for the level).
    // Fallback to m_kspr_sprite_tex when atlas is full/absent/unsupported.
    static const int k_kspr_atlas_layers = 2048;       // 2048x256x256 GL_R8 = 128 MB
    static const int k_kspr_atlas_preload_max = k_kspr_atlas_layers / 2;
    GLuint m_kspr_sprite_array  = 0;
    GLuint m_kspr_atlas_shader  = 0;  // sampler2DArray variant
    GLint  m_kspr_atlas_loc_viewport = -1;
    GLint  m_kspr_atlas_loc_sprite   = -1;
    GLint  m_kspr_atlas_loc_clut     = -1;
    GLint  m_kspr_atlas_loc_alpha    = -1;
    GLint  m_kspr_atlas_loc_z_ndc    = -1;
    GLint  m_kspr_atlas_loc_layer    = -1;
    GLint  m_kspr_atlas_loc_clut_v   = -1;

    // Additive-glow variant of the atlas shader (Beat 3) -- same relationship
    // to m_kspr_atlas_shader as m_kspr_glow_shader has to m_kspr_shader.
    GLuint m_kspr_atlas_glow_shader       = 0;
    GLint  m_kspr_atlas_glow_loc_viewport = -1;
    GLint  m_kspr_atlas_glow_loc_sprite   = -1;
    GLint  m_kspr_atlas_glow_loc_z_ndc    = -1;
    GLint  m_kspr_atlas_glow_loc_layer    = -1;

    int    m_kspr_atlas_used    = 0;  // next free layer index
    int    m_kspr_atlas_peak    = 0;  // high-water mark (diagnostic only)
    int    m_kspr_atlas_hits    = 0;  // cache hits this frame (diagnostic only)
    int    m_kspr_atlas_misses  = 0;  // cache misses this frame (diagnostic only)
    struct AtlasEntry { int layer; int src_w; };
    // Keyed by sprite id (keepsprite_id-equivalent), not data pointer --
    // heap block addresses aren't safe to use as a long-lived key (see the
    // .cpp file header's note on reset_heap_manager()).
    std::unordered_map<int32_t, AtlasEntry> m_kspr_atlas_map;

    // CLUT (Colour Lookup Table): 256 x k_clut_rows GL_RGBA8 texture.
    // Row 0 = identity (palette[i] for all i). Rows 1..k_clut_rows-1 = one
    // row per unique remap table seen this level.
    static const int k_clut_rows = 128;
    GLuint m_kspr_clut_tex      = 0;
    int    m_kspr_clut_used     = 1;   // next free row (0 = identity, always allocated)
    std::vector<std::array<uint8_t, 256>> m_kspr_clut_remaps;
    uint8_t m_kspr_clut_palette_snap[768] = {};

    // Instanced fast path: all atlas-resident sprites of a frame collapse
    // into one glDrawArraysInstanced call.
    GLuint m_kspr_inst_shader     = 0;
    GLuint m_kspr_inst_quad_vbo   = 0;  // static unit quad shared by the VAOs
    GLuint m_kspr_inst_vao        = 0;
    GLuint m_kspr_inst_vbo        = 0;
    GLint  m_kspr_inst_loc_viewport = -1;
    std::vector<KsprInstance> m_kspr_instances;  // RT: batch scratch

    // Instanced depth-fail outline/edge pass (Beat 4) -- own VAO/VBO
    // (KsprOutlineInstance layout differs from KsprInstance), shares
    // m_kspr_inst_quad_vbo's unit quad. Both shaders share one VAO/VBO;
    // flush_keeper_sprite_instances() just switches GL program.
    GLuint m_kspr_inst_outline_shader       = 0;
    GLuint m_kspr_inst_edge_shader          = 0;
    GLuint m_kspr_inst_outline_vao          = 0;
    GLuint m_kspr_inst_outline_vbo          = 0;
    GLint  m_kspr_inst_outline_loc_viewport = -1;
    GLint  m_kspr_inst_edge_loc_viewport    = -1;
    std::vector<KsprOutlineInstance> m_kspr_outline_instances;  // RT: batch scratch

    // Active VGA palette (R,G,B x 256) -- source pointer registered via
    // SetPaletteSource(); no producer calls it yet (see .cpp header), so
    // this stays null and m_rt_palette stays zeroed until a future beat
    // wires a real caller. Needed for CLUT row construction only -- tiles
    // don't read this (they sample the pre-baked palette texture instead).
    const uint8_t* m_palette_data = nullptr;  // GT:
    uint8_t m_rt_palette[768] = {};           // RT: snapshotted at FlipBuffers()

    // Per-frame sprite capture state -- game-thread write, reset at BeginWorldPass()
    float    m_current_sprite_z       = 0.0f;  // GT: NDC depth for current bucket's sprites
    uint32_t m_current_sprite_sort_key = 0;    // GT: (bucket_idx << 16) | entry_seq
    uint32_t m_sprite_entry_seq        = 0;    // GT: reset per world pass
    size_t   m_kspr_pass_start         = 0;    // GT: kspr IR size at BeginWorldPass

    // GT: sprite IR captured by SubmitKeeperSprite(); RT: stable copy after FlipBuffers().
    std::vector<IRWorldKeeperSpriteCmd> m_kspr_ir;
    std::vector<IRWorldKeeperSpriteCmd> m_rt_kspr_ir;
    std::vector<int> m_kspr_sorted_idx;  // RT: depth-sort scratch, reused across ranges

    // ── Cursor keeper-sprite capture (P5.7.5b / Beat 3) ───────────────────────
    // When true, SubmitKeeperSprite() appends to m_cursor_kspr_ir instead of
    // the normal per-bucket m_kspr_ir -- set/cleared by BeginCursorCapture()/
    // EndCursorCapture(), bracketing power_hand.c's keeper-sprite resolution
    // calls the same way BeginParchmentCapture()/BeginSwipeOverlay() redirect
    // GLUIRenderer's write target. Unlike m_kspr_ir (bucket-depth-sorted,
    // drawn as part of the world pass), m_cursor_kspr_ir is drawn separately
    // and unsorted, in submission order, always on top -- see
    // DrawCursorKeeperSprites().
    bool m_capturing_cursor = false;  // GT
    std::vector<IRWorldKeeperSpriteCmd> m_cursor_kspr_ir;     // GT
    std::vector<IRWorldKeeperSpriteCmd> m_rt_cursor_kspr_ir;  // RT: stable copy after FlipBuffers()

    // ── Creature shadows (P5.7.4) ───────────────────────────────────────────────
    // Minimal shader: samples the existing keeper-sprite atlas array (unit 0),
    // discards on near-zero texel (same alpha-cutout test the sprite shaders
    // use), multiplies the destination colour by (1 - darken) elsewhere --
    // see the .cpp file header for why this approximates trig_render_md10's
    // destination-dependent palette darken rather than reproducing it exactly.
    GLuint m_shadow_shader       = 0;
    GLuint m_shadow_vao          = 0;
    GLuint m_shadow_vbo          = 0;
    GLint  m_shadow_loc_viewport = -1;
    GLint  m_shadow_loc_sprite   = -1;
    // No uniform for darken/z_ndc/layer -- all three are per-vertex
    // attributes (locations 2-4), since a whole frame's shadows batch into
    // one draw call, same reasoning as flat-poly's per-vertex colour.

    // GT: shadow quads captured by append_shadow_quad(); RT: stable copy
    // after FlipBuffers(). No cross-shadow ordering/sort key is needed --
    // each is depth-tested independently against the tile buffer, and
    // multiplicative blending is commutative so shadow-vs-shadow draw
    // order doesn't affect the result.
    std::vector<ShadowSubmission> m_shadow_cmds;
    std::vector<ShadowSubmission> m_rt_shadow_cmds;

    // ── Possession lens (P5.8a) ───────────────────────────────────────────────
    // GT: written once per frame by SubmitPossessionLens(); RT: stable copy
    // after FlipBuffers(). Unlike m_shadow_cmds (a vector, naturally emptied
    // by std::move), IRWorldLensCmd::active is a scalar that survives a
    // move-from unchanged -- FlipBuffers() must explicitly reset m_lens_cmd
    // afterwards so a frame where SubmitPossessionLens() isn't called (lens
    // no longer active) doesn't keep replaying a stale command.
    IRWorldLensCmd m_lens_cmd;      // GT:
    IRWorldLensCmd m_rt_lens_cmd;   // RT:

    GLuint m_lens_scene_fbo       = 0;
    GLuint m_lens_scene_tex       = 0;
    GLuint m_lens_scene_depth_rb  = 0;
    int    m_lens_fbo_w = 0, m_lens_fbo_h = 0;   // RT: current FBO size, 0 = not yet created

    GLuint m_lens_quad_vao = 0, m_lens_quad_vbo = 0;   // shared unit quad, all three passes

    // Sampler units (u_scene, u_mist/u_remap/u_overlay, u_palette) are bound
    // once at link time in init_lens_shaders() -- only the per-frame
    // uniforms below need a cached location.
    GLuint m_lens_shader_mist      = 0;
    GLint  m_lens_mist_loc_src_off = -1, m_lens_mist_loc_src_scale = -1;
    GLint  m_lens_mist_loc_pos     = -1, m_lens_mist_loc_sec       = -1, m_lens_mist_loc_lightness = -1;

    GLuint m_lens_shader_remap       = 0;
    GLint  m_lens_remap_loc_src_off  = -1, m_lens_remap_loc_tex_size = -1;

    GLuint m_lens_shader_overlay      = 0;
    GLint  m_lens_overlay_loc_src_off = -1, m_lens_overlay_loc_src_scale = -1, m_lens_overlay_loc_alpha = -1;

    // Uploaded-texture caches, so upload_lens_textures_if_dirty() can skip
    // a re-upload when *_version hasn't changed since last frame.
    GLuint m_lens_mist_tex        = 0;
    uint32_t m_lens_mist_uploaded_version    = 0xFFFFFFFFu;   // never matches a real version 0-based frame 1
    GLuint m_lens_remap_tex       = 0;   // GL_RG16UI equivalent -- stored as two-channel signed via RG16I
    uint32_t m_lens_remap_uploaded_version   = 0xFFFFFFFFu;
    int      m_lens_remap_tex_w = 0, m_lens_remap_tex_h = 0;
    GLuint m_lens_overlay_tex     = 0;
    uint32_t m_lens_overlay_uploaded_version = 0xFFFFFFFFu;
    int      m_lens_overlay_tex_w = 0, m_lens_overlay_tex_h = 0;
};

/******************************************************************************/
