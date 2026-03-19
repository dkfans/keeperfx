/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config.c
 *     Configuration and campaign files support.
 * @par Purpose:
 *     loading of CFG files.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     30 Jan 2009 - 11 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config.h"

#include <stdarg.h>
#include <inttypes.h>
#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_datetm.h"
#include "bflib_mouse.h"
#include "bflib_sound.h"
#include "sounds.h"
#include "engine_render.h"
#include "bflib_fmvids.h"
#include "custom_sprites.h"
#include "lvl_script_lib.h"

#include "config_campaigns.h"
#include "config_keeperfx.h"
#include "front_simple.h"
#include "scrcapt.h"
#include "vidmode.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/** Line number, used when loading text files. */
unsigned long text_line_number;


/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/

const struct NamedCommand logicval_type[] = {
  {"ENABLED",  1},
  {"DISABLED", 2},
  {"ON",       1},
  {"OFF",      2},
  {"TRUE",     1},
  {"FALSE",    2},
  {"YES",      1},
  {"NO",       2},
  {"1",        1},
  {"0",        2},
  {NULL,       0},
};

TbBool parameter_is_number(const char* parstr) {
    if (parstr == NULL) {
        return false;
    }

    // Trim leading spaces
    while (*parstr == ' ') {
        parstr++;
    }

    // Trim trailing spaces
    int len = strlen(parstr);
    while (len > 0 && parstr[len - 1] == ' ') {
        len--;
    }

    if (len == 0) {
        return false;
    }

    // Check if the first character is a valid start for a number
    if (!(parstr[0] == '-' || isdigit(parstr[0]))) {
        return false;
    }

    // Check the remaining characters
    for (int i = 1; i < len; ++i) {
        if (!isdigit(parstr[i])) {
            return false;
        }
    }

    return true;
}

int get_conf_line(const char *buf, int32_t *pos, long buflen, char *dst, long dstlen)
{
    SYNCDBG(19,"Starting");
    if ((*pos) >= buflen) return ccr_endOfFile;
    // Skipping starting spaces
    while ((buf[*pos] == ' ') || (buf[*pos] == '\t') || (buf[*pos] == '\n') || (buf[*pos] == '\r') || (buf[*pos] == 26) || ((unsigned char)buf[*pos] < 7))
    {
        (*pos)++;
        if ((*pos) >= buflen) return ccr_endOfFile;
    }
    // Checking if this line is a comment
    if (buf[*pos] == ';')
        return ccr_comment;
    // Checking if this line is start of a block
    if (buf[*pos] == '[')
        return ccr_endOfBlock;
    int i = 0;
    for (i=0; i+1 < dstlen; i++)
    {
        if ((buf[*pos]=='\r') || (buf[*pos]=='\n') || ((unsigned char)buf[*pos] < 7))
            break;
        dst[i]=buf[*pos];
        (*pos)++;
        if ((*pos) > buflen) break;
    }
    // Trim ending spaces
    for (; i>0; i--)
    {
        if ( (dst[i-1] != ' ') && (dst[i-1] != '\t') && (dst[i-1] != 26) )
            break;
    }
    dst[i]='\0';
    return i;
}


TbBool skip_conf_to_next_line(const char *buf,int32_t *pos,long buflen)
{
  // Skip to end of the line
  while ((*pos) < buflen)
  {
    if ((buf[*pos]=='\r') || (buf[*pos]=='\n')) break;
    (*pos)++;
  }
  // Go to start of next line
  while ((*pos) < buflen)
  {
    if ((unsigned char)buf[*pos] > 32) break;
    if (buf[*pos]=='\n')
      text_line_number++;
    (*pos)++;
  }
  return ((*pos) < buflen);
}

TbBool skip_conf_spaces(const char *buf, int32_t *pos, long buflen)
{
  while ((*pos) < buflen)
  {
    if ((buf[*pos]!=' ') && (buf[*pos]!='\t') && (buf[*pos] != 26) && ((unsigned char)buf[*pos] >= 7)) break;
    (*pos)++;
  }
  return ((*pos) < buflen);
}

/**
 * Searches for start of INI file block with given name.
 * Starts at position given with pos, and sets it to position of block data.
 * @return Returns 1 if the block is found, -1 if buffer exceeded.
 */
short find_conf_block(const char *buf,int32_t *pos,long buflen,const char *blockname)
{
  text_line_number = 1;
  int blname_len = strlen(blockname);
  while ((*pos)+blname_len+2 < buflen)
  {
    // Skipping starting spaces
    if (!skip_conf_spaces(buf,pos,buflen))
      break;
    // Checking if this line is start of a block
    if (buf[*pos] != '[')
    {
      skip_conf_to_next_line(buf,pos,buflen);
      continue;
    }
    (*pos)++;
    // Skipping any spaces
    if (!skip_conf_spaces(buf,pos,buflen))
      break;
    if ((*pos)+blname_len+2 >= buflen)
      break;
    if (strncasecmp(&buf[*pos],blockname,blname_len) != 0)
    {
      skip_conf_to_next_line(buf,pos,buflen);
      continue;
    }
    (*pos)+=blname_len;
    // Skipping any spaces
    if (!skip_conf_spaces(buf,pos,buflen))
      break;
    if (buf[*pos] != ']')
    {
      skip_conf_to_next_line(buf,pos,buflen);
      continue;
    }
    skip_conf_to_next_line(buf,pos,buflen);
    return 1;
  }
  return -1;
}

/**
 * Reads the block name from buf, starting at pos.
 * Sets name and namelen to the block name and name length respectively.
 * Returns true on success, false when the block name is zero.
 */
TbBool conf_get_block_name(const char * buf, int32_t * pos, long buflen, const char ** name, int * namelen)
{
  const long start = *pos;
  *name = NULL;
  *namelen = 0;
  while (true) {
    if (*pos >= buflen) {
      return false;
    } else if (isalpha(buf[*pos])) {
      (*pos)++;
      continue;
    } else if (isdigit(buf[*pos])) {
      (*pos)++;
      continue;
    } else {
      if (*pos - start > 0) {
        *name = &buf[start];
        *namelen = *pos - start;
        return true;
      } else {
        return false;
      }
    }
  }
}

/**
 * Searches for the next block in buf, starting at pos.
 * Sets name and namelen to the block name and name length respectively.
 * Returns true on success, false when no more blocks are found.
 */
TbBool iterate_conf_blocks(const char * buf, int32_t * pos, long buflen, const char ** name, int * namelen)
{
  text_line_number = 1;
  *name = NULL;
  *namelen = 0;
  while (true) {
    // Skip whitespace before block start
    if (!skip_conf_spaces(buf, pos, buflen)) {
      return false;
    }
    // Check if this line is start of a block
    if (*pos >= buflen) {
      return false;
    } else if (buf[*pos] != '[') {
      skip_conf_to_next_line(buf, pos, buflen);
      continue;
    }
    (*pos)++;
    // Skip whitespace before block name
    if (!skip_conf_spaces(buf, pos, buflen)) {
      return false;
    }
    // Get block name
    if (!conf_get_block_name(buf, pos, buflen, name, namelen)) {
      skip_conf_to_next_line(buf, pos, buflen);
      return false;
    }
    // Skip whitespace after block name
    if (!skip_conf_spaces(buf, pos, buflen)) {
      return false;
    } else if (buf[*pos] != ']') {
      skip_conf_to_next_line(buf, pos, buflen);
      continue;
    }
    skip_conf_to_next_line(buf,pos,buflen);
    return true;
  }
}

/**
 * Recognizes config command and returns its number, or negative status code.
 * The string comparison is done by case-insensitive.
 * @param buf
 * @param pos
 * @param buflen
 * @param commands
 * @return If positive integer is returned, it is the command number recognized in the line.
 * If ccr_comment      is returned, that means the current line did not contained any command and should be skipped.
 * If ccr_endOfFile    is returned, that means we've reached end of file.
 * If ccr_unrecognised is returned, that means the command wasn't recognized.
 * If ccr_endOfBlock   is returned, that means we've reached end of the INI block.
 */
