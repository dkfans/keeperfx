/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLWorldViewRenderer.cpp
 *     OpenGL world-geometry renderer.
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

GLuint GLWorldViewRenderer::ResolveShaderId(GpuResourceHandle handle) const
{
    const GLProgram* prog = m_resource_mapper ? m_resource_mapper->ResolveProgram(handle) : nullptr;
    return prog ? prog->id : 0;
}


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
    {
        GpuGeometryBufferDesc geom_desc;
        geom_desc.vertex_stride = (uint32_t)sizeof(WorldVertex);
        geom_desc.attribs = {
            // layout(location=0) vec3 a_pos  — x,y,z at byte offset 0
            { 0, 3, GpuVertexAttribType::Float, 0 },
            // layout(location=1) vec2 a_uv   — u,v at byte offset 12 (after x,y,z)
            { 1, 2, GpuVertexAttribType::Float, 3 * (uint32_t)sizeof(float) },
            // layout(location=2) float a_shade — shade at byte offset 20 (after x,y,z,u,v)
            { 2, 1, GpuVertexAttribType::Float, 5 * (uint32_t)sizeof(float) },
            // layout(location=3) vec2 a_stl — subtile coords at byte offset 24 (after x,y,z,u,v,shade)
            { 3, 2, GpuVertexAttribType::Float, 6 * (uint32_t)sizeof(float) },
            // layout(location=4) float a_camera_z — camera-space depth for perspective correction, byte offset 32
            { 4, 1, GpuVertexAttribType::Float, 8 * (uint32_t)sizeof(float) },
            // layout(location=5) float a_layer — texture array layer (atlas variation), byte offset 36
            { 5, 1, GpuVertexAttribType::Float, 9 * (uint32_t)sizeof(float) },
            // layout(location=6) vec3 aWorldPos — pre-projection world-space position
            { 6, 3, GpuVertexAttribType::Float, 10 * (uint32_t)sizeof(float) },
        };
        geom_desc.dynamic = true;
        geom_desc.initial_vertex_capacity = k_max_verts * sizeof(WorldVertex);
        geom_desc.debug_name = "world_geom";
        m_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(geom_desc);
        if (m_resource_mapper->ResolveGeometryBuffer(m_geom_handle) == nullptr)
        {
            ERRORLOG("GLWorldViewRenderer: world geometry buffer realization failed");
            return false;
        }
    }

    // Cache uniform locations and bind samplers to fixed texture units
    const GLuint world_shader_id = ResolveShaderId(m_shader_handle);
    glUseProgram(world_shader_id);
    m_loc_tile_atlas = glGetUniformLocation(world_shader_id, "u_tile_atlas");
    glUniform1i(m_loc_tile_atlas, 0);   // GL_TEXTURE0 — R8 palette-index atlas array
    m_loc_palette = glGetUniformLocation(world_shader_id, "u_palette");
    glUniform1i(m_loc_palette, 1);      // GL_TEXTURE1 — 1D RGBA8 palette
   
    m_loc_fullbright    = glGetUniformLocation(world_shader_id, "u_fullbright");
    m_loc_ambient       = glGetUniformLocation(world_shader_id, "u_ambient");
    m_loc_shade_scale   = glGetUniformLocation(world_shader_id, "u_shade_scale");
    m_loc_shade_gamma   = glGetUniformLocation(world_shader_id, "u_shade_gamma");
    m_loc_lighting_mode = glGetUniformLocation(world_shader_id, "u_lighting_mode");
    m_loc_darkness_mode = glGetUniformLocation(world_shader_id, "u_darkness_mode");
    m_loc_fade_table    = glGetUniformLocation(world_shader_id, "u_fade_table");
    m_loc_time          = glGetUniformLocation(world_shader_id, "u_time");
    m_loc_fog_speed     = glGetUniformLocation(world_shader_id, "u_fog_speed");
    m_loc_fog_density   = glGetUniformLocation(world_shader_id, "u_fog_density");
    m_loc_lightmap      = glGetUniformLocation(world_shader_id, "u_lightmap");
    m_loc_missing_tile  = glGetUniformLocation(world_shader_id, "u_missing_tile");
    m_loc_tile_filter   = glGetUniformLocation(world_shader_id, "u_tile_filter");
    
    glUniform1i(m_loc_fade_table,    3);  // GL_TEXTURE3
    glUniform1f(m_loc_time,          0.0f);
    glUniform1i(m_loc_lightmap,      2);  // GL_TEXTURE2
    glUniform1f(m_loc_missing_tile,  0.0f);
    glUseProgram(0);
    push_shade_uniforms();

    // Lightmap texture — mirrors game.lish.subtile_lightness[] each frame.
    // GL_R16UI stores the raw 0..16128 lightness values; the shader normalises them.
    {
        GpuTextureDesc desc;
        desc.width = MAX_SUBTILES_X;
        desc.height = MAX_SUBTILES_Y;
        desc.format = GpuTextureFormat::R16UI;
        desc.min_filter = GpuTextureFilter::Nearest;
        desc.mag_filter = GpuTextureFilter::Nearest;
        desc.wrap = GpuTextureWrap::Clamp;
        desc.debug_name = "world_lightmap";
        m_lightmap_tex_handle = m_resource_mapper->RequestCreateTexture(desc);
        if (m_resource_mapper->ResolveTexture(m_lightmap_tex_handle) == nullptr)
        {
            ERRORLOG("GLWorldViewRenderer: lightmap texture realization failed");
            return false;
        }
    }

    if (!init_keeper_sprite_shader())
    {
        ERRORLOG("GLWorldViewRenderer: failed to initialise keeper-sprite shader");
        free_gl_resources();
        return false;
    }

    if (!init_shadow_shader())
    {
        WARNLOG("GLWorldViewRenderer: shadow shader unavailable -- creature shadows disabled");
    }

    if (!init_lens_shaders())
    {
        WARNLOG("GLWorldViewRenderer: lens shaders unavailable -- possession lens distortion disabled");
    }

    m_initialized = true;
    SYNCLOG("GLWorldViewRenderer: initialised (VBO %d verts x %u bytes)",
            k_max_verts, (unsigned)sizeof(WorldVertex));
    return true;
}

/** Pushes the settings-derived subset of the world shader's uniforms from
 *  the current g_renderer_settings. No-op if the world shader hasn't
 *  compiled yet (uniform locations still -1, harmless glUniform on an
 *  unbound program otherwise). */
void GLWorldViewRenderer::push_shade_uniforms()
{
    const GLuint world_shader_id = ResolveShaderId(m_shader_handle);
    if (world_shader_id == 0)
        return;
    glUseProgram(world_shader_id);
    glUniform1f(m_loc_fullbright,    g_renderer_settings.shade_fullbright);
    glUniform1f(m_loc_ambient,       g_renderer_settings.shade_ambient);
    glUniform1f(m_loc_shade_scale,   g_renderer_settings.shade_scale);
    glUniform1f(m_loc_shade_gamma,   g_renderer_settings.shade_gamma);
    glUniform1i(m_loc_lighting_mode, g_renderer_settings.lighting_mode);
    glUniform1i(m_loc_darkness_mode, g_renderer_settings.darkness_mode);
    glUniform1f(m_loc_fog_speed,     g_renderer_settings.fog_speed);
    glUniform1f(m_loc_fog_density,   g_renderer_settings.fog_density);
    glUniform1i(m_loc_tile_filter,   g_renderer_settings.tile_filter);
    m_tile_filter_applied = -1;  // force reapply on next flush
    glUseProgram(0);
}

