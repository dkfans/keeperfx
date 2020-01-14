/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_fileio.c
 *     File handling routines wrapper.
 * @par Purpose:
 *     Buffer library for file i/o and directory manage routines.
 *     These should be used for all file access in the game.
 * @par Comment:
 *     Wraps standard c file handling routines. You could say this has no purpose,
 *     but here it is anyway.
 * @author   Tomasz Lis
 * @date     10 Feb 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_fileio.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <time.h>
#include <share.h>

#include "bflib_basics.h"
#include "bflib_datetm.h"

#if defined(_WIN32)||defined(DOS)||defined(GO32)
#include <dos.h>
#include <direct.h>
#endif

#if defined(_WIN32)
#ifdef __cplusplus
extern "C" {
#endif
//Selected declarations frow Win32 API - I don't want to use whole API
// since it influences everything
#ifndef WINBASEAPI
#ifdef __W32API_USE_DLLIMPORT__
#define WINBASEAPI DECLSPEC_IMPORT
#else
#define WINBASEAPI
#endif
#endif
#define F_OK 0
#define WINAPI __stdcall
typedef char *PCHAR,*LPCH,*PCH,*NPSTR,*LPSTR,*PSTR;
typedef const char *LPCCH,*PCSTR,*LPCSTR;
typedef unsigned long DWORD;
typedef int WINBOOL,*PWINBOOL,*LPWINBOOL;
#define BOOL WINBOOL
typedef void *PVOID,*LPVOID;
typedef PVOID HANDLE;
#define DECLARE_HANDLE(n) typedef HANDLE n
typedef HANDLE *PHANDLE,*LPHANDLE;
WINBASEAPI DWORD WINAPI GetShortPathNameA(LPCSTR,LPSTR,DWORD);
#define GetShortPathName GetShortPathNameA
WINBASEAPI BOOL WINAPI FlushFileBuffers(HANDLE);
WINBASEAPI DWORD WINAPI GetLastError(void);
#ifdef __cplusplus
}
#endif
#endif
/******************************************************************************/
//Internal declarations
void convert_find_info(struct TbFileFind *ffind);
/******************************************************************************/

int LbDriveCurrent(unsigned int *drive)
{
#if defined(_WIN32)||defined(DOS)||defined(GO32)
  *drive=_getdrive();
#else
  //Let's assume we're on 'C' drive on Unix ;)
  *drive=3;
#endif
  return 1;
}

//  Changes the current disk drive into given one
int LbDriveChange(const unsigned int drive)
{
  int result;
#if defined(_WIN32)||defined(DOS)||defined(GO32)
  int reterror = _chdrive(drive);
  if ( reterror )
  {
    result = -1;
  } else
  {
    result = 1;
  }
#else
  //Let's assume we can only be on 'C' drive on Unix
  if ( drive!=3 )
  {
    result = -1;
  } else
  {
    result = 1;
  }
#endif
  return result;
}

/** Returns if a given drive exists.
 *
 * @param drive
 * @return
 */
int LbDriveExists(const unsigned int drive)
{
  int result;
#if defined(_WIN32)||defined(DOS)||defined(GO32)
  unsigned int lastdrive=_getdrive();
  if ( _chdrive(drive) )
  {
    result = -1;
  } else
  {
    result = 1;
    _chdrive(lastdrive);
  }
#else
  //Let's assume we have only 'C' drive on Unix
  if ( drive!=3 )
  {
    result = -1;
  } else
  {
    result = 1;
  }
#endif
  return result;
}

/** Changes the current directory on the specified drive to the specified path.
 *  If no drive is specified in path then the current drive is assumed.
 *  The path can be either relative to the current directory
 *  on the specified drive or it can be an absolute path name.
 *
 * @param path
 * @return
 */
int LbDirectoryChange(const char *path)
{
  int result;
  if ( chdir(path) )
    result = -1;
  else
    result = 1;
  return result;
}

int LbDriveFreeSpace(const unsigned int drive, struct TbDriveInfo *drvinfo)
{
  int result;
#if defined(_WIN32)||defined(DOS)||defined(GO32)
  struct _diskfree_t diskspace;
  int reterror = _getdiskfree(drive, &diskspace);
  if ( reterror )
  {
    result = -1;
  } else
  {
    drvinfo->TotalClusters = diskspace.total_clusters;
    drvinfo->FreeClusters = diskspace.avail_clusters;
    drvinfo->SectorsPerCluster = diskspace.sectors_per_cluster;
    drvinfo->BytesPerSector = diskspace.bytes_per_sector;
    result = 1;
  }
#else
    //On non-win32 systems - return anything big enough
    drvinfo->TotalClusters = 65535;
    drvinfo->FreeClusters = 65535;
    drvinfo->SectorsPerCluster = 512;
    drvinfo->BytesPerSector = 512;
    result = 1;
#endif
  return result;
}