int recognize_conf_command(const char *buf,int32_t *pos,long buflen,const struct NamedCommand commands[])
{
    SYNCDBG(19,"Starting");
    if ((*pos) >= buflen) return ccr_endOfFile;
    // Skipping starting spaces
    while ((buf[*pos] == ' ') || (buf[*pos] == '\t') || (buf[*pos] == '\n') || (buf[*pos] == '\r') || (buf[*pos] == 26) || ((unsigned char)buf[*pos] < 7))
    {
        (*pos)++;
        if ((*pos) >= buflen) return ccr_endOfFile;
    }
    // Checking if this line is a comment
    if (buf[*pos] == ';')
        return ccr_comment;
    // Checking if this line is start of a block
    if (buf[*pos] == '[')
        return ccr_endOfBlock;
    // Finding command number
    int i = 0;
    while (commands[i].num > 0)
    {
        int cmdname_len = strlen(commands[i].name);
        if ((*pos)+cmdname_len > buflen) {
            i++;
            continue;
        }
        // Find a matching command
        if (strnicmp(buf+(*pos), commands[i].name, cmdname_len) == 0)
        {
            (*pos) += cmdname_len;
            // if we're not at end of input buffer..
            if ((*pos) < buflen)
            {
                // make sure it's whole command, not just start of different one
               if ((buf[(*pos)] != ' ') && (buf[(*pos)] != '\t')
                && (buf[(*pos)] != '=')  && ((unsigned char)buf[(*pos)] >= 7))
               {
                  (*pos) -= cmdname_len;
                  i++;
                  continue;
               }
               // Skipping spaces between command and parameters
               while ((buf[*pos] == ' ') || (buf[*pos] == '\t')
                || (buf[*pos] == '=')  || ((unsigned char)buf[*pos] < 7))
               {
                 (*pos)++;
                 if ((*pos) >= buflen) break;
               }
            }
            return commands[i].num;
        }
        i++;
    }
    const int len = strcspn(&buf[(*pos)], " \n\r\t");
    CONFWRNLOG("Unrecognized command '%.*s'", len, &buf[(*pos)]);
    return ccr_unrecognised;
}

static int64_t get_datatype_min(uchar type)
{
    switch (type)
    {
        case dt_uchar:
            return 0;
        case dt_schar:
            return SCHAR_MIN;
        case dt_char:
            return CHAR_MIN;
        case dt_short:
            return SHRT_MIN;
        case dt_ushort:
            return 0;
        case dt_int:
            return INT_MIN;
        case dt_uint:
            return 0;
        case dt_long:
            return INT32_MIN;
        case dt_ulong:
            return 0;
        case dt_longlong:
            return INT64_MIN;
        case dt_ulonglong:
            return 0;
        default:
            ERRORLOG("unexpected datatype %d", type);
            break;
    }
    return 0;
}

static int64_t get_datatype_max(uchar type)
{
    switch (type)
    {
        case dt_uchar:
            return UCHAR_MAX;
        case dt_schar:
            return SCHAR_MAX;
        case dt_char:
            return CHAR_MAX;
        case dt_short:
            return SHRT_MAX;
        case dt_ushort:
            return USHRT_MAX;
        case dt_int:
            return INT_MAX;
        case dt_uint:
            return UINT_MAX;
        case dt_long:
            return INT32_MAX;
        case dt_ulong:
            return UINT32_MAX;
        case dt_longlong:
            return INT64_MAX;
        case dt_ulonglong:
            return UINT64_MAX;
        default:
            break;
    }
    return 0;
}

//if the parameter is a number return the number, if a value in the provided NamedCommand list return the value
int64_t value_default(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    if (parameter_is_number(value_text))
    {
        int64_t value = atoll(value_text);
        int64_t minimum = max(named_field->min, get_datatype_min(named_field->type));
        int64_t maximum = min(named_field->max, get_datatype_max(named_field->type));
        if( value < minimum)
        {
            NAMFIELDWRNLOG("field '%s' smaller than min value '%" PRId64 "', was '%" PRId64 "'",named_field->name,minimum,value);
            value = minimum;
        }
        else if( value > maximum)
        {
            NAMFIELDWRNLOG("field '%s' greater than max value '%" PRId64 "', was '%" PRId64 "'",named_field->name,maximum,value);
            value = maximum;
        }
        return value;

    }
    else if(named_field->namedCommand != NULL)
    {
        int64_t value = get_id(named_field->namedCommand, value_text);
        if(value >= 0)
        {
            return value;
        }
        NAMFIELDWRNLOG("Unrecognized parameter for field '%s', got '%s'",named_field->name,value_text);
    }
    else
    {
        NAMFIELDWRNLOG("Expected number for field '%s', got '%s'",named_field->name,value_text);
    }
    return 0;
}

int64_t value_name(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    size_t offset = named_fields_set->struct_size * idx;
    strncpy((char*)named_field->field + offset, value_text, COMMAND_WORD_LEN - 1);
    ((char*)named_field->field + offset)[COMMAND_WORD_LEN - 1] = '\0';
    return 0;
}

//same as value_flagsfield but treats the namedCommand field as a longnamedCommand
int64_t value_longflagsfield(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    int64_t value = 0;
    char word_buf[COMMAND_WORD_LEN];
    if (parameter_is_number(value_text))
    {
        return atoll(value_text);
    }
    if(strcasecmp(value_text,"none") == 0)
    {
        return 0;
    }

    int32_t pos = 0;
    long len = strlen(value_text);
    int i = 0;
    while (get_conf_parameter_single(value_text,&pos,len,word_buf,sizeof(word_buf)) > 0)
    {
        if (i == 1)
        {
            //if the second value is 0 or 1, treat it as a flag toggle
            if(strcmp(word_buf, "0") == 0 || strcmp(word_buf, "1") == 0)
            {
                int64_t original_value = get_named_field_value(named_field, named_fields_set, idx);
                set_flag_value(original_value,value, atoi(word_buf));
                return original_value;
            }
        }

        int64_t k = get_long_id((struct LongNamedCommand*)named_field->namedCommand, word_buf);
        if(k >= 0)
            value |= k;
        else
            NAMFIELDWRNLOG("Unexpected value for field '%s', got '%s'",named_field->name,word_buf);
        i++;
    }
    return value;
}


//expects value_text to be a space seperated list of values in the named fields named command, wich can be combined with bitwise or
int64_t value_flagsfield(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    int64_t value = 0;
    char word_buf[COMMAND_WORD_LEN];
    if (parameter_is_number(value_text))
    {
        return atoll(value_text);
    }
    if(strcasecmp(value_text,"none") == 0)
    {
        return 0;
    }

    int32_t pos = 0;
    long len = strlen(value_text);
    int i = 0;
    while (get_conf_parameter_single(value_text,&pos,len,word_buf,sizeof(word_buf)) > 0)
    {
        if (i == 1)
        {
            //if the second value is 0 or 1, treat it as a flag toggle
            if(strcmp(word_buf, "0") == 0 || strcmp(word_buf, "1") == 0)
            {
                int64_t original_value = get_named_field_value(named_field, named_fields_set, idx);
                set_flag_value(original_value,value, atoi(word_buf));
                return original_value;
            }
        }

        int k = get_id(named_field->namedCommand, word_buf);
        if(k >= 0)
            value |= k;
        else
            NAMFIELDWRNLOG("Unexpected value for field '%s', got '%s'",named_field->name,word_buf);
        i++;
    }
    return value;
}

int64_t value_icon(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    if (flag_is_set(flags,ccf_SplitExecution))
    {
        int64_t script_string_offset = script_strdup(value_text);
        if (script_string_offset < 0)
        {
            NAMFIELDWRNLOG("Run out script strings space");
            return -1;
        }
        return script_string_offset;
    }
    else
    {
        return get_icon_id(value_text);
    }
}

int64_t value_animid(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
  if (flag_is_set(flags,ccf_SplitExecution))
  {
      int64_t script_string_offset = script_strdup(value_text);
      if (script_string_offset < 0)
      {
          NAMFIELDWRNLOG("Run out script strings space");
          return -1;
      }
      return script_string_offset;
  }
  else
  {
      return get_anim_id_(value_text);
  }
}

