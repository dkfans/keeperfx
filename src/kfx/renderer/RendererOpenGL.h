#ifndef RENDERER_RENDEREROPENGL_H
#define RENDERER_RENDEREROPENGL_H

#include "kfx/renderer/IRenderer.h"
#include "kfx/renderer/IFrameGraphExecutor.h"

class RendererOpenGL : public IRenderer, public IFrameGraphExecutor {
public:
    RendererOpenGL();
    ~RendererOpenGL() override;

    bool Init() override;
    void Shutdown() override;
    const char* GetName() const override { return "OpenGL"; }

    void SetDisplayPalette(const unsigned char* rgb8) override;
    void PresentFrame() override;

    BackendCapabilities GetCapabilities() const override { return BackendCapabilities{ 1, 1 }; }
    bool BeginFrame() override;
    void EndFrame() override;
    bool PresentImage(const struct RendererPresentImageDesc* desc) override;

    // Zoom-box terrain tiles (gui_parchment.c). Returns true if the GPU path
    // accepted the submission (caller skips its own CPU tile loop); false
    // means no GL shaders available, caller falls back.
    bool SubmitZoomBoxTiles(const uint16_t* tile_block_ids, int tiles_x, int tiles_y,
                            int dst_x, int dst_y, int tile_w, int tile_h) override;

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
    bool MapFadeSupportsNativeResolution() const override;
    void BeginOverlayCapture(OverlayCaptureKind kind) override;
    void EndOverlayCapture(OverlayCaptureKind kind) override;

    void FGClearFrame() override;
    void FGBeginWorldCapture() override;
    void FGExecuteWorld() override;
    void FGFlushSwipeOverlay() override;
    void FGResolveWorldCapture() override;
    void FGExecuteMapFade() override;
    void FGDrawWorldSpriteLayer() override;
    void FGDrawWorldOverlayFlatLayer() override;
    void FGExecuteImagePresents() override;
    void FGDrawZoomBoxes() override;
    void FGDrawGameUI() override;
    void FGDrawFrontOverlay() override;
    void FGCaptureWorldFrameIfPending() override;
    void FGExecuteCursor() override;
    void FGDrawScreenTint() override;

    bool ScheduleScreenshot(const char* path, int fmt) override;
    void FGCaptureScreenshot() override;

private:
    struct Impl;
    Impl* m_impl = nullptr;

    // SDL_GLContext, kept opaque so this header doesn't need SDL/GL headers.
    void* m_gl_context = nullptr;

    void render_thread_init();
    void render_thread_work();
    void render_thread_cleanup();
};

#endif // RENDERER_RENDEREROPENGL_H
