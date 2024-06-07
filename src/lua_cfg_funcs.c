#include "pre_inc.h"
#include "globals.h"
#include "config.h"
#include "lua_base.h"
#include "game_legacy.h"
#include "post_inc.h"

FuncIdx get_function_idx(const char *func_name, const struct NamedCommand *Cfuncs) {
    // If it's a C function, return positive index
    FuncIdx id = get_id(Cfuncs, func_name);
    if (id > 0) {
        return id;
    }

    // Add it to the list and return the new negative index
    lua_getglobal(Lvl_script, func_name);
    if (lua_isfunction(Lvl_script, -1)) {
        for (size_t i = 1; i < LUA_FUNCS_MAX; i++) {
            if (strcasecmp(game.conf.lua.lua_funcs[i], func_name) == 0) {
                return -i;
            }

            if (game.conf.lua.lua_funcs[i][0] == '\0') {
                strncpy(game.conf.lua.lua_funcs[i], func_name, LUA_FUNCNAME_LENGTH - 1);
                game.conf.lua.lua_funcs[i][LUA_FUNCNAME_LENGTH - 1] = '\0'; // Ensure null-termination
                return -i;
            }
        }
        ERRORLOG("Exceeding max of %d Lua functions used by cfgs", LUA_FUNCS_MAX);
        return 0;
    }
    ERRORLOG("Couldn't find function '%s'", func_name);
    return 0;
}

char *get_function_name(FuncIdx func_idx) {
    if (func_idx > 0 || func_idx <= -LUA_FUNCS_MAX) {
        ERRORLOG("Invalid function index: %d", func_idx);
        return NULL;
    }
    return game.conf.lua.lua_funcs[-func_idx];
}

TbResult luafunc_magic_use_power(FuncIdx func_idx, PlayerNumber plyr_idx, PowerKind pwkind,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing, unsigned long allow_flags) {

    const char *func_name = get_function_name(func_idx);
    if (!func_name) {
        ERRORLOG("Invalid function index: %d", func_idx);
        return -1; // Indicate an error
    }

    lua_getglobal(Lvl_script, func_name);
    if (lua_isfunction(Lvl_script, -1)) {
        lua_pushPlayer(Lvl_script, plyr_idx);
        lua_pushinteger(Lvl_script, pwkind);
        lua_pushinteger(Lvl_script, splevel);
        lua_pushinteger(Lvl_script, stl_x);
        lua_pushinteger(Lvl_script, stl_y);
        lua_pushlightuserdata(Lvl_script, thing);
        lua_pushinteger(Lvl_script, allow_flags);

        if (lua_pcall(Lvl_script, 7, 0, 0) != LUA_OK) {
            const char *error_msg = lua_tostring(Lvl_script, -1);
            ERRORLOG("Error calling Lua function '%s': %s", func_name, error_msg);
            lua_pop(Lvl_script, 1); // Remove error message from stack
            return Lb_FAIL; // Indicate an error
        }
        return 0; // Indicate success
    } else {
        ERRORLOG("Lua function '%s' not found or not a function", func_name);
        lua_pop(Lvl_script, 1); // Remove non-function value from stack
        return Lb_FAIL; // Indicate an error
    }
}
