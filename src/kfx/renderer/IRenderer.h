#ifndef RENDERER_IRENDERER_H
#define RENDERER_IRENDERER_H

// Selectable renderer backends.
enum RendererType {
    RENDERER_INVALID  = -1,
    RENDERER_AUTO     = 0,  // pick the best available backend at startup
    RENDERER_SOFTWARE = 1,  // CPU software renderer, SDL display output
    RENDERER_OPENGL   = 2,  // OpenGL backend
};

// Backend-agnostic renderer interface. Grown as drawing migrates behind the seam.
class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual bool Init() = 0;
    virtual void Shutdown() = 0;
    virtual const char* GetName() const = 0;
    
    virtual void SetDisplayPalette(const unsigned char* rgb8) { (void)rgb8; }

    // Clear the whole display to a palette index. Default no-op.
    virtual void ClearScreen(unsigned char colour) { (void)colour; }

    // Present the drawn frame to the window (blit + flip). Default no-op.
    virtual void PresentFrame() {}

    // Lock the CPU framebuffer for drawing; return its pixels + pitch, or nullptr.
    // Backend-internal now -- only RendererBeginFrame()/RendererEndFrame()'s
    // bridge calls these (for backends without a GPU render path). Game code
    // calls BeginFrame()/EndFrame() instead; see below.
    virtual unsigned char* LockFramebuffer(int* out_pitch) { (void)out_pitch; return nullptr; }
    virtual void UnlockFramebuffer() {}

    // Declares whether this backend draws through a real GPU pipeline.
    // Governs whether RendererBeginFrame()'s bridge additionally locks the
    // CPU framebuffer for legacy direct-blit callers (software: yes; GL: no
    // -- GL content goes through the IR/bridge submission calls instead).
    struct BackendCapabilities { int hasGPURenderPath = 0; };
    virtual BackendCapabilities GetCapabilities() const { return BackendCapabilities{}; }

    // Per-frame bracket, replacing the old Lock/Unlock-as-readiness-check
    // pattern. Call once before submitting any drawing for a frame, once
    // after. GL succeeds unconditionally once initialised (no CPU pointer
    // involved -- content is already IR/bridge-submitted); software wraps
    // its own LockFramebuffer()/UnlockFramebuffer().
    virtual bool BeginFrame() = 0;
    virtual void EndFrame() = 0;

    // Present a raw indexed8 image (FMV frame, splash bitmap) at a
    // destination rect. Returns true if the backend drew it (GPU path
    // taken) -- false means the caller must fall back to its own existing
    // CPU blit into lbDisplay.WScreen (valid after BeginFrame() locked it
    // for non-GPU backends). Default: always false (software's answer).
    virtual bool PresentImage(const struct RendererPresentImageDesc* desc) { (void)desc; return false; }

    // Submit the landview zoom-in/out transition frame (Beat 10,
    // frontzoom_to_point()). Returns true if the backend accepted it (GPU
    // path taken) -- false means the caller must run its own CPU zoom loop.
    // Default: always false (software's answer).
    virtual bool SubmitLandviewZoom(const unsigned char* src_buf, int src_w, int src_h,
                                    float center_map_x, float center_map_y,
                                    float screen_cx,    float screen_cy,
                                    float scale)
    {
        (void)src_buf; (void)src_w; (void)src_h; (void)center_map_x; (void)center_map_y;
        (void)screen_cx; (void)screen_cy; (void)scale;
        return false;
    }

    // Save the current frame to a file (fmt: 1=PNG, 2=BMP). Default: unsupported.
    virtual bool ScheduleScreenshot(const char* path, int fmt) { (void)path; (void)fmt; return false; }

    // Sub-renderers. Null when a backend has none, so callers fall back to
    // drawing directly.
    virtual class ITextRenderer*      GetTextRenderer()      { return nullptr; }
    virtual class IUIRenderer*        GetUIRenderer()        { return nullptr; }
    virtual class ICursorLayer*       GetCursorLayer()       { return nullptr; }
    virtual class IWorldViewRenderer* GetWorldViewRenderer() { return nullptr; }

    // Parchment transition (P5.8b). Default no-ops -- software has no GPU
    // pass; its own CPU map_fade() (engine_redraw.c) is unaffected either
    // way, so these are safe to call unconditionally regardless of backend.
    // SubmitMapFadeStep() mirrors the world/lens bridges' shape (game
    // thread, called every frame the transition is active). Begin/EndParchmentCapture()
    // redirect UI/text submission into a separate buffer for exactly one
    // call (prepare_map_fade_buffers()'s redraw_minimal_overhead_view()) --
    // see GLMapFadePass.h for why. MapFadeSupportsNativeResolution() gates
    // gui_parchment.c's software-only resolution cap.
    // `tick_step` is software's own discrete palette_fade_step (0..32, step
    // 4) -- used only to detect the transition's first frame, exactly as
    // before. `display_step` is the (possibly Ft_DeltaTime-interpolated,
    // continuous) value GL actually composites with; software ignores it.
    virtual void SubmitMapFadeStep(int tick_step, float display_step, bool fading_in)
        { (void)tick_step; (void)display_step; (void)fading_in; }
    virtual void BeginParchmentCapture() {}
    virtual void EndParchmentCapture() {}
    virtual bool MapFadeSupportsNativeResolution() const { return false; }

    // Possession-mode swipe-attack overlay (Beat 2). Same shape as Begin/
    // EndParchmentCapture() above -- redirect UI submission into a separate
    // buffer for exactly one call (draw_swipe_graphic(), thing_creature.c),
    // drawn back by the backend at a point that composites correctly (for
    // GL: inside the lens FBO bracket, so it gets lens-distorted like
    // develop's does -- see RendererOpenGL::FGFlushSwipeOverlay()). Default
    // no-op for software: it draws immediately in the same call, already at
    // the right point in the frame, no redirect needed.
    virtual void BeginSwipeOverlay() {}
    virtual void EndSwipeOverlay() {}
};

#endif // RENDERER_IRENDERER_H
