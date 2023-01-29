#include "pre_inc.h"

#include "../lib/lua/include/lua.h"
#include "../lib/lua/include/lauxlib.h"
#include "../lib/lua/include/lualib.h"

#include "bflib_basics.h"
#include "globals.h"
#include "thing_data.h"
#include "creature_states.h"
#include "gui_msgs.h"
#include "thing_navigate.h"
#include "map_data.h"
#include "game_legacy.h"
#include "player_utils.h"

#include "post_inc.h"

/**********************************************/

struct luaThing
{
    ThingIndex thing_idx;
    GameTurn creation_turn;
};

static struct Thing *lua_toThing(lua_State *L, int index);
static struct luaThing *lua_pushThing(lua_State *L, struct Thing* thing);

/**********************************************/
// game
/**********************************************/

static int lua_get_creature_near(lua_State *L)
{
    // the arguments lua passes to the C code
    int stl_x = lua_tointeger(L, 1); // the last number is the position of the argument, just increment these
    int stl_y = lua_tointeger(L, 2);

    struct Thing *thing = get_creature_near(stl_x * COORD_PER_STL, stl_y * COORD_PER_STL);

    // arguments you push back to lua
    lua_pushThing(L, thing);
    //lua_pushinteger(L, thing->index);
    return 1; // return value is the amount of args you push back
}

static int lua_get_thing_by_idx(lua_State *L)
{
    // the arguments lua passes to the C code
    ThingIndex tng_idx = lua_tointeger(L, 1);

    JUSTLOG("idx: %d",tng_idx);

    struct Thing *thing = thing_get(tng_idx);

    // arguments you push back to lua
    lua_pushThing(L, thing);
    return 1; // return value is the amount of args you push back
}

static int send_chat_message(lua_State *L)
{
    int plyr_idx = lua_tointeger(L, 1);
    const char *msg = lua_tostring(L, 2);
    message_add(plyr_idx, msg);

    return 0;
}

static int lua_set_player_as_won_level(lua_State *L)
{
    int plyr_idx = lua_tointeger(L, 1);
    struct PlayerInfo *player = get_player(plyr_idx);
    set_player_as_won_level(player);

    return 0;
}

static int lua_set_player_as_lost_level(lua_State *L)
{
    int plyr_idx = lua_tointeger(L, 1);
    struct PlayerInfo *player = get_player(plyr_idx);
    set_player_as_lost_level(player);

    return 0;
}

static const luaL_reg game_methods[] = {
    {"GetCreatureNear", lua_get_creature_near},
    {"SendChatMessage", send_chat_message},
    {"PlayerWin", lua_set_player_as_won_level},
    {"PlayerLose", lua_set_player_as_lost_level},
    {"getThingByIdx", lua_get_thing_by_idx},
    {0, 0}};

static const luaL_reg game_meta[] = {
    {0, 0}};

static int Game_register(lua_State *L)
{
    luaL_openlib(L, "game", game_methods, 0); /* create methods table,
                                             add it to the globals */
    luaL_newmetatable(L, "game");             /* create metatable for Foo,
                                              and add it to the Lua registry */
    luaL_openlib(L, 0, game_meta, 0);         /* fill metatable */
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -3); /* dup methods table*/
    lua_rawset(L, -3);    /* metatable.__index = methods */
    lua_pushliteral(L, "__metatable");
    lua_pushvalue(L, -3); /* dup methods table*/
    lua_rawset(L, -3);    /* hide metatable:
                             metatable.__metatable = methods */
    lua_pop(L, 1);        /* drop metatable */
    return 1;             /* return methods on the stack */
}

/**********************************************/
// things
/**********************************************/

static int make_thing_zombie(lua_State *L)
{
    JUSTLOG("make_thing_zombie");
    struct Thing *thing = lua_toThing(L, 1);

    //struct Thing *thing = thing_get(lua_tointeger(L, 1));
    if(thing_is_invalid(thing))
    {
        SCRPTERRLOG("invalid thing passed to make_thing_zombie");
        return 0;
    }
    thing->alloc_flags |= TAlF_IsControlled;

    return 0;
}

static int move_thing_to(lua_State *L)
{
    int tng_idx = lua_tointeger(L, 1);
    int stl_x = lua_tointeger(L, 2);
    int stl_y = lua_tointeger(L, 3);

    struct Thing *thing = thing_get(tng_idx);
    if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
        WARNLOG("Move %s order failed", thing_model_name(thing));
    thing->continue_state = CrSt_ManualControl;

    return 0;
}

static int lua_kill_creature(lua_State *L)
{
    int tng_idx = lua_tointeger(L, 1);
    struct Thing *thing = thing_get(tng_idx);
    kill_creature(thing, INVALID_THING, -1, CrDed_NoUnconscious);

    return 0;
}
static struct luaThing *lua_pushThing(lua_State *L, struct Thing* thing)
{
    struct luaThing *lthing = (struct luaThing *)lua_newuserdata(L, sizeof(struct luaThing));
    lthing->thing_idx = thing->index;
    lthing->creation_turn = thing->creation_turn;
    luaL_getmetatable(L, "thing");
    lua_setmetatable(L, -2);
    return lthing;
}
static struct Thing *lua_toThing(lua_State *L, int index)
{
    luaL_checktype(L, index, LUA_TUSERDATA);

