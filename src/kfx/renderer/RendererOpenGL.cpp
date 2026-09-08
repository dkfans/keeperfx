#include "pre_inc.h"
#include "kfx/renderer/RendererOpenGL.h"
#include "bflib_basics.h"
#include "globals.h"        // ERRORLOG/SYNCDBG
#include "bflib_video.h"    // lbDisplay
#include "kfx/platform/WindowSystemSDL.h"
#include "kfx/platform/PlatformGLHdrWin.h"
#include "kfx/renderer/opengl/GLFunctions.h"
#include "kfx/renderer/opengl/GLSpriteAtlas.h"
#include "kfx/renderer/opengl/GLUIRenderer.h"
#include "kfx/renderer/opengl/GLTextRenderer.h"
#include "kfx/renderer/opengl/GLCursorLayer.h"
#include "kfx/renderer/opengl/GLTileAtlas.h"
#include "kfx/renderer/opengl/GLWorldViewRenderer.h"
#include "kfx/renderer/opengl/GLMapFadePass.h"
#include "kfx/renderer/opengl/GLImagePresentPass.h"
#include "kfx/renderer/opengl/GLZoomBoxTilesPass.h"
#include "kfx/renderer/opengl/GLResourceMapper.h"
#include "kfx/renderer/GpuResourceHandle.h"
#include "kfx/renderer/GpuResourceDesc.h"
#include "kfx/renderer/RendererFrameCounter.h"
#include "kfx/renderer/RenderTaskProducerRegistry.h"
#include "kfx/renderer/ir/UICommands.h"
#include "kfx/renderer/ir/TextCommands.h"
#include "kfx/renderer/ir/WorldCommands.h"
#include "kfx/renderer/RenderThreadManager.h"
#include "kfx/renderer/RendererThread.h"
#include "kfx/renderer/RenderGraph.h"
#include "kfx/renderer/RendererManager.h" // g_screen_tint
#include "engine_textures.h" // block_ptrs[] -- tile atlas build-ready check
#include "vidmode.h" // pixmap.fade_tables
#include "bflib_mouse.h" // LbMouseOnBeginSwap/EndSwap -- submits the cursor sprite around present
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h> // IMG_SavePNG (screenshots), Todo : move to platform layer
#include <cstring>
#include <string>
#include <vector>
#include "post_inc.h"

struct GLFrameData {
    UICommandBuffers    ui_cmds;
    TextCommandBuffers  text_cmds;
    WorldCommandBuffers world_cmds;
    uint32_t            frame_seq = 0;

    UICommandBuffers    parchment_ui_cmds;
    TextCommandBuffers  parchment_text_cmds;

    UICommandBuffers    swipe_cmds;

    int32_t screen_w = 0;
    int32_t screen_h = 0;

    float screen_tint[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // The frame number this GLFrameData was sealed under
    uint64_t sealed_frame_number = 0;

    bool          palette_dirty = false;
    unsigned char palette_rgba[256 * 4] = {};

    GLCursorSnapshot cursor_snapshot;
};

struct RendererOpenGL::Impl {
    GLSpriteAtlas      atlas;
    GLUIRenderer       ui;
    GLTextRenderer     text;
    GLCursorLayer      cursor;
    GLTileAtlas        world_atlas;
    GLWorldViewRenderer world;
    GLMapFadePass      mapfade;
    GLImagePresentPass imgpresent;
    GLZoomBoxTilesPass zoomboxtiles;

    GLResourceMapper   resource_mapper;
    GpuResourceHandle  palette_tex_handle    = kInvalidGpuResource;
    GpuResourceHandle  fade_table_tex_handle = kInvalidGpuResource;

    bool fade_tables_refreshed = false;

    RenderThreadManager thread_mgr;
    GLFrameData frames[2];
    int write_idx  = 0; // which frames[] slot Submit* is currently appending into
    int render_idx = 0; // which frames[] slot the render thread is consuming (only valid between Signal() and the matching WaitForCompletion())

