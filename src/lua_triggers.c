#include "pre_inc.h"

#include "../lib/lua/include/lua.h"
#include "../lib/lua/include/lauxlib.h"
#include "../lib/lua/include/lualib.h"


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

		CheckLua(Lvl_script, lua_pcall(Lvl_script, 2, 0, 0));
	}
}


void lua_game_start()
{
    lua_getglobal(Lvl_script, "GameStart");
	if (lua_isfunction(Lvl_script, -1))
	{
		CheckLua(Lvl_script, lua_pcall(Lvl_script, 0, 0, 1));
	}
}

void lua_game_loop()
{
    lua_getglobal(Lvl_script, "GameLoop");
	if (lua_isfunction(Lvl_script, -1))
	{
		CheckLua(Lvl_script, lua_pcall(Lvl_script, 0, 0, 0));
	}
}

void lua_cast_power_on_thing(PowerKind pwkind,PlayerNumber plyr_idx, struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short splevel)
{
    lua_getglobal(Lvl_script, "CastPowerOnThing");
	if (lua_isfunction(Lvl_script, -1))
	{
		lua_pushinteger(Lvl_script, pwkind);
		lua_pushinteger(Lvl_script, plyr_idx);
		lua_pushThing(Lvl_script, thing); 
		lua_pushinteger(Lvl_script, stl_x);
		lua_pushinteger(Lvl_script, stl_y);
		lua_pushinteger(Lvl_script, splevel);

		CheckLua(Lvl_script, lua_pcall(Lvl_script, 6, 0, 1));
	}
}