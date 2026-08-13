#include "pre_inc.h"
#include "kfx/renderer/RendererManager.h"
#include "kfx/renderer/RendererSoftware.h"
#include "bflib_basics.h"
#include "bflib_video.h"
#include "post_inc.h"

static IRenderer*   s_active_renderer = nullptr;
static RendererType s_active_type     = RENDERER_INVALID;

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
