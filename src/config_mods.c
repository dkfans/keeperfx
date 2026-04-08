#include "pre_inc.h"

#include "config_mods.h"

#include "bflib_dernc.h"
#include "bflib_fileio.h"

#include "config.h"
#include "game_legacy.h"

#include "post_inc.h"



#ifdef __cplusplus
extern "C" {
#endif


struct ModsConfig mods_conf = {0};

static TbBool parse_block_mods(char *buf, long len, const char *block_name, struct ModConfigItem* mod_items, int32_t *mod_cnt, long mod_max)
{
    int32_t pos = 0;
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
            struct ModConfigItem* mod_item = mod_items + (*mod_cnt);
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

static void recheck_block_mod_list_exist(struct ModConfigItem *mod_items, long mod_cnt, const char *block_name)
{
    for (long i=0; i<mod_cnt; i++)
    {
        struct ModConfigItem *mod_item = mod_items + i;
        struct ModExistState *mod_state = &mod_item->state;
        memset(mod_state, 0, sizeof(*mod_state));
        if (mod_item->name[0] == 0)
            continue;

        const char* fname = prepare_file_path_mod(MODS_DIR_NAME, FGrp_Main, mod_item->name);
        if (fname[0] == 0 || !LbFileExists(fname))
        {
            WARNMSG("The '%s' mod configured in '%s' section does not exist.", mod_item->name, block_name);
            continue;
        }
        mod_state->mod_dir = 1;


        const char *str_sep = "";
        char mod_dir[256] = {0}, config_dirs[2048] = {0}, main_dir[2048] = {0};
        sprintf(mod_dir, "%s/%s", MODS_DIR_NAME, mod_item->name);
        prepare_file_path_buf_mod(main_dir, sizeof(main_dir), mod_dir, FGrp_Main, NULL);
        int main_len = strlen(main_dir);

        fname = prepare_file_path_mod(mod_dir, FGrp_FxData, NULL);
        if (fname[0] != 0 && LbFileExists(fname))
        {
            mod_state->fx_data = 1;

            strcat(config_dirs, str_sep);
            str_sep = ", ";
            if (memcmp(main_dir, fname, main_len) == 0)
                strcat(config_dirs, fname+main_len+1);
            else
                strcat(config_dirs, "FGrp_FxData");
        }

        fname = prepare_file_path_mod(mod_dir, FGrp_StdData, NULL);
        if (fname[0] != 0 && LbFileExists(fname))
        {
            mod_state->std_data = 1;

            strcat(config_dirs, str_sep);
            str_sep = ", ";
            if (memcmp(main_dir, fname, main_len) == 0)
                strcat(config_dirs, fname+main_len+1);
            else
                strcat(config_dirs, "FGrp_StdData");
        }

        fname = prepare_file_path_mod(mod_dir, FGrp_CmpgConfig, NULL);
        if (fname[0] != 0 && LbFileExists(fname))
        {
            mod_state->cmpg_config = 1;

            strcat(config_dirs, str_sep);
            str_sep = ", ";
            if (memcmp(main_dir, fname, main_len) == 0)
                strcat(config_dirs, fname+main_len+1);
            else
                strcat(config_dirs, "FGrp_CmpgConfig");
        }

        fname = prepare_file_path_mod(mod_dir, FGrp_CmpgLvls, NULL);
        if (fname[0] != 0 && LbFileExists(fname))
        {
            mod_state->cmpg_lvls = 1;

            strcat(config_dirs, str_sep);
            str_sep = ", ";
            if (memcmp(main_dir, fname, main_len) == 0)
                strcat(config_dirs, fname+main_len+1);
            else
                strcat(config_dirs, "FGrp_CmpgLvls");
        }


        fname = prepare_file_path_mod(mod_dir, FGrp_CrtrData, NULL);
        if (fname[0] != 0 && LbFileExists(fname))
        {
            mod_state->crtr_data = 1;

            strcat(config_dirs, str_sep);
            str_sep = ", ";
            if (memcmp(main_dir, fname, main_len) == 0)
                strcat(config_dirs, fname+main_len+1);
            else
                strcat(config_dirs, "FGrp_CrtrData");
        }

        fname = prepare_file_path_mod(mod_dir, FGrp_CmpgCrtrs, NULL);
        if (fname[0] != 0 && LbFileExists(fname))
        {
            mod_state->cmpg_crtrs = 1;

            strcat(config_dirs, str_sep);
            str_sep = ", ";
            if (memcmp(main_dir, fname, main_len) == 0)
                strcat(config_dirs, fname+main_len+1);
            else
                strcat(config_dirs, "FGrp_CmpgCrtrs");
        }

        if (config_dirs[0] == 0)
            WARNMSG("The '%s' mod configured in '%s' section exists but has no valid configuration.", mod_item->name, block_name);
        else
            SYNCLOG("The '%s' mod configured in '%s' section exists and contains valid configuration: %s", mod_item->name, block_name, config_dirs);
    }
}

void recheck_all_mod_exist()
{
    SYNCDBG(8,"Check mods starts");
    recheck_block_mod_list_exist(mods_conf.after_base_item, mods_conf.after_base_cnt, MODS_AFTER_BASE_BLOCK_NAME);
    recheck_block_mod_list_exist(mods_conf.after_campaign_item, mods_conf.after_campaign_cnt, MODS_AFTER_CAMPAIGN_BLOCK_NAME);
    recheck_block_mod_list_exist(mods_conf.after_map_item, mods_conf.after_map_cnt, MODS_AFTER_CAMPAIGN_BLOCK_NAME);
    SYNCDBG(8,"Check mods end");
}

TbBool load_mods_order_config_file()
{
    SYNCDBG(8, "Starting");

    memset(&mods_conf, 0, sizeof(mods_conf));

    const char *sname = MODS_DIR_NAME "/" MODS_LOAD_ORDER_FILE_NAME;
    const char *fname = prepare_file_path(FGrp_Main, sname);

    long len = LbFileLengthRnc(fname);
    if (len < 2)
    {
        WARNMSG("Mods order file \"%s\" doesn't exist or is too small.", sname);
        return false;
    }
    if (len > 65536)
    {
        ERRORLOG("Mods order file \"%s\" is too large.", sname);
        return false;
    }
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
      return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    if (len>0)
    {
        parse_block_mods(buf, len, MODS_AFTER_BASE_BLOCK_NAME, mods_conf.after_base_item, &mods_conf.after_base_cnt, MOD_ITEM_MAX);
        parse_block_mods(buf, len, MODS_AFTER_CAMPAIGN_BLOCK_NAME, mods_conf.after_campaign_item, &mods_conf.after_campaign_cnt, MOD_ITEM_MAX);
        parse_block_mods(buf, len, MODS_AFTER_MAP_BLOCK_NAME, mods_conf.after_map_item, &mods_conf.after_map_cnt, MOD_ITEM_MAX);
    }
    free(buf);

    return true;
}



#ifdef __cplusplus
}
#endif
