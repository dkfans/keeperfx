/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file WorldCommands.h
 *     Intermediate-representation command types for the world renderer.
 * @par Purpose:
 *     These structs are written by the game thread (bucket walk in
 *     DrawIsometricView / DrawFrontView) and read by the render thread during
 *     RenderGraph::Execute().  They capture all data needed to render world
 *     geometry and sprites without accessing game-engine globals on the render
 *     thread.
 *
 *     Geometry is already converted to NDC float by the time it enters the IR;
 *     the CPU rasterizer uses the same structs as a compact record of what to
 *     draw, projecting back to screen coordinates via FrameState.
 */
/******************************************************************************/
#pragma once

#include <cstdint>
#include <cstddef>
#include <utility>
#include <vector>
#include "kfx/renderer/WorldVertex.h"
#include "kfx/renderer/FlatPolyVertex.h"
#include "kfx/renderer/EngineVertex.h"

/******************************************************************************/

/** Render layer / priority within the world pass.
 *  Higher values are drawn on top (painters order for world geometry). */
enum class WorldCmdLayer : uint8_t
{
    Geometry   = 0,  /**< Standard tiles, front-view tiles, flat polys. */
    Shadows    = 1,  /**< Creature shadow quads (rendered before sprites). */
    Sprites    = 2,  /**< Keeper sprites, jonty sprites, rotable sprites. */
    WorldUI    = 3,  /**< Depth-tested UI: status icons, floating text, flags. */
};

/******************************************************************************/
// Tile / polygon geometry
/******************************************************************************/

/** Draw a batch of tile triangles already packed into the world vertex buffer.
 *  CMD_TILES in the existing GLWorldViewRenderer::DrawCmd vocabulary. */
struct IRWorldTileBatchCmd
{
    WorldCmdLayer layer      = WorldCmdLayer::Geometry;
    int           vert_start = 0;   /**< First index in the renderer's vertex buffer. */
    int           vert_count = 0;   /**< Number of vertices (must be multiple of 3). */
    uint32_t      sort_key   = 0;   /**< Bucket index (large = far, small = near). */
};

/** Draw front-view axis-aligned tile quad. */
struct IRWorldTexQuadCmd
{
    WorldCmdLayer layer        = WorldCmdLayer::Geometry;
    uint8_t       orient       = 0;
    int32_t       texture_idx  = 0;
    int32_t       texture_x    = 0;
    int32_t       texture_y    = 0;
    int32_t       zoom_x       = 0;
    int32_t       zoom_y       = 0;
    int32_t       shade[4]     = {};  /**< Per-corner shade_intensity. */
    int32_t       marked_mode  = 0;
    uint32_t      sort_key     = 0;
};

/** Draw a batch of flat-colour triangles (QK_PolyMode0 / BasicPolygon).
 *  Vertices are screen-pixel coords; the backend converts to NDC. */
struct IRWorldFlatPolyBatchCmd
{
    WorldCmdLayer layer      = WorldCmdLayer::Geometry;
    int           vert_start = 0;   /**< First index in the flat-poly vertex buffer. */
    int           vert_count = 0;
    uint32_t      sort_key   = 0;
};

/******************************************************************************/
// Sprites
/******************************************************************************/

/** Draw a single palette-indexed keeper sprite at world-screen coordinates.
 *
 *  Captured on the game thread during the bucket walk; the render thread
 *  decodes the RLE, does atlas/CLUT management, and issues the GL draw.
 *
 *  @p data points to stable sprite storage, while remap state is copied into
 *  this command so render-thread replay does not depend on mutable globals.
 */
