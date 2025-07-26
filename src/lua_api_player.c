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


static int lua_Add_gold_to_player(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);


    GoldAmount gold = luaL_checkinteger(L, 2);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        if(gold > 0)
        {
            player_add_offmap_gold(i, gold);
        }
        else
        {
            take_money_from_dungeon(i, -gold, 0);
        }
    }
    return 0;
}

static int lua_Set_texture(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long texture_id = luaL_checkNamedCommand(L,2,texture_pack_desc);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        set_player_texture(i, texture_id);
    }
    return 0;
}


static const struct luaL_Reg player_methods[] = {
   {"set_texture"                          ,lua_Set_texture                     },   
   {"add_gold"                             ,lua_Add_gold_to_player              },
   //{"SET_PLAYER_COLOR"                     ,lua_SET_PLAYER_COLOR                },
   //{"SET_PLAYER_MODIFIER"                  ,lua_SET_PLAYER_MODIFIER             },
   //{"ADD_TO_PLAYER_MODIFIER"               ,lua_ADD_TO_PLAYER_MODIFIER          },
    {NULL, NULL}
};


static int player_get_controls(lua_State *L) {
    // Upvalue is the player number
    PlayerNumber plyr_idx = lua_tointeger(L, lua_upvalueindex(1));

    // Get the requested control key
    ThingModel crt_id = luaL_checkNamedCommand(L, 2, creature_desc);

    lua_pushinteger(L, get_condition_value(plyr_idx, SVar_CONTROLS_CREATURE, crt_id));
    return 1;
}

static int player_get_available(lua_State *L) {
    // Upvalue is the player number
    PlayerNumber plyr_idx = lua_tointeger(L, lua_upvalueindex(1));

    if(lua_isstring(L, 2))
    {
        unsigned char svartype = 0;

        //creature_type|room_type|power_kind|trap_type|door_type
        const char* text = lua_tostring(L, 2);
        long id = get_rid(creature_desc, text);
        svartype = SVar_AVAILABLE_CREATURE;
        if(id == -1)
        {
            id = get_rid(room_desc, text);
            svartype = SVar_AVAILABLE_ROOM;
        }
        if(id == -1)
        {
            id = get_rid(power_desc, text);
            svartype = SVar_AVAILABLE_MAGIC;
        }
        if(id == -1)
        {
            id = get_rid(trap_desc, text);
            svartype = SVar_AVAILABLE_TRAP;
        }
        if(id == -1)
        {
            id = get_rid(door_desc, text);
            svartype = SVar_AVAILABLE_DOOR;
        }
        if(id == -1)
        {
            return luaL_argerror(L, 2, "unrecognized command");
        }
        lua_pushinteger(L, get_condition_value(plyr_idx, svartype, id));
        return 1;
    }

    return luaL_argerror(L, 2, "unrecognized command");
}

static int player_tostring(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    
    lua_pushstring(L,get_conf_parameter_text(player_desc,player_idx));
    return 1;

}

// Function to set field values
static int player_set_field(lua_State *L) {

    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    const char* key = luaL_checkstring(L, 2);
    int value = luaL_checkinteger(L, 3);

    long variable_type;
    long variable_id;

    if (parse_get_varib(key, &variable_id, &variable_type,1))
    {
        set_variable(player_idx,variable_type,variable_id,value);
        return 0;
    }

    return 0;
}

static int player_get_field(lua_State *L) {
    const char* key = luaL_checkstring(L, 2);
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);

    long variable_type, variable_id;

    // C method lookup
    if (try_get_c_method(L, key, player_methods))
        return 1;

    // Built-in fields
    if (strcmp(key, "camera") == 0) {
        lua_pushCamera(L, plyr_idx);
    } else if (strcmp(key, "heart") == 0) {
        lua_pushThing(L, get_player_soul_container(plyr_idx));
    } else if (strcmp(key, "controls") == 0) {
        lua_pushinteger(L, plyr_idx);
        lua_pushcclosure(L, player_get_controls, 1);
    } else if (strcmp(key, "available") == 0) {
        lua_pushinteger(L, plyr_idx);
        lua_pushcclosure(L, player_get_available, 1);
    }
    else if (parse_get_varib(key, &variable_id, &variable_type, 1)) {
        lua_pushinteger(L, get_condition_value(plyr_idx, variable_type, variable_id));
    } else if (try_get_from_methods(L, 1, key)) {
        return 1;
    } else {
        return luaL_error(L, "Unknown field or method '%s' for Player", key);
    }

    return 1;
}

static int player_eq(lua_State *L) {

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

static const struct luaL_Reg player_meta[] = {
    {"__tostring", player_tostring},
    {"__index",    player_get_field},
    {"__newindex", player_set_field},
    {"__eq",       player_eq},
    {NULL, NULL}
};

void Player_register(lua_State *L) {
    // Create a metatable for thing and add it to the registry
    luaL_newmetatable(L, "Player");

    // Set the __index and __newindex metamethods
    luaL_setfuncs(L, player_meta, 0);

    // Create a methods table
    luaL_newlib(L, player_methods);

    for (int i = 0; player_methods[i].name != NULL; i++) {
        const char *name = player_methods[i].name;
        lua_pushcfunction(L, player_methods[i].func);
        lua_setfield(L, -2, name);
    }

    // Hide the metatable by setting the __metatable field to nil
    lua_pushliteral(L, "__metatable");
    lua_pushnil(L);
    lua_rawset(L, -3);

    // Pop the metatable from the stack
    lua_pop(L, 1);


    for (PlayerNumber i = 0; i < PLAYERS_COUNT; i++)
    {
        lua_pushPlayer(L,i);
        lua_setglobal(L, get_conf_parameter_text(player_desc,i));
    }
    

}


