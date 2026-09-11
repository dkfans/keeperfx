/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file IWorldViewRenderer.h
 *     Abstract interface for 3D world-view rendering.
 * @par Purpose:
 *     Separates the 3D world-view rendering pipeline (bucket submission + rasterizer)
 *     from the rest of the game. The bucket-fill step (geometry walk, column
 *     traversal, polygon-add calls) is platform-neutral and stays unchanged.
 *     Only the draw step — what is done with the accumulated bucket list —
 *     is abstracted here.
 *
 * @par Design:
 *     Three calls per frame for each rendered view:
 *       1. BeginWorldPass()  — bind the target framebuffer and setup rasterizer
 *       2. [game code fills the bucket list via draw_view() / draw_frontview_engine()]
 *       3. DrawIsometricView() or DrawFrontView() — rasterize the bucket list
 *
 *     Software implementation:
 *       BeginWorldPass     → setup_vecs(framebuf, NULL, pitch, w, h)
 *       DrawIsometricView  → display_drawlist()
 *       DrawFrontView      → display_fast_drawlist(cam)
 *
 *     Future GPU implementation:
 *       BeginWorldPass     → begin GPU geometry pass
 *       DrawIsometricView  → submit bucket list as GPU draw calls
 *       DrawFrontView      → submit front-view bucket list as GPU draw calls
 *
 */
/******************************************************************************/
#pragma once

#include <cstdint>

// Forward declarations to avoid pulling renderer IR into every includer.
struct Camera;
struct WorldCommandBuffers;
struct IRWorldShadowCmd;
struct IRWorldLensCmd;

/******************************************************************************/

class IWorldViewRenderer {
public:
    virtual ~IWorldViewRenderer() = default;

    // =========================================================================
    // Frame setup

    /** Configure the rasterizer for the world view.  Must be called before any
     *  bucket-add calls for the current view.  Each backend acquires whatever
     *  target it needs internally (the software backend its CPU framebuffer, the
     *  GPU backend nothing) — no pixel pointer crosses this interface.
     *  @param w         Viewport width in pixels.
     *  @param h         Viewport height in pixels.
     *  @param vp_x      Viewport left edge in screen pixels (0 if no sidebar).
     *  @param vp_y      Viewport top edge in screen pixels (0 if no sidebar). */
    virtual void BeginWorldPass(int w, int h, int vp_x, int vp_y) = 0;

    // =========================================================================
    // Draw

    /** Rasterize the isometric/1st-person bucket list into the bound framebuffer.
     *  Wraps display_drawlist() on the software path. */
    virtual void DrawIsometricView() = 0;

    /** Rasterize the front-view bucket list into the bound framebuffer.
     *  Wraps display_fast_drawlist(cam) on the software path.
     *  @param cam  Camera used for front-view projection. */
    virtual void DrawFrontView(struct Camera* cam) = 0;

    // =========================================================================
    // Deferred resolution (software)

    /** Realise a pending recorded world pass into the current WScreen.
     *  @return 1 if a world was drawn, 0 if none was pending. */
    virtual int ResolveDeferredWorld() { return 0; }

    /** Re-lay the last recorded world (retained bucket list) for present-only
     *  palette-fade frames that re-present without a redraw. */
    virtual void ReexecuteDeferredWorld() {}

    /** Flag the currently-recorded world pass as a possession lens capture, so
     *  ResolveDeferredWorld() routes it through the lens buffer + distort. */
    virtual void MarkDeferredWorldAsLensCapture() {}

    /** Submit this frame's active possession-lens state (built by
     *  LensManager::BuildActiveGPULensCmd() -- see the RendererManager.cpp
     *  bridge). GPU backends store it and bracket their next world-geometry
     *  pass with a scene-FBO capture + distortion composite when
     *  @p cmd.active is set. Software backends leave this as a no-op -- the
     *  CPU lens path (draw_creature_view() -> LensManager::Draw()) is
     *  entirely separate and unaffected. Game-thread only. */
    virtual void SubmitPossessionLens(const struct IRWorldLensCmd& /*cmd*/) {}

    // =========================================================================
    // Info

    /** Human-readable backend name (e.g. "SOFTWARE", "VITA_GPU"). */
    virtual const char* GetName() const = 0;

    // =========================================================================
    // Keeper sprite (world-space billboarded sprites)