struct IRWorldKeeperSpriteCmd
{
    WorldCmdLayer         layer         = WorldCmdLayer::Sprites;
    int32_t               dst_x         = 0;   /**< Screen destination left. */
    int32_t               dst_y         = 0;   /**< Screen destination top. */
    int32_t               dst_w         = 0;   /**< Destination width. */
    int32_t               dst_h         = 0;   /**< Destination height. */
    int32_t               src_w         = 0;   /**< Source sprite width. */
    int32_t               src_h         = 0;   /**< Source sprite height -- also the atlas decode/cache height (always the sprite's full, unclipped content, so a later full-height draw of the same sprite_id never samples missing rows). */
    /** Visible rows out of @p src_h for THIS draw (Beat 4: water/lava
     *  clipping) -- <= src_h; the destination rect's drawn height and the UV
     *  V-range both scale down by content_h/src_h, matching the CPU path's
     *  LbSpriteDrawUsingScalingData()/DrawAlphaSpriteUsingScalingData()
     *  behaviour of feeding fewer source rows into the same ambient scale
     *  ratio. Equal to src_h (no clipping) for every submitter except
     *  draw_keepersprite()'s water_source_cutoff case. */
    int32_t               content_h     = 0;
    uint32_t              draw_flags    = 0;   /**< Sprite draw flags (Lb_SPRITE_*). */
    /** RLE-compressed pixel data (keepersprite_array — stable for level lifetime). */
    const unsigned char*  data          = nullptr;
    /** Frame-resolved global sprite index (keepsprite[] / keepersprite_add[]).
     *  Atlas cache key — stable across sprite-heap eviction, unlike @p data.
     *  -1 = unknown (submit path without an id); such sprites bypass the atlas. */
    int32_t               sprite_id     = -1;
    /** 256-byte remap snapshot, valid when remap_enabled != 0. */
    uint8_t               remap_table[256] = {};
    uint8_t               remap_enabled = 0;
    float                 z_ndc         = 0.0f;  /**< Pre-computed NDC depth (half-bucket bias). */
    int8_t                owner         = -1;    /**< Player owner index (-1 = none). */
    int8_t                wants_outline =  0;    /**< Non-zero if depth-fail outline is wanted. */
    uint32_t               sort_key      = 0;
};

/******************************************************************************/
// Shadows
/******************************************************************************/

/** Draw a single creature shadow quad. */
struct IRWorldShadowCmd
{
    WorldCmdLayer    layer         = WorldCmdLayer::Shadows;
    EnginePolyVertex verts[4]      = {};  /**< Screen-px coords + fixed-point UV. */
    unsigned short   anim_sprite   = 0;
    short            angle         = 0;
    unsigned char    current_frame = 0;
    int              tex_w         = 0;
    int              tex_h         = 0;
    float            darkness      = 1.0f;  /**< Alpha for multiply-blend. */
    float            ndc_z         = 0.0f;  /**< NDC depth for depth test. */
    int32_t          wx            = 0, wy = 0, wz = 0;  /**< World-space caster origin. */
    uint32_t         sort_key      = 0;
    unsigned char    is_circle     = 0;  /**< 1 = draw pre-baked circle, 0 = decode sprite. */
};

// TODO remove? this isn't exactly a world command. probably should be in PostProcessCommand like I had in develop.

/******************************************************************************/
// Possession lens
/******************************************************************************/

/** Which single pixel-writing lens effect is active this frame. Decoupled
 *  from kfx::lense's LensEffectType (a different module) -- LensManager.cpp
 *  is the only place that maps between the two. */
enum class LensPixelEffectType : uint8_t
{
    None = 0,
    Mist,
    Displacement,
    Flyeye,
    Overlay,
};

/** One frame's active possession-lens state, built on the game thread by
 *  LensManager::BuildActiveGPULensCmd() and consumed by the GL world-view
 *  renderer's capture/composite bracket. Mirrors software's LensManager::Draw()
 *  precedence: only the single winning pixel effect (if any) is carried,
 *  since software itself never blends multiple enabled effects -- see the
 *  P5.8a plan for why. Pixel buffers are owned copies (std::vector), not
 *  pointers into LensEffect-owned storage, because those buffers can be
 *  reassigned/freed on the next SetLens() call (confirmed hazard, see
 *  OverlayEffect::LoadOverlay()) -- unsafe to reference across the
 *  game-thread/render-thread gap. std::move()'d by FlipBuffers(), same as
 *  every other per-frame IR vector in this file. */
struct IRWorldLensCmd
{
    bool active = false;               /**< False when possession mode isn't active this frame. */
    LensPixelEffectType type = LensPixelEffectType::None;

    // Mist (type == Mist)
    float   mist_pos_x = 0.0f, mist_pos_y = 0.0f;  /**< Primary layer offset, 0..255 wrapped. */
    float   mist_sec_x = 0.0f, mist_sec_y = 0.0f;  /**< Secondary layer offset, 0..255 wrapped. */
    uint8_t mist_lightness = 0;                     /**< 0..63, matches LensConfig::mist_lightness. */
    std::vector<uint8_t> mist_pixels;                /**< 256x256 mist density texture. */
    uint32_t mist_version = 0;   /**< Bumped only when mist_pixels changes (lens switch). */

