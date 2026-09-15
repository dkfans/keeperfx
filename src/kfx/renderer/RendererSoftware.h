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
    unsigned char* LockFramebuffer(int* out_pitch) override;
    void UnlockFramebuffer() override;
    BackendCapabilities GetCapabilities() const override { return BackendCapabilities{ 0 }; }
    bool BeginFrame() override;
    void EndFrame() override;
    bool ScheduleScreenshot(const char* path, int fmt) override;


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
