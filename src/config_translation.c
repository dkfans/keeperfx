/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_translation.c
 *     Translations configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for Translations.
 * @par Comment:
 *     None.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "pre_inc.h"
#include <toml.h>
#include <string.h>
#include <stdlib.h>
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "config.h"
#include "config_strings.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/*
[HELLO_WORLD]
ENG = "Hello World"
FRE = "baçuéttè"
RUS = "Привет, мир"
JPN = "こんにちは世界"

[GOODBYE]
ENG = "Goodbye"
FRE = "omelette du fromage"
*/

#define MAX_TRANSLATION_ENTRIES 4096
#define MAX_ALIAS_LEN           64

typedef struct {
    char  alias[MAX_ALIAS_LEN];
    char *text; // heap-allocated; NULL means allocation failed
} TranslationEntry;

static TranslationEntry translation_table[MAX_TRANSLATION_ENTRIES];
static int32_t          translation_count = 0;

typedef struct {
    const char *language_code;
} WalkContext;

static unsigned char convert_codepoint_to_internal_byte(unsigned long codepoint)
{
    if (codepoint < 0x80)
        return (unsigned char)codepoint;

    static const struct {
        unsigned long unicode;
        unsigned char byte;
    } codepage_map[] = {
        {0x0410, 0x41}, {0x0411, 0xB1}, {0x0412, 0x42}, {0x0413, 0xB2},
        {0x0414, 0xB3}, {0x0415, 0x45}, {0x0416, 0xB4}, {0x0417, 0xC2},
        {0x0418, 0xC3}, {0x0419, 0xC4}, {0x041A, 0x4B}, {0x041B, 0x7F},
        {0x041C, 0x4D}, {0x041D, 0x48}, {0x041E, 0x4F}, {0x041F, 0x92},
        {0x0420, 0x50}, {0x0421, 0x43}, {0x0422, 0x54}, {0x0423, 0xC5},
        {0x0424, 0xC8}, {0x0425, 0x58}, {0x0426, 0xC9}, {0x0427, 0xCA},
        {0x0428, 0xCB}, {0x0429, 0xCC}, {0x042A, 0x62}, {0x042B, 0xE8},
        {0x042C, 0x62}, {0x042D, 0x9B}, {0x042E, 0x9C}, {0x042F, 0x9D},
        {0x0430, 0x61}, {0x0431, 0xCD}, {0x0432, 0xCE}, {0x0433, 0xCF},
        {0x0434, 0xD0}, {0x0435, 0x65}, {0x0436, 0xD1}, {0x0437, 0xD5},
        {0x0438, 0xD9}, {0x0439, 0xDA}, {0x043A, 0x9E}, {0x043B, 0x9F},
        {0x043C, 0xA6}, {0x043D, 0xA7}, {0x043E, 0x6F}, {0x043F, 0xA9},
        {0x0440, 0x70}, {0x0441, 0x63}, {0x0442, 0xDB}, {0x0443, 0x79},
        {0x0444, 0xDC}, {0x0445, 0x78}, {0x0446, 0xDD}, {0x0447, 0xDF},
        {0x0448, 0xE6}, {0x0449, 0xE7}, {0x044A, 0xAA}, {0x044B, 0xAB},
        {0x044C, 0xAC}, {0x044D, 0xAE}, {0x044E, 0xAF}, {0x044F, 0xB0},
        {0x0407, 0xD8}, {0x0457, 0x8B}, {0x0456, 0x69}, {0x0406, 0x49},
        {0x0404, 0x0F}, {0x0454, 0x10}, {0x0490, 0x11}, {0x0491, 0x12},
        {0x2019, 0x27}, {0x2014, 0x2D}, {0x0451, 0x89}, {0x0401, 0xD3},
        {0x00C7, 0x80}, {0x00FC, 0x81}, {0x00E9, 0x82}, {0x00E2, 0x83},
        {0x00E4, 0x84}, {0x00E0, 0x85}, {0x00E5, 0x86}, {0x00E7, 0x87},
        {0x00EA, 0x88}, {0x00EB, 0x89}, {0x00E8, 0x8A}, {0x00EF, 0x8B},
        {0x00EE, 0x8C}, {0x00EC, 0x8D}, {0x00C4, 0x8E}, {0x00C5, 0x8F},
        {0x00C9, 0x90}, {0x00E6, 0x91}, {0x00F4, 0x93}, {0x00F6, 0x94},
        {0x00F2, 0x95}, {0x00FB, 0x96}, {0x00F9, 0x97}, {0x00FF, 0x98},
        {0x00D6, 0x99}, {0x00DC, 0x9A}, {0x00E1, 0xA0}, {0x00ED, 0xA1},
        {0x00F3, 0xA2}, {0x00FA, 0xA3}, {0x00F1, 0xA4}, {0x00D1, 0xA5},
        {0x00BF, 0xA8}, {0x00A1, 0xAD}, {0x00C1, 0xB5}, {0x00C2, 0xB6},
        {0x00C0, 0xB7}, {0x00E3, 0xC6}, {0x00C3, 0xC7}, {0x00CA, 0xD2},
        {0x00CB, 0xD3}, {0x00C8, 0xD4}, {0x00CD, 0xD6}, {0x00CE, 0xD7},
        {0x00CF, 0xD8}, {0x00CC, 0xDE}, {0x00D3, 0xE0}, {0x00DF, 0xE1},
        {0x00D4, 0xE2}, {0x00D2, 0xE3}, {0x00F5, 0xE4}, {0x00D5, 0xE5},
        {0x00DA, 0xE9}, {0x00DB, 0xEA}, {0x00D9, 0xEB}, {0x00FD, 0xEC},
        {0x00DD, 0xED}, {0x0105, 0xEE}, {0x0104, 0xEF}, {0x0107, 0xF0},
        {0x0106, 0xF1}, {0x0119, 0xF2}, {0x0118, 0xF3}, {0x0142, 0xF4},
        {0x0141, 0xF5}, {0x0144, 0xF6}, {0x0143, 0xF7}, {0x015B, 0xF8},
        {0x015A, 0xF9}, {0x017A, 0xFA}, {0x0179, 0xFB}, {0x017C, 0xFC},
        {0x017B, 0xFD},
        {0x011B, 0x65}, {0x010D, 0x63}, {0x010F, 0x64}, {0x0159, 0x72},
        {0x0161, 0x73}, {0x017E, 0x7A}, {0x0165, 0x74}, {0x0148, 0x6E},
        {0x016F, 0x75}, {0x010C, 0x43}, {0x010E, 0x44}, {0x0158, 0x52},
        {0x0160, 0x53}, {0x017D, 0x5A}, {0x0164, 0x54}, {0x0147, 0x4E},
        {0x016E, 0x55}, {0x011A, 0x45},
    };
    for (size_t i = 0; i < sizeof(codepage_map) / sizeof(codepage_map[0]); ++i)
    {
        if (codepage_map[i].unicode == codepoint)
            return codepage_map[i].byte;
    }
    ERRORLOG("Warning: No mapping for Unicode codepoint U+%04lX in internal codepage; using '?'", codepoint);
    return '?';
}

