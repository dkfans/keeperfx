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
#include "bflib_sprfnt.h"
#include "config.h"
#include "config_strings.h"
#include "config_keeperfx.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define MAX_ALIAS_LEN           64

typedef struct {
    char  alias[MAX_ALIAS_LEN];
    char *text;
} TranslationEntry;

static TranslationEntry translation_table[MAX_TOML_TRANSLATION_ENTRIES];
static int32_t          translation_count = 0;

static TbBool load_translation_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData keeper_translation_file_data = {
    .filename = "translation.toml",
    .load_func = load_translation_config_file,
    .pre_load_func = NULL,
    .post_load_func = NULL,
};

static const char *get_language_value(VALUE *section, uint8_t lang_id)
{
    const char *language_code = get_conf_parameter_text(lang_type, lang_id);
    VALUE *lang_val = value_dict_get(section, language_code);
    if (lang_val && value_type(lang_val) == VALUE_STRING)
        return value_string(lang_val);

    char fuzzy_language_code[64];
    fuzzy_language_code[0] = '_';
    strcpy(fuzzy_language_code + 1, language_code);
    lang_val = value_dict_get(section, fuzzy_language_code);
    if (lang_val && value_type(lang_val) == VALUE_STRING)
        return value_string(lang_val);

    return NULL;
}

static void add_entry_to_translation_table(const char *alias, const char *text)
{
    int32_t entry_index = -1;
    TbBool is_new_entry = true;
    for (int32_t i = 0; i < translation_count; i++)
    {
        if (strcmp(translation_table[i].alias, alias) == 0)
        {
            entry_index = i;
            is_new_entry = false;
            break;
        }
    }

    if (entry_index == -1)
    {
        entry_index = translation_count;
        translation_count++;
    }

    TranslationEntry *entry = &translation_table[entry_index];
    if (!is_new_entry)
    {
        free(entry->text);
        entry->text = NULL;
    }

    strncpy(entry->alias, alias, MAX_ALIAS_LEN - 1);
    entry->alias[MAX_ALIAS_LEN - 1] = '\0';

    size_t len = strlen(text);


    entry->text = (char *)calloc(len + 1, 1);
    if (!entry->text)
    {
        ERRORLOG("Out of memory allocating translation text for alias \"%s\".", alias);
        return;
    }
    strncpy(entry->text, text, len + 1);
}

static int translation_section_visitor(const VALUE *key, VALUE *section, void *ctx)
{
    int current_language_id = install_info.lang_id;
    if (translation_count >= MAX_TOML_TRANSLATION_ENTRIES)
        return 1; // stop walking — table is full

    if (value_type(section) != VALUE_DICT)
        return 0; // skip non-table top-level entries

    const char *alias = value_string(key);
    if (!alias)
        return 0;

    const char *text = get_language_value(section, current_language_id);

    if (!text && current_language_id == Lang_ChineseTra)
    {
        text = get_language_value(section, Lang_ChineseInt);
    }
    if (!text) text = get_language_value(section, Lang_English);
    
    if (!text) text = alias;

    add_entry_to_translation_table(alias, text);

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

static TbBool load_translation_config_file(const char* filepath, unsigned short flags)
{
    if (!flag_is_set(flags, CnfLd_AcceptPartial))
    {
        clear_translation_table();
    }

    long len = LbFileLengthRnc(filepath);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        SYNCDBG(17,"Translation file \"%s\" does not exist or is too small.", filepath);
        return false;
    }

    char *buf = (char *)calloc(len + 256, 1);
    if (!buf)
    {
        ERRORLOG("Out of memory loading translation file \"%s\".", filepath);
        return false;
    }

    long fsize = LbFileLoadAt(filepath, buf);
    if (fsize < len)
    {
        WARNMSG("Failed to read translation file \"%s\".", filepath);
        free(buf);
        return false;
    }

    char errbuf[256];
    VALUE root;
    if (toml_parse(buf, errbuf, sizeof(errbuf), &root))
    {
        WARNMSG("Failed to parse translation file \"%s\": %s", filepath, errbuf);
        free(buf);
        return false;
    }
    free(buf);

    value_dict_walk_sorted(&root, translation_section_visitor, NULL);

    value_fini(&root);
    return true;
}

TextStringId get_string_id_by_alias(const char* alias)
{
    if (parameter_is_number(alias))
    {
        int32_t id = atoi(alias);
        if (id <= STRINGS_MAX)
            return id;
        ERRORLOG("Invalid string ID \"%s\".", alias);
        return -1;
    }
    for (int32_t i = 0; i < translation_count; i++)
    {
        if (strcmp(translation_table[i].alias, alias) == 0)
        {
            return TRANSLATION_STRINGS_START + i;
        }
    }
    ERRORLOG("No translation entry found for alias \"%s\".", alias);
    return -1;
}

const char* get_translation_file_string(TextStringId string_id)
{
    if (string_id < TRANSLATION_STRINGS_START || string_id >= TRANSLATION_STRINGS_START + translation_count)
        return "oh_crap_invalid_string_id";
    const char *text = translation_table[string_id - TRANSLATION_STRINGS_START].text;
    return text ? text : "oh_crap_null_string";
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
