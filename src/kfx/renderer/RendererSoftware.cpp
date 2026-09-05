#include "pre_inc.h"
#include "kfx/renderer/RendererSoftware.h"
#include "bflib_video.h"       // PALETTE_COLORS, SDL, vsync_enabled
#include "kfx/platform/WindowSystemSDL.h"
#include "kfx/renderer/software/SwDisplaySurface.h"
#include "kfx/renderer/RendererManager.h" // RendererBeginFrame/EndFrame, around FGExecuteCursor
#include "kfx/renderer/RenderGraph.h"
#include "bflib_mouse.h"       // LbMouseOnBeginSwap/EndSwap (submits the cursor sprite around present)
#include <SDL3_image/SDL_image.h> // IMG_SavePNG (screenshots)
#include "post_inc.h"

bool RendererSoftware::Init()
{
    return true;
}

void RendererSoftware::Shutdown()
{
    destroy_present_target();
}

void RendererSoftware::SetDisplayPalette(const unsigned char* rgb8)
{
    SDL_Surface* draw_surface = SwDisplaySurfaceGet();
    if (draw_surface == NULL)
        return;
    SDL_Color colors[PALETTE_COLORS];
    for (int i = 0; i < PALETTE_COLORS; i++)
    {
        colors[i].r = rgb8[3 * i + 0];
        colors[i].g = rgb8[3 * i + 1];
        colors[i].b = rgb8[3 * i + 2];
        colors[i].a = SDL_ALPHA_OPAQUE;
    }
    SDL_Palette* surfpal = SDL_GetSurfacePalette(draw_surface);
    if (surfpal != NULL)
        SDL_SetPaletteColors(surfpal, colors, 0, PALETTE_COLORS);
}

void RendererSoftware::ClearScreen(unsigned char colour)
{
    SDL_Surface* draw_surface = SwDisplaySurfaceGet();
    if (draw_surface == NULL)
        return;
    if (!SDL_FillSurfaceRect(draw_surface, NULL, colour))
        ERRORLOG("Error while clearing screen: %s", SDL_GetError());
}

bool RendererSoftware::ensure_present_target()
{
    SDL_Surface* draw_surface = SwDisplaySurfaceGet();
    SDL_Window* window = GetSDLWindowSystem()->GetSDLWindow();
    if (m_renderer != nullptr && SDL_GetRenderWindow(m_renderer) != window)
        destroy_present_target();
    if (m_renderer == nullptr)
    {
        m_renderer = SDL_CreateRenderer(window, NULL);
        if (m_renderer == nullptr)
        {
            ERRORLOG("SDL_CreateRenderer failed: %s", SDL_GetError());
            return false;
        }
    }

    const int want_vsync = vsync_enabled ? 1 : 0;
    if (m_vsync != want_vsync)
    {
        SDL_SetRenderVSync(m_renderer, want_vsync);
        m_vsync = want_vsync;
    }

    if (m_texture == nullptr || m_tex_w != draw_surface->w || m_tex_h != draw_surface->h)
    {
        if (m_texture != nullptr) { SDL_DestroyTexture(m_texture); m_texture = nullptr; }
        m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA32,
                                      SDL_TEXTUREACCESS_STREAMING, draw_surface->w, draw_surface->h);
        if (m_texture == nullptr)
        {
            ERRORLOG("SDL_CreateTexture failed: %s", SDL_GetError());
            return false;
        }
        SDL_SetTextureScaleMode(m_texture, SDL_SCALEMODE_NEAREST); // crisp pixels
        m_tex_w = draw_surface->w;
        m_tex_h = draw_surface->h;
    }
    return true;
}

void RendererSoftware::destroy_present_target()
{
    if (m_texture != nullptr) { SDL_DestroyTexture(m_texture); m_texture = nullptr; }
    if (m_renderer != nullptr) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    m_tex_w = 0;
    m_tex_h = 0;
    m_vsync = -1;
}

unsigned char* RendererSoftware::LockFramebuffer(int* out_pitch)
{
    SDL_Surface* draw_surface = SwDisplaySurfaceGet();
    if (draw_surface == NULL || !SDL_LockSurface(draw_surface))
        return nullptr;
    if (out_pitch != nullptr)
        *out_pitch = draw_surface->pitch;
    return static_cast<unsigned char*>(draw_surface->pixels);
}

void RendererSoftware::UnlockFramebuffer()
{
    SDL_Surface* draw_surface = SwDisplaySurfaceGet();
    if (draw_surface != NULL)
        SDL_UnlockSurface(draw_surface);
}

bool RendererSoftware::BeginFrame()
{
    return true;
}

void RendererSoftware::EndFrame()
{
}

bool RendererSoftware::ScheduleScreenshot(const char* path, int fmt)
{
    SDL_Surface* draw_surface = SwDisplaySurfaceGet();
    if (draw_surface == NULL)
        return false;
    bool ok;
    switch (fmt)
    {
        case 1:  ok = IMG_SavePNG(draw_surface, path); break;
        case 2:  ok = SDL_SaveBMP(draw_surface, path); break;
        default: return false;
    }
    if (!ok)
        ERRORLOG("Screenshot save failed (%s): %s", path, SDL_GetError());
    return ok;
}

void RendererSoftware::PresentFrame()
{
    SDL_Surface* draw_surface = SwDisplaySurfaceGet();
    if (draw_surface == NULL || !ensure_present_target())
        return;
    SDL_Surface* texture_surface;
    if (!SDL_LockTextureToSurface(m_texture, NULL, &texture_surface))
    {
        ERRORLOG("Present texture lock failed: %s", SDL_GetError());
        return;
    }
    LbMouseOnBeginSwap(); // submits the OS pointer sprite; FGExecuteCursor() below draws it
    if (RendererBeginFrame())
    {
        RenderGraph::Execute(*this);
        RendererEndFrame();
    }
    // INDEX8 (palette) -> RGBA and present
    if (!SDL_BlitSurface(draw_surface, NULL, texture_surface, NULL))
    {
        ERRORLOG("Present blit failed: %s", SDL_GetError());
        SDL_UnlockTexture(m_texture);
        LbMouseOnEndSwap();
        return;
    }
    SDL_UnlockTexture(m_texture);
    SDL_RenderClear(m_renderer);
    SDL_RenderTexture(m_renderer, m_texture, NULL, NULL);
    SDL_RenderPresent(m_renderer);
    LbMouseOnEndSwap();
}
