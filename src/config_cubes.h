/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_cubes.h
 *     Header file for config_cubes.c.
 * @par Purpose:
 *     Terrain cubes configuration loading functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Jun 2012 - 24 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGCUBES_H
#define DK_CFGCUBES_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"
#include "light_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define CUBE_ITEMS_MAX 512
#define CUBE_TEXTURES 6

/******************************************************************************/
#pragma pack(1)

struct CubeAttribs { // sizeof=0x12
    unsigned short texture_id[CUBE_TEXTURES];
    unsigned char field_C[CUBE_TEXTURES];
};

#pragma pack()
/******************************************************************************/
struct CubeConfigStats {
    char code_name[COMMAND_WORD_LEN];
};

struct CubesConfig {
    long cube_types_count;
    struct CubeConfigStats cube_cfgstats[CUBE_ITEMS_MAX];
};
/******************************************************************************/
extern const char keeper_cubes_file[];
extern struct NamedCommand cubes_desc[CUBE_ITEMS_MAX];
/******************************************************************************/
TbBool load_cubes_config(unsigned short flags);
struct CubeConfigStats *get_cube_model_stats(long model);
const char *cube_code_name(long model);
ThingModel cube_model_id(const char * code_name);

long load_cube_file(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
