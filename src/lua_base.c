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


/* @comment
 *     The loading items of open_lua_script and open_lua_script_for_mod need to be consistent.
 * @note
 *     We do not setLuaPath for mod, so can load only a single lua file without dependencies.
 */
static TbBool open_lua_script_for_mod(lua_State* L, LevelNumber lvnum, const struct ModConfigItem *mod_item)
{
    TbBool result = false;
    const struct ModExistState *mod_state = &mod_item->state;
    char* fname = NULL;
    char mod_dir[256] = {0};
    sprintf(mod_dir, "%s/%s", MODS_DIR_NAME, mod_item->name);

    // If the mod name contains characters that the identifier does not support, ignore it.
    if (strpbrk(mod_item->name, " -") != NULL)
        return false;

    if (mod_state->fx_data)
    {
        fname = prepare_file_fmtpath_mod(mod_dir, FGrp_FxData, "lua/init.lua");
        if (LbFileExists(fname))
        {
            if (CheckLua(L, luaL_dofile(L, fname), "mod_fxdata_lua_file"))
            {
                result = true;
            }
            else
            {
                ERRORLOG("failed to load mod fxdata lua script for %s", mod_item->name);
            }
        }
    }

    if (mod_state->cmpg_config)
    {
        fname = prepare_file_fmtpath_mod(mod_dir, FGrp_CmpgConfig, "lua/init.lua");
        if (LbFileExists(fname))
        {
            if (CheckLua(L, luaL_dofile(L, fname), "mod_campaign_lua_file"))
            {
                result = true;
            }
            else
            {
                ERRORLOG("failed to load mod campaign lua script for %s", mod_item->name);
            }
        }
    }

    if (mod_state->cmpg_lvls)
    {
        short fgroup = get_level_fgroup(lvnum);
        fname = prepare_file_fmtpath_mod(mod_dir, fgroup, "map%05lu.lua", (unsigned long)lvnum);
        if (LbFileExists(fname))
        {
            if(CheckLua(L, luaL_dofile(L, fname), "mod_level_script_loading"))
            {
                result = true;
            }
            else
            {
                ERRORLOG("failed to load mod level lua script for %s", mod_item->name);
            }
        }
    }

    return result;
}

static void open_lua_script_for_mod_list(lua_State* L, LevelNumber lvnum, const struct ModConfigItem *mod_items, long mod_cnt)
{
    for (long i=0; i<mod_cnt; i++)
    {
        const struct ModConfigItem *mod_item = mod_items + i;
        if (open_lua_script_for_mod(L, lvnum, mod_item))
        {
            // If lua_on_start_for_mod_list has performance issues, a variable can be used to record the state here.
        }
    }
}

static void open_lua_script_for_mod_all(lua_State* L, LevelNumber lvnum)
{
    if (mods_conf.after_base_cnt > 0)
    {
        open_lua_script_for_mod_list(L, lvnum, mods_conf.after_base_item, mods_conf.after_base_cnt);
    }

    if (mods_conf.after_campaign_cnt > 0)
    {
        open_lua_script_for_mod_list(L, lvnum, mods_conf.after_campaign_item, mods_conf.after_campaign_cnt);
    }

    if (mods_conf.after_map_cnt > 0)
    {
        open_lua_script_for_mod_list(L, lvnum, mods_conf.after_map_item, mods_conf.after_map_cnt);
    }
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

    char* fname = prepare_file_fmtpath(FGrp_FxData, "lua/init.lua");

    // Load and parse the Lua File
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

    fname = prepare_file_fmtpath(FGrp_CmpgConfig, "lua/init.lua");
    if (LbFileExists(fname))
    {
        if (!CheckLua(Lvl_script, luaL_dofile(Lvl_script, fname), "campaign_lua_file"))
        {
            ERRORLOG("failed to load campaign lua script");
        }
    }

    short fgroup = get_level_fgroup(lvnum);
    fname = prepare_file_fmtpath(fgroup, "map%05lu.lua", (unsigned long)lvnum);
    // Load and parse the Lua File
    if (LbFileExists(fname) )
    {
        if(!CheckLua(Lvl_script, luaL_dofile(Lvl_script, fname), "level_script_loading"))
        {
            ERRORLOG("failed to load level lua script");
        }
    }

    open_lua_script_for_mod_all(Lvl_script, lvnum);

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
