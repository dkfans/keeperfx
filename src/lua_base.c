#include "pre_inc.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lua_api.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "config_keeperfx.h"
#include "game_legacy.h"
#include "globals.h"
#include "gui_msgs.h"

#include "config_creature.h"
#include "config_terrain.h"
#include "config_magic.h"
#include "config_effects.h"
#include "config_objects.h"
#include "config_trapdoor.h"
#include "config_crtrstates.h"
#include "kfx/modding/mod_api.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct lua_State *Lvl_script = NULL;


TbBool CheckLua(lua_State *L, int result, const char* func)
{
    if (result != LUA_OK) {
        // Coerce error to string using tostring()
        if (!lua_isstring(L, -1)) {
            lua_getglobal(L, "tostring"); // push tostring
            lua_pushvalue(L, -2);         // push error object
            lua_call(L, 1, 1);            // call tostring(err)
            lua_remove(L, -2);            // remove original error
        }

        const char *message = lua_tostring(L, -1);
        ERRORLOG("Lua error in %s: %s", func, message ? message : "Unknown error");
        lua_pop(L, 1); // pop error string

        if (exit_on_lua_error) {
            ERRORLOG("Exiting due to Lua error");
            exit(EXIT_FAILURE);
        }
        return false;
    }
    return true;
}


void close_lua_script()
{
    if(Lvl_script)
        lua_close(Lvl_script);
    Lvl_script = NULL;
}


int setLuaPath( lua_State* L)
{
    #define PATH_LENGTH 4096
    char fx_data_path[PATH_LENGTH];
    prepare_file_path_buf(fx_data_path, sizeof(fx_data_path), FGrp_FxData, "lua/?.lua");
    char campaignpath[PATH_LENGTH];
    prepare_file_path_buf(campaignpath, sizeof(campaignpath), FGrp_CmpgConfig, "lua/?.lua");
    char levelpath[PATH_LENGTH];
    prepare_file_path_buf(levelpath, sizeof(levelpath), FGrp_CmpgLvls, "?.lua");


    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path"); // get field "path" from table at top of stack (-1)
    const char *cur_path = lua_tostring(L, -1); // grab path string from top of stack
    char new_path[PATH_LENGTH];
    snprintf(new_path, sizeof(new_path), "%s;%s;%s;%s", levelpath,campaignpath,fx_data_path,cur_path);
    lua_pop(L, 1); // get rid of the string on the stack we just pushed on line 5
    lua_pushstring(L, new_path); // push the new one
    lua_setfield(L, -2, "path"); // set the field "path" in table at -2 with value at top of stack
    lua_pop(L, 1); // get rid of package table from top of stack
    return 0; // all done!
}


// Function to execute a piece of Lua code, and on failure, print the error message to the console
TbBool execute_lua_code_from_console(const char* code)
{
    if (Lvl_script == NULL) {
        ERRORLOG("Lua state is not initialized");
        return false;
    }

    if (code == NULL) {
        message_add(MsgType_Blank, 0, "no Lua code provided");
        return false;
    }

    int result = luaL_dostring(Lvl_script, code);
    if (result != LUA_OK) {
        const char *message = lua_tostring(Lvl_script, -1);
        ERRORLOG("Failed to execute Lua code: %s", message ? message : "Unknown error");

        // Pass the error message to message_add
        message_add(MsgType_Blank, 0, "lua error, see log");

        lua_pop(Lvl_script, 2); // Remove error message and traceback
        return false;
    }

    return true;
}

TbBool execute_lua_code_from_script(const char* code)
{
    if (Lvl_script == NULL) {
        ERRORLOG("Lua state is not initialized");
        return false;
    }

    if (code == NULL) {
        return false;
    }

    int result = luaL_dostring(Lvl_script, code);
    if (result != LUA_OK) {
        const char *message = lua_tostring(Lvl_script, -1);
        ERRORLOG("Failed to execute Lua code: %s", message ? message : "Unknown error");

        lua_pop(Lvl_script, 2); // Remove error message and traceback
        return false;
    }

    return true;
}
void lua_set_random_seed(unsigned int seed)
{
    if (Lvl_script == NULL) {
        ERRORLOG("Lua state is not initialized");
        return;
    }

    lua_getglobal(Lvl_script, "math");
    if (lua_istable(Lvl_script, -1)) {
        lua_getfield(Lvl_script, -1, "randomseed");
        if (lua_isfunction(Lvl_script, -1)) {
            lua_pushinteger(Lvl_script, seed);
            if (!CheckLua(Lvl_script, lua_pcall(Lvl_script, 1, 0, 0), "math.randomseed")) {
                ERRORLOG("Failed to call math.randomseed");
            }
        } else {
            ERRORLOG("failed to find math.randomseed function");
            lua_pop(Lvl_script, 1); // Pop nil
        }
    } else {
        ERRORLOG("failed to find math table");
    }
    lua_pop(Lvl_script, 1); // Pop math table or nil
}


