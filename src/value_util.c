/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
// Created by Sim on 12/31/22.

#include "pre_inc.h"
#include "value_util.h"

#include "thing_list.h"

#include <string.h>
#include "post_inc.h"

int value_parse_class(VALUE *value)
{
    if (value_type(value) == VALUE_INT32)
        return value_int32(value);
    else if (value_type(value) == VALUE_STRING)
    {
        const char *tst = value_string(value);
        if (0 == strcmp(tst, "Object"))
            return TCls_Object;
        else if (0 == strcmp(tst, "Shot"))
            return TCls_Shot;
        else if (0 == strcmp(tst, "EffectElem"))
            return TCls_EffectElem;
        else if (0 == strcmp(tst, "DeadCreature"))
            return TCls_DeadCreature;
        else if (0 == strcmp(tst, "Creature"))
            return TCls_Creature;
        else if (0 == strcmp(tst, "Effect"))
            return TCls_Effect;
        else if (0 == strcmp(tst, "EffectGen"))
            return TCls_EffectGen;
        else if (0 == strcmp(tst, "Trap"))
            return TCls_Trap;
        else if (0 == strcmp(tst, "Door"))
            return TCls_Door;
        else if (0 == strcmp(tst, "AmbientSnd"))
            return TCls_AmbientSnd;
        else if (0 == strcmp(tst, "CaveIn"))
            return TCls_CaveIn;
        return -1;
    }
    return -1;
}

int value_parse_model(int oclass, VALUE *value)
{
    if (value_type(value) == VALUE_INT32)
        return value_int32(value);
    // TODO: model names for different classes
    return -1;
}

static char* value_dump_impl(const VALUE *value, char *buf, char *end, int tabs);

struct Opts
{
    char *buf;
    char *end;
    int tabs;
};

static int value_visit(const VALUE *key, VALUE *val, void *arg)
{
    struct Opts *opts = arg;
    opts->buf = value_dump_impl(key, opts->buf, opts->end, 0);
    *(opts->buf++) = '='; *opts->buf = 0;
    opts->buf = value_dump_impl(val, opts->buf, opts->end, opts->tabs);
    *(opts->buf++) = '\n'; *opts->buf = 0;
    return 0;
}

static char * value_dump_impl(const VALUE *value, char *buf, char *end, int tabs)
{
#define ADD_CHAR(x) \
    *(dst++) = x;
#define ADD_TABS \
    for (int i = 0; i < tabs; i++) \
    { \
        ADD_CHAR(' ') \
        ADD_CHAR(' ') \
    } \
    *dst = 0;

    struct Opts opts;
    char *dst = buf;
    switch (value_type(value))
    {
        case VALUE_NULL:
            break;
        case VALUE_DICT:
            ADD_TABS;
            ADD_CHAR('{'); ADD_CHAR('\n')
            opts.buf = dst;
            opts.end = end;
            opts.tabs = tabs;
            value_dict_walk_sorted(value, &value_visit, &opts);
            dst = opts.buf;
            ADD_TABS;
            ADD_CHAR('}'); ADD_CHAR('\n')
            break;
        case VALUE_ARRAY:
            ADD_TABS;
            ADD_CHAR('['); ADD_CHAR('\n')
            for (int i = 0; i < value_array_size(value); i++)
            {
                value_dump_impl(value_array_get(value, i), dst, end, tabs + 1);
            }
            ADD_TABS;
            ADD_CHAR(']'); ADD_CHAR('\n')
            break;
        case VALUE_STRING:
            ADD_TABS;
            if (dst + value_string_length(value) >= end)
            {
                // TODO: Overflow
                return dst;
            }
            ADD_CHAR('"'); *dst = 0;
            strcat(dst, value_string(value));
            dst += strlen(dst);
            ADD_CHAR('"');
            break;
        case VALUE_INT32:
            ADD_TABS;
            dst += sprintf(dst, "%d", value_int32(value));
            break;
        case VALUE_BOOL:
            ADD_TABS;
            if (value_bool(value))
            {
                strcat(dst, "true");
            }
            else
            {
                strcat(dst, "false");
            }
            dst += strlen(dst);
        default:
            break;
    }
    *dst = 0;
    return dst;
}

void value_dump(VALUE *value, char *buf, char *end)
{
    *buf = 0;
    value_dump_impl(value, buf, end, 0);
}