#include "pre_inc.h"

#include "../lib/lua/include/lua.h"
#include "../lib/lua/include/lauxlib.h"
#include "../lib/lua/include/lualib.h"

#include "lua_api.h"

#include "bflib_basics.h"
#include "config.h"
#include "globals.h"

#include "post_inc.h"

lua_State *Lvl_script;

// Little error checking utility function
static TbBool CheckLua(lua_State *L, int r)
{
	if (r != LUA_OK)
	{
		ERRORLOG("%s",(char*)lua_tostring(L, -1));
		return false;
	}
	return true;
}

void close_lua_script()
{
    if(Lvl_script)
        lua_close(Lvl_script);
}

void open_lua_script(LevelNumber lvnum)
{
	Lvl_script = luaL_newstate();

	luaL_openlibs(Lvl_script);

	reg_host_functions(Lvl_script);
	 
    short fgroup = get_level_fgroup(lvnum);
    char* fname = prepare_file_fmtpath(fgroup, "map%05lu.lua", (unsigned long)lvnum);
	// Load and parse the Lua File
	if(!CheckLua(Lvl_script, luaL_dofile(Lvl_script, fname)))
	{
        close_lua_script();
	}
}



void lua_chatmsg(PlayerNumber plyr_idx, char *msg)
{
    lua_getglobal(Lvl_script, "ChatMsg");
	if (lua_isfunction(Lvl_script, -1))
	{
        //arguments the function needs
		lua_pushinteger(Lvl_script, plyr_idx);
		lua_pushstring(Lvl_script, msg);

        //2 is how many arguments C passes to lua, 1 is how many lua passes back to C
		CheckLua(Lvl_script, lua_pcall(Lvl_script, 2, 1, 0));
	}
}


void lua_game_start()
{
    lua_getglobal(Lvl_script, "GameStart");
	if (lua_isfunction(Lvl_script, -1))
	{
		CheckLua(Lvl_script, lua_pcall(Lvl_script, 0, 0, 0));
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