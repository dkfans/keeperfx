/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
// Created by Sim on 12/31/22.

#include "pre_inc.h"
#include "value_util.h"
#include "config.h"
#include "config_creature.h"
#include "config_effects.h"
#include "config_magic.h"
#include "config_objects.h"
#include "config_trapdoor.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "value_util.h"
#include "custom_sprites.h"
#include "thing_objects.h"

#include "thing_list.h"

#include <string.h>
#include "post_inc.h"

TbBool load_toml_file(const char *fname,VALUE *value, unsigned short flags)
{
    SYNCDBG(5,"Starting");
    int32_t len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if(!(flags & CnfLd_IgnoreErrors))
            WARNMSG("file \"%s\" doesn't exist or is too small.",fname);
        return false;
    }
    char* buf = (char*)calloc(len + 256, 1);
    if (!buf) return false;
    // Loading file data
    int32_t fsize = LbFileLoadAt(fname, buf);

    if (fsize < len)
    {
        WARNMSG("failed to read file \"%s\".",fname);
        free(buf);
        return false;
    }

    char err[255];

    if (toml_parse((char*)buf, err, sizeof(err), value))
    {
        WARNMSG("Unable to load %s file\n %s", fname, err);
        free(buf);
        return false;
    }
    free(buf);
    return true;
}

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
    if (value_type(value) != VALUE_STRING)
        return -1;
    const char *name = value_string(value);
    switch (oclass)
    {
    case TCls_Object:
    case TCls_AmbientSnd:
        return get_id(object_desc, name);
    case TCls_Shot:
        return get_id(shot_desc, name);
    case TCls_EffectElem:
        return get_id(effectelem_desc, name);
    case TCls_DeadCreature:
    case TCls_Creature:
        return get_id(creature_desc, name);
    case TCls_Effect:
        return get_id(effect_desc, name);
    case TCls_EffectGen:
        return get_id(effectgen_desc, name);
    case TCls_Trap:
        return get_id(trap_desc, name);
    case TCls_Door:
        return get_id(door_desc, name);
    }
    return -1;
}

int value_parse_anim(VALUE *value)
{
    if (value_type(value) == VALUE_INT32)
        return value_int32(value);
    else if (value_type(value) == VALUE_STRING)
    {
        const char *tst = value_string(value);
        struct ObjectConfigStats obj_tmp;
        return get_anim_id(tst, &obj_tmp);
    }
    return -1;
}
