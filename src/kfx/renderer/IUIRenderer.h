/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file IUIRenderer.h
 *     The UI drawing a backend has to provide.
 * @par Design:
 *     The engine's Lb* entry points route here, so this is where a backend
 *     decides how a draw is realised. The defaults are the software path, which
 *     calls the existing raster; a GPU backend overrides what it accelerates.
 *
 *     Each Submit* either appends an IR command (when a write buffer is bound
 *     via SetUICommandBuffers) or draws immediately (when it is not) -- the
 *     two halves of the same call, so binding a buffer never changes what is
 *     drawn, only when.
 */
/******************************************************************************/
#pragma once

#include "kfx/renderer/DrawState.h"
#include "kfx/renderer/SpriteHandle.h"
#include "kfx/renderer/ir/UICommands.h" // IRUILayer
#include "bflib_basics.h"
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

struct TbSprite;
struct TbSpriteSheet;
struct UICommandBuffers;

/******************************************************************************/

class IUIRenderer {
public:
    virtual ~IUIRenderer() = default;

    // Sprite handle registry -- IR commands store a handle, not a raw
    // TbSprite*, so a deferred command can't dangle if the sheet reloads
    // between submission and replay.
    void RegisterSpriteHandle(SpriteHandle h, const struct TbSprite* spr);
    virtual SpriteHandle ResolveSprite(const struct TbSprite* spr);
    virtual void RegisterSpriteSheet(const struct TbSpriteSheet* sheet);

    /** Drops every cached TbSprite*->handle mapping. Called
     *  whenever any sprite sheet frees, since a freed sheet's memory can be
     *  reused by the next one loaded, and a stale pointer-keyed cache entry
     *  would alias the new sheet's sprite onto the old one's atlas pixels.
     *  Returns the number of entries cleared. */
    int32_t ClearSpriteHandleCache();

    virtual TbResult SubmitRawSprite(int32_t x, int32_t y, const struct TbSprite* spr,
                                     KfxDrawState state);
    virtual TbResult SubmitRawSpriteOneColour(int32_t x, int32_t y, const struct TbSprite* spr,
                                              unsigned char colour, KfxDrawState state);

    // Drawn at an explicit size rather than the sprite's own.
    virtual TbResult SubmitRawSpriteScaled(int32_t x, int32_t y, const struct TbSprite* spr,
                                           int32_t w, int32_t h, KfxDrawState state);
    virtual TbResult SubmitRawSpriteScaledOneColour(int32_t x, int32_t y, const struct TbSprite* spr,
                                                    int32_t w, int32_t h, unsigned char colour,
                                                    KfxDrawState state);
    virtual int      SubmitRawSpriteScaledRemap(int32_t x, int32_t y, const struct TbSprite* spr,
                                                int32_t w, int32_t h, const unsigned char* cmap,
                                                KfxDrawState state);

    virtual void SubmitSolidBox(int32_t x, int32_t y, int32_t w, int32_t h,
                                uint8_t colour_idx, KfxDrawState state);

    /** Tile the GUI slab texture over a rect. */
    virtual void SubmitSlabBackground(int32_t x, int32_t y, int32_t w, int32_t h);

    virtual void UpdateSlabTexture(const unsigned char* /*data*/, int /*dim*/) {}

    /** Acquire a renderer-owned size*size palette-index scratch buffer
     *  (zero-filled; index 0 = "nothing drawn here"), always non-null for a
     *  valid size. The caller (the minimap draw functions) writes into it
     *  directly and calls SubmitMinimap() once done -- this is bulk raster
     *  data, not a per-command IR submission, same shape as UpdateSlabTexture()
     *  above. */
    virtual uint8_t* AcquireMinimapBuffer(int size);

    /** Display the buffer filled via AcquireMinimapBuffer() at (screen_x,
     *  screen_y). shape_start/shape_end are the per-row circular-mask bounds
     *  (size entries each, screen_x-relative) -- a backend that composites the
     *  minimap over the panel art itself (see IRenderer::BackendCapabilities::
     *  compositesMinimapBackground) doesn't need them, since anything the
     *  caller never wrote stays index 0 and is discarded like any other
     *  transparent sprite texel; the CPU default uses them to mask its blit so
     *  panel art outside the circle survives. */
    virtual void SubmitMinimap(int screen_x, int screen_y, int size,
                               const int32_t* shape_start, const int32_t* shape_end);

