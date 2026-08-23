#include "pre_inc.h"

#include "api_campaign.h"
#include "front_landview.h"
#include "config_campaigns.h"
#include "post_inc.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define API_CAMPAIGN_LEVEL_BITMAP_SIZE ((FREE_LEVELS_COUNT + 7) / 8)

/* API-controlled campaign availability state. */
static unsigned char api_campaign_enabled_levels[API_CAMPAIGN_LEVEL_BITMAP_SIZE];
static TbBool campaign_auto_advance = true;

static const char *skip_space(const char *p)
{
    while (p != NULL && isspace((unsigned char)*p))
        p++;
    return p;
}

static TbBool consume_command_name(const char **input, const char *name)
{
    const char *p = skip_space(*input);
    size_t len = strlen(name);

    if (p == NULL || strncasecmp(p, name, len) != 0)
        return false;

    p += len;
    p = skip_space(p);
    if (*p != '(')
        return false;

    *input = p + 1;
    return true;
}

static TbBool consume_end(const char *p)
{
    p = skip_space(p);
    return p != NULL && *p == '\0';
}

static TbBool consume_no_arguments(const char **input)
{
    const char *p = skip_space(*input);
    if (*p != ')')
        return false;
    p++;
    if (!consume_end(p))
        return false;
    *input = p;
    return true;
}

static TbBool consume_integer(const char **input, long *value)
{
    char *end;
    const char *p = skip_space(*input);

    if (p == NULL || *p == '\0')
        return false;

    errno = 0;
    long result = strtol(p, &end, 10);
    if (end == p || errno == ERANGE)
        return false;

    end = (char *)skip_space(end);
    if (*end != ')')
        return false;

    *value = result;
    *input = end + 1;
    return true;
}

static TbBool consume_boolean(const char **input, TbBool *value)
{
    const char *p = skip_space(*input);
    const char *end;

    if (strncasecmp(p, "true", 4) == 0)
    {
        end = skip_space(p + 4);
        if (*end == ')')
        {
            *value = true;
            *input = end + 1;
            return true;
        }
    }

    if (strncasecmp(p, "false", 5) == 0)
    {
        end = skip_space(p + 5);
        if (*end == ')')
        {
            *value = false;
            *input = end + 1;
            return true;
        }
    }

    long numeric;
    if (consume_integer(input, &numeric))
    {
        if (numeric == 0 || numeric == 1)
        {
            *value = (numeric != 0);
            return true;
        }
    }

    return false;
}

static TbBool validate_level_number(long value, LevelNumber *lvnum)
{
    if (value < 0 || value >= FREE_LEVELS_COUNT || value > INT_MAX)
        return false;

    *lvnum = (LevelNumber)value;
    return true;
}

TbBool api_campaign_execute_command(const char *command, const char **error_code)
{
    const char *p = command;
    long value;
    LevelNumber lvnum;
    TbBool enabled;

    if (error_code != NULL)
        *error_code = "FAILED_TO_EXECUTE_CAMPAIGN_COMMAND";

    if (command == NULL || strlen(command) < 1)
    {
        if (error_code != NULL)
            *error_code = "MISSING_COMMAND";
        return false;
    }

    if (consume_command_name(&p, "SET_CAMPAIGN_LEVEL_AVAILABLE"))
    {
        if (!consume_integer(&p, &value) || !consume_end(p) || !validate_level_number(value, &lvnum))
        {
            if (error_code != NULL)
                *error_code = "INVALID_CAMPAIGN_LEVEL";
            return false;
        }
        if (!campaign_level_api_set_available(lvnum))
        {
            if (error_code != NULL)
                *error_code = "INVALID_CAMPAIGN_LEVEL";
            return false;
        }
        campaign_level_api_refresh();
        return true;
    }

    p = command;
    if (consume_command_name(&p, "SET_CAMPAIGN_LEVEL_UNAVAILABLE"))
    {
        if (!consume_integer(&p, &value) || !consume_end(p) || !validate_level_number(value, &lvnum))
        {
            if (error_code != NULL)
                *error_code = "INVALID_CAMPAIGN_LEVEL";
            return false;
        }
        if (!campaign_level_api_set_unavailable(lvnum))
        {
            if (error_code != NULL)
                *error_code = "INVALID_CAMPAIGN_LEVEL";
            return false;
        }
        campaign_level_api_refresh();
        return true;
    }

    p = command;
    if (consume_command_name(&p, "SET_CAMPAIGN_BONUS_LEVEL_AVAILABLE"))
    {
        if (!consume_integer(&p, &value) || !consume_end(p) || !validate_level_number(value, &lvnum))
        {
            if (error_code != NULL)
                *error_code = "INVALID_CAMPAIGN_BONUS_LEVEL";
            return false;
        }
        if (!campaign_bonus_level_api_set_available(lvnum))
        {
            if (error_code != NULL)
                *error_code = "INVALID_CAMPAIGN_BONUS_LEVEL";
            return false;
        }
        campaign_level_api_refresh();
        return true;
    }

    p = command;
    if (consume_command_name(&p, "SET_CAMPAIGN_BONUS_LEVEL_UNAVAILABLE"))
    {
        if (!consume_integer(&p, &value) || !consume_end(p) || !validate_level_number(value, &lvnum))
        {
            if (error_code != NULL)
                *error_code = "INVALID_CAMPAIGN_BONUS_LEVEL";
            return false;
        }
        if (!campaign_bonus_level_api_set_unavailable(lvnum))
        {
            if (error_code != NULL)
                *error_code = "INVALID_CAMPAIGN_BONUS_LEVEL";
            return false;
        }
        campaign_level_api_refresh();
        return true;
    }

    p = command;
    if (consume_command_name(&p, "RESET_CAMPAIGN_LEVEL_OVERRIDES"))
    {
        if (!consume_no_arguments(&p))
            return false;
        campaign_level_api_reset();
        campaign_level_api_refresh();
        return true;
    }

    p = command;
    if (consume_command_name(&p, "REFRESH_LAND_SCREEN"))
    {
        if (!consume_no_arguments(&p))
            return false;
        campaign_level_api_refresh();
        return true;
    }

    p = command;
    if (consume_command_name(&p, "SET_CAMPAIGN_AUTO_ADVANCE"))
    {
        if (!consume_boolean(&p, &enabled) || !consume_end(p))
            return false;
        campaign_level_api_set_auto_advance(enabled);
        return true;
    }

    return false;
}

