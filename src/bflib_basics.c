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
#include "pre_inc.h"
#include "bflib_basics.h"
#include "globals.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <SDL2/SDL.h>

#include "bflib_datetm.h"
#include "bflib_fileio.h"
#include "post_inc.h"


char consoleLogArray[MAX_CONSOLE_LOG_COUNT][MAX_TEXT_LENGTH];
size_t consoleLogArraySize = 0;
int debug_display_consolelog = 0;

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
    unsigned long n = p[0];
    n = (n << 8) + p[1];
    n = (n << 8) + p[2];
    n = (n << 8) + p[3];
    return n;
}

/** Return the little-endian longword at p. */
unsigned long llong (unsigned char *p)
{
    unsigned long n = p[3];
    n = (n << 8) + p[2];
    n = (n << 8) + p[1];
    n = (n << 8) + p[0];
    return n;
}

/** Return the big-endian word at p. */
unsigned long bword (unsigned char *p)
{
    unsigned long n = p[0];
    n = (n << 8) + p[1];
    return n;
}

/* Return the little-endian word at p. */
unsigned long lword (unsigned char *p)
{
    unsigned long n = p[1];
    n = (n << 8) + p[0];
    return n;
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

/**
 * Appends a string to the end of a buffer.
 * Returns the total length of the resulting string.
 * @param buffer The buffer to append the formatted string to.
 * @param size The size of the buffer.
 * @param str The string to append.
 */
int str_append(char * buffer, int size, const char * str)
{
    const int len = strlen(buffer);
    const int available = size - len;
    if (available <= 0) {
        return len;
    }
    strncat(buffer, str, available);
    return strlen(buffer);
}

/**
 * Appends a formatted string to the end of a buffer.
 * Returns the total length of the resulting string.
 * @param buffer The buffer to append the formatted string to.
 * @param size The size of the buffer.
 * @param format The format string, similar to printf.
 * @param ... The values to format and append to the buffer.
 */
int str_appendf(char * buffer, int size, const char * format, ...)
{
    const int len = strlen(buffer);
    const int available = size - len;
    if (available <= 0) {
        return len;
    }
    va_list args;
    va_start(args, format);
    vsnprintf(&buffer[len], available, format, args);
    va_end(args);
    return strlen(buffer);
}

void error(const char *codefile,const int ecode,const char *message)
{
  LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
}

short warning_dialog(const char *codefile,const int ecode,const char *message)
{
  LbWarnLog("In source %s:\n %5d - %s\n",codefile,ecode,message);

  const SDL_MessageBoxButtonData buttons[] = {
        { .flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, .buttonid = 1, .text = "Ignore" },
    { .flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, .buttonid = 0, .text = "Abort" },
    };

    const SDL_MessageBoxData messageboxdata = {
        .flags = SDL_MESSAGEBOX_WARNING,
        .window = NULL,
        .title = PROGRAM_FULL_NAME,
        .message = message,
        .numbuttons = SDL_arraysize(buttons),
        .buttons = buttons,
        .colorScheme = NULL //colorScheme not supported on windows
    };

  int button = 0;
  SDL_ShowMessageBox(&messageboxdata, &button);
  return button;
}

short error_dialog(const char *codefile,const int ecode,const char *message)
{
  LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, PROGRAM_FULL_NAME, message, NULL);
  return 0;
}

short error_dialog_fatal(const char *codefile,const int ecode,const char *message)
{
  LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
  static char msg_text[2048];
  sprintf(msg_text, "%s This error in '%s' makes the program unable to continue. See '%s' for details.", message, codefile, log_file_name);
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, PROGRAM_FULL_NAME, msg_text, NULL);
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

int Lbvsprintf(char* buffer, const char *format, ...)
{
    va_list val;
    va_start(val, format);
    int result=vsprintf(buffer, format, val);
    va_end(val);
    return result;
}

#ifdef FUNCTESTING
int LbFTestLog(const char *format, ...)
{
    if (!error_log_initialised)
        return -1;
    LbLogSetPrefix(&error_log, "FTest: ");
    va_list val;
    va_start(val, format);
    int result=LbLog(&error_log, format, val);
    va_end(val);
    return result;
}
#endif

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
  if ( LbFileMakeFullPath(true,directory,fixed_fname,log_filename,DISKPATH_SIZE) != 1 )
    return -1;
  ulong flags = (flag == 0) + 1;
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
    return LbLogClose(&error_log);
}

FILE *file = NULL;

void LbCloseLog()
{
    fclose(file);
    file = NULL;
}

