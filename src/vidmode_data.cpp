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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern struct TbSprite *pointer_sprites;
extern struct TbSprite *end_pointer_sprites;
extern unsigned char * pointer_data;

struct TbSetupSprite setup_sprites_minimal[] = {
  {&frontend_font[0],     &frontend_end_font[0],  &frontend_font_data[0]},
  {&frontend_font[1],     &frontend_end_font[1],  &frontend_font_data[1]},
  {&frontend_font[2],     &frontend_end_font[2],  &frontend_font_data[2]},
  {&frontend_font[3],     &frontend_end_font[3],  &frontend_font_data[3]},
  {NULL,                  NULL,                   NULL},
};

static TbSprite *gui_panel_sprites_ptr = &gui_panel_sprites[0];

struct TbSetupSprite setup_sprites[] = {
  {&pointer_sprites,      &end_pointer_sprites,   &pointer_data}, // 144 Sprites
  {&font_sprites,         &end_font_sprites,      &font_data},
  {&edit_icon_sprites,    &end_edit_icon_sprites, &edit_icon_data},
  {&winfont,              &end_winfonts,          &winfont_data},
  {&button_sprite,        &end_button_sprites,    &button_sprite_data}, // 215 Sprites
  {&port_sprite,          &end_port_sprites,      &port_sprite_data}, // 0 Sprites
  {&gui_panel_sprites_ptr,    &end_gui_panel_sprites, &gui_panel_sprite_data}, // 517 Sprites
  {NULL,                  NULL,                   NULL},
};

#if (BFDEBUG_LEVEL > 0)
// Declarations for font testing screen (debug version only)
struct TbSetupSprite setup_testfont[] = {
  {&testfont[0],          &testfont_end[0],       &testfont_data[0]},
  {&testfont[1],          &testfont_end[1],       &testfont_data[1]},
  {&testfont[2],          &testfont_end[2],       &testfont_data[2]},
  {&testfont[3],          &testfont_end[3],       &testfont_data[3]},
  {&testfont[4],          &testfont_end[4],       &testfont_data[4]},
  {&testfont[5],          &testfont_end[5],       &testfont_data[5]},
  {&testfont[6],          &testfont_end[6],       &testfont_data[6]},
  {&testfont[7],          &testfont_end[7],       &testfont_data[7]},
  {&testfont[8],          &testfont_end[8],       &testfont_data[8]},
  {&testfont[9],          &testfont_end[9],       &testfont_data[9]},
  {&testfont[10],         &testfont_end[10],      &testfont_data[10]},
  {NULL,                  NULL,                   NULL},
};

struct TbLoadFiles testfont_load_files[] = {
  {"ldata/frontft1.dat", (unsigned char **)&testfont_data[0],   NULL,                                           0, 0, 0},
  {"ldata/frontft1.tab", (unsigned char **)&testfont[0],        (unsigned char **)&testfont_end[0],             0, 0, 0},
  {"ldata/frontft2.dat", (unsigned char **)&testfont_data[1],   NULL,                                           0, 0, 0},
  {"ldata/frontft2.tab", (unsigned char **)&testfont[1],        (unsigned char **)&testfont_end[1],             0, 0, 0},
  {"ldata/frontft3.dat", (unsigned char **)&testfont_data[2],   NULL,                                           0, 0, 0},
  {"ldata/frontft3.tab", (unsigned char **)&testfont[2],        (unsigned char **)&testfont_end[2],             0, 0, 0},
  {"ldata/frontft4.dat", (unsigned char **)&testfont_data[3],   NULL,                                           0, 0, 0},
  {"ldata/frontft4.tab", (unsigned char **)&testfont[3],        (unsigned char **)&testfont_end[3],             0, 0, 0},
  {"data/font0-0.dat",   (unsigned char **)&testfont_data[4],   NULL,                                           0, 0, 0},
  {"data/font0-0.tab",   (unsigned char **)&testfont[4],        (unsigned char **)&testfont_end[4],             0, 0, 0},
  {"data/font0-1.dat",   (unsigned char **)&testfont_data[5],   NULL,                                           0, 0, 0},
  {"data/font0-1.tab",   (unsigned char **)&testfont[5],        (unsigned char **)&testfont_end[5],             0, 0, 0},
  {"data/font2-32.dat",  (unsigned char **)&testfont_data[6],   NULL,                                           0, 0, 0},
  {"data/font2-32.tab",  (unsigned char **)&testfont[6],        (unsigned char **)&testfont_end[6],             0, 0, 0},
  {"data/font2-64.dat",  (unsigned char **)&testfont_data[7],   NULL,                                           0, 0, 0},
  {"data/font2-64.tab",  (unsigned char **)&testfont[7],        (unsigned char **)&testfont_end[7],             0, 0, 0},
  {"data/font1-64.dat",  (unsigned char **)&testfont_data[8],   NULL,                                           0, 0, 0},
  {"data/font1-64.tab",  (unsigned char **)&testfont[8],        (unsigned char **)&testfont_end[8],             0, 0, 0},
  {"data/font1-32.dat",  (unsigned char **)&testfont_data[9],   NULL,                                           0, 0, 0},
  {"data/font1-32.tab",  (unsigned char **)&testfont[9],        (unsigned char **)&testfont_end[9],             0, 0, 0},
  {"ldata/netfont.dat",  (unsigned char **)&testfont_data[10],  NULL,                                           0, 0, 0},
  {"ldata/netfont.tab",  (unsigned char **)&testfont[10],       (unsigned char **)&testfont_end[10],            0, 0, 0},
  {"data/frontend.pal",  (unsigned char **)&testfont_palette[0],NULL,                                           0, 0, 0},
  {"data/palette.dat",   (unsigned char **)&testfont_palette[1],NULL,                                           0, 0, 0},
  {"",                    NULL,                                 NULL,                                           0, 0, 0},
};
#endif

