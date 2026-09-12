#include "pre_inc.h"
#include "kfx/renderer/RendererManager.h"
#include "kfx/renderer/RendererSoftware.h"
#include "kfx/renderer/RendererOpenGL.h"
#include "bflib_basics.h"
#include "bflib_video.h"
#include "bflib_sprfnt.h"   // LbTextDrawResizedImmediate
#include "kfx/renderer/ITextRenderer.h"
#include "kfx/renderer/IUIRenderer.h"
#include "kfx/renderer/ICursorLayer.h"
#include "kfx/renderer/IWorldViewRenderer.h"
#include "kfx/renderer/ir/WorldCommands.h"
#include "kfx/lense/LensManager.h"
#include "kfx/renderer/RendererSettings.h"
#include "bflib_vidraw.h"   // LbSpriteDraw*Immediate, setup_vecs (world-view immediate fallback)
#include "gui_draw.h"       // draw_slab64k_background_immediate
#include "engine_render.h"  // display_drawlist/display_fast_drawlist (world-view immediate fallback)
#include <cstring>          // memcpy (s_last_palette_rgb8 buffering)
#include "post_inc.h"

static IRenderer*   s_active_renderer = nullptr;
static RendererType s_active_type     = RENDERER_INVALID;
static unsigned char s_draw_colour = 0;
static unsigned short s_draw_flags = 0;

// Last 8-bit display palette passed to RendererSetDisplayPalette(), kept
// even when there's no active backend to receive it yet. LbScreenSetup()
// calls RendererPaletteSet() -> RendererSetDisplayPalette() before
// RendererInit() ever runs (setup_screen_mode_zero() in main.cpp, ahead of
// RendererInit() by design -- see the window-lifecycle port), so the very
// first palette set was previously just dropped on the floor for whichever
// backend hadn't been created yet: RendererInit() replays this once the
// backend exists, so a GL palette texture created with undefined content
// (see RendererOpenGL.cpp's palette_tex comment) gets a real upload before
// its first sample, not zero-initialized bytes that happen to be garbage.
static unsigned char s_last_palette_rgb8[PALETTE_SIZE];
static bool          s_have_last_palette = false;

// Allocate a backend for the requested type, or nullptr if unknown.
static IRenderer* create_renderer(RendererType type)
{
    switch (type)
    {
        case RENDERER_SOFTWARE: return new RendererSoftware();
        case RENDERER_OPENGL:   return new RendererOpenGL();
        default:                return nullptr;
    }
}

RendererType RendererResolveType(RendererType requested)
{
    return (requested == RENDERER_AUTO) ? RENDERER_SOFTWARE : requested;
}

unsigned int RendererGetRequiredWindowFlags(RendererType type)
{
    switch (RendererResolveType(type))
    {
        case RENDERER_OPENGL: return KFX_WF_OPENGL;
        default:               return 0;
    }
}

