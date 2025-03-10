/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lua_cfg_funcs.h
 *     Header file for lua_cfg_funcs.c.
 * @par Purpose:
 *     Console commands
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef LUAPARAMS_H
#define LUAPARAMS_H

#include "globals.h"
#include "map_locations.h"
#include "../deps/luajit/src/lua.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PlayerRange
{
    PlayerNumber start_idx;
    PlayerNumber end_idx;
};
/****   Checks  *******/
TbBool luaL_isThing(lua_State *L, int index);

/****   Inputs  *******/
long luaL_optNamedCommand(lua_State *L, int index,const struct NamedCommand * commanddesc);
long luaL_checkNamedCommand(lua_State *L, int index,const struct NamedCommand * commanddesc);
TbMapLocation luaL_checkLocation(lua_State *L, int index);
TbMapLocation luaL_optLocation(lua_State *L, int index);
TbMapLocation luaL_checkHeadingLocation(lua_State *L, int index);
struct PlayerRange luaL_checkPlayerRange(lua_State *L, int index);
PlayerNumber luaL_checkPlayerSingle(lua_State *L, int index);
PlayerNumber luaL_checkPlayerRangeId(lua_State *L, int index);
MapSubtlCoord luaL_checkstl_x(lua_State *L, int index);
MapSubtlCoord luaL_checkstl_y(lua_State *L, int index);
MapSlabCoord luaL_checkslb_x(lua_State *L, int index);
MapSlabCoord luaL_checkslb_y(lua_State *L, int index);
ActionPointId luaL_checkActionPoint(lua_State *L, int index);
unsigned char luaL_checkCrtLevel(lua_State *L, int index);
unsigned char luaL_checkParty(lua_State *L, int index);
void luaL_checkMessageIcon(lua_State *L, int index, char* type, char* id);
long luaL_checkIntMinMax(lua_State *L, int index,long min, long max);
EffectOrEffElModel luaL_checkEffectOrEffElModel(lua_State *L, int index);
long luaL_checkCreature_or_creature_wildcard(lua_State *L, int index);
void luaL_checkGameRule(lua_State *L, int index,short *rulegroup, short *ruledesc);

struct Thing *luaL_checkThing(lua_State *L, int index);
struct Thing *luaL_checkCreature(lua_State *L, int index);

/****   Outputs  *******/
void lua_pushThing(lua_State *L, struct Thing* thing);
void lua_pushPlayer(lua_State *L, PlayerNumber plr_idx) ;
void lua_pushPos(lua_State *L, struct Coord3d* pos);

#ifdef __cplusplus
}
#endif

#endif