void GLWorldViewRenderer::RefreshFadeTableAndSettings()
{
    if (m_resource_mapper != nullptr)
    {
        const GLTexture* tex = m_resource_mapper->ResolveTexture(m_fade_tex_handle);
        if (tex != nullptr)
        {
            glBindTexture(GL_TEXTURE_2D, tex->id);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 64, GL_RED, GL_UNSIGNED_BYTE, pixmap.fade_tables);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    push_shade_uniforms();
}

void GLWorldViewRenderer::free_gl_resources()
{
    free_keeper_sprite_resources();
    free_shadow_resources();
    free_lens_resources();

    m_initialized = false;
}

void GLWorldViewRenderer::free_keeper_sprite_resources()
{
    m_kspr_atlas_used = 0;
    m_kspr_atlas_map.clear();
    m_kspr_clut_remaps.clear();
    m_kspr_clut_used = 1;
    memset(m_kspr_clut_palette_snap, 0, sizeof(m_kspr_clut_palette_snap));
    m_kspr_instances.clear();
    m_kspr_outline_instances.clear();
}

bool GLWorldViewRenderer::compile_world_shaders()
{
    if (!m_resource_mapper) return false;
    GpuProgramDesc desc;
    desc.vertex_src = WORLD_VERTEX_SHADER;
    desc.fragment_src = WORLD_FRAGMENT_SHADER;
    desc.debug_name = "world";
    m_shader_handle = m_resource_mapper->RequestCreateProgram(desc);
    return m_resource_mapper->ResolveProgram(m_shader_handle) != nullptr;
}

/******************************************************************************/
// Creature shadows 

namespace {
// GL-side vertex layout for the shadow pass: 7 floats/vertex
// (x, y, u, v, layer, z_ndc, darken). Only used within this section.
struct ShadowGLVertex { float x, y, u, v, layer, z_ndc, darken; };

std::vector<ShadowGLVertex> s_shadow_verts;
} // namespace

bool GLWorldViewRenderer::init_shadow_shader()
{
    if (!m_resource_mapper) return false;

    GpuProgramDesc prog_desc;
    prog_desc.vertex_src = SHADOW_VERTEX_SHADER;
    prog_desc.fragment_src = SHADOW_FRAGMENT_SHADER;
    prog_desc.debug_name = "shadow";
    m_shadow_shader_handle = m_resource_mapper->RequestCreateProgram(prog_desc);

    const GLProgram* prog = m_resource_mapper->ResolveProgram(m_shadow_shader_handle);
    if (!prog) return false;

    glUseProgram(prog->id);
    m_shadow_loc_viewport = glGetUniformLocation(prog->id, "u_viewport");
    m_shadow_loc_sprite   = glGetUniformLocation(prog->id, "u_sprite");
    glUniform1i(m_shadow_loc_sprite, 0);  // GL_TEXTURE0
    glUseProgram(0);

    // VAO + dynamic VBO: 7 floats/vertex (x, y, u, v, layer, z_ndc, darken)
    GpuGeometryBufferDesc geom_desc;
    geom_desc.vertex_stride = (uint32_t)sizeof(ShadowGLVertex);
    geom_desc.attribs = {
        { 0, 2, GpuVertexAttribType::Float, 0 },
        { 1, 2, GpuVertexAttribType::Float, 2 * (uint32_t)sizeof(float) },
        { 2, 1, GpuVertexAttribType::Float, 4 * (uint32_t)sizeof(float) },
        { 3, 1, GpuVertexAttribType::Float, 5 * (uint32_t)sizeof(float) },
        { 4, 1, GpuVertexAttribType::Float, 6 * (uint32_t)sizeof(float) },
    };
    geom_desc.dynamic = true;
    geom_desc.debug_name = "shadow_geom";
    m_shadow_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(geom_desc);

    return m_resource_mapper->ResolveGeometryBuffer(m_shadow_geom_handle) != nullptr;
}

void GLWorldViewRenderer::free_shadow_resources()
{
    m_shadow_cmds.clear();
    m_rt_shadow_cmds.clear();
}

/******************************************************************************/
// Possession lens
/******************************************************************************/

bool GLWorldViewRenderer::init_lens_shaders()
{
    if (!m_resource_mapper) return false;

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
    {
        GpuGeometryBufferDesc geom_desc;
        geom_desc.vertex_stride = 4 * (uint32_t)sizeof(float);
        geom_desc.attribs = {
            { 0, 2, GpuVertexAttribType::Float, 0 },
            { 1, 2, GpuVertexAttribType::Float, 2 * (uint32_t)sizeof(float) },
        };
        geom_desc.dynamic = false; // static unit quad, content never changes
        geom_desc.initial_vertex_capacity = sizeof(k_quad);
        geom_desc.debug_name = "lens_quad";
        m_lens_quad_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(geom_desc);
        const GLGeometryBuffer* geom = m_resource_mapper->ResolveGeometryBuffer(m_lens_quad_geom_handle);
        if (!geom) return false;
        glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(k_quad), k_quad);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    bool any_ok = false;

    // Mist
    {
        GpuProgramDesc desc;
        desc.vertex_src = LENS_COMPOSITE_VERTEX_SHADER;
        desc.fragment_src = LENS_MIST_FRAGMENT_SHADER;
        desc.debug_name = "lens_mist";
        m_lens_shader_mist_handle = m_resource_mapper->RequestCreateProgram(desc);
        const GLProgram* prog = m_resource_mapper->ResolveProgram(m_lens_shader_mist_handle);
        if (prog)
        {
            glUseProgram(prog->id);
            glUniform1i(glGetUniformLocation(prog->id, "u_scene"), 0);
            glUniform1i(glGetUniformLocation(prog->id, "u_mist"), 1);
            m_lens_mist_loc_src_off   = glGetUniformLocation(prog->id, "u_src_off");
            m_lens_mist_loc_src_scale = glGetUniformLocation(prog->id, "u_src_scale");
            m_lens_mist_loc_pos       = glGetUniformLocation(prog->id, "u_pos");
            m_lens_mist_loc_sec       = glGetUniformLocation(prog->id, "u_sec");
            m_lens_mist_loc_lightness = glGetUniformLocation(prog->id, "u_lightness");
            glUseProgram(0);
            any_ok = true;
        }
        else
        {
            WARNLOG("GLWorldViewRenderer: lens mist shader unavailable");
        }
    }

    // Remap (displacement/flyeye -- same table layout, same shader)
    {
        GpuProgramDesc desc;
        desc.vertex_src = LENS_COMPOSITE_VERTEX_SHADER;
        desc.fragment_src = LENS_REMAP_FRAGMENT_SHADER;
        desc.debug_name = "lens_remap";
        m_lens_shader_remap_handle = m_resource_mapper->RequestCreateProgram(desc);
        const GLProgram* prog = m_resource_mapper->ResolveProgram(m_lens_shader_remap_handle);
        if (prog)
        {
            glUseProgram(prog->id);
            glUniform1i(glGetUniformLocation(prog->id, "u_scene"), 0);
            glUniform1i(glGetUniformLocation(prog->id, "u_remap"), 1);
            m_lens_remap_loc_src_off  = glGetUniformLocation(prog->id, "u_src_off");
            m_lens_remap_loc_tex_size = glGetUniformLocation(prog->id, "u_tex_size");
            glUseProgram(0);
            any_ok = true;
        }
        else
        {
            WARNLOG("GLWorldViewRenderer: lens remap shader unavailable");
        }
    }

    // Overlay
    {
        GpuProgramDesc desc;
        desc.vertex_src = LENS_COMPOSITE_VERTEX_SHADER;
        desc.fragment_src = LENS_OVERLAY_FRAGMENT_SHADER;
        desc.debug_name = "lens_overlay";
        m_lens_shader_overlay_handle = m_resource_mapper->RequestCreateProgram(desc);
        const GLProgram* prog = m_resource_mapper->ResolveProgram(m_lens_shader_overlay_handle);
        if (prog)
        {
            glUseProgram(prog->id);
            glUniform1i(glGetUniformLocation(prog->id, "u_scene"), 0);
            glUniform1i(glGetUniformLocation(prog->id, "u_overlay"), 1);
            glUniform1i(glGetUniformLocation(prog->id, "u_palette"), 2);
            m_lens_overlay_loc_src_off   = glGetUniformLocation(prog->id, "u_src_off");
            m_lens_overlay_loc_src_scale = glGetUniformLocation(prog->id, "u_src_scale");
            m_lens_overlay_loc_alpha     = glGetUniformLocation(prog->id, "u_alpha");
            glUseProgram(0);
            any_ok = true;
        }
        else
        {
            WARNLOG("GLWorldViewRenderer: lens overlay shader unavailable");
        }
    }

    return any_ok;
}

void GLWorldViewRenderer::free_lens_resources()
{
    m_lens_mist_uploaded_version = m_lens_remap_uploaded_version = m_lens_overlay_uploaded_version = 0xFFFFFFFFu;
    m_lens_scene_gt_w = m_lens_scene_gt_h = 0;
    m_lens_remap_gt_w = m_lens_remap_gt_h = 0;
    m_lens_overlay_gt_w = m_lens_overlay_gt_h = 0;

    m_lens_cmd    = IRWorldLensCmd{};
    m_rt_lens_cmd = IRWorldLensCmd{};
}

void GLWorldViewRenderer::SubmitPossessionLens(const IRWorldLensCmd& cmd)
{
    ASSERT_GAME_THREAD();
    m_lens_cmd = cmd;

    EnsureLensSceneRT(m_screen_w, m_screen_h);

    if (cmd.type == LensPixelEffectType::Displacement || cmd.type == LensPixelEffectType::Flyeye)
        EnsureLensRemapTexture(cmd.remap_w, cmd.remap_h);
    else if (cmd.type == LensPixelEffectType::Overlay)
        EnsureLensOverlayTexture(cmd.overlay_w, cmd.overlay_h);
}

void GLWorldViewRenderer::EnsureLensSceneRT(int w, int h)
{
    if (!m_resource_mapper || w <= 0 || h <= 0)
        return;
    if (m_lens_scene_rt_handle != kInvalidGpuResource && m_lens_scene_gt_w == w && m_lens_scene_gt_h == h)
        return;

    GpuRenderTargetDesc desc;
    desc.width = w;
    desc.height = h;
    // Depth24Stencil8, not the original's depth-only GL_DEPTH_COMPONENT24
    // renderbuffer -- GpuTextureFormat has no depth-only option and
    // RealizeRenderTarget only supports texture attachments (not
    // renderbuffers). Functionally equivalent for this use (nothing reads
    // the depth buffer as a texture; GL_DEPTH_TEST is all that's needed),
    // just an unused stencil channel.
    desc.attachments = {
        { GpuTextureFormat::RGBA8, false },
        { GpuTextureFormat::Depth24Stencil8, true },
    };
    desc.debug_name = "lens_scene";

    if (m_lens_scene_rt_handle == kInvalidGpuResource)
        m_lens_scene_rt_handle = m_resource_mapper->RequestCreateRenderTarget(desc);
    else
        m_lens_scene_rt_handle = m_resource_mapper->RequestReloadRenderTarget(m_lens_scene_rt_handle, desc);

    m_lens_scene_gt_w = w;
    m_lens_scene_gt_h = h;
}

void GLWorldViewRenderer::EnsureLensRemapTexture(int w, int h)
{
    if (!m_resource_mapper || w <= 0 || h <= 0)
        return;
    if (m_lens_remap_tex_handle != kInvalidGpuResource && m_lens_remap_gt_w == w && m_lens_remap_gt_h == h)
        return;

    GpuTextureDesc desc;
    desc.width = w;
    desc.height = h;
    desc.format = GpuTextureFormat::RG16UI;
    desc.min_filter = GpuTextureFilter::Nearest;
    desc.mag_filter = GpuTextureFilter::Nearest;
    desc.wrap = GpuTextureWrap::Clamp;
    desc.debug_name = "lens_remap";

    if (m_lens_remap_tex_handle == kInvalidGpuResource)
        m_lens_remap_tex_handle = m_resource_mapper->RequestCreateTexture(desc);
    else
        m_lens_remap_tex_handle = m_resource_mapper->RequestReloadTexture(m_lens_remap_tex_handle, desc);

    m_lens_remap_gt_w = w;
    m_lens_remap_gt_h = h;
}

void GLWorldViewRenderer::EnsureLensOverlayTexture(int w, int h)
{
    if (!m_resource_mapper || w <= 0 || h <= 0)
        return;
    if (m_lens_overlay_tex_handle != kInvalidGpuResource && m_lens_overlay_gt_w == w && m_lens_overlay_gt_h == h)
        return;

    GpuTextureDesc desc;
    desc.width = w;
    desc.height = h;
    desc.format = GpuTextureFormat::R8;
    // GL_NEAREST matches COverlayRenderer::Render()'s own "nearest-neighbor
    // sampling" comment.
    desc.min_filter = GpuTextureFilter::Nearest;
    desc.mag_filter = GpuTextureFilter::Nearest;
    desc.wrap = GpuTextureWrap::Clamp;
    desc.debug_name = "lens_overlay";

    if (m_lens_overlay_tex_handle == kInvalidGpuResource)
        m_lens_overlay_tex_handle = m_resource_mapper->RequestCreateTexture(desc);
    else
        m_lens_overlay_tex_handle = m_resource_mapper->RequestReloadTexture(m_lens_overlay_tex_handle, desc);

    m_lens_overlay_gt_w = w;
    m_lens_overlay_gt_h = h;
}

void GLWorldViewRenderer::upload_lens_textures_if_dirty()
{
    if (!m_resource_mapper) return;

    switch (m_rt_lens_cmd.type)
    {
    case LensPixelEffectType::Mist:
        if (m_rt_lens_cmd.mist_pixels.size() != 256 * 256)
            break;
        if (m_lens_mist_tex_handle == kInvalidGpuResource)
        {
            GpuTextureDesc desc;
            desc.width = 256;
            desc.height = 256;
            desc.format = GpuTextureFormat::R8;
            desc.min_filter = GpuTextureFilter::Nearest;
            desc.mag_filter = GpuTextureFilter::Nearest;
            desc.wrap = GpuTextureWrap::Clamp;
            desc.debug_name = "lens_mist";
            m_lens_mist_tex_handle = m_resource_mapper->RequestCreateTexture(desc);
            m_lens_mist_uploaded_version = 0xFFFFFFFFu;
        }
        if (m_rt_lens_cmd.mist_version != m_lens_mist_uploaded_version)
        {
            const GLTexture* tex = m_resource_mapper->ResolveTexture(m_lens_mist_tex_handle);
            if (tex)
            {
                glBindTexture(GL_TEXTURE_2D, tex->id);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RED, GL_UNSIGNED_BYTE, m_rt_lens_cmd.mist_pixels.data());
                glBindTexture(GL_TEXTURE_2D, 0);
                m_lens_mist_uploaded_version = m_rt_lens_cmd.mist_version;
            }
        }
        break;

    case LensPixelEffectType::Displacement:
    case LensPixelEffectType::Flyeye:
        if (m_rt_lens_cmd.remap_w <= 0 || m_rt_lens_cmd.remap_h <= 0 ||
            (size_t)(m_rt_lens_cmd.remap_w * m_rt_lens_cmd.remap_h * 2) != m_rt_lens_cmd.remap_pixels.size())
            break;
        if (m_rt_lens_cmd.remap_version != m_lens_remap_uploaded_version)
        {
            const GLTexture* tex = m_resource_mapper->ResolveTexture(m_lens_remap_tex_handle);
            if (tex)
            {
                glBindTexture(GL_TEXTURE_2D, tex->id);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_rt_lens_cmd.remap_w, m_rt_lens_cmd.remap_h,
                                GL_RG_INTEGER, GL_UNSIGNED_SHORT, m_rt_lens_cmd.remap_pixels.data());
                glBindTexture(GL_TEXTURE_2D, 0);
                m_lens_remap_uploaded_version = m_rt_lens_cmd.remap_version;
            }
        }
        break;

    case LensPixelEffectType::Overlay:
        // Sizing already settled on the game thread (EnsureLensOverlayTexture(),
        // called from SubmitPossessionLens()) -- this only uploads content.
        if (m_rt_lens_cmd.overlay_w <= 0 || m_rt_lens_cmd.overlay_h <= 0 ||
            (size_t)(m_rt_lens_cmd.overlay_w * m_rt_lens_cmd.overlay_h) != m_rt_lens_cmd.overlay_pixels.size())
            break;
        if (m_rt_lens_cmd.overlay_version != m_lens_overlay_uploaded_version)
        {
            const GLTexture* tex = m_resource_mapper->ResolveTexture(m_lens_overlay_tex_handle);
            if (tex)
            {
                glBindTexture(GL_TEXTURE_2D, tex->id);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_rt_lens_cmd.overlay_w, m_rt_lens_cmd.overlay_h,
                                GL_RED, GL_UNSIGNED_BYTE, m_rt_lens_cmd.overlay_pixels.data());
                glBindTexture(GL_TEXTURE_2D, 0);
                m_lens_overlay_uploaded_version = m_rt_lens_cmd.overlay_version;
            }
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
    if (!m_resource_mapper)
        return false;

    GpuResourceHandle shader_handle = kInvalidGpuResource;
    switch (m_rt_lens_cmd.type)
    {
    case LensPixelEffectType::Mist:         shader_handle = m_lens_shader_mist_handle;    break;
    case LensPixelEffectType::Displacement:
    case LensPixelEffectType::Flyeye:       shader_handle = m_lens_shader_remap_handle;   break;
    case LensPixelEffectType::Overlay:      shader_handle = m_lens_shader_overlay_handle; break;
    default: break;
    }
    if (ResolveShaderId(shader_handle) == 0)
        return false;

    const GLRenderTarget* rt = m_resource_mapper->ResolveRenderTarget(m_lens_scene_rt_handle);
    if (!rt)
        return false;

    glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return true;
}

