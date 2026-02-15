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
struct TbLoadFiles;

typedef const char * ModifyDataLoadFnameFunc(const char *);

typedef size_t (*LoadFilesGetSizeFunc)(size_t data);
typedef void (*LoadFilesUnpackFunc)(unsigned char *data, size_t size);

struct TbLoadFiles {
        char FName[DISKPATH_SIZE];
        unsigned char **Start;
        unsigned char **SEnd;
        unsigned long SLength;
        unsigned short Flags;
        unsigned short Spare;
};

struct TbLoadFilesV2 {
    char FName[DISKPATH_SIZE];
    unsigned char **Start;
    unsigned long SLength; // Actual size of data in memory
    LoadFilesGetSizeFunc GetSizeFunc;
    LoadFilesUnpackFunc UnpackFunc;
};
/******************************************************************************/
const char * defaultModifyDataLoadFilename(const char *);
ModifyDataLoadFnameFunc *LbDataLoadSetModifyFilenameFunction(ModifyDataLoadFnameFunc *newfunc);
extern ModifyDataLoadFnameFunc *modify_data_load_filename_function;

/******************************************************************************/

short LbDataFree(struct TbLoadFiles *load_file);
int LbDataLoad(struct TbLoadFiles *load_file, LoadFilesGetSizeFunc get_size_fn, LoadFilesUnpackFunc unpack_fn);
void LbDataFreeAll(struct TbLoadFiles load_files[]);
void LbDataFreeAllV2(struct TbLoadFilesV2 load_files[]);

int LbDataLoadAll(struct TbLoadFiles load_files[]);
int LbDataLoadAllV2(struct TbLoadFilesV2 load_files[]);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
