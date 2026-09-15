#include "pre_inc.h"
#include "kfx/renderer/RendererSoftware.h"
#include "bflib_video.h"       // PALETTE_COLORS, SDL, vsync_enabled
#include "kfx/platform/WindowSystemSDL.h"
#include "kfx/renderer/software/SwDisplaySurface.h"
#include "kfx/renderer/software/SwDrawTarget.h"
#include "bflib_vidraw.h"      // LbHugeSpriteDraw
#include "kfx/renderer/software/SwZoomBoxTiles.h"
#include "kfx/renderer/RendererManager.h" // RendererBeginFrame/EndFrame, around FGExecuteCursor
#include "kfx/renderer/RenderGraph.h"
#include "kfx/renderer/RenderTaskProducerRegistry.h"
#include "kfx/renderer/RendererFrameCounter.h"
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
        // Name the backend SDL picked for us, so a bug report tells which graphics path the
        // game was presenting through, and which driver libraries that pulls into the process.
        const char * backend = SDL_GetRendererName(m_renderer);
        SYNCLOG("Presenting through SDL renderer: %s", (backend != nullptr) ? backend : "unknown");
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

bool RendererSoftware::BeginFrame()
{
    SDL_Surface* draw_surface = SwDisplaySurfaceGet();
    if (draw_surface == NULL || !SDL_LockSurface(draw_surface))
    {
        lbDisplay.GraphicsWindowPtr = NULL;
        lbDisplay.WScreen = NULL;
        return false;
    }
    lbDisplay.WScreen = static_cast<unsigned char*>(draw_surface->pixels);
    lbDisplay.GraphicsScreenWidth = draw_surface->pitch;
    lbDisplay.GraphicsWindowPtr = &lbDisplay.WScreen[lbDisplay.GraphicsWindowX +
        RendererScreenWidth() * lbDisplay.GraphicsWindowY];
    return true;
}

void RendererSoftware::EndFrame()
{
    lbDisplay.WScreen = NULL;
    lbDisplay.GraphicsWindowPtr = NULL;
    SDL_Surface* draw_surface = SwDisplaySurfaceGet();
    if (draw_surface != NULL)
        SDL_UnlockSurface(draw_surface);
}

bool RendererSoftware::CanDraw() const
{
    return lbDisplay.WScreen != NULL;
}

