/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_heap.c
 *     Definition of heap, used for storing memory-expensive sounds and graphics.
 * @par Purpose:
 *     Functions to create and maintain memory heap.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 19 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "game_heap.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "bflib_fileio.h"
#include "config.h"
#include "front_simple.h"
#include "engine_render.h"
#include "sounds.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/

TbBool setup_heap_manager(void)
{
    SYNCDBG(8,"Starting");
    long i;
#ifdef SPRITE_FORMAT_V2
    const char* fname = prepare_file_fmtpath(FGrp_StdData,"thingspr-%d.jty",32);
#else
    const char* fname = prepare_file_path(FGrp_StdData, "creature.jty");
#endif
    jty_file_handle = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (!jty_file_handle) {
        ERRORLOG("Can not open JTY file, \"%s\"",fname);
        return false;
    }
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        keepsprite[i] = NULL;
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        sprite_heap_handle[i] = NULL;
    return true;
}

void reset_heap_manager(void)
{
    long i;
    SYNCDBG(8,"Starting");
    LbFileClose(jty_file_handle);
    jty_file_handle = NULL;
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        keepsprite[i] = NULL;
    for (i=0; i < KEEPSPRITE_LENGTH; i++)
        sprite_heap_handle[i] = NULL;
}

void *he_alloc(size_t size)
{
    // We could need some wrapper
    return malloc(size);
}
/******************************************************************************/
