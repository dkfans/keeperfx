#ifndef RENDERER_RENDERERMANAGER_H
#define RENDERER_RENDERERMANAGER_H

#include "bflib_basics.h"  // TbResult
#include "bflib_video.h"   // TbScreenMode, TbScreenCoord

// RendererType is a C++ enum; C translation units see it as an opaque int.
#ifdef __cplusplus
#  include "kfx/renderer/IRenderer.h"
#else
typedef int RendererType;
#  define RENDERER_INVALID  (-1)
#  define RENDERER_AUTO     0
#  define RENDERER_SOFTWARE 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Lifecycle: initialise the requested backend (nonzero on success) / shut it down.
int          RendererInit(RendererType type);
void         RendererShutdown(void);
RendererType RendererGetActiveType(void);

// The currently-active 6-bit VGA palette (768 bytes) that indexed drawing samples.
const unsigned char* RendererGetActivePalette(void);

// Set / read back the active game palette (the seam entry points engine code uses).
TbResult RendererPaletteSet(unsigned char *palette);
TbResult RendererPaletteGet(unsigned char *palette);

// Apply an 8-bit RGB palette (256*3 bytes) directly to the display
void RendererSetDisplayPalette(const unsigned char *rgb8);

// Clear the whole display to a palette index.
void RendererClearScreen(unsigned char colour);

// Present the drawn frame to the window (blit draw surface + flip).
void RendererPresentFrame(void);

// Lock / unlock the CPU framebuffer, pointing lbDisplay.WScreen at the backend pixels.
TbResult RendererLockFramebuffer(void);
TbResult RendererUnlockFramebuffer(void);

// Save the current frame to a file via the active backend (fmt: 1=PNG, 2=BMP).
TbBool RendererScheduleScreenshot(const char* path, int fmt);

// Screen lifecycle (window + draw surface).
TbResult RendererSetupScreen(TbScreenMode mode, TbScreenCoord width, TbScreenCoord height,
    unsigned char *palette, short buffers_count, TbBool wscreen_vid);
TbResult RendererResetScreen(TbBool exiting_application);
TbResult RendererScreenInitialize(void);
TbResult RendererSetDoubleBuffering(TbBool state);

// Current draw colour — ambient draw-call state, held off lbDisplay.  will be removing in the future, just for now it keeps the pr small
// Text. LbTextDrawResized routes here so the active backend can record the
// draw for this frame or draw it now.
TbBool RendererTextDrawResized(int posx, int posy, int units_per_px, const char *text);

// Sprites. The Lb* entry points route here so the active backend can record the
// draw for this frame or draw it now.
struct TbSprite;
TbResult RendererDrawBox(int32_t x, int32_t y, uint32_t width, uint32_t height, unsigned char colour);
void RendererDrawSlabBackground(int32_t x, int32_t y, int32_t width, int32_t height);
TbResult RendererSpriteDraw(int32_t x, int32_t y, const struct TbSprite *spr);
TbResult RendererSpriteDrawOneColour(int32_t x, int32_t y, const struct TbSprite *spr, unsigned char colour);
TbResult RendererSpriteDrawScaled(int32_t x, int32_t y, const struct TbSprite *spr, int32_t w, int32_t h);
TbResult RendererSpriteDrawScaledOneColour(int32_t x, int32_t y, const struct TbSprite *spr, int32_t w, int32_t h, unsigned char colour);
int      RendererSpriteDrawScaledRemap(int32_t x, int32_t y, const struct TbSprite *spr, int32_t w, int32_t h, const unsigned char *cmap);

unsigned char RendererGetDrawColour(void);
void RendererSetDrawColour(unsigned char colour);

// Current draw flags (TbDrawFlags bitmask) — ambient draw-call state, held off lbDisplay.
unsigned short RendererGetDrawFlags(void);
void RendererSetDrawFlags(unsigned short flags);   // = flags
void RendererAddDrawFlags(unsigned short flags);    // |= flags
void RendererClearDrawFlags(unsigned short flags);  // &= ~flags
void RendererToggleDrawFlags(unsigned short flags); // ^= flags

#ifdef __cplusplus
}
#endif

#endif // RENDERER_RENDERERMANAGER_H
