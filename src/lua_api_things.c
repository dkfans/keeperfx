#include "pre_inc.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "bflib_basics.h"
#include "bflib_sndlib.h"
#include "globals.h"
#include "thing_data.h"
#include "creature_states.h"
#include "creature_states_pray.h"
#include "gui_msgs.h"
#include "thing_navigate.h"
#include "map_data.h"
#include "game_legacy.h"
#include "player_utils.h"
#include "lvl_script_lib.h"
#include "room_library.h"
#include "room_util.h"
#include "keeperfx.hpp"
#include "power_specials.h"
#include "thing_creature.h"
#include "thing_effects.h"
#include "magic_powers.h"

#include "lua_base.h"
#include "lua_params.h"


#include "post_inc.h"

/**********************************************/
static int thing_set_field(lua_State *L);
static int thing_get_field(lua_State *L);

static const struct luaL_Reg thing_methods[];




/**********************************************/
// things
/**********************************************/

static int make_thing_zombie (lua_State *L)
{
    struct Thing *thing = luaL_checkThing(L, 1);

    //internal_set_thing_state(thing, CrSt_Disabled);
    //thing->active_state = CrSt_Disabled;
    //thing->continue_state = CrSt_Disabled;

    thing->alloc_flags |= TAlF_IsControlled;


    return 0;
}

static int lua_delete_thing(lua_State *L)
{
    struct Thing *thing = luaL_checkThing(L, 1);
    delete_thing_structure(thing,0);
    return 0;
}

static int lua_is_valid(lua_State *L)
{
    return luaL_isThing(L,1);
}


static int lua_creature_walk_to(lua_State *L)
{
    struct Thing *thing = luaL_checkThing(L, 1);
    int stl_x = luaL_checkstl_x(L, 2);
    int stl_y = luaL_checkstl_y(L, 3);

    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
        WARNLOG("Move %s order failed", thing_model_name(thing));
    thing->continue_state = CrSt_ManualControl;

    return 0;
}

static int lua_kill_creature(lua_State *L)
{
    struct Thing* thing = luaL_checkThing(L, 1);
    kill_creature(thing, INVALID_THING, -1, CrDed_NoUnconscious);

    return 0;
}

static int lua_Transfer_creature(lua_State *L)
{
    struct Thing* thing = luaL_checkCreature(L, 1);

    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);

    if (add_transfered_creature(thing->owner, thing->model, cctrl->exp_level, cctrl->creature_name))
    {
        struct Dungeon* dungeon = get_dungeon(thing->owner);
        dungeon->creatures_transferred++;
        remove_thing_from_power_hand_list(thing, thing->owner);
        struct SpecialConfigStats* specst = get_special_model_stats(SpcKind_TrnsfrCrtr);
        create_used_effect_or_element(&thing->mappos, specst->effect_id, thing->owner, thing->index);
        kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects | CrDed_NotReallyDying);
    }
    return 0;
}

static int lua_Level_up_creature(lua_State *L)
{
    struct Thing* thing = luaL_checkCreature(L, 1);
    int count = luaL_checkinteger(L, 2);

    creature_change_multiple_levels(thing,count);
    return 0;
}

static int lua_Teleport_creature(lua_State *L)
{
    struct Thing* thing = luaL_checkThing(L, 1);
    TbMapLocation location = luaL_checkLocation(L, 2);
    EffectOrEffElModel effect_id = luaL_checkEffectOrEffElModel(L, 3);

    script_move_creature(thing, location, effect_id);
    return 0;
}

static int lua_Change_creature_owner(lua_State *L)
{
    struct Thing* thing = luaL_checkThing(L, 1);
    PlayerNumber new_owner = luaL_checkPlayerSingle(L, 2);
    if (is_thing_some_way_controlled(thing))
    {
        //does not kill the creature, but does the preparations needed for when it is possessed
        prepare_to_controlled_creature_death(thing);
    }
    change_creature_owner(thing, new_owner);
    return 0;
}



static int thing_tostring(lua_State *L)
{
    char buff[64];
    struct Thing* thing = luaL_checkThing(L, 1);
    snprintf(buff, sizeof(buff), "id: %d turn: %ld %s", thing->index, thing->creation_turn, thing_class_and_model_name(thing->class_id,thing->model));

    lua_pushfstring(L, "Thing (%s)", buff);
    return 1;
}

// Function to set field values
static int thing_set_field(lua_State *L) {

    struct Thing* thing = luaL_checkThing(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "orientation") == 0) {
        thing->move_angle_xy = luaL_checkinteger(L, 3);
    } else if (strcmp(key, "owner") == 0) {
        PlayerNumber new_owner = luaL_checkPlayerSingle(L, 3);
        if (is_thing_some_way_controlled(thing))
        {
            //does not kill the creature, but does the preparations needed for when it is possessed
            prepare_to_controlled_creature_death(thing);
        }
        change_creature_owner(thing, new_owner);
    } else {
        luaL_error(L, "not a settable field: %s", key);
    }

    return 1;
}

