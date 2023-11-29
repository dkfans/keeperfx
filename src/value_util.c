/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
// Created by Sim on 12/31/22.

#include "pre_inc.h"
#include "value_util.h"
#include "config.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "value_util.h"

#include "thing_list.h"

#include <string.h>
#include "post_inc.h"

TbBool load_toml_file(const char *textname, const char *fname,VALUE *value)
{
    SYNCDBG(5,"Starting");
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        WARNMSG("The %s file \"%s\" doesn't exist or is too small.",textname,fname);
        return false;
    }
    char* buf = (char*)LbMemoryAlloc(len + 256);
    if (buf == false)
        return false;
    // Loading file data
    long fsize = LbFileLoadAt(fname, buf);

    if (fsize < len)
    {
        WARNMSG("failed to read the %s file \"%s\".",textname,fname);
        LbMemoryFree(buf);
        return false;
    }
    
    if (buf == false)
        return false;
    char err[255];
    

    if (toml_parse((char*)buf, err, sizeof(err), value))
    {
        WARNMSG("Unable to load %s file\n %s", fname, err);
        LbMemoryFree(buf);
        return false;
    }
    LbMemoryFree(buf);
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
    // TODO: model names for different classes
    return -1;
}