    bool init_ok         = false; // set by render_thread_init(), read after Start() returns
    bool functions_loaded = false; // guards render_thread_cleanup() from calling gl* before GLFunctions_Load() ran

    bool          fg_lens_captured      = false;
    bool          fg_lens_palette_active = false;
    unsigned char fg_lens_palette_rgba[256 * 4] = {};

    std::string screenshot_path;
    int         screenshot_fmt = 0;
};

// GL_KHR_debug callback -- surfaces driver-reported errors/warnings (bad
// texture uploads, shader link failures the compile-time check missed,
// invalid enum/operation, etc.) as ERRORLOG/WARNLOG/SYNCLOG lines instead of
// silently wrong pixels with no trail. Adapted from develop's equivalent
// (RendererOpenGL.cpp's DebugCallback), which this branch never had. Wired
// up unconditionally in render_thread_init() below (not gated behind a debug
// build) since this renderer is young enough that the diagnostic value
// outweighs the (small, driver-side) overhead -- revisit once it's mature.
static void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei /*length*/, const GLchar* message, const void* /*userParam*/)
{
    // Ignore noisy, harmless notifications (buffer usage hints, etc.) --
    // same ids develop's own callback filters.
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    const char* src = source == GL_DEBUG_SOURCE_API             ? "API"
                     : source == GL_DEBUG_SOURCE_WINDOW_SYSTEM   ? "WindowSys"
                     : source == GL_DEBUG_SOURCE_SHADER_COMPILER ? "ShaderCompiler"
                     : source == GL_DEBUG_SOURCE_THIRD_PARTY     ? "3rdParty"
                     : source == GL_DEBUG_SOURCE_APPLICATION     ? "Application"
                     : source == GL_DEBUG_SOURCE_OTHER           ? "Other"
                                                                  : "Unknown";
    const char* tp = type == GL_DEBUG_TYPE_ERROR               ? "Error"
                    : type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR ? "Deprecated"
                    : type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  ? "Undefined"
                    : type == GL_DEBUG_TYPE_PORTABILITY         ? "Portability"
                    : type == GL_DEBUG_TYPE_PERFORMANCE         ? "Performance"
                    : type == GL_DEBUG_TYPE_MARKER              ? "Marker"
                    : type == GL_DEBUG_TYPE_OTHER               ? "Other"
                                                                 : "Unknown";
    if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM)
        ERRORLOG("[GL DEBUG] %s/%s id=%u: %s", src, tp, id, message);
    else
        WARNLOG("[GL DEBUG] %s/%s id=%u: %s", src, tp, id, message);
}

RendererOpenGL::RendererOpenGL() = default;

RendererOpenGL::~RendererOpenGL()
{
    Shutdown();
}

