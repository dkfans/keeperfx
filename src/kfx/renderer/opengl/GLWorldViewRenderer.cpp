/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLWorldViewRenderer.cpp
 *     Desktop OpenGL world-geometry renderer. P5.7.2 ported tiles + flat-poly
 *     geometry (wired into RendererOpenGL in P5.7.2b). P5.7.3a added
 *     keeper-sprite (creature/object) rendering machinery, unwired. P5.7.3b
 *     wires it up: creatures/objects/traps are now genuinely GL-visible.
 *
 * @par P5.7.3b scope (CPU-side hookup) -- how sprites actually reach here:
 *     P5.7.3a found no CPU-side caller anywhere for SubmitKeeperSprite()/
 *     BeginWorldSpriteCapture() and assumed the fix meant restructuring
 *     `do_map_who_for_thing()` to resolve sprite id/angle/frame/remap at
 *     bucket-FILL time, matching the reference's own design. Re-examined
 *     with DrawIsometricView()/DrawFrontView()'s own bucket walk (below,
 *     P5.7.2a/2b) in hand: both already run a complete, independent walk of
 *     `buckets[]` -- they never call `display_drawlist()`/
 *     `display_fast_drawlist()` -- and simply had no `QK_JontySprite`/
 *     `QK_JontyISOSprite` case (fell to `default: break;`). The actual fix
 *     needed no fill-time restructuring at all: these two cases were added,
 *     calling the SAME unmodified `draw_jonty_mapwho()`/
 *     `draw_fastview_mapwho()`/`draw_iso_only_fastview_mapwho()` functions
 *     software already uses (declared non-static in engine_render.h for
 *     this). The only change inside that shared call chain is in
 *     `draw_keepersprite()` (engine_render.c, the true raster leaf): it now
 *     calls `RendererSubmitKeeperSprite()` before its CPU blit and returns
 *     early if the GPU handled it. `RendererSubmitKeeperSprite()`/
 *     `RendererBeginWorldSpriteCapture()` (new `extern "C"` bridges in
 *     RendererManager.cpp, mirroring `WorldViewRenderer_DrawIsometricView()`'s
 *     own `active_world_renderer()` pattern) both return 0 unconditionally
 *     when software is active -- `SoftwareWorldViewRenderer` doesn't
 *     override either method -- so software's raster path is byte-identical
 *     to before this beat.
 *
 *     Water-line clipping was disclosed here as a gap; Beat 4 closed it:
 *     `IRWorldKeeperSpriteCmd::content_h` carries the visible-row count
 *     separately from `src_h` (which stays the FULL, unclipped content --
 *     still always what's decoded/cached, so a cache hit is never short a
 *     row for a later unclipped draw); the destination rect and UV V-range
 *     both scale by content_h/src_h at draw time instead of at decode time.
 *     `draw_keepersprite()` (engine_render.c) passes its own
 *     `clipped_height` (== SHeight - water_source_cutoff) as content_h, the
 *     same value its CPU fallback already used for the water/lava
 *     clipping the GL path now matches.
 *
 *     True per-pixel alpha blend (`TRF_Transpar_Alpha`, e.g. Ghost) was also
 *     disclosed here as a gap; Beat 4 closed it too, and turned out to be a
 *     much smaller fix than expected once traced: the CPU path already
 *     draws `TRF_Transpar_Alpha` sprites through `render_alpha`/
 *     `alpha_sprite_table` (`compute_alpha_table()`, vidfade.c) -- the exact
 *     same 1-64 glow-encoded-pixel, per-family additive-delta mechanism the
 *     `Lb_SPRITE_ALPHA_ADDITIVE` glow shaders (Beat 3) already implement on
 *     GL. The engine simply never SET that draw flag for the general
 *     world-render case (only `power_hand.c`'s cursor/hand path did, an
 *     earlier session's fix) -- every other `TRF_Transpar_Alpha` site in
 *     this file only set the CPU-only `EngineSpriteDrawUsingAlpha` global.
 *     Fixed by adding `RendererAddDrawFlags(Lb_SPRITE_ALPHA_ADDITIVE)`
 *     alongside each of those (4 sites), mirroring power_hand.c's existing
 *     pattern exactly (including its `RendererClearDrawFlags(Lb_SPRITE_REMAP)`
 *     companion) -- no new GL-side code needed, Beat 3's glow shaders
 *     already handle both the instanced and non-instanced paths.
 *
 * @par P5.7.3a scope (keeper-sprite GL machinery) -- still applies, unchanged:
 *     The GL-side rendering machinery (atlas, CLUT, instancing, shaders) is
 *     ported near-verbatim from `renderer/palette-deglobalize`'s
 *     GLWorldViewRenderer and is self-contained.
 *
 *     Deliberately NOT ported (disclosed, not guessed at):
 *     (Depth-fail outline -- owner-colour silhouette/edge highlighting for
 *       occluded creatures through walls -- WAS ported in Beat 4, both the
 *       non-instanced (m_kspr_outline_shader/m_kspr_atlas_outline_shader/
 *       m_kspr_edge_shader/m_kspr_atlas_edge_shader) and instanced
 *       (m_kspr_inst_outline_shader/m_kspr_inst_edge_shader) paths, gated by
 *       the newly-ported `RendererSettings` module's creature_outline_mode/
 *       _class_mask/_alpha. The non-instanced additive-glow shader PROGRAM
 *       variant -- KSPR_GLOW_FRAGMENT_SHADER/KSPR_ARRAY_GLOW_FRAGMENT_SHADER
 *       -- WAS ported in Beat 3, alongside the instanced path's inline glow
 *       branch: render_keepersprite_gpu()'s atlas-full/unknown-sprite-id
 *       fallback now gets the same family-aware glow.)
 *     - PiP capture and cursor-sprite capture -- already out of scope per
 *       this file's own P5.7.5 note (see IWorldViewRenderer.h), and
 *       confirmed still-unwired stubs on the cursor side.
 *     - Shadows (IRWorldShadowCmd et al) -- P5.7.4, kept separate even
 *       though the reference's gpu_execute_passes() places the shadow pass
 *       adjacent to sprites and shares some decode-buffer state.
 *     - `platform_is_renderdoc_present()` guard (the reference skips
 *       GL_TEXTURE_2D_ARRAY allocation under RenderDoc, working around an
 *       observed interceptor crash) -- no equivalent exists on this branch;
 *       dropped, accepting the risk of a RenderDoc-specific crash during
 *       GPU debugging sessions rather than inventing a substitute.
 *
 *     Branch-specific gaps found and fixed (same pattern as every prior
 *     P5.7.x beat): `g_renderer_settings` doesn't exist (P5.7.2b hit this
 *     too, for tile shading) -- `transpar4_alpha`/`transpar8_alpha` are
 *     hardcoded to 0.5/0.25. `creature_table_length` wasn't `extern`-declared
 *     in creature_graphics.h (one-line fix). `Lb_SPRITE_ALPHA_ADDITIVE`
 *     didn't exist in bflib_video.h (added, matching the reference's own
 *     addition -- needed for the instanced shader's additive branch to have
 *     a flag bit to test, even though nothing sets it yet).
 *
 * @par P5.7.2a scope (tile + flat-poly geometry) -- still applies, unchanged:
 *
 *     1. DrawIsometricView()/DrawFrontView() are written fresh against
 *        THIS branch's actual display_drawlist()/display_fast_drawlist()
 *        bucket-kind switch (src/engine_render.c), not copy-pasted from the
 *        reference -- the reference walks a different, reorganised
 *        WorldIREntry/engine_buckets.h shape from a large refactor this
 *        branch never picked up. The individual leaf conversion functions
 *        (append_triangle/append_triangle_compact/append_frontview_quad)
 *        port verbatim; only the switch/dispatch shell around them differs,
 *        rewritten against the real BucketKind* structs (now shared via
 *        engine_buckets.h, extracted from engine_render.c by this same
 *        beat -- they were previously file-local there, invisible to any
 *        other translation unit).
 *
 *     2. append_flatpoly_triangle() is new code, not ported: the reference
 *        detects QK_PolyMode0/QK_PolyMode4/QK_BasicPolygon buckets and
 *        emits a CMD_FLAT_POLYS draw command, but never actually implements
 *        the bucket-to-FlatPolyVertex conversion anywhere in that branch --
 *        the GPU-side pipeline (shader, VBO, draw call) was fully built and
 *        wired to nothing. Filled in here.
 *
 *     Two engine bucket kinds are deliberately NOT handled (left as
 *     no-op/default, same as sprites): QK_PolygonNearFP (first-person
 *     near-plane subdivision -- draw_subdivided_near_polygon() in
 *     engine_render.c does real perspective-correcting subdivision with no
 *     direct PolyPoint-triple mapping; hand-porting it now, untested,
 *     risked introducing a real bug for a case that isn't needed for the
 *     isometric-view milestone this beat targets) and QK_PolygonSimple
 *     ("possibly unused" on both branches, and the two branches disagree on
 *     its treatment -- target's own switch marks it VM_SolidColor, the
 *     reference's treats it as a textured tile; picking either would be an
 *     unverified guess for a case that may never fire).
 *
 *     P5.7.2b (RendererOpenGL.cpp) wires this class in: constructs it as an
 *     Impl member, calls CompileShaders()/SetAtlas()/SetFadeTexture()/
 *     SetPaletteTexture() from render_thread_init(), binds/flips
 *     WorldCommandBuffers through GLFrameData the same way ui_cmds/text_cmds
 *     already are, and retries the tile atlas's one-time build each
 *     render_thread_work() tick until block_ptrs[] is ready. Deliberately
 *     NOT handled there either: mid-session texture reload safety (the tile
 *     atlas reads block_ptrs[]/block_mem directly with no staging-buffer
 *     mutex the way GLSpriteAtlas has -- a real but narrow race against
 *     Lua's MAP.default_texture reload, out of scope) and animated-tile
 *     refresh (ITileAtlas::UpdateAnimatedTiles() still has no caller).
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/opengl/GLWorldViewRenderer.h"

#include "kfx/renderer/ITileAtlas.h"
#include "kfx/renderer/TileAtlasPacker.h"  // GetTileUV
#include "kfx/renderer/opengl/GLShaders.h"
#include "kfx/renderer/opengl/GLResourceMapper.h"
#include "kfx/renderer/RendererThread.h"   // ASSERT_GAME_THREAD/ASSERT_RENDER_THREAD
#include "kfx/renderer/RendererSettings.h" // g_renderer_settings (Beat 4)
#include "kfx/renderer/RendererManager.h"  // RendererGetCurrentSpriteOwner/WantsOutline (Beat 4)
#include "player_data.h"                    // get_player_color_idx/player_room_colours (Beat 4 outline colour)

#include "engine_buckets.h"   // QKinds enum, BasicQ, BucketKind* structs, buckets[]
#include "engine_render.h"    // BUCKETS_COUNT, orient_to_mapU/V tables
#include "engine_textures.h"  // TEXTURE_BLOCKS_COUNT, TEXTURE_LAND_MARKED_*, block_ptrs[]
#include "creature_graphics.h" // struct KeeperSprite, creature_table[], creature_table_add[]
#include "bflib_render.h"     // struct PolyPoint, render_fade_tables/render_ghost/render_alpha
#include "bflib_vidraw.h"     // vec_window_width/height
#include "bflib_video.h"      // LbPaletteGetReadonly(), pixel_size, Lb_SPRITE_* flags
#include "bflib_basics.h"     // ERRORLOG / SYNCLOG / WARNLOG
#include "vidmode.h"          // pixmap, alpha_sprite_table (P5.7.3b CPU-fallback globals)
#include "player_data.h"      // get_my_player(), get_player_active_camera(), PVM_* (P5.7.3b)
#include "local_camera.h"     // get_local_camera() (P5.7.3b, spinning-key gate)
#include "game_legacy.h"      // game.lish.subtile_lightness (lightmap snapshot in FlipBuffers)

#include <cstring>
#include <cstdlib>
#include <algorithm>
#include "post_inc.h"

/******************************************************************************/

GLuint GLWorldViewRenderer::ResolvePaletteTexId() const
{
    const GLTexture* tex = m_resource_mapper ? m_resource_mapper->ResolveTexture(m_palette_tex_handle) : nullptr;
    return tex ? tex->id : 0;
}

GLuint GLWorldViewRenderer::ResolveFadeTexId() const
{
    const GLTexture* tex = m_resource_mapper ? m_resource_mapper->ResolveTexture(m_fade_tex_handle) : nullptr;
    return tex ? tex->id : 0;
}

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
        ERRORLOG("GLWorldViewRenderer: shader '%s' compile error: %s", debug_name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

/******************************************************************************/
// Keeper-sprite RLE decode (P5.7.3a)

/** Max dimension of a keeper-sprite decode buffer (stride for GL_UNPACK_ROW_LENGTH). */
static constexpr int k_kspr_decode_dim = 256;

/** Scratch buffer used to hold decoded palette indices before upload.
 *  Stride is always k_kspr_decode_dim so glTexSubImage*() can use
 *  GL_UNPACK_ROW_LENGTH regardless of the sprite's actual width. */
static uint8_t s_kspr_decode_buf[k_kspr_decode_dim * k_kspr_decode_dim];

/** Decode keeper-sprite RLE into a stride-k_kspr_decode_dim palette-index
 *  buffer. Format matches TbSprite.Data: negative cmd = transparent skip,
 *  positive cmd = run of palette-index bytes, 0 = end of row. */
static void decode_keeper_rle(uint8_t* dst, const uint8_t* data, int w, int h)
{
    if (!data || w <= 0 || h <= 0) return;

    for (int y = 0; y < h; ++y)
        memset(dst + y * k_kspr_decode_dim, 0, k_kspr_decode_dim);

    const signed char* sp     = reinterpret_cast<const signed char*>(data);
    const signed char* sp_end = sp + (ptrdiff_t)w * h * 3 + h;  // generous worst-case bound
    for (int y = 0; y < h; ++y) {
        uint8_t* row = dst + y * k_kspr_decode_dim;
        int x = 0;
        while (true) {
            if (sp >= sp_end) {
                WARNLOG("decode_keeper_rle: ran past expected data end at row %d", y);
                return;
            }
            signed char cmd = *sp++;
            if (cmd == 0) break;
            if (cmd < 0) {
                x += (int)(-cmd);
            } else {
                int count = (int)cmd;
                for (int i = 0; i < count; ++i) {
                    if (sp >= sp_end) {
                        WARNLOG("decode_keeper_rle: pixel data past expected end");
                        return;
                    }
                    if (x < w) row[x] = (uint8_t)(*sp);
                    ++sp;
                    ++x;
                }
            }
        }
    }
}

/******************************************************************************/

GLWorldViewRenderer::~GLWorldViewRenderer()
{
    free_gl_resources();
}

/******************************************************************************/

bool GLWorldViewRenderer::init_gl_resources()
{
    if (m_initialized)
        return true;

    if (!compile_world_shaders())
    {
        ERRORLOG("GLWorldViewRenderer: compile_world_shaders failed");
        return false;
    }

    // VAO + dynamic VBO for world geometry
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(k_max_verts * sizeof(WorldVertex)),
                 nullptr, GL_DYNAMIC_DRAW);

    // layout(location=0) vec3 a_pos  — x,y,z at byte offset 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WorldVertex),
                          (void*)0);
    glEnableVertexAttribArray(0);
    // layout(location=1) vec2 a_uv   — u,v at byte offset 12 (after x,y,z)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(WorldVertex),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // layout(location=2) float a_shade — shade at byte offset 20 (after x,y,z,u,v)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(WorldVertex),
                          (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // layout(location=3) vec2 a_stl — subtile coords at byte offset 24 (after x,y,z,u,v,shade)
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(WorldVertex),
                          (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    // layout(location=4) float a_camera_z — camera-space depth for perspective correction, byte offset 32
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(WorldVertex),
                          (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    // layout(location=5) float a_layer — texture array layer (atlas variation), byte offset 36
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(WorldVertex),
                          (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(5);
    // layout(location=6) vec3 aWorldPos — pre-projection world-space position
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(WorldVertex),
                          (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(6);

    glBindVertexArray(0);

    // Cache uniform locations and bind samplers to fixed texture units
    glUseProgram(m_shader);
    m_loc_tile_atlas = glGetUniformLocation(m_shader, "u_tile_atlas");
    glUniform1i(m_loc_tile_atlas, 0);   // GL_TEXTURE0 — R8 palette-index atlas array
    m_loc_palette = glGetUniformLocation(m_shader, "u_palette");
    glUniform1i(m_loc_palette, 1);      // GL_TEXTURE1 — 1D RGBA8 palette
    // Shade / lighting uniforms — cache locations and push defaults from
    // g_renderer_settings (Beat 4, ported from develop). Read once here
    // (render_thread_init() time, after RendererSettings_Load() has already
    // run in main.cpp) rather than every frame -- this branch has no live
    // settings-editing UI yet (console commands / ImGui panel / menu
    // overlay are all out of scope, see RendererSettings.h's file header),
    // so there is nothing that could change these after startup anyway.
    m_loc_fullbright    = glGetUniformLocation(m_shader, "u_fullbright");
    m_loc_ambient       = glGetUniformLocation(m_shader, "u_ambient");
    m_loc_shade_scale   = glGetUniformLocation(m_shader, "u_shade_scale");
    m_loc_shade_gamma   = glGetUniformLocation(m_shader, "u_shade_gamma");
    m_loc_lighting_mode = glGetUniformLocation(m_shader, "u_lighting_mode");
    m_loc_darkness_mode = glGetUniformLocation(m_shader, "u_darkness_mode");
    m_loc_fade_table    = glGetUniformLocation(m_shader, "u_fade_table");
    m_loc_time          = glGetUniformLocation(m_shader, "u_time");
    m_loc_fog_speed     = glGetUniformLocation(m_shader, "u_fog_speed");
    m_loc_fog_density   = glGetUniformLocation(m_shader, "u_fog_density");
    m_loc_lightmap      = glGetUniformLocation(m_shader, "u_lightmap");
    m_loc_missing_tile  = glGetUniformLocation(m_shader, "u_missing_tile");
    m_loc_tile_filter   = glGetUniformLocation(m_shader, "u_tile_filter");
    glUniform1f(m_loc_fullbright,    g_renderer_settings.shade_fullbright);
    glUniform1f(m_loc_ambient,       g_renderer_settings.shade_ambient);
    glUniform1f(m_loc_shade_scale,   g_renderer_settings.shade_scale);
    glUniform1f(m_loc_shade_gamma,   g_renderer_settings.shade_gamma);
    glUniform1i(m_loc_lighting_mode, g_renderer_settings.lighting_mode);
    glUniform1i(m_loc_darkness_mode, g_renderer_settings.darkness_mode);
    glUniform1i(m_loc_fade_table,    3);  // GL_TEXTURE3
    glUniform1f(m_loc_time,          0.0f);
    glUniform1f(m_loc_fog_speed,     g_renderer_settings.fog_speed);
    glUniform1f(m_loc_fog_density,   g_renderer_settings.fog_density);
    glUniform1i(m_loc_lightmap,      2);  // GL_TEXTURE2
    glUniform1f(m_loc_missing_tile,  0.0f);
    glUniform1i(m_loc_tile_filter,   g_renderer_settings.tile_filter);
    m_tile_filter_applied = -1;  // force apply on first flush
    glUseProgram(0);

    // Lightmap texture — mirrors game.lish.subtile_lightness[] each frame.
    // GL_R16UI stores the raw 0..16128 lightness values; the shader normalises them.
    glGenTextures(1, &m_tex_lightmap);
    glBindTexture(GL_TEXTURE_2D, m_tex_lightmap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI, MAX_SUBTILES_X, MAX_SUBTILES_Y,
                 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (!init_flatpoly_shader())
    {
        ERRORLOG("GLWorldViewRenderer: failed to initialise flat-colour polygon shader");
        free_gl_resources();
        return false;
    }

    if (!init_keeper_sprite_shader())
    {
        ERRORLOG("GLWorldViewRenderer: failed to initialise keeper-sprite shader");
        free_gl_resources();
        return false;
    }

    // Shadows are polish, not required for the world to draw -- same
    // "opportunistic, non-fatal" treatment as the sprite atlas array itself
    // (init_keeper_sprite_shader() above already accepts that failing).
    // Without it (or without the atlas array it depends on), shadows are
    // just silently skipped -- append_shadow_quad()/draw_shadows_gpu() both
    // guard on m_shadow_shader/m_kspr_sprite_array being non-zero.
    if (!init_shadow_shader())
    {
        WARNLOG("GLWorldViewRenderer: shadow shader unavailable -- creature shadows disabled");
    }

    // Possession lens (P5.8a). Same non-fatal treatment as shadows above --
    // BeginLensCapture()/ResolveLensComposite() both guard on the shaders
    // being non-zero, so a compile failure here just means lens distortion
    // silently doesn't apply under GL (same as it doesn't today), not a
    // renderer init failure.
    if (!init_lens_shaders())
    {
        WARNLOG("GLWorldViewRenderer: lens shaders unavailable -- possession lens distortion disabled");
    }

    m_initialized = true;
    SYNCLOG("GLWorldViewRenderer: initialised (VBO %d verts x %u bytes)",
            k_max_verts, (unsigned)sizeof(WorldVertex));
    return true;
}

void GLWorldViewRenderer::free_gl_resources()
{
    if (m_vao)    { glDeleteVertexArrays(1, &m_vao);  m_vao = 0; }
    if (m_vbo)    { glDeleteBuffers(1, &m_vbo);        m_vbo = 0; }
    if (m_shader) { glDeleteProgram(m_shader);          m_shader = 0; }

    if (m_flatpoly_vao)    { glDeleteVertexArrays(1, &m_flatpoly_vao); m_flatpoly_vao = 0; }
    if (m_flatpoly_vbo)    { glDeleteBuffers(1, &m_flatpoly_vbo);       m_flatpoly_vbo = 0; }
    if (m_flatpoly_shader) { glDeleteProgram(m_flatpoly_shader);        m_flatpoly_shader = 0; }
    if (m_tex_lightmap)    { glDeleteTextures(1, &m_tex_lightmap);      m_tex_lightmap = 0; }

    free_keeper_sprite_resources();
    free_shadow_resources();
    free_lens_resources();

    m_initialized = false;
}

void GLWorldViewRenderer::free_keeper_sprite_resources()
{
    if (m_kspr_vao)          { glDeleteVertexArrays(1, &m_kspr_vao);          m_kspr_vao = 0; }
    if (m_kspr_vbo)          { glDeleteBuffers(1, &m_kspr_vbo);                m_kspr_vbo = 0; }
    if (m_kspr_shader)       { glDeleteProgram(m_kspr_shader);                 m_kspr_shader = 0; }
    if (m_kspr_glow_shader)  { glDeleteProgram(m_kspr_glow_shader);            m_kspr_glow_shader = 0; }
    if (m_kspr_sprite_tex)   { glDeleteTextures(1, &m_kspr_sprite_tex);        m_kspr_sprite_tex = 0; }
    if (m_kspr_sprite_array) { glDeleteTextures(1, &m_kspr_sprite_array);      m_kspr_sprite_array = 0; }
    if (m_kspr_atlas_shader) { glDeleteProgram(m_kspr_atlas_shader);           m_kspr_atlas_shader = 0; }
    if (m_kspr_atlas_glow_shader) { glDeleteProgram(m_kspr_atlas_glow_shader); m_kspr_atlas_glow_shader = 0; }
    if (m_kspr_outline_shader)       { glDeleteProgram(m_kspr_outline_shader);       m_kspr_outline_shader = 0; }
    if (m_kspr_atlas_outline_shader) { glDeleteProgram(m_kspr_atlas_outline_shader); m_kspr_atlas_outline_shader = 0; }
    if (m_kspr_edge_shader)          { glDeleteProgram(m_kspr_edge_shader);          m_kspr_edge_shader = 0; }
    if (m_kspr_atlas_edge_shader)    { glDeleteProgram(m_kspr_atlas_edge_shader);    m_kspr_atlas_edge_shader = 0; }
    m_kspr_atlas_used = 0;
    m_kspr_atlas_map.clear();
    if (m_kspr_clut_tex) { glDeleteTextures(1, &m_kspr_clut_tex); m_kspr_clut_tex = 0; }
    m_kspr_clut_remaps.clear();
    m_kspr_clut_used = 1;
    memset(m_kspr_clut_palette_snap, 0, sizeof(m_kspr_clut_palette_snap));

    if (m_kspr_inst_vao)      { glDeleteVertexArrays(1, &m_kspr_inst_vao);      m_kspr_inst_vao = 0; }
    if (m_kspr_inst_vbo)      { glDeleteBuffers(1, &m_kspr_inst_vbo);            m_kspr_inst_vbo = 0; }
    if (m_kspr_inst_quad_vbo) { glDeleteBuffers(1, &m_kspr_inst_quad_vbo);       m_kspr_inst_quad_vbo = 0; }
    if (m_kspr_inst_shader)   { glDeleteProgram(m_kspr_inst_shader);             m_kspr_inst_shader = 0; }
    m_kspr_instances.clear();

    if (m_kspr_inst_outline_vao)    { glDeleteVertexArrays(1, &m_kspr_inst_outline_vao); m_kspr_inst_outline_vao = 0; }
    if (m_kspr_inst_outline_vbo)    { glDeleteBuffers(1, &m_kspr_inst_outline_vbo);       m_kspr_inst_outline_vbo = 0; }
    if (m_kspr_inst_outline_shader) { glDeleteProgram(m_kspr_inst_outline_shader);        m_kspr_inst_outline_shader = 0; }
    if (m_kspr_inst_edge_shader)    { glDeleteProgram(m_kspr_inst_edge_shader);           m_kspr_inst_edge_shader = 0; }
    m_kspr_outline_instances.clear();
}

bool GLWorldViewRenderer::compile_world_shaders()
{
    GLuint vert = compile_shader_src(GL_VERTEX_SHADER,   WORLD_VERTEX_SHADER,   "world_vert.glsl");
    GLuint frag = compile_shader_src(GL_FRAGMENT_SHADER, WORLD_FRAGMENT_SHADER, "world_frag.glsl");
    if (!vert || !frag)
    {
        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);
        return false;
    }
    m_shader = glCreateProgram();
    glAttachShader(m_shader, vert);
    glAttachShader(m_shader, frag);
    glLinkProgram(m_shader);
    glDeleteShader(vert);
    glDeleteShader(frag);

    GLint linked = 0;
    glGetProgramiv(m_shader, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[512];
        glGetProgramInfoLog(m_shader, sizeof(log), nullptr, log);
        ERRORLOG("GLWorldViewRenderer: shader link error: %s", log);
        glDeleteProgram(m_shader);
        m_shader = 0;
        return false;
    }
    return true;
}

bool GLWorldViewRenderer::init_flatpoly_shader()
{
    GLuint sv = compile_shader_src(GL_VERTEX_SHADER,   FLATPOLY_VERTEX_SHADER,   "flatpoly_vert.glsl");
    GLuint sf = compile_shader_src(GL_FRAGMENT_SHADER, FLATPOLY_FRAGMENT_SHADER, "flatpoly_frag.glsl");
    if (!sv || !sf)
    {
        if (sv) glDeleteShader(sv);
        if (sf) glDeleteShader(sf);
        return false;
    }
    m_flatpoly_shader = glCreateProgram();
    glAttachShader(m_flatpoly_shader, sv);
    glAttachShader(m_flatpoly_shader, sf);
    glLinkProgram(m_flatpoly_shader);
    glDeleteShader(sv);
    glDeleteShader(sf);

    GLint linked = 0;
    glGetProgramiv(m_flatpoly_shader, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[512];
        glGetProgramInfoLog(m_flatpoly_shader, sizeof(log), nullptr, log);
        ERRORLOG("GLWorldViewRenderer: flat-poly shader link error: %s", log);
        glDeleteProgram(m_flatpoly_shader);
        m_flatpoly_shader = 0;
        return false;
    }

    glUseProgram(m_flatpoly_shader);
    m_flatpoly_loc_viewport = glGetUniformLocation(m_flatpoly_shader, "u_viewport");
    glUseProgram(0);

    // VAO + dynamic VBO: sizeof(FlatPolyVertex) per vertex (x, y, z, r, g, b)
    glGenVertexArrays(1, &m_flatpoly_vao);
    glGenBuffers(1, &m_flatpoly_vbo);
    glBindVertexArray(m_flatpoly_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_flatpoly_vbo);
    // layout(location=0) vec3 a_pos  (x, y = screen px; z = NDC)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FlatPolyVertex), (void*)0);
    glEnableVertexAttribArray(0);
    // layout(location=1) vec3 a_color (linear RGB)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FlatPolyVertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    return true;
}

/******************************************************************************/
// Creature shadows (P5.7.4)

namespace {
// GL-side vertex layout for the shadow pass: 7 floats/vertex
// (x, y, u, v, layer, z_ndc, darken). Only used within this section.
struct ShadowGLVertex { float x, y, u, v, layer, z_ndc, darken; };
// RT scratch, rebuilt every draw_shadows_gpu() call -- same file-scope-static
// pattern as s_kspr_decode_buf above (render thread only, no cross-thread access).
std::vector<ShadowGLVertex> s_shadow_verts;
} // namespace

bool GLWorldViewRenderer::init_shadow_shader()
{
    GLuint sv = compile_shader_src(GL_VERTEX_SHADER,   SHADOW_VERTEX_SHADER,   "shadow_vert.glsl");
    GLuint sf = compile_shader_src(GL_FRAGMENT_SHADER, SHADOW_FRAGMENT_SHADER, "shadow_frag.glsl");
    if (!sv || !sf)
    {
        if (sv) glDeleteShader(sv);
        if (sf) glDeleteShader(sf);
        return false;
    }
    m_shadow_shader = glCreateProgram();
    glAttachShader(m_shadow_shader, sv);
    glAttachShader(m_shadow_shader, sf);
    glLinkProgram(m_shadow_shader);
    glDeleteShader(sv);
    glDeleteShader(sf);

    GLint linked = 0;
    glGetProgramiv(m_shadow_shader, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[512];
        glGetProgramInfoLog(m_shadow_shader, sizeof(log), nullptr, log);
        ERRORLOG("GLWorldViewRenderer: shadow shader link error: %s", log);
        glDeleteProgram(m_shadow_shader);
        m_shadow_shader = 0;
        return false;
    }

    glUseProgram(m_shadow_shader);
    m_shadow_loc_viewport = glGetUniformLocation(m_shadow_shader, "u_viewport");
    m_shadow_loc_sprite   = glGetUniformLocation(m_shadow_shader, "u_sprite");
    glUniform1i(m_shadow_loc_sprite, 0);  // GL_TEXTURE0
    glUseProgram(0);

    // VAO + dynamic VBO: 7 floats/vertex (x, y, u, v, layer, z_ndc, darken)
    glGenVertexArrays(1, &m_shadow_vao);
    glGenBuffers(1, &m_shadow_vbo);
    glBindVertexArray(m_shadow_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_shadow_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ShadowGLVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ShadowGLVertex), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ShadowGLVertex), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ShadowGLVertex), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ShadowGLVertex), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glBindVertexArray(0);

    return true;
}

void GLWorldViewRenderer::free_shadow_resources()
{
    if (m_shadow_vbo)    { glDeleteBuffers(1, &m_shadow_vbo);         m_shadow_vbo = 0; }
    if (m_shadow_vao)    { glDeleteVertexArrays(1, &m_shadow_vao);    m_shadow_vao = 0; }
    if (m_shadow_shader) { glDeleteProgram(m_shadow_shader);          m_shadow_shader = 0; }
    m_shadow_cmds.clear();
    m_rt_shadow_cmds.clear();
}

/******************************************************************************/
// Possession lens (P5.8a)
//
// Brackets GPURenderNow() from the render thread's side: BeginLensCapture()
// (called before) redirects world rendering into an offscreen FBO sized to
// match that frame's capture resolution -- which, during a possession-mode
// frame, GPURenderNow() itself already renders at full-screen dimensions,
// since draw_creature_view() (thing_creature.c) presets the engine window to
// (0,0,MyScreenWidth,MyScreenHeight) before calling engine() regardless of
// backend. No change to gpu_execute_passes() itself was needed for this --
// only the FBO bind/unbind bracket around the existing call.
//
// Deliberately not a port of develop's GLLensRenderer/GLLensPass (FBO
// ping-pong + 8-slot pass cache, up to kMaxLensGPUPasses=4 chained passes):
// every shipped lens (config/fxdata/lenses.cfg) uses exactly one pixel
// effect, and LensManager::Draw() itself doesn't blend multiple enabled
// effects even when a lens combines flags (each Draw() unconditionally
// overwrites the whole destination rect) -- so a single composite pass is
// sufficient for parity with software. See the P5.8a plan for the full
// reasoning.
/******************************************************************************/

bool GLWorldViewRenderer::init_lens_shaders()
{
    GLuint vs = compile_shader_src(GL_VERTEX_SHADER, LENS_COMPOSITE_VERTEX_SHADER, "lens_composite_vert.glsl");
    if (!vs)
        return false;

    // Shared unit quad: two triangles covering NDC [-1,1], UV [0,1]. Drawn
    // with glViewport already set to the destination sub-rect (see
    // ResolveLensComposite()), so this one static buffer serves all three
    // passes regardless of where on screen the viewport actually sits.
    static const float k_quad[] = {
        //  x,     y,    u,    v
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
    };
    glGenVertexArrays(1, &m_lens_quad_vao);
    glGenBuffers(1, &m_lens_quad_vbo);
    glBindVertexArray(m_lens_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_lens_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(k_quad), k_quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    bool any_ok = false;

    // Mist
    {
        GLuint fs = compile_shader_src(GL_FRAGMENT_SHADER, LENS_MIST_FRAGMENT_SHADER, "lens_mist_frag.glsl");
        if (fs)
        {
            GLuint prog = glCreateProgram();
            glAttachShader(prog, vs);
            glAttachShader(prog, fs);
            glLinkProgram(prog);
            glDeleteShader(fs);
            GLint linked = 0;
            glGetProgramiv(prog, GL_LINK_STATUS, &linked);
            if (linked)
            {
                m_lens_shader_mist = prog;
                glUseProgram(prog);
                glUniform1i(glGetUniformLocation(prog, "u_scene"), 0);
                glUniform1i(glGetUniformLocation(prog, "u_mist"), 1);
                m_lens_mist_loc_src_off   = glGetUniformLocation(prog, "u_src_off");
                m_lens_mist_loc_src_scale = glGetUniformLocation(prog, "u_src_scale");
                m_lens_mist_loc_pos       = glGetUniformLocation(prog, "u_pos");
                m_lens_mist_loc_sec       = glGetUniformLocation(prog, "u_sec");
                m_lens_mist_loc_lightness = glGetUniformLocation(prog, "u_lightness");
                glUseProgram(0);
                any_ok = true;
            }
            else
            {
                char log[512];
                glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
                ERRORLOG("GLWorldViewRenderer: lens mist shader link error: %s", log);
                glDeleteProgram(prog);
            }
        }
    }

    // Remap (displacement/flyeye -- same table layout, same shader)
    {
        GLuint fs = compile_shader_src(GL_FRAGMENT_SHADER, LENS_REMAP_FRAGMENT_SHADER, "lens_remap_frag.glsl");
        if (fs)
        {
            GLuint prog = glCreateProgram();
            glAttachShader(prog, vs);
            glAttachShader(prog, fs);
            glLinkProgram(prog);
            glDeleteShader(fs);
            GLint linked = 0;
            glGetProgramiv(prog, GL_LINK_STATUS, &linked);
            if (linked)
            {
                m_lens_shader_remap = prog;
                glUseProgram(prog);
                glUniform1i(glGetUniformLocation(prog, "u_scene"), 0);
                glUniform1i(glGetUniformLocation(prog, "u_remap"), 1);
                m_lens_remap_loc_src_off  = glGetUniformLocation(prog, "u_src_off");
                m_lens_remap_loc_tex_size = glGetUniformLocation(prog, "u_tex_size");
                glUseProgram(0);
                any_ok = true;
            }
            else
            {
                char log[512];
                glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
                ERRORLOG("GLWorldViewRenderer: lens remap shader link error: %s", log);
                glDeleteProgram(prog);
            }
        }
    }

    // Overlay
    {
        GLuint fs = compile_shader_src(GL_FRAGMENT_SHADER, LENS_OVERLAY_FRAGMENT_SHADER, "lens_overlay_frag.glsl");
        if (fs)
        {
            GLuint prog = glCreateProgram();
            glAttachShader(prog, vs);
            glAttachShader(prog, fs);
            glLinkProgram(prog);
            glDeleteShader(fs);
            GLint linked = 0;
            glGetProgramiv(prog, GL_LINK_STATUS, &linked);
            if (linked)
            {
                m_lens_shader_overlay = prog;
                glUseProgram(prog);
                glUniform1i(glGetUniformLocation(prog, "u_scene"), 0);
                glUniform1i(glGetUniformLocation(prog, "u_overlay"), 1);
                glUniform1i(glGetUniformLocation(prog, "u_palette"), 2);
                m_lens_overlay_loc_src_off   = glGetUniformLocation(prog, "u_src_off");
                m_lens_overlay_loc_src_scale = glGetUniformLocation(prog, "u_src_scale");
                m_lens_overlay_loc_alpha     = glGetUniformLocation(prog, "u_alpha");
                glUseProgram(0);
                any_ok = true;
            }
            else
            {
                char log[512];
                glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
                ERRORLOG("GLWorldViewRenderer: lens overlay shader link error: %s", log);
                glDeleteProgram(prog);
            }
        }
    }

    glDeleteShader(vs);
    return any_ok;
}

void GLWorldViewRenderer::free_lens_resources()
{
    if (m_lens_quad_vbo) { glDeleteBuffers(1, &m_lens_quad_vbo);      m_lens_quad_vbo = 0; }
    if (m_lens_quad_vao) { glDeleteVertexArrays(1, &m_lens_quad_vao); m_lens_quad_vao = 0; }

    if (m_lens_shader_mist)    { glDeleteProgram(m_lens_shader_mist);    m_lens_shader_mist = 0; }
    if (m_lens_shader_remap)   { glDeleteProgram(m_lens_shader_remap);   m_lens_shader_remap = 0; }
    if (m_lens_shader_overlay) { glDeleteProgram(m_lens_shader_overlay); m_lens_shader_overlay = 0; }

    if (m_lens_scene_tex)      { glDeleteTextures(1, &m_lens_scene_tex);          m_lens_scene_tex = 0; }
    if (m_lens_scene_depth_rb) { glDeleteRenderbuffers(1, &m_lens_scene_depth_rb); m_lens_scene_depth_rb = 0; }
    if (m_lens_scene_fbo)      { glDeleteFramebuffers(1, &m_lens_scene_fbo);       m_lens_scene_fbo = 0; }
    m_lens_fbo_w = m_lens_fbo_h = 0;

    if (m_lens_mist_tex)    { glDeleteTextures(1, &m_lens_mist_tex);    m_lens_mist_tex = 0; }
    if (m_lens_remap_tex)   { glDeleteTextures(1, &m_lens_remap_tex);   m_lens_remap_tex = 0; }
    if (m_lens_overlay_tex) { glDeleteTextures(1, &m_lens_overlay_tex); m_lens_overlay_tex = 0; }
    m_lens_mist_uploaded_version = m_lens_remap_uploaded_version = m_lens_overlay_uploaded_version = 0xFFFFFFFFu;
    m_lens_remap_tex_w = m_lens_remap_tex_h = 0;
    m_lens_overlay_tex_w = m_lens_overlay_tex_h = 0;

    m_lens_cmd    = IRWorldLensCmd{};
    m_rt_lens_cmd = IRWorldLensCmd{};
}

void GLWorldViewRenderer::ensure_lens_fbo(int w, int h)
{
    if (w <= 0 || h <= 0)
        return;
    if (m_lens_scene_fbo != 0 && m_lens_fbo_w == w && m_lens_fbo_h == h)
        return;  // already the right size

    if (m_lens_scene_fbo == 0)      glGenFramebuffers(1, &m_lens_scene_fbo);
    if (m_lens_scene_tex == 0)      glGenTextures(1, &m_lens_scene_tex);
    if (m_lens_scene_depth_rb == 0) glGenRenderbuffers(1, &m_lens_scene_depth_rb);

    glBindTexture(GL_TEXTURE_2D, m_lens_scene_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_lens_scene_depth_rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_lens_scene_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_lens_scene_tex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_lens_scene_depth_rb);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        ERRORLOG("GLWorldViewRenderer: lens scene FBO incomplete (0x%x) -- possession lens distortion disabled this run", (unsigned)status);
        glDeleteFramebuffers(1, &m_lens_scene_fbo);
        m_lens_scene_fbo = 0;
        m_lens_fbo_w = m_lens_fbo_h = 0;
        return;
    }

    m_lens_fbo_w = w;
    m_lens_fbo_h = h;
}

void GLWorldViewRenderer::upload_lens_textures_if_dirty()
{
    switch (m_rt_lens_cmd.type)
    {
    case LensPixelEffectType::Mist:
        if (m_rt_lens_cmd.mist_pixels.size() != 256 * 256)
            break;
        if (m_lens_mist_tex == 0)
        {
            glGenTextures(1, &m_lens_mist_tex);
            glBindTexture(GL_TEXTURE_2D, m_lens_mist_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 256, 256, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
            m_lens_mist_uploaded_version = 0xFFFFFFFFu;
        }
        if (m_rt_lens_cmd.mist_version != m_lens_mist_uploaded_version)
        {
            glBindTexture(GL_TEXTURE_2D, m_lens_mist_tex);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RED, GL_UNSIGNED_BYTE, m_rt_lens_cmd.mist_pixels.data());
            glBindTexture(GL_TEXTURE_2D, 0);
            m_lens_mist_uploaded_version = m_rt_lens_cmd.mist_version;
        }
        break;

    case LensPixelEffectType::Displacement:
    case LensPixelEffectType::Flyeye:
        if (m_rt_lens_cmd.remap_w <= 0 || m_rt_lens_cmd.remap_h <= 0 ||
            (size_t)(m_rt_lens_cmd.remap_w * m_rt_lens_cmd.remap_h * 2) != m_rt_lens_cmd.remap_pixels.size())
            break;
        {
            bool need_realloc = (m_lens_remap_tex == 0) ||
                                 (m_lens_remap_tex_w != m_rt_lens_cmd.remap_w) ||
                                 (m_lens_remap_tex_h != m_rt_lens_cmd.remap_h);
            if (m_lens_remap_tex == 0)
                glGenTextures(1, &m_lens_remap_tex);
            glBindTexture(GL_TEXTURE_2D, m_lens_remap_tex);
            if (need_realloc)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                // Uploaded as unsigned -- entries are always >= 0 in practice
                // (clamped to [0,width)/[0,height) by BuildLookupTable()), so
                // the int16_t bit pattern is identical reinterpreted unsigned.
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16UI, m_rt_lens_cmd.remap_w, m_rt_lens_cmd.remap_h,
                             0, GL_RG_INTEGER, GL_UNSIGNED_SHORT, m_rt_lens_cmd.remap_pixels.data());
                m_lens_remap_tex_w = m_rt_lens_cmd.remap_w;
                m_lens_remap_tex_h = m_rt_lens_cmd.remap_h;
                m_lens_remap_uploaded_version = m_rt_lens_cmd.remap_version;
            }
            else if (m_rt_lens_cmd.remap_version != m_lens_remap_uploaded_version)
            {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_rt_lens_cmd.remap_w, m_rt_lens_cmd.remap_h,
                                GL_RG_INTEGER, GL_UNSIGNED_SHORT, m_rt_lens_cmd.remap_pixels.data());
                m_lens_remap_uploaded_version = m_rt_lens_cmd.remap_version;
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        break;

    case LensPixelEffectType::Overlay:
        if (m_rt_lens_cmd.overlay_w <= 0 || m_rt_lens_cmd.overlay_h <= 0 ||
            (size_t)(m_rt_lens_cmd.overlay_w * m_rt_lens_cmd.overlay_h) != m_rt_lens_cmd.overlay_pixels.size())
            break;
        {
            bool need_realloc = (m_lens_overlay_tex == 0) ||
                                 (m_lens_overlay_tex_w != m_rt_lens_cmd.overlay_w) ||
                                 (m_lens_overlay_tex_h != m_rt_lens_cmd.overlay_h);
            if (m_lens_overlay_tex == 0)
                glGenTextures(1, &m_lens_overlay_tex);
            glBindTexture(GL_TEXTURE_2D, m_lens_overlay_tex);
            if (need_realloc)
            {
                // GL_NEAREST matches COverlayRenderer::Render()'s own
                // "nearest-neighbor sampling" comment.
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_rt_lens_cmd.overlay_w, m_rt_lens_cmd.overlay_h,
                             0, GL_RED, GL_UNSIGNED_BYTE, m_rt_lens_cmd.overlay_pixels.data());
                m_lens_overlay_tex_w = m_rt_lens_cmd.overlay_w;
                m_lens_overlay_tex_h = m_rt_lens_cmd.overlay_h;
                m_lens_overlay_uploaded_version = m_rt_lens_cmd.overlay_version;
            }
            else if (m_rt_lens_cmd.overlay_version != m_lens_overlay_uploaded_version)
            {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_rt_lens_cmd.overlay_w, m_rt_lens_cmd.overlay_h,
                                GL_RED, GL_UNSIGNED_BYTE, m_rt_lens_cmd.overlay_pixels.data());
                m_lens_overlay_uploaded_version = m_rt_lens_cmd.overlay_version;
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        break;

    default:
        break;
    }
}

bool GLWorldViewRenderer::BeginLensCapture()
{
    ASSERT_RENDER_THREAD();

    if (!m_rt_lens_cmd.active || m_rt_lens_cmd.type == LensPixelEffectType::None)
        return false;

    GLuint shader = 0;
    switch (m_rt_lens_cmd.type)
    {
    case LensPixelEffectType::Mist:         shader = m_lens_shader_mist;    break;
    case LensPixelEffectType::Displacement:
    case LensPixelEffectType::Flyeye:       shader = m_lens_shader_remap;   break;
    case LensPixelEffectType::Overlay:      shader = m_lens_shader_overlay; break;
    default: break;
    }
    // The shader for this frame's specific effect failed to compile (see
    // init_lens_shaders()'s WARNLOG) -- don't redirect world rendering into
    // an FBO we then couldn't resolve back to the backbuffer.
    if (shader == 0)
        return false;

    ensure_lens_fbo(m_rt_screen_w, m_rt_screen_h);
    if (m_lens_scene_fbo == 0)
        return false;

    glBindFramebuffer(GL_FRAMEBUFFER, m_lens_scene_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return true;
}

void GLWorldViewRenderer::ResolveLensComposite()
{
    ASSERT_RENDER_THREAD();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    upload_lens_textures_if_dirty();

    const float tex_w = (float)m_lens_fbo_w;
    const float tex_h = (float)m_lens_fbo_h;
    const float vp_x  = (float)m_rt_lens_cmd.viewport_x;
    const float vp_w  = (float)m_rt_lens_cmd.viewport_w;
    const float vp_h  = (float)m_rt_lens_cmd.viewport_h;
    // CPU (draw_creature_view()) only offsets the SOURCE sample by
    // viewport_x, never viewport_y -- see the shader file header note.
    const float src_off_x   = tex_w > 0.0f ? vp_x / tex_w : 0.0f;
    const float src_scale_x = tex_w > 0.0f ? vp_w / tex_w : 1.0f;
    const float src_scale_y = tex_h > 0.0f ? vp_h / tex_h : 1.0f;

    const int dest_vp_y_gl = m_full_screen_h - m_rt_lens_cmd.viewport_y - m_rt_lens_cmd.viewport_h;
    glViewport(m_rt_lens_cmd.viewport_x, dest_vp_y_gl, m_rt_lens_cmd.viewport_w, m_rt_lens_cmd.viewport_h);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glBindVertexArray(m_lens_quad_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_lens_scene_tex);

    switch (m_rt_lens_cmd.type)
    {
    case LensPixelEffectType::Mist:
        if (m_lens_shader_mist)
        {
            glUseProgram(m_lens_shader_mist);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_lens_mist_tex);
            glUniform2f(m_lens_mist_loc_src_off, src_off_x, 0.0f);
            glUniform2f(m_lens_mist_loc_src_scale, src_scale_x, src_scale_y);
            glUniform2f(m_lens_mist_loc_pos, m_rt_lens_cmd.mist_pos_x, m_rt_lens_cmd.mist_pos_y);
            glUniform2f(m_lens_mist_loc_sec, m_rt_lens_cmd.mist_sec_x, m_rt_lens_cmd.mist_sec_y);
            glUniform1f(m_lens_mist_loc_lightness, (float)m_rt_lens_cmd.mist_lightness);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        break;

    case LensPixelEffectType::Displacement:
    case LensPixelEffectType::Flyeye:
        if (m_lens_shader_remap)
        {
            glUseProgram(m_lens_shader_remap);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_lens_remap_tex);
            glUniform2f(m_lens_remap_loc_src_off, src_off_x, 0.0f);
            glUniform2f(m_lens_remap_loc_tex_size, tex_w, tex_h);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        break;

    case LensPixelEffectType::Overlay:
        if (m_lens_shader_overlay)
        {
            glUseProgram(m_lens_shader_overlay);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_lens_overlay_tex);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, ResolvePaletteTexId());
            glUniform2f(m_lens_overlay_loc_src_off, src_off_x, 0.0f);
            glUniform2f(m_lens_overlay_loc_src_scale, src_scale_x, src_scale_y);
            glUniform1f(m_lens_overlay_loc_alpha, m_rt_lens_cmd.overlay_alpha);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        break;

    default:
        break;
    }

    glUseProgram(0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDepthMask(GL_TRUE);
}

void GLWorldViewRenderer::GetActiveLensPaletteRGBA(unsigned char* out_rgba) const
{
    for (int i = 0; i < 256; i++)
    {
        out_rgba[i * 4 + 0] = m_rt_lens_cmd.palette[i * 3 + 0];
        out_rgba[i * 4 + 1] = m_rt_lens_cmd.palette[i * 3 + 1];
        out_rgba[i * 4 + 2] = m_rt_lens_cmd.palette[i * 3 + 2];
        out_rgba[i * 4 + 3] = 255;
    }
}

void GLWorldViewRenderer::append_shadow_quad(const struct BucketKindCreatureShadow* sh,
                                             int32_t draw_idx, const unsigned char* data,
                                             int src_w, int src_h)
{
    if (!m_shadow_shader)
        return;  // shadow GL resources unavailable this run -- no fallback needed, just skip

    ShadowSubmission cmd;
    const struct PolyPoint* pts[4] = { &sh->vertex_first, &sh->vertex_second, &sh->vertex_third, &sh->vertex_fourth };
    for (int i = 0; i < 4; i++)
    {
        cmd.x[i] = (float)pts[i]->X;
        cmd.y[i] = (float)pts[i]->Y;
        // U/V are TO_FIXED() content-pixel offsets (0..src_w-1 / 0..src_h-1);
        // normalize straight to atlas-layer UV [0,1] against the full
        // k_kspr_decode_dim (256) the atlas always allocates a layer at,
        // matching how the atlas is sampled everywhere else in this class.
        cmd.u[i] = (float)pts[i]->U / 65536.0f / (float)k_kspr_decode_dim;
        cmd.v[i] = (float)pts[i]->V / 65536.0f / (float)k_kspr_decode_dim;
    }
    cmd.z_ndc   = 2.0f * (float)sh->bucket_idx / (float)(BUCKETS_COUNT - 1) - 1.0f;
    // dist_sq is clamped to [16,31] by create_shadows() -- approximate linear
    // falloff (closer = darker). Not reverse-engineered from pixmap.fade_tables
    // (see the .cpp file header); a reasonable placeholder pending visual check.
    const float dist_sq = (float)sh->vertex_first.S;
    const float t = std::min(std::max((dist_sq - 16.0f) / 15.0f, 0.0f), 1.0f);
    cmd.darken  = 0.55f - 0.35f * t;
    cmd.draw_idx = draw_idx;
    cmd.data     = data;
    cmd.src_w    = src_w;
    cmd.src_h    = src_h;
    m_shadow_cmds.push_back(cmd);
}

void GLWorldViewRenderer::draw_shadows_gpu()
{
    if (m_rt_shadow_cmds.empty() || !m_shadow_shader)
        return;

    std::vector<ShadowGLVertex>& s_verts = s_shadow_verts;
    s_verts.clear();
    s_verts.reserve(m_rt_shadow_cmds.size() * 6);

    for (const auto& cmd : m_rt_shadow_cmds)
    {
        const int layer = resolve_atlas_layer(cmd.draw_idx, cmd.data, cmd.src_w, cmd.src_h);
        if (layer < 0)
            continue;  // atlas full/unsupported -- shadow silently dropped, matches
                       // the sprite pass's own "no CPU fallback for this beat" scope

        // Triangulate v0,v1,v2 + v0,v2,v3 -- same order create_shadows()'s
        // consumer (display_drawlist()) uses for the CPU trig() calls.
        // u/v are already normalized atlas-layer UV (see append_shadow_quad()).
        const int tri[6] = { 0, 1, 2, 0, 2, 3 };
        for (int t : tri)
        {
            ShadowGLVertex gv;
            gv.x = cmd.x[t];
            gv.y = cmd.y[t];
            gv.u = cmd.u[t];
            gv.v = cmd.v[t];
            gv.layer  = (float)layer;
            gv.z_ndc  = cmd.z_ndc;
            gv.darken = cmd.darken;
            s_verts.push_back(gv);
        }
    }
    if (s_verts.empty())
        return;

    glBindBuffer(GL_ARRAY_BUFFER, m_shadow_vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(s_verts.size() * sizeof(ShadowGLVertex)),
                 s_verts.data(), GL_STREAM_DRAW);

    glUseProgram(m_shadow_shader);
    glBindVertexArray(m_shadow_vao);
    glUniform2f(m_shadow_loc_viewport, (float)m_draw_screen_w, (float)m_draw_screen_h);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_kspr_sprite_array);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    glDepthMask(GL_FALSE);  // shadows test against tile depth but don't occlude each other

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)s_verts.size());

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

