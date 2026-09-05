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
#include <SDL3_image/SDL_image.h> // IMG_SavePNG (screenshots)
#include <cstring>
#include <string>
#include <vector>
#include "post_inc.h"

struct GLFrameData {
    UICommandBuffers    ui_cmds;
    TextCommandBuffers  text_cmds;
    WorldCommandBuffers world_cmds;
    uint32_t            frame_seq = 0;

    // Parchment transition (P5.8b): diverted target for
    // redraw_minimal_overhead_view()'s UI/text submissions, redirected here
    // (instead of ui_cmds/text_cmds) for the one call inside
    // prepare_map_fade_buffers() on the frame a transition starts, via
    // RendererBeginParchmentCapture()/RendererEndParchmentCapture(). Empty
    // every other frame -- DrawFromIR() on an empty buffer is a cheap no-op.
    UICommandBuffers    parchment_ui_cmds;
    TextCommandBuffers  parchment_text_cmds;

    // Swipe-attack overlay (Beat 2): diverted target for
    // draw_swipe_graphic()'s (thing_creature.c) UI submission, redirected
    // here (instead of ui_cmds) via RendererBeginSwipeOverlay()/
    // RendererEndSwipeOverlay() so FGFlushSwipeOverlay() can draw it inside
    // the lens FBO bracket (before FGDrawGameUI() runs) instead of it
    // landing in the general UI pass, which composites after the bracket
    // closes. Empty every frame the player isn't in possession mode --
    // DrawFromIR() on an empty buffer is a cheap no-op, same as
    // parchment_ui_cmds above.
    UICommandBuffers    swipe_cmds;

    int32_t screen_w = 0;
    int32_t screen_h = 0;

