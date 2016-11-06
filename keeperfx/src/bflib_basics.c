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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "bflib_datetm.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"

#include <SDL2/SDL_thread.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern TbBool emulate_integer_overflow(unsigned short nbits);

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
    if (emulate_integer_overflow(nbits))
        return (val & max);
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
#define BUFFERED_LOG_LINE_LENGTH 1024
#define BUFFERED_LOG_LINES	1024
short error_log_initialised=false;
struct TbLog error_log;
static SDL_Thread* error_log_process_thread;
static TbBool error_log_write_error;
static TbBool error_log_process_exit;
static SDL_mutex* error_log_process_mutex;
static SDL_cond* error_log_process_cond;
static unsigned long long error_log_write_count; //next to write to
static unsigned long long error_log_read_count; //next to read from
static char error_log_buffer[BUFFERED_LOG_LINES][BUFFERED_LOG_LINE_LENGTH];
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

int LbAiLog(const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "Skirmish AI: ");
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

int LbNaviLog(const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "Navi: ");
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

int LbErrorLogClose(void)
{
    if (!error_log_initialised)
        return -1;

	if (error_log_process_thread)
	{
		SDL_LockMutex(error_log_process_mutex);
		error_log_process_exit = 1;
		SDL_UnlockMutex(error_log_process_mutex);
		SDL_CondBroadcast(error_log_process_cond);

		SDL_WaitThread(error_log_process_thread, NULL);

		SDL_DestroyMutex(error_log_process_mutex);
		SDL_DestroyCond(error_log_process_cond);
		error_log_process_thread = NULL;
		error_log_process_exit = 0;
		error_log_write_error = 0;
	}

    return LbLogClose(&error_log);
}

static void ErrorLogFlushToFile(void)
{
	enum Header {
		NONE   = 0,
		CREATE = 1,
		APPEND = 2,
	};

	char line_buffer[BUFFERED_LOG_LINE_LENGTH];
	FILE *file;
	short need_initial_newline;
	char header;
	unsigned read_index;
	struct TbLog* log;

	SDL_UnlockMutex(error_log_process_mutex);

	//file access follows
	log = &error_log;
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
		error_log_write_error = 1;
		SDL_CondBroadcast(error_log_process_cond);
		goto exit_with_mutex;
	}
	log->Created = true;
	if (header != NONE)
	{
		if ( need_initial_newline )
			fprintf(file, "\n");
		const char *actn;
		if (header == CREATE)
		{
			fprintf(file, PROGRAM_NAME" ver "VER_STRING" (%s release)\n", (BFDEBUG_LEVEL>7)?"heavylog":"standard");
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

	SDL_LockMutex(error_log_process_mutex);
	while (error_log_read_count < error_log_write_count)
	{
		//copy next log line and relinquish mutex to avoid locking up writer (application)
		read_index = error_log_read_count % BUFFERED_LOG_LINES;
		LbStringCopy(line_buffer, error_log_buffer[read_index], BUFFERED_LOG_LINE_LENGTH);
		error_log_read_count += 1;
		SDL_UnlockMutex(error_log_process_mutex);
		SDL_CondBroadcast(error_log_process_cond); //writer might be stalled on full buffer

		fputs(line_buffer, file);

		SDL_LockMutex(error_log_process_mutex);
	}

	SDL_UnlockMutex(error_log_process_mutex);
	log->position = ftell(file);
	fclose(file);

	//reacquire mutex before exiting
exit_with_mutex:
	SDL_LockMutex(error_log_process_mutex);
}

static int ErrorLogProcess(void * dummy)
{
	SDL_LockMutex(error_log_process_mutex);

	while (!error_log_process_exit && !error_log_write_error)
	{
		while (!error_log_process_exit && error_log_write_count == error_log_read_count)
			SDL_CondWait(error_log_process_cond, error_log_process_mutex);

		if (!error_log_process_exit)
			ErrorLogFlushToFile();
	}

	SDL_UnlockMutex(error_log_process_mutex);

	return 0; //don't care
}

void LbErrorLogDetach(void)
{
	if (!error_log_initialised)
		return;
	if (error_log_process_thread)
		return;

	error_log_process_mutex = SDL_CreateMutex();
	error_log_process_cond = SDL_CreateCond();
	error_log_process_thread = SDL_CreateThread(ErrorLogProcess, "Error log processing thread", (void*)NULL);
}

static int ErrorLogDeferredWrite(struct TbLog* log, const char* fmt_str, va_list arg)
{
	char line_buffer[BUFFERED_LOG_LINE_LENGTH];
	unsigned write_index;
	unsigned line_index;
	int retval;
	int n;
	retval = 1;
	line_index = 0;
	
	if ((log->flags & LbLog_DateInLines) != 0 && line_index < BUFFERED_LOG_LINE_LENGTH)
	{
		struct TbDate curr_date;
		LbDate(&curr_date);
		n = snprintf(line_buffer + line_index, BUFFERED_LOG_LINE_LENGTH - line_index, "%02d-%02d-%d ",curr_date.Day,curr_date.Month,curr_date.Year);
		if (n > 0) line_index += n;
	}
	if ((log->flags & LbLog_TimeInLines) != 0 && line_index < BUFFERED_LOG_LINE_LENGTH)
	{
		struct TbTime curr_time;
		LbTime(&curr_time);
		n = snprintf(line_buffer + line_index, BUFFERED_LOG_LINE_LENGTH - line_index, "%02d:%02d:%02d ",
			curr_time.Hour,curr_time.Minute,curr_time.Second);
		if (n > 0) line_index += n;
	}
	if (log->prefix[0] != '\0' && line_index < BUFFERED_LOG_LINE_LENGTH)
	{
		n = snprintf(line_buffer + line_index, BUFFERED_LOG_LINE_LENGTH - line_index, log->prefix);
		if (n > 0) line_index += n;
	}

	if (line_index < BUFFERED_LOG_LINE_LENGTH)
	{
		n = vsnprintf(line_buffer + line_index, BUFFERED_LOG_LINE_LENGTH - line_index, fmt_str, arg);
		if (n > 0) line_index += n;
	}
	
	//locked write back
	SDL_LockMutex(error_log_process_mutex);
	while (!error_log_write_error && error_log_write_count >= BUFFERED_LOG_LINES + error_log_read_count)
		SDL_CondWait(error_log_process_cond, error_log_process_mutex);

	if (error_log_write_error)
		retval = -1;
	else
	{
		write_index = error_log_write_count % BUFFERED_LOG_LINES;
		LbStringCopy(error_log_buffer[write_index], line_buffer, BUFFERED_LOG_LINE_LENGTH);
		error_log_write_count += 1;
	}

	SDL_UnlockMutex(error_log_process_mutex);
	SDL_CondBroadcast(error_log_process_cond); //reader may be stalled on empty buffer
	return retval;
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
  if ( log->Suspended )
	  return 1;
  if (log == &error_log && error_log_process_thread)
	  return ErrorLogDeferredWrite(log, fmt_str, arg);
  FILE *file;
  short need_initial_newline;
  char header;
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
        fprintf(file, PROGRAM_NAME" ver "VER_STRING" (%s release)\n", (BFDEBUG_LEVEL>7)?"heavylog":"standard");
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