/* -----------------------------------------------------------------------
 * Unified Lua script loading via kfx/modding walker
 * --------------------------------------------------------------------- */

/**
 * Locations for `lua/init.lua` — after_base mods, campaign, and after_campaign mods.
 * (Base FxData tier is handled separately since it is required.)
 */
static const ModLocation s_lua_init_locs[] = {
    { ModTier_AfterBase,     (short)FGrp_FxData,     ModLifetime_Startup,  offsetof(struct ModExistState, fx_data),     ModRes_File, NULL },
    { ModTier_Campaign,      (short)FGrp_CmpgConfig, ModLifetime_Campaign, SIZE_MAX,                                    ModRes_File, NULL },
    { ModTier_AfterCampaign, (short)FGrp_CmpgConfig, ModLifetime_Campaign, offsetof(struct ModExistState, cmpg_config), ModRes_File, NULL },
};

/**
 * Locations for per-map level scripts (`map%05lu.lua`) — per-map base and after_map mods.
 */
static const ModLocation s_lua_level_locs[] = {
    { ModTier_PerMap,  (short)FGrp_CmpgLvls, ModLifetime_Level, SIZE_MAX,                                  ModRes_File, NULL },
    { ModTier_AfterMap,(short)FGrp_CmpgLvls, ModLifetime_Level, offsetof(struct ModExistState, cmpg_lvls), ModRes_File, NULL },
};

static KfxModHandle s_lua_init_walker  = NULL;
static KfxModHandle s_lua_level_walker = NULL;

static void on_lua_file_found(const char *path, void *userdata)
{
    lua_State *L = (lua_State *)userdata;
    if (!CheckLua(L, luaL_dofile(L, path), "lua_file"))
        ERRORLOG("failed to load lua script: %s", path);
}


/* @comment
 *     The loading items of open_lua_script and open_lua_script_for_mod need to be consistent.
 */
TbBool open_lua_script(LevelNumber lvnum)
{
    Lvl_script = luaL_newstate();

    luaL_openlibs(Lvl_script);
    lua_set_random_seed(game.action_random_seed);

    reg_host_functions(Lvl_script);

    setLuaPath(Lvl_script);

    /* Lazy init walkers */
    if (s_lua_init_walker == NULL)
        s_lua_init_walker = kfx_mod_create_walker(s_lua_init_locs,
            sizeof(s_lua_init_locs) / sizeof(s_lua_init_locs[0]));
    if (s_lua_level_walker == NULL)
        s_lua_level_walker = kfx_mod_create_walker(s_lua_level_locs,
            sizeof(s_lua_level_locs) / sizeof(s_lua_level_locs[0]));

    /* Base FxData lua/init.lua is REQUIRED — check before walker to give a clear error */
    char* fname = prepare_file_fmtpath(FGrp_FxData, "lua/init.lua");
    if ( !LbFileExists(fname) )
    {
        ERRORLOG("file %s missing",fname);
        return false;
    }
    if(!CheckLua(Lvl_script, luaL_dofile(Lvl_script, fname), "global_lua_file"))
    {
        ERRORLOG("failed to load global lua script");
        close_lua_script();
        return false;
    }

    /* after_base mods lua/init.lua, campaign lua/init.lua, after_campaign mods lua/init.lua */
    kfx_mod_visit(s_lua_init_walker, "lua/init.lua", on_lua_file_found, Lvl_script);

    /* Per-map script: map%05lu.lua (campaign base + per-map mods) */
    char map_lua[40];
    snprintf(map_lua, sizeof(map_lua), "map%05lu.lua", (unsigned long)lvnum);
    kfx_mod_visit(s_lua_level_walker, map_lua, on_lua_file_found, Lvl_script);

    return true;
}