/******************************************************************************/
// Keeper sprites (P5.7.3a)

bool GLWorldViewRenderer::init_keeper_sprite_shader()
{
    // Non-instanced fallback: single-texture, palette+CLUT shader.
    GLuint sv = compile_shader_src(GL_VERTEX_SHADER,   KSPR_VERTEX_SHADER,   "kspr_vert.glsl");
    GLuint sf = compile_shader_src(GL_FRAGMENT_SHADER, KSPR_FRAGMENT_SHADER, "kspr_frag.glsl");
    if (!sv || !sf)
    {
        if (sv) glDeleteShader(sv);
        if (sf) glDeleteShader(sf);
        return false;
    }
    m_kspr_shader = glCreateProgram();
    glAttachShader(m_kspr_shader, sv);
    glAttachShader(m_kspr_shader, sf);
    glLinkProgram(m_kspr_shader);
    glDeleteShader(sv);
    glDeleteShader(sf);

    GLint linked = 0;
    glGetProgramiv(m_kspr_shader, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[512];
        glGetProgramInfoLog(m_kspr_shader, sizeof(log), nullptr, log);
        ERRORLOG("GLWorldViewRenderer: keeper-sprite shader link error: %s", log);
        glDeleteProgram(m_kspr_shader);
        m_kspr_shader = 0;
        return false;
    }

    glUseProgram(m_kspr_shader);
    m_kspr_loc_viewport = glGetUniformLocation(m_kspr_shader, "u_viewport");
    m_kspr_loc_sprite   = glGetUniformLocation(m_kspr_shader, "u_sprite");
    m_kspr_loc_palette  = glGetUniformLocation(m_kspr_shader, "u_palette");
    m_kspr_loc_alpha    = glGetUniformLocation(m_kspr_shader, "u_alpha");
    m_kspr_loc_z_ndc    = glGetUniformLocation(m_kspr_shader, "u_z_ndc");
    glUniform1i(m_kspr_loc_sprite,  0);  // GL_TEXTURE0
    glUniform1i(m_kspr_loc_palette, 1);  // GL_TEXTURE1
    glUseProgram(0);

    // Additive-glow variant (Beat 3): same vertex shader, dedicated glow
    // fragment shader -- no palette uniform needed. A link failure here
    // just leaves m_kspr_glow_shader == 0, which render_keepersprite_gpu()'s
    // additive branch already checks before selecting it (falls through to
    // the normal m_kspr_shader path, same "degrade, don't crash" pattern
    // every other optional GL resource on this branch follows).
    {
        GLuint gv = compile_shader_src(GL_VERTEX_SHADER,   KSPR_VERTEX_SHADER,       "kspr_vert.glsl");
        GLuint gf = compile_shader_src(GL_FRAGMENT_SHADER, KSPR_GLOW_FRAGMENT_SHADER, "kspr_glow_frag.glsl");
        if (gv && gf)
        {
            m_kspr_glow_shader = glCreateProgram();
            glAttachShader(m_kspr_glow_shader, gv);
            glAttachShader(m_kspr_glow_shader, gf);
            glLinkProgram(m_kspr_glow_shader);
            glDeleteShader(gv);
            glDeleteShader(gf);
            GLint glinked = 0;
            glGetProgramiv(m_kspr_glow_shader, GL_LINK_STATUS, &glinked);
            if (!glinked)
            {
                char log[512];
                glGetProgramInfoLog(m_kspr_glow_shader, sizeof(log), nullptr, log);
                WARNLOG("GLWorldViewRenderer: keeper-sprite glow shader link error: %s", log);
                glDeleteProgram(m_kspr_glow_shader);
                m_kspr_glow_shader = 0;
            }
            else
            {
                glUseProgram(m_kspr_glow_shader);
                m_kspr_glow_loc_viewport = glGetUniformLocation(m_kspr_glow_shader, "u_viewport");
                m_kspr_glow_loc_sprite   = glGetUniformLocation(m_kspr_glow_shader, "u_sprite");
                m_kspr_glow_loc_z_ndc    = glGetUniformLocation(m_kspr_glow_shader, "u_z_ndc");
                glUniform1i(m_kspr_glow_loc_sprite, 0);  // GL_TEXTURE0
                glUseProgram(0);
            }
        }
        else
        {
            if (gv) glDeleteShader(gv);
            if (gf) glDeleteShader(gf);
            WARNLOG("GLWorldViewRenderer: failed to compile keeper-sprite glow shader");
        }
    }

    // Reusable 256x256 R8 texture -- overwritten per sprite. Zero-initialised
    // so unwritten regions sample as index 0 (transparent) rather than
    // undefined garbage.
    static const uint8_t s_zero_256x256[256 * 256] = {};
    glGenTextures(1, &m_kspr_sprite_tex);
    glBindTexture(GL_TEXTURE_2D, m_kspr_sprite_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 256, 256, 0, GL_RED, GL_UNSIGNED_BYTE, s_zero_256x256);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // VAO + VBO: 6 vertices x (vec2 pos + vec2 uv) = 4 floats each
    glGenVertexArrays(1, &m_kspr_vao);
    glGenBuffers(1, &m_kspr_vbo);
    glBindVertexArray(m_kspr_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_kspr_vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Attempt to build the sprite decode atlas (GL_TEXTURE_2D_ARRAY). This is
    // an optional optimisation: on first use each unique sprite id is decoded
    // once and cached in a layer of the array, so per-frame decode and
    // per-draw upload are both eliminated for repeated sprites. If the
    // driver does not support GL_TEXTURE_2D_ARRAY the atlas is skipped and
    // the single-texture fallback path above is used unchanged.
    do {
        GLuint av = compile_shader_src(GL_VERTEX_SHADER,   KSPR_VERTEX_SHADER,        "kspr_vert.glsl");
        GLuint af = compile_shader_src(GL_FRAGMENT_SHADER, KSPR_ARRAY_FRAGMENT_SHADER, "kspr_array_frag.glsl");
        if (!av || !af) { if (av) glDeleteShader(av); if (af) glDeleteShader(af); break; }

        m_kspr_atlas_shader = glCreateProgram();
        glAttachShader(m_kspr_atlas_shader, av);
        glAttachShader(m_kspr_atlas_shader, af);
        glLinkProgram(m_kspr_atlas_shader);
        glDeleteShader(av);
        glDeleteShader(af);
        GLint alinked = 0;
        glGetProgramiv(m_kspr_atlas_shader, GL_LINK_STATUS, &alinked);
        if (!alinked) {
            char log[512];
            glGetProgramInfoLog(m_kspr_atlas_shader, sizeof(log), nullptr, log);
            WARNLOG("kspr_array shader link failed: %s", log);
            glDeleteProgram(m_kspr_atlas_shader);
            m_kspr_atlas_shader = 0;
            break;
        }
        glUseProgram(m_kspr_atlas_shader);
        m_kspr_atlas_loc_viewport = glGetUniformLocation(m_kspr_atlas_shader, "u_viewport");
        m_kspr_atlas_loc_sprite   = glGetUniformLocation(m_kspr_atlas_shader, "u_sprite");
        m_kspr_atlas_loc_clut     = glGetUniformLocation(m_kspr_atlas_shader, "u_clut");
        m_kspr_atlas_loc_alpha    = glGetUniformLocation(m_kspr_atlas_shader, "u_alpha");
        m_kspr_atlas_loc_z_ndc    = glGetUniformLocation(m_kspr_atlas_shader, "u_z_ndc");
        m_kspr_atlas_loc_layer    = glGetUniformLocation(m_kspr_atlas_shader, "u_layer");
        m_kspr_atlas_loc_clut_v   = glGetUniformLocation(m_kspr_atlas_shader, "u_clut_v");
        glUniform1i(m_kspr_atlas_loc_sprite, 0);  // GL_TEXTURE0
        glUniform1i(m_kspr_atlas_loc_clut,   1);  // GL_TEXTURE1
        glUseProgram(0);

        // Additive-glow variant of the atlas shader (Beat 3) -- same
        // relationship to m_kspr_atlas_shader as m_kspr_glow_shader has to
        // m_kspr_shader above. Failure here just leaves
        // m_kspr_atlas_glow_shader == 0 (checked before use), same
        // degrade-not-crash pattern as the rest of this function.
        {
            GLuint gav = compile_shader_src(GL_VERTEX_SHADER,   KSPR_VERTEX_SHADER,             "kspr_vert.glsl");
            GLuint gaf = compile_shader_src(GL_FRAGMENT_SHADER, KSPR_ARRAY_GLOW_FRAGMENT_SHADER, "kspr_array_glow_frag.glsl");
            if (gav && gaf)
            {
                m_kspr_atlas_glow_shader = glCreateProgram();
                glAttachShader(m_kspr_atlas_glow_shader, gav);
                glAttachShader(m_kspr_atlas_glow_shader, gaf);
                glLinkProgram(m_kspr_atlas_glow_shader);
                glDeleteShader(gav);
                glDeleteShader(gaf);
                GLint galinked = 0;
                glGetProgramiv(m_kspr_atlas_glow_shader, GL_LINK_STATUS, &galinked);
                if (!galinked)
                {
                    char log[512];
                    glGetProgramInfoLog(m_kspr_atlas_glow_shader, sizeof(log), nullptr, log);
                    WARNLOG("kspr_array_glow shader link failed: %s", log);
                    glDeleteProgram(m_kspr_atlas_glow_shader);
                    m_kspr_atlas_glow_shader = 0;
                }
                else
                {
                    glUseProgram(m_kspr_atlas_glow_shader);
                    m_kspr_atlas_glow_loc_viewport = glGetUniformLocation(m_kspr_atlas_glow_shader, "u_viewport");
                    m_kspr_atlas_glow_loc_sprite   = glGetUniformLocation(m_kspr_atlas_glow_shader, "u_sprite");
                    m_kspr_atlas_glow_loc_z_ndc    = glGetUniformLocation(m_kspr_atlas_glow_shader, "u_z_ndc");
                    m_kspr_atlas_glow_loc_layer    = glGetUniformLocation(m_kspr_atlas_glow_shader, "u_layer");
                    glUniform1i(m_kspr_atlas_glow_loc_sprite, 0);  // GL_TEXTURE0
                    glUseProgram(0);
                }
            }
            else
            {
                if (gav) glDeleteShader(gav);
                if (gaf) glDeleteShader(gaf);
                WARNLOG("GLWorldViewRenderer: failed to compile keeper-sprite array glow shader");
            }
        }

        // Allocate the texture array. Clear any pre-existing GL error so the
        // subsequent error check is reliable.
        while (glGetError() != GL_NO_ERROR) {}
        glGenTextures(1, &m_kspr_sprite_array);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_kspr_sprite_array);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8,
                     k_kspr_decode_dim, k_kspr_decode_dim, k_kspr_atlas_layers,
                     0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            WARNLOG("GL_TEXTURE_2D_ARRAY alloc failed (err=%u), sprite atlas disabled", (unsigned)err);
            glDeleteTextures(1, &m_kspr_sprite_array);
            m_kspr_sprite_array = 0;
            glDeleteProgram(m_kspr_atlas_shader);
            m_kspr_atlas_shader = 0;
            if (m_kspr_atlas_glow_shader) { glDeleteProgram(m_kspr_atlas_glow_shader); m_kspr_atlas_glow_shader = 0; }
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            break;
        }
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        SYNCLOG("GLWorldViewRenderer: keeper-sprite decode atlas ready (%d layers, 256x256 GL_R8)",
                k_kspr_atlas_layers);
    } while (false);

    // CLUT texture -- only useful (and only allocated) when the atlas shader
    // compiled successfully.
    if (m_kspr_atlas_shader && !m_kspr_clut_tex)
    {
        glGenTextures(1, &m_kspr_clut_tex);
        glBindTexture(GL_TEXTURE_2D, m_kspr_clut_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, k_clut_rows,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        {
            // Clear the entire CLUT to zero (transparent black) so unwritten
            // rows never produce garbage. Only 256*k_clut_rows*4 = 128 KB.
            std::vector<uint8_t> zero_clut((size_t)256 * k_clut_rows * 4, 0);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, k_clut_rows,
                            GL_RGBA, GL_UNSIGNED_BYTE, zero_clut.data());
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        m_kspr_clut_used = 1;
    }

    // ── Depth-fail outline shaders (Beat 4) ───────────────────────────────────
    // Two variants: single-texture (fallback path) and array-atlas (cached
    // path). Compiled opportunistically -- missing shaders just disable the
    // outline (checked at the draw call site via m_kspr_outline_shader/
    // m_kspr_atlas_outline_shader being 0), same degrade-not-crash pattern
    // every other optional GL resource on this branch follows.
    {
        GLuint ov = compile_shader_src(GL_VERTEX_SHADER,   KSPR_VERTEX_SHADER,          "kspr_vert.glsl");
        GLuint of = compile_shader_src(GL_FRAGMENT_SHADER, KSPR_OUTLINE_FRAGMENT_SHADER, "kspr_outline_frag.glsl");
        if (ov && of)
        {
            m_kspr_outline_shader = glCreateProgram();
            glAttachShader(m_kspr_outline_shader, ov);
            glAttachShader(m_kspr_outline_shader, of);
            glLinkProgram(m_kspr_outline_shader);
            glDeleteShader(ov); glDeleteShader(of);
            GLint ol = 0;
            glGetProgramiv(m_kspr_outline_shader, GL_LINK_STATUS, &ol);
            if (ol)
            {
                glUseProgram(m_kspr_outline_shader);
                m_kspr_outline_loc_viewport = glGetUniformLocation(m_kspr_outline_shader, "u_viewport");
                m_kspr_outline_loc_sprite   = glGetUniformLocation(m_kspr_outline_shader, "u_sprite");
                m_kspr_outline_loc_z_ndc    = glGetUniformLocation(m_kspr_outline_shader, "u_z_ndc");
                m_kspr_outline_loc_color    = glGetUniformLocation(m_kspr_outline_shader, "u_outline_color");
                glUniform1i(m_kspr_outline_loc_sprite, 0); // GL_TEXTURE0
                glUseProgram(0);
            }
            else
            {
                char log[512];
                glGetProgramInfoLog(m_kspr_outline_shader, sizeof(log), nullptr, log);
                WARNLOG("GLWorldViewRenderer: kspr_outline shader link failed: %s", log);
                glDeleteProgram(m_kspr_outline_shader);
                m_kspr_outline_shader = 0;
            }
        }
        else { if (ov) glDeleteShader(ov); if (of) glDeleteShader(of); }

        if (m_kspr_atlas_shader)
        {
            GLuint oav = compile_shader_src(GL_VERTEX_SHADER,   KSPR_VERTEX_SHADER,                "kspr_vert.glsl");
            GLuint oaf = compile_shader_src(GL_FRAGMENT_SHADER, KSPR_ARRAY_OUTLINE_FRAGMENT_SHADER, "kspr_array_outline_frag.glsl");
            if (oav && oaf)
            {
                m_kspr_atlas_outline_shader = glCreateProgram();
                glAttachShader(m_kspr_atlas_outline_shader, oav);
                glAttachShader(m_kspr_atlas_outline_shader, oaf);
                glLinkProgram(m_kspr_atlas_outline_shader);
                glDeleteShader(oav); glDeleteShader(oaf);
                GLint oal = 0;
                glGetProgramiv(m_kspr_atlas_outline_shader, GL_LINK_STATUS, &oal);
                if (oal)
                {
                    glUseProgram(m_kspr_atlas_outline_shader);
                    m_kspr_atlas_outline_loc_viewport = glGetUniformLocation(m_kspr_atlas_outline_shader, "u_viewport");
                    m_kspr_atlas_outline_loc_sprite   = glGetUniformLocation(m_kspr_atlas_outline_shader, "u_sprite");
                    m_kspr_atlas_outline_loc_z_ndc    = glGetUniformLocation(m_kspr_atlas_outline_shader, "u_z_ndc");
                    m_kspr_atlas_outline_loc_color    = glGetUniformLocation(m_kspr_atlas_outline_shader, "u_outline_color");
                    m_kspr_atlas_outline_loc_layer    = glGetUniformLocation(m_kspr_atlas_outline_shader, "u_layer");
                    glUniform1i(m_kspr_atlas_outline_loc_sprite, 0); // GL_TEXTURE0
                    glUseProgram(0);
                }
                else
                {
                    char log[512];
                    glGetProgramInfoLog(m_kspr_atlas_outline_shader, sizeof(log), nullptr, log);
                    WARNLOG("GLWorldViewRenderer: kspr_array_outline shader link failed: %s", log);
                    glDeleteProgram(m_kspr_atlas_outline_shader);
                    m_kspr_atlas_outline_shader = 0;
                }
            }
            else { if (oav) glDeleteShader(oav); if (oaf) glDeleteShader(oaf); }
        }
    }

    // ── Edge-detect outline shaders (Beat 4) ──────────────────────────────────
    // Same structure as the silhouette outline shaders above, but sample 4
    // neighbours to emit only boundary pixels.
    {
        GLuint ev = compile_shader_src(GL_VERTEX_SHADER,   KSPR_VERTEX_SHADER,        "kspr_vert.glsl");
        GLuint ef = compile_shader_src(GL_FRAGMENT_SHADER, KSPR_EDGE_FRAGMENT_SHADER, "kspr_edge_frag.glsl");
        if (ev && ef)
        {
            m_kspr_edge_shader = glCreateProgram();
            glAttachShader(m_kspr_edge_shader, ev);
            glAttachShader(m_kspr_edge_shader, ef);
            glLinkProgram(m_kspr_edge_shader);
            glDeleteShader(ev); glDeleteShader(ef);
            GLint el = 0;
            glGetProgramiv(m_kspr_edge_shader, GL_LINK_STATUS, &el);
            if (el)
            {
                glUseProgram(m_kspr_edge_shader);
                m_kspr_edge_loc_viewport = glGetUniformLocation(m_kspr_edge_shader, "u_viewport");
                m_kspr_edge_loc_sprite   = glGetUniformLocation(m_kspr_edge_shader, "u_sprite");
                m_kspr_edge_loc_z_ndc    = glGetUniformLocation(m_kspr_edge_shader, "u_z_ndc");
                m_kspr_edge_loc_color    = glGetUniformLocation(m_kspr_edge_shader, "u_outline_color");
                glUniform1i(m_kspr_edge_loc_sprite, 0);
                glUseProgram(0);
            }
            else
            {
                char log[512];
                glGetProgramInfoLog(m_kspr_edge_shader, sizeof(log), nullptr, log);
                WARNLOG("GLWorldViewRenderer: kspr_edge shader link failed: %s", log);
                glDeleteProgram(m_kspr_edge_shader);
                m_kspr_edge_shader = 0;
            }
        }
        else { if (ev) glDeleteShader(ev); if (ef) glDeleteShader(ef); }

        if (m_kspr_atlas_shader)
        {
            GLuint eav = compile_shader_src(GL_VERTEX_SHADER,   KSPR_VERTEX_SHADER,              "kspr_vert.glsl");
            GLuint eaf = compile_shader_src(GL_FRAGMENT_SHADER, KSPR_ARRAY_EDGE_FRAGMENT_SHADER, "kspr_array_edge_frag.glsl");
            if (eav && eaf)
            {
                m_kspr_atlas_edge_shader = glCreateProgram();
                glAttachShader(m_kspr_atlas_edge_shader, eav);
                glAttachShader(m_kspr_atlas_edge_shader, eaf);
                glLinkProgram(m_kspr_atlas_edge_shader);
                glDeleteShader(eav); glDeleteShader(eaf);
                GLint eal = 0;
                glGetProgramiv(m_kspr_atlas_edge_shader, GL_LINK_STATUS, &eal);
                if (eal)
                {
                    glUseProgram(m_kspr_atlas_edge_shader);
                    m_kspr_atlas_edge_loc_viewport = glGetUniformLocation(m_kspr_atlas_edge_shader, "u_viewport");
                    m_kspr_atlas_edge_loc_sprite   = glGetUniformLocation(m_kspr_atlas_edge_shader, "u_sprite");
                    m_kspr_atlas_edge_loc_z_ndc    = glGetUniformLocation(m_kspr_atlas_edge_shader, "u_z_ndc");
                    m_kspr_atlas_edge_loc_color    = glGetUniformLocation(m_kspr_atlas_edge_shader, "u_outline_color");
                    m_kspr_atlas_edge_loc_layer    = glGetUniformLocation(m_kspr_atlas_edge_shader, "u_layer");
                    glUniform1i(m_kspr_atlas_edge_loc_sprite, 0);
                    glUseProgram(0);
                }
                else
                {
                    char log[512];
                    glGetProgramInfoLog(m_kspr_atlas_edge_shader, sizeof(log), nullptr, log);
                    WARNLOG("GLWorldViewRenderer: kspr_array_edge shader link failed: %s", log);
                    glDeleteProgram(m_kspr_atlas_edge_shader);
                    m_kspr_atlas_edge_shader = 0;
                }
            }
            else { if (eav) glDeleteShader(eav); if (eaf) glDeleteShader(eaf); }
        }
    }

    SYNCLOG("GLWorldViewRenderer: keeper-sprite shader initialised");

    // Instanced fast path: optional, requires the decode atlas. Failure just
    // leaves the per-sprite path active (m_kspr_inst_shader == 0 gates usage
    // in gpu_execute_passes()).
    if (m_kspr_sprite_array && !init_keeper_sprite_instancing())
        WARNLOG("GLWorldViewRenderer: sprite instancing unavailable, using per-sprite path");

    return true;
}

