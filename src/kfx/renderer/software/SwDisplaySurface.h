#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SDL_Surface;

/** Create the persistent 8-bit indexed draw surface the software backend
 *  presents from (with its palette), destroying any previous one first.
 *  NULL on failure. */
struct SDL_Surface *SwDisplaySurfaceCreate(int32_t width, int32_t height);

/** Destroy the draw surface, if one exists. */
void SwDisplaySurfaceDestroy(void);

/** The current draw surface, or NULL if none exists yet. */
struct SDL_Surface *SwDisplaySurfaceGet(void);

#ifdef __cplusplus
}
#endif
