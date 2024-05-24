#include "pre_inc.h"

#include "../deps/luajit/src/lua.h"
#include "../deps/luajit/src/lauxlib.h"
#include "../deps/luajit/src/lualib.h"


#include "lua_base.h"
#include "lua_api.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "config.h"
#include "globals.h"
#include "thing_data.h"


#include "post_inc.h"

static struct luaThing *lua_pushThing(lua_State *L, struct Thing* thing);

void lua_chatmsg(PlayerNumber plyr_idx, char *msg)
{
    lua_getglobal(Lvl_script, "ChatMsg");
	if (lua_isfunction(Lvl_script, -1))
	{
		lua_pushinteger(Lvl_script, plyr_idx);
		lua_pushstring(Lvl_script, msg);

		CheckLua(Lvl_script, lua_pcall(Lvl_script, 2, 0, 0),"ChatMsg");
	}
}


void lua_game_start()
{
    lua_getglobal(Lvl_script, "OnGameStart");
	if (lua_isfunction(Lvl_script, -1))
	{
		CheckLua(Lvl_script, lua_pcall(Lvl_script, 0, 0, 1),"OnGameStart");
	}
}

void lua_game_tick()
{
    lua_getglobal(Lvl_script, "OnGameTick");
	if (lua_isfunction(Lvl_script, -1))
	{
		CheckLua(Lvl_script, lua_pcall(Lvl_script, 0, 0, 1), "OnGameTick");
	}
}

void lua_on_power_cast(PlayerNumber plyr_idx, PowerKind pwkind,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing)
	{
    lua_getglobal(Lvl_script, "OnPowerCast");
	if (lua_isfunction(Lvl_script, -1))
	{
		lua_pushinteger(Lvl_script, pwkind);
		lua_pushinteger(Lvl_script, plyr_idx);
		lua_pushThing(Lvl_script, thing); 
		lua_pushinteger(Lvl_script, stl_x);
		lua_pushinteger(Lvl_script, stl_y);
		lua_pushinteger(Lvl_script, splevel);

		CheckLua(Lvl_script, lua_pcall(Lvl_script, 6, 0, 1),"OnPowerCast");
	}
}