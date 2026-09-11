/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLWorldViewRenderer.h
 *     Desktop OpenGL world-geometry renderer. 
 */
#pragma once

#include "kfx/renderer/opengl/GLFunctions.h"
#include "kfx/renderer/IWorldViewRenderer.h"
#include "kfx/renderer/WorldVertex.h"
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
            CMD_IR_KEEPER_SPRITES,
            CMD_PRELOAD_KSPR_ATLAS,
            CMD_CLEAR_KSPR_ATLAS,
            CMD_UPDATE_ANIMATED_TILES,
        } type;
        int vert_start = 0;       // CMD_TILES
        int vert_count = 0;       // CMD_TILES
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

    struct KsprOutlineInstance {
        float rect[4];
        float uvext[2];
        float layer;
        float z_ndc;        // sprite z + outline bias
        float flip;         // 0/1
        float color[4];     // owner colour + outline alpha
    };

    /** One creature-shadow quad captured at DrawIsometricView() time */
    struct ShadowSubmission {
        float x[4], y[4], u[4], v[4];
        float z_ndc;
        float darken;
        int32_t draw_idx;
        const unsigned char* data;
        int src_w, src_h;
    };

    GLWorldViewRenderer() = default;
    ~GLWorldViewRenderer() override;

    /** Tile atlas providing GL texture handles. Owned externally
     *  (RendererOpenGL); must outlive this. */
    void SetAtlas(ITileAtlas* atlas) { m_atlas = atlas; }
    /** Re-upload the animated tile rows of the tile atlas. Queued as a
     *  render-thread command, same shape as ClearKeeperSpriteAtlas() below --
     *  call once per game tick, right after update_animating_texture_maps(). */
    void UpdateAnimatedTiles() override;
    // Palette/fade textures: only the handle is stored, resolved fresh at
    // each point of use rather than cached as a raw GLuint, so it stays
    // valid across a reload of the underlying texture.
    void SetResourceMapper(GLResourceMapper* mapper) { m_resource_mapper = mapper; }
    /** GPU resource handle for the 256x256 fade/lighting LUT. */
    void SetFadeTexture(GpuResourceHandle tex) { m_fade_tex_handle = tex; }
    /** GPU resource handle for the 256x1 RGBA palette. */
    void SetPaletteTexture(GpuResourceHandle tex) { m_palette_tex_handle = tex; }

    /** Re-uploads the fade/lighting LUT from pixmap.fade_tables and re-pushes
     *  the shade uniforms from g_renderer_settings. Both are snapshotted too
     *  early at CompileShaders() time (before init_fades_table()/
     *  RendererSettings_Load() have run); the caller invokes this once,
     *  after both are confirmed ready. */
    void RefreshFadeTableAndSettings();

    // IWorldViewRenderer
    void BeginWorldPass(int w, int h, int vp_x, int vp_y) override;
    void DrawIsometricView() override;
    void DrawFrontView(struct Camera* cam) override;
    const char* GetName() const override { return "GLWorldViewRenderer"; }

    /** Compile all GLSL programs owned by this renderer. Idempotent (safe to
     *  call after construction). */
    bool CompileShaders() { return init_gl_resources(); }

    /** Release all GL resources. Must be called on the render thread (the
     *  only thread where the context is current) before the context itself
     *  is destroyed */
    void Shutdown() { free_gl_resources(); }

    /** Notify of the full OS-window dimensions (not the world viewport). */
    void SetScreenSize(int w, int h) override { m_full_screen_w = w; m_full_screen_h = h; }

    void GPURenderNow(const WorldCommandBuffers& cmds);

    void FlipBuffers();

    bool HasPendingCommands() const
    {
        return !m_draw_cmds.empty()
            || (m_vert_count > m_cmd_vert_start)
            || !m_kspr_ir.empty()
            || !m_shadow_cmds.empty();
    }

    // ── IR (Intermediate Representation) path ─────────────────────────────────

    /** Set the IR write target for this frame. Tile vertex data is
     *  written into @p cmds by append_triangle(); ordering is recorded
     *  separately in m_draw_cmds. Call with nullptr to close the write window. */
    void SetWorldCommandBuffers(WorldCommandBuffers* cmds) override { m_world_write_cmds = cmds; }

    /** Replay the captured tile geometry on the render thread. */
    void ExecuteFromIR(const WorldCommandBuffers& cmds) override { GPURenderNow(cmds); }

    /** Attempt to initialise GL resources outside of a world pass, e.g. from
     *  RendererOpenGL::BeginFrame(). No-op when already initialised.
     *  Returns true when initialisation is complete. */
    bool TryEarlyInit() { return init_gl_resources(); }

    // ── Keeper sprites  ───────────────────────────────────────────────

    void SetPaletteSource(const uint8_t* palette) override { m_palette_data = palette; }

    /** Clear the keeper-sprite atlas + CLUT caches (called between levels). */
    void ClearKeeperSpriteAtlas() override;

    /** Queue a bulk decode+upload of every currently-RAM-resident keeper
     *  sprite into the atlas, executed on the render thread. */
    void PreloadKeeperSpriteAtlas() override;

    // ── Possession lens  ─────────────────────────────────────────────

    void SubmitPossessionLens(const IRWorldLensCmd& cmd) override;

    bool BeginLensCapture();

    void ResolveLensComposite();

    bool HasActiveLensPalette() const { return m_rt_lens_cmd.active && m_rt_lens_cmd.has_palette; }

    /** Render thread: the active lens's palette, expanded to RGBA8 (matching
     *  GLFrameData::palette_rgba's layout) into @p out_rgba (>= 1024 bytes).
     *  Only valid when HasActiveLensPalette() is true. */
    void GetActiveLensPaletteRGBA(unsigned char* out_rgba) const;

    /** Submit one keeper-sprite (creature/object) for GPU rendering.*/
    int SubmitKeeperSprite(int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
                           const unsigned char* data, int src_w, int src_h, int32_t content_h,
                           unsigned int draw_flags, const unsigned char* remap,
                           int32_t sprite_id) override;
    int BeginWorldSpriteCapture(int32_t bucket_idx) override;

    // ToDo : Remove.
    /** Vestigial after P5.7.3b: nothing branches on this return value
     *  anymore (draw_keepersprite() just checks SubmitKeeperSprite()'s own
     *  result). Kept for now in case a future caller needs it; candidate
     *  for removal if none turns up. */
    int UsesFillTimeWorldSubmit() const override
    {
        return (m_initialized && m_world_write_cmds != nullptr) ? 1 : 0;
    }

    // ── Cursor keeper-sprite (P5.7.5) ───────────────────────────────────────────
    void BeginCursorCapture();

    /** Game thread: end the redirect started by BeginCursorCapture(). */
    void EndCursorCapture();

    void DrawCursorKeeperSprites();

