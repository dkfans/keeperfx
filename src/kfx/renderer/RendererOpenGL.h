#ifndef RENDERER_RENDEREROPENGL_H
#define RENDERER_RENDEREROPENGL_H

#include "kfx/renderer/IRenderer.h"
#include "kfx/renderer/IFrameGraphExecutor.h"

// GL-specific state (atlas, shaders, command buffers, ...) lives entirely in
// an opaque Impl defined in the .cpp, so this header -- and every other file
// that includes it -- stays free of GL types.
//
// IFrameGraphExecutor: render_thread_work() no longer inlines the per-frame
// draw sequence itself -- it calls RenderGraph::Execute(*this), the same
// shared, centrally-owned phase order RendererSoftware uses. Only the FG*
// phases this branch actually has content for are overridden below; every
// other phase inherits IFrameGraphExecutor's no-op default, which is exactly
// what "omitting an element should be trivial" means in practice.
class RendererOpenGL : public IRenderer, public IFrameGraphExecutor {
public:
    RendererOpenGL();
    ~RendererOpenGL() override;

    bool Init() override;
    void Shutdown() override;
    const char* GetName() const override { return "OpenGL"; }

    void SetDisplayPalette(const unsigned char* rgb8) override;
    void PresentFrame() override;

    BackendCapabilities GetCapabilities() const override { return BackendCapabilities{ 1 }; }
    bool BeginFrame() override;
    void EndFrame() override;
    bool PresentImage(const struct RendererPresentImageDesc* desc) override;

    // Beat 10: landview zoom-in/out transition (frontzoom_to_point()).
    bool SubmitLandviewZoom(const unsigned char* src_buf, int src_w, int src_h,
                            float center_map_x, float center_map_y,
                            float screen_cx,    float screen_cy,
                            float scale) override;

    class ITextRenderer*        GetTextRenderer() override;
    class IUIRenderer*          GetUIRenderer() override;
    class ICursorLayer*         GetCursorLayer() override;
    class IWorldViewRenderer*   GetWorldViewRenderer() override;

    // Parchment transition (P5.8b). See GLMapFadePass.h for the design.
    void SubmitMapFadeStep(int tick_step, float display_step, bool fading_in) override;
    void BeginParchmentCapture() override;
    void EndParchmentCapture() override;
    bool MapFadeSupportsNativeResolution() const override;
    void BeginSwipeOverlay() override;
    void EndSwipeOverlay() override;

    // IFrameGraphExecutor overrides -- called from render_thread_work(), via
    // RenderGraph::Execute(*this), on the render thread. Each reads the
    // current frame's GLFrameData via m_impl->frames[m_impl->render_idx].
    // FGDrawGameUI draws UI *and* text together (see .cpp for why: this
    // branch's GLUIRenderer::DrawGameUILayerRT() interleaves them by shared
    // sequence number, the same ordering the software renderer's replay
    // relies on -- develop's separate FGExecuteText phase assumes
    // independent draw batches, which these provably are not) --
    // FGExecuteText is left as the inherited no-op.
    // FGDrawWorldSpriteLayer/FGDrawWorldOverlayFlatLayer (Beat 5) flush the
    // WorldOverlay/WorldOverlayFlat quad layers GLUIRenderer::
    // BuildQuadsFromIR() (called from FGClearFrame()) already classified
    // this frame's ui_cmds into -- room-flag poles, floating gold/damage
    // text, creature status (Beat 1's content, now depth-aware).
    void FGClearFrame() override;
    void FGBeginWorldCapture() override;
    void FGExecuteWorld() override;
    void FGFlushSwipeOverlay() override;
    void FGResolveWorldCapture() override;
    void FGExecuteMapFade() override;
    void FGDrawWorldSpriteLayer() override;
    void FGDrawWorldOverlayFlatLayer() override;
    void FGExecuteImagePresents() override;
    void FGDrawGameUI() override;
    void FGCaptureWorldFrameIfPending() override;
    void FGExecuteCursor() override;

    // Beat 7: full-screen tint overlay (pain/possession vignette, death/
    // zoom-to-heart white flash) -- drawn from g_screen_tint (RendererManager),
    // after text/front-overlay, before cursor (matches RenderGraph.cpp's
    // already-correct call order).
    void FGDrawScreenTint() override;

    // Beat 8: glReadPixels-based screenshot capture, run at the frame-graph's
    // existing FGCaptureScreenshot() call site (present-time, before swap).
    bool ScheduleScreenshot(const char* path, int fmt) override;
    void FGCaptureScreenshot() override;

private:
    struct Impl;
    Impl* m_impl = nullptr;

    // SDL_GLContext, kept opaque so this header doesn't need SDL/GL headers.
    void* m_gl_context = nullptr;

    // P5.6: the three RenderThreadManager callbacks. init_fn/cleanup_fn run
    // once each, at Start()/Stop(); work_fn runs once per present, on the
    // render thread that owns the GL context. Kept as named member functions
    // (not lambdas inline at the call site) since each is substantial enough
    // to want its own doc comment -- see RendererOpenGL.cpp.
    void render_thread_init();
    void render_thread_work();
    void render_thread_cleanup();
};

#endif // RENDERER_RENDEREROPENGL_H
