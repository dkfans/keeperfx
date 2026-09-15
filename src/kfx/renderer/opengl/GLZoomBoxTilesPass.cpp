/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLZoomBoxTilesPass.cpp
 *     See GLZoomBoxTilesPass.h for the design rationale.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/opengl/GLZoomBoxTilesPass.h"
#include "kfx/renderer/opengl/GLShaders.h"
#include "kfx/renderer/opengl/GLResourceMapper.h"
#include "kfx/renderer/opengl/GLTileAtlas.h"
#include "kfx/renderer/TileAtlasPacker.h"
#include "kfx/renderer/RendererThread.h"   // ASSERT_GAME_THREAD/ASSERT_RENDER_THREAD
#include "engine_textures.h"                // TEXTURE_BLOCKS_COUNT
#include "bflib_basics.h"                   // ERRORLOG
#include "post_inc.h"

/******************************************************************************/

GLZoomBoxTilesPass::~GLZoomBoxTilesPass()
{
    // Shutdown() must be called explicitly on the render thread before
    // destruction -- see GLMapFadePass's own destructor comment.
}

bool GLZoomBoxTilesPass::CompileShaders()
{
    if (m_resource_mapper == nullptr)
    {
        ERRORLOG("GLZoomBoxTilesPass::CompileShaders -- no resource mapper set");
        return false;
    }

    GpuProgramDesc desc;
    desc.vertex_src = ZOOMBOX_TILES_VERTEX_SHADER;
    desc.fragment_src = ZOOMBOX_TILES_FRAGMENT_SHADER;
    desc.debug_name = "zoombox_tiles";
    m_shader_handle = m_resource_mapper->RequestCreateProgram(desc);

    const GLProgram* shader = m_resource_mapper->ResolveProgram(m_shader_handle);
    if (!shader)
        return false;

    glUseProgram(shader->id);
    glUniform1i(glGetUniformLocation(shader->id, "u_atlas"), 0);
    glUniform1i(glGetUniformLocation(shader->id, "u_palette"), 1);
    m_loc_screen_size = glGetUniformLocation(shader->id, "u_screen_size");
    glUseProgram(0);

    // x,y,u,v,layer -- grown on demand each Resolve() via glBufferData
    // (orphan + refill), same as GLUIRenderer's batched layer draws; this
    // initial capacity is just a seed, not a hard cap.
    GpuGeometryBufferDesc geom_desc;
    geom_desc.vertex_stride = 5 * (uint32_t)sizeof(float);
    geom_desc.attribs = {
        { 0, 2, GpuVertexAttribType::Float, 0 },
        { 1, 3, GpuVertexAttribType::Float, 2 * (uint32_t)sizeof(float) },
    };
    geom_desc.dynamic = true;
    geom_desc.initial_vertex_capacity = 6 * 64 * 5 * (uint32_t)sizeof(float);
    geom_desc.debug_name = "zoombox_tiles_quad";
    m_geom_handle = m_resource_mapper->RequestCreateGeometryBuffer(geom_desc);

    return m_resource_mapper->ResolveGeometryBuffer(m_geom_handle) != nullptr;
}

void GLZoomBoxTilesPass::Shutdown()
{
    m_cmd = IRZoomBoxTilesCmd{};
    m_rt_cmd = IRZoomBoxTilesCmd{};
}

void GLZoomBoxTilesPass::Submit(const uint16_t* tile_block_ids, int tiles_x, int tiles_y,
                                int dst_x, int dst_y, int tile_w, int tile_h)
{
    ASSERT_GAME_THREAD();
    if (tile_block_ids == nullptr || tiles_x <= 0 || tiles_y <= 0)
        return;

    m_cmd.active = true;
    m_cmd.tiles_x = tiles_x; m_cmd.tiles_y = tiles_y;
    m_cmd.dst_x = dst_x; m_cmd.dst_y = dst_y;
    m_cmd.tile_w = tile_w; m_cmd.tile_h = tile_h;
    m_cmd.tile_block_ids.assign(tile_block_ids, tile_block_ids + (size_t)tiles_x * (size_t)tiles_y);
}