    // Beat 7: copy of g_screen_tint (RendererManager) at frame-submit time --
    // same handoff point as screen_w/h below, so FGDrawScreenTint() (render
    // thread) never reads the game thread's live global mid-update.
    float screen_tint[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

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
    unsigned int       palette_tex     = 0;
    unsigned int       fade_table_tex  = 0;

    RenderThreadManager thread_mgr;
    GLFrameData frames[2];
    int write_idx  = 0; // which frames[] slot Submit* is currently appending into
    int render_idx = 0; // which frames[] slot the render thread is consuming (only valid between Signal() and the matching WaitForCompletion())

    bool init_ok         = false; // set by render_thread_init(), read after Start() returns
    bool functions_loaded = false; // guards render_thread_cleanup() from calling gl* before GLFunctions_Load() ran

    // Render-thread-only scratch, valid for the span of one RenderGraph::Execute()
    // call. Used to be locals inside the single render_thread_work() function;
    // now split across FGBeginWorldCapture()/FGResolveWorldCapture(), so they
    // need to persist on Impl between those two calls instead of the stack.
    bool          fg_lens_captured      = false;
    bool          fg_lens_palette_active = false;
    unsigned char fg_lens_palette_rgba[256 * 4] = {};

    // Beat 8: screenshot request handoff. Set by ScheduleScreenshot() (game
    // thread, in response to the screenshot hotkey -- a rare, one-shot
    // action), consumed by FGCaptureScreenshot() (render thread). Not
    // atomic/locked, matching every other game-thread -> render-thread value
    // on this branch (GL doesn't genuinely run the two concurrently yet --
    // see the P5.6 notes elsewhere in this file).
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
    // The window is created exactly once, by LbScreenSetup() before
    // RendererInit() ever runs (main.cpp's resolve_startup_config() resolves
    // the renderer type -- and with it, the window's required SDL3 flags --
    // before setup_screen_mode_zero() creates it). By the time Init() runs
    // here, the window must already exist and already carry SDL_WINDOW_OPENGL;
    // there is no "recreate the window to add the flag" step any more. A
    // violation here means the startup ordering broke, not something to
    // silently work around with a recreated/placeholder window -- fail loud.
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

    if (!m_impl->atlas.Init() || !m_impl->ui.Init())
    {
        ERRORLOG("RendererOpenGL::Init: UI renderer/atlas init failed");
        m_impl->init_ok = false;
        return;
    }

    // 256x1 RGBA8 palette lookup texture; content uploaded from the per-frame
    // snapshot in render_thread_work() whenever SetDisplayPalette() marked
    // one dirty. Seeded black+opaque (not `nullptr`, which leaves undefined
    // driver content) so any sprite sampled before the first real upload
    // lands reads a defined black, not implementation-defined VRAM garbage
    // that happened to read back as solid white -- RendererManager.cpp's
    // RendererInit() also replays the last palette set before this backend
    // existed, so in practice this seed is a defensive fallback, not the
    // load-bearing fix; a real upload should land within the first frame.
    unsigned char black_palette[256 * 4] = {};
    for (int i = 0; i < 256; ++i)
        black_palette[i * 4 + 3] = 255; // alpha opaque, RGB stays zeroed

    glGenTextures(1, &m_impl->palette_tex);
    glBindTexture(GL_TEXTURE_2D, m_impl->palette_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, black_palette);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 256x64 R8 remap/fade table, matching pixmap.fade_tables. Uploaded once
    // here; a runtime fade-table rebuild wouldn't be picked up -- not hit by
    // this milestone's draw paths, but worth knowing if remap output looks stale.
    glGenTextures(1, &m_impl->fade_table_tex);
    glBindTexture(GL_TEXTURE_2D, m_impl->fade_table_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 256, 64, 0, GL_RED, GL_UNSIGNED_BYTE, pixmap.fade_tables);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_impl->ui.SetAtlas(&m_impl->atlas);
    m_impl->ui.SetPaletteTexture(m_impl->palette_tex);
    m_impl->ui.SetFadeTableTexture(m_impl->fade_table_tex);
    m_impl->ui.SetScreenSize((int)RendererPhysicalWidth(), (int)lbDisplay.PhysicalScreenHeight);
    m_impl->text.SetUIRenderer(&m_impl->ui);
    m_impl->cursor.SetUIRenderer(&m_impl->ui);
    // Power-hand keeper sprite(s) (P5.7.5 / Beat 3) share world's keeper-
    // sprite atlas/shaders/IR (BeginCursorCapture()/SubmitKeeperSprite()/
    // EndCursorCapture()) rather than owning a second copy of that machinery.
    m_impl->cursor.SetWorldRenderer(&m_impl->world);

    // World geometry renderer (P5.7.2b). world_atlas.Init() is NOT called
    // here -- unlike the sprite atlas, its source data (block_ptrs[]) isn't
    // populated yet this early (textures load per-level, after startup), so
    // it's retried once per render_thread_work() tick instead (see there).
    m_impl->world.SetAtlas(&m_impl->world_atlas);
    m_impl->world.SetFadeTexture(m_impl->fade_table_tex);
    m_impl->world.SetPaletteTexture(m_impl->palette_tex);
    m_impl->world.SetScreenSize((int)RendererPhysicalWidth(), (int)lbDisplay.PhysicalScreenHeight);
    if (!m_impl->world.CompileShaders())
    {
        ERRORLOG("RendererOpenGL::Init: world view renderer shader compile failed");
        m_impl->init_ok = false;
        return;
    }

    // Parchment transition (P5.8b). Same non-fatal treatment as P5.8a's
    // lens shaders: a compile failure just means the transition silently
    // stays a no-op (BeginParchmentCapture()/ResolveComposite() both guard
    // on m_shader/m_tex_* being non-zero), not a renderer init failure.
    if (!m_impl->mapfade.CompileShaders())
    {
        WARNLOG("RendererOpenGL::Init: parchment transition shaders unavailable -- transition disabled");
    }

    // Raw image present (FMV/splash). Same non-fatal treatment -- a compile
    // failure means PresentImage() keeps returning false (Resolve() guards
    // on m_shader being non-zero), so callers fall back to their existing
    // CPU blit, same as before this change.
    m_impl->imgpresent.SetPaletteTexture(m_impl->palette_tex);
    if (!m_impl->imgpresent.CompileShaders())
    {
        WARNLOG("RendererOpenGL::Init: raw image present shaders unavailable -- FMV/splash will fall back to CPU blit");
    }

    // GL always defers -- unlike software, there is no immediate draw path,
    // so a write buffer stays bound for the renderer's whole lifetime; which
    // FrameData slot it points at flips every PresentFrame().
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

// Render thread, once per present. Consumes m_impl->frames[render_idx],
// which PresentFrame() sealed and handed off (via Signal()) before this ran.
//
// The per-frame draw sequence itself is NOT inlined here any more -- it's
// RenderGraph::Execute(*this), the same shared, centrally-owned phase order
// RendererSoftware uses (see RendererOpenGL.h's class comment for why).
// What stays here is backend-resource bookkeeping that isn't frame *content*
// (palette upload, atlas maintenance) as a prologue, and the buffer swap as
// an epilogue -- matching develop's own RenderGraph::Execute() comment that
// the swap stays in the backend, outside the graph.
void RendererOpenGL::render_thread_work()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];