static void convert_utf8_to_internal_codepage(const char *src, char *dst, size_t dst_size)
{
    size_t out_len = 0;
    const unsigned char *text = (const unsigned char *)src;

    while (*text != '\0' && out_len + 1 < dst_size)
    {
        unsigned long codepoint;
        size_t seq_len;
        if (*text < 0x80)
        {
            codepoint = *text;
            seq_len = 1;
        }
        else if ((*text & 0xE0) == 0xC0 && (text[1] & 0xC0) == 0x80)
        {
            codepoint = ((text[0] & 0x1F) << 6) | (text[1] & 0x3F);
            seq_len = 2;
        }
        else if ((*text & 0xF0) == 0xE0 && (text[1] & 0xC0) == 0x80 && (text[2] & 0xC0) == 0x80)
        {
            codepoint = ((text[0] & 0x0F) << 12) | ((text[1] & 0x3F) << 6) | (text[2] & 0x3F);
            seq_len = 3;
        }
        else if ((*text & 0xF8) == 0xF0 && (text[1] & 0xC0) == 0x80 && (text[2] & 0xC0) == 0x80 && (text[3] & 0xC0) == 0x80)
        {
            codepoint = ((text[0] & 0x07) << 18) | ((text[1] & 0x3F) << 12) | ((text[2] & 0x3F) << 6) | (text[3] & 0x3F);
            seq_len = 4;
        }
        else
        {
            dst[out_len++] = '?';
            text += 1;
            continue;
        }

        dst[out_len++] = (char)convert_codepoint_to_internal_byte(codepoint);
        text += seq_len;
    }
    dst[out_len] = '\0';
}