    struct luaThing *ltng = (struct luaThing *)lua_touserdata(L, index);

    if (ltng == NULL)
    {
        JUSTLOG("3");
        luaL_typerror(L, index, "thing");
        return INVALID_THING;
    }
    JUSTLOG("4");
    struct Thing* thing = thing_get(ltng->thing_idx);
    JUSTLOG("5");
    if(thing_is_invalid(thing))
    {
        JUSTLOG("6");
        return INVALID_THING;
    }
    if(ltng->creation_turn != thing->creation_turn)
    {
        JUSTLOG("7");
        return INVALID_THING;
    }
    JUSTLOG("8");
    return thing;
}

static int thing_tostring(lua_State *L)
{
    char buff[32];
    struct Thing* thing = lua_toThing(L, 1);
    sprintf(buff, "id: %d turn: %ld class: %d", thing->index, thing->creation_turn,thing->class_id);

    lua_pushfstring(L, "Foo (%s)", buff);
    return 1;
}

static const luaL_reg thing_methods[] = {
    {"MakeThingZombie", make_thing_zombie},
    {"MoveThingTo", move_thing_to},
    {"KillCreature", lua_kill_creature},
    {0, 0}};

static const luaL_reg thing_meta[] = {
    {"__tostring", thing_tostring},
    {0, 0}};

static int Thing_register(lua_State *L)
{
    luaL_openlib(L, "thing", thing_methods, 0); /* create methods table,
                                             add it to the globals */
    luaL_newmetatable(L, "thing");              /* create metatable for Foo,
                                               and add it to the Lua registry */
    luaL_openlib(L, 0, thing_meta, 0);           /* fill metatable */
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -3); /* dup methods table*/
    lua_rawset(L, -3);    /* metatable.__index = methods */
    lua_pushliteral(L, "__metatable");
    lua_pushvalue(L, -3); /* dup methods table*/
    lua_rawset(L, -3);    /* hide metatable:
                             metatable.__metatable = methods */
    lua_pop(L, 1);        /* drop metatable */
    return 1;             /* return methods on the stack */
}

/**********************************************/
//...
/**********************************************/


void reg_host_functions(lua_State *L)
{
    Game_register(L);
    Thing_register(L);
}



#ifdef potato



extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

class Account {
  public:
    Account(double balance)      { m_balance = balance; }
    void deposit(double amount)  { m_balance += amount; }
    void withdraw(double amount) { m_balance -= amount; }
    double balance(void)         { return m_balance; }
  private:
    double m_balance;
};

class LuaAccount {
    static const char className[];
    static const luaL_reg methods[];

    static Account *checkaccount(lua_State *L, int narg) {
      luaL_checktype(L, narg, LUA_TUSERDATA);
      void *ud = luaL_checkudata(L, narg, className);
      if(!ud) luaL_typerror(L, narg, className);
      return *(Account**)ud;  // unbox pointer
    }

    static int create_account(lua_State *L) {
      double balance = luaL_checknumber(L, 1);
      Account *a = new Account(balance);
      lua_boxpointer(L, a);
      luaL_getmetatable(L, className);
      lua_setmetatable(L, -2);
      return 1;
    }
    static int deposit(lua_State *L) {
      Account *a = checkaccount(L, 1);
      double amount = luaL_checknumber(L, 2);
      a->deposit(amount);
      return 0;
    }
    static int withdraw(lua_State *L) {
      Account *a = checkaccount(L, 1);
      double amount = luaL_checknumber(L, 2);
      a->withdraw(amount);
      return 0;
    }
    static int balance(lua_State *L) {
      Account *a = checkaccount(L, 1);
      double balance = a->balance();
      lua_pushnumber(L, balance);
      return 1;
    }
    static int gc_account(lua_State *L) {
      Account *a = (Account*)lua_unboxpointer(L, 1);
      delete a;
      return 0;
    }

  public:
    static void Register(lua_State* L) {
      lua_newtable(L);                 int methodtable = lua_gettop(L);
      luaL_newmetatable(L, className); int metatable   = lua_gettop(L);

      lua_pushliteral(L, "__metatable");
      lua_pushvalue(L, methodtable);
      lua_settable(L, metatable);  // hide metatable from Lua getmetatable()

      lua_pushliteral(L, "__index");
      lua_pushvalue(L, methodtable);
      lua_settable(L, metatable);

      lua_pushliteral(L, "__gc");
      lua_pushcfunction(L, gc_account);
      lua_settable(L, metatable);

      lua_pop(L, 1);  // drop metatable

      luaL_openlib(L, 0, methods, 0);  // fill methodtable
      lua_pop(L, 1);  // drop methodtable

      lua_register(L, className, create_account);
    }
};

