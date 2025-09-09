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



static TbBool parse_module_block(char *buf, long len, const char *block_name, struct ModuleConfigItem* mod_items, long *mod_cnt, long mod_max)
{
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_name);
    if (k < 0)
    {
        return false;
    }

    while (pos<len)
    {
        if (*mod_cnt >= mod_max)
            break;
        char line_buf[COMMAND_WORD_LEN] = {0};
        int line_len = get_conf_line(buf, &pos, len, line_buf, COMMAND_WORD_LEN);
        if (line_len < 0)
            break;
        if (line_len > 0)
        {
            struct ModuleConfigItem* mod_item = mod_items + (*mod_cnt);
            if (line_len < sizeof(mod_item->name))
            {
                memcpy(mod_item->name, line_buf, line_len);
                (*mod_cnt)++;
            }
        }
        skip_conf_to_next_line(buf,&pos,len);
    }

    return true;
}

static void check_module_exist(struct ModuleConfigItem *mod_items, long mod_cnt)
{
    for (long i=0; i<mod_cnt; i++)
    {
        struct ModuleConfigItem *mod_item = mod_items + i;
	if (mod_item->name[0] == 0)
            continue;

        const char* fname = prepare_file_path_mod(MODULE_DIR_NAME, FGrp_Main, mod_item->name);
        if (fname[0] == 0 || !LbFileExists(fname))
            continue;
        mod_item->exist_mod = 1;


        char mod_dir[256] = {0};
        sprintf(mod_dir, "%s/%s", MODULE_DIR_NAME, mod_item->name);


        fname = prepare_file_path_mod(mod_dir, FGrp_FxData, NULL);
        if (fname[0] != 0 && LbFileExists(fname))
            mod_item->exist_fx_data = 1;

        fname = prepare_file_path_mod(mod_dir, FGrp_CmpgConfig, NULL);
        if (fname[0] != 0 && LbFileExists(fname))
            mod_item->exist_cmpg_config = 1;

        fname = prepare_file_path_mod(mod_dir, FGrp_CmpgLvls, NULL);
        if (fname[0] != 0 && LbFileExists(fname))
            mod_item->exist_cmpg_lvls = 1;


        fname = prepare_file_path_mod(mod_dir, FGrp_CrtrData, NULL);
        if (fname[0] != 0 && LbFileExists(fname))
            mod_item->exist_crtr_data = 1;

        fname = prepare_file_path_mod(mod_dir, FGrp_CmpgCrtrs, NULL);
        if (fname[0] != 0 && LbFileExists(fname))
            mod_item->exist_cmpg_crtrs = 1;

    }
}

TbBool load_module_order_config_file()
{
    SYNCDBG(8, "Starting");

    memset(&game.conf.module_conf, 0, sizeof(game.conf.module_conf));

    const char *sname = MODULE_DIR_NAME "/" MODULE_LOAD_ORDER_FILE_NAME;
    const char *fname = prepare_file_path(FGrp_Main, sname);

    long len = LbFileLengthRnc(fname);
    if (len < 2)
    {
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
        parse_module_block(buf, len, MODULE_AFTER_BASE_BLOCK_NAME, game.conf.module_conf.after_base_item, &game.conf.module_conf.after_base_cnt, MODULE_ITEM_MAX);
        parse_module_block(buf, len, MODULE_AFTER_CAMPAIGN_BLOCK_NAME, game.conf.module_conf.after_campaign_item, &game.conf.module_conf.after_campaign_cnt, MODULE_ITEM_MAX);
        parse_module_block(buf, len, MODULE_AFTER_MAP_BLOCK_NAME, game.conf.module_conf.after_map_item, &game.conf.module_conf.after_map_cnt, MODULE_ITEM_MAX);
    }
    free(buf);

    check_module_exist(game.conf.module_conf.after_base_item, game.conf.module_conf.after_base_cnt);
    check_module_exist(game.conf.module_conf.after_campaign_item, game.conf.module_conf.after_campaign_cnt);
    check_module_exist(game.conf.module_conf.after_map_item, game.conf.module_conf.after_map_cnt);

    return true;
}



#ifdef __cplusplus
}
#endif