    if (fd.palette_dirty)
    {
        fd.palette_dirty = false;
        glBindTexture(GL_TEXTURE_2D, m_impl->palette_tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, fd.palette_rgba);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    m_impl->atlas.FlushPendingGL();

    // Tile atlas build, retried once per tick until block_ptrs[] is
    // populated (P5.7.2b) -- level textures load on the game thread, after
    // this render thread has already started ticking, so this can't be a
    // one-shot init the way the sprite atlas's can. Cheap no-op once built
    // (IsInitialized() short-circuits), same shape as atlas.FlushPendingGL()
    // above. Does NOT handle a texture reload after the first build (Lua's
    // MAP.default_texture can trigger one mid-session) -- deliberately out
    // of scope, see GLWorldViewRenderer.cpp's file header.
    if (!m_impl->world_atlas.IsInitialized() && block_ptrs[0] != nullptr)
        m_impl->world_atlas.Init();

    RenderGraph::Execute(*this);

    SDL_GL_SwapWindow(GetSDLWindowSystem()->GetSDLWindow());

#ifdef _WIN32
    // DXGI factory calls are free-threaded -- safe to tick from the render
    // thread even though the factory was created on the game thread in
    // Init(). Matches develop's platform_swap_gl_buffers().
    PlatformGLHdrWin_Tick();
#endif
}

// ---------------------------------------------------------------------------
// IFrameGraphExecutor phases. Called from render_thread_work() above, via
// RenderGraph::Execute(*this), in RenderGraph.cpp's fixed order. Each reads
// the current frame's GLFrameData fresh -- render_idx doesn't change for the
// span of one Execute() call, so this is just the old function-local `fd`
// split across methods instead of computed once at the top.
// ---------------------------------------------------------------------------

void RendererOpenGL::FGClearFrame()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];

    // world/ui's cached "full screen" dimensions were previously set exactly
    // once, at render_thread_init() time (the frontend/legal-screen
    // resolution) and never refreshed -- entering a level at a different
    // resolution (the shipped config's FRONTEND_RES=640x480w32 vs
    // INGAME_RES=DESKTOP guarantees a mismatch) left GLWorldViewRenderer's
    // viewport-Y math computed against the stale size, pushing the world
    // geometry off-screen with no GL error (pure viewport-rect arithmetic).
    // fd.screen_w/h is already refreshed every frame from
    // RendererPhysicalWidth()/Height (PresentFrame()'s stamp below) --
    // both setters are trivial field assignments, so just re-assert it here
    // every frame rather than hunting down every video-mode-change call site
    // that would otherwise need its own refresh hook.
    m_impl->world.SetScreenSize(fd.screen_w, fd.screen_h);
    m_impl->ui.SetScreenSize(fd.screen_w, fd.screen_h);

    glViewport(0, 0, fd.screen_w, fd.screen_h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Beat 5: classify this frame's ui_cmds into the WorldOverlay/
    // WorldOverlayFlat/GameUI quad layers up front, before any of the phases
    // that consume them run (FGDrawWorldSpriteLayer/FGDrawWorldOverlayFlatLayer/
    // FGDrawGameUI, in that order later in this same frame) -- classification
    // is pure CPU work, so doing it once here rather than splitting it across
    // 3 call sites keeps BuildQuadsFromIR() itself simple (one pass over the
    // whole buffer, not 3 partial ones).
    m_impl->ui.BuildQuadsFromIR(fd.ui_cmds);
}