int64_t value_effOrEffEl(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    return effect_or_effect_element_id(value_text);
}

int64_t value_function(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    return get_function_idx(value_text, named_field->namedCommand);
}

void assign_icon(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    if (flag_is_set(flags,ccf_SplitExecution))
    {
        short icon_id = get_icon_id(script_strval(value));
        assign_default(named_field,icon_id,named_fields_set,idx,src_str,flags);
    }
    else
    {
        assign_default(named_field,value,named_fields_set,idx,src_str,flags);
    }
}

void assign_animid(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    if (flag_is_set(flags,ccf_SplitExecution))
    {
        short anim_id = get_anim_id_(script_strval(value));
        assign_default(named_field,anim_id,named_fields_set,idx,src_str,flags);
    }
    else
    {
        assign_default(named_field,value,named_fields_set,idx,src_str,flags);
    }
}

int64_t value_transpflg(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{

    if (parameter_is_number(value_text))
    {
        return atoll(value_text) << 4;
    }
    else
    {
        NAMFIELDWRNLOG("Expected number for field '%s', got '%s'",named_field->name,value_text);
    }
    return 0;
}

int64_t value_stltocoord(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{

    if (parameter_is_number(value_text))
    {
        return atoll(value_text) * COORD_PER_STL;
    }
    else
    {
        NAMFIELDWRNLOG("Expected number for field '%s', got '%s'",named_field->name,value_text);
    }
    return 0;
}

int64_t parse_named_field_value(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    if (named_field->parse_func != NULL)
      return named_field->parse_func(named_field,value_text,named_fields_set,idx,src_str,flags);
    else
        NAMFIELDWRNLOG("No parse_func for field %s",named_field->name);
    return 0;
}

int64_t get_named_field_value(const struct NamedField* named_field, const struct NamedFieldSet* named_fields_set, int idx)
{
    void* field = (char*)named_field->field + named_fields_set->struct_size * idx;
    switch (named_field->type)
    {
    case dt_uchar:
        return *(unsigned char*)field;
    case dt_schar:
        return *(signed char*)field;
    case dt_char:
        return *(char*)field;
    case dt_short:
        return *(signed short*)field;
    case dt_ushort:
        return *(unsigned short*)field;
    case dt_int:
        return *(signed int*)field;
    case dt_uint:
        return *(unsigned int*)field;
    case dt_long:
        return *(int32_t *)field;
    case dt_ulong:
        return *(uint32_t *)field;
    case dt_longlong:
        return *(int64_t *)field;
    case dt_ulonglong:
        return *(uint64_t *)field;
    case dt_float:
        return (int64_t)(*(float*)field);
    case dt_double:
        return (int64_t)(*(double*)field);
    case dt_longdouble:
        return (int64_t)(*(long double*)field);
    case dt_charptr:
    case dt_default:
    case dt_void:
    default:
        ERRORLOG("unexpected datatype for field '%s', '%d'", named_field->name, named_field->type);
        return -1;
    }
}

//for fields that are fully handled in the parse function
void assign_null(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
}

void assign_default(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    void* field = (char*)named_field->field + named_fields_set->struct_size * idx;
    switch (named_field->type)
    {
    case dt_uchar:
        if (value < 0 || value > UCHAR_MAX)
            NAMFIELDWRNLOG("Value out of range for unsigned char: %" PRId64, value);
        else
            *(unsigned char*)field = (unsigned char)value;
        break;
    case dt_schar:
            *(signed char*)field = (signed char)value;
        break;
    case dt_char:
        if (value < CHAR_MIN || value > CHAR_MAX)
            NAMFIELDWRNLOG("Value out of range for char: %" PRId64, value);
        else
            *(char*)field = (char)value;
        break;
    case dt_short:
        if (value < SHRT_MIN || value > SHRT_MAX)
            NAMFIELDWRNLOG("Value out of range for signed short: %" PRId64, value);
        else
            *(signed short*)field = (signed short)value;
        break;
    case dt_ushort:
        if (value < 0 || value > USHRT_MAX)
            NAMFIELDWRNLOG("Value out of range for unsigned short: %" PRId64, value);
        else
            *(unsigned short*)field = (unsigned short)value;
        break;
    case dt_int:
        if (value < INT_MIN || value > INT_MAX)
            NAMFIELDWRNLOG("Value out of range for signed int: %" PRId64, value);
        else
            *(signed int*)field = (signed int)value;
        break;
    case dt_uint:
        if (value < 0 || value > UINT_MAX)
            NAMFIELDWRNLOG("Value out of range for unsigned int: %" PRId64, value);
        else
            *(unsigned int*)field = (unsigned int)value;
        break;
    case dt_long:
        if (value < LONG_MIN || value > LONG_MAX)
            NAMFIELDWRNLOG("Value out of range for signed long: %" PRId64, value);
        else
            *(signed long *)field = (signed long)value;
        break;
    case dt_ulong:
        if (value < 0 || value > ULONG_MAX)
            NAMFIELDWRNLOG("Value out of range for unsigned long: %" PRId64, value);
        else
            *(unsigned long *)field = (unsigned long)value;
        break;
    case dt_longlong:
        if (value < INT64_MIN || value > INT64_MAX)
            NAMFIELDWRNLOG("Value out of range for signed long long: %" PRId64, value);
        else
            *(signed long long *)field = (signed long long)value;
        break;
    case dt_ulonglong:
        if (value < 0)
            NAMFIELDWRNLOG("Value out of range for unsigned long long: %" PRId64, value);
        else
            *(unsigned long long *)field = (unsigned long long)value;
        break;
    case dt_float:
        *(float*)field = (float)value;
        break;
    case dt_double:
        *(double*)field = (double)value;
        break;
    case dt_longdouble:
        *(long double*)field = (long double)value;
        break;
    case dt_charptr:
    case dt_default:
    case dt_void:
    default:
        NAMFIELDWRNLOG("unexpected datatype for field '%s', '%d'", named_field->name, named_field->type);
        break;
    }
}

void assign_named_field_value(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    if(named_field->assign_func == NULL)
    {
        ERRORLOG("No assign_func for field %s",named_field->name);
        assign_default(named_field,value,named_fields_set,idx,src_str,flags);
    }
    else
    {
        named_field->assign_func(named_field,value,named_fields_set,idx,src_str,flags);
    }
}

/**
 * Recognizes config command and returns its number, or negative status code.
 * @param buf
 * @param pos
 * @param buflen
 * @param commands
 * @return If ccr_ok is returned the field has been correctly assigned
 * If ccr_comment      is returned, that means the current line did not contained any command and should be skipped.
 * If ccr_endOfFile    is returned, that means we've reached end of file.
 * If ccr_unrecognised is returned, that means the command wasn't recognized.
 * If ccr_endOfBlock   is returned, that means we've reached end of the INI block.
 * If ccr_error        is returned, that means something went wrong.
 */

int assign_conf_command_field(const char *buf,int32_t *pos,long buflen,const struct NamedField commands[], const struct NamedFieldSet* named_fields_set, int idx, unsigned short flags, const char *config_textname)
{
    SYNCDBG(19,"Starting");
    if ((*pos) >= buflen) return -1;
    // Skipping starting spaces
    while ((buf[*pos] == ' ') || (buf[*pos] == '\t') || (buf[*pos] == '\n') || (buf[*pos] == '\r') || (buf[*pos] == 26) || ((unsigned char)buf[*pos] < 7))
    {
        (*pos)++;
        if ((*pos) >= buflen) return -1;
    }
    // Checking if this line is a comment
    if (buf[*pos] == ';')
        return ccr_comment;
    // Checking if this line is start of a block
    if (buf[*pos] == '[')
        return ccr_endOfBlock;
    // Finding command number
    int i = 0;
    while (commands[i].name != NULL)
    {
        if (flag_is_set(flags,CnfLd_ListOnly) && strcasecmp(commands[i].name,"Name") != 0)
        {
            i++;
            continue;
        }

        if (commands[i].argnum > 0)
        {
            i++;
            continue;
        }

        int cmdname_len = strlen(commands[i].name);
        if ((*pos)+cmdname_len > buflen) {
            i++;
            continue;
        }
        // Find a matching command
        if (strnicmp(buf+(*pos), commands[i].name, cmdname_len) == 0)
        {
            (*pos) += cmdname_len;
            // if we're not at end of input buffer..
            if ((*pos) < buflen)
            {
                // make sure it's whole command, not just start of different one
               if ((buf[(*pos)] != ' ') && (buf[(*pos)] != '\t')
                && (buf[(*pos)] != '=')  && ((unsigned char)buf[(*pos)] >= 7))
               {
                  (*pos) -= cmdname_len;
                  i++;
                  continue;
               }
               // Skipping spaces between command and parameters
               while ((buf[*pos] == ' ') || (buf[*pos] == '\t')
                || (buf[*pos] == '=')  || ((unsigned char)buf[*pos] < 7))
               {
                 (*pos)++;
                 if ((*pos) >= buflen) break;
               }
            }


            int64_t k = 0;
            if (commands[i].argnum == -1)
            {
                #define MAX_LINE_LEN 1024
                char line_buf[MAX_LINE_LEN];
                int line_len = 0;

                // Copy characters until newline or end of buffer
                while ((*pos) + line_len < buflen &&
                      buf[(*pos) + line_len] != '\n' &&
                      buf[(*pos) + line_len] != '\r')
                {
                    line_buf[line_len] = buf[(*pos) + line_len];
                    line_len++;

                    // Prevent buffer overflow
                    if (line_len >= MAX_LINE_LEN - 1)
                    {
                      ERRORLOG("preventing overflow for long conf line");
                      break;
                    }
                }

                #undef MAX_LINE_LEN
                line_buf[line_len] = '\0'; // Null-terminate the string

                // Move position to the next line
                (*pos) += line_len;

                // Pass extracted string
              k = parse_named_field_value(&commands[i], line_buf,named_fields_set,idx,config_textname,ccf_None);
              assign_named_field_value(&commands[i],k,named_fields_set,idx,config_textname,ccf_None);
            }
            else
            {
                char word_buf[COMMAND_WORD_LEN];
                uchar n = 0;
                while (get_conf_parameter_single(buf,pos,buflen,word_buf,sizeof(word_buf)) > 0)
                {
                    if(strcmp(commands[i + n].name, commands[i].name) != 0)
                    {
                        CONFWRNLOG("more params than expected for command '%s' '%s'",commands[i].name, word_buf);
                    }
                    else
                    {
                        k = parse_named_field_value(&commands[i + n],word_buf,named_fields_set,idx,config_textname,ccf_None);
                        assign_named_field_value(&commands[i + n],k,named_fields_set,idx,config_textname,ccf_None);
                        n++;
                    }
                }
            }
            return ccr_ok;
        }
        i++;
    }
    return ccr_unrecognised;
}

TbBool parse_named_field_block(const char *buf, long len, const char *config_textname, unsigned short flags,const char* blockname,
                         const struct NamedField named_field[], const struct NamedFieldSet* named_fields_set, int idx)
{
    int32_t pos = 0;
    int k = find_conf_block(buf, &pos, len, blockname);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
            WARNMSG("Block [%s] not found in %s file.",blockname,config_textname);
        return false;
    }

    while (pos<len)
    {
        // Finding command number in this line.
        int assignresult = assign_conf_command_field(buf, &pos, len, named_field,named_fields_set,idx,flags,config_textname);
        if( assignresult == ccr_ok || assignresult == ccr_comment )
        {
            skip_conf_to_next_line(buf,&pos,len);
            continue;
        }
        else if( assignresult == ccr_unrecognised)
        {
            skip_conf_to_next_line(buf,&pos,len);
            continue;
        }
        else if( assignresult == ccr_endOfBlock || assignresult == ccr_error || assignresult == ccr_endOfFile)
        {
            break;
        }
    }
    return true;
}

void set_defaults(const struct NamedFieldSet* named_fields_set, const char *config_textname)
{
  memset((void *)named_fields_set->struct_base, 0, named_fields_set->struct_size * named_fields_set->max_count);

  const struct NamedField* name_NamedField = NULL;

  for (long i = 0; named_fields_set->named_fields[i].name != NULL; i++)
  {
      if (named_fields_set->named_fields[i].default_value != 0)
      {
          for (long j = 0; j < named_fields_set->max_count; j++)
          {
              assign_default(&named_fields_set->named_fields[i], named_fields_set->named_fields[i].default_value, named_fields_set, j, config_textname, ccf_None);
          }
      }

      if(strcmp(named_fields_set->named_fields[i].name, "NAME") == 0)
      {
          name_NamedField = &named_fields_set->named_fields[i];
      }

  }

  if (name_NamedField != NULL && named_fields_set->names != NULL)
  {
      for (int i = 0; i < named_fields_set->max_count; i++)
      {
          named_fields_set->names[i].name = (char*)name_NamedField->field + i * named_fields_set->struct_size;
          named_fields_set->names[i].num = i;
      }
      named_fields_set->names[named_fields_set->max_count - 1].name = NULL; // must be null for get_id
  }
}


TbBool parse_named_field_blocks(char *buf, long len, const char *config_textname, unsigned short flags,
                               const struct NamedFieldSet* named_fields_set)
{
    int32_t pos = 0;
    // Initialize the array
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        set_defaults(named_fields_set,config_textname);
    }

    const char * blockname = NULL;
    int blocknamelen = 0;
    const int basename_len = strlen(named_fields_set->block_basename);
    while (iterate_conf_blocks(buf, &pos, len, &blockname, &blocknamelen))
    {
        // look for blocks starting with block_basename, followed by one or more digits
        if (blocknamelen < basename_len + 1) {
            continue;
        } else if (memcmp(blockname, named_fields_set->block_basename, basename_len) != 0) {
            continue;
        }
        const int i = natoi(&blockname[basename_len], blocknamelen - basename_len);
        if (i < 0 || i >= named_fields_set->max_count) {
            continue;
        } else if (i >= *named_fields_set->count_field) {
            *named_fields_set->count_field = i + 1;
        }
        char blockname_null[COMMAND_WORD_LEN];
        strncpy(blockname_null, blockname, blocknamelen);
        blockname_null[blocknamelen] = '\0';

        parse_named_field_block(buf, len, config_textname, flags, blockname_null, named_fields_set->named_fields, named_fields_set, i);
    }

    return true;
}

