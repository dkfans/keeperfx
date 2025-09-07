#include "pre_inc.h"

#include "config_modules.h"

#include "bflib_dernc.h"
#include "bflib_fileio.h"

#include "config.h"
#include "game_legacy.h"

#include "post_inc.h"



#ifdef __cplusplus
extern "C" {
#endif



static const struct NamedField modules_named_fields[] = {
    //name                     //pos    //field                                                    //default //min //max //NamedCommand
    {"NAME",                     0, field(game.conf.module_conf.mod_item[0].name),                 0, LONG_MIN,ULONG_MAX, NULL,         value_name,     assign_null},
    {"LOADPERIOD",               0, field(game.conf.module_conf.mod_item[0].load_period),          0, LONG_MIN,ULONG_MAX, NULL,         value_default,  assign_default},
    {"PRIORITY",                 0, field(game.conf.module_conf.mod_item[0].priority),             0, LONG_MIN,ULONG_MAX, NULL,         value_default,  assign_default},
    {"DISABLE",                  0, field(game.conf.module_conf.mod_item[0].disable),              0, LONG_MIN,ULONG_MAX, NULL,         value_default,  assign_default},
    {NULL},
};

static const struct NamedFieldSet modules_named_fields_set = {
    &game.conf.module_conf.mod_item_cnt,
    "mod",
    modules_named_fields,
    NULL,
    MODULE_ITEM_MAX,
    sizeof(game.conf.module_conf.mod_item[0]),
    game.conf.module_conf.mod_item,
};

static int module_compare_fn(const void *ptr_a, const void *ptr_b)
{
    struct ModuleConfigItem *a = (struct ModuleConfigItem*)ptr_a;
    struct ModuleConfigItem *b = (struct ModuleConfigItem*)ptr_b;
    return a->priority - b->priority;
}

TbBool load_module_config_file()
{
    SYNCDBG(8, "Starting");

    memset(&game.conf.module_conf, 0, sizeof(game.conf.module_conf));

    unsigned short flags = CnfLd_Standard|CnfLd_IgnoreErrors;

    const char *sname = MODULE_DIR_NAME "/" MODULE_CFG_FILE_NAME;
    const char *fname = prepare_file_path(FGrp_Main, sname);

    long len = LbFileLengthRnc(fname);
    if (len < 2)
    {
        if (!flag_is_set(flags,CnfLd_IgnoreErrors))
          ERRORLOG("Module file \"%s\" doesn't exist or is too small.",fname);
        return false;
    }
    if (len > 65536)
    {
        ERRORLOG("Module file \"%s\" is too large.",fname);
        return false;
    }
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
      return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    if (len>0)
    {
        parse_named_field_blocks(buf, len, fname, flags, &modules_named_fields_set);
    }
    free(buf);

    if (game.conf.module_conf.mod_item_cnt > 1)
        qsort(game.conf.module_conf.mod_item, game.conf.module_conf.mod_item_cnt, sizeof(game.conf.module_conf.mod_item[0]), &module_compare_fn);

    int i;
    for (i=0; i<game.conf.module_conf.mod_item_cnt; i++)
    {
        struct ModuleConfigItem *mod_item = game.conf.module_conf.mod_item + i;
        const char* fname1 = prepare_file_path_mod(MODULE_DIR_NAME, FGrp_Main, mod_item->name);
        if (LbFileExists(fname1))
            mod_item->exist = 1;
    }

    return true;
}



#ifdef __cplusplus
}
#endif

