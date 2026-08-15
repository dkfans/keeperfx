#include "pre_inc.h"
#include "kfx/renderer/RendererManager.h"
#include "kfx/renderer/RendererSoftware.h"
#include "bflib_basics.h"
#include "bflib_video.h"
#include "post_inc.h"

static IRenderer*   s_active_renderer = nullptr;
static RendererType s_active_type     = RENDERER_INVALID;
static unsigned char s_draw_colour = 0;
static unsigned short s_draw_flags = 0;

// Allocate a backend for the requested type, or nullptr if unknown.
static IRenderer* create_renderer(RendererType type)
{
    switch (type)
    {
        case RENDERER_SOFTWARE: return new RendererSoftware();
        default:                return nullptr;
    }
}

int RendererInit(RendererType type)
{
    if (s_active_renderer != nullptr)
        RendererShutdown();

    RendererType resolved = (type == RENDERER_AUTO) ? RENDERER_SOFTWARE : type;
    IRenderer* rend = create_renderer(resolved);
    if (rend == nullptr)
    {
        ERRORLOG("Unknown renderer type %d", (int)type);
        return 0;
    }
    if (!rend->Init())
    {
        ERRORLOG("Renderer '%s' failed to initialise", rend->GetName());
        delete rend;
        return 0;
    }
    s_active_renderer = rend;
    s_active_type     = resolved;
    SYNCDBG(0, "Renderer backend '%s' active", rend->GetName());
    return 1;
}

void RendererShutdown(void)
{
    if (s_active_renderer == nullptr)
        return;
    s_active_renderer->Shutdown();
    delete s_active_renderer;
    s_active_renderer = nullptr;
    s_active_type     = RENDERER_INVALID;
}

RendererType RendererGetActiveType(void)
{
    return s_active_type;
}

const unsigned char* RendererGetActivePalette(void)
{
    return LbPaletteGetReadonly();
}

// Palette channels are stored 6-bit (0..63) - because of VGA constraint, convert to 8-bit (0..255) for display.
static inline unsigned char chan6_to_8(unsigned char v)
{
    return (unsigned char)((v * 255) / 63);
}

TbResult RendererPaletteSet(unsigned char *palette)
{
    if (!lbScreenInitialised)
        return Lb_FAIL;
    TbResult ret = LbPaletteStore(palette);
    if (ret == Lb_SUCCESS)
    {
        const unsigned char* pal6 = LbPaletteGetReadonly();
        unsigned char rgb8[PALETTE_SIZE];
        for (int i = 0; i < PALETTE_SIZE; i++)
            rgb8[i] = chan6_to_8(pal6[i]);
        RendererSetDisplayPalette(rgb8);
    }
    return ret;
}

void RendererSetDisplayPalette(const unsigned char *rgb8)
{
    if (s_active_renderer != nullptr)
        s_active_renderer->SetDisplayPalette(rgb8);
}

void RendererClearScreen(unsigned char colour)
{
    if (s_active_renderer != nullptr)
        s_active_renderer->ClearScreen(colour);
}

void RendererPresentFrame(void)
{
    if (s_active_renderer != nullptr)
        s_active_renderer->PresentFrame();
}

TbResult RendererLockFramebuffer(void)
{
    if (!lbScreenInitialised || s_active_renderer == nullptr)
        return Lb_FAIL;
    int pitch = 0;
    unsigned char* px = s_active_renderer->LockFramebuffer(&pitch);
    if (px == nullptr)
    {
        lbDisplay.GraphicsWindowPtr = NULL;
        lbDisplay.WScreen = NULL;
        return Lb_FAIL;
    }
    lbDisplay.WScreen = px;
    lbDisplay.GraphicsScreenWidth = pitch;
    lbDisplay.GraphicsWindowPtr = &lbDisplay.WScreen[lbDisplay.GraphicsWindowX +
        lbDisplay.GraphicsScreenWidth * lbDisplay.GraphicsWindowY];
    return Lb_SUCCESS;
}

TbResult RendererUnlockFramebuffer(void)
{
    lbDisplay.WScreen = NULL;
    lbDisplay.GraphicsWindowPtr = NULL;
    if (s_active_renderer != nullptr)
        s_active_renderer->UnlockFramebuffer();
    return Lb_SUCCESS;
}

TbBool RendererScheduleScreenshot(const char* path, int fmt)
{
    return (s_active_renderer != nullptr) ? s_active_renderer->ScheduleScreenshot(path, fmt) : 0;
}

TbResult RendererSetupScreen(TbScreenMode mode, TbScreenCoord width, TbScreenCoord height,
    unsigned char *palette, short buffers_count, TbBool wscreen_vid)
{
    return LbScreenSetup(mode, width, height, palette, buffers_count, wscreen_vid);
}

TbResult RendererResetScreen(TbBool exiting_application)
{
    return LbScreenReset(exiting_application);
}

TbResult RendererScreenInitialize(void)
{
    return LbScreenInitialize();
}

TbResult RendererSetDoubleBuffering(TbBool state)
{
    return LbScreenSetDoubleBuffering(state);
}

unsigned char RendererGetDrawColour(void) { return s_draw_colour; }
void RendererSetDrawColour(unsigned char colour) { s_draw_colour = colour; }

unsigned short RendererGetDrawFlags(void) { return s_draw_flags; }
void RendererSetDrawFlags(unsigned short flags) { s_draw_flags = flags; }
void RendererAddDrawFlags(unsigned short flags) { s_draw_flags |= flags; }
void RendererClearDrawFlags(unsigned short flags) { s_draw_flags &= ~flags; }
void RendererToggleDrawFlags(unsigned short flags) { s_draw_flags ^= flags; }

TbResult RendererPaletteGet(unsigned char *palette)
{
    return LbPaletteGet(palette);
}