int get_conf_parameter_whole(const char *buf,int32_t *pos,long buflen,char *dst,long dstlen)
{
  int i;
  if ((*pos) >= buflen) return 0;
  // Skipping spaces after previous parameter
  while ((buf[*pos] == ' ') || (buf[*pos] == '\t'))
  {
    (*pos)++;
    if ((*pos) >= buflen) return 0;
  }
  for (i=0; i+1 < dstlen; i++)
  {
    if ((buf[*pos]=='\r') || (buf[*pos]=='\n') || ((unsigned char)buf[*pos] < 7))
      break;
    dst[i]=buf[*pos];
    (*pos)++;
    if ((*pos) > buflen) break;
  }
  dst[i]='\0';
  return i;
}

int get_conf_parameter_single(const char *buf,int32_t *pos,long buflen,char *dst,long dstlen)
{
    int i;
    if ((*pos) >= buflen) return 0;
    // Skipping spaces after previous parameter
    while ((buf[*pos] == ' ') || (buf[*pos] == '\t'))
    {
        (*pos)++;
        if ((*pos) >= buflen) return 0;
    }
    for (i=0; i+1 < dstlen; i++)
    {
        if ((buf[*pos] == ' ') || (buf[*pos] == '\t') || (buf[*pos] == '\r')
         || (buf[*pos] == '\n') || ((unsigned char)buf[*pos] < 7))
          break;
        dst[i]=buf[*pos];
        (*pos)++;
        if ((*pos) >= buflen) {
            i++;
            break;
        }
    }
    dst[i]='\0';
    return i;
}

