/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_lua.c
 *     The level script's DAIMON_* commands as Lua functions (daimon_lua.h): the fork's functions, as Daimon's port tools copy them (scripts/daimon_port/sync.py in the fork).
 */
/******************************************************************************/
#include "pre_inc.h"
#include "daimon_lua.h"
#include <lua.h>
#include <lauxlib.h>
#include <ctype.h>
#include <string.h>
#include "globals.h"
#include "bflib_basics.h"
#include "lua_base.h"
#include "lua_params.h"
#include "player_data.h"
#include "player_instances.h"
#include "config_terrain.h"
#include "config_daimon.h"
#include "daimon/Daimon.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

static int lua_Daimon_set(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    const char *key = luaL_checkstring(L, 2);
    char text[64];
    if (lua_isboolean(L, 3))
        snprintf(text, sizeof(text), "%s", lua_toboolean(L, 3) ? "TRUE" : "FALSE");
    else
        snprintf(text, sizeof(text), "%s", luaL_checkstring(L, 3));
    char why[160];
    if (!DaimonCheckKey(key, text, why, sizeof(why)))
    {
        SCRPTERRLOG("DaimonSet: %s", why);
        lua_pushboolean(L, 0);
        return 1;
    }
    if ((player_idx == PLAYER_NEUTRAL) || (player_idx == PLAYER_GOOD) || !DaimonScriptSet(player_idx, key, text))
    {
        SCRPTWRNLOG("DaimonSet: player %d is not Daimon's; %s left as it is", (int)player_idx, key);
        lua_pushboolean(L, 0);
        return 1;
    }
    lua_pushboolean(L, 1);
    return 1;
}

static int lua_Daimon_build(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    const int64_t room = luaL_checkNamedCommand(L, 2, room_desc);
    const int64_t x = luaL_checkIntMinMax(L, 3, 0, 0xffff), y = luaL_checkIntMinMax(L, 4, 0, 0xffff);
    const int64_t w = luaL_checkIntMinMax(L, 5, 1, 255), h = luaL_checkIntMinMax(L, 6, 1, 255);
    lua_pushboolean(L, DaimonScriptBuild(player_idx, (RoomKind)room, x, y, w, h) ? 1 : 0);
    return 1;
}

static int lua_Daimon_attack(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    PlayerNumber rival = -1;
    if (!(lua_isstring(L, 2) && (strcasecmp(lua_tostring(L, 2), "NONE") == 0)))
        rival = luaL_checkPlayerSingle(L, 2);
    const char *objective = luaL_optstring(L, 3, "");
    lua_pushboolean(L, DaimonScriptAttack(player_idx, rival, objective) ? 1 : 0);
    return 1;
}

static int lua_Daimon_dig(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    const int64_t x = luaL_checkIntMinMax(L, 2, 0, 0xffff), y = luaL_checkIntMinMax(L, 3, 0, 0xffff);
    lua_pushboolean(L, DaimonScriptDig(player_idx, x, y) ? 1 : 0);
    return 1;
}

static int lua_Daimon_player(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    const char *personality = luaL_optstring(L, 2, "");
    const char *difficulty = luaL_optstring(L, 3, "");
    char lc[64];
    snprintf(lc, sizeof(lc), "%s", personality);
    for (char *c = lc; *c != '\0'; c++) *c = (char)tolower((unsigned char)*c);   // as DAIMON_PLAYER's (the facade too)
    if ((lc[0] != '\0') && (strcmp(lc, "random") != 0) && (daimon_config_personality(lc) == NULL))
        SCRPTWRNLOG("DaimonPlayer: no personality '%s' in daimon.toml; the default plays", personality);
    if ((player_idx == PLAYER_NEUTRAL) || (player_idx == PLAYER_GOOD) || !DaimonScriptPlayer(player_idx, personality, difficulty))
    {
        SCRPTWRNLOG("DaimonPlayer: player %d cannot be Daimon's (a human's, an External seat's, or no keeper)", (int)player_idx);
        lua_pushboolean(L, 0);
        return 1;
    }
    lua_pushboolean(L, 1);
    return 1;
}

static const luaL_Reg daimon_methods[] = {
    {"DaimonPlayer", lua_Daimon_player},
    {"DaimonSet",    lua_Daimon_set},
    {"DaimonBuild",  lua_Daimon_build},
    {"DaimonAttack", lua_Daimon_attack},
    {"DaimonDig",    lua_Daimon_dig},
};

void register_lua_daimon_api(lua_State *L)
{
    for (size_t i = 0; i < sizeof(daimon_methods) / sizeof(daimon_methods[0]); i++)
        lua_register(L, daimon_methods[i].name, daimon_methods[i].func);
}

#ifdef __cplusplus
}
#endif