void GLZoomBoxTilesPass::FlipBuffers()
{
    ASSERT_GAME_THREAD();
    m_rt_cmd = std::move(m_cmd);
    // `active` is a scalar -- std::move leaves it unchanged in the moved-from
    // object (same hazard IRImagePresentCmd's own comment documents). Reset
    // explicitly so a frame with no Submit() call starts clean rather than
    // replaying stale state.
    m_cmd = IRZoomBoxTilesCmd{};
}

void GLZoomBoxTilesPass::Resolve(int screen_w, int screen_h)
{
    ASSERT_RENDER_THREAD();
    if (!IsActiveRT() || !m_resource_mapper || !m_tile_atlas || !m_tile_atlas->IsInitialized())
        return;

    const GLProgram* shader_prog = m_resource_mapper->ResolveProgram(m_shader_handle);
    const GLTexture* atlas_tex = m_resource_mapper->ResolveTexture(m_tile_atlas->GetAtlasTextureArray());
    const GLTexture* palette_tex = m_resource_mapper->ResolveTexture(m_palette_tex_handle);
    const GLGeometryBuffer* geom = m_resource_mapper->ResolveGeometryBuffer(m_geom_handle);
    if (!shader_prog || !atlas_tex || !palette_tex || !geom || screen_w <= 0 || screen_h <= 0)
        return;

    m_vertex_scratch.clear();
    m_vertex_scratch.reserve((size_t)m_rt_cmd.tiles_x * (size_t)m_rt_cmd.tiles_y * 6 * 5);

    for (int ty = 0; ty < m_rt_cmd.tiles_y; ++ty)
    {
        for (int tx = 0; tx < m_rt_cmd.tiles_x; ++tx)
        {
            const uint16_t tile_id = m_rt_cmd.tile_block_ids[(size_t)ty * m_rt_cmd.tiles_x + tx];
            if (tile_id == 0xFFFF)
                continue;

            const int variation  = tile_id / TEXTURE_BLOCKS_COUNT;
            const int tile_local = tile_id % TEXTURE_BLOCKS_COUNT;
            float u0, v0, u1, v1;
            TileAtlasPacker::GetTileUV(tile_local, &u0, &v0, &u1, &v1);
            const float layer = (float)variation;

            const float x0 = (float)(m_rt_cmd.dst_x + tx * m_rt_cmd.tile_w);
            const float y0 = (float)(m_rt_cmd.dst_y + ty * m_rt_cmd.tile_h);
            const float x1 = x0 + (float)m_rt_cmd.tile_w;
            const float y1 = y0 + (float)m_rt_cmd.tile_h;

            const float verts[6][5] = {
                { x0, y0, u0, v0, layer },
                { x1, y0, u1, v0, layer },
                { x1, y1, u1, v1, layer },
                { x0, y0, u0, v0, layer },
                { x1, y1, u1, v1, layer },
                { x0, y1, u0, v1, layer },
            };
            for (const auto& row : verts)
                for (float f : row)
                    m_vertex_scratch.push_back(f);
        }
    }

    if (m_vertex_scratch.empty())
        return;

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);

    glUseProgram(shader_prog->id);
    glUniform2f(m_loc_screen_size, (float)screen_w, (float)screen_h);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas_tex->id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, palette_tex->id);

    glBindVertexArray(geom->vao);
    glBindBuffer(GL_ARRAY_BUFFER, geom->vbo);
    // Orphan + refill: avoids the driver stalling this frame's batch on last
    // frame's still-in-flight draw of the same buffer.
    glBufferData(GL_ARRAY_BUFFER, (long)(m_vertex_scratch.size() * sizeof(float)), nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (long)(m_vertex_scratch.size() * sizeof(float)), m_vertex_scratch.data());
    glDrawArrays(GL_TRIANGLES, 0, (int)(m_vertex_scratch.size() / 5));

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

/******************************************************************************/
