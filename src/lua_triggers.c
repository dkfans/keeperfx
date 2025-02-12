#include "pre_inc.h"

#include "../deps/luajit/src/lua.h"
#include "../deps/luajit/src/lauxlib.h"
#include "../deps/luajit/src/lualib.h"


#include "lua_base.h"
#include "lua_api.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "config.h"
#include "config_magic.h"
#include "globals.h"
#include "thing_data.h"


#include "post_inc.h"

void lua_pushThing(lua_State *L, struct Thing* thing);
void lua_pushPlayer(lua_State *L, PlayerNumber plr_idx);

void lua_chatmsg(PlayerNumber plyr_idx, char *msg)
{
	SYNCDBG(6,"Starting");
    lua_getglobal(Lvl_script, "ChatMsg");
	if (lua_isfunction(Lvl_script, -1))
	{
		lua_pushPlayer(Lvl_script, plyr_idx);
		lua_pushstring(Lvl_script, msg);

		CheckLua(Lvl_script, lua_pcall(Lvl_script, 2, 0, 0),"OnChatMsg");
	}
}


void lua_game_start()
{
	SYNCDBG(6,"Starting");
    lua_getglobal(Lvl_script, "OnGameStart");
	if (lua_isfunction(Lvl_script, -1))
	{
		CheckLua(Lvl_script, lua_pcall(Lvl_script, 0, 0, 0),"OnGameStart");
	}
}

void lua_game_tick()
{
	SYNCDBG(6,"Starting");
    lua_getglobal(Lvl_script, "OnGameTick");
	if (lua_isfunction(Lvl_script, -1))
	{
		CheckLua(Lvl_script, lua_pcall(Lvl_script, 0, 0, 0), "OnGameTick");
	}
}

void lua_on_power_cast(PlayerNumber plyr_idx, PowerKind pwkind,
    unsigned short splevel, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct Thing *thing)
	{
	SYNCDBG(6,"Starting");
    lua_getglobal(Lvl_script, "OnPowerCast");
	if (lua_isfunction(Lvl_script, -1))
	{
		lua_pushstring(Lvl_script,get_conf_parameter_text(power_desc,pwkind));
		lua_pushPlayer(Lvl_script, plyr_idx);
		lua_pushThing(Lvl_script, thing); 
		lua_pushinteger(Lvl_script, stl_x);
		lua_pushinteger(Lvl_script, stl_y);
		lua_pushinteger(Lvl_script, splevel);

		CheckLua(Lvl_script, lua_pcall(Lvl_script, 6, 0, 0),"OnPowerCast");
	}
}

void lua_on_special_box_activate(PlayerNumber plyr_idx, struct Thing *cratetng)
{
	SYNCDBG(6,"Starting");
    lua_getglobal(Lvl_script, "OnSpecialActivated");
	if (lua_isfunction(Lvl_script, -1))
	{
		lua_pushPlayer(Lvl_script, plyr_idx);
		lua_pushThing(Lvl_script, cratetng);
		lua_pushinteger(Lvl_script, cratetng->custom_box.box_kind);

		CheckLua(Lvl_script, lua_pcall(Lvl_script, 3, 0, 0),"OnSpecialActivated");
	}
}

static char* lua_serialized_data = NULL;

const char* lua_get_serialised_data(size_t *len)
{
    lua_getglobal(Lvl_script, "GetSerializedData");
	if (lua_isfunction(Lvl_script, -1))
	{

		CheckLua(Lvl_script, lua_pcall(Lvl_script, 0, 1, 0),"GetSerializedData");
		const char *data = lua_tolstring(Lvl_script, -1, len);  // Get the result
        if (data) {
            lua_serialized_data = (char*)malloc(*len);
            memcpy(lua_serialized_data, data, *len);
            lua_pop(Lvl_script, 1);  // Pop the result
            return lua_serialized_data;
        }
		return NULL;
	}
	else
	{
		ERRORLOG("failed to find GetSerializedData lua function");
        lua_pop(Lvl_script, 1);  // Pop nil
		return NULL;
	}
}


void lua_set_serialised_data(const char *data, size_t len)
{
	if(Lvl_script == NULL)
	{
		ERRORLOG("Lvl_script not initialised");
		return;
	}

    lua_getglobal(Lvl_script, "SetSerializedData");
	if (lua_isfunction(Lvl_script, -1))
	{
		lua_pushlstring(Lvl_script, data, len); 
		CheckLua(Lvl_script, lua_pcall(Lvl_script, 1, 0, 0),"SetSerializedData");
	}
	else
	{
		ERRORLOG("failed to find SetSerializedData lua function");
        lua_pop(Lvl_script, 1);  // Pop nil
	}
}

void cleanup_serialized_data() {
    if (lua_serialized_data != NULL) {
        free(lua_serialized_data);
        lua_serialized_data = NULL;
    }
}