// Possession lens (P5.8a): BeginLensCapture() redirects the world pass into
// an offscreen FBO when a pixel distortion effect is active this frame;
// FGResolveWorldCapture() then composites it back into the real viewport
// sub-rect of the backbuffer. A palette-only lens (no pixel effect) doesn't
// redirect -- instead the palette texture is swapped to the tinted lens
// palette for just the world draw, then restored before UI/cursor so they're
// unaffected, matching the CPU path's PaletteEffect being a pure
// palette-state effect with no pixel write of its own. fg_lens_captured/
// fg_lens_palette_active/fg_lens_palette_rgba persist on Impl between this
// call and FGResolveWorldCapture() -- they used to be locals spanning both
// halves of one function.
void RendererOpenGL::FGBeginWorldCapture()
{
    m_impl->fg_lens_palette_active = m_impl->world.HasActiveLensPalette();
    if (m_impl->fg_lens_palette_active)
    {
        m_impl->world.GetActiveLensPaletteRGBA(m_impl->fg_lens_palette_rgba);
        glBindTexture(GL_TEXTURE_2D, m_impl->palette_tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, m_impl->fg_lens_palette_rgba);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    m_impl->fg_lens_captured = m_impl->world.BeginLensCapture();
}

void RendererOpenGL::FGExecuteWorld()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    m_impl->world.GPURenderNow(fd.world_cmds);
}

// Possession-mode swipe overlay (Beat 2). RenderGraph::Execute() calls this
// right after FGExecuteWorld(), still inside the lens FBO bracket (before
// FGResolveWorldCapture() closes it) -- drawing here, rather than letting
// draw_swipe_graphic()'s submission land in the general ui_cmds buffer
// (consumed later by FGDrawGameUI(), after the bracket closes), is what
// makes the swipe sprite lens-distorted like develop's does instead of
// compositing flat on top of the finished frame.
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
        glBindTexture(GL_TEXTURE_2D, m_impl->palette_tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, fd.palette_rgba);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

// Parchment transition (P5.8b). Capture happens once, on the frame the
// transition starts: BeginParchmentCapture()/DrawFromIR() (against the
// diverted parchment_ui_cmds/parchment_text_cmds, populated via
// RendererBeginParchmentCapture()/RendererEndParchmentCapture() around
// prepare_map_fade_buffers()'s redraw_minimal_overhead_view() call) captures
// the overhead map into its own texture; CaptureWorldFrame() then blits this
// frame's already-drawn world (FGExecuteWorld() above -- map-fade runs after
// world in RenderGraph's order) into the other. ResolveComposite() runs
// every active frame, not just the capture one -- non-capture frames have
// nothing else to show (see the plan doc), so this is the only visible
// content until the transition ends.
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
            // CaptureWorldFrame() moved to FGCaptureWorldFrameIfPending() below
            // -- see that override's comment for why.
        }
        m_impl->mapfade.ResolveComposite(fd.screen_w, fd.screen_h);
    }
}

// Regression fix: CaptureWorldFrame() used to run here, inside
// FGExecuteMapFade(), which RenderGraph::Execute() calls *before*
// FGDrawGameUI(). CaptureWorldFrame()'s own comment says it blits "world +
// UI, whatever this frame's normal draw calls already produced" -- but at
// that point in the frame, UI hadn't been drawn yet, so every parchment
// transition captured a snapshot missing the sidebar. develop deliberately
// keeps this as its own later phase, run after FGDrawGameUI so the sidebar
// is part of the crossfaded snapshot (RenderGraph's own ordering comment
// says so directly) -- this override restores that placement.
void RendererOpenGL::FGCaptureWorldFrameIfPending()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    if (m_impl->mapfade.IsActiveRT() && m_impl->mapfade.IsCapturePendingRT())
        m_impl->mapfade.CaptureWorldFrame(fd.screen_w, fd.screen_h);
}

// Raw image present (FMV/splash). Callers only submit one of these per
// BeginFrame()/EndFrame() bracket, so on a genuine FMV/splash frame this
// frame's other content (world/UI/mapfade) is empty -- drawing this here,
// per RenderGraph's fixed order, is safe regardless.
void RendererOpenGL::FGExecuteImagePresents()
{
    GLFrameData& fd = m_impl->frames[m_impl->render_idx];
    m_impl->imgpresent.Resolve(fd.screen_w, fd.screen_h);
}

