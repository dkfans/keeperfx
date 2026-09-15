/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file GLZoomBoxTilesPass.h
 *     GPU-native render of the zoom box's terrain tiles
 *     (draw_zoom_box_terrain(), gui_parchment.c) as textured quads sampling
 *     the world tile atlas (GLTileAtlas) -- same texture content the
 *     isometric world view samples, no separate decode. Same GT->RT
 *     double-buffered shape as GLImagePresentPass.
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/opengl/GLFunctions.h"
#include "kfx/renderer/GpuResourceHandle.h"
#include <vector>
#include <cstdint>

class GLResourceMapper;
class GLTileAtlas;

/******************************************************************************/

// One zoom-box terrain submission. GT: Submit() copies the tile-block-id
// buffer now (the caller's array is scratch-allocated, not guaranteed to
// outlive this call). RT: FlipBuffers() moves it to the RT-stable copy.
struct IRZoomBoxTilesCmd {
    bool active = false;
    int tiles_x = 0, tiles_y = 0;
    int dst_x = 0, dst_y = 0;
    int tile_w = 0, tile_h = 0;
    // tiles_x * tiles_y, row-major. 0xFFFF = unrevealed -- skipped here; the
    // caller submits those through the ordinary UI solid-box path instead.
    std::vector<uint16_t> tile_block_ids;
};

class GLZoomBoxTilesPass {
public:
    GLZoomBoxTilesPass() = default;
    ~GLZoomBoxTilesPass();

    /** Compile the shader + build the geometry buffer. Safe after construction. */
    bool CompileShaders();

    /** Release all GL resources. Must be called on the render thread. */
    void Shutdown();

    /** True once CompileShaders() has succeeded -- gates
     *  RendererOpenGL::SubmitZoomBoxTiles()'s return value (false means the
     *  caller falls back to its own CPU tile loop). */
    bool IsReady() const { return m_shader_handle != kInvalidGpuResource; }

    void SetResourceMapper(GLResourceMapper* mapper) { m_resource_mapper = mapper; }
    void SetPaletteTexture(GpuResourceHandle tex) { m_palette_tex_handle = tex; }
    /** Not owned; must outlive this. Shared with GLWorldViewRenderer. */
    void SetTileAtlas(GLTileAtlas* atlas) { m_tile_atlas = atlas; }

    // -- Game thread --------------------------------------------------------

    /** Copies tile_block_ids now (see IRZoomBoxTilesCmd's comment). */
    void Submit(const uint16_t* tile_block_ids, int tiles_x, int tiles_y,
               int dst_x, int dst_y, int tile_w, int tile_h);

    /** FlipBuffers()-style move into the render-thread-stable copy, called
     *  from RendererOpenGL::PresentFrame() alongside every other per-frame
     *  command. */
    void FlipBuffers();

    // -- Render thread --------------------------------------------------------

    bool IsActiveRT() const { return m_rt_cmd.active; }

    /** Build one quad per revealed tile and draw them in a single batched
     *  call. */
    void Resolve(int screen_w, int screen_h);

private:
    IRZoomBoxTilesCmd m_cmd;    // GT: written by Submit()
    IRZoomBoxTilesCmd m_rt_cmd; // RT: stable copy after FlipBuffers()

    GLResourceMapper* m_resource_mapper = nullptr;
    GLTileAtlas*       m_tile_atlas = nullptr;

    GpuResourceHandle m_shader_handle = kInvalidGpuResource;
    GLint m_loc_screen_size = -1;
    GpuResourceHandle m_geom_handle = kInvalidGpuResource;

    GpuResourceHandle m_palette_tex_handle = kInvalidGpuResource; // not owned (shared game palette)

    // RT-only scratch, rebuilt each Resolve() call.
    std::vector<float> m_vertex_scratch;
};

/******************************************************************************/
