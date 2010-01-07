/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_filelst.h
 *     Header file for bflib_filelst.c.
 * @par Purpose:
 *     Reading/freeing of file lists.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     02 Mar 2008 - 08 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_FILELST_H
#define BFLIB_FILELST_H

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct TbLoadFiles;

typedef char *ModifyDataLoadFnameFunc(struct TbLoadFiles *);

struct TbLoadFiles {
        char FName[28];
        unsigned char **Start;
        unsigned char **SEnd;
        unsigned long SLength;
        unsigned short Flags;
        unsigned short Spare;
};

#pragma pack()
/******************************************************************************/
char *defaultModifyDataLoadFilename(struct TbLoadFiles *ldfiles);
ModifyDataLoadFnameFunc *LbDataLoadSetModifyFilenameFunction(ModifyDataLoadFnameFunc *newfunc);

/******************************************************************************/

short LbDataFree(struct TbLoadFiles *load_file);
short LbDataFreeAll(struct TbLoadFiles load_files[]);

short LbDataLoad(struct TbLoadFiles *load_file);
short LbDataLoadAll(struct TbLoadFiles load_files[]);

int LbDataFindNameIndex(struct TbLoadFiles load_files[],char *fname);
int LbDataFindStartIndex(struct TbLoadFiles load_files[],unsigned char **start);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
