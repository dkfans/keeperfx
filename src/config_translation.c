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
        memcpy(entry->text, text, len);

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
