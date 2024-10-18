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

#include <time.h>
#include <stdio.h>

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
#define COMMAND_WORD_LEN  64

// Max length of any processed string
#define MAX_TEXT_LENGTH 4096
// Smaller buffer, also widely used
#define TEXT_BUFFER_LENGTH 2048

#define MAX_CONSOLE_LOG_COUNT 1000   // Maximum number of log messages

enum TbErrorCode {
    Lb_FAIL                 = -1,
    Lb_OK                   =  0,
    Lb_SUCCESS              =  1,
};

/******************************************************************************/
#pragma pack(1)

// These types should be deprecated because we have stdint.h now.
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
typedef FILE * TbFileHandle;
typedef unsigned char TbBool;
typedef short TbScreenPos;

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

enum LogLevel {
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR,
  LOG_OFF,
};

enum LogChan {
  LOG_GENERAL,
  LOG_NET,
  LOG_AI,
  LOG_NAV,
  LOG_TEST,
  LOG_SCRIPT,
  LOG_CONFIG,
};

#pragma pack()
/******************************************************************************/
extern const char *log_file_name;
extern int debug_display_consolelog;
extern char consoleLogArray[MAX_CONSOLE_LOG_COUNT][MAX_TEXT_LENGTH];
extern size_t consoleLogArraySize;

// High level functions - DK specific
void error(const char *codefile,const int ecode,const char *message);
short warning_dialog(const char *codefile,const int ecode,const char *message);
short error_dialog(const char *codefile,const int ecode,const char *message);
short error_dialog_fatal(const char *codefile,const int ecode,const char *message);
char *buf_sprintf(const char *format, ...);
/******************************************************************************/

void LbLog(int chan, int level, const char * format, ...);
void LbSetLogLevel(int chan, int level);
void LbLogConsole(const char * format, ...);
#define LbDebugLog(format, ...) LbLog(LOG_GENERAL, LOG_DEBUG, "[%d] %s:%d %s: Debug: " format "\n", get_gameturn(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LbErrorLog(format, ...) LbLog(LOG_GENERAL, LOG_ERROR, "[%d] %s:%d %s: Error: " format "\n", get_gameturn(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LbWarnLog(format, ...) LbLog(LOG_GENERAL, LOG_WARNING, "[%d] %s:%d %s: Warning: " format "\n", get_gameturn(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LbSyncLog(format, ...) LbLog(LOG_GENERAL, LOG_INFO, "[%d] %s:%d %s: Info: " format "\n", get_gameturn(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LbNetLog(format, ...) LbLog(LOG_NET, LOG_INFO, "[%d] %s:%d %s: Info: " format "\n", get_gameturn(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LbJustLog(format, ...) LbLog(LOG_GENERAL, LOG_INFO, "[%d] %s:%d %s: Info: " format "\n", get_gameturn(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LbAiLog(format, ...) LbLog(LOG_AI, LOG_INFO, "[%d] %s:%d %s: Info: " format "\n", get_gameturn(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LbNaviLog(format, ...) LbLog(LOG_NAV, LOG_INFO, "[%d] %s:%d %s: Info: " format "\n", get_gameturn(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LbFTestLog(format, ...) LbLog(LOG_TEST, LOG_INFO, "[%d] %s:%d %s: Info: " format "\n", get_gameturn(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LbScriptLog(format, ...) LbLog(LOG_SCRIPT, LOG_INFO, "[%d] %s:%d %s: Info: " format "\n", get_gameturn(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LbConfigLog(format, ...) LbLog(LOG_CONFIG, LOG_INFO, "[%d] %s:%d %s: Info: " format "\n", get_gameturn(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
int Lbvsprintf(char* buffer, const char *format, ...);
void LbPrint(const char *format, ...);

TbBool LbLogOpen(const char *filename);
void LbLogClose(void);

