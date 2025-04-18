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


static int lua_Add_gold_to_player(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);


    GoldAmount gold = luaL_checkinteger(L, 2);
    JUSTLOG("gold %d",(int)gold);

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

static int lua_SET_PLAYER_COLOR(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long color = luaL_checkNamedCommand(L, 2,cmpgn_human_player_options);
    
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        if (i == PLAYER_NEUTRAL)
        {
            continue;;
        }
        set_player_colour(i, color);
    }
    return 0;
}

static const struct luaL_Reg player_methods[] = {
   {"Set_texture"                          ,lua_Set_texture                     },   
   {"Add_gold"                             ,lua_Add_gold_to_player              },
   {"SET_PLAYER_COLOR"                     ,lua_SET_PLAYER_COLOR                },
   //{"SET_PLAYER_MODIFIER"                  ,lua_SET_PLAYER_MODIFIER             },
   //{"ADD_TO_PLAYER_MODIFIER"               ,lua_ADD_TO_PLAYER_MODIFIER          },
    {NULL, NULL}
};


static int player_get_control(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    //int plyr_idx;

    // Get the player_id from the table
    lua_getfield(L, 1, "player_id");
    //plyr_idx = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    // Get the requested control key
    //const char* action = luaL_checkstring(L, 2);
    //lua_pushinteger(L, get_control_value(plyr_idx, action));
    return 1;
}

static int player_get_controls(lua_State *L) {
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);

    // Create a new table for CONTROLS
    lua_newtable(L);

    // Set its __index to another function
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2);

    // Attach a function that handles the control lookup
    lua_pushcfunction(L, player_get_control);
    lua_setfield(L, -2, "__index");

    // Store the player index inside the table
    lua_pushinteger(L, plyr_idx);
    lua_setfield(L, -2, "player_id");

    return 1;
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

// Function to get field values
static int player_get_field(lua_State *L) {
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);
    const char* key = luaL_checkstring(L, 2);

    // Check if the key exists in player_methods
    for (int i = 0; player_methods[i].name != NULL; i++) {
        if (strcmp(key, player_methods[i].name) == 0) {
            // Instead of calling the function, return its reference
            lua_pushcfunction(L, player_methods[i].func);
            return 1;
        }
    }


    //heart
    if (strcmp(key, "heart") == 0) {
        struct Thing* heartng = get_player_soul_container(plyr_idx);
        lua_pushThing(L, heartng);
        return 1;
    }
    else if (strcmp(key, "CONTROLS") == 0) {
        return player_get_controls(L);
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


