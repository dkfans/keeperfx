#include "pre_inc.h"

#include "../deps/luau/VM/include/lua.h"
#include "../deps/luau/VM/include/lualib.h"
#include "../deps/luau/Compiler/include/luacode.h"

#include "lua_api.h"

#include "bflib_basics.h"
#include "bflib_dernc.h"
#include "config.h"
#include "globals.h"


#include "post_inc.h"

lua_State *L;

// Little error checking utility function
static TbBool CheckLua(lua_State *Ls, int r)
{
	if (r != LUA_OK)
	{
		ERRORLOG("%s",(char*)lua_tostring(Ls, -1));
		return false;
	}
	return true;
}

void close_lua_script()
{
    if(L)
        lua_close(L);
}

void open_lua_script(LevelNumber lvnum)
{
	L = luaL_newstate();

	luaL_openlibs(L);

	reg_host_functions(L);
	 
    short fgroup = get_level_fgroup(lvnum);
    char* fname = prepare_file_fmtpath(fgroup, "map%05lu.lua", (unsigned long)lvnum);
	// Load and parse the Lua File

    char* source;
    char* chunkname = "chunk";
    size_t len = LbFileLoadAt(fname, source);
    size_t bytecodeSize = 0;
    char* bytecode = luau_compile(source, len, NULL, &bytecodeSize);
    int result = luau_load(L, chunkname, bytecode, bytecodeSize, 0);
    free(bytecode);
    free(source);
    //if (result == 0)

    
	//if(!CheckLua(Lvl_script, luaL_dofile(Lvl_script, fname)))
	{
        close_lua_script();
	}
}



void lua_chatmsg(PlayerNumber plyr_idx, char *msg)
{
    lua_getglobal(L, "ChatMsg");
	if (lua_isfunction(L, -1))
	{
        //arguments the function needs
		lua_pushinteger(L, plyr_idx);
		lua_pushstring(L, msg);

        //2 is how many arguments C passes to lua, 1 is how many lua passes back to C
		CheckLua(L, lua_pcall(L, 2, 1, 0));
	}
}


void lua_game_start()
{
    lua_getglobal(L, "GameStart");
	if (lua_isfunction(L, -1))
	{
		CheckLua(L, lua_pcall(L, 0, 0, 0));
	}
}

void lua_game_loop()
{
    lua_getglobal(L, "GameLoop");
	if (lua_isfunction(L, -1))
	{
		CheckLua(L, lua_pcall(L, 0, 0, 0));
	}
}