/**
 * Returns parameter num from given NamedCommand array, or 0 if not found.
 */
int recognize_conf_parameter(const char *buf,int32_t *pos,long buflen,const struct NamedCommand commands[])
{
  if ((*pos) >= buflen) return 0;
  // Skipping spaces after previous parameter
  while ((buf[*pos] == ' ') || (buf[*pos] == '\t'))
  {
    (*pos)++;
    if ((*pos) >= buflen) return 0;
  }
  int i = 0;
  while (commands[i].name != NULL)
  {
      int par_len = strlen(commands[i].name);
      if (strncasecmp(&buf[(*pos)], commands[i].name, par_len) == 0)
      {
          // If EOLN found, finish and return position before the EOLN
          if ((buf[(*pos)+par_len] == '\n') || (buf[(*pos)+par_len] == '\r'))
          {
            (*pos) += par_len;
            return commands[i].num;
          }
          // If non-EOLN blank char, finish and return position after the char
          if ((buf[(*pos)+par_len] == ' ') || (buf[(*pos)+par_len] == '\t')
           || ((unsigned char)buf[(*pos)+par_len] < 7))
          {
            (*pos) += par_len+1;
            return commands[i].num;
          }
      }
      i++;
  }
  return 0;
}

/**
 * Returns name of a config parameter with given number, or empty string.
 */
const char *get_conf_parameter_text(const struct NamedCommand commands[],int num)
{
    long i = 0;
    while (commands[i].name != NULL)
    {
        //SYNCLOG("\"%s\", %d %d",commands[i].name,commands[i].num,num);
        if (commands[i].num == num)
            return commands[i].name;
        i++;
  }
  return "";
}

/**
 * Returns ID of given item using NamedField list.
 * If not found, returns -1.
 */
long get_named_field_id(const struct NamedField *desc, const char *itmname)
{
  if ((desc == NULL) || (itmname == NULL))
    return -1;
  for (long i = 0; desc[i].name != NULL; i++)
  {
    if (strcasecmp(desc[i].name, itmname) == 0)
      return i;
  }
  return -1;
}

/**
 * Returns ID of given item using NamedCommands list.
 * Similar to recognize_conf_parameter(), but for use only if the buffer stores
 * one word, ended with "\0".
 * If not found, returns -1.
 */
long get_id(const struct NamedCommand *desc, const char *itmname)
{
  if ((desc == NULL) || (itmname == NULL))
    return -1;
  for (long i = 0; desc[i].name != NULL; i++)
  {
    if (strcasecmp(desc[i].name, itmname) == 0)
      return desc[i].num;
  }
  return -1;
}

/**
 * Returns ID of given item using NamedCommands list.
 * Similar to recognize_conf_parameter(), but for use only if the buffer stores
 * one word, ended with "\0".
 * If not found, returns -1.
 */
long long get_long_id(const struct LongNamedCommand* desc, const char* itmname)
{
    if ((desc == NULL) || (itmname == NULL))
        return -1;
    for (long i = 0; desc[i].name != NULL; i++)
    {
        if (strcasecmp(desc[i].name, itmname) == 0)
            return desc[i].num;
    }
    return -1;
}

/**
 * Returns ID of given item using NamedCommands list, or any item if the string is 'RANDOM'.
 * Similar to recognize_conf_parameter(), but for use only if the buffer stores
 * one word, ended with "\0".
 * If not found, returns -1.
 */
long get_rid(const struct NamedCommand *desc, const char *itmname)
{
  long i;
  if ((desc == NULL) || (itmname == NULL))
    return -1;
  for (i=0; desc[i].name != NULL; i++)
  {
    if (strcasecmp(desc[i].name, itmname) == 0)
      return desc[i].num;
  }
  if (strcasecmp("RANDOM", itmname) == 0)
  {
      i = (rand() % i);
      return desc[i].num;
  }
  return -1;
}

char *prepare_file_path_buf(char *dst, int dst_size, short fgroup, const char *fname)
{
    return prepare_file_path_buf_mod(dst, dst_size, NULL, fgroup, fname);
}
/*
 * @mod_dir insert before fgroup related sdir, set NULL if no mod.
 * @fname insert after fgroup related sdir.
 */
char *prepare_file_path_buf_mod(char *dst, int dst_size, const char *mod_dir, short fgroup, const char *fname)
{
  const char *mdir = NULL;
  const char *sdir = NULL;
  switch (fgroup)
  {
  case FGrp_StdData:
      mdir=keeper_runtime_directory;
      sdir="data";
      break;
  case FGrp_LrgData:
      mdir=keeper_runtime_directory;
      sdir="data";
      break;
  case FGrp_FxData:
      mdir=keeper_runtime_directory;
      sdir="fxdata";
      break;
  case FGrp_LoData:
      mdir=install_info.inst_path;
      sdir="ldata";
      break;
  case FGrp_HiData:
      mdir=keeper_runtime_directory;
      sdir="hdata";
      break;
  case FGrp_Music:
      mdir=keeper_runtime_directory;
      sdir="music";
      break;
  case FGrp_VarLevels:
      mdir=install_info.inst_path;
      sdir="levels";
      break;
  case FGrp_Save:
      mdir=keeper_runtime_directory;
      sdir="save";
      break;
  case FGrp_SShots:
      mdir=keeper_runtime_directory;
      sdir="scrshots";
      break;
  case FGrp_StdSound:
      mdir=keeper_runtime_directory;
      sdir="sound";
      break;
  case FGrp_LrgSound:
      mdir=keeper_runtime_directory;
      sdir="sound";
      break;
  case FGrp_AtlSound:
      if (campaign.speech_location[0] == '\0') {
          mdir=NULL; sdir=NULL;
          break;
      }
      mdir=keeper_runtime_directory;
      sdir=campaign.speech_location;
      break;
  case FGrp_Main:
      mdir=keeper_runtime_directory;
      sdir=NULL;
      break;
  case FGrp_Campgn:
      mdir=keeper_runtime_directory;
      sdir="campgns";
      break;
  case FGrp_CmpgLvls:
      if (campaign.levels_location[0] == '\0') {
          mdir=NULL; sdir=NULL;
          break;
      }
      mdir=install_info.inst_path;
      sdir=campaign.levels_location;
      break;
  case FGrp_CmpgCrtrs:
      if (campaign.creatures_location[0] == '\0') {
          mdir=NULL; sdir=NULL;
          break;
      }
      mdir=install_info.inst_path;
      sdir=campaign.creatures_location;
      break;
  case FGrp_CmpgConfig:
      if (campaign.configs_location[0] == '\0') {
          mdir=NULL; sdir=NULL;
          break;
      }
      mdir=install_info.inst_path;
      sdir=campaign.configs_location;
      break;
  case FGrp_CmpgMedia:
      if (campaign.media_location[0] == '\0') {
          mdir=NULL; sdir=NULL;
          break;
      }
      mdir=install_info.inst_path;
      sdir=campaign.media_location;
      break;
  case FGrp_LandView:
      if (campaign.land_location[0] == '\0') {
          mdir=NULL; sdir=NULL;
          break;
      }
      mdir=install_info.inst_path;
      sdir=campaign.land_location;
      break;
  case FGrp_CrtrData:
      mdir=keeper_runtime_directory;
      sdir="creatrs";
      break;
  default:
      mdir="./";
      sdir=NULL;
      break;
  }
  if (mdir == NULL)
      dst[0] = '\0';
  else {
      if (mod_dir == NULL)
          mod_dir = "";
      if (sdir == NULL)
          sdir = "";
      if (fname == NULL)
          fname = "";

      const char *mod_sep = mod_dir[0] ==0 ? "" : "/";
      const char *dir_sep = sdir[0] == 0 ? "" : "/";
      const char *file_sep = fname[0] ==0 ? "" : "/";
      snprintf(dst, dst_size, "%s%s%s%s%s%s%s", mdir, mod_sep, mod_dir, dir_sep, sdir, file_sep, fname);
  }
  return dst;
}

