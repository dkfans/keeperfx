#include "pre_inc.h"

#include "lua_params.h"

#include "../deps/luajit/src/lua.h"
#include "../deps/luajit/src/lauxlib.h"
#include "../deps/luajit/src/lualib.h"

#include "bflib_basics.h"
#include "globals.h"
#include "thing_data.h"
#include "creature_states.h"
#include "gui_msgs.h"
#include "thing_navigate.h"
#include "map_data.h"
#include "game_legacy.h"
#include "player_utils.h"
#include "lvl_script_lib.h"
#include "room_library.h"
#include "keeperfx.hpp"
#include "music_player.h"
#include "lua_base.h"


#include "post_inc.h"

/**********************************************/



/***************************************************************************************************/
/************    Inputs   **************************************************************************/
/***************************************************************************************************/

long luaL_checkNamedCommand(lua_State *L, int index,const struct NamedCommand * commanddesc)
{
    if (lua_isnumber(L, index))
    {
        return lua_tointeger(L, index);
    }
    else if(lua_isstring(L, index))
    {
        const char* text = lua_tostring(L, index);
        long id = get_rid(commanddesc, text);

        luaL_argcheck(L,id != -1,index,"invalid namedcommandoption");

        return id;
    }
    luaL_error(L,"invalid namedcommandoption");
    return 0;

}
struct Thing *luaL_checkThing(lua_State *L, int index)
{
    if (!lua_istable(L, index)) {
        luaL_error(L, "Expected a table");
        return NULL;
    }

    // Get idx field
    lua_getfield(L, index, "ThingIndex");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'index' to be an integer");
        return NULL;
    }
    int idx = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack

    // Get creation_turn field
    lua_getfield(L, index, "creation_turn");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'creation_turn' to be an integer");
        return NULL;
    }
    int creation_turn = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the creation_turn value off the stack

    struct Thing* thing = thing_get(idx);
    if (thing_is_invalid(thing) || thing->creation_turn != creation_turn) {
        luaL_error(L, "Failed to resolve thing");
        return NULL;
    }
    return thing;
}

TbMapLocation luaL_checkLocation(lua_State *L, int index)
{
    if (lua_istable(L, index)) {
        lua_getfield(L, index, "stl_x");
        int stl_x = lua_tointeger(L, -1);
        lua_getfield(L, index, "stl_y");
        int stl_y = lua_tointeger(L, -1);

        return get_coord_encoded_location(stl_x,stl_y);
    }

    const char* locname = lua_tostring(L, index);
    TbMapLocation location;
    if(!get_map_location_id(locname, &location))
    {
        luaL_error (L,"Invalid location, '%s'", locname);
    }
    return location;
}

TbMapLocation luaL_optLocation(lua_State *L, int index)
{
    if (lua_isnone(L,index))
        return 0;
    else
        return luaL_checkLocation(L,index);
}

TbMapLocation luaL_checkHeadingLocation(lua_State *L, int index)
{
    const char* locname = lua_tostring(L, index);
    long target = luaL_checkNamedCommand(L, index + 1,head_for_desc);


    TbMapLocation location;
    if(!get_map_heading_id(locname, target, &location))
    {
        luaL_error (L,"Invalid location, '%s'", locname);
    }
    return location;
}


struct PlayerRange luaL_checkPlayerRange(lua_State *L, int index)
{
    struct PlayerRange playerRange = {0,0};
    if (lua_istable(L, index))
    {
        lua_getfield(L, index, "playerId");
        if (lua_isnumber(L, -1)) {
            int i = lua_tointeger(L, -1);
            playerRange.start_idx = i;
            playerRange.end_idx   = i;
            return playerRange;
        }
        luaL_error(L, "Expected table to be of class Player");
        return playerRange;
    }

    const char* plrname = lua_tostring(L, index);

    long plr_range_id = get_id(player_desc, plrname);
    if (plr_range_id == ALL_PLAYERS)
    {
        playerRange.start_idx = 0;
        playerRange.end_idx = PLAYERS_COUNT;
    }
    else
    {
        playerRange.start_idx = plr_range_id;
        playerRange.end_idx   = plr_range_id;
    }

    return playerRange;
}

PlayerNumber luaL_checkPlayerSingle(lua_State *L, int index)
{
    

    struct PlayerRange playerRange = luaL_checkPlayerRange(L,index);
    if(playerRange.start_idx != playerRange.end_idx)
    {
        luaL_error (L,"player range not supported for this command");
    }
    return playerRange.start_idx;
}

MapSubtlCoord luaL_checkstl_x(lua_State *L, int index)
{
    MapSubtlCoord stl_x = luaL_checkint(L,index);
    luaL_argcheck(L, 0 <= stl_x && stl_x <= gameadd.map_subtiles_x, index,
                       "x subtile coord out of range");
    return stl_x;
}

MapSubtlCoord luaL_checkstl_y(lua_State *L, int index)
{
    MapSubtlCoord stl_y = luaL_checkint(L,index);
    luaL_argcheck(L, 0 <= stl_y && stl_y <= gameadd.map_subtiles_y, index,
                       "y subtile coord out of range");
    return stl_y;
}

ActionPointId luaL_checkActionPoint(lua_State *L, int index)
{
    int apt_num = luaL_checkint(L,index);
    ActionPointId apt_idx = action_point_number_to_index(apt_num);
    if (!action_point_exists_idx(apt_idx))
    {
        luaL_error(L,"Non-existing Action Point, no %d", apt_num);
        return 0;
    }
    return apt_idx;
}




/***************************************************************************************************/
/************    Outputs   *************************************************************************/
/***************************************************************************************************/

void lua_pushThing(lua_State *L, struct Thing* thing) {
    if (thing_is_invalid(thing)) {
        lua_pushnil(L);
        return;
    }

    lua_createtable(L, 0, 2);

    lua_pushinteger(L, thing->index);
    lua_setfield(L, -2, "ThingIndex");

    lua_pushinteger(L, thing->creation_turn);
    lua_setfield(L, -2, "creation_turn");

    luaL_getmetatable(L, "Thing");
    lua_setmetatable(L, -2);
}

void lua_pushPlayer(lua_State *L, PlayerNumber plr_idx) {

    lua_createtable(L, 0, 2);

    lua_pushinteger(L, plr_idx);
    lua_setfield(L, -2, "playerId");

    luaL_getmetatable(L, "Player");
    lua_setmetatable(L, -2);
}

void lua_pushPos(lua_State *L, struct Coord3d* pos) {

    lua_createtable(L, 0, 2);

    lua_pushinteger(L, pos->x.stl.num);
    lua_setfield(L, -2, "stl_x");

    lua_pushinteger(L,  pos->y.stl.num);
    lua_setfield(L, -2, "stl_y");

    //luaL_getmetatable(L, "Thing");
    //lua_setmetatable(L, -2);
}