const char LuaAccount::className[] = "Account";

#define method(class, name) {#name, class::name}

const luaL_reg LuaAccount::methods[] = {
  method(LuaAccount, deposit),
  method(LuaAccount, withdraw),
  method(LuaAccount, balance),
  {0,0}
};

int main(int argc, char* argv[])
{
  lua_State *L = lua_open();

  luaopen_base(L);
  luaopen_table(L);
  luaopen_io(L);
  luaopen_string(L);
  luaopen_math(L);
  luaopen_debug(L);

  LuaAccount::Register(L);

  if(argc>1) lua_dofile(L, argv[1]);

  lua_close(L);
  return 0;
}
















#include "lua.h"
#include "lauxlib.h"

#define FOO "Foo"

typedef struct Foo
{
    int x;
    int y;
} Foo;



static Foo *checkFoo(lua_State *L, int index)
{
    Foo *bar;
    luaL_checktype(L, index, LUA_TUSERDATA);
    bar = (Foo *)luaL_checkudata(L, index, FOO);
    if (bar == NULL)
        luaL_typerror(L, index, FOO);
    return bar;
}

static Foo *pushFoo(lua_State *L)
{
    Foo *bar = (Foo *)lua_newuserdata(L, sizeof(Foo));
    luaL_getmetatable(L, FOO);
    lua_setmetatable(L, -2);
    return bar;
}

static int Foo_new(lua_State *L)
{
    int x = luaL_optint(L, 1, 0);
    int y = luaL_optint(L, 2, 0);
    Foo *bar = pushFoo(L);
    bar->x = x;
    bar->y = y;
    return 1;
}

static int Foo_yourCfunction(lua_State *L)
{
    Foo *bar = checkFoo(L, 1);
    printf("this is yourCfunction\t");
    lua_pushnumber(L, bar->x);
    lua_pushnumber(L, bar->y);
    return 2;
}

static int Foo_setx(lua_State *L)
{
    Foo *bar = checkFoo(L, 1);
    bar->x = luaL_checkint(L, 2);
    lua_settop(L, 1);
    return 1;
}

static int Foo_sety(lua_State *L)
{
    Foo *bar = checkFoo(L, 1);
    bar->y = luaL_checkint(L, 2);
    lua_settop(L, 1);
    return 1;
}

static int Foo_add(lua_State *L)
{
    Foo *bar1 = checkFoo(L, 1);
    Foo *bar2 = checkFoo(L, 2);
    Foo *sum = pushFoo(L);
    sum->x = bar1->x + bar2->x;
    sum->y = bar1->y + bar2->y;
    return 1;
}

static int Foo_dot(lua_State *L)
{
    Foo *bar1 = checkFoo(L, 1);
    Foo *bar2 = checkFoo(L, 2);
    lua_pushnumber(L, bar1->x * bar2->x + bar1->y * bar2->y);
    return 1;
}

static const luaL_reg Foo_methods[] = {
    {"new", Foo_new},
    {"yourCfunction", Foo_yourCfunction},
    {"setx", Foo_setx},
    {"sety", Foo_sety},
    {"add", Foo_add},
    {"dot", Foo_dot},
    {0, 0}};

static int Foo_gc(lua_State *L)
{
    printf("bye, bye, bar = %p\n", toFoo(L, 1));
    return 0;
}

static int Foo_tostring(lua_State *L)
{
    char buff[32];
    sprintf(buff, "%p", toFoo(L, 1));
    lua_pushfstring(L, "Foo (%s)", buff);
    return 1;
}

static const luaL_reg Foo_meta[] = {
    {"__gc", Foo_gc},
    {"__tostring", Foo_tostring},
    {"__add", Foo_add},
    {0, 0}};

int Foo_register(lua_State *L)
{
    luaL_openlib(L, FOO, Foo_methods, 0); /* create methods table,
                                             add it to the globals */
    luaL_newmetatable(L, FOO);            /* create metatable for Foo,
                                             and add it to the Lua registry */
    luaL_openlib(L, 0, Foo_meta, 0);      /* fill metatable */
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -3); /* dup methods table*/
    lua_rawset(L, -3);    /* metatable.__index = methods */
    lua_pushliteral(L, "__metatable");
    lua_pushvalue(L, -3); /* dup methods table*/
    lua_rawset(L, -3);    /* hide metatable:
                             metatable.__metatable = methods */
    lua_pop(L, 1);        /* drop metatable */
    return 1;             /* return methods on the stack */
}

int main(int argc, char *argv[])
{
    lua_State *L = lua_open();

    luaopen_base(L);
    luaopen_table(L);
    luaopen_io(L);
    luaopen_string(L);
    luaopen_math(L);
    luaopen_debug(L);

    Foo_register(L);
    lua_pop(L, 1); // After foo register the methods are still on the stack, remove them.

    if (argc > 1)
        lua_dofile(L, argv[1]);

    lua_close(L);
    return 0;
}

#endif