char *prepare_file_path_mod(const char *mod_dir, short fgroup, const char *fname)
{
  static char ffullpath[2048];
  return prepare_file_path_buf_mod(ffullpath, sizeof(ffullpath), mod_dir, fgroup, fname);
}

char *prepare_file_path(short fgroup, const char *fname)
{
  static char ffullpath[2048];
  return prepare_file_path_buf_mod(ffullpath, sizeof(ffullpath), NULL, fgroup, fname);
}

char *prepare_file_path_va_mod(const char *mod_dir, short fgroup, const char *fmt_str, va_list arg)
{
  char fname[255] = "";
  vsnprintf(fname, sizeof(fname), fmt_str, arg);
  static char ffullpath[2048];
  return prepare_file_path_buf_mod(ffullpath, sizeof(ffullpath), mod_dir, fgroup, fname);
}

char *prepare_file_fmtpath_mod(const char *mod_dir, short fgroup, const char *fmt_str, ...)
{
  va_list val;
  va_start(val, fmt_str);
  char* result = prepare_file_path_va_mod(mod_dir, fgroup, fmt_str, val);
  va_end(val);
  return result;
}

char *prepare_file_fmtpath(short fgroup, const char *fmt_str, ...)
{
  va_list val;
  va_start(val, fmt_str);
  char* result = prepare_file_path_va_mod(NULL, fgroup, fmt_str, val);
  va_end(val);
  return result;
}

/**
 * Returns the folder specified by LEVELS_LOCATION
 */
short get_level_fgroup(LevelNumber lvnum)
{
    return FGrp_CmpgLvls;
}

/**
 * Loads data file into allocated buffer.
 * @return Returns NULL if the file doesn't exist or is smaller than ldsize;
 * on success, returns a buffer which should be freed after use,
 * and sets ldsize into its size.
 */
unsigned char *load_data_file_to_buffer(int32_t *ldsize, short fgroup, const char *fmt_str, ...)
{
  // Prepare file name
  va_list arg;
  va_start(arg, fmt_str);
  char fname[255] = "";
  vsnprintf(fname, sizeof(fname), fmt_str, arg);
  char ffullpath[2048];
  prepare_file_path_buf(ffullpath, sizeof(ffullpath), fgroup, fname);
  va_end(arg);
  // Load the file
   long fsize = LbFileLengthRnc(ffullpath);
   if (fsize < *ldsize)
   {
       WARNMSG("File \"%s\" doesn't exist or is too small.", fname);
       return NULL;
  }
  unsigned char* buf = calloc(fsize + 16, 1);
  if (buf == NULL)
  {
    WARNMSG("Can't allocate %ld bytes to load \"%s\".",fsize,fname);
    return NULL;
  }
  fsize = LbFileLoadAt(ffullpath,buf);
  if (fsize < *ldsize)
  {
    WARNMSG("Reading file \"%s\" failed.",fname);
    free(buf);
    return NULL;
  }
  memset(buf+fsize, '\0', 15);
  *ldsize = fsize;
  return buf;
}






struct LevelInformation *get_level_info(LevelNumber lvnum)
{
  return get_campaign_level_info(&campaign, lvnum);
}

struct LevelInformation *get_or_create_level_info(LevelNumber lvnum, unsigned long lvoptions)
{
    struct LevelInformation* lvinfo = get_campaign_level_info(&campaign, lvnum);
    if (lvinfo != NULL)
    {
        lvinfo->level_type |= lvoptions;
        return lvinfo;
  }
  lvinfo = new_level_info_entry(&campaign, lvnum);
  if (lvinfo != NULL)
  {
    lvinfo->level_type |= lvoptions;
    return lvinfo;
  }
  return NULL;
}

/**
 * Returns first level info structure in the array.
 */
struct LevelInformation *get_first_level_info(void)
{
  if (campaign.lvinfos == NULL)
    return NULL;
  return &campaign.lvinfos[0];
}

/**
 * Returns last level info structure in the array.
 */
struct LevelInformation *get_last_level_info(void)
{
  if ((campaign.lvinfos == NULL) || (campaign.lvinfos_count < 1))
    return NULL;
  return &campaign.lvinfos[campaign.lvinfos_count-1];
}

/**
 * Returns next level info structure in the array.
 * Note that it's not always corresponding to next campaign level; use
 * get_level_info() to get information for specific level. This function
 * is used for sweeping through all level info entries.
 */
struct LevelInformation *get_next_level_info(struct LevelInformation *previnfo)
{
  if (campaign.lvinfos == NULL)
    return NULL;
  if (previnfo == NULL)
    return NULL;
  unsigned long i = previnfo - &campaign.lvinfos[0];
  i++;
  if (i >= campaign.lvinfos_count)
    return NULL;
  return &campaign.lvinfos[i];
}

/**
 * Returns previous level info structure in the array.
 * Note that it's not always corresponding to previous campaign level; use
 * get_level_info() to get information for specific level. This function
 * is used for reverse sweeping through all level info entries.
 */
struct LevelInformation *get_prev_level_info(struct LevelInformation *nextinfo)
{
  if (campaign.lvinfos == NULL)
    return NULL;
  if (nextinfo == NULL)
    return NULL;
  int i = nextinfo - &campaign.lvinfos[0];
  i--;
  if (i < 0)
    return NULL;
  return &campaign.lvinfos[i];
}

short set_level_info_string_index(LevelNumber lvnum, char *stridx, unsigned long lvoptions)
{
    if (campaign.lvinfos == NULL)
        init_level_info_entries(&campaign, 0);
    struct LevelInformation* lvinfo = get_or_create_level_info(lvnum, lvoptions);
    if (lvinfo == NULL)
        return false;
    int k = atoi(stridx);
    if (k > 0)
    {
        lvinfo->name_stridx = k;
        return true;
    }
  return false;
}

short set_level_info_text_name(LevelNumber lvnum, char *name, unsigned long lvoptions)
{
    if (campaign.lvinfos == NULL)
        init_level_info_entries(&campaign, 0);
    struct LevelInformation* lvinfo = get_or_create_level_info(lvnum, lvoptions);
    if (lvinfo == NULL)
        return false;
    snprintf(lvinfo->name, LINEMSG_SIZE, "%s", name);
    if ((lvoptions & LvKind_IsFree) != 0)
    {
        lvinfo->ensign_x += ((LANDVIEW_MAP_WIDTH >> 4) * (LbSinL(lvnum * DEGREES_11_25) >> 6)) >> 10;
        lvinfo->ensign_y -= ((LANDVIEW_MAP_HEIGHT >> 4) * (LbCosL(lvnum * DEGREES_11_25) >> 6)) >> 10;
  }
  return true;
}

TbBool reset_credits(struct CreditsItem *credits)
{
    for (long i = 0; i < CAMPAIGN_CREDITS_COUNT; i++)
    {
        memset(&credits[i], 0, sizeof(struct CreditsItem));
        credits[i].kind = CIK_None;
  }
  return true;
}

TbBool parse_credits_block(struct CreditsItem *credits,char *buf,char *buffer_end_pointer)
{
  const char * block_name = "credits";
  // Find the block
  long len = buffer_end_pointer - buf;
  int32_t pos = 0;
  int k = find_conf_block(buf, &pos, len, block_name);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in Credits file.", block_name);
    return 0;
  }
  int n = 0;
  while (pos<len)
  {
    if ((buf[pos] != 0) && (buf[pos] != '[') && (buf[pos] != ';'))
    {
      credits[n].kind = CIK_EmptyLine;
      credits[n].font = 2;
      char word_buf[32];
      switch (buf[pos])
      {
      case '*':
        pos++;
        if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          k = atol(word_buf);
        else
          k = 0;
        if (k > 0)
        {
          credits[n].kind = CIK_StringId;
          credits[n].font = 1;
          credits[n].num = k;
        }
        break;
      case '&':
        pos++;
        if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          k = atoi(word_buf);
        else
          k = 0;
        if (k > 0)
        {
          credits[n].kind = CIK_StringId;
          credits[n].font = 2;
          credits[n].num = k;
        }
        break;
      case '%':
        pos++;
        credits[n].kind = CIK_DirectText;
        credits[n].font = 0;
        credits[n].str = &buf[pos];
        break;
      case '#':
        pos++;
        credits[n].kind = CIK_DirectText;
        credits[n].font = 1;
        credits[n].str = &buf[pos];
        break;
      default:
        credits[n].kind = CIK_DirectText;
        credits[n].font = 2;
        credits[n].str = &buf[pos];
        break;
      }
      n++;
    }
    // Finishing the line
    while (pos < len)
    {
      if (buf[pos] < 32) break;
      pos++;
    }
    if (buf[pos] == '\r')
    {
      buf[pos] = '\0';
      pos+=2;
    } else
    {
      buf[pos] = '\0';
      pos++;
    }
  }
  if (credits[0].kind == CIK_None) {
    WARNMSG("Credits list empty after parsing [%s] block of Credits file.", block_name);
  }
  return true;
}

