#include "pre_inc.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lua_base.h"
#include "lua_params.h"
#include "lua_utils.h"

#include "player_data.h"
#include "lvl_script_lib.h"
#include "player_utils.h"


#include "post_inc.h"


static const struct luaL_Reg room_methods[] = {
    {NULL, NULL}
};


static int room_tostring(lua_State *L)
{
    struct Room* room = luaL_checkRoom(L, 1);
    
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
    }
    else if (strcmp(key, "health") == 0) {
        room->health = luaL_checkinteger(L, 3);
    }
    else
    {
        luaL_error(L, "attempt to assign unassignable field '%s'", key);
    }
    
    return 0;
}

static void push_room_slabs(lua_State *L, struct Room* room) {
    // Create a new table for slabs
    lua_newtable(L);

    SlabCodedCoords slbnum = room->slabs_list;
    int i = 1; // Lua tables are 1-indexed

    while (1) {
        lua_pushSlab(L, slb_num_decode_x(slbnum), slb_num_decode_y(slbnum));
        lua_rawseti(L, -2, i); // Insert at index i
        i++;

        slbnum = get_next_slab_number_in_room(slbnum);
        if (slbnum == 0 || i > room->slabs_count) {
            break;
        }
    }
}


static void push_room_workers(lua_State *L, struct Room* room) {
    // Create a new table for slabs
    lua_newtable(L);

    ThingIndex worker_idx = room->creatures_list;
    int i = 1; // Lua tables are 1-indexed

    while (1) {
        struct Thing* worker = thing_get(worker_idx);
        lua_pushThing(L, worker);
        lua_rawseti(L, -2, i); // Insert at index i
        i++;
        struct CreatureControl* cctrl = creature_control_get_from_thing(worker);

        worker_idx = cctrl->next_in_room;
        if (worker_idx == 0)
        {
          break;
        }
        if (i > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
}

// Function to get field values
static int room_get_field(lua_State *L) {
    struct Room* room = luaL_checkRoom(L, 1);
    const char* key = luaL_checkstring(L, 2);

    // Check if the key exists in room_methods
    if (try_get_c_method(L, key, room_methods))
    {
        return 1;
    }

    if (strcmp(key, "type") == 0) {
        lua_pushstring(L, get_conf_parameter_text(room_desc, room->kind));
    } else if (strcmp(key, "owner") == 0) {
        lua_pushPlayer(L, room->owner);
    } else if (strcmp(key, "slabs") == 0) {
        push_room_slabs(L, room);
    } else if (strcmp(key, "workers") == 0) {
        push_room_workers(L, room);
    } else if (strcmp(key, "health") == 0) {
        lua_pushinteger(L, room->health);
    } else if (strcmp(key, "max_health") == 0) {
        lua_pushinteger(L, compute_room_max_health(room->slabs_count, room->efficiency));
    } else if (strcmp(key, "used_capacity") == 0) {
        lua_pushinteger(L, room->used_capacity);
    } else if (strcmp(key, "max_capacity") == 0) {
        lua_pushinteger(L, room->total_capacity);
    } else if (strcmp(key, "efficiency") == 0) {
        lua_pushinteger(L, room->efficiency);
    } else if (strcmp(key, "centerpos") == 0) {
        struct Coord3d centerpos;
        centerpos.x.val = subtile_coord_center(room->central_stl_x);
        centerpos.y.val = subtile_coord_center(room->central_stl_y);
        centerpos.z.val = get_floor_height_at(&centerpos);
        lua_pushPos(L, &centerpos);
    } else {
        return luaL_error(L, "Unknown field or method '%s' for Room", key);
    }

    return 1;

}

static int room_eq(lua_State *L) {

    struct Room* r1 = luaL_checkRoom(L, 1);
    struct Room* r2 = luaL_checkRoom(L, 2);
    lua_pushboolean(L, r1 == r2);
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
        const char *name = room_methods[i].name;
        lua_pushcfunction(L, room_methods[i].func);
        lua_setfield(L, -2, name);
    }

    // Hide the metatable by setting the __metatable field to nil
    lua_pushliteral(L, "__metatable");
    lua_pushnil(L);
    lua_rawset(L, -3);

    // Pop the metatable from the stack
    lua_pop(L, 1);
   

}