void GLWorldViewRenderer::ResolveLensComposite()
{
    ASSERT_RENDER_THREAD();
    if (!m_resource_mapper) return;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    upload_lens_textures_if_dirty();

    const GLRenderTarget* scene_rt = m_resource_mapper->ResolveRenderTarget(m_lens_scene_rt_handle);
    const GLGeometryBuffer* quad_geom = m_resource_mapper->ResolveGeometryBuffer(m_lens_quad_geom_handle);
    if (!scene_rt || scene_rt->color_attachments.empty() || !quad_geom)
        return;

    const float tex_w = (float)scene_rt->width;
    const float tex_h = (float)scene_rt->height;
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

    glBindVertexArray(quad_geom->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_rt->color_attachments[0]);

    switch (m_rt_lens_cmd.type)
    {
    case LensPixelEffectType::Mist:
    {
        const GLProgram* prog = m_resource_mapper->ResolveProgram(m_lens_shader_mist_handle);
        const GLTexture* mist_tex = m_resource_mapper->ResolveTexture(m_lens_mist_tex_handle);
        if (prog && mist_tex)
        {
            glUseProgram(prog->id);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, mist_tex->id);
            glUniform2f(m_lens_mist_loc_src_off, src_off_x, 0.0f);
            glUniform2f(m_lens_mist_loc_src_scale, src_scale_x, src_scale_y);
            glUniform2f(m_lens_mist_loc_pos, m_rt_lens_cmd.mist_pos_x, m_rt_lens_cmd.mist_pos_y);
            glUniform2f(m_lens_mist_loc_sec, m_rt_lens_cmd.mist_sec_x, m_rt_lens_cmd.mist_sec_y);
            glUniform1f(m_lens_mist_loc_lightness, (float)m_rt_lens_cmd.mist_lightness);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        break;
    }

    case LensPixelEffectType::Displacement:
    case LensPixelEffectType::Flyeye:
    {
        const GLProgram* prog = m_resource_mapper->ResolveProgram(m_lens_shader_remap_handle);
        const GLTexture* remap_tex = m_resource_mapper->ResolveTexture(m_lens_remap_tex_handle);
        if (prog && remap_tex)
        {
            glUseProgram(prog->id);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, remap_tex->id);
            glUniform2f(m_lens_remap_loc_src_off, src_off_x, 0.0f);
            glUniform2f(m_lens_remap_loc_tex_size, tex_w, tex_h);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        break;
    }

    case LensPixelEffectType::Overlay:
    {
        const GLProgram* prog = m_resource_mapper->ResolveProgram(m_lens_shader_overlay_handle);
        const GLTexture* overlay_tex = m_resource_mapper->ResolveTexture(m_lens_overlay_tex_handle);
        if (prog && overlay_tex)
        {
            glUseProgram(prog->id);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, overlay_tex->id);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, ResolvePaletteTexId());
            glUniform2f(m_lens_overlay_loc_src_off, src_off_x, 0.0f);
            glUniform2f(m_lens_overlay_loc_src_scale, src_scale_x, src_scale_y);
            glUniform1f(m_lens_overlay_loc_alpha, m_rt_lens_cmd.overlay_alpha);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        break;
    }

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
    if (m_shadow_shader_handle == kInvalidGpuResource)
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
    // dist_sq is clamped to [16,31] by create_shadows() -- approximate linear falloff (closer = darker). 
    // Todo make configurable?
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
    if (m_rt_shadow_cmds.empty() || !m_resource_mapper)
        return;
    const GLProgram* const shadow_prog = m_resource_mapper->ResolveProgram(m_shadow_shader_handle);
    const GLGeometryBuffer* const shadow_geom = m_resource_mapper->ResolveGeometryBuffer(m_shadow_geom_handle);
    if (!shadow_prog || !shadow_geom)
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

    glBindBuffer(GL_ARRAY_BUFFER, shadow_geom->vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(s_verts.size() * sizeof(ShadowGLVertex)),
                 s_verts.data(), GL_STREAM_DRAW);

    glUseProgram(shadow_prog->id);
    glBindVertexArray(shadow_geom->vao);
    glUniform2f(m_shadow_loc_viewport, (float)m_draw_screen_w, (float)m_draw_screen_h);

    const GLTexture* sprite_array_tex = m_resource_mapper->ResolveTexture(m_kspr_sprite_array_handle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sprite_array_tex ? sprite_array_tex->id : 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    glDepthMask(GL_FALSE);  // shadows test against tile depth but don't occlude each other

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)s_verts.size());

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

/******************************************************************************/
// Keeper sprites
namespace {
/** Mechanical helper for init_keeper_sprite_shader()'s many "create program,
 *  resolve, cache uniform locations if it linked" blocks -- returns the
 *  resolved GLProgram* (nullptr on failure), so the caller's own
 *  uniform-caching code (which differs per shader) stays inline. */
const GLProgram* CreateKsprProgram(GLResourceMapper* mapper, GpuResourceHandle& handle,
                                   const char* vs_src, const char* fs_src, const char* debug_name)
{
    GpuProgramDesc desc;
    desc.vertex_src = vs_src;
    desc.fragment_src = fs_src;
    desc.debug_name = debug_name;
    handle = mapper->RequestCreateProgram(desc);
    return mapper->ResolveProgram(handle);
}
} // namespace

bool GLWorldViewRenderer::init_keeper_sprite_shader()
{
    if (!m_resource_mapper) return false;

    // Non-instanced fallback: single-texture, palette+CLUT shader.
    const GLProgram* kspr_prog = CreateKsprProgram(m_resource_mapper, m_kspr_shader_handle,
        KSPR_VERTEX_SHADER, KSPR_FRAGMENT_SHADER, "kspr");
    if (!kspr_prog)
    {
        ERRORLOG("GLWorldViewRenderer: keeper-sprite shader link error");
        return false;
    }
    glUseProgram(kspr_prog->id);
    m_kspr_loc_viewport = glGetUniformLocation(kspr_prog->id, "u_viewport");
    m_kspr_loc_sprite   = glGetUniformLocation(kspr_prog->id, "u_sprite");
    m_kspr_loc_palette  = glGetUniformLocation(kspr_prog->id, "u_palette");
    m_kspr_loc_alpha    = glGetUniformLocation(kspr_prog->id, "u_alpha");
    m_kspr_loc_z_ndc    = glGetUniformLocation(kspr_prog->id, "u_z_ndc");
    glUniform1i(m_kspr_loc_sprite,  0);  // GL_TEXTURE0
    glUniform1i(m_kspr_loc_palette, 1);  // GL_TEXTURE1
    glUseProgram(0);

    // Additive-glow variant
    {
        const GLProgram* glow_prog = CreateKsprProgram(m_resource_mapper, m_kspr_glow_shader_handle,
            KSPR_VERTEX_SHADER, KSPR_GLOW_FRAGMENT_SHADER, "kspr_glow");
        if (glow_prog)
        {
            glUseProgram(glow_prog->id);
            m_kspr_glow_loc_viewport = glGetUniformLocation(glow_prog->id, "u_viewport");
            m_kspr_glow_loc_sprite   = glGetUniformLocation(glow_prog->id, "u_sprite");
            m_kspr_glow_loc_z_ndc    = glGetUniformLocation(glow_prog->id, "u_z_ndc");
            glUniform1i(m_kspr_glow_loc_sprite, 0);  // GL_TEXTURE0
            glUseProgram(0);
        }
        else
        {
            WARNLOG("GLWorldViewRenderer: keeper-sprite glow shader unavailable");
        }
    }

    // Reusable 256x256 R8 texture -- overwritten per sprite. Zero-initialised
    // so unwritten regions sample as index 0 (transparent) rather than
    // undefined garbage.
    {
        static const uint8_t s_zero_256x256[256 * 256] = {};
        GpuTextureDesc desc;
        desc.width = 256;
        desc.height = 256;
        desc.format = GpuTextureFormat::R8;
        desc.min_filter = GpuTextureFilter::Nearest;
        desc.mag_filter = GpuTextureFilter::Nearest;
        desc.wrap = GpuTextureWrap::Clamp;
        desc.initial_pixels.assign(s_zero_256x256, s_zero_256x256 + sizeof(s_zero_256x256));
        desc.debug_name = "kspr_sprite";
        m_kspr_sprite_tex_handle = m_resource_mapper->RequestCreateTexture(desc);
        if (m_resource_mapper->ResolveTexture(m_kspr_sprite_tex_handle) == nullptr)
        {
            ERRORLOG("GLWorldViewRenderer: keeper-sprite texture realization failed");
            return false;
        }
    }

    // VAO + VBO: 6 vertices x (vec2 pos + vec2 uv) = 4 floats each
    {
        GpuGeometryBufferDesc desc;
        desc.vertex_stride = 4 * (uint32_t)sizeof(float);
        desc.attribs = {
            { 0, 2, GpuVertexAttribType::Float, 0 },
            { 1, 2, GpuVertexAttribType::Float, 2 * (uint32_t)sizeof(float) },
        };
        desc.dynamic = true;
        desc.initial_vertex_capacity = 6 * 4 * sizeof(float);
        desc.debug_name = "kspr_geom";
        m_kspr_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(desc);
        if (m_resource_mapper->ResolveGeometryBuffer(m_kspr_geom_handle) == nullptr)
        {
            ERRORLOG("GLWorldViewRenderer: keeper-sprite geometry buffer realization failed");
            return false;
        }
    }

    // Attempt to build the sprite decode atlas (GL_TEXTURE_2D_ARRAY).
    do {
        const GLProgram* atlas_prog = CreateKsprProgram(m_resource_mapper, m_kspr_atlas_shader_handle,
            KSPR_VERTEX_SHADER, KSPR_ARRAY_FRAGMENT_SHADER, "kspr_array");
        if (!atlas_prog)
        {
            WARNLOG("kspr_array shader link failed");
            break;
        }
        glUseProgram(atlas_prog->id);
        m_kspr_atlas_loc_viewport = glGetUniformLocation(atlas_prog->id, "u_viewport");
        m_kspr_atlas_loc_sprite   = glGetUniformLocation(atlas_prog->id, "u_sprite");
        m_kspr_atlas_loc_clut     = glGetUniformLocation(atlas_prog->id, "u_clut");
        m_kspr_atlas_loc_alpha    = glGetUniformLocation(atlas_prog->id, "u_alpha");
        m_kspr_atlas_loc_z_ndc    = glGetUniformLocation(atlas_prog->id, "u_z_ndc");
        m_kspr_atlas_loc_layer    = glGetUniformLocation(atlas_prog->id, "u_layer");
        m_kspr_atlas_loc_clut_v   = glGetUniformLocation(atlas_prog->id, "u_clut_v");
        glUniform1i(m_kspr_atlas_loc_sprite, 0);  // GL_TEXTURE0
        glUniform1i(m_kspr_atlas_loc_clut,   1);  // GL_TEXTURE1
        glUseProgram(0);

        // Additive-glow variant of the atlas shader
        {
            const GLProgram* atlas_glow_prog = CreateKsprProgram(m_resource_mapper, m_kspr_atlas_glow_shader_handle,
                KSPR_VERTEX_SHADER, KSPR_ARRAY_GLOW_FRAGMENT_SHADER, "kspr_array_glow");
            if (atlas_glow_prog)
            {
                glUseProgram(atlas_glow_prog->id);
                m_kspr_atlas_glow_loc_viewport = glGetUniformLocation(atlas_glow_prog->id, "u_viewport");
                m_kspr_atlas_glow_loc_sprite   = glGetUniformLocation(atlas_glow_prog->id, "u_sprite");
                m_kspr_atlas_glow_loc_z_ndc    = glGetUniformLocation(atlas_glow_prog->id, "u_z_ndc");
                m_kspr_atlas_glow_loc_layer    = glGetUniformLocation(atlas_glow_prog->id, "u_layer");
                glUniform1i(m_kspr_atlas_glow_loc_sprite, 0);  // GL_TEXTURE0
                glUseProgram(0);
            }
            else
            {
                WARNLOG("GLWorldViewRenderer: failed to compile keeper-sprite array glow shader");
            }
        }

        // Allocate the texture array. RequestCreateTexture's initial_pixels
        // stays empty (nullptr upload) -- undefined storage is valid and
        // expected here, matching the original's own glTexImage3D(...,
        // nullptr) call.
        GpuTextureDesc array_desc;
        array_desc.width = k_kspr_decode_dim;
        array_desc.height = k_kspr_decode_dim;
        array_desc.array_layers = k_kspr_atlas_layers;
        array_desc.format = GpuTextureFormat::R8;
        array_desc.min_filter = GpuTextureFilter::Nearest;
        array_desc.mag_filter = GpuTextureFilter::Nearest;
        array_desc.wrap = GpuTextureWrap::Clamp;
        array_desc.debug_name = "kspr_sprite_array";
        m_kspr_sprite_array_handle = m_resource_mapper->RequestCreateTexture(array_desc);
        if (m_resource_mapper->ResolveTexture(m_kspr_sprite_array_handle) == nullptr)
        {
            WARNLOG("GL_TEXTURE_2D_ARRAY alloc failed, sprite atlas disabled");
            m_kspr_sprite_array_handle = kInvalidGpuResource;
            m_kspr_atlas_shader_handle = kInvalidGpuResource;
            m_kspr_atlas_glow_shader_handle = kInvalidGpuResource;
            break;
        }
        SYNCLOG("GLWorldViewRenderer: keeper-sprite decode atlas ready (%d layers, 256x256 GL_R8)",
                k_kspr_atlas_layers);
    } while (false);

    // CLUT texture -- only useful (and only allocated) when the atlas shader
    // compiled successfully.
    if (m_kspr_atlas_shader_handle != kInvalidGpuResource && m_kspr_clut_tex_handle == kInvalidGpuResource)
    {
        // Clear the entire CLUT to zero (transparent black) so unwritten
        // rows never produce garbage. Only 256*k_clut_rows*4 = 128 KB.
        std::vector<uint8_t> zero_clut((size_t)256 * k_clut_rows * 4, 0);
        GpuTextureDesc desc;
        desc.width = 256;
        desc.height = k_clut_rows;
        desc.format = GpuTextureFormat::RGBA8;
        desc.min_filter = GpuTextureFilter::Nearest;
        desc.mag_filter = GpuTextureFilter::Nearest;
        desc.wrap = GpuTextureWrap::Clamp;
        desc.initial_pixels = std::move(zero_clut);
        desc.debug_name = "kspr_clut";
        m_kspr_clut_tex_handle = m_resource_mapper->RequestCreateTexture(desc);
        m_resource_mapper->ResolveTexture(m_kspr_clut_tex_handle); // force realization now
        m_kspr_clut_used = 1;
    }

    // ── Depth-fail outline shaders ───────────────────────────────────
    // Todo : fix and customise,the colour should be the owner's colour of the creature, same rules as unclaimed rooms
    {
        const GLProgram* outline_prog = CreateKsprProgram(m_resource_mapper, m_kspr_outline_shader_handle,
            KSPR_VERTEX_SHADER, KSPR_OUTLINE_FRAGMENT_SHADER, "kspr_outline");
        if (outline_prog)
        {
            glUseProgram(outline_prog->id);
            m_kspr_outline_loc_viewport = glGetUniformLocation(outline_prog->id, "u_viewport");
            m_kspr_outline_loc_sprite   = glGetUniformLocation(outline_prog->id, "u_sprite");
            m_kspr_outline_loc_z_ndc    = glGetUniformLocation(outline_prog->id, "u_z_ndc");
            m_kspr_outline_loc_color    = glGetUniformLocation(outline_prog->id, "u_outline_color");
            glUniform1i(m_kspr_outline_loc_sprite, 0); // GL_TEXTURE0
            glUseProgram(0);
        }
        else
        {
            WARNLOG("GLWorldViewRenderer: kspr_outline shader link failed");
        }

        if (m_kspr_atlas_shader_handle != kInvalidGpuResource)
        {
            const GLProgram* atlas_outline_prog = CreateKsprProgram(m_resource_mapper, m_kspr_atlas_outline_shader_handle,
                KSPR_VERTEX_SHADER, KSPR_ARRAY_OUTLINE_FRAGMENT_SHADER, "kspr_array_outline");
            if (atlas_outline_prog)
            {
                glUseProgram(atlas_outline_prog->id);
                m_kspr_atlas_outline_loc_viewport = glGetUniformLocation(atlas_outline_prog->id, "u_viewport");
                m_kspr_atlas_outline_loc_sprite   = glGetUniformLocation(atlas_outline_prog->id, "u_sprite");
                m_kspr_atlas_outline_loc_z_ndc    = glGetUniformLocation(atlas_outline_prog->id, "u_z_ndc");
                m_kspr_atlas_outline_loc_color    = glGetUniformLocation(atlas_outline_prog->id, "u_outline_color");
                m_kspr_atlas_outline_loc_layer    = glGetUniformLocation(atlas_outline_prog->id, "u_layer");
                glUniform1i(m_kspr_atlas_outline_loc_sprite, 0); // GL_TEXTURE0
                glUseProgram(0);
            }
            else
            {
                WARNLOG("GLWorldViewRenderer: kspr_array_outline shader link failed");
            }
        }
    }

    // ── Edge-detect outline shaders ──────────────────────────────────
    // Todo : fix and customise,the colour should be the owner's colour of the creature, same rules as unclaimed rooms
    {
        const GLProgram* edge_prog = CreateKsprProgram(m_resource_mapper, m_kspr_edge_shader_handle,
            KSPR_VERTEX_SHADER, KSPR_EDGE_FRAGMENT_SHADER, "kspr_edge");
        if (edge_prog)
        {
            glUseProgram(edge_prog->id);
            m_kspr_edge_loc_viewport = glGetUniformLocation(edge_prog->id, "u_viewport");
            m_kspr_edge_loc_sprite   = glGetUniformLocation(edge_prog->id, "u_sprite");
            m_kspr_edge_loc_z_ndc    = glGetUniformLocation(edge_prog->id, "u_z_ndc");
            m_kspr_edge_loc_color    = glGetUniformLocation(edge_prog->id, "u_outline_color");
            glUniform1i(m_kspr_edge_loc_sprite, 0);
            glUseProgram(0);
        }
        else
        {
            WARNLOG("GLWorldViewRenderer: kspr_edge shader link failed");
        }

        if (m_kspr_atlas_shader_handle != kInvalidGpuResource)
        {
            const GLProgram* atlas_edge_prog = CreateKsprProgram(m_resource_mapper, m_kspr_atlas_edge_shader_handle,
                KSPR_VERTEX_SHADER, KSPR_ARRAY_EDGE_FRAGMENT_SHADER, "kspr_array_edge");
            if (atlas_edge_prog)
            {
                glUseProgram(atlas_edge_prog->id);
                m_kspr_atlas_edge_loc_viewport = glGetUniformLocation(atlas_edge_prog->id, "u_viewport");
                m_kspr_atlas_edge_loc_sprite   = glGetUniformLocation(atlas_edge_prog->id, "u_sprite");
                m_kspr_atlas_edge_loc_z_ndc    = glGetUniformLocation(atlas_edge_prog->id, "u_z_ndc");
                m_kspr_atlas_edge_loc_color    = glGetUniformLocation(atlas_edge_prog->id, "u_outline_color");
                m_kspr_atlas_edge_loc_layer    = glGetUniformLocation(atlas_edge_prog->id, "u_layer");
                glUniform1i(m_kspr_atlas_edge_loc_sprite, 0);
                glUseProgram(0);
            }
            else
            {
                WARNLOG("GLWorldViewRenderer: kspr_array_edge shader link failed");
            }
        }
    }

    SYNCLOG("GLWorldViewRenderer: keeper-sprite shader initialised");

    // Instanced fast path: optional, requires the decode atlas. Failure just
    // leaves the per-sprite path active (handle invalid gates usage in
    // gpu_execute_passes()).
    if (m_kspr_sprite_array_handle != kInvalidGpuResource && !init_keeper_sprite_instancing())
        WARNLOG("GLWorldViewRenderer: sprite instancing unavailable, using per-sprite path");

    return true;
}

