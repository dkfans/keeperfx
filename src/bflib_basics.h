/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 10 Feb 2008

// Purpose:
//    Header file for bflib_basics.c.

// Comment:
//   Just a header file - #defines, typedefs, function prototypes etc.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#ifndef BFLIB_BASICS_H
#define BFLIB_BASICS_H

#include <io.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// Buffer sizes
#define DISKPATH_SIZE   144
#define LINEMSG_SIZE    160
#define READ_BUFSIZE    256

#pragma pack(1)

enum TbErrorLogFlag {
        Lb_ERROR_LOG_APPEND = 0,
        Lb_ERROR_LOG_NEW =  1,
};

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

struct TbTime {
        unsigned char Hour;
        unsigned char Minute;
        unsigned char Second;
        unsigned char HSecond;
};
struct TbDate {
        unsigned char Day;
        unsigned char Month;
        unsigned short Year;
        unsigned char DayOfWeek;
};

typedef int TbFileHandle;
struct TbFileFind {
          char Filename[144];
          char AlternateFilename[14];
          unsigned long Attributes;
          unsigned long Length;
          struct TbDate CreationDate;
          struct TbTime CreationTime;
          struct TbDate LastWriteDate;
          struct TbTime LastWriteTime;
          unsigned long ReservedHandle;
          struct _finddata_t Reserved;
};

#define LOG_PREFIX_LEN 32

struct TbLog {
        char Filename[DISKPATH_SIZE];
        char Prefix[LOG_PREFIX_LEN];
        ulong Flags;
        bool Initialised;
        bool Created;
        bool Suspended;
};

#pragma pack()

/******************************************************************************/
extern const char *log_file_name;
// High level functions - DK specific
void error(const char *codefile,const int ecode,const char *message);
short error_dialog(const char *codefile,const int ecode,const char *message);
short error_dialog_fatal(const char *codefile,const int ecode,const char *message);
/******************************************************************************/
int LbErrorLog(const char *format, ...);
int LbSyncLog(const char *format, ...);
int __fastcall LbErrorLogSetup(const char *directory, const char *filename, uchar flag);
int __fastcall LbErrorLogClose();

int __fastcall LbLogClose(TbLog *log);
int __fastcall LbLogSetup(TbLog *log, const char *filename, int flags);
int __fastcall LbLogSetPrefix(TbLog *log, const char *prefix);
/******************************************************************************/
// Return the big-endian longword at p.
inline unsigned long blong (unsigned char *p)
{
    unsigned long n;
    n = p[0];
    n = (n << 8) + p[1];
    n = (n << 8) + p[2];
    n = (n << 8) + p[3];
    return n;
}

// Return the little-endian longword at p.
inline unsigned long llong (unsigned char *p)
{
    unsigned long n;
    n = p[3];
    n = (n << 8) + p[2];
    n = (n << 8) + p[1];
    n = (n << 8) + p[0];
    return n;
}

// Return the big-endian word at p.
inline unsigned long bword (unsigned char *p)
{
    unsigned long n;
    n = p[0];
    n = (n << 8) + p[1];
    return n;
}

// Return the little-endian word at p.
inline unsigned long lword (unsigned char *p)
{
    unsigned long n;
    n = p[1];
    n = (n << 8) + p[0];
    return n;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
