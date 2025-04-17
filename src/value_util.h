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

static inline MapCoord value_read_stl_coord(VALUE *value)
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
int value_parse_anim(VALUE *value);
TbBool load_toml_file(const char *fname,VALUE *value, unsigned short flags);

#define KEY_SIZE 64

#define CONDITIONAL_ASSIGN_INT(section,name,field) \
{\
    VALUE *val = value_dict_get(section,name);\
    if (value_type(val) == VALUE_INT32)\
        field = value_int32(val);\
}

#define CONDITIONAL_ASSIGN_INT_SCALED(section,name,field, scale) \
{\
    VALUE *val = value_dict_get(section,name);\
    if (value_type(val) == VALUE_INT32)\
        field = scale * value_int32(val);\
}

#define CONDITIONAL_ASSIGN_ANIMID(section,name,field) \
{\
    VALUE *val = value_dict_get(section,name);\
    if (value_type(val) == VALUE_INT32 || value_type(val) == VALUE_STRING )\
        field = value_parse_anim(val);\
}

#define CONDITIONAL_ASSIGN_CLASS(section,name,field) \
{\
    VALUE *val = value_dict_get(section,name);\
    if (value_type(val) == VALUE_INT32 || value_type(val) == VALUE_STRING )\
        field = value_parse_class(val);\
}

#define CONDITIONAL_ASSIGN_MODEL(section,name,field,class_id) \
{\
    VALUE *val = value_dict_get(section,name);\
    if (value_type(val) == VALUE_INT32 || value_type(val) == VALUE_STRING )\
        field = value_parse_model(class_id,val);\
}

#define CONDITIONAL_ASSIGN_BOOL(section,name,field) \
{\
    VALUE *val = value_dict_get(section,name);\
    if (value_type(val) == VALUE_BOOL)\
        field = value_bool(val);\
    else if (value_type(val) == VALUE_INT32)\
        field = value_int32(val);\
}

#define CONDITIONAL_ASSIGN_ARR2_INT(section,name,field1,field2) \
{\
    VALUE *val_arr = value_dict_get(section,name);\
    if (value_type(val_arr) == VALUE_ARRAY)\
    {\
        field1 = value_int32(value_array_get(val_arr, 0));\
        field2 = value_int32(value_array_get(val_arr, 1));\
    }\
}

#define CONDITIONAL_ASSIGN_ARR3_INT(section,name,field1,field2,field3) \
{\
    VALUE *val_arr = value_dict_get(section,name);\
    if (value_type(val_arr) == VALUE_ARRAY)\
    {\
        field1 = value_int32(value_array_get(val_arr, 0));\
        field2 = value_int32(value_array_get(val_arr, 1));\
        field3 = value_int32(value_array_get(val_arr, 2));\
    }\
}

#define CONDITIONAL_ASSIGN_EFFECT_OR_EL_MODEL(section,name,field) \
{\
    VALUE *val = value_dict_get(section,name);\
    if (value_type(val) == VALUE_INT32)\
    {\
        field = value_int32(val);\
    }\
    else\
    if (value_type(val) == VALUE_STRING)\
    {\
        field = effect_or_effect_element_id(value_string(val));\
    }\
}

#define CONDITIONAL_ASSIGN_SPELL(section,name,field) \
{\
    VALUE *val = value_dict_get(section,name);\
    if (value_type(val) == VALUE_INT32)\
    {\
        field = value_int32(val);\
    }\
    else\
    if (value_type(val) == VALUE_STRING)\
    {\
        field = get_id(spell_desc,value_string(val));\
    }\
}

#define SET_NAME(section,desc,namefield) \
{\
    const char* name = value_string(value_dict_get(section, "Name"));\
    if(name != NULL)\
    {\
        if(strlen(name) > COMMAND_WORD_LEN - 1 )\
        {\
            ERRORLOG("name (%s) to long max %d chars", name,COMMAND_WORD_LEN - 1);\
            break;\
        }\
        strncpy(namefield,name,COMMAND_WORD_LEN);\
        desc[id].name = namefield;\
        desc[id].num = id;\
    }\
    if ((flags & CnfLd_ListOnly) != 0)\
    {\
        continue;\
    }\
}
