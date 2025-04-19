#include "pre_inc.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lua_api.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "config_keeperfx.h"
#include "globals.h"
#include "gui_msgs.h"


#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct lua_State *Lvl_script = NULL;


// Little error checking utility function
TbBool CheckLua(lua_State *L, int result,const char* func)
{
    if (result != LUA_OK) {
        const char *message = lua_tostring(L, -1);
        ERRORLOG("Lua error in %s: %s", func, message ? message : "Unknown error");

        if (message) {
            luaL_traceback(L, L, NULL, 1);
            const char *trace = lua_tostring(L, -1);
            ERRORLOG("Stack trace:\n%s", trace ? trace : "No stack trace available");
            lua_pop(L, 2); // Pop both message and traceback
        } else {
            lua_pop(L, 1); // Pop the non-existent error message
        }

        if (exit_on_lua_error)
        {
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
    prepare_file_path_buf(fx_data_path, FGrp_FxData, "lua/?.lua");
    char campaignpath[PATH_LENGTH];
    prepare_file_path_buf(campaignpath, FGrp_CmpgConfig, "lua/?.lua");
    char levelpath[PATH_LENGTH];
    prepare_file_path_buf(levelpath, FGrp_CmpgLvls, "?.lua");

    
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

TbBool open_lua_script(LevelNumber lvnum)
{
	Lvl_script = luaL_newstate();

	luaL_openlibs(Lvl_script);

	reg_host_functions(Lvl_script);
	 
    setLuaPath(Lvl_script);
    
    short fgroup = get_level_fgroup(lvnum);
    char* fname = prepare_file_fmtpath(fgroup, "map%05lu.lua", (unsigned long)lvnum);

	// Load and parse the Lua File
    if ( !LbFileExists(fname) )
      return false;

    
	if(!CheckLua(Lvl_script, luaL_dofile(Lvl_script, fname),"script_loading"))
	{
        ERRORLOG("failed to load lua script");
        close_lua_script();
        return false;
	}



    fname = prepare_file_fmtpath(FGrp_FxData, "lua/global.lua");

	// Load and parse the Lua File
    if ( !LbFileExists(fname) )
    {
        ERRORLOG("file %s missing",fname);
        return false;
    }
    
	if(!CheckLua(Lvl_script, luaL_dofile(Lvl_script, fname),"global_lua_file"))
	{
        ERRORLOG("failed to load global lua script");
        close_lua_script();
        return false;
	}
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


#ifdef __cplusplus
}
#endif
