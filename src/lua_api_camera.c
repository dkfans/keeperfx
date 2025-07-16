#include "pre_inc.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "bflib_basics.h"
#include "globals.h"
#include "map_blocks.h"
#include "map_data.h"
#include "thing_data.h"
#include "thing_creature.h"
#include "slab_data.h"

#include "lua_base.h"
#include "lua_params.h"
#include "lua_utils.h"

#include "post_inc.h"
#include "game_merge.h"
#include "lvl_script_lib.h"

/**********************************************/

int luaL_checkCamera(lua_State *L, int idx)
{
    if (!lua_istable(L, idx)) {
        return luaL_argerror(L, idx, "Expected a table");
    }

    lua_getfield(L, idx, "playerId");
    if (!lua_isnumber(L, -1)) {
        return luaL_argerror(L, idx, "Table must have a numeric 'playerId' field");
    }
    playerId = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return playerId;
}

static const struct luaL_Reg slab_methods[] = {
     {NULL, NULL}
 };


 static int camera_tostring(lua_State *L)
 {
    int playerId = luaL_checkCamera(L, 1);
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Camera(%d)", playerId);
    lua_pushstring(L, buffer);
    return 1;
 }
 
 // Function to set field values
 static int camera_set_field(lua_State *L) {
    int playerId = luaL_checkCamera(L, 1, &playerId);
    const char* key = luaL_checkstring(L, 2);
    
    struct PlayerInfo *player = get_player(playerId);
    struct Camera* cam = player->acamera;

    if (strcmp(key, "pos") == 0) {
        luaL_checkPos(L, 3, &cam->mappos);
    } else if (strcmp(key, "yaw") == 0) {
        cam->orient_a = luaL_checkinteger(L, 3);
    } else if (strcmp(key, "pitch") == 0) {
        cam->orient_b = luaL_checkinteger(L, 3);
    } else if (strcmp(key, "roll") == 0) {
        cam->orient_c = luaL_checkinteger(L, 3);
    } else if (strcmp(key, "horizontal_fov") == 0) {
        cam->horizontal_fov = luaL_checkinteger(L, 3);
    } else if (strcmp(key, "zoom") == 0) {
        cam->zoom = luaL_checkinteger(L, 3);
    } else if (strcmp(key, "view_mode") == 0) {
        cam->view_mode = luaL_checkinteger(L, 3);
    } else {
        return luaL_error(L, "Unknown field '%s' for Camera", key);
    }

    return 0;
 }
 
 // Function to get field values
 static int camera_get_field(lua_State *L) {

    int playerId = luaL_checkCamera(L, 1, &playerId);

    const char* key = luaL_checkstring(L, 2);

    if (try_get_c_method(L, key, slab_methods))
    {
        return 1;
    }

    sturct PlayerInfo *player = get_player(playerId);
    const struct Camera* cam = player->acamera;

    if (strcmp(key, "pos") == 0) {
        lua_pushPos(L, cam.mappos);
    } else if (strcmp(key, "yaw") == 0) {
        lua_pushinteger(L, cam.orient_a);
    } else if (strcmp(key, "pitch") == 0) {
        lua_pushinteger(L, cam.orient_b);
    } else if (strcmp(key, "roll") == 0) {
        lua_pushinteger(L, cam.orient_c);
    } else if (strcmp(key, "horizontal_fov") == 0) {
        lua_pushinteger(L, cam.horizontal_fov);
    } else if (strcmp(key, "zoom") == 0) {
        lua_pushinteger(L, cam.zoom);
    } else if (strcmp(key, "view_mode") == 0) {
        lua_pushinteger(L, cam.view_mode);
    } else if (try_get_from_methods(L, 1, key)) {
        return 1;
    } else {
        return luaL_error(L, "Unknown field or method '%s' for Player", key);
    }

    return 1;
 
 }

static int camera_eq(lua_State *L) {
    int playerId1, playerId2;

    playerId1 = luaL_checkCamera(L, 1);
    playerId2 = luaL_checkCamera(L, 2);

    // Compare the coordinates
    lua_pushboolean(L, (playerId1 == playerId2));
    return 1;
}

 static const struct luaL_Reg camera_meta[] = {
     {"__tostring", camera_tostring},
     {"__index",    camera_get_field},
     {"__newindex", camera_set_field},
     {"__eq",       camera_eq},
     {NULL, NULL}
 };
 
 void Camera_register(lua_State *L) {
     // Create a metatable for thing and add it to the registry
     luaL_newmetatable(L, "Camera");
 
     // Set the __index and __newindex metamethods
     luaL_setfuncs(L, slab_meta, 0);
  
     // Hide the metatable by setting the __metatable field to nil
     lua_pushliteral(L, "__metatable");
     lua_pushnil(L);
     lua_rawset(L, -3);
 
     // Pop the metatable from the stack
     lua_pop(L, 1);    
 
 }