short LbFileExists(const char *fname)
{
  return access(fname,F_OK) == 0;
}

int LbFilePosition(TbFileHandle handle)
{
  int result = tell(handle);
  return result;
}

TbFileHandle LbFileOpen(const char *fname, const unsigned char accmode)
{
  unsigned char mode = accmode;

  if ( !LbFileExists(fname) )
  {
#ifdef __DEBUG
    LbSyncLog("LbFileOpen: file doesn't exist\n");
#endif
    if ( mode == Lb_FILE_MODE_READ_ONLY )
      return -1;
    if ( mode == Lb_FILE_MODE_OLD )
      mode = Lb_FILE_MODE_NEW;
  }
/* DISABLED - NOT NEEDED
  if ( mode == Lb_FILE_MODE_NEW )
  {
#ifdef __DEBUG
    LbSyncLog("LbFileOpen: creating file\n");
#endif
    rc = _sopen(fname, _O_WRONLY|_O_CREAT|_O_TRUNC|_O_BINARY, _SH_DENYNO);
    //setmode(rc,_O_TRUNC);
    close(rc);
  }
*/
  TbFileHandle rc = -1;
  switch (mode)
  {
  case Lb_FILE_MODE_NEW:
    {
#ifdef __DEBUG
      LbSyncLog("LbFileOpen: LBO_CREAT mode\n");
#endif
        rc = _sopen(fname, O_RDWR|O_CREAT|O_BINARY, SH_DENYNO, S_IREAD|S_IWRITE);
    };break;
  case Lb_FILE_MODE_OLD:
    {
#ifdef __DEBUG
        LbSyncLog("LbFileOpen: LBO_RDWR mode\n");
#endif
        rc = _sopen(fname, O_RDWR|O_BINARY, SH_DENYNO);
    };break;
  case Lb_FILE_MODE_READ_ONLY:
    {
#ifdef __DEBUG
        LbSyncLog("LbFileOpen: LBO_RDONLY mode\n");
#endif
        rc = _sopen(fname, O_RDONLY|O_BINARY, SH_DENYNO);
    };break;
  }
#ifdef __DEBUG
  LbSyncLog("LbFileOpen: out handle = %ld, errno = %d\n",rc,errno);
#endif
  return rc;
}

//Closes a file
int LbFileClose(TbFileHandle handle)
{
  if ( close(handle) )
    return -1;
  else
    return 1;
}

/*
 * Checks if the file position indicator is placed at end of the file.
 */
TbBool LbFileEof(TbFileHandle handle)
{
  if (LbFilePosition(handle) >= LbFileLengthHandle(handle))
    return 1;
  return 0;
}

/** Changes position in opened file.
 *
 * @param handle
 * @param offset
 * @param origin
 * @return Returns new file position, or -1 on error.
 */
int LbFileSeek(TbFileHandle handle, long offset, unsigned char origin)
{
  int rc;
  switch (origin)
  {
  case Lb_FILE_SEEK_BEGINNING:
      rc = lseek(handle, offset, SEEK_SET);
      break;
  case Lb_FILE_SEEK_CURRENT:
      rc = lseek(handle, offset, SEEK_CUR);
      break;
  case Lb_FILE_SEEK_END:
      rc = lseek(handle, offset, SEEK_END);
      break;
  default:
      rc = -1;
      break;
  }
  return rc;
}

/**
 * Reads from previously opened disk file.
 *
 * @param handle
 * @param buffer
 * @param len
 * @return Gives amount of bytes read, or -1 on error.
 */
int LbFileRead(TbFileHandle handle, void *buffer, unsigned long len)
{
  //'read' returns (-1) on error
  int result = read(handle, buffer, len);
  return result;
}

/**
 * Writes data at the operating system level.
 * The number of bytes transmitted is given by len and the data
 * to be transmitted is located at the address specified by buffer.
 * @return Returns the number of bytes (does not include any extra carriage-return
 * characters transmitted) of data transmitted to the file.
*/
long LbFileWrite(TbFileHandle handle, const void *buffer, const unsigned long len)
{
    long result = write(handle, buffer, len);
    return result;
}