static char* lua_serialized_data = NULL;

const char* lua_get_serialised_data(size_t *len)
{
    lua_getglobal(Lvl_script, "GetSerializedData");
	if (lua_isfunction(Lvl_script, -1))
	{
        JUSTLOG("calling GetSerializedData");
        int result = lua_pcall(Lvl_script, 0, 1, 0);
        JUSTLOG("lua_pcall result: %d", result);
        if (!CheckLua(Lvl_script, result, "GetSerializedData")) {
            ERRORLOG("Failed to call GetSerializedData");
            return NULL;
        }

        JUSTLOG("called GetSerializedData");
        if (!lua_isstring(Lvl_script, -1)) {
            ERRORLOG("Expected 'GetSerializedData' to return a string");
            lua_pop(Lvl_script, 1);
            return NULL;
        }
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

void generate_lua_types_file()
{
    const char *filename = "native_types.lua";
    FILE *out = fopen(filename, "w");
    if (!out) {
        perror("Failed to open output file");
        return;
    }
    fprintf(out, "---@meta native_types\n");
    fprintf(out, "-- file not used by the game, but used for telling the IDE about what exists\n");
    fprintf(out, "-- this file contains fields changeble through cfg files, the one here in fxdata is the default\n");
    fprintf(out, "-- custom maps/campaigns can generate their own version, so all custom types are still recognized by the IDE\n\n");
    fprintf(out, "-- to generate one with the values specific to your map use the !luatypedump on the specific map you'd like the file for\n\n");

    #define GENERATE_ALIAS(alias_name, desc)            \
    do {                                                \
        int count = 0;                                  \
        for (int i = 0; desc[i].name != NULL; ++i) {    \
            if (desc[i].name[0] != '\0') ++count;       \
        }                                               \
        fprintf(out, "---@alias %s ", alias_name);      \
        if (count >= 90) {                              \
            fprintf(out, "string|");                    \
        }                                               \
        int written = 0;                                \
        for (int i = 0; desc[i].name != NULL; ++i) {    \
            if (desc[i].name[0] == '\0') continue;      \
            if (written > 0) fprintf(out, "|");         \
            fprintf(out, "\"%s\"", desc[i].name);       \
            ++written;                                  \
        }                                               \
        fprintf(out, "\n");                             \
    } while (0)

    #define GENERATE_FIELDS(class_name, desc)               \
    do {                                                 \
        fprintf(out, "---@class %s\n", class_name);       \
        for (int i = 0; desc[i].name != NULL; ++i) {      \
            if (desc[i].name[0] == '\0') continue; \
            fprintf(out, "---@field %s integer\n", desc[i].name); \
        }                                                \
        fprintf(out, "\n");                               \
        } while (0)

    // Generate sections
    GENERATE_ALIAS("creature_type", creature_desc);
    GENERATE_ALIAS("room_type", room_desc);
    GENERATE_ALIAS("power_kind", power_desc);
    GENERATE_ALIAS("trap_type", trap_desc);
    GENERATE_ALIAS("door_type", door_desc);
    GENERATE_ALIAS("object_type", object_desc);
    GENERATE_ALIAS("effect_generator_type", effectgen_desc);
    GENERATE_ALIAS("effect_element_type", effectelem_desc);
    GENERATE_ALIAS("effect_type", effect_desc);
    GENERATE_ALIAS("spell_type", spell_desc);
    GENERATE_ALIAS("slab_type", slab_desc);
    GENERATE_ALIAS("creature_state", creatrstate_desc);
    GENERATE_ALIAS("shot_type", shot_desc);
    fprintf(out, "\n");

    GENERATE_FIELDS("roomfields", room_desc);
    GENERATE_FIELDS("creaturefields", creature_desc);
    GENERATE_FIELDS("trapfields", trap_desc);
    GENERATE_FIELDS("doorfields", door_desc);

    // Cleanup
    #undef GENERATE_ALIAS
    #undef GENERATE_FIELDS

    fclose(out);
}


#ifdef __cplusplus
}
#endif
