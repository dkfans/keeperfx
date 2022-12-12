#include "pre_inc.h"

#include "../lib/lua/include/lua.h"
#include "../lib/lua/include/lauxlib.h"
#include "../lib/lua/include/lualib.h"

#include "bflib_basics.h"
#include "globals.h"
#include "thing_data.h"
#include "creature_states.h"
#include "gui_msgs.h"
#include "thing_navigate.h"
#include "map_data.h"
#include "game_legacy.h"

#include "post_inc.h"


static int lua_get_creature_near(lua_State *L)
{
    //the arguments lua passes to the C code
	int stl_x = lua_tointeger(L, 1); // the last number is the position of the argument, just increment these
	int stl_y = lua_tointeger(L, 2);

    struct Thing* thing = get_creature_near(stl_x * COORD_PER_STL,stl_y * COORD_PER_STL);

    //arguments you push back to lua
    lua_pushinteger(L, thing->index);
	return 1; // return value is the amount of args you push back
}

static int make_thing_zombie(lua_State *L)
{
	int tng_idx = lua_tointeger(L, 1);

    struct Thing* thing = thing_get(tng_idx);
    thing->alloc_flags |= TAlF_IsControlled;

	return 0;
}

static int send_chat_message(lua_State *L)
{
	int plyr_idx = lua_tointeger(L, 1);
    const char* msg = lua_tostring(L, 2);
    message_add(plyr_idx, msg);

	return 0;
}

static int move_thing_to(lua_State *L)
{
	int tng_idx = lua_tointeger(L, 1);
	int stl_x = lua_tointeger(L, 2);
	int stl_y = lua_tointeger(L, 3);

    struct Thing* thing = thing_get(tng_idx);
    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
        WARNLOG("Move %s order failed",thing_model_name(thing));
    thing->continue_state = CrSt_ManualControl;

	return 0;
}

static int lua_kill_creature(lua_State *L)
{
	int tng_idx = lua_tointeger(L, 1);
    struct Thing* thing = thing_get(tng_idx);
    kill_creature(thing, INVALID_THING, -1, CrDed_NoUnconscious);

	return 0;
}

static int lua_set_player_as_won_level(lua_State *L)
{
	int plr_idx = lua_tointeger(L, 1);
    struct PlayerInfo* player = get_player(plyr_idx);
    set_player_as_won_level(player);

	return 0;
}

static int lua_set_player_as_lost_level(lua_State *L)
{
	int plr_idx = lua_tointeger(L, 1);
    struct PlayerInfo* player = get_player(plyr_idx);
    set_player_as_lost_level(player);

	return 0;
}



void reg_host_functions(lua_State *L)
{
    lua_register(L, "GetCreatureNear", lua_get_creature_near);
    lua_register(L, "MakeThingZombie", make_thing_zombie);
    lua_register(L, "SendChatMessage", send_chat_message);
    lua_register(L, "MoveThingTo", move_thing_to);
    lua_register(L, "KillCreature", lua_kill_creature);
    lua_register(L, "PlayerWin", lua_set_player_as_won_level);
    lua_register(L, "PlayerLose", lua_set_player_as_lost_level);
}


