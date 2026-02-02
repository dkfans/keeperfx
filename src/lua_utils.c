#include "pre_inc.h"

#include "globals.h"
#include "lua_utils.h"

#include "post_inc.h"

TbBool try_get_from_methods(lua_State *L, int obj_index, const char *key) {
    if (!lua_getmetatable(L, obj_index)) return false;

    lua_getfield(L, -1, "__methods");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, key);
        if (!lua_isnil(L, -1)) {
            lua_remove(L, -2); // __methods
            lua_remove(L, -2); // metatable
            return true;
        }
        lua_pop(L, 1); // nil
    }
    lua_pop(L, 2); // __methods or nil + metatable
    return false;
}


TbBool try_get_c_method(lua_State *L, const char *key, const luaL_Reg *methods) {
    for (int i = 0; methods[i].name != NULL; i++) {
        if (strcmp(key, methods[i].name) == 0) {
            lua_pushcfunction(L, methods[i].func);
            return true;
        }
    }
    return false;
}