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
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "post_inc.h"


char consoleLogArray[MAX_CONSOLE_LOG_COUNT][MAX_TEXT_LENGTH];
size_t consoleLogArraySize = 0;
int debug_display_consolelog = 0;
FILE * logfile = NULL;

int log_levels[] = {
  LOG_INFO, // LOG_GENERAL - LOG_INFO, LOG_WARNING and LOG_ERROR
  LOG_ERROR, // LOG_NET - LOG_ERROR only
  LOG_ERROR, // LOG_AI - LOG_ERROR only
  LOG_ERROR, // LOG_NAV - LOG_ERROR only
  LOG_ERROR, // LOG_TEST - LOG_ERROR only
  LOG_ERROR, // LOG_SCRIPT - LOG_ERROR only
  LOG_ERROR, // LOG_CONFIG - LOG_ERROR only
};
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

char *buf_sprintf(const char *format, ...)
{
    va_list val;
    va_start(val, format);
    static char text[TEXT_BUFFER_LENGTH + 1];
    vsnprintf(text, sizeof(text), format, val);
    va_end(val);
    return text;
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

int Lbvsprintf(char* buffer, const char *format, ...)
{
    va_list val;
    va_start(val, format);
    int result=vsprintf(buffer, format, val);
    va_end(val);
    return result;
}

TbBool LbLogOpen(const char *filename)
{
  LbLogClose();
  logfile = fopen(filename, "w");
  if (logfile) {
    fprintf(logfile, PROGRAM_NAME" ver "VER_STRING" git:%s", GIT_REVISION);
    struct TbDate curr_date;
    struct TbTime curr_time;
    if (LbDateTime(&curr_date, &curr_time) == Lb_SUCCESS)
    {
        fprintf(logfile, " @ %02u:%02u:%02u %02u-%02u-%u",
            curr_time.Hour, curr_time.Minute, curr_time.Second,
            curr_date.Day, curr_date.Month, curr_date.Year
        );
    }
    fprintf(logfile, "\n\n");
    return true;
  }
  return false;
}

void LbLogClose(void)
{
  if (logfile) {
    fclose(logfile);
    logfile = NULL;
  }
}

void LbLog(int chan, int level, const char * format, ...) {
  if (chan >= sizeof(log_levels) / sizeof(*log_levels)) {
    return; // invalid channel number
  } else if (level < log_levels[chan]) {
    return; // ignore
  } else if (!logfile) {
    return; // cannot log
  }
  va_list args;
  va_start(args, format);
  vfprintf(logfile, format, args);
  va_end(args);
  fflush(logfile);
}

void LbSetLogLevel(int chan, int level) {
  if (chan >= sizeof(log_levels) / sizeof(*log_levels)) {
    return; // invalid channel number
  }
  log_levels[chan] = level;
}

void LbLogConsole(const char * format, ...) {
    va_list args;

    if (consoleLogArraySize >= MAX_CONSOLE_LOG_COUNT) {
        // Array is full - so clear it. This is a bit of a stopgap solution, it will lose us the older entries.
        memset(consoleLogArray, 0, sizeof(consoleLogArray));
        consoleLogArraySize = 0;
    }

    char buffer[MAX_TEXT_LENGTH];
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Add the combined message to the array
    strncpy(consoleLogArray[consoleLogArraySize], buffer, MAX_TEXT_LENGTH);
    consoleLogArray[consoleLogArraySize][MAX_TEXT_LENGTH - 1] = '\0';
    consoleLogArraySize++;
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