// Function to get field values
static int thing_get_field(lua_State *L) {
    const char* key = luaL_checkstring(L, 2);

    // Check if the key exists in thing_methods (C functions)
    for (int i = 0; thing_methods[i].name != NULL; i++) {
        if (strcmp(key, thing_methods[i].name) == 0) {
            lua_pushcfunction(L, thing_methods[i].func);
            return 1;
        }
    }

    // Get the Thing object
    struct Thing* thing = luaL_checkThing(L, 1);

    // Check known fields (direct fields of the Thing struct)
    if (strcmp(key, "ThingIndex") == 0) {
        lua_pushinteger(L, thing->index);
    } else if (strcmp(key, "creation_turn") == 0) {
        lua_pushinteger(L, thing->creation_turn);
    } else if (strcmp(key, "model") == 0) {
        lua_pushstring(L, thing_model_only_name(thing->class_id, thing->model));
    } else if (strcmp(key, "owner") == 0) {
        lua_pushPlayer(L, thing->owner);
    } else if (strcmp(key, "pos") == 0) {
        lua_pushPos(L, &thing->mappos);
    } else if (strcmp(key, "orientation") == 0) {
        lua_pushinteger(L, thing->move_angle_xy);

    } else if (strcmp(key, "level") == 0) {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl)) {
            luaL_error(L, "Attempt to access level of non-creature thing");
            return 0;
        }
        lua_pushinteger(L, cctrl->exp_level);
    } else {
        // Check if the key exists in the metatable's __methods table (Lua functions)
        lua_getmetatable(L, 1);             // Get metatable
        lua_getfield(L, -1, "__methods");   // Get the __methods table
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, key);  // Try to get the key from __methods
            if (!lua_isnil(L, -1)) {
                lua_remove(L, -2);  // Remove __methods table, leaving the function
                return 1;
            }
        }
        lua_pop(L, 2); // Pop metatable and __methods table (or nil)

        // If not found in __methods, check the table directly (standard field lookup)
        lua_getfield(L, 1, key);
        if (lua_isnil(L, -1)) {
            // If key is not found, check if __index is defined in the metatable
            lua_getmetatable(L, 1);  // Push metatable again
            lua_getfield(L, -1, "__index");  // Check for __index
            if (lua_istable(L, -1)) {
                // If __index is a table, attempt to access the key in __index
                lua_getfield(L, -1, key);
                if (!lua_isnil(L, -1)) {
                    lua_remove(L, -2);  // Remove __index
                    return 1;
                }
            }
            lua_pop(L, 2);  // Pop metatable and __index (or nil)
        }
    }

    return 1;
}


static int thing_eq(lua_State *L) {



    if (!lua_istable(L, 1) || !lua_istable(L, 2)) {
        luaL_error(L, "Expected a table");
        return 1;
    }

    // Get idx field
    lua_getfield(L, 1, "ThingIndex");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'index' to be an integer");
        return 1;
    }
    int idx1 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack

    // Get idx field
    lua_getfield(L, 2, "ThingIndex");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'index' to be an integer");
        return 1;
    }
    int idx2 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack

    if(idx1 != idx2)
    {
        lua_pushboolean(L, false);
        return 1;
    }


    // Get creation_turn field
    lua_getfield(L, 1, "creation_turn");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'creation_turn' to be an integer");
        return 1;
    }
    int creation_turn1 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the creation_turn value off the stack

    lua_getfield(L,2, "creation_turn");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'creation_turn' to be an integer");
        return 1;
    }
    int creation_turn2 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the creation_turn value off the stack

    lua_pushboolean(L, creation_turn1 == creation_turn2);
    return 1;
}


static const struct luaL_Reg thing_methods[] = {
    {"Make_thing_zombie", make_thing_zombie},
    {"Creature_walk_to",  lua_creature_walk_to},
    {"Kill_creature",    lua_kill_creature},
    {"Delete_thing",     lua_delete_thing},
    {"isValid",         lua_is_valid},
    
   {"Transfer_creature"                    ,lua_Transfer_creature               },
   {"Level_up_creature"                    ,lua_Level_up_creature               },
   {"Teleport_creature"                    ,lua_Teleport_creature               },
   {"Change_creature_owner"                ,lua_Change_creature_owner           },
    {NULL, NULL}
};

static const struct luaL_Reg thing_meta[] = {
    {"__tostring", thing_tostring},
    {"__index",    thing_get_field},
    {"__newindex", thing_set_field},
    {"__eq",       thing_eq},
    {NULL, NULL}
};


int Thing_register(lua_State *L)
{
    // Create a metatable for thing and add it to the registry
    luaL_newmetatable(L, "Thing");

    // Set the __index and __newindex metamethods
    luaL_setfuncs(L, thing_meta, 0);

    // Create a methods table
    luaL_newlib(L, thing_methods);

    for (int i = 0; thing_methods[i].name != NULL; i++) {
        const char *name = thing_methods[i].name;
        lua_pushcfunction(L, thing_methods[i].func);
        lua_setfield(L, -2, name);
    }


    // Hide the metatable by setting the __metatable field to nil
    lua_pushliteral(L, "__metatable");
    lua_pushnil(L);
    lua_rawset(L, -3);

    // Pop the metatable from the stack
    lua_pop(L, 1);

    return 1; // Return the methods table
}