static TbBool build_campaign_levels(VALUE *data)
{
    value_init_dict(data);
    VALUE *levels = value_dict_add(data, "levels");
    value_init_array(levels);

    LevelNumber lvnum = first_singleplayer_level();
    while (lvnum > 0)
    {
        value_init_int32(value_array_append(levels), lvnum);
        lvnum = next_singleplayer_level(lvnum, true);
    }
    return true;
}

static TbBool build_available_campaign_levels(VALUE *data)
{
    value_init_dict(data);
    VALUE *levels = value_dict_add(data, "levels");
    value_init_array(levels);

    LevelNumber lvnum = first_singleplayer_level();
    while (lvnum > 0)
    {
        if (campaign_level_api_is_available(lvnum))
            value_init_int32(value_array_append(levels), lvnum);
        lvnum = next_singleplayer_level(lvnum, true);
    }
    return true;
}

static TbBool build_campaign_bonus_levels(VALUE *data)
{
    value_init_dict(data);
    VALUE *levels = value_dict_add(data, "levels");
    value_init_array(levels);

    LevelNumber sp_lvnum = first_singleplayer_level();
    while (sp_lvnum > 0)
    {
        LevelNumber bn_lvnum = bonus_level_for_singleplayer_level(sp_lvnum);
        if (bn_lvnum > 0)
        {
            VALUE *entry = value_array_append(levels);
            value_init_dict(entry);
            value_init_int32(value_dict_add(entry, "campaign_level"), sp_lvnum);
            value_init_int32(value_dict_add(entry, "bonus_level"), bn_lvnum);
            value_init_bool(value_dict_add(entry, "available"), campaign_bonus_level_api_is_available(bn_lvnum));
        }
        sp_lvnum = next_singleplayer_level(sp_lvnum, true);
    }
    return true;
}

TbBool api_campaign_execute_query(const char *query, VALUE *data, const char **error_code)
{
    const char *p = query;
    long value;
    LevelNumber lvnum;

    if (error_code != NULL)
        *error_code = "FAILED_TO_EXECUTE_CAMPAIGN_QUERY";

    if (query == NULL || strlen(query) < 1)
    {
        if (error_code != NULL)
            *error_code = "MISSING_QUERY";
        return false;
    }

    if (consume_command_name(&p, "GET_CAMPAIGN_LEVELS"))
    {
        if (!consume_no_arguments(&p))
            return false;
        return build_campaign_levels(data);
    }

    p = query;
    if (consume_command_name(&p, "GET_AVAILABLE_CAMPAIGN_LEVELS"))
    {
        if (!consume_no_arguments(&p))
            return false;
        return build_available_campaign_levels(data);
    }

    p = query;
    if (consume_command_name(&p, "GET_CAMPAIGN_BONUS_LEVELS"))
    {
        if (!consume_no_arguments(&p))
            return false;
        return build_campaign_bonus_levels(data);
    }

    p = query;
    if (consume_command_name(&p, "IS_CAMPAIGN_LEVEL_AVAILABLE"))
    {
        if (!consume_integer(&p, &value) || !consume_end(p) || !validate_level_number(value, &lvnum))
        {
            if (error_code != NULL)
                *error_code = "INVALID_CAMPAIGN_LEVEL";
            return false;
        }

        value_init_dict(data);
        value_init_int32(value_dict_add(data, "level"), lvnum);
        value_init_bool(value_dict_add(data, "available"), campaign_level_api_is_available(lvnum));
        return true;
    }

    p = query;
    if (consume_command_name(&p, "IS_CAMPAIGN_BONUS_LEVEL_AVAILABLE"))
    {
        if (!consume_integer(&p, &value) || !consume_end(p) || !validate_level_number(value, &lvnum))
        {
            if (error_code != NULL)
                *error_code = "INVALID_CAMPAIGN_BONUS_LEVEL";
            return false;
        }

        value_init_dict(data);
        value_init_int32(value_dict_add(data, "level"), lvnum);
        value_init_bool(value_dict_add(data, "available"), campaign_bonus_level_api_is_available(lvnum));
        return true;
    }

    p = query;
    if (consume_command_name(&p, "IS_CAMPAIGN_AUTO_ADVANCE_ENABLED"))
    {
        if (!consume_no_arguments(&p))
            return false;

        value_init_dict(data);
        value_init_bool(value_dict_add(data, "enabled"), get_campaign_auto_advance_enabled());
        return true;
    }

    return false;
}