bool RendererOpenGL::Init()
{

    // Todo : Make opaque, this bleeds SDL into OpenGL
    SDL_Window* window = GetSDLWindowSystem()->GetSDLWindow();
    if (window == nullptr)
    {
        ERRORLOG("RendererOpenGL::Init: no SDL window (startup ordering invariant violated -- "
                 "the window must exist before RendererInit() runs)");
        return false;
    }
    if (!(SDL_GetWindowFlags(window) & SDL_WINDOW_OPENGL))
    {
        ERRORLOG("RendererOpenGL::Init: window was not created with SDL_WINDOW_OPENGL "
                 "(startup ordering invariant violated -- see RendererGetRequiredWindowFlags())");
        return false;
    }

    // TODO : Create Context from abstraction layer, not direrctly from SDL. This is a temporary solution to get the OpenGL renderer working.
    SDL_GLContext ctx = SDL_GL_CreateContext(window);
    if (ctx == nullptr)
    {
        ERRORLOG("RendererOpenGL::Init: SDL_GL_CreateContext failed: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetSwapInterval(0);

    {
        int floatbuf = 0;
        SDL_GL_GetAttribute(SDL_GL_FLOATBUFFERS, &floatbuf);
        if (floatbuf != 0)
        {
            WARNLOG("RendererOpenGL::Init: driver granted a float (scRGB) backbuffer -- "
                    "gamma-lift compensation is not implemented on this branch yet "
                    "(see develop's FGApplyScrgbLift); colours will look washed out "
                    "until it is ported");
        }
    }

    SDL_ShowWindow(window);

    // Todo : replace with agnostic Hdr initialization. This is a temporary solution to get the OpenGL renderer working.
#ifdef _WIN32
    PlatformGLHdrWin_OnContextReady(window,
        (GetSDLWindowSystem()->GetWindowFlags() & KFX_WF_FULLSCREEN_DESKTOP) != 0);
#endif

    if (!SDL_GL_MakeCurrent(window, nullptr))
    {
        ERRORLOG("RendererOpenGL::Init: failed to release GL context on the game thread: %s", SDL_GetError());
        SDL_GL_DestroyContext(ctx);
#ifdef _WIN32
        PlatformGLHdrWin_Shutdown();
#endif
        return false;
    }
    m_gl_context = ctx;

    m_impl = new Impl();

    m_impl->thread_mgr.Start(
        [this]{ render_thread_init(); },
        [this]{ render_thread_work(); },
        [this]{ render_thread_cleanup(); });

    if (!m_impl->init_ok)
    {

        m_impl->thread_mgr.Stop();
        delete m_impl;
        m_impl = nullptr;
        m_gl_context = nullptr; // destroyed by render_thread_cleanup()
#ifdef _WIN32
        PlatformGLHdrWin_Shutdown();
#endif
        return false;
    }

    return true;
}

void RendererOpenGL::Shutdown()
{
    if (m_impl != nullptr)
    {
        m_impl->thread_mgr.Stop(); // runs render_thread_cleanup() on the render thread, joins
        delete m_impl;
        m_impl = nullptr;
        m_gl_context = nullptr; // destroyed by render_thread_cleanup()
    }
    if (m_gl_context != nullptr)
    {
        SDL_GL_DestroyContext(static_cast<SDL_GLContext>(m_gl_context));
        m_gl_context = nullptr;
    }
#ifdef _WIN32
    // Todo : replace with agnostic Hdr shutdown. This is a temporary solution to get the OpenGL renderer working.
    PlatformGLHdrWin_Shutdown();
#endif
}

// Render thread. Everything that creates or touches GL state lives here so
// it all runs on the thread that ends up owning the context.
void RendererOpenGL::render_thread_init()
{
    SDL_GLContext ctx = static_cast<SDL_GLContext>(m_gl_context);
    if (!SDL_GL_MakeCurrent(GetSDLWindowSystem()->GetSDLWindow(), ctx))
    {
        ERRORLOG("RendererOpenGL::Init: SDL_GL_MakeCurrent failed: %s", SDL_GetError());
        m_impl->init_ok = false;
        return;
    }

    SYNCDBG(0, "RendererOpenGL::Init: GL context current on render thread (version: %s)", (const char*)glGetString(GL_VERSION));

    if (!GLFunctions_Load())
    {
        ERRORLOG("RendererOpenGL::Init: failed to load required GL entry points");
        m_impl->init_ok = false;
        return;
    }
    m_impl->functions_loaded = true;

    if (glDebugMessageCallback != nullptr)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugCallback, nullptr);
        if (glDebugMessageControl != nullptr)
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
    else
    {
        WARNLOG("RendererOpenGL::Init: GL_KHR_debug not available; debug callback disabled");
    }

    m_impl->atlas.SetResourceMapper(&m_impl->resource_mapper);
    m_impl->ui.SetResourceMapper(&m_impl->resource_mapper);
    if (!m_impl->atlas.Init() || !m_impl->ui.Init())
    {
        ERRORLOG("RendererOpenGL::Init: UI renderer/atlas init failed");
        m_impl->init_ok = false;
        return;
    }

    unsigned char black_palette[256 * 4] = {};
    for (int i = 0; i < 256; ++i)
        black_palette[i * 4 + 3] = 255; // alpha opaque, RGB stays zeroed

    GpuTextureDesc palette_desc;
    palette_desc.width = 256;
    palette_desc.height = 1;
    palette_desc.format = GpuTextureFormat::RGBA8;
    palette_desc.min_filter = GpuTextureFilter::Nearest;
    palette_desc.mag_filter = GpuTextureFilter::Nearest;
    palette_desc.wrap = GpuTextureWrap::Clamp;
    palette_desc.initial_pixels.assign(black_palette, black_palette + sizeof(black_palette));
    palette_desc.debug_name = "palette";
    m_impl->palette_tex_handle = m_impl->resource_mapper.RequestCreateTexture(palette_desc);
    if (m_impl->resource_mapper.ResolveTexture(m_impl->palette_tex_handle) == nullptr)
    {
        ERRORLOG("RendererOpenGL::Init: palette texture realization failed");
        m_impl->init_ok = false;
        return;
    }

    // 256x64 R8 remap/fade table, matching pixmap.fade_tables.
    GpuTextureDesc fade_table_desc;
    fade_table_desc.width = 256;
    fade_table_desc.height = 64;
    fade_table_desc.format = GpuTextureFormat::R8;
    fade_table_desc.min_filter = GpuTextureFilter::Nearest;
    fade_table_desc.mag_filter = GpuTextureFilter::Nearest;
    fade_table_desc.initial_pixels.assign(pixmap.fade_tables, pixmap.fade_tables + (256 * 64));
    fade_table_desc.debug_name = "fade_table";
    m_impl->fade_table_tex_handle = m_impl->resource_mapper.RequestCreateTexture(fade_table_desc);
    if (m_impl->resource_mapper.ResolveTexture(m_impl->fade_table_tex_handle) == nullptr)
    {
        ERRORLOG("RendererOpenGL::Init: fade table texture realization failed");
        m_impl->init_ok = false;
        return;
    }

    m_impl->ui.SetAtlas(&m_impl->atlas);
    m_impl->ui.SetPaletteTexture(m_impl->palette_tex_handle);
    m_impl->ui.SetFadeTableTexture(m_impl->fade_table_tex_handle);
    m_impl->ui.SetScreenSize((int)RendererPhysicalWidth(), (int)lbDisplay.PhysicalScreenHeight);
    m_impl->text.SetUIRenderer(&m_impl->ui);
    m_impl->cursor.SetUIRenderer(&m_impl->ui);
    m_impl->cursor.SetWorldRenderer(&m_impl->world);


    m_impl->world_atlas.SetResourceMapper(&m_impl->resource_mapper);
    m_impl->world.SetAtlas(&m_impl->world_atlas);
    m_impl->world.SetResourceMapper(&m_impl->resource_mapper);
    m_impl->world.SetFadeTexture(m_impl->fade_table_tex_handle);
    m_impl->world.SetPaletteTexture(m_impl->palette_tex_handle);
    m_impl->world.SetScreenSize((int)RendererPhysicalWidth(), (int)lbDisplay.PhysicalScreenHeight);
    if (!m_impl->world.CompileShaders())
    {
        ERRORLOG("RendererOpenGL::Init: world view renderer shader compile failed");
        m_impl->init_ok = false;
        return;
    }

    m_impl->mapfade.SetResourceMapper(&m_impl->resource_mapper);
    if (!m_impl->mapfade.CompileShaders())
    {
        WARNLOG("RendererOpenGL::Init: parchment transition shaders unavailable -- transition disabled");
    }

    m_impl->imgpresent.SetResourceMapper(&m_impl->resource_mapper);
    m_impl->imgpresent.SetPaletteTexture(m_impl->palette_tex_handle);
    if (!m_impl->imgpresent.CompileShaders())
    {
        WARNLOG("RendererOpenGL::Init: raw image present shaders unavailable -- FMV/splash will fall back to CPU blit");
    }

    m_impl->zoomboxtiles.SetResourceMapper(&m_impl->resource_mapper);
    m_impl->zoomboxtiles.SetPaletteTexture(m_impl->palette_tex_handle);
    m_impl->zoomboxtiles.SetTileAtlas(&m_impl->world_atlas);
    if (!m_impl->zoomboxtiles.CompileShaders())
    {
        WARNLOG("RendererOpenGL::Init: zoom-box tile shaders unavailable -- zoom box terrain will fall back to CPU rasterisation");
    }

    for (GLFrameData& fd : m_impl->frames)
    {
        fd.ui_cmds.shared_seq   = &fd.frame_seq;
        fd.text_cmds.shared_seq = &fd.frame_seq;
        fd.parchment_ui_cmds.shared_seq   = &fd.frame_seq;
        fd.parchment_text_cmds.shared_seq = &fd.frame_seq;
    }
    m_impl->ui.SetUICommandBuffers(&m_impl->frames[0].ui_cmds);
    m_impl->text.SetTextCommandBuffers(&m_impl->frames[0].text_cmds);
    m_impl->world.SetWorldCommandBuffers(&m_impl->frames[0].world_cmds);
    m_impl->write_idx = 0;

    glClearColor(0.06f, 0.06f, 0.10f, 1.0f);
    m_impl->init_ok = true;
}

void RendererOpenGL::render_thread_work()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];

    m_impl->resource_mapper.ProcessDeferredDestroys(fd.sealed_frame_number);

    if (fd.palette_dirty)
    {
        fd.palette_dirty = false;
        const GLTexture* const tex = m_impl->resource_mapper.ResolveTexture(m_impl->palette_tex_handle);
        if (tex != nullptr)
        {
            glBindTexture(GL_TEXTURE_2D, tex->id);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, fd.palette_rgba);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    m_impl->atlas.FlushPendingGL();

    if (!m_impl->world_atlas.IsInitialized() && block_ptrs[0] != nullptr)
        m_impl->world_atlas.Init();

    if (!m_impl->fade_tables_refreshed && fade_tables_ready)
    {
        m_impl->world.RefreshFadeTableAndSettings();
        m_impl->fade_tables_refreshed = true;
    }

    RenderGraph::Execute(*this);

    SDL_GL_SwapWindow(GetSDLWindowSystem()->GetSDLWindow());

// Todo : Remove this should not be specific to Win32.
#ifdef _WIN32
    // DXGI factory calls are free-threaded -- safe to tick from the render
    // thread even though the factory was created on the game thread in
    // Init(). Matches develop's platform_swap_gl_buffers().
    PlatformGLHdrWin_Tick();
#endif
}

