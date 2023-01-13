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