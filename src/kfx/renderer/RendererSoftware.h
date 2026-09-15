#ifndef RENDERER_RENDERERSOFTWARE_H
#define RENDERER_RENDERERSOFTWARE_H

#include "kfx/renderer/IRenderer.h"
#include "kfx/renderer/IFrameGraphExecutor.h"
#include "kfx/renderer/backends/SoftwareUIRenderer.h"
#include "kfx/renderer/backends/SoftwareTextRenderer.h"
#include "kfx/renderer/backends/SoftwareCursorLayer.h"
#include "kfx/renderer/backends/SoftwareWorldViewRenderer.h"

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;

// Software backend. it's small for now, it's gonna grow the more I bring things into it.
class RendererSoftware : public IRenderer, public IFrameGraphExecutor {
public:
    bool Init() override;
    void Shutdown() override;
    const char* GetName() const override { return "software"; }
    void SetDisplayPalette(const unsigned char* pal6) override;
    void ClearScreen(unsigned char colour) override;
    void PresentFrame() override;
    BackendCapabilities GetCapabilities() const override { return BackendCapabilities{ 0 }; }
    bool BeginFrame() override;
    void EndFrame() override;
    bool CanDraw() const override;
    bool PresentImage(const struct RendererPresentImageDesc* desc) override;
    void PresentHugeSprite(const struct TbHugeSprite* spr, int32_t sp_len,
                           int32_t x_shift, int32_t y_shift, int32_t units_per_px) override;
    void SubmitZoomBoxTiles(const uint16_t* tile_block_ids, int tiles_x, int tiles_y,
                            int dst_x, int dst_y, int tile_w, int tile_h) override;
    bool ScheduleScreenshot(const char* path, int fmt) override;
    bool SubmitLandviewZoom(const unsigned char* src_buf, int src_w, int src_h,
                            float center_map_x, float center_map_y,
                            float screen_cx,    float screen_cy,
                            float scale) override;


    IUIRenderer*         GetUIRenderer()        override { return &m_ui_renderer; }
    ITextRenderer*       GetTextRenderer()      override { return &m_text_renderer; }
    ICursorLayer*        GetCursorLayer()       override { return &m_cursorLayer; }
    IWorldViewRenderer*  GetWorldViewRenderer() override { return &m_world_renderer; }

    void FGExecuteCursor() override { m_cursorLayer.Draw(); }
    // Software rasterizes the world synchronously inside
    // WorldViewRenderer_DrawIsometricView()/_DrawFrontView() in keeper_screen_redraw
    void FGResolveWorldCapture() override { m_world_renderer.ResolveDeferredWorld(); }

private:
    bool ensure_present_target();
    void destroy_present_target();

    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture*  m_texture  = nullptr;
    int           m_tex_w    = 0;
    int           m_tex_h    = 0;
    int           m_vsync    = -1; // SDL_SetRenderVSync value; -1 = unset

    SoftwareUIRenderer        m_ui_renderer;
    SoftwareTextRenderer      m_text_renderer;
    SoftwareCursorLayer       m_cursorLayer;
    SoftwareWorldViewRenderer m_world_renderer;
};

#endif // RENDERER_RENDERERSOFTWARE_H