namespace {
/** Compile + link one instanced sprite program; returns 0 on failure. */
GLuint link_kspr_inst_program(const char* vsrc, const char* fsrc,
                              const char* vname, const char* fname,
                              const char* what)
{
    GLuint sv = compile_shader_src(GL_VERTEX_SHADER,   vsrc, vname);
    GLuint sf = compile_shader_src(GL_FRAGMENT_SHADER, fsrc, fname);
    if (!sv || !sf)
    {
        if (sv) glDeleteShader(sv);
        if (sf) glDeleteShader(sf);
        return 0;
    }
    GLuint prog = glCreateProgram();
    glAttachShader(prog, sv);
    glAttachShader(prog, sf);
    glLinkProgram(prog);
    glDeleteShader(sv);
    glDeleteShader(sf);
    GLint linked = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[512];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        WARNLOG("GLWorldViewRenderer: %s link failed: %s", what, log);
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}
} // namespace

bool GLWorldViewRenderer::init_keeper_sprite_instancing()
{
    m_kspr_inst_shader = link_kspr_inst_program(
        KSPR_INST_VERTEX_SHADER, KSPR_INST_FRAGMENT_SHADER,
        "kspr_inst_vert.glsl", "kspr_inst_frag.glsl", "kspr_inst shader");
    if (!m_kspr_inst_shader)
        return false;

    glUseProgram(m_kspr_inst_shader);
    m_kspr_inst_loc_viewport = glGetUniformLocation(m_kspr_inst_shader, "u_viewport");
    glUniform1i(glGetUniformLocation(m_kspr_inst_shader, "u_sprite"), 0);  // GL_TEXTURE0
    glUniform1i(glGetUniformLocation(m_kspr_inst_shader, "u_clut"),   1);  // GL_TEXTURE1
    glUseProgram(0);

    // Static unit quad shared by the VAO (triangle strip: TL TR BL BR).
    static const float k_unit_quad[8] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };
    glGenBuffers(1, &m_kspr_inst_quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_kspr_inst_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(k_unit_quad), k_unit_quad, GL_STATIC_DRAW);

    // Attribute byte offsets, locked to KsprInstance's layout.
    static_assert(sizeof(KsprInstance) == 44, "KsprInstance layout changed -- update attrib offsets");
    constexpr uintptr_t k_inst_off_rect  = 0;   // float[4]
    constexpr uintptr_t k_inst_off_uvext = 16;  // float[2]
    constexpr uintptr_t k_inst_off_misc  = 24;  // layer, clut_v, alpha, z_ndc
    constexpr uintptr_t k_inst_off_flags = 40;  // uint32_t

    glGenVertexArrays(1, &m_kspr_inst_vao);
    glGenBuffers(1, &m_kspr_inst_vbo);
    glBindVertexArray(m_kspr_inst_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_kspr_inst_quad_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_kspr_inst_vbo);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(KsprInstance),
                          (void*)k_inst_off_rect);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(KsprInstance),
                          (void*)k_inst_off_uvext);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(KsprInstance),
                          (void*)k_inst_off_misc);
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(KsprInstance),
                           (void*)k_inst_off_flags);
    for (GLuint loc = 1; loc <= 4; ++loc)
    {
        glEnableVertexAttribArray(loc);
        glVertexAttribDivisor(loc, 1);
    }
    glBindVertexArray(0);

    // Instanced depth-fail outline shader (Beat 4) -- optional, same
    // degrade-not-crash pattern as the non-instanced outline shaders:
    // without it, sprites still draw instanced, just with no depth-fail
    // outline in the instanced path (m_kspr_outline_shader/
    // m_kspr_atlas_outline_shader still cover the non-instanced fallback).
    m_kspr_inst_outline_shader = link_kspr_inst_program(
        KSPR_INST_OUTLINE_VERTEX_SHADER, KSPR_INST_OUTLINE_FRAGMENT_SHADER,
        "kspr_inst_outline_vert.glsl", "kspr_inst_outline_frag.glsl",
        "kspr_inst_outline shader");
    if (m_kspr_inst_outline_shader)
    {
        glUseProgram(m_kspr_inst_outline_shader);
        m_kspr_inst_outline_loc_viewport =
            glGetUniformLocation(m_kspr_inst_outline_shader, "u_viewport");
        glUniform1i(glGetUniformLocation(m_kspr_inst_outline_shader, "u_sprite"), 0);
        glUseProgram(0);
    }

    // Instanced edge-detect shader -- shares the same vertex shader and VAO
    // as the instanced outline, but uses edge-detection in the fragment shader.
    m_kspr_inst_edge_shader = link_kspr_inst_program(
        KSPR_INST_OUTLINE_VERTEX_SHADER, KSPR_INST_EDGE_FRAGMENT_SHADER,
        "kspr_inst_outline_vert.glsl", "kspr_inst_edge_frag.glsl",
        "kspr_inst_edge shader");
    if (m_kspr_inst_edge_shader)
    {
        glUseProgram(m_kspr_inst_edge_shader);
        m_kspr_inst_edge_loc_viewport =
            glGetUniformLocation(m_kspr_inst_edge_shader, "u_viewport");
        glUniform1i(glGetUniformLocation(m_kspr_inst_edge_shader, "u_sprite"), 0);
        glUseProgram(0);
    }

    // Outline VAO: same shared unit quad, KsprOutlineInstance layout.
    static_assert(sizeof(KsprOutlineInstance) == 52, "KsprOutlineInstance layout changed -- update attrib offsets");
    constexpr uintptr_t k_outl_off_rect  = 0;   // float[4]
    constexpr uintptr_t k_outl_off_uvext = 16;  // float[2]
    constexpr uintptr_t k_outl_off_lzf   = 24;  // layer, z_ndc, flip
    constexpr uintptr_t k_outl_off_color = 36;  // float[4]

    glGenVertexArrays(1, &m_kspr_inst_outline_vao);
    glGenBuffers(1, &m_kspr_inst_outline_vbo);
    glBindVertexArray(m_kspr_inst_outline_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_kspr_inst_quad_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_kspr_inst_outline_vbo);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(KsprOutlineInstance),
                          (void*)k_outl_off_rect);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(KsprOutlineInstance),
                          (void*)k_outl_off_uvext);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(KsprOutlineInstance),
                          (void*)k_outl_off_lzf);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(KsprOutlineInstance),
                          (void*)k_outl_off_color);
    for (GLuint loc = 1; loc <= 4; ++loc)
    {
        glEnableVertexAttribArray(loc);
        glVertexAttribDivisor(loc, 1);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    SYNCLOG("GLWorldViewRenderer: instanced keeper-sprite path ready");
    return true;
}

