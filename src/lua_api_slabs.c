#include "pre_inc.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "bflib_basics.h"
#include "globals.h"
#include "map_blocks.h"
#include "map_data.h"
#include "thing_data.h"
#include "thing_creature.h"
#include "slab_data.h"

#include "lua_base.h"
#include "lua_params.h"
#include "lua_utils.h"

#include "post_inc.h"
#include "game_merge.h"
#include "lvl_script_lib.h"

/**********************************************/

//-----------------------------------------------
//  Slab stuff
//-----------------------------------------------


static int lua_GET_CREATURES(lua_State *L)
{
    MapSlabCoord slb_x = 0, slb_y = 0;
    luaL_checkSlab(L, 1, &slb_x, &slb_y);

    MapSubtlCoord stl_x = slab_subtile_center(slb_x);
    MapSubtlCoord stl_y = slab_subtile_center(slb_y);
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);

    if (mapblk == NULL) {
        luaL_error(L, "Invalid slab coordinates");
    }

    lua_newtable(L); // Create a new table to store creatures

    int k = 0;
    ThingIndex i = get_mapwho_thing_index(mapblk);
    while (i != 0) {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing)) {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;

        // Check if the thing is a creature
        if (thing_is_creature(thing)) {
            lua_pushThing(L, thing);  // Push the creature onto the stack
            lua_rawseti(L, -2, k + 1); // Add it to the table
            k++;
        }

        if (k > THINGS_COUNT) {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }

    return 1; // Return the table
}

static const struct luaL_Reg slab_methods[] = {
    {"GET_CREATURES"                          ,lua_GET_CREATURES                     },
     {NULL, NULL}
 };
 
 
 static int slab_tostring(lua_State *L)
 {
    MapSlabCoord slb_x, slb_y;
    luaL_checkSlab(L, 1, &slb_x, &slb_y);
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Slab(%d,%d)", (int)slb_x, (int)slb_y);
    lua_pushstring(L, buffer);
    return 1;
 }
 
 // Function to set field values
 static int slab_set_field(lua_State *L) {
 
    MapSlabCoord slb_x, slb_y;
    luaL_checkSlab(L, 1, &slb_x, &slb_y);
    const char* key = luaL_checkstring(L, 2);
 
    if (strcmp(key, "revealed") == 0) {
        struct Map* mapblk = get_map_block_at(slb_x * STL_PER_SLB, slb_y * STL_PER_SLB);
        mapblk->revealed = lua_toboolean(L, 3);
    } else if (strcmp(key, "owner") == 0) {
        set_slab_owner(slb_x, slb_y, luaL_checkPlayerSingle(L, 3));
    } else if (strcmp(key, "kind") == 0) {
        struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
        place_slab_type_on_map(luaL_checkNamedCommand(L, 3, slab_desc), slab_subtile_center(slb_x), slab_subtile_center(slb_y), slabmap_owner(slb), 0);
    } else if (strcmp(key, "style") == 0) {
        SlabCodedCoords slb_num = get_slab_number(slb_x, slb_y);
        game.slab_ext_data[slb_num] = luaL_checkNamedCommand(L, 3, texture_pack_desc);
    }
    return 0;
 }
 
 // Function to get field values
 static int slab_get_field(lua_State *L) {

    MapSlabCoord slb_x = 0, slb_y = 0;
    luaL_checkSlab(L, 1, &slb_x, &slb_y);

    const char* key = luaL_checkstring(L, 2);

    if (try_get_c_method(L, key, slab_methods))
    {
        return 1;
    }

    if (strcmp(key, "revealed") == 0) {
        const struct Map* mapblk = get_map_block_at(slb_x * STL_PER_SLB, slb_y * STL_PER_SLB);
       lua_pushboolean(L, mapblk->revealed);
    } else if (strcmp(key, "owner") == 0) {
        struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
        lua_pushPlayer(L, slabmap_owner(slb));
    } else if (strcmp(key, "kind") == 0) {
        struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
        lua_pushstring(L, get_conf_parameter_text(slab_desc,slb->kind));
    } else if (strcmp(key, "style") == 0) {
        SlabCodedCoords slb_num = get_slab_number(slb_x, slb_y);
        lua_pushstring(L, get_conf_parameter_text(texture_pack_desc,game.slab_ext_data[slb_num]));
    } else if (strcmp(key, "centerpos") == 0) {
        struct Coord3d centerpos;
        centerpos.x.val = subtile_coord_center(slab_subtile_center(slb_x));
        centerpos.y.val = subtile_coord_center(slab_subtile_center(slb_y));
        centerpos.z.val = get_floor_height_at(&centerpos);
        lua_pushPos(L, &centerpos);
        return 1;
    } else if (try_get_from_methods(L, 1, key)) {
        return 1;
    } else {
        return luaL_error(L, "Unknown field or method '%s' for Player", key);
    }

    return 1;
 
 }
 
static int slab_eq(lua_State *L) {
    MapSlabCoord slb_x1, slb_y1, slb_x2, slb_y2;

    luaL_checkSlab(L, 1, &slb_x1, &slb_y1);
    luaL_checkSlab(L, 2, &slb_x2, &slb_y2);

    // Compare the coordinates
    lua_pushboolean(L, (slb_x1 == slb_x2) && (slb_y1 == slb_y2));
    return 1;
}
 
 static const struct luaL_Reg slab_meta[] = {
     {"__tostring", slab_tostring},
     {"__index",    slab_get_field},
     {"__newindex", slab_set_field},
     {"__eq",       slab_eq},
     {NULL, NULL}
 };
 
 void Slab_register(lua_State *L) {
     // Create a metatable for thing and add it to the registry
     luaL_newmetatable(L, "Slab");
 
     // Set the __index and __newindex metamethods
     luaL_setfuncs(L, slab_meta, 0);
 
     // Create a methods table
     luaL_newlib(L, slab_methods);
 
     for (int i = 0; slab_methods[i].name != NULL; i++) {
         const char *name = slab_methods[i].name;
         lua_pushcfunction(L, slab_methods[i].func);
         lua_setfield(L, -2, name);
     }
 
     // Hide the metatable by setting the __metatable field to nil
     lua_pushliteral(L, "__metatable");
     lua_pushnil(L);
     lua_rawset(L, -3);
 
     // Pop the metatable from the stack
     lua_pop(L, 1);    
 
 }