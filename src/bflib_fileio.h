/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_fileio.h
 *     Header file for bflib_fileio.c.
 * @par Purpose:
 *     File handling routines wrapper.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Feb 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_FILEIO_H
#define BFLIB_FILEIO_H

#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
enum TbFileMode {
        Lb_FILE_MODE_NEW       = 0,
        Lb_FILE_MODE_OLD       = 1,
        Lb_FILE_MODE_READ_ONLY = 2,
};

enum TbFileSeekMode {
        Lb_FILE_SEEK_BEGINNING = 0,
        Lb_FILE_SEEK_CURRENT,
        Lb_FILE_SEEK_END,
};


/******************************************************************************/

short LbFileExists(const char *fname);
int LbFilePosition(TbFileHandle handle);
TbFileHandle LbFileOpen(const char *fname, unsigned char accmode);
TbBool LbFileEof(TbFileHandle handle);
int LbFileClose(TbFileHandle handle);
int LbFileSeek(TbFileHandle handle, long offset, unsigned char origin);
int LbFileRead(TbFileHandle handle, void *buffer, unsigned long len);
long LbFileWrite(TbFileHandle handle, const void *buffer, const unsigned long len);
long LbFileLength(const char *fname);
long LbFileLengthHandle(TbFileHandle handle);
int LbFileFindFirst(const char *filespec, struct TbFileFind *ffind,unsigned int attributes);
int LbFileFindNext(struct TbFileFind *ffind);
int LbFileFindEnd(struct TbFileFind *ffind);
int LbFileDelete(const char *filename);
short LbFileFlush(TbFileHandle handle);
int LbFileMakeFullPath(const short append_cur_dir,
  const char *directory, const char *filename, char *buf, const unsigned long len);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
