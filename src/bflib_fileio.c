/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_fileio.c
 *     File handling routines wrapper.
 * @par Purpose:
 *     Thin delegates to the active IPlatform implementation registered with
 *     PlatformManager.  All OS-specific logic lives in PlatformWindows.cpp or
 *     PlatformLinux.cpp; this file only forwards calls.
 * @author   Tomasz Lis
 * @date     10 Feb 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_fileio.h"

#include <string.h>
#include <stdlib.h>

#include "platform/PlatformManager.h"
#include "bflib_basics.h"

#include "post_inc.h"

/******************************************************************************/

short LbFileExists(const char *fname)
{
    return PlatformManager_FileExists(fname);
}

int LbFilePosition(TbFileHandle handle)
{
    return PlatformManager_FilePosition(handle);
}

TbFileHandle LbFileOpen(const char *fname, const unsigned char accmode)
{
    return PlatformManager_FileOpen(fname, accmode);
}

int LbFileClose(TbFileHandle handle)
{
    return PlatformManager_FileClose(handle);
}

TbBool LbFileEof(TbFileHandle handle)
{
    return PlatformManager_FileEof(handle);
}

int LbFileSeek(TbFileHandle handle, long offset, unsigned char origin)
{
    return PlatformManager_FileSeek(handle, offset, origin);
}

int LbFileRead(TbFileHandle handle, void *buffer, unsigned long len)
{
    return PlatformManager_FileRead(handle, buffer, len);
}

long LbFileWrite(TbFileHandle handle, const void *buffer, const unsigned long len)
{
    return PlatformManager_FileWrite(handle, buffer, len);
}

short LbFileFlush(TbFileHandle handle)
{
    return PlatformManager_FileFlush(handle);
}

long LbFileLengthHandle(TbFileHandle handle)
{
    long pos = LbFilePosition(handle);
    LbFileSeek(handle, 0, Lb_FILE_SEEK_END);
    long result = LbFilePosition(handle);
    LbFileSeek(handle, pos, Lb_FILE_SEEK_BEGINNING);
    return result;
}

long LbFileLength(const char *fname)
{
    return PlatformManager_FileLength(fname);
}

int LbFileDelete(const char *filename)
{
    return PlatformManager_FileDelete(filename);
}

int LbDirectoryCurrent(char *buf, unsigned long buflen)
{
    return PlatformManager_GetCurrentDirectory(buf, buflen);
}

int LbFileMakeFullPath(const short append_cur_dir,
  const char *directory, const char *filename, char *buf, const unsigned long len)
{
  if (filename==NULL)
    { buf[0]='\0'; return -1; }
  unsigned long namestart;
  if ( append_cur_dir )
  {
    if ( LbDirectoryCurrent(buf, len-2) == -1 )
    { buf[0]='\0'; return -1; }
    namestart = strlen(buf);
    if ( (namestart>0) && (buf[namestart-1]!='\\') && (buf[namestart-1]!='/'))
    {
      buf[namestart] = '/';
      namestart++;
    }
  } else
  {
    namestart = 0;
  }
  buf[namestart] = '\0';

  if ( directory != NULL )
  {
      int copy_len = strlen(directory);
      if (len - 2 <= namestart + copy_len - 1)
          return -1;
      memcpy(buf + namestart, directory, copy_len);
      namestart += copy_len - 1;
      if ((namestart > 0) && (buf[namestart - 1] != '\\') && (buf[namestart - 1] != '/'))
      {
          buf[namestart] = '/';
          namestart++;
    }
    buf[namestart] = '\0';
  }
  if ( strlen(filename)+namestart-1 < len )
  {
    const char *ptr = filename;
    int invlen;
    for (invlen=-1;invlen!=0;invlen--)
    {
     if (*ptr++ == 0)
       {invlen--;break;}
    }
    int copy_len = ~invlen;
    const char* copy_src = &ptr[-copy_len];
    char* copy_dst = buf;
    for (invlen=-1;invlen!=0;invlen--)
    {
     if (*copy_dst++ == 0)
       {invlen--;break;}
    }
    memcpy(copy_dst-1, copy_src, copy_len);
    return 1;
  }
  return -1;
}

/******************************************************************************/
