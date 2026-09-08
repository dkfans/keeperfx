#ifndef RENDERER_IRENDERER_H
#define RENDERER_IRENDERER_H

#include <cstdint>

// Selectable renderer backends.
enum RendererType {
    RENDERER_INVALID  = -1,
    RENDERER_AUTO     = 0,  // pick the best available backend at startup
    RENDERER_SOFTWARE = 1,  // CPU software renderer, SDL display output
    RENDERER_OPENGL   = 2,  // OpenGL backend
};

enum OverlayCaptureKind {
    OVERLAY_CAPTURE_PARCHMENT = 0,
    OVERLAY_CAPTURE_SWIPE     = 1,
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

    struct BackendCapabilities { int hasGPURenderPath = 0; };
    virtual BackendCapabilities GetCapabilities() const { return BackendCapabilities{}; }

    virtual bool BeginFrame() = 0;
    virtual void EndFrame() = 0;

    // Present a raw indexed8 image (FMV frame, splash bitmap) at a
    // destination rect. Returns true if the backend drew it (GPU path
    // taken). // Todo : software calls this instead
    virtual bool PresentImage(const struct RendererPresentImageDesc* desc) { (void)desc; return false; }

    // Submit the zoom box's terrain as texture-block-indexed tiles
    virtual bool SubmitZoomBoxTiles(const uint16_t* tile_block_ids, int tiles_x, int tiles_y,
                                    int dst_x, int dst_y, int tile_w, int tile_h)
    {
        (void)tile_block_ids; (void)tiles_x; (void)tiles_y;
        (void)dst_x; (void)dst_y; (void)tile_w; (void)tile_h;
        return false;
    }

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

    virtual void SubmitMapFadeStep(int tick_step, float display_step, bool fading_in)
        { (void)tick_step; (void)display_step; (void)fading_in; }
    virtual bool MapFadeSupportsNativeResolution() const { return false; }

    virtual void BeginOverlayCapture(OverlayCaptureKind kind) { (void)kind; }
    virtual void EndOverlayCapture(OverlayCaptureKind kind) { (void)kind; }
};

#endif // RENDERER_IRENDERER_H