bool GLWorldViewRenderer::init_keeper_sprite_instancing()
{
    if (!m_resource_mapper) return false;

    const GLProgram* inst_prog = CreateKsprProgram(m_resource_mapper, m_kspr_inst_shader_handle,
        KSPR_INST_VERTEX_SHADER, KSPR_INST_FRAGMENT_SHADER, "kspr_inst");
    if (!inst_prog)
        return false;

    glUseProgram(inst_prog->id);
    m_kspr_inst_loc_viewport = glGetUniformLocation(inst_prog->id, "u_viewport");
    glUniform1i(glGetUniformLocation(inst_prog->id, "u_sprite"), 0);  // GL_TEXTURE0
    glUniform1i(glGetUniformLocation(inst_prog->id, "u_clut"),   1);  // GL_TEXTURE1
    glUseProgram(0);

    static const float k_unit_quad[8] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };
    {
        GpuGeometryBufferDesc desc;
        desc.vertex_stride = 2 * (uint32_t)sizeof(float);
        desc.attribs = { { 0, 2, GpuVertexAttribType::Float, 0 } };
        desc.dynamic = false; // static unit quad, content never changes
        desc.initial_vertex_capacity = sizeof(k_unit_quad);
        desc.debug_name = "kspr_inst_quad";
        m_kspr_inst_quad_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(desc);
        const GLGeometryBuffer* quad_geom = m_resource_mapper->ResolveGeometryBuffer(m_kspr_inst_quad_geom_handle);
        if (!quad_geom) return false;
        glBindBuffer(GL_ARRAY_BUFFER, quad_geom->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(k_unit_quad), k_unit_quad);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Attribute byte offsets, locked to KsprInstance's layout.
    static_assert(sizeof(KsprInstance) == 44, "KsprInstance layout changed -- update attrib offsets");
    constexpr uintptr_t k_inst_off_rect  = 0;   // float[4]
    constexpr uintptr_t k_inst_off_uvext = 16;  // float[2]
    constexpr uintptr_t k_inst_off_misc  = 24;  // layer, clut_v, alpha, z_ndc
    constexpr uintptr_t k_inst_off_flags = 40;  // uint32_t

    {
        GpuGeometryBufferDesc desc;
        desc.vertex_stride = (uint32_t)sizeof(KsprInstance);
        desc.dynamic = true;
        desc.debug_name = "kspr_inst_geom";
        m_kspr_inst_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(desc);
    }
    const GLGeometryBuffer* quad_geom = m_resource_mapper->ResolveGeometryBuffer(m_kspr_inst_quad_geom_handle);
    const GLGeometryBuffer* inst_geom = m_resource_mapper->ResolveGeometryBuffer(m_kspr_inst_geom_handle);
    if (!quad_geom || !inst_geom) return false;

    glBindVertexArray(inst_geom->vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_geom->vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, inst_geom->vbo);
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

    // Instanced depth-fail outline shader
    {
        const GLProgram* inst_outline_prog = CreateKsprProgram(m_resource_mapper, m_kspr_inst_outline_shader_handle,
            KSPR_INST_OUTLINE_VERTEX_SHADER, KSPR_INST_OUTLINE_FRAGMENT_SHADER, "kspr_inst_outline");
        if (inst_outline_prog)
        {
            glUseProgram(inst_outline_prog->id);
            m_kspr_inst_outline_loc_viewport = glGetUniformLocation(inst_outline_prog->id, "u_viewport");
            glUniform1i(glGetUniformLocation(inst_outline_prog->id, "u_sprite"), 0);
            glUseProgram(0);
        }
    }

    // Instanced edge-detect shader -- shares the same vertex shader and VAO
    // as the instanced outline, but uses edge-detection in the fragment shader.
    {
        const GLProgram* inst_edge_prog = CreateKsprProgram(m_resource_mapper, m_kspr_inst_edge_shader_handle,
            KSPR_INST_OUTLINE_VERTEX_SHADER, KSPR_INST_EDGE_FRAGMENT_SHADER, "kspr_inst_edge");
        if (inst_edge_prog)
        {
            glUseProgram(inst_edge_prog->id);
            m_kspr_inst_edge_loc_viewport = glGetUniformLocation(inst_edge_prog->id, "u_viewport");
            glUniform1i(glGetUniformLocation(inst_edge_prog->id, "u_sprite"), 0);
            glUseProgram(0);
        }
    }

    // Outline VAO: same shared unit quad, KsprOutlineInstance layout.
    static_assert(sizeof(KsprOutlineInstance) == 52, "KsprOutlineInstance layout changed -- update attrib offsets");
    constexpr uintptr_t k_outl_off_rect  = 0;   // float[4]
    constexpr uintptr_t k_outl_off_uvext = 16;  // float[2]
    constexpr uintptr_t k_outl_off_lzf   = 24;  // layer, z_ndc, flip
    constexpr uintptr_t k_outl_off_color = 36;  // float[4]

    {
        GpuGeometryBufferDesc desc;
        desc.vertex_stride = (uint32_t)sizeof(KsprOutlineInstance);
        desc.dynamic = true;
        desc.debug_name = "kspr_inst_outline_geom";
        m_kspr_inst_outline_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(desc);
    }
    const GLGeometryBuffer* outline_geom = m_resource_mapper->ResolveGeometryBuffer(m_kspr_inst_outline_geom_handle);
    if (!outline_geom) return false;

    glBindVertexArray(outline_geom->vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_geom->vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, outline_geom->vbo);
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
    m_kspr_pass_start   = m_kspr_ir.size();

    gpu_flush();
    m_cmd_vert_start = m_vert_count;

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

/******************************************************************************/
// Keeper sprites

void GLWorldViewRenderer::ClearKeeperSpriteAtlas()
{
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
    DrawCmd cmd;
    cmd.type = DrawCmd::CMD_PRELOAD_KSPR_ATLAS;
    m_draw_cmds.push_back(cmd);
    SYNCLOG("GLWorldViewRenderer: CMD_PRELOAD_KSPR_ATLAS queued");
}

void GLWorldViewRenderer::execute_preload_atlas()
{
    if (!m_resource_mapper) return;
    const GLTexture* sprite_array = m_resource_mapper->ResolveTexture(m_kspr_sprite_array_handle);
    if (!sprite_array || m_kspr_atlas_shader_handle == kInvalidGpuResource) {
        ERRORLOG("execute_preload_atlas: GL resources not ready -- preload skipped");
        return;
    }

    ensure_clut_valid();  // identity row 0 must exist before any atlas draws

    int preloaded = 0;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sprite_array->id);

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
    if (!m_resource_mapper) return;
    const GLTexture* clut_tex = m_resource_mapper->ResolveTexture(m_kspr_clut_tex_handle);
    if (!clut_tex) return;
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
    glBindTexture(GL_TEXTURE_2D, clut_tex->id);
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
    if (!m_resource_mapper) return -1;
    const GLTexture* sprite_array = m_resource_mapper->ResolveTexture(m_kspr_sprite_array_handle);
    const GLTexture* clut_tex = m_resource_mapper->ResolveTexture(m_kspr_clut_tex_handle);
    if (!sprite_array || m_kspr_atlas_shader_handle == kInvalidGpuResource || !clut_tex)
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
    glBindTexture(GL_TEXTURE_2D_ARRAY, sprite_array->id);
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
    if (!m_resource_mapper) return clut_v;
    const GLTexture* clut_tex = m_resource_mapper->ResolveTexture(m_kspr_clut_tex_handle);
    if (!remap || !clut_tex)
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
    glBindTexture(GL_TEXTURE_2D, clut_tex->id);
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

    float alpha = 1.0f;
    if      (cmd.draw_flags & Lb_SPRITE_TRANSPAR4) alpha = g_renderer_settings.transpar4_alpha;
    else if (cmd.draw_flags & Lb_SPRITE_TRANSPAR8) alpha = g_renderer_settings.transpar8_alpha;

    // Water/lava clipping
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

    // Depth-fail outline pass
    if (g_renderer_settings.creature_outline_mode != RENDERER_OUTLINE_NONE && !additive && cmd.wants_outline
        && (m_kspr_inst_outline_shader_handle != kInvalidGpuResource || m_kspr_inst_edge_shader_handle != kInvalidGpuResource))
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
    if (!m_resource_mapper) return;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    // Premultiplied alpha: the shader outputs (rgb*a, a) for normal sprites
    // and (rgb, 0) for additive glow, so one blend mode covers both.
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    {
        const GLTexture* clut_tex = m_resource_mapper->ResolveTexture(m_kspr_clut_tex_handle);
        const GLTexture* sprite_array = m_resource_mapper->ResolveTexture(m_kspr_sprite_array_handle);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, clut_tex ? clut_tex->id : 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, sprite_array ? sprite_array->id : 0);
    }

    // Depth-fail outline pass first
    if (!m_kspr_outline_instances.empty())
    {
        const bool use_edge = (g_renderer_settings.creature_outline_mode == RENDERER_OUTLINE_EDGE);
        GpuResourceHandle prog_handle = use_edge ? m_kspr_inst_edge_shader_handle : m_kspr_inst_outline_shader_handle;
        GLint  vloc = use_edge ? m_kspr_inst_edge_loc_viewport : m_kspr_inst_outline_loc_viewport;
        const GLProgram* prog = m_resource_mapper->ResolveProgram(prog_handle);
        const GLGeometryBuffer* outline_geom = m_resource_mapper->ResolveGeometryBuffer(m_kspr_inst_outline_geom_handle);
        if (prog && outline_geom)
        {
            glDepthFunc(GL_GREATER);
            glUseProgram(prog->id);
            glUniform2f(vloc, (float)m_draw_screen_w, (float)m_draw_screen_h);
            glBindVertexArray(outline_geom->vao);
            glBindBuffer(GL_ARRAY_BUFFER, outline_geom->vbo);
            glBufferData(GL_ARRAY_BUFFER,
                         (GLsizeiptr)(m_kspr_outline_instances.size() * sizeof(KsprOutlineInstance)),
                         m_kspr_outline_instances.data(), GL_STREAM_DRAW);
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4,
                                  (GLsizei)m_kspr_outline_instances.size());
        }
    }

    if (!m_kspr_instances.empty())
    {
        const GLProgram* prog = m_resource_mapper->ResolveProgram(m_kspr_inst_shader_handle);
        const GLGeometryBuffer* inst_geom = m_resource_mapper->ResolveGeometryBuffer(m_kspr_inst_geom_handle);
        if (prog && inst_geom)
        {
            glDepthFunc(GL_LEQUAL);
            glUseProgram(prog->id);
            glUniform2f(m_kspr_inst_loc_viewport,
                        (float)m_draw_screen_w, (float)m_draw_screen_h);
            glBindVertexArray(inst_geom->vao);
            glBindBuffer(GL_ARRAY_BUFFER, inst_geom->vbo);
            glBufferData(GL_ARRAY_BUFFER,
                         (GLsizeiptr)(m_kspr_instances.size() * sizeof(KsprInstance)),
                         m_kspr_instances.data(), GL_STREAM_DRAW);
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)m_kspr_instances.size());
        }
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
    // No mapper, no run.
    if (!m_resource_mapper) return 1;
    const GLProgram* kspr_prog = m_resource_mapper->ResolveProgram(m_kspr_shader_handle);
    const GLTexture* kspr_sprite_tex = m_resource_mapper->ResolveTexture(m_kspr_sprite_tex_handle);
    const GLGeometryBuffer* kspr_geom = m_resource_mapper->ResolveGeometryBuffer(m_kspr_geom_handle);
    if (!kspr_prog || !kspr_sprite_tex || !kspr_geom || m_palette_tex_handle == kInvalidGpuResource) {
        static int s_miss = 0;
        if (s_miss++ < 5)
            ERRORLOG("render_keepersprite_gpu: GL resources not ready -- sprite dropped");
        return 1;
    }
    const GLProgram* kspr_glow_prog = m_resource_mapper->ResolveProgram(m_kspr_glow_shader_handle);
    const GLProgram* kspr_atlas_glow_prog = m_resource_mapper->ResolveProgram(m_kspr_atlas_glow_shader_handle);
    const GLProgram* kspr_atlas_prog = m_resource_mapper->ResolveProgram(m_kspr_atlas_shader_handle);
    const GLTexture* kspr_sprite_array = m_resource_mapper->ResolveTexture(m_kspr_sprite_array_handle);
    const GLTexture* kspr_clut_tex = m_resource_mapper->ResolveTexture(m_kspr_clut_tex_handle);
    const GLProgram* kspr_outline_prog = m_resource_mapper->ResolveProgram(m_kspr_outline_shader_handle);
    const GLProgram* kspr_atlas_outline_prog = m_resource_mapper->ResolveProgram(m_kspr_atlas_outline_shader_handle);
    const GLProgram* kspr_edge_prog = m_resource_mapper->ResolveProgram(m_kspr_edge_shader_handle);
    const GLProgram* kspr_atlas_edge_prog = m_resource_mapper->ResolveProgram(m_kspr_atlas_edge_shader_handle);
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
    if (atlas_layer >= 0 && use_remap && kspr_clut_tex)
        clut_v = resolve_clut_v(remap);

    // Water/lava clipping
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
    glBindBuffer(GL_ARRAY_BUFFER, kspr_geom->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sv), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sv), sv);

    float alpha = 1.0f;
    if      (draw_flags & Lb_SPRITE_TRANSPAR4) alpha = g_renderer_settings.transpar4_alpha;
    else if (draw_flags & Lb_SPRITE_TRANSPAR8) alpha = g_renderer_settings.transpar8_alpha;

    // Additive glow
    const bool use_glow_atlas    = additive && atlas_layer >= 0 && kspr_atlas_glow_prog;
    const bool use_glow_fallback = additive && atlas_layer <  0 && kspr_glow_prog;
    const GLenum blend_dfactor = additive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA;

    glBindVertexArray(kspr_geom->vao);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    // ── Depth-fail outline pass ──────────────────────────────────────
    // Draws a flat owner-colour silhouette only where the sprite is BEHIND
    // geometry (GL_GREATER depth test). The normal draw below then paints
    // over the visible portion with GL_LEQUAL, leaving the outline only
    // peeking out from behind walls/columns.
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
            const GLProgram* prog = use_edge ? kspr_atlas_edge_prog : kspr_atlas_outline_prog;
            GLint  loc_vp  = use_edge ? m_kspr_atlas_edge_loc_viewport   : m_kspr_atlas_outline_loc_viewport;
            GLint  loc_z   = use_edge ? m_kspr_atlas_edge_loc_z_ndc      : m_kspr_atlas_outline_loc_z_ndc;
            GLint  loc_lay = use_edge ? m_kspr_atlas_edge_loc_layer      : m_kspr_atlas_outline_loc_layer;
            GLint  loc_col = use_edge ? m_kspr_atlas_edge_loc_color      : m_kspr_atlas_outline_loc_color;
            if (prog && kspr_sprite_array)
            {
                // Atlas path: texture already cached in kspr_sprite_array.
                glUseProgram(prog->id);
                glUniform2f(loc_vp, (float)m_draw_screen_w, (float)m_draw_screen_h);
                glUniform1f(loc_z,    outline_z);
                glUniform1f(loc_lay,  (float)atlas_layer);
                glUniform4f(loc_col,  oc_r, oc_g, oc_b, oc_a);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D_ARRAY, kspr_sprite_array->id);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
        else
        {
            // Fallback path: decode + upload now so the outline has pixel data.
            // The normal draw below will re-use the already-uploaded texture.
            const GLProgram* prog = use_edge ? kspr_edge_prog : kspr_outline_prog;
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
                glBindTexture(GL_TEXTURE_2D, kspr_sprite_tex->id);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, k_kspr_decode_dim);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, src_w, src_h,
                                GL_RED, GL_UNSIGNED_BYTE, s_kspr_decode_buf);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

                glUseProgram(prog->id);
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
            glUseProgram(kspr_atlas_glow_prog->id);
            glUniform2f(m_kspr_atlas_glow_loc_viewport, (float)m_draw_screen_w, (float)m_draw_screen_h);
            glUniform1f(m_kspr_atlas_glow_loc_z_ndc, z_ndc);
            glUniform1f(m_kspr_atlas_glow_loc_layer, (float)atlas_layer);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, kspr_sprite_array ? kspr_sprite_array->id : 0);
            glBlendFunc(GL_ONE, GL_ONE);
        } else {
            // Normal or remapped sprite via atlas + CLUT.
            glUseProgram(kspr_atlas_prog->id);
            glUniform2f(m_kspr_atlas_loc_viewport, (float)m_draw_screen_w, (float)m_draw_screen_h);
            glUniform1f(m_kspr_atlas_loc_alpha, alpha);
            glUniform1f(m_kspr_atlas_loc_z_ndc, z_ndc);
            glUniform1f(m_kspr_atlas_loc_layer, (float)atlas_layer);
            glUniform1f(m_kspr_atlas_loc_clut_v, clut_v);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, kspr_clut_tex ? kspr_clut_tex->id : 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, kspr_sprite_array ? kspr_sprite_array->id : 0);
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
                                   && (kspr_outline_prog || kspr_edge_prog);
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
        glBindTexture(GL_TEXTURE_2D, kspr_sprite_tex->id);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, k_kspr_decode_dim);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, src_w, src_h,
                        GL_RED, GL_UNSIGNED_BYTE, s_kspr_decode_buf);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }

    if (use_glow_fallback)
    {
        // Pure additive blend: adds the glow RGB delta to the framebuffer.
        glUseProgram(kspr_glow_prog->id);
        glUniform2f(m_kspr_glow_loc_viewport, (float)m_draw_screen_w, (float)m_draw_screen_h);
        glUniform1f(m_kspr_glow_loc_z_ndc,    z_ndc);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, kspr_sprite_tex->id);
        glBlendFunc(GL_ONE, GL_ONE);
    }
    else
    {
        glUseProgram(kspr_prog->id);
        glUniform2f(m_kspr_loc_viewport, (float)m_draw_screen_w, (float)m_draw_screen_h);
        glUniform1f(m_kspr_loc_alpha,    alpha);
        glUniform1f(m_kspr_loc_z_ndc,    z_ndc);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ResolvePaletteTexId());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, kspr_sprite_tex->id);
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
    gpu_execute_passes(m_rt_vp_x, vp_y_gl, m_rt_screen_w, m_rt_screen_h, cmds.tile_verts);
}