/******************************************************************************/

void GLWorldViewRenderer::BeginWorldPass(int w, int h, int vp_x, int vp_y)
{
    ASSERT_GAME_THREAD();

    m_screen_w         = w;
    m_screen_h         = h;
    m_vp_x             = vp_x;
    m_vp_y             = vp_y;
    m_current_bucket   = 0;
    m_world_pass_active = true;
    m_sprite_entry_seq  = 0;
    // Sprites captured at fill time (BeginWorldSpriteCapture) between this
    // call and DrawIsometricView()/DrawFrontView() form one whole-pass IR range.
    m_kspr_pass_start   = m_kspr_ir.size();

    // Flush any tile batch from the *previous* sub-pass before starting the
    // new one — but do NOT clear the draw list. Multiple sub-passes per
    // frame (isometric view + front view) must accumulate into a single
    // draw list so GPURenderNow sees all geometry. We DO need to reset the
    // vertex write cursor so the new pass starts fresh.
    gpu_flush();
    m_cmd_vert_start = m_vert_count;

    // P5.7.2b: no GL calls here anymore. The P5.7.2a version of this
    // function called init_gl_resources() and the tile atlas's Init()
    // retry directly, which only happened to compile because nothing
    // called BeginWorldPass() (a game-thread function, per
    // ASSERT_GAME_THREAD() above) yet -- real GL work must run on the
    // render thread instead. CompileShaders() is now called once from
    // RendererOpenGL::render_thread_init(), and the tile atlas build is
    // retried once per render_thread_work() tick until block_ptrs[] is
    // ready, mirroring GLSpriteAtlas's Init()-then-FlushPendingGL() split.
    // By the time BeginWorldPass() is ever called, m_initialized is always
    // true (render_thread_init() runs to completion, blocking, before
    // RendererOpenGL::Init() returns).

    // Set the vec globals that bucket-list filling code reads
    // (setup_rotate_stuff, fill_in_points_*, etc.) without invoking the
    // full software fallback which would also zero WScreen and set
    // vec_screen/poly_screen for the CPU rasteriser -- both unnecessary
    // for the GPU path.
    if (w > 0) vec_window_width  = (long)w;
    if (h > 0) vec_window_height = (long)h;
}