/**
 * Flushes the file buffers, writing all data immediately.
 * @return Returns 1 on success, 0 on error.
*/
short LbFileFlush(TbFileHandle handle)
{
#if defined(_WIN32)
  // Crappy Windows has its own
  int result = FlushFileBuffers((HANDLE)handle);
  // It returns 'invalid handle' error sometimes for no reason.. so disabling this error
  if (result != 0)
      return 1;
  result = GetLastError();
  return ((result == 0) || (result == 6));
#else
#if defined(DOS)||defined(GO32)
  // No idea how to do this on old systems
  return 1;
#else
  // For normal POSIX systems
  // (should also work on Win, as its IEEE standard... but it currently isn't)
  return (ioctl(handle,I_FLUSH,FLUSHRW) != -1);
#endif
#endif

}

long LbFileLengthHandle(TbFileHandle handle)
{
    long result = filelength(handle);
    return result;
}

//Returns disk size of file
long LbFileLength(const char *fname)
{
    TbFileHandle handle = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    long result = handle;
    if (handle != -1)
    {
        result = filelength(handle);
        LbFileClose(handle);
  }
  return result;
}

//Converts file search information from platform-specific into independent form
//Yeah, right...
void convert_find_info(struct TbFileFind *ffind)
{
  struct _finddata_t *fdata=&(ffind->Reserved);
  strncpy(ffind->Filename,fdata->name,144);
  ffind->Filename[143]='\0';
#if defined(_WIN32)
  GetShortPathName(fdata->name,ffind->AlternateFilename,14);
#else
  strncpy(ffind->AlternateFilename,fdata->name,14);
#endif
  ffind->AlternateFilename[13]='\0';
  if (fdata->size>ULONG_MAX)
    ffind->Length=ULONG_MAX;
  else
    ffind->Length = fdata->size;
  ffind->Attributes = fdata->attrib;
  LbDateTimeDecode(&fdata->time_create,&ffind->CreationDate,&ffind->CreationTime);
  LbDateTimeDecode(&fdata->time_write,&ffind->LastWriteDate,&ffind->LastWriteTime);
}

// returns -1 if no match is found. Otherwise returns 1 and stores a handle
// to be used in _findnext and _findclose calls inside TbFileFind struct.
int LbFileFindFirst(const char *filespec, struct TbFileFind *ffind,unsigned int attributes)
{
    // original Watcom code was
    //dos_findfirst_(path, attributes,&(ffind->Reserved))
    //The new code skips 'attributes' as Win32 prototypes seem not to use them
    ffind->ReservedHandle = _findfirst(filespec,&(ffind->Reserved));
    int result;
    if (ffind->ReservedHandle == -1)
    {
      result = -1;
    } else
    {
      convert_find_info(ffind);
      result = 1;
    }
    return result;
}

// returns -1 if no match is found, otherwise returns 1
int LbFileFindNext(struct TbFileFind *ffind)
{
    int result;
    if ( _findnext(ffind->ReservedHandle,&(ffind->Reserved)) < 0 )
    {
        _findclose(ffind->ReservedHandle);
        ffind->ReservedHandle = -1;
        result = -1;
    } else
    {
        convert_find_info(ffind);
        result = 1;
    }
    return result;
}

//Ends file searching sequence
int LbFileFindEnd(struct TbFileFind *ffind)
{
    if (ffind->ReservedHandle != -1)
    {
        _findclose(ffind->ReservedHandle);
    }
    return 1;
}

//Renames a disk file
int LbFileRename(const char *fname_old, const char *fname_new)
{
  int result;
  if ( rename(fname_old,fname_new) )
    result = -1;
  else
    result = 1;
  return result;
}

//Removes a disk file
int LbFileDelete(const char *filename)
{
  int result;
  if ( remove(filename) )
    result = -1;
  else
    result = 1;
  return result;
}

char *LbGetCurrWorkDir(char *dest, const unsigned long maxlen)
{
  return getcwd(dest,maxlen);
}

int LbDirectoryCurrent(char *buf, unsigned long buflen)
{
//  if ( GetCurrentDirectoryA(buflen, buf) )
  if ( getcwd(buf,buflen) != NULL )
  {
    if ( buf[1] == ':' )
      strcpy(buf, buf+2);
    int len = strlen(buf);
    if ( len>1 )
    {
      if ( buf[len-2] == '\\' )
        buf[len-2] = '\0';
    }
    return 1;
  }
  return -1;
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
