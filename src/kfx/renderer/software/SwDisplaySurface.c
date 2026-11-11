/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file SwDisplaySurface.c
 *     Owns the persistent SDL surface the software backend presents from.
 *     Replaces the old raw `extern SDL_Surface* lbDrawSurface` — everything
 *     outside this file reaches it through these accessors instead.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/software/SwDisplaySurface.h"

#include "bflib_basics.h"
#include <SDL3/SDL.h>
#include "post_inc.h"

/******************************************************************************/

static SDL_Surface *s_draw_surface = NULL;

SDL_Surface *SwDisplaySurfaceCreate(int32_t width, int32_t height)
{
    SwDisplaySurfaceDestroy();
    s_draw_surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8);
    if (s_draw_surface == NULL) {
        return NULL;
    }
    if (!SDL_CreateSurfacePalette(s_draw_surface)) {
        SDL_DestroySurface(s_draw_surface);
        s_draw_surface = NULL;
        return NULL;
    }
    return s_draw_surface;
}

void SwDisplaySurfaceDestroy(void)
{
    if (s_draw_surface != NULL) {
        SDL_DestroySurface(s_draw_surface);
        s_draw_surface = NULL;
    }
}

SDL_Surface *SwDisplaySurfaceGet(void)
{
    return s_draw_surface;
}

/******************************************************************************/