/******************************************************************************/

bool GLWorldViewRenderer::append_triangle(int tile_id,
                                           const struct PolyPoint* p0,
                                           const struct PolyPoint* p1,
                                           const struct PolyPoint* p2,
                                           int32_t cam_z0, int32_t cam_z1, int32_t cam_z2,
                                           int32_t wx0, int32_t wy0, int32_t wz0,
                                           int32_t wx1, int32_t wy1, int32_t wz1,
                                           int32_t wx2, int32_t wy2, int32_t wz2)
{
    const int variation  = tile_id / TEXTURE_BLOCKS_COUNT;
    const int tile_local = tile_id % TEXTURE_BLOCKS_COUNT;

    if (m_vert_count + 3 > k_max_verts)
    {
        gpu_flush();
        if (m_vert_count + 3 > k_max_verts)
            return false; // buffer full; drop gracefully rather than write OOB
    }

    if (!m_world_write_cmds)
        return false;
    std::vector<WorldVertex>& verts_vec = m_world_write_cmds->tile_verts;
    verts_vec.resize((size_t)(m_vert_count + 3));

    // Look up the normalised UV rectangle for this tile in the atlas.
    float u0f, v0f, u1f, v1f;
    TileAtlasPacker::GetTileUV(tile_local, &u0f, &v0f, &u1f, &v1f);

    // Derive NDC depth from the painter's-algorithm bucket index:
    //   high bi (far away) -> z near +1.0; low bi (close) -> z near -1.0.
    const float z_ndc = 2.0f * (float)m_current_bucket / (float)(BUCKETS_COUNT - 1) - 1.0f;
    const float layer = (float)variation;

    WorldVertex* const verts = verts_vec.data();
    const struct PolyPoint* pts[3] = { p0, p1, p2 };
    const int32_t cam_z[3]   = { cam_z0, cam_z1, cam_z2 };
    const int32_t world_x[3] = { wx0, wx1, wx2 };
    const int32_t world_y[3] = { wy0, wy1, wy2 };
    const int32_t world_z[3] = { wz0, wz1, wz2 };
    for (int i = 0; i < 3; i++)
    {
        WorldVertex* wv = &verts[m_vert_count + i];
        // X/Y are integer screen pixels (not 16:16 fixed-point).
        wv->x = (float)(pts[i]->X) / (float)m_screen_w * 2.0f - 1.0f;
        wv->y = 1.0f - (float)(pts[i]->Y) / (float)m_screen_h * 2.0f;
        wv->z = z_ndc;
        // U/V are 16:16; integer part (>>16) is texel 0..31 within the 32-px tile.
        wv->u = u0f + ((float)(pts[i]->U >> 16) / 32.0f) * (u1f - u0f);
        wv->v = v0f + ((float)(pts[i]->V >> 16) / 32.0f) * (v1f - v0f);
        // S = shade_intensity<<8; (S>>16) gives shade level 0..62.
        wv->shade = (float)(pts[i]->S >> 16) / 32.0f;
        wv->stl_x = 0.0f;
        wv->stl_y = 0.0f;
        wv->camera_z = (cam_z[i] > 0) ? (float)cam_z[i] : 1.0f;
        wv->atlas_layer = layer;
        wv->wx = (float)world_x[i];
        wv->wy = (float)world_y[i];
        wv->wz = (float)world_z[i];
    }
    m_vert_count += 3;
    return true;
}

bool GLWorldViewRenderer::append_triangle_compact(
    int sx0, int sy0, int u0, int v0, int shade0,
    int sx1, int sy1, int u1, int v1, int shade1,
    int sx2, int sy2, int u2, int v2, int shade2)
{
    // Compact UV (0..255) addresses the original 8-tiles-wide source layout:
    //   column in 8-wide layout = u8 / 32,  within-tile x = u8 % 32
    //   row in source layout    = v8 / 32,  within-tile y = v8 % 32
    //   tile_id = tile_row * block_count_per_row + tile_col
    // The atlas is repacked 64 tiles wide (2048 px), so we must remap via
    // GetTileUV just as append_triangle does for full PolyPoint polygons.
    // Compact triangles always reference tiles with variation 0.

    if (m_vert_count + 3 > k_max_verts)
    {
        gpu_flush();
        if (m_vert_count + 3 > k_max_verts)
            return false; // buffer full; drop gracefully rather than write OOB
    }

    if (!m_world_write_cmds)
        return false;
    std::vector<WorldVertex>& verts_vec = m_world_write_cmds->tile_verts;
    verts_vec.resize((size_t)(m_vert_count + 3));

    auto compact_to_atlas = [](int u8, int v8, float& out_u, float& out_v)
    {
        const int tile_col = (u8 >> 5) & 7;   // u8 / 32 — column in 8-wide source (0-7)
        const int within_x =  u8 & 31;         // u8 % 32 — pixel within tile horizontally
        const int tile_row =  v8 >> 5;          // v8 / 32 — tile row in source
        const int within_y =  v8 & 31;          // v8 % 32 — pixel within tile vertically
        const int tile_id  = tile_row * (int)block_count_per_row + tile_col;
        float u0f, v0f, u1f, v1f;
        TileAtlasPacker::GetTileUV(tile_id, &u0f, &v0f, &u1f, &v1f);
        out_u = u0f + ((float)within_x / 32.0f) * (u1f - u0f);
        out_v = v0f + ((float)within_y / 32.0f) * (v1f - v0f);
    };

    const float z_ndc = 2.0f * (float)m_current_bucket / (float)(BUCKETS_COUNT - 1) - 1.0f;

    WorldVertex* const verts = verts_vec.data();
    WorldVertex* v = &verts[m_vert_count];

    COMPACT_UV_TO_WORLDVERTEX(&v[0], sx0, sy0, u0, v0, shade0, m_screen_w, m_screen_h);
    COMPACT_UV_TO_WORLDVERTEX(&v[1], sx1, sy1, u1, v1, shade1, m_screen_w, m_screen_h);
    COMPACT_UV_TO_WORLDVERTEX(&v[2], sx2, sy2, u2, v2, shade2, m_screen_w, m_screen_h);
    compact_to_atlas(u0, v0, v[0].u, v[0].v);
    compact_to_atlas(u1, v1, v[1].u, v[1].v);
    compact_to_atlas(u2, v2, v[2].u, v[2].v);
    v[0].z = z_ndc; v[1].z = z_ndc; v[2].z = z_ndc;
    m_vert_count += 3;
    return true;
}

bool GLWorldViewRenderer::append_frontview_quad(const struct BucketKindTexturedQuad* txquad)
{
    // Mirror draw_texturedquad_block() (engine_render.c) exactly, including
    // its /pixel_size scaling (the reference this was ported from has no
    // such division -- this branch's own draw_texturedquad_block() does,
    // so match it, not the reference).
    struct PolyPoint a, d, b, c;
    a.X = (txquad->texture_x >> 8) / pixel_size;
    a.Y = (txquad->texture_y >> 8) / pixel_size;
    a.U = orient_to_mapU1[txquad->orient];
    a.V = orient_to_mapV1[txquad->orient];
    a.S = txquad->shade_intensity0;

    d.X = ((txquad->zoom_x + txquad->texture_x) >> 8) / pixel_size;
    d.Y = (txquad->texture_y >> 8) / pixel_size;
    d.U = orient_to_mapU2[txquad->orient];
    d.V = orient_to_mapV2[txquad->orient];
    d.S = txquad->shade_intensity1;

    b.X = ((txquad->zoom_x + txquad->texture_x) >> 8) / pixel_size;
    b.Y = ((txquad->zoom_y + txquad->texture_y) >> 8) / pixel_size;
    b.U = orient_to_mapU3[txquad->orient];
    b.V = orient_to_mapV3[txquad->orient];
    b.S = txquad->shade_intensity2;

    c.X = (txquad->texture_x >> 8) / pixel_size;
    c.Y = ((txquad->zoom_y + txquad->texture_y) >> 8) / pixel_size;
    c.U = orient_to_mapU4[txquad->orient];
    c.V = orient_to_mapV4[txquad->orient];
    c.S = txquad->shade_intensity3;

    int tile_id;
    switch (txquad->marked_mode)
    {
        case 0:  tile_id = TEXTURE_LAND_MARKED_LAND; break;
        case 1:  tile_id = TEXTURE_LAND_MARKED_GOLD; break;
        default: tile_id = (int)txquad->texture_idx; break;
    }

    bool ok = append_triangle(tile_id, &a, &d, &b);
    ok     &= append_triangle(tile_id, &a, &b, &c);
    return ok;
}

bool GLWorldViewRenderer::append_flatpoly_triangle(uint8_t colour_index,
                                                    const struct PolyPoint* p0,
                                                    const struct PolyPoint* p1,
                                                    const struct PolyPoint* p2)
{
    // New code, not ported -- see the file-level comment: the reference
    // detects flat-poly buckets but never implements this conversion.
    if (!m_world_write_cmds)
        return false;

    std::vector<FlatPolyVertex>& verts = m_world_write_cmds->flat_poly_verts;
    const size_t base = verts.size();
    verts.resize(base + 3);

    const float z_ndc = 2.0f * (float)m_current_bucket / (float)(BUCKETS_COUNT - 1) - 1.0f;

    // 6-bit-per-channel palette -> 8-bit -> [0,1] float, same expansion
    // TileAtlasPacker.cpp already uses for tile pixels (P5.7.1).
    const uint8_t* pal = LbPaletteGetReadonly();
    const float r = (float)(pal[colour_index * 3 + 0] << 2) / 255.0f;
    const float g = (float)(pal[colour_index * 3 + 1] << 2) / 255.0f;
    const float b = (float)(pal[colour_index * 3 + 2] << 2) / 255.0f;

    const struct PolyPoint* pts[3] = { p0, p1, p2 };
    for (int i = 0; i < 3; i++)
    {
        FlatPolyVertex* fv = &verts[base + i];
        fv->x = (float)(pts[i]->X);  // raw screen pixels -- shader does NDC conversion
        fv->y = (float)(pts[i]->Y);
        fv->z = z_ndc;
        fv->r = r;
        fv->g = g;
        fv->b = b;
    }
    return true;
}

/******************************************************************************/
// Keeper sprites (P5.7.3a)

void GLWorldViewRenderer::ClearKeeperSpriteAtlas()
{
    // Game thread: must not touch m_kspr_atlas_map/m_kspr_clut_* directly --
    // resolve_atlas_layer()/resolve_clut_v() (render thread) can be mid-flight
    // on the *previous* frame's m_rt_kspr_ir/m_rt_shadow_cmds concurrently
    // (double-buffered architecture), and a concurrent clear()+find()/insert()
    // on the same unordered_map is undefined behaviour (P5.7.6 fix -- this
    // directly mutated the maps here before, latent since P5.7.3a until
    // P5.7.5 gave it its first real caller). Same queued-command pattern
    // PreloadKeeperSpriteAtlas() already used correctly below.
    DrawCmd cmd;
    cmd.type = DrawCmd::CMD_CLEAR_KSPR_ATLAS;
    m_draw_cmds.push_back(cmd);
    SYNCLOG("GLWorldViewRenderer: CMD_CLEAR_KSPR_ATLAS queued");
}

void GLWorldViewRenderer::execute_clear_atlas()
{
    m_kspr_atlas_map.clear();
    m_kspr_atlas_used = 0;
    m_kspr_clut_remaps.clear();
    m_kspr_clut_used = 1;
    memset(m_kspr_clut_palette_snap, 0, sizeof(m_kspr_clut_palette_snap));
    SYNCDBG(6, "GLWorldViewRenderer: keeper-sprite atlas cleared");
}

void GLWorldViewRenderer::PreloadKeeperSpriteAtlas()
{
    // Push a CMD_PRELOAD_KSPR_ATLAS into the game-thread draw list. It
    // crosses FlipBuffers() safely (inside m_rt_draw_cmds) and is consumed
    // by execute_preload_atlas() on the render thread before any sprite
    // draws -- same IR-consistent pattern as every other deferred command
    // here, no bare cross-thread booleans.
    DrawCmd cmd;
    cmd.type = DrawCmd::CMD_PRELOAD_KSPR_ATLAS;
    m_draw_cmds.push_back(cmd);
    SYNCLOG("GLWorldViewRenderer: CMD_PRELOAD_KSPR_ATLAS queued");
}

