#include "pre_inc.h"
#include "kfx/renderer/RendererSoftware.h"
#include "bflib_video.h"       // PALETTE_COLORS, lbWindow, lbHasSecondSurface, SDL
#include "bflib_vidsurface.h"  // lbDrawSurface / lbScreenSurface (go away when FB is removed)
#include "bflib_mouse.h"       // LbMouseOnBeginSwap/EndSwap (software cursor around present)
#include "post_inc.h"

bool RendererSoftware::Init()
{
    return true;
}

void RendererSoftware::Shutdown()
{
}

void RendererSoftware::SetDisplayPalette(const unsigned char* rgb8)
{
    if (lbDrawSurface == NULL)
        return;
    SDL_Color colors[PALETTE_COLORS];
    for (int i = 0; i < PALETTE_COLORS; i++)
    {
        colors[i].r = rgb8[3 * i + 0];
        colors[i].g = rgb8[3 * i + 1];
        colors[i].b = rgb8[3 * i + 2];
        colors[i].a = SDL_ALPHA_OPAQUE;
    }
    SDL_Palette* surfpal = SDL_GetSurfacePalette(lbDrawSurface);
    if (surfpal != NULL)
        SDL_SetPaletteColors(surfpal, colors, 0, PALETTE_COLORS);
}

void RendererSoftware::ClearScreen(unsigned char colour)
{
    if (lbDrawSurface == NULL)
        return;
    if (!SDL_FillSurfaceRect(lbDrawSurface, NULL, colour))
        ERRORLOG("Error while clearing screen: %s", SDL_GetError());
}

void RendererSoftware::PresentFrame()
{
    TbResult ret = LbMouseOnBeginSwap();
    // Put the Draw Surface onto the window Surface (refetch each frame to survive alt-tab).
    if ((ret == Lb_SUCCESS) && lbHasSecondSurface)
    {
        lbScreenSurface = SDL_GetWindowSurface(lbWindow);
        if (!SDL_BlitSurface(lbDrawSurface, NULL, lbScreenSurface, NULL))
        {
            ERRORLOG("Blit failed: %s", SDL_GetError());
            ret = Lb_FAIL;
        }
    }
    // Flip the window surface to screen.
    if (ret == Lb_SUCCESS)
    {
        if (!SDL_UpdateWindowSurface(lbWindow))
        {
            ERRORDBG(11, "Flip failed: %s", SDL_GetError());
        }
    }
    LbMouseOnEndSwap();
}
