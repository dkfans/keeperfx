#include "pre_inc.h"

#include "lua_params.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "bflib_basics.h"
#include "config_rules.h"
#include "creature_states.h"
#include "custom_sprites.h"
#include "game_legacy.h"
#include "globals.h"
#include "gui_msgs.h"
#include "keeperfx.hpp"
#include "lua_base.h"
#include "lvl_script_lib.h"
#include "map_data.h"
#include "player_utils.h"
#include "room_library.h"
#include "thing_data.h"
#include "thing_navigate.h"

#include "post_inc.h"

/**********************************************/

TbBool luaL_isThing(lua_State *L, int index)
{
    if (!lua_istable(L, index)) {
        return false;
    }

    // Get idx field
    lua_getfield(L, index, "ThingIndex");
    if (!lua_isnumber(L, -1)) {
        return false;
    }
    int idx = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack

    // Get creation_turn field
    lua_getfield(L, index, "creation_turn");
    if (!lua_isnumber(L, -1)) {
        return false;
    }
    int creation_turn = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the creation_turn value off the stack

    struct Thing* thing = thing_get(idx);
    if (thing_is_invalid(thing) || thing->creation_turn != creation_turn) {
        return false;
    }
    return true;
}

TbBool luaL_isCreature(lua_State *L, int index)
{
    if (!luaL_isThing(L, index)) {
        return false;
    }

    struct Thing *thing = luaL_checkThing(L, index);
    if (thing->class_id != TCls_Creature)
    {
        return false;
    }
    return true;
}

TbBool luaL_isPlayer(lua_State *L, int index)
{
    if(lua_isstring(L, index))
    {
        const char* plrname = lua_tostring(L, index);
        return get_id(player_desc, plrname) != -1;
    }
    else if (lua_istable(L, index))
    {
        lua_getfield(L, index, "playerId");
        if (lua_isnumber(L, -1)) {
            return true;
        }
        return false;
    }

    return false;
}

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

        luaL_argcheck(L,id != -1,index,"unrecognized command");

        return id;
    }
    luaL_argerror(L,index,"unrecognized command");
    return 0;
}

long luaL_optCheckinteger(lua_State* L, int index)
{
    if (lua_isnone(L, index))
        return -1;
    return luaL_checkinteger(L, index);
}

long luaL_optNamedCommand(lua_State *L, int index,const struct NamedCommand * commanddesc)
{
    if (lua_isnone(L,index))
        return 0;
    return luaL_checkNamedCommand(L,index,commanddesc);
}

struct Thing *luaL_checkThing(lua_State *L, int index)
{
    if (!lua_istable(L, index)) {
        luaL_argerror(L,index, "Expected a table");
        return NULL;
    }

    // Get idx field
    lua_getfield(L, index, "ThingIndex");
    if (!lua_isnumber(L, -1)) {
        luaL_argerror(L,index, "Expected 'index' to be an integer");
        return NULL;
    }
    int idx = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack

    // Get creation_turn field
    lua_getfield(L, index, "creation_turn");
    if (!lua_isnumber(L, -1)) {
        luaL_argerror(L,index, "Expected 'creation_turn' to be an integer");
        return NULL;
    }
    int creation_turn = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the creation_turn value off the stack

    struct Thing* thing = thing_get(idx);
    if (thing_is_invalid(thing) || thing->creation_turn != creation_turn) {
        luaL_argerror(L,index, "Failed to resolve thing");
        return NULL;
    }
    return thing;
}

struct Thing *luaL_checkCreature(lua_State *L, int index)
{
    struct Thing *thing = luaL_checkThing(L, index);
    if (thing->class_id != TCls_Creature)
    {
        luaL_argerror(L,index, "Expected a creature");
        return NULL;
    }
    return thing;
}

