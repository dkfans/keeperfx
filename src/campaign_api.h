#ifndef KFX_CAMPAIGN_API_H
#define KFX_CAMPAIGN_API_H

#include "bflib_basics.h"
#include "json-dom.h"

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
TbBool campaign_api_execute_command(const char *command, const char **error_code);

/**
 * Execute a campaign query using KeeperFX command syntax, for example:
 *   GET_CAMPAIGN_LEVELS()
 *   IS_CAMPAIGN_LEVEL_AVAILABLE(8)
 *
 * On success, data is initialized with the query result.
 * On failure, error_code receives a stable API error name.
 */
TbBool campaign_api_execute_query(const char *query, VALUE *data, const char **error_code);

#ifdef __cplusplus
}
#endif

#endif
