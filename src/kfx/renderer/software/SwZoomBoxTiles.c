/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file SwZoomBoxTiles.c
 *     Software raster for the parchment zoom box's terrain tiles.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/software/SwZoomBoxTiles.h"
#include "kfx/renderer/software/SwDrawTarget.h"
#include "kfx/renderer/RendererManager.h" // RendererScreenWidth
#include "bflib_video.h"     // MyScreenWidth/Height, pixel_size
#include "bflib_vidraw.h"    // setup_vecs, vec_screen*
#include "engine_textures.h" // block_ptrs
#include "vidmode.h"         // pixmap.fade_tables
#include "post_inc.h"

/******************************************************************************/

static void scale_tmap2(long texture_block_index, long flags, long fade_level, long screen_x, long screen_y, long scaled_width, long scaled_height)
{
    if ((scaled_width == 0) || (scaled_height == 0)) {
        return;
    }
    long xstart;
    long ystart;
    long xend;
    long yend;
    char orient;
    switch (flags)
    {
    case 0:
        xstart = 0;
        ystart = 0;
        xend = 2097151 / scaled_width;
        yend = 2097151 / scaled_height;
        orient = 0;
        break;
    case 0x10:
        xstart = 2097151;
        ystart = 0;
        xend = -2097151 / scaled_width;
        yend = 2097151 / scaled_height;
        orient = 0;
        break;
    case 0x20:
        xstart = 0;
        ystart = 2097151;
        xend = 2097151 / scaled_width;
        yend = -2097151 / scaled_height;
        orient = 0;
        break;
    case 0x30:
        xstart = 2097151;
        ystart = 2097151;
        xend = -2097151 / scaled_width;
        yend = -2097151 / scaled_height;
        orient = 0;
        break;
    case 0x40:
        ystart = 0;
        xstart = 0;
        yend = 2097151 / scaled_height;
        xend = 2097151 / scaled_width;
        orient = 1;
        break;
    case 0x50:
        ystart = 0;
        xstart = 2097151;
        yend = 2097151 / scaled_height;
        xend = -2097151 / scaled_width;
        orient = 1;
        break;
    case 0x60:
        ystart = 2097151;
        xstart = 0;
        yend = -2097151 / scaled_height;
        xend = 2097151 / scaled_width;
        orient = 1;
        break;
    case 0x70:
        xstart = 2097151;
        ystart = 2097151;
        yend = -2097151 / scaled_height;
        xend = -2097151 / scaled_width;
        orient = 1;
        break;
    default:
          return;
    }
    long local_screen_x;
    long local_screen_y;
    local_screen_x = screen_x;
    if (local_screen_x < 0)
    {
        scaled_width += local_screen_x;
        if (scaled_width < 0) {
            return;
        }
        xstart -= xend * local_screen_x;
        local_screen_x = 0;
    }
    if (local_screen_x + scaled_width > vec_window_width)
    {
        scaled_width = vec_window_width - local_screen_x;
        if (scaled_width < 0) {
            return;
        }
    }
    local_screen_y = screen_y;
    if (local_screen_y < 0)
    {
        scaled_height += local_screen_y;
        if (scaled_height < 0) {
            return;
        }
        ystart -= local_screen_y * yend;
        local_screen_y = 0;
    }
    if (local_screen_y + scaled_height > vec_window_height)
    {
        scaled_height = vec_window_height - local_screen_y;
        if (scaled_height < 0) {
            return;
        }
    }
    int i;
    int32_t hlimits[480];
    int32_t wlimits[640];
    int32_t *xlim;
    int32_t *ylim;
    unsigned char *dbuf;
    unsigned char *block;
    if (!orient)
    {
        xlim = wlimits;
        for (i = scaled_width; i > 0; i--)
        {
            *xlim = xstart;
            xlim++;
            xstart += xend;
        }
        ylim = hlimits;
        for (i = scaled_height; i > 0; i--)
        {
            *ylim = ystart;
            ylim++;
            ystart += yend;
        }
        dbuf = &vec_screen[local_screen_x + local_screen_y * vec_screen_width];
        block = block_ptrs[texture_block_index];
        ylim = hlimits;
        long px;
        long py;
        int srcx;
        int srcy;
        unsigned char *d;
        if ( fade_level >= 0 )
        {
          for (py = scaled_height; py > 0; py--)
          {
              xlim = wlimits;
              d = dbuf;
              srcy = (((*ylim) & 0xFF0000u) >> 16);
              for (px = scaled_width; px > 0; px--)
              {
                srcx = (((*xlim) & 0xFF0000u) >> 16);
                xlim++;
                *d = pixmap.fade_tables[256 * fade_level + block[(srcy << 8) + srcx]];
                ++d;
              }
              dbuf += vec_screen_width;
              ylim++;
          }
        } else
        {
          for (py = scaled_height; py > 0; py--)
          {
            xlim = wlimits;
            d = dbuf;
            srcy = (((*ylim) & 0xFF0000u) >> 16);
            for (px = scaled_width; px > 0; px--)
            {
              srcx = (((*xlim) & 0xFF0000u) >> 16);
              xlim++;
              *d = block[(srcy << 8) + srcx];
              ++d;
            }
            dbuf += vec_screen_width;
            ylim++;
          }
        }
    } else
    {
        ylim = wlimits;
        for (i = scaled_height; i > 0; i--)
        {
          *ylim = ystart;
          ylim++;
          ystart += yend;
        }
        xlim = hlimits;
        for (i = scaled_width; i > 0; i--)
        {
          *xlim = xstart;
          xlim++;
          xstart += xend;
        }
        dbuf = &vec_screen[local_screen_x + local_screen_y * vec_screen_width];
        block = block_ptrs[texture_block_index];
        ylim = wlimits;
        long px;
        long py;
        int srcx;
        int srcy;
        unsigned char *d;
        if ( fade_level >= 0 )
        {
          for (py = scaled_height; py > 0; py--)
          {
              xlim = hlimits;
              d = dbuf;
              srcy = (((*ylim) & 0xFF0000u) >> 16);
              for (px = scaled_width; px > 0; px--)
              {
                srcx = (((*xlim) & 0xFF0000u) >> 16);
                xlim++;
                *d = pixmap.fade_tables[256 * fade_level + block[(srcx << 8) + srcy]];
                ++d;
              }
              dbuf += vec_screen_width;
              ylim++;
          }
        } else
        {
          for (py = scaled_height; py > 0; py--)
          {
            xlim = hlimits;
            d = dbuf;
            srcy = (((*ylim) & 0xFF0000u) >> 16);
            for (px = scaled_width; px > 0; px--)
            {
              srcx = (((*xlim) & 0xFF0000u) >> 16);
              xlim++;
              *d = block[(srcx << 8) + srcy];
              ++d;
            }
            dbuf += vec_screen_width;
            ylim++;
          }
        }
    }
}

void SwDrawZoomBoxTiles(const uint16_t* tile_block_ids, int32_t tiles_x, int32_t tiles_y,
                        int32_t dst_x, int32_t dst_y, int32_t tile_w, int32_t tile_h)
{
    setup_vecs(SwTargetWScreen(), 0, RendererScreenWidth(), MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    int32_t scr_y = dst_y;
    for (int32_t dy = 0; dy < tiles_y; dy++, scr_y += tile_h)
    {
        int32_t scr_x = dst_x;
        for (int32_t dx = 0; dx < tiles_x; dx++, scr_x += tile_w)
        {
            const uint16_t k = tile_block_ids[dy * tiles_x + dx];
            if (k == 0xFFFF)
                continue;
            scale_tmap2(k, 0, -1, scr_x / pixel_size, scr_y / pixel_size, tile_w / pixel_size, tile_h / pixel_size);
        }
    }
}