/**
 * Loads the credits data for the current campaign.
 */
TbBool setup_campaign_credits_data(struct GameCampaign *campgn)
{
  SYNCDBG(18,"Starting");
  char* fname = prepare_file_path(FGrp_LandView, campgn->credits_fname);
  long filelen = LbFileLengthRnc(fname);
  if (filelen <= 0)
  {
    ERRORLOG("Campaign Credits file \"%s\" does not exist or can't be opened",campgn->credits_fname);
    return false;
  }
  campgn->credits_data = (char *)calloc(filelen + 256, 1);
  if (campgn->credits_data == NULL)
  {
    ERRORLOG("Can't allocate memory for Campaign Credits file \"%s\"",campgn->credits_fname);
    return false;
  }
  char* credits_data_end = campgn->credits_data + filelen + 255;
  short result = true;
  long loaded_size = LbFileLoadAt(fname, campgn->credits_data);
  if (loaded_size < 4)
  {
    ERRORLOG("Campaign Credits file \"%s\" couldn't be loaded or is too small",campgn->credits_fname);
    result = false;
  }
  // Resetting all values to unused
  reset_credits(campgn->credits);
  // Analyzing credits data and filling correct values
  if (result)
  {
    result = parse_credits_block(campgn->credits, campgn->credits_data, credits_data_end);
    if (!result)
      WARNMSG("Parsing credits file \"%s\" credits block failed",campgn->credits_fname);
  }
  SYNCDBG(19,"Finished");
  return result;
}

short is_bonus_level(LevelNumber lvnum)
{
  if (lvnum < 1) return false;
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.bonus_levels[i] == lvnum)
    {
        SYNCDBG(7,"Level %d identified as bonus",lvnum);
        return true;
    }
  }
  SYNCDBG(7,"Level %d not recognized as bonus",lvnum);
  return false;
}

short is_extra_level(LevelNumber lvnum)
{
  if (lvnum < 1) return false;
  for (int i = 0; i < EXTRA_LEVELS_COUNT; i++)
  {
      if (campaign.extra_levels[i] == lvnum)
      {
          SYNCDBG(7,"Level %d identified as extra",lvnum);
          return true;
      }
  }
  SYNCDBG(7,"Level %d not recognized as extra",lvnum);
  return false;
}

/**
 * Returns index for Game->bonus_levels associated with given single player level.
 * Gives -1 if there's no store place for the level.
 */
int storage_index_for_bonus_level(LevelNumber bn_lvnum)
{
    if (bn_lvnum < 1)
        return -1;
    int k = 0;
    for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
    {
        if (campaign.bonus_levels[i] == bn_lvnum)
            return k;
        if (campaign.bonus_levels[i] != 0)
            k++;
  }
  return -1;
}

/**
 * Returns index for Campaign->single_levels associated with given singleplayer level.
 * If the level is not found, returns -1.
 */
int array_index_for_singleplayer_level(LevelNumber sp_lvnum)
{
  if (sp_lvnum < 1) return -1;
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.single_levels[i] == sp_lvnum)
        return i;
  }
  return -1;
}

/**
 * Returns bonus level number for given singleplayer level number.
 * If no bonus found, returns 0.
 */
LevelNumber bonus_level_for_singleplayer_level(LevelNumber sp_lvnum)
{
    int i = array_index_for_singleplayer_level(sp_lvnum);
    if (i >= 0)
        return campaign.bonus_levels[i];
    return 0;
}

/**
 * Returns first single player level number.
 * On error, returns SINGLEPLAYER_NOTSTARTED.
 */
LevelNumber first_singleplayer_level(void)
{
    long lvnum = campaign.single_levels[0];
    if (lvnum > 0)
        return lvnum;
    return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns last single player level number.
 * On error, returns SINGLEPLAYER_NOTSTARTED.
 */
LevelNumber last_singleplayer_level(void)
{
    int i = campaign.single_levels_count;
    if ((i > 0) && (i <= CAMPAIGN_LEVELS_COUNT))
        return campaign.single_levels[i - 1];
    return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns first multi player level number.
 * On error, returns SINGLEPLAYER_NOTSTARTED.
 */
LevelNumber first_multiplayer_level(void)
{
  long lvnum = campaign.multi_levels[0];
  if (lvnum > 0)
    return lvnum;
  return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns first extra level number.
 * On error, returns SINGLEPLAYER_NOTSTARTED.
 */
LevelNumber first_extra_level(void)
{
    for (unsigned long lvidx = 0; lvidx < campaign.extra_levels_index; lvidx++)
    {
        long lvnum = campaign.extra_levels[lvidx];
        if (lvnum > 0)
            return lvnum;
  }
  return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns the extra level number. Gives SINGLEPLAYER_NOTSTARTED if no such level,
 * LEVELNUMBER_ERROR on error.
 */
LevelNumber get_extra_level(unsigned short elv_kind)
{
    int i = elv_kind;
    i--;
    if ((i < 0) || (i >= EXTRA_LEVELS_COUNT))
        return LEVELNUMBER_ERROR;
    LevelNumber lvnum = campaign.extra_levels[i];
    SYNCDBG(5, "Extra level kind %d has number %d", (int)elv_kind, lvnum);
    if (lvnum > 0)
    {
        return lvnum;
  }
  return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns the next single player level. Gives SINGLEPLAYER_FINISHED if
 * last level was won, LEVELNUMBER_ERROR on error.
 */
LevelNumber next_singleplayer_level(LevelNumber sp_lvnum, TbBool ignore)
{
  if (sp_lvnum == SINGLEPLAYER_FINISHED) return SINGLEPLAYER_FINISHED;
  if (sp_lvnum == SINGLEPLAYER_NOTSTARTED) return first_singleplayer_level();
  if (sp_lvnum < 1) return LEVELNUMBER_ERROR;
  int next_level;

  if ((intralvl.next_level > 0) && !ignore)
  {
      next_level = intralvl.next_level;
      intralvl.next_level = 0;
      if (next_level < 0)
          return SINGLEPLAYER_FINISHED;

      for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
      {
          if (campaign.single_levels[i] == next_level)
          {
              return next_level;
          }
      }
      WARNLOG("Trying to jump to level %d that does not exist.", next_level);
      return LEVELNUMBER_ERROR;
  }

  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.single_levels[i] == sp_lvnum)
    {
      if (i+1 >= CAMPAIGN_LEVELS_COUNT)
        return SINGLEPLAYER_FINISHED;
      if (campaign.single_levels[i+1] <= 0)
        return SINGLEPLAYER_FINISHED;
      return campaign.single_levels[i+1];
    }
  }
  return LEVELNUMBER_ERROR;
}

/**
 * Returns the previous single player level. Gives SINGLEPLAYER_NOTSTARTED if
 * first level was given, LEVELNUMBER_ERROR on error.
 */
LevelNumber prev_singleplayer_level(LevelNumber sp_lvnum)
{
  if (sp_lvnum == SINGLEPLAYER_NOTSTARTED) return SINGLEPLAYER_NOTSTARTED;
  if (sp_lvnum == SINGLEPLAYER_FINISHED) return last_singleplayer_level();
  if (sp_lvnum < 1) return LEVELNUMBER_ERROR;
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.single_levels[i] == sp_lvnum)
    {
      if (i < 1)
        return SINGLEPLAYER_NOTSTARTED;
      if (campaign.single_levels[i-1] <= 0)
        return SINGLEPLAYER_NOTSTARTED;
      return campaign.single_levels[i-1];
    }
  }
  return LEVELNUMBER_ERROR;
}

/**
 * Returns the next multi player level. Gives SINGLEPLAYER_FINISHED if
 * last level was given, LEVELNUMBER_ERROR on error.
 */
LevelNumber next_multiplayer_level(LevelNumber mp_lvnum)
{
  if (mp_lvnum == SINGLEPLAYER_FINISHED) return SINGLEPLAYER_FINISHED;
  if (mp_lvnum == SINGLEPLAYER_NOTSTARTED) return first_multiplayer_level();
  if (mp_lvnum < 1) return LEVELNUMBER_ERROR;
  for (int i = 0; i < MULTI_LEVELS_COUNT; i++)
  {
    if (campaign.multi_levels[i] == mp_lvnum)
    {
      if (i+1 >= MULTI_LEVELS_COUNT)
        return SINGLEPLAYER_FINISHED;
      if (campaign.multi_levels[i+1] <= 0)
        return SINGLEPLAYER_FINISHED;
      return campaign.multi_levels[i+1];
    }
  }
  return LEVELNUMBER_ERROR;
}

/**
 * Returns the next extra level. Gives SINGLEPLAYER_FINISHED if
 * last level was given, LEVELNUMBER_ERROR on error.
 */
LevelNumber next_extra_level(LevelNumber ex_lvnum)
{
  if (ex_lvnum == SINGLEPLAYER_FINISHED) return SINGLEPLAYER_FINISHED;
  if (ex_lvnum == SINGLEPLAYER_NOTSTARTED) return first_extra_level();
  if (ex_lvnum < 1) return LEVELNUMBER_ERROR;
  for (int i = 0; i < EXTRA_LEVELS_COUNT; i++)
  {
    if (campaign.extra_levels[i] == ex_lvnum)
    {
      i++;
      while (i < EXTRA_LEVELS_COUNT)
      {
        if (campaign.extra_levels[i] > 0)
          return campaign.extra_levels[i];
        i++;
      }
      return SINGLEPLAYER_FINISHED;
    }
  }
  return LEVELNUMBER_ERROR;
}

/**
 * Returns if the level is a single player campaign level,
 * or special non-existing level at start/end of campaign.
 */
short is_singleplayer_like_level(LevelNumber lvnum)
{
  if ((lvnum == SINGLEPLAYER_FINISHED) || (lvnum == SINGLEPLAYER_NOTSTARTED))
    return true;
  return is_singleplayer_level(lvnum);
}

/**
 * Returns if the level is a single player campaign level.
 */
short is_singleplayer_level(LevelNumber lvnum)
{
  if (lvnum < 1)
  {
    SYNCDBG(17,"Level index %d is not correct",lvnum);
    return false;
  }
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.single_levels[i] == lvnum)
    {
      SYNCDBG(17,"Level %d identified as SP",lvnum);
      return true;
    }
  }
  SYNCDBG(17,"Level %d not recognized as SP",lvnum);
  return false;
}

