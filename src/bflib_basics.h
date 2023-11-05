/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_basics.h
 *     Header file for bflib_basics.c.
 * @par Purpose:
 *     Integrates all elements of the library with a common toolkit.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Feb 2008 - 22 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_BASICS_H
#define BFLIB_BASICS_H

#include <io.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// Buffer sizes
// Disk path max length
#define DISKPATH_SIZE    144
#define LINEMSG_SIZE     160
#define READ_BUFSIZE     256
#define LOOPED_FILE_LEN 4096
#define COMMAND_WORD_LEN  32

// Max length of any processed string
#define MAX_TEXT_LENGTH 4096
// Smaller buffer, also widely used
#define TEXT_BUFFER_LENGTH 2048

enum TbErrorLogFlags {
        Lb_ERROR_LOG_APPEND = 0,
        Lb_ERROR_LOG_NEW    = 1,
};

enum TbLogFlags {
        LbLog_DateInHeader = 0x0010,
        LbLog_TimeInHeader = 0x0020,
        LbLog_DateInLines  = 0x0040,
        LbLog_TimeInLines  = 0x0080,
        LbLog_LoopedFile   = 0x0100,
};

enum TbErrorCode {
    Lb_FAIL                 = -1,
    Lb_OK                   =  0,
    Lb_SUCCESS              =  1,
};

/******************************************************************************/
#pragma pack(1)

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
typedef long TbClockMSec;
typedef time_t TbTimeSec;

typedef unsigned char TbChecksum;
typedef unsigned long TbBigChecksum;
typedef long Offset;
typedef int TbFileHandle;
typedef unsigned char TbBool;
typedef short TbScreenPos;

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
        char filename[DISKPATH_SIZE];
        char prefix[LOG_PREFIX_LEN];
        ulong flags;
        TbBool Initialised;
        TbBool Created;
        TbBool Suspended;
        long position;
};

struct TbNetworkCallbackData;
/** Command function result, alias for TbResult. */
typedef int TbError;
/** Command function result, valid values are of TbErrorCode enumeration. */
typedef int TbResult;
typedef size_t TbSize;

struct DebugMessage {
  struct DebugMessage * next;
  char text[0];
};

extern struct DebugMessage * debug_messages_head;
extern struct DebugMessage ** debug_messages_tail;

#pragma pack()
/******************************************************************************/
extern const char *log_file_name;
// High level functions - DK specific
void error(const char *codefile,const int ecode,const char *message);
short error_dialog(const char *codefile,const int ecode,const char *message);
short error_dialog_fatal(const char *codefile,const int ecode,const char *message);
char *buf_sprintf(const char *format, ...);
/******************************************************************************/
int LbErrorLog(const char *format, ...);
int LbWarnLog(const char *format, ...);
int LbSyncLog(const char *format, ...);
int LbNetLog(const char *format, ...);
int LbJustLog(const char *format, ...);
int LbAiLog(const char *format, ...);
int LbNaviLog(const char *format, ...);
int LbScriptLog(unsigned long line,const char *format, ...);
int LbConfigLog(unsigned long line,const char *format, ...);
void LbPrint(const char *format, ...);

int LbErrorLogSetup(const char *directory, const char *filename, TbBool flag);
int LbErrorLogClose(void);

int LbLogClose(struct TbLog *log);
int LbLogSetup(struct TbLog *log, const char *filename, ulong flags);
int LbLogSetPrefix(struct TbLog *log, const char *prefix);
int LbLogSetPrefixFmt(struct TbLog *log, const char *format, ...);

void LbCloseLog();
/******************************************************************************/
typedef void (*TbNetworkCallbackFunc)(struct TbNetworkCallbackData *, void *);
/******************************************************************************/
unsigned long blong (unsigned char *p);
unsigned long llong (unsigned char *p);
unsigned long bword (unsigned char *p);
unsigned long lword (unsigned char *p);
void set_flag_byte(unsigned char *flags,unsigned char mask,short value);
void set_flag_word(unsigned short *flags,unsigned short mask,short value);
void set_flag_dword(unsigned long *flags,unsigned long mask,short value);
void toggle_flag_byte(unsigned char *flags,unsigned char mask);
void toggle_flag_word(unsigned short *flags,unsigned short mask);
void toggle_flag_dword(unsigned long *flags,unsigned long mask);
long saturate_set_signed(long long val,unsigned short nbits);
unsigned long saturate_set_unsigned(unsigned long long val,unsigned short nbits);
void make_lowercase(char *);
void make_uppercase(char *);

// TODO: refactor usage of set_flag_* and toggle_flag_* functions to use the following macros instead

/** Returns TRUE if the masked bit is set in the given bitflags. */
#define flag_is_set(flags, masked_bit) (flags & masked_bit)
/** Set the masked bit in the given bitflags. */
#define set_flag(flags, masked_bit) flags |= masked_bit
/** Clear the masked bit in the given bitflags. */
#define clear_flag(flags, masked_bit) flags &= ~(masked_bit)
/** Toggle the masked bit in the given bitflags. */
#define toggle_flag(flags, masked_bit) flags ^= masked_bit

/** convert an index number to a bitflag (e.g. idx 0 = 0b001, idx 3 = 0b100). */
#define index_to_flag(idx) (1 << idx)
/** Returns TRUE if the nth bit is set in the given bitflags. */
#define indexed_flag_is_set(flags, n) flag_is_set(flags, index_to_flag(n))
/** Set the nth bit in the given bitflags. */
#define set_indexed_flag(flags, n) set_flag(flags, index_to_flag(n))
/** Clear the nth bit in the given bitflags. */
#define clear_indexed_flag(flags, n) clear_flag(flags, index_to_flag(n))
/** Toggle the nth bit in the given bitflags. */
#define toggle_indexed_flag(flags, n) toggle_flag(flags, index_to_flag(n))

/** Returns TRUE if the all flags (up to the given max index) are set in the given bitflags. */
#define all_flags_set(flags, max_index) ((1 << (max_index + 1)) - flags == 1)
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
