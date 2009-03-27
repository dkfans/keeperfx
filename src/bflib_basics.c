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
const char *log_file_name=DEFAULT_LOG_FILENAME;

#ifndef __cplusplus
#include "bflib_basinln.h"
#endif

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
  LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
  sprintf(msg_text,"%s This error in '%s' makes the program unable to continue. See '%s' for details.",message,codefile,log_file_name);
  MessageBox(NULL, msg_text, PROGRAM_FULL_NAME, MB_OK | MB_ICONERROR);
  return 0;
}

/******************************************************************************/
short error_log_initialised=false;
struct TbLog error_log;
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
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "Net: ");
    va_list val;
    va_start(val, format);
    int result=LbLog(&error_log, format, val);
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

int LbScriptLog(const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "Script: ");
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

int __fastcall LbErrorLogSetup(const char *directory, const char *filename, uchar flag)
{
  if ( error_log_initialised )
    return -1;
  const char *fixed_fname;
  if ( (filename!=NULL)&&(filename[0]!='\0') )
    fixed_fname = filename;
  else
    fixed_fname = "error.log";
  char log_filename[DISKPATH_SIZE];
  int result;
  int flags;
  if ( LbFileMakeFullPath(true,directory,fixed_fname,log_filename,DISKPATH_SIZE) != 1 )
    return -1;
  flags = (flag==0)+53;
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

int __fastcall LbErrorLogClose()
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
  if ( !log->Initialised )
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
      if ( (!(log->Flags & 4)) || LbFileExists(log->Filename) )
      {
        if ( (log->Flags & 1) && (log->Flags & 4) )
        {
          header = CREATE;
        } else
        if ( (log->Flags & 2) && (log->Flags & 8) )
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
    if ( (log->Created) || !(log->Flags & 1) )
      accmode = "a";
    else
      accmode = "w";
    file = fopen(log->Filename, accmode);
    if ( file==NULL )
    {
      return -1;
    }
    log->Created = true;
    if ( header != NONE )
    {
      if ( need_initial_newline )
        fprintf(file, "\n");
      const char *actn;
      if ( header == CREATE )
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
      if ( log->Flags & 0x20 )
      {
        struct TbTime curr_time;
        LbTime(&curr_time);
        fprintf(file, "  @ %02d:%02d:%02d",
            curr_time.Hour,curr_time.Minute,curr_time.Second);
        at_used = 1;
      }
      if ( log->Flags & 0x10 )
      {
        struct TbDate curr_date;
        LbDate(&curr_date);
        const char *sep;
        if ( at_used )
          sep = " ";
        else
          sep = "  @ ";
        fprintf(file," %s%d-%02d-%d",sep,curr_date.Day,curr_date.Month,curr_date.Year);
      }
      fprintf(file, "\n\n");
    }
    if ( log->Flags & 0x40 )
    {
        struct TbDate curr_date;
        LbDate(&curr_date);
        fprintf(file,"%02d-%02d-%d ",curr_date.Day,curr_date.Month,curr_date.Year);
    }
    if ( log->Flags & 0x80 )
    {
        struct TbTime curr_time;
        LbTime(&curr_time);
        fprintf(file, "%02d:%02d:%02d ",
            curr_time.Hour,curr_time.Minute,curr_time.Second);
    }
    if ( log->Prefix[0] != '\0' )
      fprintf(file, log->Prefix);
    vfprintf(file, fmt_str, arg);
  fclose(file);
  return 1;
}

int __fastcall LbLogSetPrefix(struct TbLog *log, const char *prefix)
{
  if ( !log->Initialised )
    return -1;
  if (prefix)
  {
    LbStringCopy(log->Prefix, prefix, LOG_PREFIX_LEN);
  } else
  {
    LbMemorySet(log->Prefix, 0, LOG_PREFIX_LEN);
  }
  return 1;
}

int __fastcall LbLogSetup(struct TbLog *log, const char *filename, int flags)
{
  log->Initialised = false;
  LbMemorySet(log->Filename, 0, DISKPATH_SIZE);
  LbMemorySet(log->Prefix, 0, LOG_PREFIX_LEN);
  log->Initialised=false;
  log->Created=false;
  log->Suspended=false;
  if (LbStringLength(filename)>DISKPATH_SIZE)
    return -1;
  LbStringCopy(log->Filename, filename, DISKPATH_SIZE);
  log->Flags = flags;
  log->Initialised = true;
  return 1;
}

int __fastcall LbLogClose(struct TbLog *log)
{
  if ( !log->Initialised )
    return -1;
  LbMemorySet(log->Filename, 0, DISKPATH_SIZE);
  LbMemorySet(log->Prefix, 0, LOG_PREFIX_LEN);
  log->Flags=0;
  log->Initialised=false;
  log->Created=false;
  log->Suspended=false;
  return 1;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif

