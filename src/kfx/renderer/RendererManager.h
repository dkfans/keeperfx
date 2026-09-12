#ifndef RENDERER_RENDERERMANAGER_H
#define RENDERER_RENDERERMANAGER_H

#include "bflib_basics.h"  // TbResult
#include "bflib_video.h"   // TbScreenMode, TbScreenCoord
#include "kfx/renderer/DrawState.h" // TbDrawFlagsMask

// RendererType/OverlayCaptureKind are C++ enums; C translation units see
// them as opaque ints.
#ifdef __cplusplus
#  include "kfx/renderer/IRenderer.h"
#else
typedef int RendererType;
#  define RENDERER_INVALID  (-1)
#  define RENDERER_AUTO     0
#  define RENDERER_SOFTWARE 1
#  define RENDERER_OPENGL   2

typedef int OverlayCaptureKind;
#  define OVERLAY_CAPTURE_PARCHMENT 0
#  define OVERLAY_CAPTURE_SWIPE     1
#endif

// UIRenderer_Submit*/Begin*/End* free-function wrappers, implemented in
// their own translation unit (RendererBridge_UI.cpp).
#include "kfx/renderer/RendererBridge_UI.h"

#ifdef __cplusplus
extern "C" {
#endif

// Lifecycle: initialise the requested backend (nonzero on success) / shut it down.
int          RendererInit(RendererType type);
void         RendererShutdown(void);
RendererType RendererGetActiveType(void);

RendererType RendererResolveType(RendererType requested);

// SDL3 window-creation flags (KfxWindowFlags, bflib_video.h) required by a
// given backend -- e.g. KFX_WF_OPENGL for RENDERER_OPENGL. Queried by
// LbScreenSetup() before the one-and-only SDL_CreateWindow() call, so the
// window is born with the correct flags instead of being destroyed and
// recreated later to add them.
unsigned int RendererGetRequiredWindowFlags(RendererType type);

// The currently-active 6-bit VGA palette (768 bytes) that indexed drawing samples.
const unsigned char* RendererGetActivePalette(void);

// Set / read back the active game palette (the seam entry points engine code uses).
TbResult RendererPaletteSet(unsigned char *palette);
TbResult RendererPaletteGet(unsigned char *palette);

// Apply an 8-bit RGB palette (256*3 bytes) directly to the display
void RendererSetDisplayPalette(const unsigned char *rgb8);

// Forward the raw 6-bit VGA palette to the active world-view renderer's
// SetPaletteSource() (keeper-sprite CLUT / outline colour resolution).
void RendererSetPaletteForRenderers(const unsigned char *pal6);

// Clear the whole display to a palette index.
void RendererClearScreen(unsigned char colour);

// Present the drawn frame to the window (blit draw surface + flip).
void RendererPresentFrame(void);

TbBool RendererBeginFrame(void);
void   RendererEndFrame(void);

// Palette source for RendererPresentImageDesc::palette.
#define PRESENT_PALETTE_GAME     0  /* live game palette (default -- existing zero-initialised callers) */
#define PRESENT_PALETTE_EMBEDDED 1  /* per-present palette carried in embedded_palette (FMV, 256x4 BGRA) */

// Compositing behaviour for RendererPresentImageDesc::kind.
#define PRESENT_KIND_OPAQUE      0  /* whole-screen present: fills the dest rect, letterboxing
                                        (clearing) everything outside it too (default) */
#define PRESENT_KIND_TRANSPARENT 1  /* draws over whatever's already there; index 0 = see-through
                                        unless coverage below is set */
#define PRESENT_KIND_COMPOSITE   2  /* draws over whatever's already there, like TRANSPARENT, but
                                        index 0 is a literal opaque colour, not see-through -- for a
                                        sub-rect present (e.g. the parchment overhead map) that owns
                                        every pixel in its own rect and none outside it */

// Source pixel format for RendererPresentImageDesc::format. Only INDEXED8
// exists today (every present source -- FMV frames, splash bitmaps, the
// parchment background -- is 8-bit indexed); true-colour formats are future
// work, not scaffolded here.
#define PRESENT_FORMAT_INDEXED8  0

struct RendererPresentImageDesc {
    int dst_x, dst_y, dst_w, dst_h;
    const unsigned char* src;
    int src_pitch;
    int src_w, src_h;
    int format;                             /* PRESENT_FORMAT_* */
    int palette;                            /* PRESENT_PALETTE_* */
    const unsigned char* embedded_palette;  /* 256x4 BGRA, PRESENT_PALETTE_EMBEDDED only, else NULL */
    int kind;                               /* PRESENT_KIND_* */
    /* Optional, PRESENT_KIND_TRANSPARENT only: per-pixel opacity, src_w*src_h
       tightly packed (255 = opaque, 0 = transparent), same dimensions as
       src. NULL (default) keeps the index-0-key behaviour. Set this when the
       source legitimately paints with palette index 0 as an opaque colour
       (e.g. the landview window frame). */
    const unsigned char* coverage;
};
TbBool RendererPresentImage(const struct RendererPresentImageDesc* desc);

/** Submit the zoom box's terrain as texture-block-indexed tiles. Returns
 *  true when the GPU path accepted the submission (caller skips its own
 *  CPU tile rasteriser); false when no GPU backend is active (software) --
 *  caller runs its existing CPU tile loop instead.
 *  @param tile_block_ids  tiles_x*tiles_y, row-major; 0xFFFF = unrevealed
 *                         (caller draws those separately, e.g. a solid box).
 *  @param dst_x/dst_y     Screen top-left of the tile grid (pixels).
 *  @param tile_w/tile_h   On-screen size of each tile (pixels). */
TbBool RendererSubmitZoomBoxTiles(const unsigned short* tile_block_ids, int tiles_x, int tiles_y,
                                  int dst_x, int dst_y, int tile_w, int tile_h);

/** Route an FMV frame's embedded palette through the renderer instead of
 *  calling RendererSetDisplayPalette() directly */
void RendererNotifyFmvPalette(const unsigned char *bgra_1024);

/** Submit the landview zoom-in/out transition frame through the GPU path.
 *  @param src_buf     map_screen -- 8-bit indexed pixels (src_w x src_h),
 *                     cached by pointer identity: only re-uploaded when this
 *                     differs from the last call's pointer.
 *  @param center_map_x/y  Zoom centre, in source texel coordinates.
 *  @param screen_cx/cy    Zoom centre, in screen pixel coordinates (y-down).
 *  @param scale           Source texels per screen pixel (src_delta/256.0 in
 *                         frontzoom_to_point()'s own terms).
 *  @return true when the GPU path accepted the frame (caller skips its own
 *          CPU zoom loop); false when no GPU backend is active (software
 *          renderer) -- caller runs its existing CPU path instead. */
TbBool RendererSubmitLandviewZoom(const unsigned char *src_buf, int src_w, int src_h,
                                  float center_map_x, float center_map_y,
                                  float screen_cx,    float screen_cy,
                                  float scale);

// Save the current frame to a file via the active backend (fmt: 1=PNG, 2=BMP).
TbBool RendererScheduleScreenshot(const char* path, int fmt);

/** True when the active backend composites the minimap over the panel
 *  artwork itself (draw-order layering) -- callers that build minimap pixel
 *  data must not bake a background colour into it themselves in that case.
 *  See IRenderer::BackendCapabilities::compositesMinimapBackground. */
TbBool RendererCompositesMinimapBackground(void);

// Full-screen tint overlay (pain/possession vignette, death/zoom-to-heart
// white flash). Plain ambient state, backend-agnostic -- GL blends a
// fullscreen quad from it each frame (FGDrawScreenTint()); software has no
// consumer (see RendererApplyPossessionPalette() below for its equivalent).
extern float g_screen_tint[4];
void RendererSetScreenTint(float r, float g, float b, float a);

/** Tell the GPU renderer to preserve the last real frame's content across
 *  PresentFrame() (world/UI/image-present buffers not flipped, only the
 *  palette/tint refreshed). Call with 1 before entering a blocking palette-
 *  fade loop, 0 after -- without it, a fade loop's repeated PresentFrame()
 *  calls advance to buffers nothing was freshly submitted into. */
void RendererPreserveFadeCache(int active);
int  RendererIsFadeCachePreserved(void);

/** Force the next PresentFrame() to perform a real flip even if
 *  RendererIsFadeCachePreserved() is currently true. One-shot: cleared as
 *  soon as it's consumed. Call whenever a player's view_type changes in a
 *  way that affects what the UI submits (set_player_mode(), player_data.c)
 *  so the new content commits to the read-side buffer before any subsequent
 *  fade-preserve window replays stale content over it. */
void RendererForceUIFlipNextFrame(void);
int  RendererConsumeForceUIFlip(void);

void RendererApplyPossessionPalette(long step, const unsigned char *main_palette);

// Screen lifecycle (window + draw surface).
TbResult RendererSetupScreen(TbScreenMode mode, TbScreenCoord width, TbScreenCoord height,
    unsigned char *palette, short buffers_count, TbBool wscreen_vid);
TbResult RendererResetScreen(TbBool exiting_application);
TbResult RendererScreenInitialize(void);
TbResult RendererSetDoubleBuffering(TbBool state);

/******************************************************************************/
/* Display property accessors                                                 */
/******************************************************************************/

/** Visible display width in pixels (window or fullscreen). */
TbScreenCoord RendererPhysicalWidth(void);

/** Visible display height in pixels (window or fullscreen). */
TbScreenCoord RendererPhysicalHeight(void);

/** Graphics buffer scanline width (pitch) in pixels.
 *  Use for pixel address arithmetic (ptr + y * stride + x). */
TbScreenCoord RendererScreenWidth(void);

/** Graphics buffer height in pixels. */
TbScreenCoord RendererScreenHeight(void);

TbBool RendererTextDrawResized(int posx, int posy, int units_per_px, const char *text);

struct TbSprite;
struct Camera;

unsigned char RendererGetDrawColour(void);
void RendererSetDrawColour(unsigned char colour);


void CursorLayer_Draw(void);
void CursorLayer_Clear(void);
void CursorLayer_SubmitPointerSprite(const struct TbSprite* spr, int32_t x, int32_t y, int units_per_px);
// Returns 1 if a cursor layer handled the sprite (caller must not also draw
// it via process_keeper_sprite()), 0 to fall back.
int RendererSubmitKeeperHandSprite(short x, short y, unsigned short kspr_base,
    short angle, unsigned char sprgroup, int32_t scale, TbDrawFlagsMask draw_flags);

void WorldViewRenderer_BeginWorldPass(int w, int h, int vp_x, int vp_y);
void WorldViewRenderer_DrawIsometricView(void);
void WorldViewRenderer_DrawFrontView(struct Camera* cam);


int RendererBeginWorldSpriteCapture(int32_t bucket_idx);
// content_h = visible rows out of src_h for this draw (water/lava
// clipping) -- pass == src_h for "no clipping".
int RendererSubmitKeeperSprite(int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
    const unsigned char* data, int src_w, int src_h, int32_t content_h,
    unsigned int draw_flags, const unsigned char* remap, int32_t sprite_id);

void RendererSetCurrentSpriteContext(int player_idx, int wants_outline);
int  RendererGetCurrentSpriteOwner(void);
int  RendererGetCurrentSpriteWantsOutline(void);

void RendererClearKeeperSpriteAtlas(void);
void RendererPreloadKeeperSpriteAtlas(void);

/** Re-upload the tile atlas's animated rows (lava, water, dig-tag
 *  cross-hatch). Call once per game tick, right after
 *  update_animating_texture_maps(). No-op when no renderer is active. */
void RendererUpdateAnimatedTiles(void);

void RendererSetWorldOverlay(float ndc_z);
void RendererClearWorldOverlay(void);
void RendererSetWorldOverlayFlat(float ndc_z);
void RendererClearWorldOverlayFlat(void);
void RendererSetGameViewport(int x, int y, int w, int h);

void RendererUpdateSlabTexture(const unsigned char* data, int dim);

void RendererSubmitPossessionLens(long viewport_x, long viewport_y, long viewport_w, long viewport_h);

void RendererSubmitMapFadeStep(int tick_step, float display_step, TbBool fading_in);
TbBool MapFadePass_SupportsNativeResolution(void);

// See OverlayCaptureKind's own comment (IRenderer.h) for each kind's caller.
void RendererBeginOverlayCapture(OverlayCaptureKind kind);
void RendererEndOverlayCapture(OverlayCaptureKind kind);

/******************************************************************************/
/* Zoom-box ambient state                                                     */
/******************************************************************************/

/** Screen-space rect of the active zoom box for the current frame. Set by
 *  draw_zoom_box() before terrain/things are submitted so draw_overhead_*
 *  can skip markers that fall inside the box. Half-open: [x0,x1) x [y0,y1). */
void RendererSetZoomBoxScreenRect(int x0, int y0, int x1, int y1);
void RendererClearZoomBoxScreenRect(void);
int  RendererPointInZoomBoxScreenRect(int x, int y);

// Current draw flags (TbDrawFlags bitmask) — ambient draw-call state, held off lbDisplay.
unsigned short RendererGetDrawFlags(void);
void RendererSetDrawFlags(unsigned short flags);   // = flags
void RendererAddDrawFlags(unsigned short flags);    // |= flags
void RendererClearDrawFlags(unsigned short flags);  // &= ~flags
void RendererToggleDrawFlags(unsigned short flags); // ^= flags

struct RendererSettings;
void RendererApplySettings(const struct RendererSettings* s);
const struct RendererSettings* RendererGetSettings(void);

#ifdef __cplusplus
}
#endif

#endif // RENDERER_RENDERERMANAGER_H
