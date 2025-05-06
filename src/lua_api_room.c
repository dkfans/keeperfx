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
static int room_set_field(lua_State *L) {

    struct Room* room = luaL_checkRoom(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "owner") == 0) {
        take_over_room(room, luaL_checkPlayerSingle(L, 3));
        return 0;
    }
    else if (strcmp(key, "health") == 0) {
        room->health = luaL_checkinteger(L, 3);
        return 1;
    }
    
    luaL_error(L, "Invalid field name '%s' for Room", key);
    return 0;
}

static void push_room_slabs(lua_State *L, struct Room* room) {
    // Create a new table for slabs
    lua_newtable(L);


    while (1)
    {
        MapSlabCoord slb_x = slb_num_decode_x(slbnum);
        MapSlabCoord slb_y = slb_num_decode_y(slbnum);
        // Per slab code
        if (gold_store <= 0)
            break;
        gold_store = add_gold_to_treasure_room_slab(slb_x, slb_y, gold_store);
        // Per slab code ends
        slbnum = get_next_slab_number_in_room(slbnum);
        if (slbnum == 0) {
            slbnum = room->slabs_list;
        }
        k++;
        if (k >= room->slabs_count) {
            break;
        }
    }

    // Iterate through the slabs and add them to the table
    for (int i = 0; i < room->slabs_count; i++) {
        struct Slab* slab = get_slab(room->slabs_list[i]);
        lua_pushinteger(L, slab->index);
        lua_seti(L, -2, i + 1); // Lua tables are 1-indexed
    }
}

// Function to get field values
static int room_get_field(lua_State *L) {
    struct Room* room = luaL_checkRoom(L, 1);
    const char* key = luaL_checkstring(L, 2);

    // Check if the key exists in room_methods
    for (int i = 0; room_methods[i].name != NULL; i++) {
        if (strcmp(key, room_methods[i].name) == 0) {
            // Instead of calling the function, return its reference
            lua_pushcfunction(L, room_methods[i].func);
            return 1;
        }
    }

    if (strcmp(key, "type") == 0) {
        
        return 1;
    }
    else if (strcmp(key, "owner") == 0) {
        lua_pushPlayer(L, room->owner);
        return 1;
    }
    else if (strcmp(key, "slabs") == 0) {
        push_room_slabs(L, room);
        return 1;
    }
    else if (strcmp(key, "workers") == 0) {
        return 1;
    }
    else if (strcmp(key, "health") == 0) {
        lua_pushinteger(L, room->health);
        return 1;
    }
    else if (strcmp(key, "max_health") == 0) {
        lua_pushinteger(L, compute_room_max_health(room->slabs_count, room->efficiency));
        return 1;
    }
    else if (strcmp(key, "used_capacity") == 0) {
        lua_pushinteger(L, room->used_capacity);
        return 1;
    }
    else if (strcmp(key, "max_capacity") == 0) {
        lua_pushinteger(L, room->total_capacity);
        return 1;
    }
    else if (strcmp(key, "efficiency") == 0) {
        lua_pushinteger(L, room->efficiency);
        return 1;
    }

    return 0;

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


