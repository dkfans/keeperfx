#include "pre_inc.h"

#include "../lib/lua/include/lua.h"
#include "../lib/lua/include/lauxlib.h"
#include "../lib/lua/include/lualib.h"

#include "bflib_basics.h"
#include "globals.h"
#include "thing_data.h"
#include "creature_states.h"
#include "gui_msgs.h"

#include "post_inc.h"



static int lua_HostFunction(lua_State *L)
{
	const char* a = lua_tostring(L, 1);

    JUSTLOG("[CPP S4] HostFunction(%s) called from Lua",a);

	return 0;
}

static int make_thing_zombie(lua_State *L)
{
	int tng_idx = lua_tointeger(L, 1);

    struct Thing* tng = thing_get(tng_idx);
    tng->alloc_flags |= TAlF_IsControlled;


	return 0;
}

static int send_chat_message(lua_State *L)
{
	int plyr_idx = lua_tointeger(L, 1);
    const char* msg = lua_tostring(L, 2);
    message_add(plyr_idx, msg);

	return 2;
}

static int move_thing_to(lua_State *L)
{
	int tng_idx = lua_tointeger(L, 1);

    struct Thing* tng = thing_get(tng_idx);
    tng->alloc_flags |= TAlF_IsControlled;


	return 0;
}


void reg_host_functions(lua_State *L)
{
    lua_register(L, "HostFunction", lua_HostFunction);
    lua_register(L, "MakeThingZombie", make_thing_zombie);
    lua_register(L, "SendChatMessage", send_chat_message);
    lua_register(L, "MoveThingTo", move_thing_to);
}


