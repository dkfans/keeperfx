#include "pre_inc.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


#include "lua_base.h"
#include "lua_params.h"


#include "player_data.h"
#include "lvl_script_lib.h"
#include "player_utils.h"


#include "post_inc.h"


static const struct luaL_Reg room_methods[] = {
   //{"set_texture"                          ,lua_Set_texture                     },
    {NULL, NULL}
};


static int room_tostring(lua_State *L)
{
    struct Room* room = luaL_checkRoom(L, 1);
    
    lua_pushstring(L,);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "[%s %s %d]", get_conf_parameter_text(room_desc,room->kind), 
                                              get_conf_parameter_text(player_desc,room->owner),
                                              room->index);
    lua_pushstring(L, buffer);
    return 1;

}

// Function to set field values
static int roomset_field(lua_State *L) {

    PlayerNumber room_idx = luaL_checkPlayerSingle(L, 1);
    const char* key = luaL_checkstring(L, 2);
    int value = luaL_checkinteger(L, 3);

    long variable_type;
    long variable_id;

    if (parse_get_varib(key, &variable_id, &variable_type,1))
    {
        set_variable(room_idx,variable_type,variable_id,value);
        return 0;
    }
    

    return 0;
}

// Function to get field values
static int room_get_field(lua_State *L) {
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);
    const char* key = luaL_checkstring(L, 2);

    // Check if the key exists in room_methods
    for (int i = 0; room_methods[i].name != NULL; i++) {
        if (strcmp(key, room_methods[i].name) == 0) {
            // Instead of calling the function, return its reference
            lua_pushcfunction(L, room_methods[i].func);
            return 1;
        }
    }


    //heart
    if (strcmp(key, "heart") == 0) {
        struct Thing* heartng = get_room_soul_container(plyr_idx);
        lua_pushThing(L, heartng);
        return 1;
    }
    else if (strcmp(key, "controls") == 0) {
        // Push the player index as upvalue
        lua_pushinteger(L, plyr_idx);
        lua_pushcclosure(L, room_get_controls, 1);
        return 1;
    }
    else if (strcmp(key, "available") == 0) {
        // Push the player index as upvalue
        lua_pushinteger(L, plyr_idx);
        lua_pushcclosure(L, room_get_available, 1);
        return 1;
    }
    
    long variable_type;
    long variable_id;

    if (parse_get_varib(key, &variable_id, &variable_type,1))
    {
        lua_pushinteger(L, get_condition_value(plyr_idx, variable_type, variable_id));
        return 1;
    }

/*
    if (strcmp(key, "index") == 0) {
        lua_pushinteger(L, thing->index);
    } else if (strcmp(key, "creation_turn") == 0) {
        lua_pushinteger(L, thing->creation_turn);
    } else if (strcmp(key, "Owner") == 0) {
        lua_pushPlayer(L, thing->owner);


    } else {
        lua_pushnil(L);
    }
*/
    return 1;

}

static int room_eq(lua_State *L) {

    if (!lua_istable(L, 1) || !lua_istable(L, 2)) {
        luaL_error(L, "Expected a table");
        return 1;
    }

    // Get idx field
    lua_getfield(L, 1, "playerId");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'playerId' to be an integer");
        return 1;
    }
    int idx1 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack

    // Get idx field
    lua_getfield(L, 2, "playerId");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'playerId' to be an integer");
        return 1;
    }
    int idx2 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack


    lua_pushboolean(L, idx1 == idx2);
    return 1;
}

static const struct luaL_Reg room_meta[] = {
    {"__tostring", room_tostring},
    {"__index",    room_get_field},
    {"__newindex", room_set_field},
    {"__eq",       room_eq},
    {NULL, NULL}
};

void room_register(lua_State *L) {
    // Create a metatable for thing and add it to the registry
    luaL_newmetatable(L, "Room");

    // Set the __index and __newindex metamethods
    luaL_setfuncs(L, room_meta, 0);

    // Create a methods table
    luaL_newlib(L, room_methods);

    for (int i = 0; room_methods[i].name != NULL; i++) {
        const char *name = roommethods[i].name;
        lua_pushcfunction(L, roommethods[i].func);
        lua_setfield(L, -2, name);
    }

    // Hide the metatable by setting the __metatable field to nil
    lua_pushliteral(L, "__metatable");
    lua_pushnil(L);
    lua_rawset(L, -3);

    // Pop the metatable from the stack
    lua_pop(L, 1);
   

}