// Nearest-neighbour scale of desc's source into its destination rect.
// OPAQUE also clears everything outside the rect; TRANSPARENT skips
// uncovered pixels (coverage 0, or index 0 when there is no coverage map).
bool RendererSoftware::PresentImage(const struct RendererPresentImageDesc* desc)
{
    TbPixel* screen = SwTargetWScreen();
    if (desc == nullptr || desc->src == nullptr || screen == nullptr ||
        desc->format != PRESENT_FORMAT_INDEXED8 || desc->src_w <= 0 || desc->src_h <= 0)
        return false;

    if (desc->palette == PRESENT_PALETTE_EMBEDDED)
    {
        if (desc->embedded_palette == nullptr)
            return false;
        unsigned char rgb8[PALETTE_SIZE];
        for (int i = 0; i < PALETTE_COLORS; ++i)
        {
            rgb8[i * 3 + 0] = desc->embedded_palette[i * 4 + 2];
            rgb8[i * 3 + 1] = desc->embedded_palette[i * 4 + 1];
            rgb8[i * 3 + 2] = desc->embedded_palette[i * 4 + 0];
        }
        RendererSetDisplayPalette(rgb8);
    }

    const int32_t scanline = SwTargetScanline();
    const int32_t nlines = SwTargetScreenHeight();
    const int32_t src_pitch = (desc->src_pitch > 0) ? desc->src_pitch : desc->src_w;
    const bool clear_margins = (desc->kind == PRESENT_KIND_OPAQUE);
    const bool transparent = (desc->kind == PRESENT_KIND_TRANSPARENT);

    if (clear_margins)
    {
        for (int32_t sh = 0; sh < desc->dst_y; sh++)
            memset(screen + sh * scanline, 0, scanline);
        for (int32_t sh = desc->dst_y + desc->dst_h; sh < nlines; sh++)
            memset(screen + sh * scanline, 0, scanline);
    }
    int32_t dhstart = desc->dst_y;
    for (int32_t sh = 0; sh < desc->src_h; sh++)
    {
        const int32_t dhend = desc->dst_y + (desc->dst_h * (sh + 1) / desc->src_h);
        const unsigned char* src = desc->src + sh * src_pitch;
        const unsigned char* cov = (desc->coverage != nullptr) ? desc->coverage + sh * desc->src_w : nullptr;
        const int32_t mhmin = (dhstart < 0) ? -dhstart : 0;
        const int32_t mhmax = (dhend < nlines) ? dhend - dhstart : nlines - dhstart;
        for (int32_t k = mhmin; k < mhmax; k++)
        {
            TbPixel* dst = screen + (dhstart + k) * scanline;
            int32_t dwstart = desc->dst_x;
            if (clear_margins && dwstart > 0)
                memset(dst, 0, dwstart);
            for (int32_t sw = 0; sw < desc->src_w; sw++)
            {
                const int32_t dwend = desc->dst_x + (desc->dst_w * (sw + 1) / desc->src_w);
                const bool skip = transparent && ((cov != nullptr) ? (cov[sw] == 0) : (src[sw] == 0));
                if (!skip)
                {
                    const int32_t mwmin = (dwstart < 0) ? -dwstart : 0;
                    const int32_t mwmax = (dwend < scanline) ? dwend - dwstart : scanline - dwstart;
                    for (int32_t i = mwmin; i < mwmax; i++)
                        dst[dwstart + i] = src[sw];
                }
                dwstart = dwend;
            }
            if (clear_margins && dwstart < scanline)
                memset(dst + dwstart, 0, scanline - dwstart);
        }
        dhstart = dhend;
    }
    return true;
}

