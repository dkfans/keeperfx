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

    struct BackendCapabilities {
        // True when the backend composites the minimap over the panel artwork
        // itself (draw-order layering): its minimap buffer starts zeroed and
        // index 0 is transparent, so real black must use another index.
        int compositesMinimapBackground = 0;
    };
    virtual BackendCapabilities GetCapabilities() const { return BackendCapabilities{}; }

    virtual bool BeginFrame() = 0;
    virtual void EndFrame() = 0;

    // True when draws made now reach the current frame.
    virtual bool CanDraw() const = 0;

    // Present a raw indexed8 image (FMV frame, splash bitmap) at a
    // destination rect. Returns false if the image couldn't be drawn.
    virtual bool PresentImage(const struct RendererPresentImageDesc* desc) = 0;

    // Draw a full-screen RLE huge sprite (the landview window frame) over
    // whatever is already on screen.
    virtual void PresentHugeSprite(const struct TbHugeSprite* spr, int32_t sp_len,
                                   int32_t x_shift, int32_t y_shift, int32_t units_per_px) = 0;

    // Draw the zoom box's terrain from texture-block-indexed tiles.
    // 0xFFFF tiles (unrevealed) are skipped.
    virtual void SubmitZoomBoxTiles(const uint16_t* tile_block_ids, int tiles_x, int tiles_y,
                                    int dst_x, int dst_y, int tile_w, int tile_h) = 0;

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

    // Sub-renderers. Every backend provides all four once initialised.
    virtual class ITextRenderer*      GetTextRenderer()      = 0;
    virtual class IUIRenderer*        GetUIRenderer()        = 0;
    virtual class ICursorLayer*       GetCursorLayer()       = 0;
    virtual class IWorldViewRenderer* GetWorldViewRenderer() = 0;

    virtual void SubmitMapFadeStep(int tick_step, float display_step, bool fading_in,
                                   const unsigned char* ghost_table)
        { (void)tick_step; (void)display_step; (void)fading_in; (void)ghost_table; }
    virtual bool MapFadeSupportsNativeResolution() const { return false; }

    virtual void BeginOverlayCapture(OverlayCaptureKind kind) { (void)kind; }
    virtual void EndOverlayCapture(OverlayCaptureKind kind) { (void)kind; }
};

#endif // RENDERER_IRENDERER_H