private:
    bool init_gl_resources();
    void free_gl_resources();
    bool compile_world_shaders();
    void push_shade_uniforms();

    bool append_triangle(int tile_id,
                         const struct PolyPoint* p0,
                         const struct PolyPoint* p1,
                         const struct PolyPoint* p2,
                         int32_t cam_z0 = 0, int32_t cam_z1 = 0, int32_t cam_z2 = 0,
                         int32_t wx0 = 0, int32_t wy0 = 0, int32_t wz0 = 0,
                         int32_t wx1 = 0, int32_t wy1 = 0, int32_t wz1 = 0,
                         int32_t wx2 = 0, int32_t wy2 = 0, int32_t wz2 = 0);

    // Append a front-view textured quad (2 triangles = 6 vertices) from a BucketKindTexturedQuad.
    // Converts the axis-aligned screen quad to WorldVertex format using the tile atlas.
    bool append_frontview_quad(const struct BucketKindTexturedQuad* txquad);

    // Record the current tile batch as a deferred draw command; advances the
    // batch start pointer. No GL calls are issued -- everything is replayed
    // in GPURenderNow().
    void gpu_flush();

    /** Core GL draw pass: uploads the vertex buffer, executes the tile
     *  draw commands in order, then resets the viewport to the full
     *  screen and clears the draw-command list. */
    void gpu_execute_passes(int vp_x, int vp_y_gl, int screen_w, int screen_h,
                            const std::vector<WorldVertex>& tile_verts);

    // ── Keeper sprites (P5.7.3a) ───────────────────────────────────────────────
    bool init_keeper_sprite_shader();
    bool init_keeper_sprite_instancing();
    void free_keeper_sprite_resources();  // called from free_gl_resources()

    /** Render-thread: bulk decode+upload of every currently-RAM-resident
     *  keeper sprite, capped at k_kspr_atlas_preload_max. */
    void execute_preload_atlas();

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

    int render_keepersprite_gpu(int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
                                const unsigned char* data, int src_w, int src_h, int32_t content_h,
                                unsigned int draw_flags, const unsigned char* remap,
                                float z_ndc, int sprite_owner, int sprite_wants_outline,
                                int32_t sprite_id);
    void DrawKeeperSpriteGL(const IRWorldKeeperSpriteCmd& cmd);

    bool init_lens_shaders();
    void free_lens_resources();  // called from free_gl_resources()
    void ensure_lens_fbo(int w, int h);
    void upload_lens_textures_if_dirty();

    bool init_shadow_shader();
    void free_shadow_resources();  // called from free_gl_resources()

    void append_shadow_quad(const struct BucketKindCreatureShadow* sh,
                            int32_t draw_idx, const unsigned char* data,
                            int src_w, int src_h);

    void draw_shadows_gpu();

    GLuint ResolvePaletteTexId() const;
    GLuint ResolveShaderId(GpuResourceHandle handle) const;
    GLuint ResolveFadeTexId() const;

    // Injected resources (not owned)
    ITileAtlas* m_atlas       = nullptr;
    GLResourceMapper* m_resource_mapper = nullptr;
    GpuResourceHandle m_fade_tex_handle    = kInvalidGpuResource;
    GpuResourceHandle m_palette_tex_handle = kInvalidGpuResource;

    // Tile GL objects
    GpuResourceHandle m_geom_handle   = kInvalidGpuResource; // VAO+VBO bundle
    GpuResourceHandle m_shader_handle = kInvalidGpuResource;

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
    GpuResourceHandle m_lightmap_tex_handle = kInvalidGpuResource;

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

    std::vector<DrawCmd> m_rt_draw_cmds;  // RT:
    int m_rt_screen_w = 0;   // RT:
    int m_rt_screen_h = 0;   // RT:
    int m_rt_vp_x     = 0;   // RT:
    int m_rt_vp_y     = 0;   // RT:

    static constexpr int k_lightmap_w = 511;
    static constexpr int k_lightmap_h = 511;
    uint16_t m_rt_lightmap[k_lightmap_w * k_lightmap_h] = {};

    bool m_initialized = false;
    // Set to true in BeginWorldPass(); not currently consumed by anything in
    // this slice, kept for parity with the reference's frame-activity tracking.
    bool m_world_pass_active = false;  // GT:

    // IR write target -- set by SetWorldCommandBuffers(); used as sentinel.
    WorldCommandBuffers* m_world_write_cmds = nullptr;

    // ── Keeper sprites  ───────────────────────────────────────────────

    // Non-instanced fallback: single-texture, palette+CLUT shader.
    GpuResourceHandle m_kspr_shader_handle     = kInvalidGpuResource;
    GpuResourceHandle m_kspr_sprite_tex_handle = kInvalidGpuResource;  // 256x256 GL_R8, overwritten per sprite
    GpuResourceHandle m_kspr_geom_handle       = kInvalidGpuResource;  // VAO+VBO bundle
    GLint  m_kspr_loc_viewport  = -1;
    GLint  m_kspr_loc_sprite    = -1;
    GLint  m_kspr_loc_palette   = -1;
    GLint  m_kspr_loc_alpha     = -1;
    GLint  m_kspr_loc_z_ndc     = -1;

    GpuResourceHandle m_kspr_glow_shader_handle = kInvalidGpuResource;
    GLint  m_kspr_glow_loc_viewport = -1;
    GLint  m_kspr_glow_loc_sprite   = -1;
    GLint  m_kspr_glow_loc_z_ndc    = -1;

    // Depth-fail outline shaders
    GpuResourceHandle m_kspr_outline_shader_handle = kInvalidGpuResource;
    GLint  m_kspr_outline_loc_viewport = -1;
    GLint  m_kspr_outline_loc_sprite   = -1;
    GLint  m_kspr_outline_loc_z_ndc    = -1;
    GLint  m_kspr_outline_loc_color    = -1;
    // Array-atlas variant (normal sprites using GL_TEXTURE_2D_ARRAY):
    GpuResourceHandle m_kspr_atlas_outline_shader_handle = kInvalidGpuResource;
    GLint  m_kspr_atlas_outline_loc_viewport = -1;
    GLint  m_kspr_atlas_outline_loc_sprite   = -1;
    GLint  m_kspr_atlas_outline_loc_z_ndc    = -1;
    GLint  m_kspr_atlas_outline_loc_color    = -1;
    GLint  m_kspr_atlas_outline_loc_layer    = -1;

    GpuResourceHandle m_kspr_edge_shader_handle = kInvalidGpuResource;
    GLint  m_kspr_edge_loc_viewport     = -1;
    GLint  m_kspr_edge_loc_sprite       = -1;
    GLint  m_kspr_edge_loc_z_ndc        = -1;
    GLint  m_kspr_edge_loc_color        = -1;

    // Array-atlas variant:
    GpuResourceHandle m_kspr_atlas_edge_shader_handle = kInvalidGpuResource;
    GLint  m_kspr_atlas_edge_loc_viewport = -1;
    GLint  m_kspr_atlas_edge_loc_sprite   = -1;
    GLint  m_kspr_atlas_edge_loc_z_ndc    = -1;
    GLint  m_kspr_atlas_edge_loc_color    = -1;
    GLint  m_kspr_atlas_edge_loc_layer    = -1;

    // Sprite decode atlas: GL_TEXTURE_2D_ARRAY where each layer holds one
    // pre-decoded sprite (populated on first use, persists for the level).
    // Fallback to m_kspr_sprite_tex_handle when atlas is full/absent/unsupported.
    static const int k_kspr_atlas_layers = 2048;       // 2048x256x256 GL_R8 = 128 MB
    static const int k_kspr_atlas_preload_max = k_kspr_atlas_layers / 2;
    GpuResourceHandle m_kspr_sprite_array_handle = kInvalidGpuResource;
    GpuResourceHandle m_kspr_atlas_shader_handle = kInvalidGpuResource;  // sampler2DArray variant
    GLint  m_kspr_atlas_loc_viewport = -1;
    GLint  m_kspr_atlas_loc_sprite   = -1;
    GLint  m_kspr_atlas_loc_clut     = -1;
    GLint  m_kspr_atlas_loc_alpha    = -1;
    GLint  m_kspr_atlas_loc_z_ndc    = -1;
    GLint  m_kspr_atlas_loc_layer    = -1;
    GLint  m_kspr_atlas_loc_clut_v   = -1;

    // Additive-glow variant of the atlas shader
    GpuResourceHandle m_kspr_atlas_glow_shader_handle = kInvalidGpuResource;
    GLint  m_kspr_atlas_glow_loc_viewport = -1;
    GLint  m_kspr_atlas_glow_loc_sprite   = -1;
    GLint  m_kspr_atlas_glow_loc_z_ndc    = -1;
    GLint  m_kspr_atlas_glow_loc_layer    = -1;

    int    m_kspr_atlas_used    = 0;  // next free layer index
    int    m_kspr_atlas_peak    = 0;  // high-water mark (diagnostic only)
    int    m_kspr_atlas_hits    = 0;  // cache hits this frame (diagnostic only)
    int    m_kspr_atlas_misses  = 0;  // cache misses this frame (diagnostic only)
    struct AtlasEntry { int layer; int src_w; };

    std::unordered_map<int32_t, AtlasEntry> m_kspr_atlas_map;

    static const int k_clut_rows = 128;
    GpuResourceHandle m_kspr_clut_tex_handle = kInvalidGpuResource;
    int    m_kspr_clut_used     = 1;   // next free row (0 = identity, always allocated)
    std::vector<std::array<uint8_t, 256>> m_kspr_clut_remaps;
    uint8_t m_kspr_clut_palette_snap[768] = {};

    GpuResourceHandle m_kspr_inst_shader_handle = kInvalidGpuResource;
    GpuResourceHandle m_kspr_inst_quad_geom_handle = kInvalidGpuResource;
    GpuResourceHandle m_kspr_inst_geom_handle = kInvalidGpuResource;
    GLint  m_kspr_inst_loc_viewport = -1;
    std::vector<KsprInstance> m_kspr_instances;  // RT: batch scratch

    // Instanced depth-fail outline/edge pass
    GpuResourceHandle m_kspr_inst_outline_shader_handle = kInvalidGpuResource;
    GpuResourceHandle m_kspr_inst_edge_shader_handle    = kInvalidGpuResource;
    GpuResourceHandle m_kspr_inst_outline_geom_handle   = kInvalidGpuResource;
    GLint  m_kspr_inst_outline_loc_viewport = -1;
    GLint  m_kspr_inst_edge_loc_viewport    = -1;
    std::vector<KsprOutlineInstance> m_kspr_outline_instances;  // RT: batch scratch

    const uint8_t* m_palette_data = nullptr;  // GT:
    uint8_t m_rt_palette[768] = {};           // RT: snapshotted at FlipBuffers()

    float    m_current_sprite_z       = 0.0f;
    uint32_t m_current_sprite_sort_key = 0;
    uint32_t m_sprite_entry_seq        = 0;
    size_t   m_kspr_pass_start         = 0;

    std::vector<IRWorldKeeperSpriteCmd> m_kspr_ir;
    std::vector<IRWorldKeeperSpriteCmd> m_rt_kspr_ir;
    std::vector<int> m_kspr_sorted_idx;

    bool m_capturing_cursor = false;  // GT
    std::vector<IRWorldKeeperSpriteCmd> m_cursor_kspr_ir;
    std::vector<IRWorldKeeperSpriteCmd> m_rt_cursor_kspr_ir;

    GpuResourceHandle m_shadow_shader_handle = kInvalidGpuResource;
    GpuResourceHandle m_shadow_geom_handle   = kInvalidGpuResource; // VAO+VBO bundle
    GLint  m_shadow_loc_viewport = -1;
    GLint  m_shadow_loc_sprite   = -1;
    std::vector<ShadowSubmission> m_shadow_cmds;
    std::vector<ShadowSubmission> m_rt_shadow_cmds;

    IRWorldLensCmd m_lens_cmd;
    IRWorldLensCmd m_rt_lens_cmd;

    void EnsureLensSceneRT(int w, int h);
    void EnsureLensRemapTexture(int w, int h);
    void EnsureLensOverlayTexture(int w, int h);

    GpuResourceHandle m_lens_scene_rt_handle = kInvalidGpuResource; // RGBA8 color + Depth24Stencil8
    int m_lens_scene_gt_w = 0, m_lens_scene_gt_h = 0; // GT: size last committed at

    GpuResourceHandle m_lens_quad_geom_handle = kInvalidGpuResource; // shared unit quad, all three passes

    GpuResourceHandle m_lens_shader_mist_handle = kInvalidGpuResource;
    GLint  m_lens_mist_loc_src_off = -1, m_lens_mist_loc_src_scale = -1;
    GLint  m_lens_mist_loc_pos     = -1, m_lens_mist_loc_sec       = -1, m_lens_mist_loc_lightness = -1;

    GpuResourceHandle m_lens_shader_remap_handle = kInvalidGpuResource;
    GLint  m_lens_remap_loc_src_off  = -1, m_lens_remap_loc_tex_size = -1;

    GpuResourceHandle m_lens_shader_overlay_handle = kInvalidGpuResource;
    GLint  m_lens_overlay_loc_src_off = -1, m_lens_overlay_loc_src_scale = -1, m_lens_overlay_loc_alpha = -1;

    // Uploaded-texture caches, so upload_lens_textures_if_dirty() can skip
    // a re-upload when *_version hasn't changed since last frame.
    GpuResourceHandle m_lens_mist_tex_handle = kInvalidGpuResource; // fixed 256x256, created lazily on RT
    uint32_t m_lens_mist_uploaded_version    = 0xFFFFFFFFu;   // never matches a real version 0-based frame 1
    GpuResourceHandle m_lens_remap_tex_handle = kInvalidGpuResource; // GL_RG16UI
    uint32_t m_lens_remap_uploaded_version   = 0xFFFFFFFFu;
    int      m_lens_remap_gt_w = 0, m_lens_remap_gt_h = 0; // GT: size last committed at
    GpuResourceHandle m_lens_overlay_tex_handle = kInvalidGpuResource;
    uint32_t m_lens_overlay_uploaded_version = 0xFFFFFFFFu;
    int      m_lens_overlay_gt_w = 0, m_lens_overlay_gt_h = 0; // GT: size last committed at
};

/******************************************************************************/
