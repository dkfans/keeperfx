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
#include <SDL3/SDL.h>

#include "post_inc.h"


#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern TbBool emulate_integer_overflow(unsigned short nbits);

// Functions which were previously defined as Inline,
// but redefined for compatibility with both Ansi-C and C++.

/** Return the little-endian longword at p. */
unsigned long llong (unsigned char *p)
{
    unsigned long n = p[3];
    n = (n << 8) + p[2];
    n = (n << 8) + p[1];
    n = (n << 8) + p[0];
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
  long long maximum_value = (1 << (nbits-1)) - 1;
  if (val >= maximum_value)
    return maximum_value;
  if (val <= -maximum_value)
    return -maximum_value;
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
    unsigned long long maximum_value = (1 << (nbits)) - 1;
    if (emulate_integer_overflow(nbits))
        return (val & maximum_value);
    if (val >= maximum_value)
        return maximum_value;
    return val;
}

/******************************************************************************/

/**
 * Appends a string to the end of a buffer.
 * Returns the total length of the resulting string.
 * @param buffer The buffer to append the formatted string to.
 * @param size The size of the buffer.
 * @param str The string to append.
 */
int str_append(char * buffer, int size, const char * str)
{
    const int buffer_length = strlen(buffer);
    const int available = size - buffer_length;
    if (available <= 0) {
        return buffer_length;
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
    const int buffer_length = strlen(buffer);
    const int available = size - buffer_length;
    if (available <= 0) {
        return buffer_length;
    }
    va_list args;
    va_start(args, format);
    vsnprintf(&buffer[buffer_length], available, format, args);
    va_end(args);
    return strlen(buffer);
}

short warning_dialog(const char *codefile,const int ecode,const char *message)
{
  LbWarnLog("In source %s:\n %5d - %s\n",codefile,ecode,message);

  const SDL_MessageBoxButtonData buttons[] = {
        { .flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, .buttonID = 1, .text = "Ignore" },
    { .flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, .buttonID = 0, .text = "Abort" },
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
  char msg_text[2048];
  snprintf(msg_text, sizeof(msg_text), "%s This error in '%s' makes the program unable to continue. See '%s' for details.", message, codefile, log_file_name);
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, PROGRAM_FULL_NAME, msg_text, NULL);
  return 0;
}

struct DebugMessage * debug_messages_head = NULL;
struct DebugMessage ** debug_messages_tail = &debug_messages_head;

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
