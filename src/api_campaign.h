#ifndef KFX_API_CAMPAIGN_H
#define KFX_API_CAMPAIGN_H

#include "bflib_basics.h"
#include "json-dom.h"
#include "globals.h"
#include "game_merge.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Execute a campaign command using KeeperFX command syntax, for example:
 *   SET_CAMPAIGN_LEVEL_AVAILABLE(8)
 *   SET_CAMPAIGN_AUTO_ADVANCE(0)
 *
 * On failure, error_code receives a stable API error name.
 */
TbBool api_campaign_execute_command(const char *command, const char **error_code);

/**
 * Execute a campaign query using KeeperFX command syntax, for example:
 *   GET_CAMPAIGN_LEVELS()
 *   IS_CAMPAIGN_LEVEL_AVAILABLE(8)
 *
 * On success, data is initialized with the query result.
 * On failure, error_code receives a stable API error name.
 */
TbBool api_campaign_execute_query(const char *query, VALUE *data, const char **error_code);
TbBool campaign_level_api_set_available(LevelNumber lvnum);
TbBool campaign_level_api_set_unavailable(LevelNumber lvnum);
TbBool campaign_level_api_is_enabled(LevelNumber lvnum);
TbBool campaign_level_api_is_available(LevelNumber lvnum);
void campaign_level_api_reset(void);
void campaign_level_api_refresh(void);
TbBool get_campaign_auto_advance_enabled(void);
void campaign_level_api_set_auto_advance(TbBool enabled);
TbBool campaign_bonus_level_api_is_in_campaign(LevelNumber bn_lvnum);
TbBool campaign_bonus_level_api_set_available(LevelNumber bn_lvnum);
TbBool campaign_bonus_level_api_set_unavailable(LevelNumber bn_lvnum);
TbBool campaign_bonus_level_api_is_available(LevelNumber bn_lvnum);
#ifdef __cplusplus
}
#endif

#endif