short is_multiplayer_level(LevelNumber lvnum)
{
  int i;
  if (lvnum < 1) return false;
  for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.multi_levels[i] == lvnum)
    {
        SYNCDBG(17,"Level %d identified as MP",lvnum);
        return true;
    }
  }
  SYNCDBG(17,"Level %d not recognized as MP",lvnum);
  return false;
}

/**
 * Returns if the level is 'campaign' level.
 * All levels mentioned in campaign file are campaign levels. Campaign and
 * freeplay levels are exclusive.
 */
short is_campaign_level(LevelNumber lvnum)
{
  if (is_singleplayer_level(lvnum) || is_bonus_level(lvnum)
   || is_extra_level(lvnum) || is_multiplayer_level(lvnum))
    return true;
  return false;
}

/**
 * Returns if the level is 'free play' level, which should be visible
 * in list of levels.
 */
short is_freeplay_level(LevelNumber lvnum)
{
  if (lvnum < 1) return false;
  for (int i = 0; i < FREE_LEVELS_COUNT; i++)
  {
    if (campaign.freeplay_levels[i] == lvnum)
    {
        SYNCDBG(18,"%d is freeplay",lvnum);
        return true;
    }
  }
  SYNCDBG(18,"%d is NOT freeplay",lvnum);
  return false;
}

/**
  * checks if currently in a campaign, and if the provided level number is also part of it.
 */
TbBool is_level_in_current_campaign(LevelNumber lvnum)
{
    if (!is_campaign_level(game.loaded_level_number))
    {
        return false;
    }
    for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
    {
        if (campaign.single_levels[i] == lvnum)
        {
            return true;
        }
    }
    return false;
}


/* @comment
 *     The loading items of load_config and load_config_for_mod_one need to be consistent.
 */
static void load_config_for_mod_one(const struct ConfigFileData* file_data, unsigned short flags, const struct ModConfigItem *mod_item)
{
    set_flag(flags, (CnfLd_AcceptPartial | CnfLd_IgnoreErrors));

    const char* conf_fname = file_data->filename;
    const struct ModExistState *mod_state = &mod_item->state;
    char* fname = NULL;
    char mod_dir[256] = {0};
    sprintf(mod_dir, "%s/%s", MODS_DIR_NAME, mod_item->name);

    if (mod_state->fx_data)
    {
        fname = prepare_file_path_mod(mod_dir, FGrp_FxData, conf_fname);
        if (strlen(fname) > 0)
        {
            file_data->load_func(fname, flags);
        }
    }

    if (mod_state->cmpg_config)
    {
        fname = prepare_file_path_mod(mod_dir, FGrp_CmpgConfig, conf_fname);
        if (strlen(fname) > 0)
        {
            file_data->load_func(fname,flags);
        }
    }

    if (mod_state->cmpg_lvls)
    {
        fname = prepare_file_fmtpath_mod(mod_dir, FGrp_CmpgLvls, "map%05lu.%s", get_selected_level_number(), conf_fname);
        if (strlen(fname) > 0)
        {
            file_data->load_func(fname,flags);
        }
    }
}

static void load_config_for_mod_list(const struct ConfigFileData* file_data, unsigned short flags, const struct ModConfigItem *mod_items, long mod_cnt)
{
    for (long i=0; i<mod_cnt; i++)
    {
        const struct ModConfigItem *mod_item = mod_items + i;
        if (mod_item->state.mod_dir == 0)
            continue;

        load_config_for_mod_one(file_data, flags, mod_item);
    }
}

/* @comment
 *     The loading items of load_config and load_config_for_mod_one need to be consistent.
 */
TbBool load_config(const struct ConfigFileData* file_data, unsigned short flags)
{
    if (file_data->pre_load_func != NULL)
    {
        file_data->pre_load_func();
    }

    const char* conf_fname = file_data->filename;

    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = file_data->load_func(fname, flags);

    if (mods_conf.after_base_cnt > 0)
    {
        load_config_for_mod_list(file_data, flags, mods_conf.after_base_item, mods_conf.after_base_cnt);
    }

    fname = prepare_file_path(FGrp_CmpgConfig, conf_fname);
    if (strlen(fname) > 0)
    {
        file_data->load_func(fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }

    if (mods_conf.after_campaign_cnt > 0)
    {
        load_config_for_mod_list(file_data, flags, mods_conf.after_campaign_item, mods_conf.after_campaign_cnt);
    }

    fname = prepare_file_fmtpath(FGrp_CmpgLvls, "map%05lu.%s", get_selected_level_number(), conf_fname);
    if (strlen(fname) > 0)
    {
        file_data->load_func(fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }

    if (mods_conf.after_map_cnt > 0)
    {
        load_config_for_mod_list(file_data, flags, mods_conf.after_map_item, mods_conf.after_map_cnt);
    }

    if (file_data->post_load_func != NULL)
    {
        file_data->post_load_func();
    }

    return result;
}

/******************************************************************************/