static TbBool campaign_level_api_is_in_current_campaign(LevelNumber lvnum)
{
    LevelNumber current = first_singleplayer_level();

    while (current > 0)
    {
        if (current == lvnum)
            return true;

        current = next_singleplayer_level(current, true);
    }

    return false;
}

TbBool campaign_level_api_set_available(LevelNumber lvnum)
{
    int byte_idx;
    int bit_idx;

    if (lvnum < 0 || lvnum >= FREE_LEVELS_COUNT)
        return false;
    if (!campaign_level_api_is_in_current_campaign(lvnum))
        return false;

    byte_idx = lvnum / 8;
    bit_idx = lvnum % 8;
    api_campaign_enabled_levels[byte_idx] |= (1 << bit_idx);
    return true;
}

TbBool campaign_level_api_set_unavailable(LevelNumber lvnum)
{
    int byte_idx;
    int bit_idx;

    if (lvnum < 0 || lvnum >= FREE_LEVELS_COUNT)
        return false;
    if (!campaign_level_api_is_in_current_campaign(lvnum))
        return false;

    byte_idx = lvnum / 8;
    bit_idx = lvnum % 8;
    api_campaign_enabled_levels[byte_idx] &= ~(1 << bit_idx);
    return true;
}

TbBool campaign_level_api_is_enabled(LevelNumber lvnum)
{
    int byte_idx;
    int bit_idx;

    if (lvnum < 0 || lvnum >= FREE_LEVELS_COUNT)
        return false;

    byte_idx = lvnum / 8;
    bit_idx = lvnum % 8;
    return (api_campaign_enabled_levels[byte_idx] & (1 << bit_idx)) != 0;
}

void campaign_level_api_reset(void)
{
    memset(api_campaign_enabled_levels, 0, sizeof(api_campaign_enabled_levels));
}

TbBool campaign_level_api_is_available(LevelNumber lvnum)
{
    LevelNumber continue_lvnum;

    if (get_level_info(lvnum) == NULL)
        return false;

    continue_lvnum = get_continue_level_number();

    if (continue_lvnum > 0 && lvnum == continue_lvnum)
        return true;

    if (continue_lvnum == SINGLEPLAYER_FINISHED)
        return true;

    return campaign_level_api_is_enabled(lvnum);
}

void campaign_level_api_refresh(void)
{
    update_ensigns_visibility();
}

TbBool campaign_bonus_level_api_is_in_campaign(LevelNumber bn_lvnum)
{
    LevelNumber sp_lvnum = first_singleplayer_level();

    while (sp_lvnum > 0)
    {
        if (bonus_level_for_singleplayer_level(sp_lvnum) == bn_lvnum)
            return true;

        sp_lvnum = next_singleplayer_level(sp_lvnum, true);
    }

    return false;
}

TbBool campaign_bonus_level_api_set_available(LevelNumber bn_lvnum)
{
    if (bn_lvnum < 1 || bn_lvnum >= FREE_LEVELS_COUNT)
        return false;

    if (!campaign_bonus_level_api_is_in_campaign(bn_lvnum))
        return false;

    return set_bonus_level_visibility(bn_lvnum, true);
}

TbBool campaign_bonus_level_api_set_unavailable(LevelNumber bn_lvnum)
{
    if (bn_lvnum < 1 || bn_lvnum >= FREE_LEVELS_COUNT)
        return false;

    if (!campaign_bonus_level_api_is_in_campaign(bn_lvnum))
        return false;

    return set_bonus_level_visibility(bn_lvnum, false);
}

TbBool campaign_bonus_level_api_is_available(LevelNumber bn_lvnum)
{
    if (!campaign_bonus_level_api_is_in_campaign(bn_lvnum))
        return false;

    return is_bonus_level_visible(get_my_player(), bn_lvnum);
}

TbBool get_campaign_auto_advance_enabled(void)
{
    return campaign_auto_advance;
}

void campaign_level_api_set_auto_advance(TbBool enabled)
{
    campaign_auto_advance = enabled;
}