struct TbLoadFiles gui_load_files_320[] = {
  {"data/gui1-32.dat",   (unsigned char **)&button_sprite_data, (unsigned char **)&end_button_sprite_data,      0, 0, 0},
  {"data/gui1-32.tab",   (unsigned char **)&button_sprite,      (unsigned char **)&end_button_sprites,          0, 0, 0},
  {"data/font2-32.dat",  (unsigned char **)&winfont_data,       (unsigned char **)&end_winfont_data,            0, 0, 0},
  {"data/font2-32.tab",  (unsigned char **)&winfont,            (unsigned char **)&end_winfonts,                0, 0, 0},
  {"data/font1-32.dat",  (unsigned char **)&font_data,          NULL,                                           0, 0, 0},
  {"data/font1-32.tab",  (unsigned char **)&font_sprites,       (unsigned char **)&end_font_sprites,            0, 0, 0},
  {"data/slab0-0.dat",   (unsigned char **)&gui_slab,           NULL,                                           0, 0, 0},
  {"data/gui2-32.dat",   (unsigned char **)&gui_panel_sprite_data,(unsigned char **)&end_gui_panel_sprite_data, 0, 0, 0},
  {"!data/gui2-32.tab",  (unsigned char **)&gui_panel_sprites_ptr,  (unsigned char **)&end_gui_panel_sprites,   0, 0, 0},
  {"",                    NULL,                                 NULL,                                           0, 0, 0},
};

struct TbLoadFiles gui_load_files_640[] = {
  {"data/gui1-64.dat",   (unsigned char **)&button_sprite_data, (unsigned char **)&end_button_sprite_data,      0, 0, 0},
  {"data/gui1-64.tab",   (unsigned char **)&button_sprite,      (unsigned char **)&end_button_sprites,          0, 0, 0},
  {"data/font2-64.dat",  (unsigned char **)&winfont_data,       (unsigned char **)&end_winfont_data,            0, 0, 0},
  {"data/font2-64.tab",  (unsigned char **)&winfont,            (unsigned char **)&end_winfonts,                0, 0, 0},
  {"data/font1-64.dat",  (unsigned char **)&font_data,          NULL,                                           0, 0, 0},
  {"data/font1-64.tab",  (unsigned char **)&font_sprites,       (unsigned char **)&end_font_sprites,            0, 0, 0},
  {"data/slab0-1.dat",   (unsigned char **)&gui_slab,           NULL,                                           0, 0, 0},
  {"data/gui2-64.dat",   (unsigned char **)&gui_panel_sprite_data,(unsigned char **)&end_gui_panel_sprite_data, 0, 0, 0},
  {"!data/gui2-64.tab",  (unsigned char **)&gui_panel_sprites_ptr,  (unsigned char **)&end_gui_panel_sprites,       0, 0, 0},
  {"*B_SCREEN",          (unsigned char **)&hires_parchment,    NULL,                                     640*480, 0, 0},
  {"",                   NULL,                                  NULL,                                           0, 0, 0},
};

struct TbLoadFiles front_load_files_minimal_320[] = {
  {"",                   NULL,                                  NULL,                                           0, 0, 0},
};