void RendererOpenGL::FGClearFrame()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];

    m_impl->world.SetScreenSize(fd.screen_w, fd.screen_h);
    m_impl->ui.SetScreenSize(fd.screen_w, fd.screen_h);

    glViewport(0, 0, fd.screen_w, fd.screen_h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_impl->ui.BuildQuadsFromIR(fd.ui_cmds);
}

void RendererOpenGL::FGBeginWorldCapture()
{
    m_impl->fg_lens_palette_active = m_impl->world.HasActiveLensPalette();
    if (m_impl->fg_lens_palette_active)
    {
        m_impl->world.GetActiveLensPaletteRGBA(m_impl->fg_lens_palette_rgba);
        const GLTexture* const tex = m_impl->resource_mapper.ResolveTexture(m_impl->palette_tex_handle);
        if (tex != nullptr)
        {
            glBindTexture(GL_TEXTURE_2D, tex->id);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, m_impl->fg_lens_palette_rgba);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    m_impl->fg_lens_captured = m_impl->world.BeginLensCapture();
}

void RendererOpenGL::FGExecuteWorld()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    m_impl->world.GPURenderNow(fd.world_cmds);
}

void RendererOpenGL::FGFlushSwipeOverlay()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    static const TextCommandBuffers s_no_text; // swipe never submits text
    m_impl->ui.DrawFromIR(fd.swipe_cmds, s_no_text, nullptr);
}

void RendererOpenGL::FGResolveWorldCapture()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    if (m_impl->fg_lens_captured)
        m_impl->world.ResolveLensComposite();

    if (m_impl->fg_lens_palette_active)
    {
        const GLTexture* const tex = m_impl->resource_mapper.ResolveTexture(m_impl->palette_tex_handle);
        if (tex != nullptr)
        {
            glBindTexture(GL_TEXTURE_2D, tex->id);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, fd.palette_rgba);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}

void RendererOpenGL::FGExecuteMapFade()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    if (m_impl->mapfade.IsActiveRT())
    {
        if (m_impl->mapfade.IsCapturePendingRT())
        {
            m_impl->mapfade.BeginParchmentCapture(fd.screen_w, fd.screen_h);
            m_impl->ui.DrawFromIR(fd.parchment_ui_cmds, fd.parchment_text_cmds, &m_impl->text);
            m_impl->mapfade.EndParchmentCapture();
        }
        m_impl->mapfade.ResolveComposite(fd.screen_w, fd.screen_h);
    }
}

void RendererOpenGL::FGCaptureWorldFrameIfPending()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    if (m_impl->mapfade.IsActiveRT() && m_impl->mapfade.IsCapturePendingRT())
        m_impl->mapfade.CaptureWorldFrame(fd.screen_w, fd.screen_h);
}

void RendererOpenGL::FGExecuteImagePresents()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    m_impl->imgpresent.Resolve(fd.screen_w, fd.screen_h);
}

void RendererOpenGL::FGDrawZoomBoxes()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    m_impl->zoomboxtiles.Resolve(fd.screen_w, fd.screen_h);
}

void RendererOpenGL::FGDrawWorldSpriteLayer()
{
    m_impl->ui.DrawWorldSpriteLayerRT();
}

void RendererOpenGL::FGDrawWorldOverlayFlatLayer()
{
    m_impl->ui.DrawWorldOverlayFlatLayerRT();
}

void RendererOpenGL::FGDrawGameUI()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    m_impl->ui.DrawGameUILayerRT(fd.text_cmds, &m_impl->text);
}

void RendererOpenGL::FGDrawFrontOverlay()
{
    m_impl->ui.DrawFrontOverlay();
}

void RendererOpenGL::FGDrawScreenTint()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    const float* tint = fd.screen_tint;
    if (tint[3] <= 0.0f)
        return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_impl->ui.DrawSolidRect(0.0f, 0.0f, (float)fd.screen_w, (float)fd.screen_h,
                             tint[0], tint[1], tint[2], tint[3]);
    glDisable(GL_BLEND);
}

bool RendererOpenGL::ScheduleScreenshot(const char* path, int fmt)
{
    if (!path || (fmt != 1 && fmt != 2))
        return false;
    m_impl->screenshot_path = path;
    m_impl->screenshot_fmt  = fmt;
    return true;
}

void RendererOpenGL::FGCaptureScreenshot()
{
    if (m_impl->screenshot_path.empty())
        return;

    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    const int w = fd.screen_w;
    const int h = fd.screen_h;
    if (w <= 0 || h <= 0)
    {
        m_impl->screenshot_path.clear();
        return;
    }

    std::vector<unsigned char> pixels((size_t)w * (size_t)h * 4);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // GL's origin is bottom-left; flip rows for a top-down image file.
    std::vector<unsigned char> flipped(pixels.size());
    const size_t row_bytes = (size_t)w * 4;
    for (int row = 0; row < h; ++row)
        std::memcpy(flipped.data() + (size_t)row * row_bytes,
                    pixels.data() + (size_t)(h - 1 - row) * row_bytes, row_bytes);

    SDL_Surface* surf = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32,
                                              flipped.data(), (int)row_bytes);
    if (surf != nullptr)
    {
        bool ok = (m_impl->screenshot_fmt == 1) ? IMG_SavePNG(surf, m_impl->screenshot_path.c_str())
                                                 : SDL_SaveBMP(surf, m_impl->screenshot_path.c_str());
        if (!ok)
            ERRORLOG("Screenshot save failed (%s): %s", m_impl->screenshot_path.c_str(), SDL_GetError());
        SDL_DestroySurface(surf);
    }
    m_impl->screenshot_path.clear();
}

