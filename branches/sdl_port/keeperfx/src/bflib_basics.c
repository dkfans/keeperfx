/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_basics.c
 *     Basic definitions and global routines for all library files.
 * @par Purpose:
 *     Integrates all elements of the library with a common toolkit.
 * @par Comment:
 *     Only simple, basic functions which can be used in every library file.
 * @author   Tomasz Lis
 * @date     10 Feb 2008 - 22 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_basics.h"
#include "globals.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

#include "bflib_datetm.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
// Functions which were previously defined as Inline,
// but redefined for compatibility with both Ansi-C and C++.

/** Return the big-endian longword at p. */
unsigned long blong (unsigned char *p)
{
    unsigned long n;
    n = p[0];
    n = (n << 8) + p[1];
    n = (n << 8) + p[2];
    n = (n << 8) + p[3];
    return n;
}

/** Return the little-endian longword at p. */
unsigned long llong (unsigned char *p)
{
    unsigned long n;
    n = p[3];
    n = (n << 8) + p[2];
    n = (n << 8) + p[1];
    n = (n << 8) + p[0];
    return n;
}

/** Return the big-endian word at p. */
unsigned long bword (unsigned char *p)
{
    unsigned long n;
    n = p[0];
    n = (n << 8) + p[1];
    return n;
}

/* Return the little-endian word at p. */
unsigned long lword (unsigned char *p)
{
    unsigned long n;
    n = p[1];
    n = (n << 8) + p[0];
    return n;
}

/**
 * Toggles a masked bit in the flags field to the value.
 * This version assumes the flag field is 1 byte long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 */
void toggle_flag_byte(unsigned char *flags,unsigned char mask)
{
  if ((*flags & mask) == 0)
    *flags |= mask;
  else
    *flags ^= mask;
}

/**
 * Toggles a masked bit in the flags field to the value.
 * This version assumes the flag field is 4 bytes long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 */
void toggle_flag_dword(unsigned long *flags,unsigned long mask)
{
  if ((*flags & mask) == 0)
    *flags |= mask;
  else
    *flags ^= mask;
}

/**
 * Sets a masked bit in the flags field to the value.
 * This version assumes the flag field is 2 bytes long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 * @param value The new logic value.
 */
void set_flag_word(unsigned short *flags,unsigned short mask,short value)
{
  if (value)
    *flags |= mask;
  else
    *flags ^= *flags & mask;
}

/**
 * Sets a masked bit in the flags field to the value.
 * This version assumes the flag field is 1 byte long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 * @param value The new logic value.
 */
void set_flag_byte(unsigned char *flags,unsigned char mask,short value)
{
  if (value)
    *flags |= mask;
  else
    *flags ^= *flags & mask;
}

/**
 * Sets a masked bit in the flags field to the value.
 * This version assumes the flag field is 4 bytes long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 * @param value The new logic value.
 */
void set_flag_dword(unsigned long *flags,unsigned long mask,short value)
{
  if (value)
    *flags |= mask;
  else
    *flags ^= *flags & mask;
}

/**
 * Returns a signed value, which is equal to val if it fits in nbits.
 * Otherwise, returns max value that can fit in nbits.
 * @param val the value to be saturated.
 * @param nbits Max bits size, including sign bit.
 */
long saturate_set_signed(long long val,unsigned short nbits)
{
  long long max = (1 << (nbits-1)) - 1;
  if (val >= max)
    return max;
  if (val <= -max)
    return -max;
  return val;
}

/**
 * Returns an unsigned value, which is equal to val if it fits in nbits.
 * Otherwise, returns max value that can fit in nbits.
 * @param val the value to be saturated.
 * @param nbits Max bits size, including sign bit.
 */
unsigned long saturate_set_unsigned(unsigned long long val,unsigned short nbits)
{
  unsigned long long max = (1 << (nbits)) - 1;
  if (val >= max)
    return max;
  return val;
}

/******************************************************************************/
const char *log_file_name=DEFAULT_LOG_FILENAME;

char *buf_sprintf(const char *format, ...)
{
    static char text[TEXT_BUFFER_LENGTH+1];
    va_list val;
    va_start(val, format);
    vsprintf(text, format, val);
    text[TEXT_BUFFER_LENGTH]='\0';
    va_end(val);
    return text;
}

void error(const char *codefile,const int ecode,const char *message)
{
  LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
}