#ifdef SPRITE_FORMAT_V2
struct TbLoadFiles front_load_files_minimal_640[] = {
  {"ldata/frontft1-64.dat",(unsigned char **)&frontend_font_data[0],(unsigned char **)&frontend_end_font_data[0], 0, 0, 0},
  {"ldata/frontft1-64.tab",(unsigned char **)&frontend_font[0],     (unsigned char **)&frontend_end_font[0],      0, 0, 0},
  {"ldata/frontft2-64.dat",(unsigned char **)&frontend_font_data[1],(unsigned char **)&frontend_end_font_data[1], 0, 0, 0},
  {"ldata/frontft2-64.tab",(unsigned char **)&frontend_font[1],     (unsigned char **)&frontend_end_font[1],      0, 0, 0},
  {"ldata/frontft3-64.dat",(unsigned char **)&frontend_font_data[2],(unsigned char **)&frontend_end_font_data[2], 0, 0, 0},
  {"ldata/frontft3-64.tab",(unsigned char **)&frontend_font[2],     (unsigned char **)&frontend_end_font[2],      0, 0, 0},
  {"ldata/frontft4-64.dat",(unsigned char **)&frontend_font_data[3],(unsigned char **)&frontend_end_font_data[3], 0, 0, 0},
  {"ldata/frontft4-64.tab",(unsigned char **)&frontend_font[3],     (unsigned char **)&frontend_end_font[3],      0, 0, 0},
  {"*FE_BACKUP_PAL",       (unsigned char **)&frontend_backup_palette,NULL,                            PALETTE_SIZE, 0, 0},
  {"",                     NULL,                                  NULL,                                           0, 0, 0},
};

struct TbLoadFiles pointer_load_files_320[] = {
  {"data/pointer-32.dat",(unsigned char **)&pointer_data,        NULL,                                          0, 0, 0},
  {"data/pointer-32.tab",(unsigned char **)&pointer_sprites,     (unsigned char **)&end_pointer_sprites,        0, 0, 0},
  {"",                   NULL,                                   NULL,                                          0, 0, 0},
};

struct TbLoadFiles pointer_small_load_files_320[] = {
  {"data/pointsm-32.dat",(unsigned char **)&pointer_data,        NULL,                                          0, 0, 0},
  {"data/pointsm-32.tab",(unsigned char **)&pointer_sprites,     (unsigned char **)&end_pointer_sprites,        0, 0, 0},
  {"",                   NULL,                                   NULL,                                          0, 0, 0},
};

struct TbLoadFiles pointer_load_files_640[] = {
  {"data/pointer-64.dat",(unsigned char **)&pointer_data,        NULL,                                          0, 0, 0},
  {"data/pointer-64.tab",(unsigned char **)&pointer_sprites,     (unsigned char **)&end_pointer_sprites,        0, 0, 0},
  {"",                   NULL,                                   NULL,                                          0, 0, 0},
};

struct TbLoadFiles pointer_small_load_files_640[] = {
  {"data/pointsm-64.dat",(unsigned char **)&pointer_data,        NULL,                                          0, 0, 0},
  {"data/pointsm-64.tab",(unsigned char **)&pointer_sprites,     (unsigned char **)&end_pointer_sprites,        0, 0, 0},
  {"",                   NULL,                                   NULL,                                          0, 0, 0},
};
#else
struct TbLoadFiles front_load_files_minimal_640[] = {
  {"data/gui1-32.dat",   (unsigned char **)&button_sprite_data, (unsigned char **)&end_button_sprite_data,      0, 0, 0},
  {"data/gui1-32.tab",   (unsigned char **)&button_sprite,      (unsigned char **)&end_button_sprites,          0, 0, 0},
  {"ldata/frontft1.dat", (unsigned char **)&frontend_font_data[0],(unsigned char **)&frontend_end_font_data[0], 0, 0, 0},
  {"ldata/frontft1.tab", (unsigned char **)&frontend_font[0],     (unsigned char **)&frontend_end_font[0],      0, 0, 0},
  {"ldata/frontft2.dat", (unsigned char **)&frontend_font_data[1],(unsigned char **)&frontend_end_font_data[1], 0, 0, 0},
  {"ldata/frontft2.tab", (unsigned char **)&frontend_font[1],     (unsigned char **)&frontend_end_font[1],      0, 0, 0},
  {"ldata/frontft3.dat", (unsigned char **)&frontend_font_data[2],(unsigned char **)&frontend_end_font_data[2], 0, 0, 0},
  {"ldata/frontft3.tab", (unsigned char **)&frontend_font[2],     (unsigned char **)&frontend_end_font[2],      0, 0, 0},
  {"ldata/frontft4.dat", (unsigned char **)&frontend_font_data[3],(unsigned char **)&frontend_end_font_data[3], 0, 0, 0},
  {"ldata/frontft4.tab", (unsigned char **)&frontend_font[3],     (unsigned char **)&frontend_end_font[3],      0, 0, 0},
  {"*FE_BACKUP_PAL",     (unsigned char **)&frontend_backup_palette,NULL,                            PALETTE_SIZE, 0, 0},
  {"",                   NULL,                                  NULL,                                           0, 0, 0},
};

struct TbLoadFiles pointer_load_files_320[] = {
  {"data/pointer32.dat", (unsigned char **)&pointer_data,        NULL,                                          0, 0, 0},
  {"data/pointer32.tab", (unsigned char **)&pointer_sprites,     (unsigned char **)&end_pointer_sprites,        0, 0, 0},
  {"",                   NULL,                                   NULL,                                          0, 0, 0},
};

struct TbLoadFiles pointer_small_load_files_320[] = {
  {"data/points32.dat",  (unsigned char **)&pointer_data,        NULL,                                          0, 0, 0},
  {"data/points32.tab",  (unsigned char **)&pointer_sprites,     (unsigned char **)&end_pointer_sprites,        0, 0, 0},
  {"",                   NULL,                                   NULL,                                          0, 0, 0},
};

struct TbLoadFiles pointer_load_files_640[] = {
  {"data/pointer64.dat", (unsigned char **)&pointer_data,        NULL,                                          0, 0, 0},
  {"data/pointer64.tab", (unsigned char **)&pointer_sprites,     (unsigned char **)&end_pointer_sprites,        0, 0, 0},
  {"",                   NULL,                                   NULL,                                          0, 0, 0},
};

struct TbLoadFiles pointer_small_load_files_640[] = {
  {"data/points64.dat",  (unsigned char **)&pointer_data,        NULL,                                          0, 0, 0},
  {"data/points64.tab",  (unsigned char **)&pointer_sprites,     (unsigned char **)&end_pointer_sprites,        0, 0, 0},
  {"",                   NULL,                                   NULL,                                          0, 0, 0},
};
#endif

struct TbLoadFiles legal_load_files[] = {
    {"*PALETTE",         &engine_palette,                        NULL,                               PALETTE_SIZE, 0, 0},
    {"*SCRATCH",         &scratch,                               NULL,                                    0x10000, 1, 0},
    {"",                 NULL,                                   NULL,                                          0, 0, 0},
};

struct TbLoadFiles game_load_files[] = {
    {"*SCRATCH",         &scratch,                               NULL,                                    0x10000, 0, 0},
    {"*TEXTURE_PAGE",    (unsigned char **)&block_mem,                             NULL,                     max(sizeof(block_mem),960*720u), 0, 0},// Store whole texture image or land view image
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

struct TbLoadFiles map_flag_load_files[] = {
  {"ldata/lndflag_ens.dat",(unsigned char **)&map_flag_data,     (unsigned char **)&end_map_flag_data,          0, 0, 0},
  {"ldata/lndflag_ens.tab",(unsigned char **)&map_flag,          (unsigned char **)&end_map_flag,               0, 0, 0},
  {"",                   NULL,                                   NULL,                                          0, 0, 0},
};

struct TbSetupSprite map_flag_setup_sprites[] = {
  {&map_flag, &end_map_flag, &map_flag_data},
  {NULL,      NULL,          NULL,},
};

struct TbLoadFiles netmap_flag_load_files[] = {
  {"ldata/netflag_ens.dat",(unsigned char **)&map_flag_data,     (unsigned char **)&end_map_flag_data,          0, 0, 0},
  {"ldata/netflag_ens.tab",(unsigned char **)&map_flag,          (unsigned char **)&end_map_flag,               0, 0, 0},
  {"ldata/netfont.dat",  (unsigned char **)&map_font_data,       (unsigned char **)&end_map_font_data,          0, 0, 0},
  {"ldata/netfont.tab",  (unsigned char **)&map_font,            (unsigned char **)&end_map_font,               0, 0, 0},
  {"ldata/maphand.dat",  (unsigned char **)&map_hand_data,       (unsigned char **)&end_map_hand_data,          0, 0, 0},
  {"ldata/maphand.tab",  (unsigned char **)&map_hand,            (unsigned char **)&end_map_hand,               0, 0, 0},
  {"",                   NULL,                                   NULL,                                          0, 0, 0},
};

struct TbSetupSprite netmap_flag_setup_sprites[] = {
  {&map_flag, &end_map_flag, &map_flag_data},
  {&map_font, &end_map_font, &map_font_data},
  {&map_hand, &end_map_hand, &map_hand_data},
  {NULL,      NULL,          NULL,},
};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
