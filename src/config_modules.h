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
#define MODULE_CFG_FILE_NAME "mods.cfg"

#define MOD_LOAD_PERIOD_FIRST 0
#define MOD_LOAD_PERIOD_LAST 9

#define MODULE_ITEM_MAX  100

struct ModuleConfigItem {
    char name[COMMAND_WORD_LEN];
    int load_period;
    int priority;
    int disable;

    int exist;
};

struct ModulesConfig {
    long mod_item_cnt;
    struct ModuleConfigItem mod_item[MODULE_ITEM_MAX];
};


TbBool load_module_config_file();



#ifdef __cplusplus
}
#endif



#endif // DK_CFGMODULES_H