short error_dialog(const char *codefile,const int ecode,const char *message)
{
  LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
  MessageBox(NULL, message, PROGRAM_FULL_NAME, MB_OK | MB_ICONERROR);
  return 0;
}

short error_dialog_fatal(const char *codefile,const int ecode,const char *message)
{
  static char msg_text[2048];
  HWND whandle;
  LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
  sprintf(msg_text,"%s This error in '%s' makes the program unable to continue. See '%s' for details.",message,codefile,log_file_name);
  whandle = GetDesktopWindow();
  MessageBox(whandle, msg_text, PROGRAM_FULL_NAME, MB_OK | MB_ICONERROR);
  return 0;
}

/******************************************************************************/
short error_log_initialised=false;
struct TbLog error_log;
TbBool net_log_initialised = false;
struct TbLog net_log;
/******************************************************************************/
int LbLog(struct TbLog *log, const char *fmt_str, va_list arg);
/******************************************************************************/

int LbErrorLog(const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "Error: ");
    va_list val;
    va_start(val, format);
    int result=LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbWarnLog(const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "Warning: ");
    va_list val;
    va_start(val, format);
    int result=LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbNetLog(const char *format, ...)
{
    if (!net_log_initialised)
        return -1;
    LbLogSetPrefix(&net_log, "Net: ");
    va_list val;
    va_start(val, format);
    int result=LbLog(&net_log, format, val);
    va_end(val);
    return result;
}

int LbSyncLog(const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "Sync: ");
    va_list val;
    va_start(val, format);
    int result=LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

/*
 * Logs script-related message.
 */