int RendererInit(RendererType type)
{
    // Initialise settings to defaults on first call (Beat 4, ported from
    // develop). keeperfx.cfg parsing may override startup-only knobs before
    // this function is reached; only reset if this is the first init.
    static int s_settings_initialised = 0;
    if (!s_settings_initialised) {
        RendererSettings_Reset();
        s_settings_initialised = 1;
    }

    if (s_active_renderer != nullptr)
        RendererShutdown();

    RendererType resolved = RendererResolveType(type);
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
    if (s_have_last_palette)
        s_active_renderer->SetDisplayPalette(s_last_palette_rgb8);
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
    memcpy(s_last_palette_rgb8, rgb8, PALETTE_SIZE);
    s_have_last_palette = true;
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

TbBool RendererBeginFrame(void)
{
    if (!lbScreenInitialised || s_active_renderer == nullptr)
        return 0;
    if (!s_active_renderer->BeginFrame())
        return 0;
    if (!s_active_renderer->GetCapabilities().hasGPURenderPath)
    {
        int pitch = 0;
        unsigned char* px = s_active_renderer->LockFramebuffer(&pitch);
        if (px == nullptr)
        {
            lbDisplay.GraphicsWindowPtr = NULL;
            lbDisplay.WScreen = NULL;
            s_active_renderer->EndFrame();
            return 0;
        }
        lbDisplay.WScreen = px;
        lbDisplay.GraphicsScreenWidth = pitch;
        lbDisplay.GraphicsWindowPtr = &lbDisplay.WScreen[lbDisplay.GraphicsWindowX +
            lbDisplay.GraphicsScreenWidth * lbDisplay.GraphicsWindowY];
    }
    return 1;
}

void RendererEndFrame(void)
{
    if (s_active_renderer == nullptr)
        return;
    if (!s_active_renderer->GetCapabilities().hasGPURenderPath)
    {
        lbDisplay.WScreen = NULL;
        lbDisplay.GraphicsWindowPtr = NULL;
        s_active_renderer->UnlockFramebuffer();
    }
    s_active_renderer->EndFrame();
}

TbBool RendererPresentImage(const struct RendererPresentImageDesc* desc)
{
    if (s_active_renderer == nullptr || desc == nullptr)
        return 0;
    return s_active_renderer->PresentImage(desc) ? 1 : 0;
}

TbBool RendererSubmitLandviewZoom(const unsigned char *src_buf, int src_w, int src_h,
                                  float center_map_x, float center_map_y,
                                  float screen_cx,    float screen_cy,
                                  float scale)
{
    if (s_active_renderer == nullptr || src_buf == nullptr)
        return 0;
    return s_active_renderer->SubmitLandviewZoom(src_buf, src_w, src_h, center_map_x, center_map_y,
                                                 screen_cx, screen_cy, scale) ? 1 : 0;
}

TbBool RendererScheduleScreenshot(const char* path, int fmt)
{
    return (s_active_renderer != nullptr) ? s_active_renderer->ScheduleScreenshot(path, fmt) : 0;
}

/******************************************************************************/
/* Full-screen tint overlay                                                   */
/******************************************************************************/

float g_screen_tint[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

void RendererSetScreenTint(float r, float g, float b, float a)
{
    g_screen_tint[0] = r;
    g_screen_tint[1] = g;
    g_screen_tint[2] = b;
    g_screen_tint[3] = a;
}

void RendererApplyPossessionPalette(long step, const unsigned char *main_palette)
{
    // GPU renderers use the screen tint overlay for possession/pain effects
    // (RendererSetScreenTint(), called by every caller of this function
    // alongside it); the software path has no tint consumer and must modify
    // the palette directly instead, exactly as it always has.
    if (s_active_renderer != nullptr && s_active_renderer->GetCapabilities().hasGPURenderPath)
        return;

    unsigned char palette[PALETTE_SIZE];
    for (int i = 0; i < PALETTE_COLORS; i++)
    {
        const unsigned char *src = &main_palette[3 * i];
        unsigned char       *dst = &palette[3 * i];
        unsigned long pix = ((step * (((long)src[0]) - 63)) / 120) + 63;
        if (pix > 63) pix = 63;
        dst[0] = (unsigned char)pix;
        pix = (step * ((long)src[1])) / 120;
        if (pix > 63) pix = 63;
        dst[1] = (unsigned char)pix;
        pix = (step * ((long)src[2])) / 120;
        if (pix > 63) pix = 63;
        dst[2] = (unsigned char)pix;
    }
    LbScreenWaitVbi();
    RendererPaletteSet(palette);
}

void RendererNotifyFmvPalette(const unsigned char *bgra_1024)
{
    // GPU backends: FMV's own palette travels with the present itself
    // (RendererPresentImageDesc::embedded_palette) -- no need to also mutate
    // the shared game-palette texture, which is exactly the latent risk
    // (concurrent rendering seeing a half-restored palette) this split
    // exists to remove.
    if (s_active_renderer != nullptr && s_active_renderer->GetCapabilities().hasGPURenderPath)
        return;
    if (bgra_1024 == nullptr)
        return;

    unsigned char rgb8[PALETTE_SIZE];
    for (int i = 0; i < PALETTE_COLORS; ++i)
    {
        rgb8[i * 3 + 0] = bgra_1024[i * 4 + 2]; // red
        rgb8[i * 3 + 1] = bgra_1024[i * 4 + 1]; // green
        rgb8[i * 3 + 2] = bgra_1024[i * 4 + 0]; // blue
    }
    RendererSetDisplayPalette(rgb8);
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

TbBool RendererTextDrawResized(int posx, int posy, int units_per_px, const char *text)
{
    ITextRenderer* tr = (s_active_renderer != nullptr) ? s_active_renderer->GetTextRenderer() : nullptr;
    if (tr == nullptr)
        return LbTextDrawResizedImmediate(posx, posy, units_per_px, text);
    return tr->DrawTextResized(posx, posy, units_per_px, text);
}

static IUIRenderer* active_ui_renderer(void)
{
    return (s_active_renderer != nullptr) ? s_active_renderer->GetUIRenderer() : nullptr;
}

/* The ambient draw state is what the caller set before the call, so it travels with
 * the submission and is reapplied if the draw is replayed later. */
static KfxDrawState ambient_draw_state(void)
{
    return draw_state_make(RendererGetDrawFlags(), RendererGetDrawColour());
}

void RendererDrawSlabBackground(int32_t x, int32_t y, int32_t width, int32_t height)
{
    IUIRenderer* ui = active_ui_renderer();
    if (ui == nullptr) { draw_slab64k_background_immediate(x, y, width, height); return; }
    ui->SubmitSlabBackground((int32_t)x, (int32_t)y, (int32_t)width, (int32_t)height);
}

TbResult RendererDrawBox(int32_t x, int32_t y, uint32_t width, uint32_t height, unsigned char colour)
{
    IUIRenderer* ui = active_ui_renderer();
    if (ui == nullptr) return LbDrawBoxImmediate(x, y, width, height, colour);
    ui->SubmitSolidBox(x, y, (int32_t)width, (int32_t)height, colour, ambient_draw_state());
    return Lb_SUCCESS;
}

TbResult RendererSpriteDraw(int32_t x, int32_t y, const struct TbSprite *spr)
{
    IUIRenderer* ui = active_ui_renderer();
    if (ui == nullptr) return LbSpriteDrawImmediate(x, y, spr);
    return ui->SubmitRawSprite(x, y, spr, ambient_draw_state());
}

TbResult RendererSpriteDrawOneColour(int32_t x, int32_t y, const struct TbSprite *spr, unsigned char colour)
{
    IUIRenderer* ui = active_ui_renderer();
    if (ui == nullptr) return LbSpriteDrawOneColourImmediate(x, y, spr, colour);
    return ui->SubmitRawSpriteOneColour(x, y, spr, colour, ambient_draw_state());
}

TbResult RendererSpriteDrawScaled(int32_t x, int32_t y, const struct TbSprite *spr, int32_t w, int32_t h)
{
    IUIRenderer* ui = active_ui_renderer();
    if (ui == nullptr) return LbSpriteDrawScaledImmediate(x, y, spr, w, h);
    return ui->SubmitRawSpriteScaled(x, y, spr, w, h, ambient_draw_state());
}

TbResult RendererSpriteDrawScaledOneColour(int32_t x, int32_t y, const struct TbSprite *spr, int32_t w, int32_t h, unsigned char colour)
{
    IUIRenderer* ui = active_ui_renderer();
    if (ui == nullptr) return LbSpriteDrawScaledOneColourImmediate(x, y, spr, w, h, colour);
    return ui->SubmitRawSpriteScaledOneColour(x, y, spr, w, h, colour, ambient_draw_state());
}

int RendererSpriteDrawScaledRemap(int32_t x, int32_t y, const struct TbSprite *spr, int32_t w, int32_t h, const unsigned char *cmap)
{
    IUIRenderer* ui = active_ui_renderer();
    if (ui == nullptr) return LbSpriteDrawScaledRemapImmediate(x, y, spr, w, h, cmap);
    return ui->SubmitRawSpriteScaledRemap(x, y, spr, w, h, cmap, ambient_draw_state());
}

// Beat 5: bracket world-positioned UI content (creature status, room flags,
// floating gold/damage text) so it submits into the WorldOverlay/
// WorldOverlayFlat IR layers instead of always landing in GameUI -- ported
// from develop's UIRenderer_BeginWorldOverlay()/EndWorldOverlay()/
// BeginWorldOverlayFlat()/EndWorldOverlayFlat() bridges. No-op (not an
// error) when no UI renderer is active, same as every other bridge here --
// software doesn't need this layering distinction, so its no-op default
// (IUIRenderer::SetWorldOverlay() etc.) is never even reached without a
// real backend to call it through.
void RendererSetWorldOverlay(float ndc_z)
{
    IUIRenderer* ui = active_ui_renderer();
    if (ui != nullptr) ui->SetWorldOverlay(ndc_z);
}

void RendererClearWorldOverlay(void)
{
    IUIRenderer* ui = active_ui_renderer();
    if (ui != nullptr) ui->ClearWorldOverlay();
}

void RendererSetWorldOverlayFlat(float ndc_z)
{
    IUIRenderer* ui = active_ui_renderer();
    if (ui != nullptr) ui->SetWorldOverlayFlat(ndc_z);
}

void RendererClearWorldOverlayFlat(void)
{
    IUIRenderer* ui = active_ui_renderer();
    if (ui != nullptr) ui->ClearWorldOverlayFlat();
}

void RendererUpdateSlabTexture(const unsigned char* data, int dim)
{
    IUIRenderer* ui = active_ui_renderer();
    if (ui != nullptr) ui->UpdateSlabTexture(data, dim);
}

unsigned char RendererGetDrawColour(void) { return s_draw_colour; }
void RendererSetDrawColour(unsigned char colour) { s_draw_colour = colour; }

unsigned short RendererGetDrawFlags(void) { return s_draw_flags; }
void RendererSetDrawFlags(unsigned short flags) { s_draw_flags = flags; }
void RendererAddDrawFlags(unsigned short flags) { s_draw_flags |= flags; }
void RendererClearDrawFlags(unsigned short flags) { s_draw_flags &= ~flags; }
void RendererToggleDrawFlags(unsigned short flags) { s_draw_flags ^= flags; }

// Display property accessors (declared in the header but never implemented --
// a partial de-globalisation left mid-way by the f2be58ea2b checkpoint commit,
// which added these declarations plus ~19 RendererPhysicalWidth() call sites
// without ever writing the .cpp bodies, causing a link error). develop has
// fully de-globalised these (own statics, lbDisplay fields removed entirely);
// this is the minimal-fix version -- thin passthroughs to the still-present
// lbDisplay fields, zero behaviour change, so the existing call sites just
// start working instead of pulling in develop's much larger field-removal
// across ~19 files.
TbScreenCoord RendererPhysicalWidth(void)  { return lbDisplay.PhysicalScreenWidth;  }
TbScreenCoord RendererPhysicalHeight(void) { return lbDisplay.PhysicalScreenHeight; }
TbScreenCoord RendererScreenWidth(void)    { return lbDisplay.GraphicsScreenWidth;  }
TbScreenCoord RendererScreenHeight(void)   { return lbDisplay.GraphicsScreenHeight; }

// Ported from develop's RendererManager.cpp (Beat 4). develop's own version
// also rebuilds the sprite atlas when palette_mode changes and propagates
// zoom_box_mode to a runtime selector -- both dropped here since neither has
// a consumer on this branch (no truecolour atlas, no picture-in-picture
// zoom-box), matching develop's own admission for the rest of this function
// ("TODO: push shade/filter uniforms to the active world-view renderer" --
// those are read directly from g_renderer_settings each frame by the GL
// draw code instead, same as develop itself still does).
void RendererApplySettings(const RendererSettings* s)
{
    if (!s) return;
    g_renderer_settings = *s;
}

const RendererSettings* RendererGetSettings(void)
{
    return &g_renderer_settings;
}

TbResult RendererPaletteGet(unsigned char *palette)
{
    return LbPaletteGet(palette);
}

static ICursorLayer* active_cursor_layer(void)
{
    return (s_active_renderer != nullptr) ? s_active_renderer->GetCursorLayer() : nullptr;
}

void CursorLayer_Draw(void)
{
    ICursorLayer* cursor = active_cursor_layer();
    if (cursor != nullptr)
        cursor->Draw();
}

void CursorLayer_Clear(void)
{
    ICursorLayer* cursor = active_cursor_layer();
    if (cursor != nullptr)
        cursor->Clear();
}

void CursorLayer_SubmitPointerSprite(const struct TbSprite* spr, int32_t x, int32_t y, int units_per_px)
{
    ICursorLayer* cursor = active_cursor_layer();
    if (cursor != nullptr)
        cursor->SubmitPointerSprite(spr, x, y, units_per_px);
}

static IWorldViewRenderer* active_world_renderer(void)
{
    return (s_active_renderer != nullptr) ? s_active_renderer->GetWorldViewRenderer() : nullptr;
}

// Immediate-fallback bodies: reproduce the pre-P5.7.0 behaviour exactly for
// callers with no world-view backend (GL, or before RendererInit()).
void WorldViewRenderer_BeginWorldPass(int w, int h, int vp_x, int vp_y)
{
    IWorldViewRenderer* world = active_world_renderer();
    if (world != nullptr) { world->BeginWorldPass(w, h, vp_x, vp_y); return; }
    (void)vp_x; (void)vp_y;
    setup_vecs(lbDisplay.GraphicsWindowPtr, NULL, lbDisplay.GraphicsScreenWidth,
               (unsigned int)w, (unsigned int)h);
}

void WorldViewRenderer_DrawIsometricView(void)
{
    IWorldViewRenderer* world = active_world_renderer();
    if (world != nullptr) { world->DrawIsometricView(); return; }
    display_drawlist();
}

void WorldViewRenderer_DrawFrontView(struct Camera* cam)
{
    IWorldViewRenderer* world = active_world_renderer();
    if (world != nullptr) { world->DrawFrontView(cam); return; }
    display_fast_drawlist(cam);
}

int RendererSubmitKeeperHandSprite(short x, short y, unsigned short kspr_base,
    short angle, unsigned char sprgroup, int32_t scale, TbDrawFlagsMask draw_flags)
{
    ICursorLayer* cursor = active_cursor_layer();
    if (cursor != nullptr)
        return cursor->SubmitKeeperHandSprite(x, y, kspr_base, angle, sprgroup, scale, draw_flags);
    return 0;
}

int RendererBeginWorldSpriteCapture(int32_t bucket_idx)
{
    IWorldViewRenderer* world = active_world_renderer();
    if (world != nullptr)
        return world->BeginWorldSpriteCapture(bucket_idx);
    return 0;
}

int RendererSubmitKeeperSprite(int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
    const unsigned char* data, int src_w, int src_h, int32_t content_h,
    unsigned int draw_flags, const unsigned char* remap, int32_t sprite_id)
{
    IWorldViewRenderer* world = active_world_renderer();
    if (world != nullptr)
        return world->SubmitKeeperSprite(dst_x, dst_y, dst_w, dst_h, data, src_w, src_h, content_h,
                                          draw_flags, remap, sprite_id);
    return 0;
}

/******************************************************************************/
/* Sprite-owner tracking for the depth-fail creature outline (Beat 4)         */
/******************************************************************************/

static int s_current_sprite_owner         = -1;
static int s_current_sprite_wants_outline =  0;

void RendererSetCurrentSpriteContext(int player_idx, int wants_outline)
{
    s_current_sprite_owner         = player_idx;
    s_current_sprite_wants_outline = wants_outline;
}

int RendererGetCurrentSpriteOwner(void)
{
    return s_current_sprite_owner;
}

int RendererGetCurrentSpriteWantsOutline(void)
{
    return s_current_sprite_wants_outline;
}

void RendererClearKeeperSpriteAtlas(void)
{
    IWorldViewRenderer* world = active_world_renderer();
    if (world != nullptr)
        world->ClearKeeperSpriteAtlas();
}

void RendererPreloadKeeperSpriteAtlas(void)
{
    IWorldViewRenderer* world = active_world_renderer();
    if (world != nullptr)
        world->PreloadKeeperSpriteAtlas();
}

void RendererSubmitMapFadeStep(int tick_step, float display_step, TbBool fading_in)
{
    if (s_active_renderer != nullptr)
        s_active_renderer->SubmitMapFadeStep(tick_step, display_step, fading_in != 0);
}

void RendererBeginParchmentCapture(void)
{
    if (s_active_renderer != nullptr)
        s_active_renderer->BeginParchmentCapture();
}

void RendererEndParchmentCapture(void)
{
    if (s_active_renderer != nullptr)
        s_active_renderer->EndParchmentCapture();
}

void RendererBeginSwipeOverlay(void)
{
    if (s_active_renderer != nullptr)
        s_active_renderer->BeginSwipeOverlay();
}

void RendererEndSwipeOverlay(void)
{
    if (s_active_renderer != nullptr)
        s_active_renderer->EndSwipeOverlay();
}

TbBool MapFadePass_SupportsNativeResolution(void)
{
    return (s_active_renderer != nullptr) && s_active_renderer->MapFadeSupportsNativeResolution();
}

void RendererSubmitPossessionLens(long viewport_x, long viewport_y, long viewport_w, long viewport_h)
{
    IWorldViewRenderer* world = active_world_renderer();
    if (world == nullptr)
        return;

    LensManager* lm = LensManager::GetInstance();
    IRWorldLensCmd cmd;
    if (lm != nullptr && lm->BuildActiveGPULensCmd(viewport_w, viewport_h, cmd))
    {
        cmd.viewport_x = viewport_x;
        cmd.viewport_y = viewport_y;
        cmd.viewport_w = viewport_w;
        cmd.viewport_h = viewport_h;
        world->SubmitPossessionLens(cmd);
    }
}
