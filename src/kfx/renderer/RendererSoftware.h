#ifndef RENDERER_RENDERERSOFTWARE_H
#define RENDERER_RENDERERSOFTWARE_H

#include "kfx/renderer/IRenderer.h"
#include "kfx/renderer/backends/SoftwareUIRenderer.h"
#include "kfx/renderer/backends/SoftwareTextRenderer.h"

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;

// Software backend. it's small for now, it's gonna grow the more I bring things into it.
class RendererSoftware : public IRenderer {
public:
    bool Init() override;
    void Shutdown() override;
    const char* GetName() const override { return "software"; }
    void SetDisplayPalette(const unsigned char* pal6) override;
    void ClearScreen(unsigned char colour) override;
    void PresentFrame() override;
    unsigned char* LockFramebuffer(int* out_pitch) override;
    void UnlockFramebuffer() override;
    bool ScheduleScreenshot(const char* path, int fmt) override;


    IUIRenderer*   GetUIRenderer()   override { return &m_ui_renderer; }
    ITextRenderer* GetTextRenderer() override { return &m_text_renderer; }

private:
    bool ensure_present_target();
    void destroy_present_target();

    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture*  m_texture  = nullptr;
    int           m_tex_w    = 0;
    int           m_tex_h    = 0;
    int           m_vsync    = -1; // SDL_SetRenderVSync value; -1 = unset

    SoftwareUIRenderer   m_ui_renderer;
    SoftwareTextRenderer m_text_renderer;
};

#endif // RENDERER_RENDERERSOFTWARE_H
