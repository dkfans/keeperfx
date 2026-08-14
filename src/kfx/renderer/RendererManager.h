#ifndef RENDERER_RENDERERMANAGER_H
#define RENDERER_RENDERERMANAGER_H

#include "bflib_basics.h"  // TbResult

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

#ifdef __cplusplus
}
#endif

#endif // RENDERER_RENDERERMANAGER_H
