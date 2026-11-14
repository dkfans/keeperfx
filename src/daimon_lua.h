/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_lua.h
 *     The level script's Daimon commands as Lua functions: DaimonPlayer, DaimonSet, DaimonBuild, DaimonAttack and
 *     DaimonDig.
 */
/******************************************************************************/
#ifndef DK_DAIMON_LUA_H
#define DK_DAIMON_LUA_H

#ifdef __cplusplus
extern "C" {
#endif

struct lua_State;

void register_lua_daimon_api(struct lua_State *L);

#ifdef __cplusplus
}
#endif
#endif