void write_log_to_array_for_live_viewing(const char* fmt_str, va_list args, const char* add_log_prefix) {
    if (consoleLogArraySize >= MAX_CONSOLE_LOG_COUNT) {
        // Array is full - so clear it. This is a bit of a stopgap solution, it will lose us the older entries.
        memset(consoleLogArray, 0, sizeof(consoleLogArray));
        consoleLogArraySize = 0;
    }

    char formattedString[MAX_TEXT_LENGTH];
    vsnprintf(formattedString, sizeof(formattedString), fmt_str, args);

    char buffer[MAX_TEXT_LENGTH];
    snprintf(buffer, sizeof(buffer), "%s%s", add_log_prefix, formattedString); // merge prefix and formatted string

    // Add the combined message to the array
    strncpy(consoleLogArray[consoleLogArraySize], buffer, MAX_TEXT_LENGTH);
    consoleLogArray[consoleLogArraySize][MAX_TEXT_LENGTH - 1] = '\0';
    consoleLogArraySize++;
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
  char header = NONE;
  short need_initial_newline = false;
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
    // Only load log if it's not already open
    if (file == NULL)
    {
      file = fopen(log->filename, accmode);
      // Couldn't open. Abort
      if (file == NULL)
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
        fprintf(file, PROGRAM_NAME" ver "VER_STRING" (%s release) git:%s\n", (BFDEBUG_LEVEL>7)?"heavylog":"standard", GIT_REVISION);
        actn = "CREATED";
      } else
      {
        actn = "APPENDED";
      }
      fprintf(file, "LOG %s", actn);
      short at_used = 0;
      if ((log->flags & LbLog_TimeInHeader) != 0)
      {
        struct TbTime curr_time;
        if (LbTime(&curr_time) == Lb_SUCCESS)
        {
            fprintf(file, "  @ %02u:%02u:%02u",
                curr_time.Hour,curr_time.Minute,curr_time.Second);
            at_used = 1;
        }
      }
      if ((log->flags & LbLog_DateInHeader) != 0)
      {
        struct TbDate curr_date;
        if (LbDate(&curr_date) == Lb_SUCCESS)
        {
            const char *sep;
            if ( at_used )
              sep = " ";
            else
              sep = "  @ ";
            fprintf(file," %s%02u-%02u-%u",sep,curr_date.Day,curr_date.Month,curr_date.Year);
        }
      }
      fprintf(file, "\n\n");
    }
    if ((log->flags & LbLog_DateInLines) != 0)
    {
        struct TbDate curr_date;
        if (LbDate(&curr_date) == Lb_SUCCESS)
        {
            fprintf(file,"%02u-%02u-%u ",curr_date.Day,curr_date.Month,curr_date.Year);
        }
    }
    if ((log->flags & LbLog_TimeInLines) != 0)
    {
        struct TbTime curr_time;
        if (LbTime(&curr_time) == Lb_SUCCESS)
        {
            fprintf(file, "%02u:%02u:%02u ",
                curr_time.Hour,curr_time.Minute,curr_time.Second);
        }
    }
  if (log->prefix[0] != '\0') {
      fputs(log->prefix, file);
  }

  // Write formatted message to the array
  write_log_to_array_for_live_viewing(fmt_str, arg, log->prefix);

  vfprintf(file, fmt_str, arg);
  log->position = ftell(file);
  // fclose is slow and automatically happens on normal program exit.
  // Opening/closing every time we log something hits performance hard.
  // fclose(file);
  fflush(file);
  return 1;
}

int LbLogSetPrefix(struct TbLog *log, const char *prefix)
{
  if (!log->Initialised)
    return -1;
  if (prefix)
  {
    snprintf(log->prefix, LOG_PREFIX_LEN, "%s", prefix);
  } else
  {
    memset(log->prefix, 0, LOG_PREFIX_LEN);
  }
  return 1;
}

int LbLogSetPrefixFmt(struct TbLog *log, const char *format, ...)
{
  if (!log->Initialised)
    return -1;
  if (format)
  {
      va_list val;
      va_start(val, format);
      vsprintf(log->prefix, format, val);
      va_end(val);
  } else
  {
    memset(log->prefix, 0, LOG_PREFIX_LEN);
  }
  return 1;
}

int LbLogSetup(struct TbLog *log, const char *filename, ulong flags)
{
  log->Initialised = false;
  memset(log->filename, 0, DISKPATH_SIZE);
  memset(log->prefix, 0, LOG_PREFIX_LEN);
  log->Initialised=false;
  log->Created=false;
  log->Suspended=false;
  if (filename == NULL || strlen(filename) > DISKPATH_SIZE) {
    return -1;
  }
  snprintf(log->filename, DISKPATH_SIZE, "%s", filename);
  log->flags = flags;
  log->Initialised = true;
  log->position = 0;
  return 1;
}

int LbLogClose(struct TbLog *log)
{
  if ( !log->Initialised )
    return -1;
  memset(log->filename, 0, DISKPATH_SIZE);
  memset(log->prefix, 0, LOG_PREFIX_LEN);
  log->flags = 0;
  log->Initialised = false;
  log->Created = false;
  log->Suspended = false;
  log->position = 0;
  return 1;
}

struct DebugMessage * debug_messages_head = NULL;
struct DebugMessage ** debug_messages_tail = &debug_messages_head;

void LbPrint(const char * format, ...) {
  va_list args;
  va_start(args, format);
  const int message_length = vsnprintf(NULL, 0, format, args);
  va_end(args);
  if (message_length <= 0) {
    return;
  }
  const int message_size = message_length + 1;
  const int block_size = sizeof(struct DebugMessage) + message_size;
  struct DebugMessage * message = malloc(block_size);
  if (message == NULL) {
    return;
  }
  va_start(args, format);
  vsnprintf(message->text, message_size, format, args);
  va_end(args);
  message->next = NULL;
  *debug_messages_tail = message;
  debug_messages_tail = &message->next;
}

void make_lowercase(char * string) {
  for (char * ptr = string; *ptr != 0; ++ptr) {
    *ptr = tolower(*ptr);
  }
}

void make_uppercase(char * string) {
  for (char * ptr = string; *ptr != 0; ++ptr) {
    *ptr = toupper(*ptr);
  }
}

int natoi(const char * str, int len) {
  int value = -1;
  for (int i = 0; i < len; ++i) {
    if (!isdigit(str[i])) {
      return value;
    } else if (value < 0) {
      value = 0;
    }
    value = (value * 10) + (str[i] - '0');
  }
  return value;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