// Draws UI *and* text together: GLUIRenderer::DrawFromIR() interleaves them
// by shared sequence number (see its own header comment), the same ordering
// the software renderer's replay relies on -- they are not independent draw
// batches the way develop's separate FGDrawGameUI/FGExecuteText assume.
// FGExecuteText is left as IFrameGraphExecutor's inherited no-op.
// Beat 5: flush the WorldOverlay/WorldOverlayFlat quad layers
// BuildQuadsFromIR() (FGClearFrame(), above) already classified this frame's
// ui_cmds into. RenderGraph::Execute() calls these right after map-fade
// compose, before image-presents -- was already correctly scaffolded
// (IFrameGraphExecutor's no-op defaults), just never overridden until now.
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

// Present-time (before swap), matching RenderGraph.cpp's phase order.
// glReadPixels() reads whatever is currently in the backbuffer -- everything
// drawn earlier this frame (world, UI, tint, cursor) -- so this must stay
// the last real draw-adjacent phase, same as develop's placement.
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
    // Power-hand keeper sprites (held thing + hand graphic), captured this
    // frame via GLCursorLayer::SubmitKeeperHandSprite() -> GLWorldViewRenderer::
    // BeginCursorCapture()/SubmitKeeperSprite()/EndCursorCapture() and flipped
    // into m_rt_cursor_kspr_ir by world.FlipBuffers() (P5.6's usual game/render
    // thread handoff point) -- drawn separately from DrawSnapshot()'s pointer
    // sprite since this is GLWorldViewRenderer's own IR, not GLCursorLayer's.
    m_impl->world.DrawCursorKeeperSprites();
}

// Render thread, once, at Stop()/join. Must tolerate a partial init (Start()
// always runs this even if render_thread_init() bailed out early -- see
// Init()'s init_ok check), so every step is guarded by what actually
// succeeded rather than assumed.
void RendererOpenGL::render_thread_cleanup()
{
    if (m_impl->functions_loaded)
    {
        m_impl->ui.Shutdown();
        m_impl->atlas.Free();
        m_impl->world.Shutdown();
        m_impl->world_atlas.Free();
        m_impl->mapfade.Shutdown();
        m_impl->imgpresent.Shutdown();
        if (m_impl->palette_tex)    glDeleteTextures(1, &m_impl->palette_tex);
        if (m_impl->fade_table_tex) glDeleteTextures(1, &m_impl->fade_table_tex);
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

    // Deferred, not applied here: this can be called from the game thread at
    // any point during a frame (palette fades reprogram it every frame), and
    // once threaded, direct gl* calls from here would run concurrently with
    // the render thread's own GL work on a context that isn't current on
    // this thread. Stash into the slot the game thread currently owns
    // (frames[write_idx]); render_thread_work() uploads it when that slot's
    // turn comes, the same handoff every other per-frame draw already goes
    // through.
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

    // Blocks until the render thread finishes the previous handoff's work
    // (first call returns immediately -- nothing signalled yet). After this,
    // frames[next] is guaranteed idle: the render thread is done reading it.
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

    // GLWorldViewRenderer's own draw-command ordering list (m_draw_cmds,
    // separate from the WorldCommandBuffers vertex payload rebound below)
    // has its own internal double buffer -- flip it at the same handoff
    // point frames[] itself flips, before rebinding the write target.
    m_impl->world.FlipBuffers();
    // Parchment transition (P5.8b): same handoff point, its own small
    // double buffer (IRMapFadeCmd), not part of GLFrameData's Reset() above
    // since it isn't reset to empty so much as reset to "inactive" -- see
    // GLMapFadePass::FlipBuffers()'s own comment.
    m_impl->mapfade.FlipBuffers();
    m_impl->imgpresent.FlipBuffers();

    // From here, engine draw calls for the NEXT frame land in frames[next]
    // while the render thread works through frames[filled] concurrently.
    m_impl->ui.SetUICommandBuffers(&next_fd.ui_cmds);
    m_impl->text.SetTextCommandBuffers(&next_fd.text_cmds);
    m_impl->world.SetWorldCommandBuffers(&next_fd.world_cmds);

    GLFrameData& filled_fd = m_impl->frames[filled];
    filled_fd.screen_w = (int32_t)RendererPhysicalWidth();
    filled_fd.screen_h = (int32_t)lbDisplay.PhysicalScreenHeight;
    std::memcpy(filled_fd.screen_tint, g_screen_tint, sizeof(filled_fd.screen_tint));

    // LbMouseOnBeginSwap() is what actually submits the OS pointer sprite
    // (-> CursorLayer_SubmitPointerSprite() -> GLCursorLayer's m_pointer_handle)
    // -- RendererSoftware::PresentFrame() calls this pair around its own
    // present (see its comment), but nothing called it for GL at all, so the
    // cursor was never submitted and FGExecuteCursor() had nothing to draw
    // every single frame. LbMouseOnEndSwap() is a no-op for a deferred
    // backend (see LbI_PointerHandler::OnEndSwap()'s own comment) so calling
    // it immediately after, on the game thread, rather than after the actual
    // async present completes on the render thread, is correct -- there's no
    // state to restore either way.
    LbMouseOnBeginSwap();
    filled_fd.cursor_snapshot = m_impl->cursor.Snapshot();
    LbMouseOnEndSwap();

    m_impl->render_idx = filled;
    m_impl->write_idx  = next;

    m_impl->thread_mgr.Signal();
}

// Frame bracket (see IRenderer.h). GL has no CPU pointer to hand back --
// content for this frame is already going into frames[write_idx] via the
// existing Submit*/DrawGlyphs/etc. bridges, called by the same game code
// that used to be gated behind RendererLockFramebuffer(). Succeeds
// unconditionally once render_thread_init() has actually run (init_ok),
// matching the old gate's one real check (lbScreenInitialised is checked
// by the RendererBeginFrame() bridge itself, before this is even called).
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

// Parchment transition (P5.8b). Game thread -- called via the
// RendererSubmitMapFadeStep()/RendererBeginParchmentCapture()/
// RendererEndParchmentCapture()/MapFadeSupportsNativeResolution() bridges
// (RendererManager.cpp), from engine_redraw.c/gui_parchment.c.
void RendererOpenGL::SubmitMapFadeStep(int tick_step, float display_step, bool fading_in)
{
    if (m_impl == nullptr) return;
    m_impl->mapfade.SubmitStep(tick_step, display_step, fading_in);
}

void RendererOpenGL::BeginParchmentCapture()
{
    if (m_impl == nullptr) return;
    // Redirect the UI/text renderers' write target to this frame's
    // parchment-capture buffers -- restored by EndParchmentCapture() right
    // after the one call (redraw_minimal_overhead_view(), via
    // prepare_map_fade_buffers()) this brackets.
    GLFrameData& fd = m_impl->frames[m_impl->write_idx];
    m_impl->ui.SetUICommandBuffers(&fd.parchment_ui_cmds);
    m_impl->text.SetTextCommandBuffers(&fd.parchment_text_cmds);
}

void RendererOpenGL::EndParchmentCapture()
{
    if (m_impl == nullptr) return;
    GLFrameData& fd = m_impl->frames[m_impl->write_idx];
    m_impl->ui.SetUICommandBuffers(&fd.ui_cmds);
    m_impl->text.SetTextCommandBuffers(&fd.text_cmds);
}

void RendererOpenGL::BeginSwipeOverlay()
{
    if (m_impl == nullptr) return;
    // Redirect the UI renderer's write target to this frame's swipe-overlay
    // buffer -- restored by EndSwipeOverlay() right after the one call
    // (draw_swipe_graphic(), thing_creature.c) this brackets. Text isn't
    // redirected: the swipe overlay only ever submits sprites.
    GLFrameData& fd = m_impl->frames[m_impl->write_idx];
    m_impl->ui.SetUICommandBuffers(&fd.swipe_cmds);
}

void RendererOpenGL::EndSwipeOverlay()
{
    if (m_impl == nullptr) return;
    GLFrameData& fd = m_impl->frames[m_impl->write_idx];
    m_impl->ui.SetUICommandBuffers(&fd.ui_cmds);
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