void GLWorldViewRenderer::execute_preload_atlas()
{
    if (!m_kspr_sprite_array || !m_kspr_atlas_shader) {
        ERRORLOG("execute_preload_atlas: GL resources not ready -- preload skipped (sprite_array=%u, shader=%u)",
                 m_kspr_sprite_array, m_kspr_atlas_shader);
        return;
    }

    ensure_clut_valid();  // identity row 0 must exist before any atlas draws

    int preloaded = 0;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_kspr_sprite_array);

    // Vanilla sprites: only those already resident in RAM. keepsprite[i] is
    // lazy-loaded from disk; null entries haven't been accessed yet and
    // cannot be preloaded without a full disk-load pass.
    for (int i = 0; i < KEEPSPRITE_LENGTH && m_kspr_atlas_used < k_kspr_atlas_preload_max; i++)
    {
        if (!keepsprite[i] || !*keepsprite[i]) continue;
        if ((size_t)i >= creature_table_length) continue;
        const struct KeeperSprite& ks = creature_table[i];
        if (ks.SWidth <= 0 || ks.SHeight <= 0 ||
            ks.SWidth > k_kspr_decode_dim || ks.SHeight > k_kspr_decode_dim) continue;
        const uint8_t* data = *keepsprite[i];
        if (m_kspr_atlas_map.count(i)) continue;

        memset(s_kspr_decode_buf, 0, sizeof(s_kspr_decode_buf));
        decode_keeper_rle(s_kspr_decode_buf, data, ks.SWidth, ks.SHeight);
        int layer = m_kspr_atlas_used++;
        glPixelStorei(GL_UNPACK_ROW_LENGTH, k_kspr_decode_dim);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer,
                        ks.SWidth, k_kspr_decode_dim, 1,
                        GL_RED, GL_UNSIGNED_BYTE, s_kspr_decode_buf);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        m_kspr_atlas_map[i] = {layer, ks.SWidth};
        preloaded++;
    }

    // Custom sprites: fully in RAM after init_custom_sprites().
    for (int i = 0; i < KEEPERSPRITE_ADD_NUM && m_kspr_atlas_used < k_kspr_atlas_preload_max; i++)
    {
        if (!keepersprite_add[i]) continue;
        const struct KeeperSprite& ks = creature_table_add[i];
        if (ks.SWidth <= 0 || ks.SHeight <= 0 ||
            ks.SWidth > k_kspr_decode_dim || ks.SHeight > k_kspr_decode_dim) continue;
        const uint8_t* data = keepersprite_add[i];
        const int32_t sprite_id = KEEPERSPRITE_ADD_OFFSET + i;
        if (m_kspr_atlas_map.count(sprite_id)) continue;

        memset(s_kspr_decode_buf, 0, sizeof(s_kspr_decode_buf));
        decode_keeper_rle(s_kspr_decode_buf, data, ks.SWidth, ks.SHeight);
        int layer = m_kspr_atlas_used++;
        glPixelStorei(GL_UNPACK_ROW_LENGTH, k_kspr_decode_dim);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer,
                        ks.SWidth, k_kspr_decode_dim, 1,
                        GL_RED, GL_UNSIGNED_BYTE, s_kspr_decode_buf);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        m_kspr_atlas_map[sprite_id] = {layer, ks.SWidth};
        preloaded++;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    if (m_kspr_atlas_used > m_kspr_atlas_peak)
        m_kspr_atlas_peak = m_kspr_atlas_used;
    SYNCLOG("GLWorldViewRenderer: atlas preload complete -- %d sprites uploaded (%d/%d layers used)",
            preloaded, m_kspr_atlas_used, k_kspr_atlas_layers);
}

void GLWorldViewRenderer::ensure_clut_valid()
{
    if (!m_kspr_clut_tex) return;
    if (memcmp(m_rt_palette, m_kspr_clut_palette_snap, sizeof(m_rt_palette)) == 0) return;

    memcpy(m_kspr_clut_palette_snap, m_rt_palette, sizeof(m_rt_palette));

    // Rebuild identity CLUT row 0: palette[i] for all i. DK palette is 6-bit
    // (0-63); shift left 2 to get 8-bit. Index 0 = transparent (alpha 0).
    uint8_t row[256 * 4];
    for (int i = 0; i < 256; i++) {
        row[i*4+0] = (uint8_t)(m_rt_palette[i*3+0] << 2);
        row[i*4+1] = (uint8_t)(m_rt_palette[i*3+1] << 2);
        row[i*4+2] = (uint8_t)(m_rt_palette[i*3+2] << 2);
        row[i*4+3] = (i == 0) ? 0 : 255;
    }
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_kspr_clut_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, row);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);  // restore active unit

    // Invalidate all cached remap CLUTs -- they depend on the old palette values.
    m_kspr_clut_remaps.clear();
    m_kspr_clut_used = 1;

    SYNCDBG(6, "GLWorldViewRenderer: CLUT rebuilt (palette changed)");
}

int GLWorldViewRenderer::SubmitKeeperSprite(
    int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
    const unsigned char* data, int src_w, int src_h, int32_t content_h,
    unsigned int draw_flags, const unsigned char* remap,
    int32_t sprite_id)
{
    if (src_w <= 0 || src_h <= 0 || src_w > k_kspr_decode_dim || src_h > k_kspr_decode_dim) {
        static int s_dim = 0;
        if (s_dim++ < 20)
            WARNLOG("SubmitKeeperSprite: invalid sprite dimensions %dx%d (max %d) -- dropped",
                    src_w, src_h, k_kspr_decode_dim);
        return 1;
    }
    if (dst_w <= 0 || dst_h <= 0) return 1;
    // Clamp rather than trust the caller: a bad content_h must not produce
    // an inverted/oversized UV range or a negative destination height.
    if (content_h <= 0 || content_h > src_h) content_h = src_h;

    IRWorldKeeperSpriteCmd cmd;
    cmd.dst_x         = dst_x;
    cmd.dst_y         = dst_y;
    cmd.dst_w         = dst_w;
    cmd.dst_h         = dst_h;
    cmd.src_w         = src_w;
    cmd.src_h         = src_h;
    cmd.content_h     = content_h;
    cmd.draw_flags    = draw_flags;
    cmd.data          = data;
    cmd.remap_enabled = (remap != nullptr) ? 1 : 0;
    if (cmd.remap_enabled)
        memcpy(cmd.remap_table, remap, sizeof(cmd.remap_table));
    cmd.z_ndc         = m_current_sprite_z;
    // Beat 4: owner/outline now come from the ambient context
    // draw_jonty_mapwho()/draw_fastview_mapwho() set via
    // RendererSetCurrentSpriteContext() right before dispatching this
    // thing's sprite(s) (and reset to (-1,0) once done) -- read here, on the
    // game thread, at the exact same point the rest of cmd is built, so the
    // value baked into the IR is never subject to a later call's context
    // changing underneath it. Cursor sprites (m_capturing_cursor) always see
    // (-1,0) here too, since the last world thing's draw already reset the
    // context before cursor submission happens later in the frame -- correct,
    // cursor content has no depth-fail concept.
    cmd.owner         = (int8_t)RendererGetCurrentSpriteOwner();
    cmd.wants_outline = (int8_t)RendererGetCurrentSpriteWantsOutline();
    cmd.sprite_id     = sprite_id;
    cmd.sort_key      = m_current_sprite_sort_key;
    if (m_capturing_cursor)
        m_cursor_kspr_ir.push_back(cmd);
    else
        m_kspr_ir.push_back(cmd);
    return 1;
}

int GLWorldViewRenderer::resolve_atlas_layer(int32_t sprite_id, const unsigned char* data, int src_w, int src_h)
{
    if (!m_kspr_sprite_array || !m_kspr_atlas_shader || !m_kspr_clut_tex)
        return -1;
    if (sprite_id < 0)
        return -1;  // no stable identity -- cannot cache safely
    auto it = m_kspr_atlas_map.find(sprite_id);
    if (it != m_kspr_atlas_map.end())
    {
        m_kspr_atlas_hits++;
        return it->second.layer;
    }
    if (m_kspr_atlas_used >= k_kspr_atlas_layers)
        return -1;

    // Clear the full scratch buffer before decoding so rows beyond src_h
    // (uploaded at full k_kspr_decode_dim height) are transparent (index 0).
    memset(s_kspr_decode_buf, 0, sizeof(s_kspr_decode_buf));
    decode_keeper_rle(s_kspr_decode_buf, data, src_w, src_h);
    const int atlas_layer = m_kspr_atlas_used++;
    if (m_kspr_atlas_used > m_kspr_atlas_peak)
        m_kspr_atlas_peak = m_kspr_atlas_used;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_kspr_sprite_array);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, k_kspr_decode_dim);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0, 0, atlas_layer,
                    src_w, k_kspr_decode_dim, 1,
                    GL_RED, GL_UNSIGNED_BYTE, s_kspr_decode_buf);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    m_kspr_atlas_map[sprite_id] = {atlas_layer, src_w};
    m_kspr_atlas_misses++;
    return atlas_layer;
}

float GLWorldViewRenderer::resolve_clut_v(const unsigned char* remap)
{
    // Row 0 = identity (non-remapped sprites).
    float clut_v = 0.5f / (float)k_clut_rows;
    if (!remap || !m_kspr_clut_tex)
        return clut_v;
    for (size_t i = 0; i < m_kspr_clut_remaps.size(); ++i)
    {
        if (memcmp(m_kspr_clut_remaps[i].data(), remap, 256) == 0)
            return (float(i + 1) + 0.5f) / (float)k_clut_rows;
    }
    if (m_kspr_clut_used >= k_clut_rows)
        return clut_v;  // CLUT full: identity fallback (wrong colour, no crash)

    const int row_idx = m_kspr_clut_used++;
    uint8_t row_data[256 * 4];
    for (int ci = 0; ci < 256; ci++) {
        int ri = remap[ci];
        row_data[ci*4+0] = (uint8_t)(m_rt_palette[ri*3+0] << 2);
        row_data[ci*4+1] = (uint8_t)(m_rt_palette[ri*3+1] << 2);
        row_data[ci*4+2] = (uint8_t)(m_rt_palette[ri*3+2] << 2);
        row_data[ci*4+3] = (ci == 0) ? 0 : 255;
    }
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_kspr_clut_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, row_idx, 256, 1,
                    GL_RGBA, GL_UNSIGNED_BYTE, row_data);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    std::array<uint8_t, 256> remap_copy;
    memcpy(remap_copy.data(), remap, remap_copy.size());
    m_kspr_clut_remaps.push_back(remap_copy);
    return (float(row_idx) + 0.5f) / (float)k_clut_rows;
}

int GLWorldViewRenderer::BeginWorldSpriteCapture(int32_t bucket_idx)
{
    if (!UsesFillTimeWorldSubmit())
        return 0;
    if (bucket_idx >= BUCKETS_COUNT)
        bucket_idx = BUCKETS_COUNT - 1;
    else if (bucket_idx < 0)
        bucket_idx = 0;
    // Biased half a bucket closer to the camera so sprites always pass the
    // depth test against same-bucket ground polygons.
    m_current_sprite_z = 2.0f * ((float)bucket_idx - 0.5f) / (float)(BUCKETS_COUNT - 1) - 1.0f;
    m_current_sprite_sort_key = ((uint32_t)bucket_idx << 16)
                              | (m_sprite_entry_seq++ & 0xFFFFu);
    return 1;
}

void GLWorldViewRenderer::BeginCursorCapture()
{
    ASSERT_GAME_THREAD();
    m_capturing_cursor = true;
    // Nearest legal NDC depth -- GL_LEQUAL against the world pass's own
    // [-1,1] z_ndc range means cursor sprites are never occluded by world
    // geometry.
    m_current_sprite_z = -1.0f;
    m_current_sprite_sort_key = 0;
}

void GLWorldViewRenderer::EndCursorCapture()
{
    ASSERT_GAME_THREAD();
    m_capturing_cursor = false;
}

void GLWorldViewRenderer::append_keeper_sprite_instance(const IRWorldKeeperSpriteCmd& cmd)
{
    if (cmd.src_w <= 0 || cmd.src_h <= 0 ||
        cmd.src_w > k_kspr_decode_dim || cmd.src_h > k_kspr_decode_dim)
        return;
    if (cmd.dst_w <= 0 || cmd.dst_h <= 0)
        return;

    const bool additive = (cmd.draw_flags & Lb_SPRITE_ALPHA_ADDITIVE) != 0;
    const unsigned char* remap = cmd.remap_enabled ? cmd.remap_table : nullptr;
    const bool use_remap = remap && (cmd.draw_flags & Lb_TEXT_UNDERLNSHADOW) && !additive;

    const int layer = resolve_atlas_layer(cmd.sprite_id, cmd.data, cmd.src_w, cmd.src_h);
    if (layer < 0)
    {
        // Atlas full: flush what we have so painter's order holds, then draw
        // this one sprite through the legacy per-sprite path.
        flush_keeper_sprite_instances();
        DrawKeeperSpriteGL(cmd);
        return;
    }

    float clut_v = 0.5f / (float)k_clut_rows;
    if (use_remap)
        clut_v = resolve_clut_v(remap);

    // Beat 4: g_renderer_settings now exists (ported from develop) --
    // transpar4_alpha/transpar8_alpha are real config fields instead of
    // hardcoded literals.
    float alpha = 1.0f;
    if      (cmd.draw_flags & Lb_SPRITE_TRANSPAR4) alpha = g_renderer_settings.transpar4_alpha;
    else if (cmd.draw_flags & Lb_SPRITE_TRANSPAR8) alpha = g_renderer_settings.transpar8_alpha;

    // Water/lava clipping (Beat 4): content_h <= src_h means only the top
    // content_h rows of the decoded content are visible this draw. The quad
    // shrinks from the bottom (top-left stays anchored) and the UV V-range
    // shrinks by the same fraction, matching the CPU path's
    // LbSpriteDrawUsingScalingData()/DrawAlphaSpriteUsingScalingData()
    // behaviour of feeding fewer source rows into the same ambient scale
    // ratio (see IRWorldKeeperSpriteCmd::content_h's doc comment).
    const float clip_frac = (float)cmd.content_h / (float)cmd.src_h;

    KsprInstance inst;
    inst.rect[0]  = (float)cmd.dst_x;
    inst.rect[1]  = (float)cmd.dst_y;
    inst.rect[2]  = (float)cmd.dst_w;
    inst.rect[3]  = (float)cmd.dst_h * clip_frac;
    inst.uvext[0] = (float)cmd.src_w / (float)k_kspr_decode_dim;
    inst.uvext[1] = (float)cmd.content_h / (float)k_kspr_decode_dim;
    inst.layer    = (float)layer;
    inst.clut_v   = clut_v;
    inst.alpha    = alpha;
    inst.z_ndc    = cmd.z_ndc;
    inst.flags    = ((cmd.draw_flags & Lb_SPRITE_FLIP_HORIZ) ? 1u : 0u)
                  | (additive ? 2u : 0u);
    m_kspr_instances.push_back(inst);

    // Depth-fail outline pass (Beat 4).
    if (g_renderer_settings.creature_outline_mode != RENDERER_OUTLINE_NONE && !additive && cmd.wants_outline
        && (m_kspr_inst_outline_shader || m_kspr_inst_edge_shader))
    {
        // Resolve owner -> player colour index -> linear RGB from the palette.
        float oc_r = 0.9f, oc_g = 0.9f, oc_b = 0.9f;
        if (cmd.owner >= 0)
        {
            unsigned char color_idx = get_player_color_idx((PlayerNumber)cmd.owner);
            if (color_idx < 9)
            {
                uint8_t pal_idx = player_room_colours[color_idx];
                oc_r = (float)((int)m_rt_palette[pal_idx * 3 + 0] << 2) / 255.0f;
                oc_g = (float)((int)m_rt_palette[pal_idx * 3 + 1] << 2) / 255.0f;
                oc_b = (float)((int)m_rt_palette[pal_idx * 3 + 2] << 2) / 255.0f;
            }
        }
        KsprOutlineInstance o;
        memcpy(o.rect,  inst.rect,  sizeof(o.rect));
        memcpy(o.uvext, inst.uvext, sizeof(o.uvext));
        o.layer    = (float)layer;
        // Push the outline slightly farther from the camera so it only appears
        // when the sprite is meaningfully behind geometry, not at tile edges
        // where depth values are nearly equal (avoids stray corner pixels).
        o.z_ndc    = cmd.z_ndc + 0.002f;
        o.flip     = (cmd.draw_flags & Lb_SPRITE_FLIP_HORIZ) ? 1.0f : 0.0f;
        o.color[0] = oc_r;
        o.color[1] = oc_g;
        o.color[2] = oc_b;
        o.color[3] = g_renderer_settings.creature_outline_alpha;
        m_kspr_outline_instances.push_back(o);
    }
}

void GLWorldViewRenderer::flush_keeper_sprite_instances()
{
    if (m_kspr_instances.empty() && m_kspr_outline_instances.empty())
        return;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    // Premultiplied alpha: the shader outputs (rgb*a, a) for normal sprites
    // and (rgb, 0) for additive glow, so one blend mode covers both.
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_kspr_clut_tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_kspr_sprite_array);

    // Depth-fail outline pass first (Beat 4). Outline pixels only appear
    // where the sprite is occluded (GL_GREATER); main pixels only where it
    // is visible (GL_LEQUAL) -- disjoint regions, so pass order doesn't
    // matter. Select the silhouette or edge-detect shader based on outline mode.
    if (!m_kspr_outline_instances.empty())
    {
        const bool use_edge = (g_renderer_settings.creature_outline_mode == RENDERER_OUTLINE_EDGE);
        GLuint prog = use_edge ? m_kspr_inst_edge_shader    : m_kspr_inst_outline_shader;
        GLint  vloc = use_edge ? m_kspr_inst_edge_loc_viewport : m_kspr_inst_outline_loc_viewport;
        if (prog)
        {
            glDepthFunc(GL_GREATER);
            glUseProgram(prog);
            glUniform2f(vloc, (float)m_draw_screen_w, (float)m_draw_screen_h);
            glBindVertexArray(m_kspr_inst_outline_vao);
            glBindBuffer(GL_ARRAY_BUFFER, m_kspr_inst_outline_vbo);
            glBufferData(GL_ARRAY_BUFFER,
                         (GLsizeiptr)(m_kspr_outline_instances.size() * sizeof(KsprOutlineInstance)),
                         m_kspr_outline_instances.data(), GL_STREAM_DRAW);
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4,
                                  (GLsizei)m_kspr_outline_instances.size());
        }
    }

    if (!m_kspr_instances.empty())
    {
        glDepthFunc(GL_LEQUAL);
        glUseProgram(m_kspr_inst_shader);
        glUniform2f(m_kspr_inst_loc_viewport,
                    (float)m_draw_screen_w, (float)m_draw_screen_h);
        glBindVertexArray(m_kspr_inst_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_kspr_inst_vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     (GLsizeiptr)(m_kspr_instances.size() * sizeof(KsprInstance)),
                     m_kspr_instances.data(), GL_STREAM_DRAW);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)m_kspr_instances.size());
    }

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
    glUseProgram(0);

    m_kspr_instances.clear();
    m_kspr_outline_instances.clear();
}

