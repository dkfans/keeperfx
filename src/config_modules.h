/******************************************************************************/
// Free implementation of KeeperFx.
/******************************************************************************/
/* @par  Purpose:
 *     The original intention of the design is that users can customize any data, and can overwrite the default settings of KeeperFX.
 *     Typical, when upgrading to a new version, simply copying a mods directory can complete the annoying reconfiguration.
 *     So the version release should not integrate any modules.
 *
 * @par  Log:
 *     hzzdev - 07 Sep 2025
 *     First version mainly implements functions related to basic configuration files and creature configuration files.
 *     If more types of data can be loaded later(eg. effects), the functionality will become very powerful.
 *     More information can be referred to https://github.com/dkfans/keeperfx/issues/3027
 *
 */
/******************************************************************************/



#ifndef DK_CFGMODULES_H
#define DK_CFGMODULES_H



#include "globals.h"
#include "bflib_basics.h"

#include "config.h"



#ifdef __cplusplus
extern "C" {
#endif



#define MODULE_DIR_NAME "mods"
#define MODULE_LOAD_ORDER_FILE_NAME "load_order.cfg"

#define MODULE_AFTER_BASE_BLOCK_NAME "after_base"
#define MODULE_AFTER_CAMPAIGN_BLOCK_NAME "after_campaign"
#define MODULE_AFTER_MAP_BLOCK_NAME "after_map"

#define MODULE_ITEM_MAX  50


struct ModuleConfigItem {
    char name[COMMAND_WORD_LEN];

    int exist_mod;

    // load_config
    int exist_fx_data;
    int exist_cmpg_config;
    int exist_cmpg_lvls;

    // load_creaturemodel_config
    int exist_crtr_data;
    int exist_cmpg_crtrs;
    // int exist_cmpg_lvls; // dup

};

struct ModulesConfig {
    long after_base_cnt;
    struct ModuleConfigItem after_base_item[MODULE_ITEM_MAX];

    long after_campaign_cnt;
    struct ModuleConfigItem after_campaign_item[MODULE_ITEM_MAX];

    long after_map_cnt;
    struct ModuleConfigItem after_map_item[MODULE_ITEM_MAX];
};


TbBool load_module_order_config_file();



#ifdef __cplusplus
}
#endif



#endif // DK_CFGMODULES_H
