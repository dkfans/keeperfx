#include "pre_inc.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "bflib_basics.h"
#include "globals.h"
#include "creature_control.h"
#include "engine_camera.h"
#include "packets.h"
#include "player_data.h"
#include "player_instances.h"
#include "thing_creature.h"

#include "lua_base.h"
#include "lua_params.h"
#include "lua_utils.h"

#include "post_inc.h"
#include "game_merge.h"
#include "lvl_script_lib.h"

/**********************************************/
#define LUA_CAMERA_HORIZONTAL_FOV 94
#define LUA_CAMERA_VIEW_CREATURE 1
#define LUA_CAMERA_VIEW_PARCHMENT 3

struct LuaCameraView {
    struct Coord3d pos;
    int32_t yaw;
    int32_t pitch;
    int32_t roll;
    int32_t zoom;
    int32_t view_mode;
};

static void get_lua_camera_view(const struct PlayerInfo *player, const struct UserState *ustate, struct LuaCameraView *view)
{
    const struct DungeonCamera dcam = ustate->dungeon_camera;
    memset(view, 0, sizeof(*view));
    view->pos.x.val = dcam.x;
    view->pos.y.val = dcam.y;
    view->yaw = dcam.yaw[ustate->dungeon_camera.use_front_view];
    view->pitch = dcam.pitch;
    view->zoom = dcam.zoom[ustate->dungeon_camera.use_front_view];
    view->view_mode = get_dungeon_view_mode(ustate);
    switch (ustate->view_type)
    {
    case PVT_CreatureContrl:
    case PVT_CreaturePasngr:
    {
        view->zoom = 0;
        view->view_mode = LUA_CAMERA_VIEW_CREATURE;
        const struct Thing *thing = thing_get(player->controlled_thing_idx);
        if (!thing_exists(thing))
            break;
        view->pos = thing->mappos;
        view->yaw = thing->move_angle_xy;
        view->pitch = thing->move_angle_z;
        if (thing_is_creature(thing))
        {
            view->pos.z.val += get_creature_eye_height(thing);
            if ((thing->movement_flags & TMvF_Flying) != 0)
                view->roll = creature_control_get_from_thing(thing)->roll;
        }
        break;
    }
    case PVT_MapScreen:
    case PVT_MapFadeOut:
    {
        const struct Packet *pckt = get_packet(player->user_id);
        view->pos.x.val = pckt->pos_x;
        view->pos.y.val = pckt->pos_y;
        view->zoom = 0;
        view->view_mode = LUA_CAMERA_VIEW_PARCHMENT;
        break;
    }
    default:
        break;
    }
}

int luaL_checkCamera(lua_State *L, int idx)
{
    if (!lua_istable(L, idx)) {
        return luaL_argerror(L, idx, "Expected a table");
    }

    lua_getfield(L, idx, "playerId");
    if (!lua_isnumber(L, -1)) {
        return luaL_argerror(L, idx, "Table must have a numeric 'playerId' field");
    }
    PlayerNumber playerId = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return playerId;
}

static const struct luaL_Reg camera_methods[] = {
     {NULL, NULL}
 };


 static int camera_tostring(lua_State *L)
 {
    int playerId = luaL_checkCamera(L, 1);
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Camera(%d)", playerId);
    lua_pushstring(L, buffer);
    return 1;
 }

 // The camera is read-only for now; assignments are ignored rather than landing on the table and shadowing the real value.
 static int camera_set_field(lua_State *L) {
    luaL_checkCamera(L, 1);
    return 0;
 }

 // Function to get field values
 static int camera_get_field(lua_State *L) {

    int playerId = luaL_checkCamera(L, 1);

    const char* key = luaL_checkstring(L, 2);

    if (try_get_c_method(L, key, camera_methods))
    {
        return 1;
    }

    struct PlayerInfo *player = get_player(playerId);
    const struct UserState *ustate = get_player_user_state(player);
    if (user_state_invalid(ustate)) {
        lua_pushnil(L);
        return 1;
    }
    struct LuaCameraView view;
    get_lua_camera_view(player, ustate, &view);

    if (strcmp(key, "pos") == 0) {
        lua_pushPos(L, &view.pos);
    } else if (strcmp(key, "yaw") == 0) {
        lua_pushinteger(L, view.yaw);
    } else if (strcmp(key, "pitch") == 0) {
        lua_pushinteger(L, view.pitch);
    } else if (strcmp(key, "roll") == 0) {
        lua_pushinteger(L, view.roll);
    } else if (strcmp(key, "horizontal_fov") == 0) {
        lua_pushinteger(L, LUA_CAMERA_HORIZONTAL_FOV);
    } else if (strcmp(key, "zoom") == 0) {
        lua_pushinteger(L, view.zoom);
    } else if (strcmp(key, "view_mode") == 0) {
        lua_pushinteger(L, view.view_mode);
    } else if (try_get_from_methods(L, 1, key)) {
        return 1;
    } else {
        return luaL_error(L, "Unknown field or method '%s' for Camera", key);
    }

    return 1;

 }

static int camera_eq(lua_State *L) {
    int playerId1, playerId2;

    playerId1 = luaL_checkCamera(L, 1);
    playerId2 = luaL_checkCamera(L, 2);

    // Compare the coordinates
    lua_pushboolean(L, (playerId1 == playerId2));
    return 1;
}

 static const struct luaL_Reg camera_meta[] = {
     {"__tostring", camera_tostring},
     {"__index",    camera_get_field},
     {"__newindex", camera_set_field},
     {"__eq",       camera_eq},
     {NULL, NULL}
 };

 void Camera_register(lua_State *L) {
     // Create a metatable for thing and add it to the registry
     luaL_newmetatable(L, "Camera");

     // Set the __index and __newindex metamethods
     luaL_setfuncs(L, camera_meta, 0);

     // Hide the metatable by setting the __metatable field to nil
     lua_pushliteral(L, "__metatable");
     lua_pushnil(L);
     lua_rawset(L, -3);

     // Pop the metatable from the stack
     lua_pop(L, 1);

 }
