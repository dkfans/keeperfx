/******************************************************************************/
// Free implementation of KeeperFx.
/******************************************************************************/
/* @par  Purpose:
 *     The original intention of the design is that users can customize any data, and can overwrite the default settings of KeeperFX.
 *     Typical, when upgrading to a new version, simply copying a mods directory can complete the annoying reconfiguration.
 *
 * @par  Log:
 *     hzzdev - 07 Sep 2025
 *     First version mainly implements functions related to basic configuration files and creature configuration files.
 *     If more types of data can be loaded later(eg. effects), the functionality will become very powerful.
 *     More information can be referred to https://github.com/dkfans/keeperfx/issues/3027
 *
 *     hzzdev - 18 Sep 2025, Add sprite loading for mods.
 *     hzzdev - 30 Oct 2025, Add multi-lang string loading for mods.
 *     hzzdev - 23 Feb 2026, Add texture loading for mods.
 *
 */
/******************************************************************************/



#ifndef DK_CFG_MODS_H
#define DK_CFG_MODS_H



#include "globals.h"
#include "bflib_basics.h"

#include "config.h"



#ifdef __cplusplus
extern "C" {
#endif



#define MODS_DIR_NAME "mods"
#define MODS_LOAD_ORDER_FILE_NAME "load_order.cfg"

#define MODS_AFTER_BASE_BLOCK_NAME "after_base"
#define MODS_AFTER_CAMPAIGN_BLOCK_NAME "after_campaign"
#define MODS_AFTER_MAP_BLOCK_NAME "after_map"

// base, campaign, map
#define MOD_ITEM_TYPE_CNT  3
#define MOD_ITEM_MAX  50



struct ModExistState{
    int mod_dir;

    int fx_data;	// FGrp_FxData: string, config, sprite
    int std_data;	// FGrp_StdData: texture
    int cmpg_config;	// FGrp_CmpgConfig: config, sprite, texture
    int cmpg_lvls;	// FGrp_CmpgLvls: creaturemodel, config, sprite, texture

    int crtr_data;	// FGrp_CrtrData: creaturemodel
    int cmpg_crtrs;	// FGrp_CmpgCrtrs: creaturemodel
};

struct ModConfigItem {
    char name[COMMAND_WORD_LEN];

    struct ModExistState state;
};

struct ModsConfig {
    int32_t after_base_cnt;
    struct ModConfigItem after_base_item[MOD_ITEM_MAX];

    int32_t after_campaign_cnt;
    struct ModConfigItem after_campaign_item[MOD_ITEM_MAX];

    int32_t after_map_cnt;
    struct ModConfigItem after_map_item[MOD_ITEM_MAX];
};

extern struct ModsConfig mods_conf;
void recheck_all_mod_exist();
TbBool load_mods_order_config_file();


#ifdef __cplusplus
}
#endif



#endif // DK_CFG_MODS_H