void GLWorldViewRenderer::gpu_execute_passes(int vp_x, int vp_y_gl, int screen_w, int screen_h,
                                             const std::vector<WorldVertex>& tile_verts)
{
    m_draw_screen_w = screen_w;
    m_draw_screen_h = screen_h;

    m_kspr_atlas_hits   = 0;
    m_kspr_atlas_misses = 0;

    ensure_clut_valid();  // identity row 0 must exist before any sprite draws

    if (!m_resource_mapper) return;
    const GLGeometryBuffer* const world_geom = m_resource_mapper->ResolveGeometryBuffer(m_geom_handle);
    const GLuint world_shader_id = ResolveShaderId(m_shader_handle);
    const GLTexture* const lightmap_tex = m_resource_mapper->ResolveTexture(m_lightmap_tex_handle);
    if (!world_geom || !world_shader_id || !lightmap_tex) return;

    glBindBuffer(GL_ARRAY_BUFFER, world_geom->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    (GLsizeiptr)(tile_verts.size() * sizeof(WorldVertex)),
                    tile_verts.data());

    glViewport(vp_x, vp_y_gl, screen_w, screen_h);
    glUseProgram(world_shader_id);
    glBindVertexArray(world_geom->vao);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    bool atlas_bound       = false;  // tile atlas array bound to GL_TEXTURE0

    // Upload lightmap shadow copy (snapshotted from game.lish.subtile_lightness[]
    // during FlipBuffers() on the game thread — no live game struct access here).
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, lightmap_tex->id);
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
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindTexture(GL_TEXTURE_2D_ARRAY, atlas_tex);
                glUniform1f(m_loc_missing_tile, (atlas_tex == 0) ? 1.0f : 0.0f);
                atlas_bound = true;
                if (m_tile_filter_applied != g_renderer_settings.tile_filter)
                {
                    const GLint filter = (g_renderer_settings.tile_filter == RENDERER_FILTER_LINEAR)
                                        ? GL_LINEAR : GL_NEAREST;
                    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, filter);
                    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, filter);
                    m_tile_filter_applied = g_renderer_settings.tile_filter;
                }
            }
            glDepthFunc(GL_ALWAYS);
            glDrawArrays(GL_TRIANGLES, cmd.vert_start, cmd.vert_count);
        }
        else if (cmd.type == DrawCmd::CMD_PRELOAD_KSPR_ATLAS)
        {
            execute_preload_atlas();
            // Restore tile shader and VAO that were active before the preload.
            glUseProgram(world_shader_id);
            glBindVertexArray(world_geom->vao);
            atlas_bound = false;
        }
        else if (cmd.type == DrawCmd::CMD_CLEAR_KSPR_ATLAS)
        {
            // Pure CPU-side cache reset, no GL calls -- unlike preload, no
            // shader/VAO state to restore afterward.
            execute_clear_atlas();
        }
    }

    glDepthFunc(GL_LEQUAL);

    // ── Creature shadows  ───────────────────────────────────────────────
    draw_shadows_gpu();
    glUseProgram(world_shader_id);
    glBindVertexArray(world_geom->vao);

    // ── Keeper sprites ────────────────────────────────────────────────────────
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

            const bool use_instancing = (m_kspr_inst_shader_handle != kInvalidGpuResource)
                                      && (m_kspr_sprite_array_handle != kInvalidGpuResource);
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
            glUseProgram(world_shader_id);
            glBindVertexArray(world_geom->vao);
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

    render_fade_tables = pixmap.fade_tables;
    render_ghost        = pixmap.ghost;
    render_alpha        = (unsigned char *)&alpha_sprite_table;

    union {
        struct BasicQ *b;
        struct BucketKindPolygonStandard *polygonStandard;
        struct BucketKindPolygonNearFP *polygonNearFP;
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
            case QK_JontySprite:
                draw_jonty_mapwho(item.jontySprite);
                break;
            case QK_JontyISOSprite:
                player = get_my_player();
                cam = get_local_camera(get_player_active_camera(player));
                if (cam != NULL)
                {
                    if (cam->view_mode == PVM_IsoWibbleView || cam->view_mode == PVM_IsoStraightView) {
                        draw_jonty_mapwho(item.jontySprite);
                    }
                }
                break;
            case QK_CreatureShadow:
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
            case QK_RoomFlagBottomPole: // Room-flag pole
            {
                const float ndc_z = 2.0f * ((float)m_current_bucket - 0.5f) / (float)(BUCKETS_COUNT - 1) - 1.0f;
                RendererSetWorldOverlayFlat(ndc_z);
                draw_engine_room_flagpole(item.roomFlag);
                RendererClearWorldOverlayFlat();
                break;
            }
            case QK_RoomFlagStatusBox: // Room-flag top status box
            {
                const float ndc_z = 2.0f * ((float)m_current_bucket - 0.5f) / (float)(BUCKETS_COUNT - 1) - 1.0f;
                RendererSetWorldOverlayFlat(ndc_z);
                draw_engine_room_flag_top(item.roomFlag);
                RendererClearWorldOverlayFlat();
                break;
            }
            case QK_FloatingGoldText: // Floating gold/damage/experience numbers
            {
                const float ndc_z = 2.0f * ((float)m_current_bucket - 0.5f) / (float)(BUCKETS_COUNT - 1) - 1.0f;
                RendererSetWorldOverlayFlat(ndc_z);
                draw_engine_number(item.floatingGoldText);
                RendererClearWorldOverlayFlat();
                break;
            }
            case QK_CreatureStatus: // Health-flower/anger/state icon above a creature.
            {
                const float ndc_z = 2.0f * ((float)m_current_bucket - 0.5f) / (float)(BUCKETS_COUNT - 1) - 1.0f;
                RendererSetWorldOverlay(ndc_z);
                draw_status_sprites(item.creatureStatus->x, item.creatureStatus->y, item.creatureStatus->thing);
                RendererClearWorldOverlay();
                break;
            }

            case QK_PolygonNearFP:
                append_triangle(item.polygonNearFP->block,
                                &item.polygonNearFP->vertex_first,
                                &item.polygonNearFP->vertex_second,
                                &item.polygonNearFP->vertex_third);
                break;

            default:
                break;
            }
        }
    }

    gpu_flush();


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

    render_fade_tables = pixmap.fade_tables;
    render_ghost        = pixmap.ghost;
    render_alpha        = (unsigned char *)&alpha_sprite_table;

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
            else if (b->kind == QK_PolygonNearFP)
            {
                const struct BucketKindPolygonNearFP* p = (const struct BucketKindPolygonNearFP*)b;
                append_triangle((int)p->block, &p->vertex_first, &p->vertex_second, &p->vertex_third);
            }
            else if (b->kind == QK_PolygonStandard)
            {
                const struct BucketKindPolygonStandard* p = (const struct BucketKindPolygonStandard*)b;
                append_triangle((int)p->block, &p->vertex_first, &p->vertex_second, &p->vertex_third);
            }
        }
    }

    gpu_flush();
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