int GLWorldViewRenderer::render_keepersprite_gpu(
    int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
    const unsigned char* data, int src_w, int src_h, int32_t content_h,
    unsigned int draw_flags, const unsigned char* remap,
    float z_ndc, int sprite_owner, int sprite_wants_outline, int32_t sprite_id)
{
    // GL resources must be ready before any sprite is submitted. If they
    // are not, this is a hard initialisation failure -- never fall back to
    // the CPU software rasteriser from here; that would produce mixed-path
    // frames (and there's no caller that would even understand a "fall
    // back to CPU" return value yet -- see the file header).
    if (!m_kspr_shader || !m_kspr_sprite_tex || m_palette_tex_handle == kInvalidGpuResource) {
        static int s_miss = 0;
        if (s_miss++ < 5)
            ERRORLOG("render_keepersprite_gpu: GL resources not ready -- sprite dropped");
        return 1;
    }
    if (src_w <= 0 || src_h <= 0 || src_w > k_kspr_decode_dim || src_h > k_kspr_decode_dim) {
        static int s_dim = 0;
        if (s_dim++ < 20)
            WARNLOG("render_keepersprite_gpu: invalid sprite dimensions %dx%d (max %d) -- dropped",
                    src_w, src_h, k_kspr_decode_dim);
        return 1;
    }
    if (dst_w <= 0 || dst_h <= 0) return 1;
    if (content_h <= 0 || content_h > src_h) content_h = src_h;

    const bool additive = (draw_flags & Lb_SPRITE_ALPHA_ADDITIVE) != 0;
    const bool use_remap = remap && (draw_flags & Lb_TEXT_UNDERLNSHADOW) && !additive;
    int atlas_layer = resolve_atlas_layer(sprite_id, data, src_w, src_h);

    float clut_v = 0.5f / (float)k_clut_rows;
    if (atlas_layer >= 0 && use_remap && m_kspr_clut_tex)
        clut_v = resolve_clut_v(remap);

    // Water/lava clipping (Beat 4) -- see IRWorldKeeperSpriteCmd::content_h's
    // doc comment. Shrinks both the UV V-range and the destination rect's
    // bottom edge by the same fraction; top-left stays anchored.
    const float clip_frac = (float)content_h / (float)src_h;

    float u1 = (float)src_w / (float)k_kspr_decode_dim;
    float v1 = (float)content_h / (float)k_kspr_decode_dim;
    float ul = 0.0f, ur = u1;
    if (draw_flags & Lb_SPRITE_FLIP_HORIZ) { ul = u1; ur = 0.0f; }

    float vx0 = (float)dst_x,          vy0 = (float)dst_y;
    float vx1 = (float)(dst_x + dst_w), vy1 = (float)dst_y + (float)dst_h * clip_frac;

    float sv[6][4] = {
        { vx0, vy0, ul,  0.0f },
        { vx1, vy0, ur,  0.0f },
        { vx1, vy1, ur,  v1   },
        { vx0, vy0, ul,  0.0f },
        { vx1, vy1, ur,  v1   },
        { vx0, vy1, ul,  v1   },
    };
    glBindBuffer(GL_ARRAY_BUFFER, m_kspr_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sv), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sv), sv);

    float alpha = 1.0f;
    if      (draw_flags & Lb_SPRITE_TRANSPAR4) alpha = g_renderer_settings.transpar4_alpha;
    else if (draw_flags & Lb_SPRITE_TRANSPAR8) alpha = g_renderer_settings.transpar8_alpha;

    // Additive glow (Beat 3): when the dedicated glow shader compiled, it
    // computes the real per-colour-family glow (same k_glow_step table the
    // instanced fragment shader already uses) from the sprite's own DK
    // glow-encoding pixel indices, drawn with GL_ONE/GL_ONE so the RGB delta
    // adds directly onto the framebuffer. If glow shader compilation failed
    // (m_kspr_glow_shader/m_kspr_atlas_glow_shader == 0), degrade to the
    // older flat-additive-blend approximation via the normal shader rather
    // than crash -- same "optional GL resource, don't require it" pattern
    // every other shader in this function follows.
    const bool use_glow_atlas    = additive && atlas_layer >= 0 && m_kspr_atlas_glow_shader;
    const bool use_glow_fallback = additive && atlas_layer <  0 && m_kspr_glow_shader;
    const GLenum blend_dfactor = additive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA;

    glBindVertexArray(m_kspr_vao);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    // ── Depth-fail outline pass (Beat 4) ──────────────────────────────────────
    // Draws a flat owner-colour silhouette only where the sprite is BEHIND
    // geometry (GL_GREATER depth test). The normal draw below then paints
    // over the visible portion with GL_LEQUAL, leaving the outline only
    // peeking out from behind walls/columns. Additive glow sprites are
    // excluded (no meaningful silhouette). The class-mask in
    // g_renderer_settings controls which entity types are outlined.
    const bool wants_outline = sprite_wants_outline != 0;
    if (g_renderer_settings.creature_outline_mode != RENDERER_OUTLINE_NONE && !additive && wants_outline)
    {
        // Resolve owner -> player colour index -> linear RGB from the palette.
        float oc_r = 0.9f, oc_g = 0.9f, oc_b = 0.9f;
        if (sprite_owner >= 0)
        {
            unsigned char color_idx = get_player_color_idx((PlayerNumber)sprite_owner);
            if (color_idx < 9)
            {
                uint8_t pal_idx = player_room_colours[color_idx];
                oc_r = (float)((int)m_rt_palette[pal_idx * 3 + 0] << 2) / 255.0f;
                oc_g = (float)((int)m_rt_palette[pal_idx * 3 + 1] << 2) / 255.0f;
                oc_b = (float)((int)m_rt_palette[pal_idx * 3 + 2] << 2) / 255.0f;
            }
        }
        const float oc_a = g_renderer_settings.creature_outline_alpha;

        glDepthFunc(GL_GREATER);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Push the outline slightly farther from the camera so it only appears
        // when the sprite is meaningfully behind geometry, not at tile edges
        // where depth values are nearly equal (avoids stray corner pixels).
        const float outline_z = z_ndc + 0.002f;

        const bool use_edge = (g_renderer_settings.creature_outline_mode == RENDERER_OUTLINE_EDGE);

        if (atlas_layer >= 0)
        {
            // Select edge vs silhouette shader for the atlas path.
            GLuint prog    = use_edge ? m_kspr_atlas_edge_shader         : m_kspr_atlas_outline_shader;
            GLint  loc_vp  = use_edge ? m_kspr_atlas_edge_loc_viewport   : m_kspr_atlas_outline_loc_viewport;
            GLint  loc_z   = use_edge ? m_kspr_atlas_edge_loc_z_ndc      : m_kspr_atlas_outline_loc_z_ndc;
            GLint  loc_lay = use_edge ? m_kspr_atlas_edge_loc_layer      : m_kspr_atlas_outline_loc_layer;
            GLint  loc_col = use_edge ? m_kspr_atlas_edge_loc_color      : m_kspr_atlas_outline_loc_color;
            if (prog)
            {
                // Atlas path: texture already cached in m_kspr_sprite_array.
                glUseProgram(prog);
                glUniform2f(loc_vp, (float)m_draw_screen_w, (float)m_draw_screen_h);
                glUniform1f(loc_z,    outline_z);
                glUniform1f(loc_lay,  (float)atlas_layer);
                glUniform4f(loc_col,  oc_r, oc_g, oc_b, oc_a);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D_ARRAY, m_kspr_sprite_array);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
        else
        {
            // Fallback path: decode + upload now so the outline has pixel data.
            // The normal draw below will re-use the already-uploaded texture.
            GLuint prog    = use_edge ? m_kspr_edge_shader         : m_kspr_outline_shader;
            GLint  loc_vp  = use_edge ? m_kspr_edge_loc_viewport   : m_kspr_outline_loc_viewport;
            GLint  loc_z   = use_edge ? m_kspr_edge_loc_z_ndc      : m_kspr_outline_loc_z_ndc;
            GLint  loc_col = use_edge ? m_kspr_edge_loc_color      : m_kspr_outline_loc_color;
            if (prog)
            {
                decode_keeper_rle(s_kspr_decode_buf, data, src_w, src_h);
                if (use_remap)
                {
                    for (int oy = 0; oy < src_h; ++oy) {
                        uint8_t* orow = s_kspr_decode_buf + oy * k_kspr_decode_dim;
                        for (int ox = 0; ox < src_w; ++ox)
                            if (orow[ox] != 0) orow[ox] = remap[orow[ox]];
                    }
                }
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_kspr_sprite_tex);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, k_kspr_decode_dim);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, src_w, src_h,
                                GL_RED, GL_UNSIGNED_BYTE, s_kspr_decode_buf);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

                glUseProgram(prog);
                glUniform2f(loc_vp, (float)m_draw_screen_w, (float)m_draw_screen_h);
                glUniform1f(loc_z,    outline_z);
                glUniform4f(loc_col,  oc_r, oc_g, oc_b, oc_a);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }

        glDepthFunc(GL_LEQUAL); // restore for normal draw below
    }

    if (atlas_layer >= 0) {
        if (use_glow_atlas) {
            // Additive glow via atlas: no CLUT/palette needed, glow math in shader.
            glUseProgram(m_kspr_atlas_glow_shader);
            glUniform2f(m_kspr_atlas_glow_loc_viewport, (float)m_draw_screen_w, (float)m_draw_screen_h);
            glUniform1f(m_kspr_atlas_glow_loc_z_ndc, z_ndc);
            glUniform1f(m_kspr_atlas_glow_loc_layer, (float)atlas_layer);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_kspr_sprite_array);
            glBlendFunc(GL_ONE, GL_ONE);
        } else {
            // Normal or remapped sprite via atlas + CLUT.
            glUseProgram(m_kspr_atlas_shader);
            glUniform2f(m_kspr_atlas_loc_viewport, (float)m_draw_screen_w, (float)m_draw_screen_h);
            glUniform1f(m_kspr_atlas_loc_alpha, alpha);
            glUniform1f(m_kspr_atlas_loc_z_ndc, z_ndc);
            glUniform1f(m_kspr_atlas_loc_layer, (float)atlas_layer);
            glUniform1f(m_kspr_atlas_loc_clut_v, clut_v);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_kspr_clut_tex);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_kspr_sprite_array);
            glBlendFunc(GL_SRC_ALPHA, blend_dfactor);
        }
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_BLEND);
        glBindVertexArray(0);
        glUseProgram(0);
        return 1;
    }

    // Atlas miss/full/unknown-id: decode into the reusable single texture.
    // If the outline block above already decoded + uploaded the sprite
    // (fallback-path outline), skip the redundant decode/upload here.
    const bool outline_uploaded = g_renderer_settings.creature_outline_mode != RENDERER_OUTLINE_NONE
                                   && !additive
                                   && wants_outline
                                   && atlas_layer < 0
                                   && (m_kspr_outline_shader || m_kspr_edge_shader);
    if (!outline_uploaded)
    {
        decode_keeper_rle(s_kspr_decode_buf, data, src_w, src_h);
        if (use_remap)
        {
            for (int y = 0; y < src_h; ++y) {
                uint8_t* row = s_kspr_decode_buf + y * k_kspr_decode_dim;
                for (int x = 0; x < src_w; ++x)
                    if (row[x] != 0) row[x] = remap[row[x]];
            }
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_kspr_sprite_tex);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, k_kspr_decode_dim);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, src_w, src_h,
                        GL_RED, GL_UNSIGNED_BYTE, s_kspr_decode_buf);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }

    if (use_glow_fallback)
    {
        // Pure additive blend: adds the glow RGB delta to the framebuffer.
        glUseProgram(m_kspr_glow_shader);
        glUniform2f(m_kspr_glow_loc_viewport, (float)m_draw_screen_w, (float)m_draw_screen_h);
        glUniform1f(m_kspr_glow_loc_z_ndc,    z_ndc);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_kspr_sprite_tex);
        glBlendFunc(GL_ONE, GL_ONE);
    }
    else
    {
        glUseProgram(m_kspr_shader);
        glUniform2f(m_kspr_loc_viewport, (float)m_draw_screen_w, (float)m_draw_screen_h);
        glUniform1f(m_kspr_loc_alpha,    alpha);
        glUniform1f(m_kspr_loc_z_ndc,    z_ndc);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ResolvePaletteTexId());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_kspr_sprite_tex);
        glBlendFunc(GL_SRC_ALPHA, blend_dfactor);
    }

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisable(GL_BLEND);
    glBindVertexArray(0);
    glUseProgram(0);
    return 1;
}

void GLWorldViewRenderer::DrawKeeperSpriteGL(const IRWorldKeeperSpriteCmd& cmd)
{
    ASSERT_RENDER_THREAD();
    const unsigned char* remap = cmd.remap_enabled ? cmd.remap_table : nullptr;
    render_keepersprite_gpu(cmd.dst_x, cmd.dst_y, cmd.dst_w, cmd.dst_h,
                            cmd.data, cmd.src_w, cmd.src_h, cmd.content_h, cmd.draw_flags, remap,
                            cmd.z_ndc, (int)cmd.owner, (int)cmd.wants_outline, cmd.sprite_id);
}

void GLWorldViewRenderer::DrawCursorKeeperSprites()
{
    ASSERT_RENDER_THREAD();
    if (m_rt_cursor_kspr_ir.empty())
        return;
    // Unlike the normal world-sprite pass (gpu_execute_passes(), depth-sorted
    // ranges via m_kspr_sorted_idx), cursor sprites need no sort: there are
    // at most a couple per frame (held thing + hand graphic), all pinned to
    // the same z_ndc=-1.0f, so submission order alone gives the right
    // painter's-order composite (e.g. held creature drawn, then the hand
    // graphic on top).
    for (const IRWorldKeeperSpriteCmd& cmd : m_rt_cursor_kspr_ir)
        append_keeper_sprite_instance(cmd);
    flush_keeper_sprite_instances();
}

void GLWorldViewRenderer::gpu_flush()
{
    if (!m_initialized || m_vert_count <= m_cmd_vert_start)
        return;

    // Record the current tile batch as a deferred draw command. No GL calls
    // are issued here — everything is replayed in GPURenderNow().
    DrawCmd cmd;
    cmd.type       = DrawCmd::CMD_TILES;
    cmd.vert_start = m_cmd_vert_start;
    cmd.vert_count = m_vert_count - m_cmd_vert_start;
    m_draw_cmds.push_back(cmd);
    m_cmd_vert_start = m_vert_count;
}

void GLWorldViewRenderer::FlipBuffers()
{
    ASSERT_GAME_THREAD();
    m_rt_draw_cmds      = std::move(m_draw_cmds);
    m_rt_kspr_ir        = std::move(m_kspr_ir);
    m_rt_cursor_kspr_ir = std::move(m_cursor_kspr_ir);
    m_rt_shadow_cmds    = std::move(m_shadow_cmds);
    m_rt_lens_cmd    = std::move(m_lens_cmd);
    // IRWorldLensCmd::active is a scalar -- std::move leaves it unchanged in
    // the moved-from object, unlike the vectors above which end up empty.
    // Reset explicitly so a frame with no SubmitPossessionLens() call (lens
    // no longer active) starts clean rather than replaying a stale command.
    m_lens_cmd = IRWorldLensCmd{};
    m_rt_screen_w  = m_screen_w;
    m_rt_screen_h  = m_screen_h;
    m_rt_vp_x      = m_vp_x;
    m_rt_vp_y      = m_vp_y;

    if (m_palette_data)
        memcpy(m_rt_palette, m_palette_data, sizeof(m_rt_palette));
    else
        memset(m_rt_palette, 0, sizeof(m_rt_palette));

    // Snapshot the lightmap so the render thread never reads the live game array.
    memcpy(m_rt_lightmap, game.lish.subtile_lightness, sizeof(m_rt_lightmap));

    m_vert_count     = 0;
    m_cmd_vert_start = 0;
    // m_kspr_ir is already empty after std::move above.
}

/******************************************************************************/

void GLWorldViewRenderer::GPURenderNow(const WorldCommandBuffers& cmds)
{
    ASSERT_RENDER_THREAD();

    m_world_pass_active = false;

    if (!m_initialized)
    {
        m_rt_draw_cmds.clear();
        m_cmd_vert_start = 0;
        return;
    }

    if (m_rt_draw_cmds.empty())
        return;

    const int vp_y_gl = m_full_screen_h - m_rt_vp_y - m_rt_screen_h;
    gpu_execute_passes(m_rt_vp_x, vp_y_gl, m_rt_screen_w, m_rt_screen_h,
                       cmds.tile_verts, cmds.flat_poly_verts);
}

void GLWorldViewRenderer::gpu_execute_passes(int vp_x, int vp_y_gl, int screen_w, int screen_h,
                                             const std::vector<WorldVertex>& tile_verts,
                                             const std::vector<FlatPolyVertex>& fp_verts)
{
    m_draw_screen_w = screen_w;
    m_draw_screen_h = screen_h;

    // Diagnostic counters, reset per pass -- moved here from BeginWorldPass()
    // (game thread) in P5.7.6: resolve_atlas_layer() (below, render thread)
    // increments these, so resetting them from the game thread raced it.
    m_kspr_atlas_hits   = 0;
    m_kspr_atlas_misses = 0;

    ensure_clut_valid();  // identity row 0 must exist before any sprite draws

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    (GLsizeiptr)(tile_verts.size() * sizeof(WorldVertex)),
                    tile_verts.data());

    glViewport(vp_x, vp_y_gl, screen_w, screen_h);
    glUseProgram(m_shader);
    glBindVertexArray(m_vao);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    bool atlas_bound       = false;  // tile atlas array bound to GL_TEXTURE0
    bool flatpoly_uploaded = false;  // flat-poly VBO uploaded on first CMD_FLAT_POLYS

    // Upload lightmap shadow copy (snapshotted from game.lish.subtile_lightness[]
    // during FlipBuffers() on the game thread — no live game struct access here).
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_tex_lightmap);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, k_lightmap_w, k_lightmap_h,
                    GL_RED_INTEGER, GL_UNSIGNED_SHORT,
                    m_rt_lightmap);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  // restore default

    // Bind fade table to unit 3 unconditionally — the sampler must always
    // reference a valid texture even when darkness_mode != PALETTE.
    if (m_fade_tex_handle != kInvalidGpuResource)
    {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ResolveFadeTexId());
    }

    glActiveTexture(GL_TEXTURE0);  // restore default active texture unit

    // Bind palette once for the entire pass — it never changes mid-frame.
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ResolvePaletteTexId());

    // Opaque geometry only (tiles, flat-colour polys) -- no shadow/sprite
    // passes in this slice.
    for (const auto& cmd : m_rt_draw_cmds)
    {
        if (cmd.type == DrawCmd::CMD_TILES)
        {
            if (!atlas_bound)
            {
                GLuint atlas_tex = 0;
                if (m_atlas && m_atlas->IsInitialized() && m_resource_mapper)
                {
                    const GLTexture* const tex = m_resource_mapper->ResolveTexture(m_atlas->GetAtlasTextureArray());
                    atlas_tex = tex ? tex->id : 0;
                }
                glActiveTexture(GL_TEXTURE0);
                // Unbind any GL_TEXTURE_2D that may linger on unit 0 -- having
                // both a TEXTURE_2D and TEXTURE_2D_ARRAY bound on the same
                // unit is undefined behaviour in core GL 3.3.
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindTexture(GL_TEXTURE_2D_ARRAY, atlas_tex);
                // When no valid atlas texture is bound the shader would silently
                // output black tiles (palette[0]). Set the diagnostic flag so the
                // fragment shader renders a magenta/black checkerboard instead.
                glUniform1f(m_loc_missing_tile, (atlas_tex == 0) ? 1.0f : 0.0f);
                atlas_bound = true;
                // Beat 4: actually branch on g_renderer_settings.tile_filter
                // instead of hardcoding GL_NEAREST -- m_tile_filter_applied
                // caches which mode is currently applied so an unchanged
                // setting doesn't re-issue glTexParameteri every flush.
                if (m_tile_filter_applied != g_renderer_settings.tile_filter)
                {
                    const GLint filter = (g_renderer_settings.tile_filter == RENDERER_FILTER_LINEAR)
                                        ? GL_LINEAR : GL_NEAREST;
                    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, filter);
                    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filter);
                    m_tile_filter_applied = g_renderer_settings.tile_filter;
                }
            }
            glDrawArrays(GL_TRIANGLES, cmd.vert_start, cmd.vert_count);
        }
        else if (cmd.type == DrawCmd::CMD_FLAT_POLYS)
        {
            if (!fp_verts.empty())
            {
                if (!flatpoly_uploaded)
                {
                    glBindBuffer(GL_ARRAY_BUFFER, m_flatpoly_vbo);
                    glBufferData(GL_ARRAY_BUFFER,
                                 (GLsizeiptr)(fp_verts.size() * sizeof(FlatPolyVertex)),
                                 fp_verts.data(), GL_STREAM_DRAW);
                    flatpoly_uploaded = true;
                }
                glUseProgram(m_flatpoly_shader);
                glBindVertexArray(m_flatpoly_vao);
                glUniform2f(m_flatpoly_loc_viewport, (float)screen_w, (float)screen_h);
                glDrawArrays(GL_TRIANGLES, cmd.vert_start, cmd.vert_count);
                // Restore tile shader state for subsequent CMD_TILES.
                glUseProgram(m_shader);
                glBindVertexArray(m_vao);
                atlas_bound = false;
            }
        }
        else if (cmd.type == DrawCmd::CMD_PRELOAD_KSPR_ATLAS)
        {
            execute_preload_atlas();
            // Restore tile shader and VAO that were active before the preload.
            glUseProgram(m_shader);
            glBindVertexArray(m_vao);
            atlas_bound = false;
        }
        else if (cmd.type == DrawCmd::CMD_CLEAR_KSPR_ATLAS)
        {
            // Pure CPU-side cache reset, no GL calls -- unlike preload, no
            // shader/VAO state to restore afterward.
            execute_clear_atlas();
        }
    }

    // ── Creature shadows (P5.7.4) ───────────────────────────────────────────────
    // Drawn on the ground before sprites, testing against the tile depth
    // buffer written above -- matches software rendering shadows before
    // creature bodies get walked in the same bucket-order pass. No sort key
    // needed between shadows (multiplicative blend is commutative) or
    // relative to tiles (each is independently depth-tested).
    draw_shadows_gpu();
    glUseProgram(m_shader);
    glBindVertexArray(m_vao);

    // ── Keeper sprites ────────────────────────────────────────────────────────
    // Sprites depth-test against the tile z-buffer written above (wall
    // occlusion). Back-to-front bucket order is preserved because
    // DrawIsometricView()/DrawFrontView() recorded CMD_IR_KEEPER_SPRITES
    // entries in bucket-walk order and each range is depth-sorted by
    // sort_key just before drawing (descending: far bucket first, LIFO
    // within a bucket, matching the legacy software bucket walk exactly).
    //
    // Scissor-clip to the world viewport so sprites can't bleed into the
    // sidebar region -- without it the GPU still rasterises (and
    // alpha-blends) sprites behind the sidebar UI.
    {
        bool any_sprites = false;
        for (const auto& cmd : m_rt_draw_cmds)
            if (cmd.type == DrawCmd::CMD_IR_KEEPER_SPRITES) { any_sprites = true; break; }

        if (any_sprites)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(vp_x, vp_y_gl, screen_w, screen_h);
            glBindVertexArray(0);
            glUseProgram(0);

            auto sort_range = [&](const DrawCmd& cmd) -> const std::vector<int>& {
                m_kspr_sorted_idx.clear();
                const int end = cmd.sprite_ir_start + cmd.sprite_ir_count;
                for (int i = cmd.sprite_ir_start; i < end; ++i)
                    m_kspr_sorted_idx.push_back(i);
                std::stable_sort(m_kspr_sorted_idx.begin(), m_kspr_sorted_idx.end(),
                                 [this](int a, int b) {
                                     return m_rt_kspr_ir[a].sort_key > m_rt_kspr_ir[b].sort_key;
                                 });
                return m_kspr_sorted_idx;
            };

            const bool use_instancing = (m_kspr_inst_shader != 0) && (m_kspr_sprite_array != 0);
            if (use_instancing)
            {
                m_kspr_instances.clear();
                for (const auto& cmd : m_rt_draw_cmds)
                {
                    if (cmd.type != DrawCmd::CMD_IR_KEEPER_SPRITES) continue;
                    for (int i : sort_range(cmd))
                        append_keeper_sprite_instance(m_rt_kspr_ir[i]);
                }
                flush_keeper_sprite_instances();
            }
            else
            {
                for (const auto& cmd : m_rt_draw_cmds)
                {
                    if (cmd.type != DrawCmd::CMD_IR_KEEPER_SPRITES) continue;
                    for (int i : sort_range(cmd))
                        DrawKeeperSpriteGL(m_rt_kspr_ir[i]);
                }
            }

            glDisable(GL_SCISSOR_TEST);
            glUseProgram(m_shader);
            glBindVertexArray(m_vao);
            // Sprite pass leaves unit 1 bound to m_kspr_clut_tex. The tile
            // shader samples unit 1 as the 256x1 palette -- restore it so a
            // subsequent CMD_TILES batch (front view after iso, etc.)
            // doesn't sample the wrong row of the CLUT.
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, ResolvePaletteTexId());
            glActiveTexture(GL_TEXTURE0);
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, m_full_screen_w, m_full_screen_h);

    m_rt_draw_cmds.clear();
    m_rt_kspr_ir.clear();
    m_rt_shadow_cmds.clear();
    m_cmd_vert_start = 0;
}