void RendererOpenGL::FGExecuteCursor()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    m_impl->cursor.DrawSnapshot(fd.cursor_snapshot);
    m_impl->world.DrawCursorKeeperSprites();
}

void RendererOpenGL::render_thread_cleanup()
{
    if (m_impl->functions_loaded)
    {
        m_impl->resource_mapper.ShutdownAll();
        m_impl->ui.Shutdown();
        m_impl->atlas.Free();
        m_impl->world.Shutdown();
        m_impl->world_atlas.Free();
        m_impl->mapfade.Shutdown();
        m_impl->imgpresent.Shutdown();
        m_impl->zoomboxtiles.Shutdown();
    }
    if (m_gl_context != nullptr)
    {
        SDL_GL_DestroyContext(static_cast<SDL_GLContext>(m_gl_context));
        m_gl_context = nullptr;
    }
}

void RendererOpenGL::SetDisplayPalette(const unsigned char* rgb8)
{
    if (m_impl == nullptr || rgb8 == nullptr) return;

    GLFrameData& fd = m_impl->frames[m_impl->write_idx];
    for (int i = 0; i < 256; ++i) {
        fd.palette_rgba[i * 4 + 0] = rgb8[i * 3 + 0];
        fd.palette_rgba[i * 4 + 1] = rgb8[i * 3 + 1];
        fd.palette_rgba[i * 4 + 2] = rgb8[i * 3 + 2];
        fd.palette_rgba[i * 4 + 3] = 255;
    }
    fd.palette_dirty = true;
}

void RendererOpenGL::PresentFrame()
{
    if (m_gl_context == nullptr || GetSDLWindowSystem()->GetSDLWindow() == nullptr || m_impl == nullptr)
        return;

    RenderTaskProducerRegistry_ProduceAll(RendererFrameCounter_Current());

    m_impl->thread_mgr.WaitForCompletion();

    const int filled = m_impl->write_idx;  // this frame's IR, fully submitted by now
    const int next    = 1 - filled;        // safe to reuse for the NEXT frame

    GLFrameData& next_fd = m_impl->frames[next];
    next_fd.ui_cmds.Reset();
    next_fd.text_cmds.Reset();
    next_fd.world_cmds.Reset();
    next_fd.parchment_ui_cmds.Reset();
    next_fd.parchment_text_cmds.Reset();
    next_fd.swipe_cmds.Reset();
    next_fd.frame_seq = 0;
    next_fd.palette_dirty = false; // defensive; a real change always re-sets this itself

    
    m_impl->world.FlipBuffers();
    m_impl->mapfade.FlipBuffers();
    m_impl->imgpresent.FlipBuffers();
    m_impl->zoomboxtiles.FlipBuffers();

    m_impl->ui.SetUICommandBuffers(&next_fd.ui_cmds);
    m_impl->text.SetTextCommandBuffers(&next_fd.text_cmds);
    m_impl->world.SetWorldCommandBuffers(&next_fd.world_cmds);

    GLFrameData& filled_fd = m_impl->frames[filled];
    filled_fd.screen_w = (int32_t)RendererPhysicalWidth();
    filled_fd.screen_h = (int32_t)lbDisplay.PhysicalScreenHeight;
    std::memcpy(filled_fd.screen_tint, g_screen_tint, sizeof(filled_fd.screen_tint));
    filled_fd.sealed_frame_number = RendererFrameCounter_Current();

    LbMouseOnBeginSwap();
    filled_fd.cursor_snapshot = m_impl->cursor.Snapshot();
    LbMouseOnEndSwap();

    m_impl->render_idx = filled;
    m_impl->write_idx  = next;

    RendererFrameCounter_Advance();

    m_impl->thread_mgr.Signal();
}

