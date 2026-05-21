#include "pre_inc.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "globals.h"
#include "game_legacy.h"         // game.map_tiles_x/y, game.pool, game.texture_id
#include "config_campaigns.h"    // is_map_pack, campaign, LevelInformation, get_level_info
#include "config.h"              // LvKind_IsSingle, LvKind_IsMulti, etc.
#include "config_creature.h"     // creature_desc, CREATURE_TYPES_MAX
#include "engine_textures.h"     // load_texture_map_file
#include "lvl_filesdk1.h"        // get_loaded_level_number, get_level_fgroup
#include "lvl_script_lib.h"      // texture_pack_desc

#include "lua_base.h"
#include "lua_params.h"          // luaL_checkNamedCommand

#include "post_inc.h"

/**********************************************/

//guard against writings
static int map_readonly(lua_State *L) {
    return luaL_error(L, "MAP and its sub-tables are read-only");
}

static int map_tostring(lua_State *L) {
    lua_pushfstring(L, "Map(%dx%d)", game.map_tiles_x, game.map_tiles_y);
    return 1;
}

//read creature pool from a map
static void push_map_pool(lua_State *L) {
    lua_newtable(L);
    for (int i = 0; i < CREATURE_TYPES_MAX; i++) {
        int n = game.pool.crtr_kind[i];
        if (n <= 0) continue;
        lua_pushstring(L, get_conf_parameter_text(creature_desc, i));
        lua_pushinteger(L, n);
        lua_rawset(L, -3);
    }
    // Read-only
    lua_newtable(L);
    lua_pushcfunction(L, map_readonly);
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L,-2);
}

//read map fields
static int map_get_field(lua_State *L){
    const char *key = luaL_checkstring(L, 2);
    LevelNumber lv_number = get_loaded_level_number();
    struct LevelInformation *lv_inf = get_level_info(lv_number);

    const char *map_name = "";
    unsigned long ltype  = 0;
    if (lv_inf) {
        if (lv_inf->name_stridx > 0) {
        map_name = get_string(lv_inf->name_stridx);
        } else {
        map_name = lv_inf->name;
        }
        lv_number    = lv_inf->lvnum;
        ltype    = lv_inf->level_type;
    }

    if (strcmp(key, "creature_pool") == 0)           
        push_map_pool(L);
    else if (strcmp(key, "map_name") == 0) {
        if (lv_inf->name_stridx > 0) {
            map_name = get_string(lv_inf->name_stridx);
        } else {
            map_name = lv_inf->name;
        }          
        lua_pushstring(L, map_name);
    } else if (strcmp(key, "map_number") == 0)          
        lua_pushinteger(L, lv_number);
    else if (strcmp(key, "campaign") == 0)          
        lua_pushstring(L, campaign.name);
    else if (strcmp(key, "map_type") == 0) {     
        if (ltype & LvKind_IsMulti) {
            lua_pushstring(L, "Multiplayer");
        } else if (ltype & LvKind_IsExtra) {
            lua_pushstring(L, "Moon");
        } else if (ltype & LvKind_IsBonus) {
            lua_pushstring(L, "Bonus");
        } else if (ltype & LvKind_IsFree) {
            lua_pushstring(L, "MapPack");
        } else if (ltype & LvKind_IsSingle) {
            lua_pushstring(L, "Campaign");
        } else {
            lua_pushstring(L, "Unknown");
        }
    } else if (strcmp(key, "width") == 0) {
        lua_pushinteger(L, game.map_tiles_x);
    } else if (strcmp(key, "height") == 0) {          
        lua_pushinteger(L, game.map_tiles_y);
    } else if (strcmp(key, "default_texture") == 0) {
        const char *name = get_conf_parameter_text(texture_pack_desc, game.texture_id);
        if (name[0] != '\0') {
            lua_pushstring(L, name);            // "STANDARD", "ANCIENT", ...
        } else {
            lua_pushinteger(L, game.texture_id); // 17, 20, ... (Custom)
        }
    } else {
        return luaL_error(L, "Unknown field '%s' for MAP", key);
    }
    return 1;
}

//write map fields
static int map_set_field(lua_State *L) {
    const char *key = luaL_checkstring(L, 2);

    if (strcmp(key, "default_texture") == 0) {
        long texture_id = luaL_checkNamedCommand(L, 3, texture_pack_desc);
        game.texture_id = texture_id;                           
        LevelNumber lvnumb = get_loaded_level_number();
        load_texture_map_file(texture_id, lvnumb, get_level_fgroup(lvnumb));

        return 0;
    }

    return luaL_error(L, "MAP field '%s' is read-only", key);
}

// Metamethod registration table for the MAP metatable.
static const struct luaL_Reg map_meta[] = {
    {"__index", map_get_field},
    {"__newindex", map_set_field},
    {"__tostring", map_tostring},
    {NULL, NULL}
};

// Builds the global MAP singleton and binds its metatable.
void Map_register(lua_State *L) {
    luaL_newmetatable(L, "Map");
    luaL_setfuncs(L, map_meta, 0);

    lua_pushliteral(L, "__metatable");
    lua_pushnil(L);
    lua_rawset(L, -3);

    lua_pop(L, 1);

    lua_newtable(L);

    lua_pushstring(L, "Map");
    lua_setfield(L, -2, "__class");

    luaL_getmetatable(L, "Map");
    lua_setmetatable(L, -2);

    lua_setglobal(L, "Map");
}