/******************************************************************************/

void GLWorldViewRenderer::DrawIsometricView()
{
    if (!m_initialized)
    {
        ERRORLOG("GLWorldViewRenderer asked to draw ISO but not initialised — frame dropped");
        return;
    }
    if (!m_world_write_cmds)
        return;

    // P5.7.3b: draw_keepersprite() (engine_render.c) falls through to the
    // CPU raster path whenever RendererSubmitKeeperSprite() returns 0 (atlas
    // full, unsupported dims). That fallback reads these same 3 ambient
    // globals display_drawlist() normally sets once per walk -- this walk
    // never calls display_drawlist(), so they'd otherwise be stale/unset.
    render_fade_tables = pixmap.fade_tables;
    render_ghost        = pixmap.ghost;
    render_alpha        = (unsigned char *)&alpha_sprite_table;

    std::vector<FlatPolyVertex>& fpverts = m_world_write_cmds->flat_poly_verts;

    // Walk the depth-sorted world draw list back-to-front (painter's
    // algorithm), mirroring display_drawlist() (engine_render.c) exactly --
    // same bucket range, same union-of-BucketKind* dispatch. Bucket 0 is
    // excluded, matching display_drawlist()'s own range (its comment notes
    // the software scan-converter can't handle near-plane vertices there;
    // unlike the reference, which includes it since GL clips natively --
    // kept matching THIS branch's existing behaviour rather than diverging
    // on an assumption, since nothing here has been visually verified yet).
    union {
        struct BasicQ *b;
        struct BucketKindPolygonStandard *polygonStandard;
        struct BucketKindPolyMode0 *polyMode0;
        struct BucketKindPolyMode4 *polyMode4;
        struct BucketKindTrigMode2 *trigMode2;
        struct BucketKindPolyMode5 *polyMode5;
        struct BucketKindTrigMode3 *trigMode3;
        struct BucketKindTrigMode6 *trigMode6;
        struct BucketKindBasicUnk10 *basicUnk10;
        struct BucketKindJontySprite *jontySprite;
        struct BucketKindCreatureShadow *creatureShadow;
        struct BucketKindRoomFlag *roomFlag;
        struct BucketKindFloatingGoldText *floatingGoldText;
        struct BucketKindCreatureStatus *creatureStatus;
    } item;
    struct PlayerInfo *player;
    struct Camera *cam;

    for (long bucket_num = BUCKETS_COUNT - 1; bucket_num > 0; bucket_num--)
    {
        m_current_bucket = (int)bucket_num;
        bool bucket_has_flat_polys = false;
        const int flatpoly_vert_start = (int)fpverts.size();

        for (item.b = buckets[bucket_num]; item.b != NULL; item.b = item.b->next)
        {
            switch (item.b->kind)
            {
            case QK_PolygonStandard: // All textured polygons for isometric and 'far' textures in 1st person view
                append_triangle(item.polygonStandard->block,
                                &item.polygonStandard->vertex_first,
                                &item.polygonStandard->vertex_second,
                                &item.polygonStandard->vertex_third);
                break;
            case QK_PolyMode0: // Flat-colour triangle
            {
                struct PolyPoint pm0_a = {}, pm0_b = {}, pm0_c = {};
                pm0_a.X = item.polyMode0->vertex_first_x;  pm0_a.Y = item.polyMode0->vertex_first_y;
                pm0_b.X = item.polyMode0->vertex_second_x; pm0_b.Y = item.polyMode0->vertex_second_y;
                pm0_c.X = item.polyMode0->vertex_third_x;  pm0_c.Y = item.polyMode0->vertex_third_y;
                append_flatpoly_triangle(item.polyMode0->colour, &pm0_a, &pm0_b, &pm0_c);
                bucket_has_flat_polys = true;
                break;
            }
            case QK_PolyMode4: // Flat-colour triangle
            {
                struct PolyPoint pm4_a = {}, pm4_b = {}, pm4_c = {};
                pm4_a.X = item.polyMode4->vertex_first_x;  pm4_a.Y = item.polyMode4->vertex_first_y;
                pm4_b.X = item.polyMode4->vertex_second_x; pm4_b.Y = item.polyMode4->vertex_second_y;
                pm4_c.X = item.polyMode4->vertex_third_x;  pm4_c.Y = item.polyMode4->vertex_third_y;
                append_flatpoly_triangle(item.polyMode4->colour, &pm4_a, &pm4_b, &pm4_c);
                bucket_has_flat_polys = true;
                break;
            }
            case QK_TrigMode2: // Compact textured triangle (no per-vertex shade -- full bright)
                append_triangle_compact(
                    item.trigMode2->vertex_first_x,  item.trigMode2->vertex_first_y,  item.trigMode2->texture_u_first,  item.trigMode2->texture_v_first,  255,
                    item.trigMode2->vertex_second_x, item.trigMode2->vertex_second_y, item.trigMode2->texture_u_second, item.trigMode2->texture_v_second, 255,
                    item.trigMode2->vertex_third_x,  item.trigMode2->vertex_third_y,  item.trigMode2->texture_u_third,  item.trigMode2->texture_v_third,  255);
                break;
            case QK_PolyMode5: // Compact textured triangle
                append_triangle_compact(
                    item.polyMode5->vertex_first_x,  item.polyMode5->vertex_first_y,  item.polyMode5->texture_u_first,  item.polyMode5->texture_v_first,  item.polyMode5->texture_w_first,
                    item.polyMode5->vertex_second_x, item.polyMode5->vertex_second_y, item.polyMode5->texture_u_second, item.polyMode5->texture_v_second, item.polyMode5->texture_w_second,
                    item.polyMode5->vertex_third_x,  item.polyMode5->vertex_third_y,  item.polyMode5->texture_u_third,  item.polyMode5->texture_v_third,  item.polyMode5->texture_w_third);
                break;
            case QK_TrigMode3: // Compact textured triangle (no per-vertex shade -- full bright)
                append_triangle_compact(
                    item.trigMode3->vertex_first_x,  item.trigMode3->vertex_first_y,  item.trigMode3->texture_u_first,  item.trigMode3->texture_v_first,  255,
                    item.trigMode3->vertex_second_x, item.trigMode3->vertex_second_y, item.trigMode3->texture_u_second, item.trigMode3->texture_v_second, 255,
                    item.trigMode3->vertex_third_x,  item.trigMode3->vertex_third_y,  item.trigMode3->texture_u_third,  item.trigMode3->texture_v_third,  255);
                break;
            case QK_TrigMode6: // Compact textured triangle
                append_triangle_compact(
                    item.trigMode6->vertex_first_x,  item.trigMode6->vertex_first_y,  item.trigMode6->texture_u_first,  item.trigMode6->texture_v_first,  item.trigMode6->texture_w_first,
                    item.trigMode6->vertex_second_x, item.trigMode6->vertex_second_y, item.trigMode6->texture_u_second, item.trigMode6->texture_v_second, item.trigMode6->texture_w_second,
                    item.trigMode6->vertex_third_x,  item.trigMode6->vertex_third_y,  item.trigMode6->texture_u_third,  item.trigMode6->texture_v_third,  item.trigMode6->texture_w_third);
                break;
            case QK_BasicPolygon: // Flat-colour triangle (real PolyPoint triple)
                append_flatpoly_triangle(item.basicUnk10->color_value,
                                         &item.basicUnk10->vertex_first,
                                         &item.basicUnk10->vertex_second,
                                         &item.basicUnk10->vertex_third);
                bucket_has_flat_polys = true;
                break;
            case QK_JontySprite: // All creatures and things (P5.7.3b) --
                // reuses the exact same walk-time resolution code software
                // uses (engine_render.c); only the final raster call inside
                // draw_keepersprite() diverts to the GPU when it can.
                draw_jonty_mapwho(item.jontySprite);
                break;
            case QK_JontyISOSprite: // Spinning key -- mirrors display_drawlist()'s own gate.
                player = get_my_player();
                cam = get_local_camera(get_player_active_camera(player));
                if (cam != NULL)
                {
                    if (cam->view_mode == PVM_IsoWibbleView || cam->view_mode == PVM_IsoStraightView) {
                        draw_jonty_mapwho(item.jontySprite);
                    }
                }
                break;
            case QK_CreatureShadow: // Creature shadows (P5.7.4) -- fully
                // resolved at fill time by create_shadows(); just needs the
                // atlas layer for the shadow's decoded silhouette, shared
                // with the keeper-sprite atlas (same cache, same sprite_id
                // convention a real sprite draw of that frame would use).
            {
                int32_t draw_idx;
                const unsigned char* data;
                int src_w, src_h;
                if (resolve_keepersprite_draw_data(item.creatureShadow->anim_sprite,
                        (short)item.creatureShadow->angle, item.creatureShadow->current_frame,
                        &draw_idx, &data, &src_w, &src_h, nullptr))
                {
                    append_shadow_quad(item.creatureShadow, draw_idx, data, src_w, src_h);
                }
                break;
            }
            case QK_RoomFlagBottomPole: // Room-flag pole -- was silently dropped under GL (Beat 1)
            {
                // Beat 5: bracket with the WorldOverlayFlat IR layer (ported
                // from develop's UIRenderer_BeginWorldOverlayFlat()/
                // EndWorldOverlayFlat()) so this composites through GLUIRenderer's
                // batched draw at the correct point in the frame (after map-fade,
                // before GameUI) instead of always landing in the GameUI layer.
                // Same half-bucket-biased NDC depth BeginWorldSpriteCapture() uses,
                // for consistent depth ordering against the world sprite pass.
                const float ndc_z = 2.0f * ((float)m_current_bucket - 0.5f) / (float)(BUCKETS_COUNT - 1) - 1.0f;
                RendererSetWorldOverlayFlat(ndc_z);
                draw_engine_room_flagpole(item.roomFlag);
                RendererClearWorldOverlayFlat();
                break;
            }
            case QK_RoomFlagStatusBox: // Room-flag top status box -- ditto
            {
                const float ndc_z = 2.0f * ((float)m_current_bucket - 0.5f) / (float)(BUCKETS_COUNT - 1) - 1.0f;
                RendererSetWorldOverlayFlat(ndc_z);
                draw_engine_room_flag_top(item.roomFlag);
                RendererClearWorldOverlayFlat();
                break;
            }
            case QK_FloatingGoldText: // Floating gold/damage/experience numbers -- ditto
            {
                const float ndc_z = 2.0f * ((float)m_current_bucket - 0.5f) / (float)(BUCKETS_COUNT - 1) - 1.0f;
                RendererSetWorldOverlayFlat(ndc_z);
                draw_engine_number(item.floatingGoldText);
                RendererClearWorldOverlayFlat();
                break;
            }
            case QK_CreatureStatus: // Health-flower/anger/state icon above a creature -- ditto.
            {
                // Beat 5: WorldOverlay (not Flat) -- this layer depth-tests
                // against world geometry, matching develop's own WorldOverlay/
                // WorldOverlayFlat split (creature status occludes behind walls
                // intentionally; room flags/floating text do not). Closes the
                // "no depth test against the world" gap this case's own comment
                // used to disclose.
                const float ndc_z = 2.0f * ((float)m_current_bucket - 0.5f) / (float)(BUCKETS_COUNT - 1) - 1.0f;
                RendererSetWorldOverlay(ndc_z);
                draw_status_sprites(item.creatureStatus->x, item.creatureStatus->y, item.creatureStatus->thing);
                RendererClearWorldOverlay();
                break;
            }
            // QK_PolygonSimple and QK_PolygonNearFP deliberately not handled
            // -- see the file-level comment. QK_RotableSprite is a documented
            // no-op even in the software path. QK_SlabSelector (the slab
            // digging/placement preview outline) is NOT handled here either --
            // display_drawlist()'s equivalent case calls draw_clipped_line(),
            // a direct lbDisplay.WScreen line-drawing primitive with no IR/
            // renderer-bridge equivalent yet (unlike the four cases above,
            // which already route through LbSpriteDrawScaled()/LbDrawBox()).
            // Disclosed gap, not silently dropped: needs a real line-drawing
            // IR command, out of scope for this pass.
            default:
                break;
            }
        }

        if (bucket_has_flat_polys)
        {
            gpu_flush();

            DrawCmd cmd;
            cmd.type       = DrawCmd::CMD_FLAT_POLYS;
            cmd.vert_start = flatpoly_vert_start;
            cmd.vert_count = (int)fpverts.size() - flatpoly_vert_start;
            m_draw_cmds.push_back(cmd);
        }
    }

    // Commit any trailing tile geometry not closed by a flat-poly interrupt
    // in the last bucket. Must happen before FlipBuffers() so m_draw_cmds
    // contains all CMD_TILES entries and m_vert_count == m_cmd_vert_start
    // (no open batch) at flip time.
    gpu_flush();

    // Record the whole-pass range of sprites captured this frame (via
    // BeginWorldSpriteCapture()/SubmitKeeperSprite(), called from
    // draw_jonty_mapwho() as this walk reaches each QK_JontySprite/
    // QK_JontyISOSprite entry above -- P5.7.3b).
    {
        const int ir_count = (int)(m_kspr_ir.size() - m_kspr_pass_start);
        if (ir_count > 0)
        {
            DrawCmd cmd;
            cmd.type            = DrawCmd::CMD_IR_KEEPER_SPRITES;
            cmd.sprite_ir_start = (int)m_kspr_pass_start;
            cmd.sprite_ir_count = ir_count;
            m_draw_cmds.push_back(cmd);
            m_kspr_pass_start = m_kspr_ir.size();
        }
    }
}

void GLWorldViewRenderer::DrawFrontView(struct Camera* cam)
{
    if (!m_initialized)
        return;
    if (!m_world_write_cmds)
        return;

    // See DrawIsometricView() -- same CPU-fallback ambient-global gap.
    render_fade_tables = pixmap.fade_tables;
    render_ghost        = pixmap.ghost;
    render_alpha        = (unsigned char *)&alpha_sprite_table;

    // Mirror display_fast_drawlist() (engine_render.c): tile geometry is
    // QK_TextureQuad -> draw_texturedquad_block(); QK_JontySprite/
    // QK_JontyISOSprite (P5.7.3b) reuse the same walk-time resolution code
    // software uses. Everything else in that switch is UI/decoration, out
    // of scope.
    for (long bucket_num = BUCKETS_COUNT - 1; bucket_num >= 0; bucket_num--)
    {
        m_current_bucket = (int)bucket_num;
        for (struct BasicQ* b = buckets[bucket_num]; b != NULL; b = b->next)
        {
            if (b->kind == QK_TextureQuad)
                append_frontview_quad((const struct BucketKindTexturedQuad*)b);
            else if (b->kind == QK_JontySprite)
                draw_fastview_mapwho(cam, (struct BucketKindJontySprite*)b);
            else if (b->kind == QK_JontyISOSprite)
                draw_iso_only_fastview_mapwho(cam, (struct BucketKindJontySprite*)b);
        }
    }

    gpu_flush(); // commit any trailing tile geometry

    // Record the whole-pass range of sprites captured this frame (see
    // DrawIsometricView() for details).
    {
        const int ir_count = (int)(m_kspr_ir.size() - m_kspr_pass_start);
        if (ir_count > 0)
        {
            DrawCmd cmd;
            cmd.type            = DrawCmd::CMD_IR_KEEPER_SPRITES;
            cmd.sprite_ir_start = (int)m_kspr_pass_start;
            cmd.sprite_ir_count = ir_count;
            m_draw_cmds.push_back(cmd);
            m_kspr_pass_start = m_kspr_ir.size();
        }
    }
}