TbMapLocation luaL_checkLocation(lua_State *L, int index)
{
    if (luaL_isPlayer(L, index))
    {
        PlayerNumber playerId = luaL_checkPlayerSingle(L,index);
        return  ((unsigned long)playerId << 4) | MLoc_PLAYERSHEART;
    }
    else if (lua_istable(L, index)) {
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
        const char* err_msg = lua_pushfstring(L, "Invalid location, '%s'", locname);
        return luaL_argerror(L, index, err_msg);
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

PlayerNumber luaL_checkPlayerRangeId(lua_State *L, int index)
{
    if (lua_istable(L, index))
    {
        lua_getfield(L, index, "playerId");
        if (lua_isnumber(L, -1)) {
            int i = lua_tointeger(L, -1);
            return i;
        }
        luaL_argerror(L,index, "Expected table to be of class Player");
        return -1;
    }

    const char* plrname = lua_tostring(L, index);

    return get_id(player_desc, plrname);
}

struct PlayerRange luaL_checkPlayerRange(lua_State *L, int index)
{
    struct PlayerRange playerRange = {0,0};

    long plr_range_id = luaL_checkPlayerRangeId(L,index);

    if (plr_range_id == ALL_PLAYERS)
    {
        playerRange.start_idx = 0;
        playerRange.end_idx = PLAYERS_COUNT;
    }
    else
    {
        playerRange.start_idx = plr_range_id;
        playerRange.end_idx   = plr_range_id + 1;
    }

    return playerRange;
}

PlayerNumber luaL_checkPlayerSingle(lua_State *L, int index)
{
    PlayerNumber playerId = luaL_checkPlayerRangeId(L,index);
    if(playerId == ALL_PLAYERS)
    {
        luaL_argerror(L,index,"player range not supported for this command");
    }
    if(playerId > PLAYERS_COUNT || playerId < 0)
    {
        luaL_argerror(L,index,"expected a valid player");
    }
    return playerId;
}

PlayerNumber luaL_optPlayerSingle(lua_State *L, int index)
{
    if (lua_isnone(L,index))
        return PLAYER_NEUTRAL;
    return luaL_checkPlayerSingle(L,index);
}

MapSubtlCoord luaL_checkstl_x(lua_State *L, int index)
{
    MapSubtlCoord stl_x = luaL_checkint(L,index);
    luaL_argcheck(L, 0 <= stl_x && stl_x <= game.map_subtiles_x, index,
                       "x subtile coord out of range");
    return stl_x;
}

MapSubtlCoord luaL_checkstl_y(lua_State *L, int index)
{
    MapSubtlCoord stl_y = luaL_checkint(L,index);
    luaL_argcheck(L, 0 <= stl_y && stl_y <= game.map_subtiles_y, index,
                       "y subtile coord out of range");
    return stl_y;
}

MapSlabCoord luaL_checkslb_x(lua_State *L, int index)
{
    MapSlabCoord slb_x = luaL_checkint(L,index);
    luaL_argcheck(L, 0 <= slb_x && slb_x <= game.map_tiles_x, index,
                       "x slab coord out of range");
    return slb_x;
}

MapSlabCoord luaL_checkslb_y(lua_State *L, int index)
{
    MapSlabCoord slb_y = luaL_checkint(L,index);
    luaL_argcheck(L, 0 <= slb_y && slb_y <= game.map_tiles_y, index,
                       "y slab coord out of range");
    return slb_y;
}

ActionPointId luaL_checkActionPoint(lua_State *L, int index)
{
    int apt_num = luaL_checkint(L,index);
    ActionPointId apt_idx = action_point_number_to_index(apt_num);
    if (!action_point_exists_idx(apt_idx))
    {
        return luaL_argerror(L, index, lua_pushfstring(L, "Non-existing Action Point, no %d", apt_num));
    }
    return apt_idx;
}

unsigned char luaL_checkCrtLevel(lua_State *L, int index)
{
    MapSubtlCoord crtr_level = luaL_checkint(L,index);

    luaL_argcheck(L, (crtr_level > 0) && (crtr_level <= CREATURE_MAX_LEVEL), index,
                       "Invalid CREATURE LEVEL parameter");
    return crtr_level - 1; //-1 because C code counts 0-9 instead of 1-10

}

unsigned char luaL_checkParty(lua_State *L, int index)
{
    const char *party_name = lua_tostring(L,  index);
    // Recognize party name
    int prty_id = get_party_index_of_name(party_name);
    if (prty_id < 0)
    {
        return luaL_argerror(L, index, lua_pushfstring(L, "Party of requested name, '%s', is not defined", party_name));
    }
    return prty_id;
}

void luaL_checkMessageIcon(lua_State *L, int index, char* type, char* id)
{
    if (lua_isnone(L, index))
    {
        *id = 0;
        *type = MsgType_Blank;
        return;
    }
    else if (luaL_isCreature(L, index))
    {
        struct Thing *thing = luaL_checkCreature(L, index);
        *id = thing->model;
        *type = MsgType_Creature;
        return;
    }
    else if(luaL_isPlayer(L, index))
    {
        PlayerNumber plr_idx = luaL_checkPlayerRangeId(L, index);
        *id = plr_idx;
        *type = MsgType_Player;
        return;
    }
    else if (lua_isstring(L, index))
    {
        const char *icon_text = lua_tostring(L,  index);
        get_chat_icon_from_value(icon_text, id, type);
        return;
    }

    luaL_argerror(L, index, "Message icon param not recognised");
    
}

EffectOrEffElModel luaL_checkEffectOrEffElModel(lua_State *L, int index)
{
    if (lua_isnumber(L, index))
    {
        return lua_tointeger(L, index);
    }
    else if(lua_isstring(L, index))
    {
        const char* text = lua_tostring(L, index);
        long id = effect_or_effect_element_id(text);

        luaL_argcheck(L,id != 0,index,"invalid effect option");

        return id;
    }
    luaL_argerror(L, index,"invalid effect option");
    return 0;
}

long luaL_checkCreature_or_creature_wildcard(lua_State *L, int index)
{
    if (lua_isnumber(L, index))
    {
        return lua_tointeger(L, index);
    }
    else if(lua_isstring(L, index))
    {
        const char* text = lua_tostring(L, index);
        long id = get_rid(creature_desc, text);

        if (0 == strcasecmp(text, "ANY_CREATURE"))
        {
            return CREATURE_NOT_A_DIGGER; //For scripts, when we say 'ANY_CREATURE' we exclude diggers.
        }

        luaL_argcheck(L,id != -1,index,"unrecognized creature type");

        return id;
    }
    luaL_argerror(L,index,"invalid creature type");
    return 0;

}
    
long luaL_checkIntMinMax(lua_State *L, int index,long min, long max)
{
    long val = luaL_checkinteger(L,index);
    luaL_argcheck(L, min <= val && val <= max, index, "value out of range");
    return val;
}

int luaL_checkSlab(lua_State *L, int idx, MapSlabCoord* slb_x, MapSlabCoord* slb_y)
{
    *slb_x = 0;
    *slb_y = 0;
    if (!lua_istable(L, idx)) {
        return luaL_argerror(L, idx, "Expected a table with 'slb_x' and 'slb_y'");
    }

    lua_getfield(L, idx, "slb_x");
    if (!lua_isnumber(L, -1)) {
        return luaL_argerror(L, idx, "Table must have a numeric 'slb_x' field");
    }
    *slb_x = lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, idx, "slb_y");
    if (!lua_isnumber(L, -1)) {
        return luaL_argerror(L, idx, "Table must have a numeric 'slb_y' field");
    }
    *slb_y = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return 0;
}

struct Room* luaL_checkRoom(lua_State *L, int idx)
{
    if (!lua_istable(L, idx)) {
        luaL_argerror(L, idx, "Expected a room");
        return INVALID_ROOM;
    }

    lua_getfield(L, idx, "room_idx");
    if (!lua_isnumber(L, -1)) {
        luaL_argerror(L, idx, "Expected a room");
        return INVALID_ROOM;
    }
    RoomIndex room_idx = lua_tointeger(L, -1);
    lua_pop(L, 1);


    struct Room* room = room_get(room_idx);
    if ((room->alloc_flags & RoF_Allocated) == 0)
    {
        luaL_argerror(L, idx, "Room no longer exists");
        return INVALID_ROOM;
    }


    lua_getfield(L, idx, "creation_turn");
    if (!lua_isnumber(L, -1)) {
        luaL_argerror(L, idx, "Table must have a numeric 'creation_turn' field");
        return INVALID_ROOM;
    }
    GameTurn creation_turn = lua_tointeger(L, -1);
    lua_pop(L, 1);
    if (room_is_invalid(room) || room->creation_turn != creation_turn) {
        luaL_argerror(L, idx, "Failed to resolve room");
        return INVALID_ROOM;
    }
    return room;
}

void luaL_checkCoord3d(lua_State *L, int index, struct Coord3d* pos)
{
    if (lua_istable(L, index)) {

        lua_getfield(L, index, "val_x");
        pos->x.val = lua_tointeger(L, -1);
        lua_getfield(L, index, "val_y");
        pos->y.val = lua_tointeger(L, -1);
        lua_getfield(L, index, "val_z");
        pos->z.val = lua_tointeger(L, -1);

        return;
    }
    luaL_argerror(L, index, "expected a pos");
    return;
}

long luaL_checkAnimationId(lua_State* L, int index)
{
    if (lua_isnumber(L, index))
    {
        return lua_tointeger(L, index);
    }
    else if (lua_isstring(L, index))
    {
        const char* text = lua_tostring(L, index);
        long id = get_anim_id_(text);

        luaL_argcheck(L, id != -1, index, "unrecognized command");

        return id;
    }
    luaL_argerror(L, index, "unrecognized command");
    return 0;
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
    
    // Store a class name for Bitser
    lua_pushstring(L, "Thing");
    lua_setfield(L, -2, "__class");

    luaL_getmetatable(L, "Thing");
    lua_setmetatable(L, -2);
}

void lua_pushPlayer(lua_State *L, PlayerNumber plr_idx) {

    lua_createtable(L, 0, 2);

    lua_pushinteger(L, plr_idx);
    lua_setfield(L, -2, "playerId");

    // Store a class name for Bitser
    lua_pushstring(L, "Player");
    lua_setfield(L, -2, "__class");

    luaL_getmetatable(L, "Player");
    lua_setmetatable(L, -2);  
}

void lua_pushPos(lua_State *L, struct Coord3d* pos) {
    // Create a new table with 3 fields
    lua_createtable(L, 0, 3);

    // Set val_x
    lua_pushinteger(L, pos->x.val);
    lua_setfield(L, -2, "val_x");

    // Set val_y
    lua_pushinteger(L, pos->y.val);
    lua_setfield(L, -2, "val_y");

    // Set val_z
    lua_pushinteger(L, pos->z.val);
    lua_setfield(L, -2, "val_z");

    // Get metatable from registry
    lua_getfield(L, LUA_REGISTRYINDEX, "Pos3d");
    lua_setmetatable(L, -2);
}

void lua_pushSlab(lua_State *L, MapSlabCoord slb_x, MapSlabCoord slb_y) {

    if (slb_x < 0 || slb_x >= game.map_tiles_x || slb_y < 0 || slb_y >= game.map_tiles_y) {
        lua_pushnil(L);
        return;
    }
    lua_createtable(L, 0, 2);

    lua_pushinteger(L, slb_x);
    lua_setfield(L, -2, "slb_x");

    lua_pushinteger(L,  slb_y);
    lua_setfield(L, -2, "slb_y");

    // Store a class name for Bitser
    lua_pushstring(L, "Slab");
    lua_setfield(L, -2, "__class");

    luaL_getmetatable(L, "Slab");
    lua_setmetatable(L, -2);  
}

//takes the leader of the party as argument
//pushes a table of all the creatures in the party onto the stack
void lua_pushPartyTable(lua_State *L, struct Thing* thing) {
    lua_newtable(L);
    long i = thing->index;
    long k = 0;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->next_in_group;
        // Per-thing code
        lua_pushThing(L, thing);
        lua_rawseti(L, -2, k + 1);
        
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

void lua_pushRoom(lua_State *L, struct Room* room) {
    if (room_is_invalid(room)) {
        lua_pushnil(L);
        return;
    }

    lua_createtable(L, 0, 2);

    lua_pushinteger(L, room->index);
    lua_setfield(L, -2, "room_idx");

    lua_pushinteger(L, room->creation_turn);
    lua_setfield(L, -2, "creation_turn");

    // Store a class name for Bitser
    lua_pushstring(L, "Room");
    lua_setfield(L, -2, "__class");

    luaL_getmetatable(L, "Room");
    lua_setmetatable(L, -2);  
}