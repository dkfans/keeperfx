#include "pre_inc.h"
#include "globals.h"
#include "config.h"
#include "lua_base.h"
#include "game_legacy.h"
#include "post_inc.h"






FuncIdx get_function_idx(const char *func_name,const struct NamedCommand * Cfuncs)
{
    //if it's a C func return positive index
    FuncIdx id = get_id(Cfuncs,func_name);
    if (id > 0)
        return id;

    //add it to the list and return the new negative idx
    lua_getglobal(Lvl_script, func_name);
	if (lua_isfunction(Lvl_script, -1))
	{
		for (size_t i = 1; i < LUA_FUNCS_MAX; i++)
        {
            if (strcasecmp(&game.conf.lua.lua_funcs[i][0], func_name) == 0)
                return -i;

            if(game.conf.lua.lua_funcs[i][0] == 0)
            {
                strncpy(&game.conf.lua.lua_funcs[i], func_name, LUA_FUNCNAME_LENGHT);
                return -i;
            }
        }
        ERRORLOG("Exceeding max of %d luafunctions used by cfgs",LUA_FUNCS_MAX);
        return 0;
         
	}
    ERRORLOG("couldn't find function '%s'",func_name);
    return 0;
}  