bool RendererOpenGL::BeginFrame()
{
    return m_impl != nullptr && m_impl->init_ok;
}

void RendererOpenGL::EndFrame()
{
    // No-op: the real flip/signal work is PresentFrame() below, called
    // separately (unchanged) once a frame's submission is complete.
}

bool RendererOpenGL::PresentImage(const struct RendererPresentImageDesc* desc)
{
    if (m_impl == nullptr || !m_impl->imgpresent.IsReady())
        return false;
    m_impl->imgpresent.Submit(desc);
    return true;
}

bool RendererOpenGL::SubmitZoomBoxTiles(const uint16_t* tile_block_ids, int tiles_x, int tiles_y,
                                        int dst_x, int dst_y, int tile_w, int tile_h)
{
    if (m_impl == nullptr || !m_impl->zoomboxtiles.IsReady())
        return false;
    m_impl->zoomboxtiles.Submit(tile_block_ids, tiles_x, tiles_y, dst_x, dst_y, tile_w, tile_h);
    return true;
}

bool RendererOpenGL::SubmitLandviewZoom(const unsigned char* src_buf, int src_w, int src_h,
                                        float center_map_x, float center_map_y,
                                        float screen_cx,    float screen_cy,
                                        float scale)
{
    if (m_impl == nullptr || !m_impl->imgpresent.IsReady())
        return false;
    m_impl->imgpresent.SubmitZoom(src_buf, src_w, src_h, center_map_x, center_map_y,
                                  screen_cx, screen_cy, scale);
    return true;
}

class ITextRenderer* RendererOpenGL::GetTextRenderer()
{
    return m_impl ? &m_impl->text : nullptr;
}

class IUIRenderer* RendererOpenGL::GetUIRenderer()
{
    return m_impl ? &m_impl->ui : nullptr;
}

void RendererOpenGL::SubmitMapFadeStep(int tick_step, float display_step, bool fading_in)
{
    if (m_impl == nullptr) return;
    m_impl->mapfade.SubmitStep(tick_step, display_step, fading_in);
}

void RendererOpenGL::BeginOverlayCapture(OverlayCaptureKind kind)
{
    if (m_impl == nullptr) return;
    GLFrameData& fd = m_impl->frames[m_impl->write_idx];
    switch (kind)
    {
    case OVERLAY_CAPTURE_PARCHMENT:
        m_impl->ui.SetUICommandBuffers(&fd.parchment_ui_cmds);
        m_impl->text.SetTextCommandBuffers(&fd.parchment_text_cmds);
        break;
    case OVERLAY_CAPTURE_SWIPE:
        // Text isn't redirected: the swipe overlay only ever submits sprites.
        m_impl->ui.SetUICommandBuffers(&fd.swipe_cmds);
        break;
    }
}

void RendererOpenGL::EndOverlayCapture(OverlayCaptureKind kind)
{
    if (m_impl == nullptr) return;
    GLFrameData& fd = m_impl->frames[m_impl->write_idx];
    m_impl->ui.SetUICommandBuffers(&fd.ui_cmds);
    if (kind == OVERLAY_CAPTURE_PARCHMENT)
        m_impl->text.SetTextCommandBuffers(&fd.text_cmds);
}

bool RendererOpenGL::MapFadeSupportsNativeResolution() const
{
    return (m_impl != nullptr) && m_impl->mapfade.IsReady();
}

class ICursorLayer* RendererOpenGL::GetCursorLayer()
{
    return m_impl ? &m_impl->cursor : nullptr;
}

class IWorldViewRenderer* RendererOpenGL::GetWorldViewRenderer()
{
    return m_impl ? &m_impl->world : nullptr;
}
