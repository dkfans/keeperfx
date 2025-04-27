#include "pre_inc.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


#include "lua_triggers.h"

#include "lua_base.h"
#include "lua_api.h"
#include "lua_params.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "config.h"
#include "config_magic.h"
#include "globals.h"
#include "thing_data.h"


#include "post_inc.h"

void lua_on_game_lost(PlayerNumber plyr_idx)
{
	SYNCDBG(6,"Starting");
	lua_getglobal(Lvl_script, "OnGameLost");
	if (lua_isfunction(Lvl_script, -1))
	{
		lua_pushPlayer(Lvl_script, plyr_idx);
		CheckLua(Lvl_script, lua_pcall(Lvl_script, 1, 0, 0),"OnGameLost");
	}
}

void lua_on_chatmsg(PlayerNumber plyr_idx, char *msg)
{
	SYNCDBG(6,"Starting");
    lua_getglobal(Lvl_script, "OnChatMsg");
	if (lua_isfunction(Lvl_script, -1))
	{
		lua_pushPlayer(Lvl_script, plyr_idx);
		lua_pushstring(Lvl_script, msg);

		CheckLua(Lvl_script, lua_pcall(Lvl_script, 2, 0, 0),"OnChatMsg");
	}
}


void lua_on_game_start()
{
	SYNCDBG(6,"Starting");
    lua_getglobal(Lvl_script, "OnGameStart");
	if (lua_isfunction(Lvl_script, -1))
	{
		CheckLua(Lvl_script, lua_pcall(Lvl_script, 0, 0, 0),"OnGameStart");
	}
}

void lua_on_game_tick()
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
		lua_pushinteger(Lvl_script, splevel + 1); // Lua is 1-based, so we add 1 to the level

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

void lua_on_trap_placed(struct Thing *traptng)
{
	SYNCDBG(6,"Starting");
    lua_getglobal(Lvl_script, "OnTrapPlaced");
	if (lua_isfunction(Lvl_script, -1))
	{
		lua_pushThing(Lvl_script, traptng);

		CheckLua(Lvl_script, lua_pcall(Lvl_script, 1, 0, 0),"OnTrapPlaced");
	}
}

void lua_on_creature_death(struct Thing *crtng)
{
	SYNCDBG(6,"Starting");
    lua_getglobal(Lvl_script, "OnUnitDeath");
	if (lua_isfunction(Lvl_script, -1))
	{
		lua_pushThing(Lvl_script, crtng);

		CheckLua(Lvl_script, lua_pcall(Lvl_script, 1, 0, 0),"OnUnitDeath");
	}
}


void lua_on_apply_damage_to_thing(struct Thing *thing, HitPoints dmg, PlayerNumber dealing_plyr_idx)
{
	SYNCDBG(6,"Starting");
    lua_getglobal(Lvl_script, "OnApplyDamage");
	if (lua_isfunction(Lvl_script, -1))
	{
		lua_pushThing(Lvl_script, thing);
		lua_pushinteger(Lvl_script, dmg);
		lua_pushPlayer(Lvl_script, dealing_plyr_idx);

		CheckLua(Lvl_script, lua_pcall(Lvl_script, 3, 0, 0),"OnApplyDamage");
	}
}