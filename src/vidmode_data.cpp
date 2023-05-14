/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file vidmode_data.cpp
 *     Video mode switching/setting function.
 * @par Purpose:
 *     Structures which store video-related parameters.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 12 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "vidmode.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fmvids.h"
#include "bflib_video.h"
#include "bflib_mouse.h"
#include "bflib_sprite.h"
#include "bflib_dernc.h"
#include "bflib_sprfnt.h"
#include "bflib_filelst.h"

#include "front_simple.h"
#include "front_landview.h"
#include "frontend.h"
#include "game_heap.h"
#include "gui_draw.h"
#include "gui_parchment.h"
#include "engine_redraw.h"
#include "engine_textures.h"
#include "config.h"
#include "lens_api.h"
#include "config_settings.h"
#include "game_legacy.h"
#include "creature_graphics.h"
#include "keeperfx.hpp"
#include "sprites.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern struct SpriteSheet *pointer_sprites;

struct TbSetupSprite setup_sprites[] = {
  {NULL,                  NULL,                   NULL},
};

#if (BFDEBUG_LEVEL > 0)
// Declarations for font testing screen (debug version only)

struct TbLoadFiles testfont_load_files[] = {
  {"data/frontend.pal",  (unsigned char **)&testfont_palette[0],NULL,                                           0, 0, 0},
  {"data/palette.dat",   (unsigned char **)&testfont_palette[1],NULL,                                           0, 0, 0},
  {"",                    NULL,                                 NULL,                                           0, 0, 0},
};
#endif

struct TbLoadFiles gui_load_files_320[] = {
  {"data/slab0-0.dat",   (unsigned char **)&gui_slab,           NULL,                                           0, 0, 0},
  {"",                    NULL,                                 NULL,                                           0, 0, 0},
};

struct TbLoadFiles gui_load_files_640[] = {
  {"data/slab0-1.dat",   (unsigned char **)&gui_slab,           NULL,                                           0, 0, 0},
  {"*B_SCREEN",          (unsigned char **)&hires_parchment,    NULL,                                     640*480, 0, 0},
  {"",                   NULL,                                  NULL,                                           0, 0, 0},
};

struct TbLoadFiles front_load_files_minimal_320[] = {
  {"",                   NULL,                                  NULL,                                           0, 0, 0},
};

#ifdef SPRITE_FORMAT_V2
struct TbLoadFiles front_load_files_minimal_640[] = {
  {"*FE_BACKUP_PAL",       (unsigned char **)&frontend_backup_palette,NULL,                            PALETTE_SIZE, 0, 0},
  {"",                     NULL,                                  NULL,                                           0, 0, 0},
};
#else
struct TbLoadFiles front_load_files_minimal_640[] = {
  {"*FE_BACKUP_PAL",     (unsigned char **)&frontend_backup_palette,NULL,                            PALETTE_SIZE, 0, 0},
  {"",                   NULL,                                  NULL,                                           0, 0, 0},
};
#endif

struct TbLoadFiles legal_load_files[] = {
    {"*PALETTE",         &engine_palette,                        NULL,                               PALETTE_SIZE, 0, 0},
    {"*SCRATCH",         &scratch,                               NULL,                                    0x10000, 1, 0},
    {"",                 NULL,                                   NULL,                                          0, 0, 0},
};

struct TbLoadFiles game_load_files[] = {
    {"*SCRATCH",         &scratch,                               NULL,                                    0x10000, 0, 0},
    {"*TEXTURE_PAGE",    (unsigned char **)&block_mem, NULL, max(sizeof(block_mem), size_t(960*720)), 0, 0},// Store whole texture image or land view image
#ifdef SPRITE_FORMAT_V2
    {"data/thingspr-32.tab",(unsigned char**)&creature_table,    NULL,                                          0, 0, 0},
#else
    {"data/creature.tab",(unsigned char**)&creature_table,       NULL,                                          0, 0, 0},
#endif
    {"data/palette.dat", &engine_palette,                        NULL,                                          0, 0, 0},
    {"data/bluepal.dat", &blue_palette,                          NULL,                                          0, 0, 0},
    {"data/redpall.dat", &red_palette,                           NULL,                                          0, 0, 0},
    {"data/lightng.pal", &lightning_palette,                     NULL,                                          0, 0, 0},
    {"data/dogpal.pal",  &dog_palette,                           NULL,                                          0, 0, 0},
    {"data/vampal.pal",  &vampire_palette,                       NULL,                                          0, 0, 0},
    {"",                 NULL,                                   NULL,                                          0, 0, 0},
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
