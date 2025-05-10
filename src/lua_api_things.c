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
#include "lua_utils.h"


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

    if (thing->class_id == TCls_Creature)
    {
        kill_creature(thing, INVALID_THING, -1, CrDed_NoEffects | CrDed_NotReallyDying);
    }
    else
    {
        delete_thing_structure(thing,0);
    }
    return 0;
}

static int lua_is_valid(lua_State *L)
{
    lua_pushboolean(L, luaL_isThing(L,1));
    return 1;
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
        if (is_thing_some_way_controlled(thing)) {
            prepare_to_controlled_creature_death(thing);
        }
        change_creature_owner(thing, new_owner);

    } else if (strcmp(key, "name") == 0) {
        if (thing->class_id != TCls_Creature) {
            return luaL_error(L, "Attempt to set name of non-creature thing");
        }

        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl)) {
            return luaL_error(L, "Invalid creature control block");
        }

        const char* name = luaL_checkstring(L, 3);
        if (strlen(name) > CREATURE_NAME_MAX) {
            return luaL_error(L, "Creature name too long (max %d)", CREATURE_NAME_MAX);
        }

        strncpy(cctrl->creature_name, name, CREATURE_NAME_MAX);

    } else if (strcmp(key, "health") == 0) {
        thing->health = luaL_checkinteger(L, 3);

    } else if (strcmp(key, "shots") == 0) {
        if (thing->class_id != TCls_Trap) {
            return luaL_error(L, "Attempt to set shots of non-trap thing");
        }
        set_trap_shots(thing, luaL_checkinteger(L, 3));

    } else {
        return luaL_error(L, "Field '%s' is not writable on Thing", key);
    }

    return 1;
}

static int thing_get_field(lua_State *L) {
    const char* key = luaL_checkstring(L, 2);

    if (try_get_c_method(L, key, thing_methods))
    {
        return 1;
    }

    struct Thing* thing = luaL_checkThing(L, 1);

    // Built-in fields
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
    } else if (strcmp(key, "health") == 0) {
        lua_pushinteger(L, thing->health);
    } else if (strcmp(key, "max_health") == 0) {
        lua_pushinteger(L, get_thing_max_health(thing));
    } else if (strcmp(key, "shots") == 0) {
        if (thing->class_id != TCls_Trap)
            return luaL_error(L, "Attempt to access 'shots' of non-trap thing");
        lua_pushinteger(L, thing->trap.num_shots);
    } else if (strcmp(key, "level") == 0) {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl))
            return luaL_error(L, "Attempt to access 'level' of non-creature thing");
        lua_pushinteger(L, cctrl->exp_level);
    } else if (strcmp(key, "name") == 0) {
        if (thing->class_id != TCls_Creature)
            return luaL_error(L, "Attempt to get 'name' of non-creature thing");
        lua_pushstring(L, creature_own_name(thing));
    } else if (strcmp(key, "party") == 0) {
        if (thing->class_id != TCls_Creature)
            return luaL_error(L, "Attempt to get 'party' of non-creature thing");
        lua_pushPartyTable(L, get_group_leader(thing));
    } else if (strcmp(key, "picked_up") == 0) {
        lua_pushboolean(L, thing_is_picked_up(thing));
    } else if (try_get_from_methods(L, 1, key)) {
        return 1;
    } else {
        return luaL_error(L, "Unknown field or method '%s' for Thing", key);
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
    {"make_thing_zombie", make_thing_zombie},
    {"creature_walk_to",  lua_creature_walk_to},
    {"kill",    lua_kill_creature},
    {"delete",     lua_delete_thing},
    {"isValid",         lua_is_valid},
    
   {"transfer"                    ,lua_Transfer_creature               },
   {"level_up"                    ,lua_Level_up_creature               },
   {"teleport"                    ,lua_Teleport_creature               },
   {"change_owner"                ,lua_Change_creature_owner           },
    {NULL, NULL}
};

static const struct luaL_Reg thing_meta[] = {
    {"__tostring", thing_tostring},
    {"__index",    thing_get_field},
    {"__newindex", thing_set_field},
    {"__eq",       thing_eq},
    {NULL, NULL}
};

void Thing_register(lua_State *L) {
    // Create and register the metatable as "Thing"
    luaL_newmetatable(L, "Thing");

    // Set metamethods (__index, __eq, etc.) from thing_meta[]
    luaL_setfuncs(L, thing_meta, 0);

    // Create the method table for Lua-accessible methods
    lua_newtable(L);
    luaL_setfuncs(L, thing_methods, 0); // your C methods

    // Save method table into metatable under __methods
    lua_setfield(L, -2, "__methods");

    // Hide metatable from Lua code
    lua_pushliteral(L, "__metatable");
    lua_pushnil(L);
    lua_rawset(L, -3);

    // Save methods table globally as "Thing" for Lua access
    lua_getfield(L, -1, "__methods");
    lua_setglobal(L, "Thing");

    // Alias methods to Creature (Lua-side)
    lua_getglobal(L, "Thing");
    lua_setglobal(L, "Creature");

    // Pop the metatable
    lua_pop(L, 1);
}