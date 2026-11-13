/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_script_commands.c
 *     The level script's DAIMON_PLAYER, DAIMON_SET, DAIMON_BUILD, DAIMON_ATTACK and DAIMON_DIG (daimon_script_commands.h): the fork's functions, as Daimon's port tools copy them (scripts/daimon_port/sync.py in the fork).
 */
/******************************************************************************/
#include "pre_inc.h"
#include "daimon_script_commands.h"
#include <ctype.h>
#include <string.h>
#include "globals.h"
#include "bflib_basics.h"
#include "lvl_script_conditions.h"
#include "lvl_script_lib.h"
#include "player_data.h"
#include "player_instances.h"
#include "config_daimon.h"
#include "daimon/Daimon.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

void daimon_player_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    int plr_start;
    int plr_end;
    if (get_players_range(scline->np[0], &plr_start, &plr_end) < 0)
    {
        SCRPTERRLOG("Given owning player range %" PRId64 " is not supported in this command", (int64_t)scline->np[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    // The script reader upper-cases names; daimon.toml's personalities are lower case.
    char personality[MAX_TEXT_LENGTH];
    snprintf(personality, sizeof(personality), "%s", scline->tp[1]);
    for (char *c = personality; *c != '\0'; c++)
        *c = (char)tolower((unsigned char)*c);
    if ((personality[0] != '\0') && (strcmp(personality, "random") != 0) && (daimon_config_personality(personality) == NULL))
    {
        SCRPTWRNLOG("DAIMON_PLAYER: no personality '%s' in daimon.toml; the default plays", personality);
    }
    value->shorts[0] = plr_start;
    value->shorts[1] = plr_end;
    value->longs[1] = script_strdup(personality);
    value->longs[2] = script_strdup(scline->tp[2]);
    if ((value->longs[1] < 0) || (value->longs[2] < 0))
    {
        SCRPTERRLOG("Run out script strings space");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

void daimon_player_process(struct ScriptContext* context)
{
    const char *personality = script_strval(context->value->longs[1]);
    const char *difficulty = script_strval(context->value->longs[2]);
    for (int64_t i = context->value->shorts[0]; i < context->value->shorts[1]; i++)
    {
        if ((i == PLAYER_NEUTRAL) || (i == PLAYER_GOOD))
            continue;
        if (!DaimonScriptPlayer((PlayerNumber)i, personality, difficulty))
        {
            SCRPTWRNLOG("DAIMON_PLAYER: player %" PRId64 " cannot be Daimon's (a human's, an External seat's, or no keeper)", i);
        }
    }
}

void daimon_set_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    int plr_start;
    int plr_end;
    if (get_players_range(scline->np[0], &plr_start, &plr_end) < 0)
    {
        SCRPTERRLOG("Given owning player range %" PRId64 " is not supported in this command", (int64_t)scline->np[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    char why[160];
    if (!DaimonCheckKey(scline->tp[1], scline->tp[2], why, sizeof(why)))
    {
        SCRPTERRLOG("DAIMON_SET: %s", why);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->shorts[0] = plr_start;
    value->shorts[1] = plr_end;
    value->longs[1] = script_strdup(scline->tp[1]);
    value->longs[2] = script_strdup(scline->tp[2]);
    if ((value->longs[1] < 0) || (value->longs[2] < 0))
    {
        SCRPTERRLOG("Run out script strings space");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

void daimon_set_process(struct ScriptContext* context)
{
    const char *key = script_strval(context->value->longs[1]);
    const char *val = script_strval(context->value->longs[2]);
    for (int64_t i = context->value->shorts[0]; i < context->value->shorts[1]; i++)
    {
        if ((i == PLAYER_NEUTRAL) || (i == PLAYER_GOOD))
            continue;
        if (!DaimonScriptSet((PlayerNumber)i, key, val))
        {
            SCRPTWRNLOG("DAIMON_SET: player %" PRId64 " is not Daimon's; %s left as it is", i, key);
        }
    }
}

void daimon_build_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    int plr_start;
    int plr_end;
    if (get_players_range(scline->np[0], &plr_start, &plr_end) < 0)
    {
        SCRPTERRLOG("Given owning player range %" PRId64 " is not supported in this command", (int64_t)scline->np[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    const int64_t x = scline->np[2], y = scline->np[3], w = scline->np[4], h = scline->np[5];
    if ((x < 0) || (y < 0) || (w < 1) || (h < 1) || (w > 255) || (h > 255) || (x > 0xffff) || (y > 0xffff))
    {
        SCRPTERRLOG("DAIMON_BUILD: rectangle (%" PRId64 ",%" PRId64 ") %" PRId64 "x%" PRId64 " is not one", x, y, w, h);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->bytes[0] = (unsigned char)plr_start;
    value->bytes[1] = (unsigned char)plr_end;
    value->bytes[2] = (unsigned char)w;
    value->bytes[3] = (unsigned char)h;
    value->longs[1] = (int32_t)scline->np[1];   // the room kind
    value->longs[2] = (int32_t)x;
    value->longs[3] = (int32_t)y;
    PROCESS_SCRIPT_VALUE(scline->command);
}

void daimon_build_process(struct ScriptContext* context)
{
    const struct ScriptValue *v = context->value;
    for (int64_t i = v->bytes[0]; i < v->bytes[1]; i++)
    {
        if ((i == PLAYER_NEUTRAL) || (i == PLAYER_GOOD))
            continue;
        if (!DaimonScriptBuild((PlayerNumber)i, (RoomKind)v->longs[1], v->longs[2], v->longs[3], v->bytes[2], v->bytes[3]))
            SCRPTWRNLOG("DAIMON_BUILD: player %" PRId64 " is not Daimon's, or the room is not one it builds", i);
    }
}

void daimon_attack_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    int plr_start;
    int plr_end;
    if (get_players_range(scline->np[0], &plr_start, &plr_end) < 0)
    {
        SCRPTERRLOG("Given owning player range %" PRId64 " is not supported in this command", (int64_t)scline->np[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    int64_t rival = -1;
    if (strcasecmp(scline->tp[1], "NONE") != 0)
    {
        int32_t rival_range = 0;
        int r_start, r_end;
        if (!get_player_id(scline->tp[1], &rival_range) || (get_players_range(rival_range, &r_start, &r_end) < 0) || (r_end != r_start + 1))
        {
            SCRPTERRLOG("DAIMON_ATTACK: '%s' is not one player (PLAYERn) nor NONE", scline->tp[1]);
            DEALLOCATE_SCRIPT_VALUE
            return;
        }
        rival = r_start;
    }
    const char *obj = scline->tp[2];
    if ((obj[0] != '\0') && (strcasecmp(obj, "HEART") != 0) && (strcasecmp(obj, "PORTAL") != 0) && (strcasecmp(obj, "ROOMS") != 0)
        && (strcasecmp(obj, "ANY") != 0))
    {
        SCRPTERRLOG("DAIMON_ATTACK: '%s' is not HEART, PORTAL, ROOMS or ANY", obj);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->shorts[0] = plr_start;
    value->shorts[1] = plr_end;
    value->longs[1] = (int32_t)rival;
    value->longs[2] = script_strdup(obj);
    if (value->longs[2] < 0)
    {
        SCRPTERRLOG("Run out script strings space");
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    PROCESS_SCRIPT_VALUE(scline->command);
}

void daimon_attack_process(struct ScriptContext* context)
{
    const char *obj = script_strval(context->value->longs[2]);
    for (int64_t i = context->value->shorts[0]; i < context->value->shorts[1]; i++)
    {
        if ((i == PLAYER_NEUTRAL) || (i == PLAYER_GOOD))
            continue;
        if (!DaimonScriptAttack((PlayerNumber)i, (PlayerNumber)context->value->longs[1], obj))
            SCRPTWRNLOG("DAIMON_ATTACK: player %" PRId64 " is not Daimon's, or cannot attack that player", i);
    }
}

void daimon_dig_check(const struct ScriptLine* scline)
{
    ALLOCATE_SCRIPT_VALUE(scline->command, 0);
    int plr_start;
    int plr_end;
    if (get_players_range(scline->np[0], &plr_start, &plr_end) < 0)
    {
        SCRPTERRLOG("Given owning player range %" PRId64 " is not supported in this command", (int64_t)scline->np[0]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    if ((scline->np[1] < 0) || (scline->np[2] < 0) || (scline->np[1] > 0xffff) || (scline->np[2] > 0xffff))
    {
        SCRPTERRLOG("DAIMON_DIG: slab (%" PRId64 ",%" PRId64 ") is not on a map", (int64_t)scline->np[1], (int64_t)scline->np[2]);
        DEALLOCATE_SCRIPT_VALUE
        return;
    }
    value->shorts[0] = plr_start;
    value->shorts[1] = plr_end;
    value->longs[1] = (int32_t)scline->np[1];
    value->longs[2] = (int32_t)scline->np[2];
    PROCESS_SCRIPT_VALUE(scline->command);
}

void daimon_dig_process(struct ScriptContext* context)
{
    for (int64_t i = context->value->shorts[0]; i < context->value->shorts[1]; i++)
    {
        if ((i == PLAYER_NEUTRAL) || (i == PLAYER_GOOD))
            continue;
        if (!DaimonScriptDig((PlayerNumber)i, context->value->longs[1], context->value->longs[2]))
            SCRPTWRNLOG("DAIMON_DIG: player %" PRId64 " is not Daimon's", i);
    }
}

#ifdef __cplusplus
}
#endif