static int translation_section_visitor(const VALUE *key, VALUE *section, void *ctx)
{
    if (translation_count >= MAX_TRANSLATION_ENTRIES)
        return 1; // stop walking — table is full

    if (value_type(section) != VALUE_DICT)
        return 0; // skip non-table top-level entries

    const char *alias = value_string(key);
    if (!alias)
        return 0;

    WalkContext *wctx = (WalkContext *)ctx;

    // Try the requested language first, fall back to English
    const char *text = NULL;
    VALUE *lang_val = value_dict_get(section, wctx->language_code);
    if (lang_val && value_type(lang_val) == VALUE_STRING)
        text = value_string(lang_val);

    if (!text)
    {
        VALUE *eng_val = value_dict_get(section, "ENG");
        if (eng_val && value_type(eng_val) == VALUE_STRING)
            text = value_string(eng_val);
    }

    if (!text)
        text = "";

    TranslationEntry *entry = &translation_table[translation_count];

    strncpy(entry->alias, alias, MAX_ALIAS_LEN - 1);
    entry->alias[MAX_ALIAS_LEN - 1] = '\0';

    size_t len = strlen(text);
    entry->text = (char *)calloc(len + 1, 1);
    if (entry->text)
        convert_utf8_to_internal_codepage(text, entry->text, len + 1);

    translation_count++;
    return 0;
}

void clear_translation_table(void)
{
    for (int32_t i = 0; i < translation_count; i++)
    {
        free(translation_table[i].text);
        translation_table[i].text = NULL;
        translation_table[i].alias[0] = '\0';
    }
    translation_count = 0;
}

//will be read first, language doesn't change during run, so any other language is irrelevant
void read_translation_file(const char* filepath, const char* language_code)
{
    long len = LbFileLengthRnc(filepath);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        WARNMSG("Translation file \"%s\" does not exist or is too small.", filepath);
        return;
    }

    char *buf = (char *)calloc(len + 256, 1);
    if (!buf)
    {
        ERRORLOG("Out of memory loading translation file \"%s\".", filepath);
        return;
    }

    long fsize = LbFileLoadAt(filepath, buf);
    if (fsize < len)
    {
        WARNMSG("Failed to read translation file \"%s\".", filepath);
        free(buf);
        return;
    }

    char errbuf[256];
    VALUE root;
    if (toml_parse(buf, errbuf, sizeof(errbuf), &root))
    {
        WARNMSG("Failed to parse translation file \"%s\": %s", filepath, errbuf);
        free(buf);
        return;
    }
    free(buf);

    WalkContext ctx;
    ctx.language_code = language_code;
    value_dict_walk_sorted(&root, translation_section_visitor, &ctx);

    value_fini(&root);
}

//rest of the code will reference it by number, when loading I'll get the allias so eg HELLO_WORLD would be 1
int32_t get_string_id_by_alias(const char* alias)
{
    JUSTLOG("Looking up translation alias \"%s\"", alias);
    for (int32_t i = 0; i < translation_count; i++)
    {
        if (strcmp(translation_table[i].alias, alias) == 0)
        {
            JUSTLOG("Found alias \"%s\" at ID %d", alias, STRINGS_MAX + GUI_STRINGS_COUNT + i + 1);
            return STRINGS_MAX + GUI_STRINGS_COUNT + i + 1; // 1-based ID
        }
    }
    JUSTLOG("Alias \"%s\" not found in translation table", alias);
    return -1;
}

//so that 1 that's tied to HELLO_WORLD would turn into "Hello world" here
const char* get_translation_file_string(int32_t string_id)
{
    if (string_id < STRINGS_MAX + GUI_STRINGS_COUNT + 1 || string_id > STRINGS_MAX + GUI_STRINGS_COUNT + translation_count)
        return "oh_crap_invalid_string_id";
    const char *text = translation_table[string_id - STRINGS_MAX - GUI_STRINGS_COUNT - 1].text;
    return text ? text : "oh_crap_null_string";
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