    /** Submit a keeper-sprite (creature/object/shadow) for GPU rendering.
     *  dst_x/y/w/h = screen destination rect (pixels), computed from the
     *                sprite's FULL (unclipped) src_h -- see content_h.
     *  data        = raw RLE palette-index sprite data.
     *  src_w/h     = sprite source dimensions (full content -- also the
     *                atlas decode/cache height; always pass the unclipped
     *                value here so a cache hit never has fewer rows decoded
     *                than a later, unclipped draw of the same sprite needs).
     *  content_h   = visible rows out of src_h for THIS draw (Beat 4:
     *                water/lava clipping) -- <= src_h; backends scale both
     *                the destination rect height and the UV V-range by
     *                content_h/src_h. Pass == src_h for "no clipping" (every
     *                caller except draw_keepersprite()'s
     *                water_source_cutoff case).
     *  draw_flags  = caller draw flags at time of call (flip, transpar, remap, additive).
     *  remap       = colour remap table (may be NULL).
     *  sprite_id   = frame-resolved global sprite index (stable cache key; -1 = unknown).
     *  Returns 1 if the GPU handled it (CPU blit should be skipped), 0 to fall back. */
    virtual int SubmitKeeperSprite(int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
                                   const unsigned char* data, int src_w, int src_h,
                                   int32_t content_h,
                                   unsigned int draw_flags, const unsigned char* remap,
                                   int32_t sprite_id)
    {
        (void)dst_x; (void)dst_y; (void)dst_w; (void)dst_h;
        (void)data;  (void)src_w; (void)src_h; (void)content_h;
        (void)draw_flags; (void)remap; (void)sprite_id;
        return 0;
    }

    /** Begin IR capture for one world-sprite entry (one thing) at the given
     *  depth bucket */
    virtual int BeginWorldSpriteCapture(int32_t /*bucket_idx*/) { return 0; }

    /** Whether this backend wants world sprites submitted through
     *  SubmitKeeperSprite() at all */
    virtual int UsesFillTimeWorldSubmit() const { return 0; }

    /** Clear the keeper sprite atlas (called between levels).
     *  GPU backends should flush their per-level sprite cache here.
     *  Default: no-op. */
    virtual void ClearKeeperSpriteAtlas() {}

    /** Preload all known keeper sprites into the atlas so no decode/upload
     *  happens during gameplay.  Call after init_custom_sprites() completes
     *  (e.g. on the loading screen).  Default: no-op. */
    virtual void PreloadKeeperSpriteAtlas() {}

    /** Re-upload the tile atlas's animated rows (lava, water, dig-tag
     *  cross-hatch, etc.).  Call once per game tick, right after
     *  update_animating_texture_maps().  Default: no-op. */
    virtual void UpdateAnimatedTiles() {}

    /** Notify the renderer of the current OS-window dimensions.
     *  Called from RendererOpenGL::BeginFrame_GL() whenever the screen size
     *  changes.  GPU backends use this to update projection matrices and
     *  viewport dimensions.  Default: no-op. */
    virtual void SetScreenSize(int /*w*/, int /*h*/) {}

    /** Supply the active 256-colour VGA palette (768 bytes: R,G,B × 256).
     *  Called by RendererSetPaletteForRenderers() on the game thread.
     *  Only a pointer copy — safe to call without holding the GL context.
     *  Default: no-op. */
    virtual void SetPaletteSource(const uint8_t* /*palette*/) {}

    /** Set the active world IR write buffer for this frame.
     *  GPU backends use this as the target for game-thread world submissions.
     *  Default: no-op. */
    virtual void SetWorldCommandBuffers(struct WorldCommandBuffers* /*cmds*/) {}

    /** Submit a creature-shadow world command during the bucket walk.
     *  Returns 1 if captured by the active hardware renderer, 0 to fall back. */
    virtual int SubmitWorldShadowCmd(const struct IRWorldShadowCmd& /*cmd*/) { return 0; }

    // ── IR (Intermediate Representation) dispatch ──────────────────────────────

    /** Execute GPU world rendering from the read-side IR command buffer.
     *
     *  Called from RendererOpenGL::EndFrame_GL() inside the lens-FBO bracket
     *  (not from RenderGraph::Execute()) because the output FBO must be bound
     *  before the world is drawn and unbound afterwards for post-processing.
     *
     *  GPU backends override this to submit world geometry to the GPU.
     *  The base no-op is correct for software renderers.
     *
     *  Forward-declared WorldCommandBuffers so IWorldViewRenderer.h does not
     *  pull in the full WorldCommands.h include chain. */
    virtual void ExecuteFromIR(const struct WorldCommandBuffers& /*cmds*/) {}
};

/******************************************************************************/