void RendererSoftware::PresentHugeSprite(const struct TbHugeSprite* spr, int32_t sp_len,
                                         int32_t x_shift, int32_t y_shift, int32_t units_per_px)
{
    TbPixel* screen = SwTargetWScreen();
    if (screen == nullptr)
        return;
    LbHugeSpriteDraw(spr, sp_len, screen, RendererScreenWidth(), lbDisplay.PhysicalScreenHeight,
                     x_shift, y_shift, units_per_px);
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

// Landview zoom transition: expands the map outward from the zoom centre,
// one quadrant at a time, in 24.8 fixed point.
void RendererSoftware::SubmitZoomBoxTiles(const uint16_t* tile_block_ids, int tiles_x, int tiles_y,
                                          int dst_x, int dst_y, int tile_w, int tile_h)
{
    if (SwTargetWScreen() == nullptr)
        return;
    SwDrawZoomBoxTiles(tile_block_ids, tiles_x, tiles_y, dst_x, dst_y, tile_w, tile_h);
}

bool RendererSoftware::SubmitLandviewZoom(const unsigned char* src_buf, int src_w, int src_h,
                                          float center_map_x, float center_map_y,
                                          float screen_cx,    float screen_cy,
                                          float scale)
{
    (void)src_h;
    TbPixel* screen = SwTargetWScreen();
    if (screen == nullptr)
        return false;

    const int32_t map_x = (int32_t)center_map_x;
    const int32_t map_y = (int32_t)center_map_y;
    const int32_t scr_x = (int32_t)screen_cx;
    const int32_t scr_y = (int32_t)screen_cy;
    const int32_t src_delta = (int32_t)(scale * 256.0f + 0.5f);
    const int32_t scanline = SwTargetScanline();
    const int32_t phys_w = RendererPhysicalWidth();
    const int32_t phys_h = lbDisplay.PhysicalScreenHeight;

    const unsigned char* centre_src = &src_buf[src_w * map_y + map_x];
    TbPixel* centre_dst = &screen[scanline * scr_y + scr_x];

    // Up-left
    TbPixel* dst = centre_dst;
    int32_t bpos_y = 0;
    for (int32_t y = 0; y <= scr_y; y++)
    {
        const unsigned char* src = &centre_src[-src_w * (bpos_y >> 8)];
        int32_t bpos_x = 0;
        for (int32_t x = 0; x <= scr_x; x++)
        {
            bpos_x += src_delta;
            dst[-x] = src[-(bpos_x >> 8)];
        }
        dst -= scanline;
        bpos_y += src_delta;
    }
    // Up-right: one pixel less in both source and destination
    dst = centre_dst + 1;
    bpos_y = 0;
    for (int32_t y = 0; y <= scr_y; y++)
    {
        const unsigned char* src = &centre_src[-src_w * (bpos_y >> 8)];
        int32_t bpos_x = (1 << 8);
        for (int32_t x = 0; x < phys_w - 1 - scr_x; x++)
        {
            bpos_x += src_delta;
            dst[x] = src[(bpos_x >> 8)];
        }
        dst -= scanline;
        bpos_y += src_delta;
    }
    // Down-left
    dst = centre_dst + scanline;
    bpos_y = (1 << 8);
    for (int32_t y = 0; y < phys_h - 1 - scr_y; y++)
    {
        const unsigned char* src = &centre_src[src_w * (bpos_y >> 8)];
        int32_t bpos_x = 0;
        for (int32_t x = 0; x <= scr_x; x++)
        {
            bpos_x += src_delta;
            dst[-x] = src[-(bpos_x >> 8)];
        }
        dst += scanline;
        bpos_y += src_delta;
    }
    // Down-right
    dst = centre_dst + scanline + 1;
    bpos_y = (1 << 8);
    for (int32_t y = 0; y < phys_h - 1 - scr_y; y++)
    {
        const unsigned char* src = &centre_src[src_w * (bpos_y >> 8)];
        int32_t bpos_x = (1 << 8);
        for (int32_t x = 0; x < phys_w - 1 - scr_x; x++)
        {
            dst[x] = src[(bpos_x >> 8)];
            bpos_x += src_delta;
        }
        dst += scanline;
        bpos_y += src_delta;
    }
    return true;
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
    // Same call as RendererOpenGL::PresentFrame(), placed ahead of
    // RenderGraph::Execute() below so producers append using this frame's
    // number before it advances.
    RenderTaskProducerRegistry_ProduceAll(RendererFrameCounter_Current());
    LbMouseOnBeginSwap(); // submits the OS pointer sprite; FGExecuteCursor() below draws it
    if (RendererBeginFrame())
    {
        RenderGraph::Execute(*this);
        RendererEndFrame();
    }
    // Software has no resource mapper of its own, but the shared frame
    // counter must still advance every PresentFrame() call so
    // RenderTaskProducer's frame_number keeps moving forward for both
    // backends (spec 2.4) -- mirrors RendererOpenGL::PresentFrame()'s own
    // RendererFrameCounter_Advance() call.
    RendererFrameCounter_Advance();
    // INDEX8 (palette) -> RGBA and present
    if (!SDL_BlitSurface(draw_surface, NULL, texture_surface, NULL))
    {
        ERRORLOG("Present blit failed: %s", SDL_GetError());
        SDL_UnlockTexture(m_texture);
        LbMouseOnEndSwap();
        return;
    }
    SDL_UnlockTexture(m_texture);
    int px = 0, py = 0, pw = 0, ph = 0;
    GetSDLWindowSystem()->GetPresentRect(&px, &py, &pw, &ph);
    const SDL_FRect dst = { (float)px, (float)py, (float)pw, (float)ph };
    SDL_RenderClear(m_renderer);
    SDL_RenderTexture(m_renderer, m_texture, NULL, (pw > 0 && ph > 0) ? &dst : NULL);
    SDL_RenderPresent(m_renderer);
    LbMouseOnEndSwap();
}