    // Displacement / Flyeye (type == Displacement or Flyeye) -- share the
    // same remap-table representation: interleaved src_x,src_y per
    // destination pixel, in the viewport-local coordinate frame (matches
    // DisplaceLookupEntry/FlyeyeLookupEntry exactly, no reshaping needed).
    std::vector<int16_t> remap_pixels;   /**< remap_w*remap_h*2 entries (x,y pairs). */
    int      remap_w = 0, remap_h = 0;
    uint32_t remap_version = 0;   /**< Bumped only when the table is rebuilt (resolution change). */

    // Overlay (type == Overlay)
    std::vector<uint8_t> overlay_pixels;   /**< overlay_w*overlay_h, 8bpp palette index. */
    int      overlay_w = 0, overlay_h = 0;
    float    overlay_alpha = 0.0f;         /**< 0..1 (LensConfig::overlay_alpha / 256). */
    uint32_t overlay_version = 0;   /**< Bumped only when overlay_pixels changes (lens switch). */

    // Palette (separate side channel -- LCF_HasPalette never touches pixels,
    // it mutates the active palette; independent of `type`/`active` above,
    // since a lens can combine a palette swap with a pixel effect).
    bool    has_palette = false;
    uint8_t palette[768] = {};

    // Real on-screen viewport rect (sidebar excluded) within the captured
    // full-screen buffer -- the composite target. Matches
    // draw_creature_view()'s capture-full/composite-viewport-only asymmetry:
    // the world is captured at full screen resolution, but only this
    // sub-rect is ever written to the real backbuffer.
    int viewport_x = 0, viewport_y = 0, viewport_w = 0, viewport_h = 0;
};

/******************************************************************************/
// Parchment transition
// TODO remove? this isn't exactly a world command. probably should be in PostProcessCommand like I had in develop.
/******************************************************************************/

/** One frame's active parchment-transition */
struct IRMapFadeCmd
{
    bool  active = false;           /**< False on any frame SubmitStep() wasn't called this transition. */
    float step = 0.0f;              /**< 0..32, matches software's palette_fade_step_map exactly (no interpolation). */
    bool  capture_pending = false;  /**< True only on the one frame the transition just started. */
};

/******************************************************************************/

/** Combined per-frame world command buffers. */
#include "kfx/renderer/ir/IRCommandBuffer.h"

struct WorldCommandBuffers
{
    IRCommandBuffer<IRWorldTileBatchCmd>     tiles;
    IRCommandBuffer<IRWorldTexQuadCmd>       tex_quads;
    IRCommandBuffer<IRWorldFlatPolyBatchCmd> flat_polys;
    IRCommandBuffer<IRWorldKeeperSpriteCmd>  keeper_sprites;
    IRCommandBuffer<IRWorldShadowCmd>        shadows;

    // Vertex data backing vert_start/vert_count in tiles and flat_polys commands.
    // Owned here so any backend's ExecuteWorldFromIR can access them without
    // coupling to backend-private arrays.
    std::vector<WorldVertex>    tile_verts;
    std::vector<FlatPolyVertex> flat_poly_verts;

    void Reset()
    {
        tiles.Reset();
        tex_quads.Reset();
        flat_polys.Reset();
        keeper_sprites.Reset();
        shadows.Reset();
        tile_verts.clear();
        flat_poly_verts.clear();
    }

    void Reserve(size_t tiles_n, size_t sprites_n, size_t shadows_n)
    {
        tiles.Reserve(tiles_n);
        flat_polys.Reserve(tiles_n / 4);
        keeper_sprites.Reserve(sprites_n);
        shadows.Reserve(shadows_n);
        tile_verts.reserve(65536);
        flat_poly_verts.reserve(8192);
    }

    void Swap(WorldCommandBuffers& other)
    {
        tiles.Swap(other.tiles);
        tex_quads.Swap(other.tex_quads);
        flat_polys.Swap(other.flat_polys);
        keeper_sprites.Swap(other.keeper_sprites);
        shadows.Swap(other.shadows);
        std::swap(tile_verts, other.tile_verts);
        std::swap(flat_poly_verts, other.flat_poly_verts);
    }
};

/******************************************************************************/
