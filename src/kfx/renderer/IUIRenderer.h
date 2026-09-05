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

struct TbSprite;
struct TbSpriteSheet;
struct UICommandBuffers;
struct TextCommandBuffers;
class  ITextRenderer;

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

    /** Beat 5: notify the backend that the slab background tile buffer
     *  (gui_slab, GUI_SLAB_DIMENSION -- see gui_draw.c/.h) has been
     *  (re)loaded, e.g. from LoadVRes256Data()/LoadMcgaData() (vidmode.c).
     *  Default no-op: software reads gui_slab directly every draw
     *  (draw_slab64k_background_immediate()), nothing to cache. GL caches it
     *  as a GL_REPEAT texture -- see GLUIRenderer::UpdateSlabTexture(). */
    virtual void UpdateSlabTexture(const unsigned char* /*data*/, int /*dim*/) {}

    // Beat 5: world-positioned UI content (creature status, room flags,
    // floating damage/gold text) is bracketed with these around the
    // existing Submit* calls (ported from develop's SetWorldOverlay()/
    // ClearWorldOverlay()/SetWorldOverlayFlat()/ClearWorldOverlayFlat()) so
    // it draws through the WorldOverlay/WorldOverlayFlat IR layers instead
    // of always landing in GameUI. Ambient state, not a Submit* parameter --
    // every Submit* call made while active picks it up automatically via
    // CurrentLayer()/CurrentNdcZ() below. Default bodies just set/clear the
    // state directly; no backend needs to override these, only to read the
    // ambient state (already done for it, inside each Submit* body).
    virtual void SetWorldOverlay(float ndc_z) { m_world_overlay_active = true; m_world_overlay_z = ndc_z; }
    virtual void ClearWorldOverlay() { m_world_overlay_active = false; }
    virtual void SetWorldOverlayFlat(float ndc_z) { m_world_overlay_flat_active = true; m_world_overlay_flat_z = ndc_z; }
    virtual void ClearWorldOverlayFlat() { m_world_overlay_flat_active = false; }

    /** Open the IR write window for this frame; nullptr closes it and returns
     *  the Submit* calls to drawing immediately. */
    virtual void SetUICommandBuffers(UICommandBuffers* cmds);

    /** Replay one frame of UI and text commands through the immediate draw
     *  path, interleaved by their shared submission order, so the result
     *  matches what drawing them as they arrived would have produced. */
    virtual void ReplayMergedFromIR(const UICommandBuffers& ui,
                                    const TextCommandBuffers& text,
                                    ITextRenderer* text_renderer);

    virtual const char* GetName() const { return "UI"; }

protected:
    // Guards the two maps + m_next_handle. For GL (P5.6), text glyphs resolve
    // lazily at draw time -- on the render thread, during DrawFromIR replay
    // -- while UI sprites resolve at submit time on the game thread; the two
    // touch the same registry concurrently. Software never contends (single
    // thread, always), so this is an uncontended, near-free lock there.
    mutable std::mutex m_handle_mutex;
    std::unordered_map<SpriteHandle, const struct TbSprite*> m_handle_to_sprite;
    std::unordered_map<const struct TbSprite*, SpriteHandle> m_sprite_to_handle;
    uint32_t m_next_handle = 0;

    /** Bound write buffer, or null to draw immediately. */
    UICommandBuffers* m_ui_write_cmds = nullptr;

    // Beat 5: ambient world-overlay state, set/cleared by SetWorldOverlay()/
    // ClearWorldOverlay()/SetWorldOverlayFlat()/ClearWorldOverlayFlat().
    // ComputeCurrentLayer()/ComputeCurrentNdcZ() (ported from develop's
    // ComputeIRLayer()) are called from every Submit* body to stamp
    // cmd.layer/cmd.ndc_z before Append(). WorldOverlay takes priority over
    // WorldOverlayFlat if both were somehow active (matches develop's own
    // precedence, though callers are expected to bracket one at a time).
    bool  m_world_overlay_active      = false;
    float m_world_overlay_z           = 0.5f;
    bool  m_world_overlay_flat_active = false;
    float m_world_overlay_flat_z      = 0.5f;

    IRUILayer ComputeCurrentLayer() const
    {
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
};

/******************************************************************************/