int LbScriptLog(unsigned long line,const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefixFmt(&error_log, "Script(line %lu): ",line);
    va_list val;
    va_start(val, format);
    int result=LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

/*
 * Logs config file related message.
 */
int LbConfigLog(unsigned long line,const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefixFmt(&error_log, "Config(line %lu): ",line);
    va_list val;
    va_start(val, format);
    int result=LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbJustLog(const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "");
    va_list val;
    va_start(val, format);
    int result=LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbErrorLogSetup(const char *directory, const char *filename, TbBool flag)
{
  if ( error_log_initialised )
    return -1;
  const char *fixed_fname;
  if ((filename != NULL) && (filename[0] != '\0'))
    fixed_fname = filename;
  else
    fixed_fname = "error.log";
  char log_filename[DISKPATH_SIZE];
  int result;
  ulong flags;
  if ( LbFileMakeFullPath(true,directory,fixed_fname,log_filename,DISKPATH_SIZE) != 1 )
    return -1;
  flags = (flag==0)+1;
  flags |= LbLog_TimeInHeader | LbLog_DateInHeader | 0x04;
  if ( LbLogSetup(&error_log, log_filename, flags) == 1 )
  {
    error_log_initialised = 1;
    result = 1;
  } else
  {
    result = -1;
  }
  return result;
}

int LbNetLogSetup(const char *directory, const char *filename, TbBool flag)
{
  if ( net_log_initialised )
    return -1;
  const char *fixed_fname;
  if ((filename != NULL) && (filename[0] != '\0'))
    fixed_fname = filename;
  else
    fixed_fname = "net.log";
  char log_filename[DISKPATH_SIZE];
  int result;
  ulong flags;
  if ( LbFileMakeFullPath(true,directory,fixed_fname,log_filename,DISKPATH_SIZE) != 1 )
    return -1;
  flags = (flag==0)+1;
  flags |= LbLog_TimeInHeader | LbLog_DateInHeader | 0x04;
  if ( LbLogSetup(&net_log, log_filename, flags) == 1 )
  {
    net_log_initialised = 1;
    result = 1;
  } else
  {
    result = -1;
  }
  return result;
}

int LbNetLogClose(void)
{
	if (!net_log_initialised)
		return -1;
	return LbLogClose(&net_log);
}

int LbErrorLogClose(void)
{
    if (!error_log_initialised)
        return -1;
    return LbLogClose(&error_log);
}

int LbLog(struct TbLog *log, const char *fmt_str, va_list arg)
{
  enum Header {
        NONE   = 0,
        CREATE = 1,
        APPEND = 2,
  };
//  printf(fmt_str, arg);
  if (!log->Initialised)
    return -1;
  FILE *file;
  short need_initial_newline;
  char header;
  if ( log->Suspended )
    return 1;
  header = NONE;
  need_initial_newline = false;
  if ( !log->Created )
  {
      if (((log->flags & 0x04) == 0) || LbFileExists(log->filename))
      {
        if (((log->flags & 0x01) != 0) && ((log->flags & 0x04) != 0))
        {
          header = CREATE;
        } else
        if (((log->flags & 0x02) != 0) && ((log->flags & 0x08) != 0))
        {
          need_initial_newline = true;
          header = APPEND;
        }
      } else
      {
        header = CREATE;
      }
  }
   const char *accmode;
    if ((log->Created) || ((log->flags & 0x01) == 0))
      accmode = "a";
    else
      accmode = "w";
    file = fopen(log->filename, accmode);
    if (file == NULL)
    {
      return -1;
    }
    log->Created = true;
    if (header != NONE)
    {
      if ( need_initial_newline )
        fprintf(file, "\n");
      const char *actn;
      if (header == CREATE)
      {
        fprintf(file, PROGRAM_NAME" ver "VER_STRING" (%s release)\n", (BFDEBUG_LEVEL>1)?"debug":"standard");
        actn = "CREATED";
      } else
      {
        actn = "APPENDED";
      }
      fprintf(file, "LOG %s", actn);
      short at_used;
      at_used = 0;
      if ((log->flags & LbLog_TimeInHeader) != 0)
      {
        struct TbTime curr_time;
        LbTime(&curr_time);
        fprintf(file, "  @ %02d:%02d:%02d",
            curr_time.Hour,curr_time.Minute,curr_time.Second);
        at_used = 1;
      }
      if ((log->flags & LbLog_DateInHeader) != 0)
      {
        struct TbDate curr_date;
        LbDate(&curr_date);
        const char *sep;
        if ( at_used )
          sep = " ";
        else
          sep = "  @ ";
        fprintf(file," %s%02d-%02d-%d",sep,curr_date.Day,curr_date.Month,curr_date.Year);
      }
      fprintf(file, "\n\n");
    }
    if ((log->flags & LbLog_DateInLines) != 0)
    {
        struct TbDate curr_date;
        LbDate(&curr_date);
        fprintf(file,"%02d-%02d-%d ",curr_date.Day,curr_date.Month,curr_date.Year);
    }
    if ((log->flags & LbLog_TimeInLines) != 0)
    {
        struct TbTime curr_time;
        LbTime(&curr_time);
        fprintf(file, "%02d:%02d:%02d ",
            curr_time.Hour,curr_time.Minute,curr_time.Second);
    }
    if (log->prefix[0] != '\0')
      fprintf(file, log->prefix);
  vfprintf(file, fmt_str, arg);
  log->position = ftell(file);
  fclose(file);
  return 1;
}

int LbLogSetPrefix(struct TbLog *log, const char *prefix)
{
  if (!log->Initialised)
    return -1;
  if (prefix)
  {
    LbStringCopy(log->prefix, prefix, LOG_PREFIX_LEN);
  } else
  {
    LbMemorySet(log->prefix, 0, LOG_PREFIX_LEN);
  }
  return 1;
}

int LbLogSetPrefixFmt(struct TbLog *log, const char *format, ...)
{
  va_list val;
  if (!log->Initialised)
    return -1;
  if (format)
  {
    va_start(val, format);
    vsprintf(log->prefix, format, val);
    va_end(val);
  } else
  {
    LbMemorySet(log->prefix, 0, LOG_PREFIX_LEN);
  }
  return 1;
}

int LbLogSetup(struct TbLog *log, const char *filename, ulong flags)
{
  log->Initialised = false;
  LbMemorySet(log->filename, 0, DISKPATH_SIZE);
  LbMemorySet(log->prefix, 0, LOG_PREFIX_LEN);
  log->Initialised=false;
  log->Created=false;
  log->Suspended=false;
  if (LbStringLength(filename)>DISKPATH_SIZE)
    return -1;
  LbStringCopy(log->filename, filename, DISKPATH_SIZE);
  log->flags = flags;
  log->Initialised = true;
  log->position = 0;
  return 1;
}

int LbLogClose(struct TbLog *log)
{
  if ( !log->Initialised )
    return -1;
  LbMemorySet(log->filename, 0, DISKPATH_SIZE);
  LbMemorySet(log->prefix, 0, LOG_PREFIX_LEN);
  log->flags = 0;
  log->Initialised = false;
  log->Created = false;
  log->Suspended = false;
  log->position = 0;
  return 1;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif

