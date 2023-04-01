/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
#pragma once
#include "bflib_basics.h"

#include "globals.h"

#include <toml.h>

static inline TbBool value_coerce_bool(VALUE *value)
{
    return (value_type(value) == VALUE_BOOL && value_bool(value)) ||
           (value_type(value) == VALUE_INT32 && value_int32(value));
}

static inline unsigned short value_read_stl_coord(VALUE *value)
{
    if (value_type(value) == VALUE_ARRAY)
    {
        int stl = value_int32(value_array_get(value, 0));
        int sub_stl = value_int32(value_array_get(value, 1));
        if ((stl == -1) || (sub_stl == -1))
        {
            WARNMSG("Invalid coords");
            return 0;
        }
        return (stl << 8) | (sub_stl & 0xFF);
    }
    else
    {
        WARNMSG("Invalid coords");
        return 0;
    }
}

int value_parse_class(VALUE *value);
int value_parse_model(int oclass, VALUE *value);