    virtual void SetWorldOverlay(float ndc_z) { m_world_overlay_active = true; m_world_overlay_z = ndc_z; }
    virtual void ClearWorldOverlay() { m_world_overlay_active = false; }
    virtual void SetWorldOverlayFlat(float ndc_z) { m_world_overlay_flat_active = true; m_world_overlay_flat_z = ndc_z; }
    virtual void ClearWorldOverlayFlat() { m_world_overlay_flat_active = false; }

    /** Top-overlay: subsequent submissions land in IRUILayer::Overlay,
     *  drawn dead-last over GameUI (zoom-box corner frames). Ambient state,
     *  same shape as SetWorldOverlay() above. */
    virtual void SetTopOverlay() { m_top_overlay_active = true; }
    virtual void ClearTopOverlay() { m_top_overlay_active = false; }

    /** Game viewport rect (screen pixels), set once per frame from the
     *  engine window. WorldOverlay/WorldOverlayFlat coordinates are
     *  submitted relative to it (the CPU immediate path resolves that
     *  through its own graphics-window pointer); a bound IR buffer gets
     *  this added back here, so IR always carries absolute-screen
     *  coordinates and no backend needs to know about the viewport. */
    void SetGameViewport(int32_t x, int32_t y, int32_t w, int32_t h);

    virtual void BeginZoomBoxOverlay(int32_t x, int32_t y, int32_t w, int32_t h);
    virtual void EndZoomBoxOverlay(int32_t x, int32_t y, int32_t w, int32_t h);

    /** Flush the Overlay layer (IRUILayer::Overlay), drawn dead-last after
     *  GameUI. CPU default: no-op -- software already drew everything
     *  immediately, in the order Submit* was called. */
    virtual void DrawFrontOverlay() {}

    /** Open the IR write window for this frame; nullptr closes it and returns
     *  the Submit* calls to drawing immediately. */
    virtual void SetUICommandBuffers(UICommandBuffers* cmds);

    virtual const char* GetName() const { return "UI"; }

protected:
    mutable std::mutex m_handle_mutex;
    std::unordered_map<SpriteHandle, const struct TbSprite*> m_handle_to_sprite;
    std::unordered_map<const struct TbSprite*, SpriteHandle> m_sprite_to_handle;
    uint32_t m_next_handle = 0;

    /** Bound write buffer, or null to draw immediately. */
    UICommandBuffers* m_ui_write_cmds = nullptr;

    bool  m_world_overlay_active      = false;
    float m_world_overlay_z           = 0.5f;
    bool  m_world_overlay_flat_active = false;
    float m_world_overlay_flat_z      = 0.5f;
    bool  m_top_overlay_active        = false;

    int32_t m_game_vp_x   = 0;
    int32_t m_game_vp_y   = 0;
    int32_t m_game_vp_w   = 0;
    int32_t m_game_vp_h   = 0;
    bool    m_game_vp_set = false;

    /** Backing store for the CPU default AcquireMinimapBuffer(). */
    std::vector<uint8_t> m_minimap_cpu_buf;
    int                  m_minimap_cpu_size = 0;

    IRUILayer ComputeCurrentLayer() const
    {
        if (m_top_overlay_active)        return IRUILayer::Overlay;
        if (m_world_overlay_active)      return IRUILayer::WorldOverlay;
        if (m_world_overlay_flat_active) return IRUILayer::WorldOverlayFlat;
        return IRUILayer::GameUI;
    }

    float ComputeCurrentNdcZ() const
    {
        if (m_world_overlay_active)      return m_world_overlay_z;
        if (m_world_overlay_flat_active) return m_world_overlay_flat_z;
        return 0.5f;
    }

    /** Adds the game-viewport origin to x/y for WorldOverlay/WorldOverlayFlat
     *  IR submissions, so a deferred command lands in the same absolute-screen
     *  coordinate an immediate draw reaches through the graphics-window
     *  pointer. No-op for every other layer or when no viewport is set. */
    void ApplyGameViewportOffset(IRUILayer layer, int32_t& x, int32_t& y) const
    {
        if (!m_game_vp_set) return;
        if (layer != IRUILayer::WorldOverlay && layer != IRUILayer::WorldOverlayFlat) return;
        x += m_game_vp_x;
        y += m_game_vp_y;
    }
};

/******************************************************************************/