/******************************************************************************/
typedef void (*TbNetworkCallbackFunc)(struct TbNetworkCallbackData *, void *);
/******************************************************************************/
unsigned long blong (unsigned char *p);
unsigned long llong (unsigned char *p);
unsigned long bword (unsigned char *p);
unsigned long lword (unsigned char *p);
long saturate_set_signed(long long val,unsigned short nbits);
unsigned long saturate_set_unsigned(unsigned long long val,unsigned short nbits);
void make_lowercase(char *);
void make_uppercase(char *);
int natoi(const char * str, int len); // like atoi but stops after len bytes

/**
 * Converts an index number to a flag - by creating a bitmask where only the nth bit is set to 1.
 * For example: idx=0 returns 0b000001, idx=1 returns 0b000010, idx=2 returns 0b000100.
 * 
 * @param idx The index number of the flag, or n, used to set the nth bit of the mask to 1.
 * @return Returns a bitmask with a single masked bit (aka "flag", "bitflag").
 */
#define to_flag(idx) (1 << idx)

/** 
 * Set a flag* - by setting the given masked bit(s) to 1 in the given flags variable. *Can set multiple flags.
 * 
 * @param flags The flags variable we want to change.
 * @param mask Bitmask, containing 1 (or more) masked bits, representing the flag(s) we want to set.
 */
#define set_flag(flags,mask) flags |= mask

/** 
 * Clear a flag* - by setting the given masked bit(s) to 0 in the given flags variable. *Can clear multiple flags.
 * 
 * @param flags The flags variable we want to change.
 * @param mask Bitmask, containing 1 (or more) masked bits, representing the flag(s) we want to clear.
 */
#define clear_flag(flags,mask) flags &= ~(mask)

/** 
 * Toggle a given flag* between set/cleared - by toggling the given masked bit(s) in the given flags variable. *Can toggle multiple flags.
 * 
 * @param flags The flags variable we want to change.
 * @param mask Bitmask, containing 1 (or more) masked bits, representing the flag(s) we want to toggle.
 */
#define toggle_flag(flags,mask) flags ^= mask

/** 
 * Check if the given flag* is set - by checking if the given masked bits are set to 1 in the given flags variable. *Can check for multiple flags.
 * 
 * @param flags The flags variable we want to check.
 * @param mask Bitmask, containing 1 (or more) masked bits, representing the flag(s) we want to check in the "flags" parameter.
 * @return Returns TRUE if the given masked bits are set to 1 in the given flags variable.
 */
#define flag_is_set(flags,mask) ((flags & mask) == mask)

/** 
 * Check if any of the given flags is set - by checking if any of the given masked bits are set to 1 in the given flags variable.
 * 
 * @param flags The flags variable we want to check.
 * @param mask Bitmask, containing 1 (or more) masked bits, representing the bit flags we want to check in the "flags" parameter.
 * @return Returns TRUE if any of the given masked bits are set to 1 in the given flags variable.
 */
#define any_flag_is_set(flags,mask) ((flags & mask) != 0)

/** 
 * Check if all of the flags are set - by checking if all of the bits are set to 1 in the given flags variable.
 * For example: all 6 players set in a flags variable would be 0b111111.
 * 
 * @param flags The flags variable we want to check.
 * @param count The number of bits used by the flags variable, i.e the count of "all of the bits".
 * @return Returns TRUE if all bits are set to 1 in the given flags variable.
 */
#define all_flags_are_set(flags,count) ((1 << count) - flags == 1)

/** 
 * Set a flag* - by setting the given masked bit(s) to "bool value" in the given flags variable. *Can set multiple flags.
 * 
 * @param flags The flags variable we want to change.
 * @param mask Bitmask, containing 1 (or more) masked bits, representing the flag(s) we want to set.
 * @param value If value == 0, then set the masked bit(s) to 0 in "flags". If value != 0, then set the masked bit(s) to 1 in "flags".
 */
#define set_flag_value(flags,mask,value) ((value) ? (set_flag(flags,mask)) : (clear_flag(flags,mask)))
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
