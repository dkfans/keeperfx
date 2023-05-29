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
#include "lvl_script_lib.h"
#include "room_library.h"
#include "keeperfx.hpp"
#include "music_player.h"


#include "post_inc.h"

/**********************************************/

struct PlayerRange
{
    PlayerNumber start_idx;
    PlayerNumber end_idx;
};

/*****/
static long luaL_checkNamedCommand(lua_State *L, int index,const struct NamedCommand * commanddesc)
{
    if (lua_isnumber(L, index))
    {
        return lua_tointeger(L, index);
    }
    else if(lua_isstring(L, index))
    {
        const char* text = lua_tostring(L, index);
        long id = get_rid(commanddesc, text);

        luaL_argcheck(L,id != -1,index,"invalid namedcommandoption");

        return id;
    }
    luaL_error(L,"invalid namedcommandoption");
    return 0;

}
static struct Thing *luaL_checkThing(lua_State *L, int index)
{
    luaL_checktype(L, index, LUA_TTABLE);
    lua_getfield(L, 1, "idx");
    lua_getfield(L, 1, "creation_turn");

    int tng_idx = luaL_checkint(L, -2);
    int creation_turn = luaL_checkint(L, -1);
    lua_pop(L, 2);

    struct Thing* thing = thing_get(tng_idx);
    if(thing_is_invalid(thing) || thing->creation_turn != creation_turn)
    {
        luaL_error (L,"failed to resolve thing");
    }
    return thing;
}

static TbMapLocation luaL_checkLocation(lua_State *L, int index)
{
    const char* locname = lua_tostring(L, index);
    TbMapLocation location;
    if(!get_map_location_id(locname, &location))
    {
        luaL_error (L,"Invalid location, '%s'", locname);
    }
    return location;
}

static TbMapLocation luaL_optLocation(lua_State *L, int index)
{
    if (lua_isnone(L,index))
        return 0;
    else
        return luaL_checkLocation(L,index);
}

static TbMapLocation luaL_checkHeadingLocation(lua_State *L, int index)
{
    const char* locname = lua_tostring(L, index);
    long target = luaL_checkNamedCommand(L, index + 1,head_for_desc);

    
    TbMapLocation location;
    if(!get_map_heading_id(locname, target, &location))
    {
        luaL_error (L,"Invalid location, '%s'", locname);
    }
    return location;
}
static struct PlayerRange luaL_checkPlayerRange(lua_State *L, int index)
{
    struct PlayerRange playerRange = {0,0};
    const char* plrname = lua_tostring(L, index);
    
    long plr_range_id = get_rid(player_desc, plrname);
    if (plr_range_id == -1)
    {
        plr_range_id = get_rid(cmpgn_human_player_options, plrname);
    }
    if (plr_range_id == ALL_PLAYERS)
    {
        playerRange.start_idx = 0;
        playerRange.end_idx = PLAYERS_COUNT;
    }
    else
    {
        playerRange.start_idx = plr_range_id;
        playerRange.end_idx   = plr_range_id;
    }

    return playerRange;
}

static PlayerNumber luaL_checkPlayerSingle(lua_State *L, int index)
{
    struct PlayerRange playerRange = luaL_checkPlayerRange(L,index);
    if(playerRange.start_idx != playerRange.end_idx)
    {
        luaL_error (L,"player range not supported for this command");
    }
    return playerRange.start_idx;
}

static MapSubtlCoord luaL_checkstl_x(lua_State *L, int index)
{
    MapSubtlCoord stl_x = luaL_checkint(L,index);
    luaL_argcheck(L, 0 <= stl_x && stl_x <= gameadd.map_subtiles_x, index,
                       "x subtile coord out of range");
    return stl_x;
}

static MapSubtlCoord luaL_checkstl_y(lua_State *L, int index)
{
    MapSubtlCoord stl_y = luaL_checkint(L,index);
    luaL_argcheck(L, 0 <= stl_y && stl_y <= gameadd.map_subtiles_y, index,
                       "y subtile coord out of range");
    return stl_y;
}

static ActionPointId luaL_checkActionPoint(lua_State *L, int index)
{
    int apt_num = luaL_checkint(L,index);
    long apt_idx = action_point_number_to_index(apt_num);
    if (!action_point_exists_idx(apt_idx))
    {
        luaL_error(L,"Non-existing Action Point, no %d", apt_num);
        return;
    }
    return apt_idx;
}





/**********/

void lua_pushThing(lua_State *L, struct Thing* thing)
{
    if(thing_is_invalid(thing))
    {
        
    }
    
    lua_createtable(L, 0, 4); /* creates and pushes new table on top of Lua stack */

    lua_pushinteger(L, thing->index); /* Pushes table value on top of Lua stack */
    lua_setfield(L, -2, "index");  /* table["name"] = row->name. Pops key value */

    lua_pushinteger(L, thing->creation_turn);
    lua_setfield(L, -2, "creation_turn");

    /* Returning one table which is already on top of Lua stack. */
    return 1;

    /*
    struct luaThing *lthing = (struct luaThing *)lua_newtable(L, );
    lthing->thing_idx = thing->index;
    lthing->creation_turn = thing->creation_turn;
    luaL_getmetatable(L, "thing");
    lua_setmetatable(L, -2);
    return lthing;
    */
}

/**********************************************/


static int lua_CREATE_PARTY(lua_State *L)
{
    const char* party_name = luaL_checklstring(L, 1,NULL);
    create_party(party_name);
    return 0;
}
static int lua_ADD_TO_PARTY(lua_State *L)
{
    const char* party_name = luaL_checkstring(L,  1);
    long crtr_id           = luaL_checkNamedCommand(L,2,creature_desc);
    long experience        = luaL_checklong(L, 3);
    long gold              = luaL_checklong(L, 4);
    long objective_id      = luaL_checkNamedCommand(L, 5,hero_objective_desc);
    long countdown         = luaL_checklong (L, 6);

    int party_id = get_party_index_of_name(party_name);
    if (party_id < 0)
    {
        SCRPTERRLOG("Invalid Party:%s",party_name);
        return 0;
    }
    if ((experience < 1) || (experience > CREATURE_MAX_LEVEL))
    {
      SCRPTERRLOG("Invalid Creature Level parameter; %ld not in range (%d,%d)",experience,1,CREATURE_MAX_LEVEL);
      return 0;
    }

    add_member_to_party(party_id, crtr_id, experience, gold, objective_id, countdown);
    return 0;
}
static int lua_DELETE_FROM_PARTY(lua_State *L)
{
    const char* party_name = lua_tostring(L,  1);
    const char* creature   = lua_tostring(L,  2);
    long experience  = lua_tointeger(L, 3);

    int party_id = get_party_index_of_name(party_name);
    if (party_id < 0)
    {
        SCRPTERRLOG("Invalid Party:%s",party_name);
        return 0;
    }
    long creature_id = get_rid(creature_desc, creature);
    if (creature_id == -1)
    {
      SCRPTERRLOG("Unknown creature, '%s'", creature);
      return 0;
    }

    delete_member_from_party(party_id, creature_id, experience);
    return 0;
}
static int lua_ADD_PARTY_TO_LEVEL(lua_State *L)
{
    long plr_range_id      = luaL_checkinteger(L, 1);
    const char *prtname    = luaL_checkstring(L,  2);
    TbMapLocation location = luaL_checkLocation(L,  3);
    long ncopies           = luaL_checkinteger(L, 4);

    if (ncopies < 1)
    {
        SCRPTERRLOG("Invalid NUMBER parameter");
        return 0;
    }
    // Verify player
    long plr_id = get_players_range_single(plr_range_id);
    if (plr_id < 0) {
        SCRPTERRLOG("Given owning player is not supported in this command");
        return 0;
    }
    // Recognize place where party is created
    if (location == 0)
        return 0;
    // Recognize party name
    long prty_id = get_party_index_of_name(prtname);
    if (prty_id < 0)
    {
        SCRPTERRLOG("Party of requested name, '%s', is not defined",prtname);
        return 0;
    }
    struct Party* party = &gameadd.script.creature_partys[prty_id];
    script_process_new_party(party, plr_id, location, ncopies);
        return 0;
}

static int lua_ADD_CREATURE_TO_LEVEL(lua_State *L)
{
    PlayerNumber plr_idx   = luaL_checkPlayerSingle(L, 1);
    long crtr_id           = luaL_checkNamedCommand(L,2,creature_desc);
    TbMapLocation location = luaL_checkLocation(L,  3);
    long ncopies           = luaL_checkinteger(L, 4);
    long crtr_level        = luaL_checkinteger(L, 5);
    long carried_gold      = luaL_checkinteger(L, 6);
       
    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return 0;
    }
    if ((ncopies <= 0) || (ncopies >= CREATURES_COUNT))
    {
        SCRPTERRLOG("Invalid number of creatures to add");
        return 0;
    }

    // Recognize place where party is created
    if (location == 0)
        return 0;

    for (long i = 0; i < ncopies; i++)
    {
        lua_pushThing(L,script_create_new_creature(plr_idx, crtr_id, location, carried_gold, crtr_level-1));
    }
    return ncopies;
}

static int lua_ADD_OBJECT_TO_LEVEL(lua_State *L)
{
    long obj_id            = luaL_checkNamedCommand(L,1,object_desc);
    TbMapLocation location = luaL_checkLocation(L,  2);
    long arg               = lua_tointeger(L,3);
    PlayerNumber plr_idx   = luaL_checkPlayerSingle(L, 4);

    lua_pushThing(L,script_process_new_object(obj_id, location, arg, plr_idx));
    return 1;
}

static int lua_SET_HATE(lua_State *L)
{
    struct PlayerRange players = luaL_checkPlayerRange(L,1);
    PlayerNumber enmy_plr_id   = luaL_checkPlayerSingle(L,2);
    long hate_val   = luaL_checkinteger(L,3);

    for (PlayerNumber i=players.start_idx; i < players.end_idx; i++)
    {
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        dungeon->hates_player[enmy_plr_id%DUNGEONS_COUNT] = hate_val;
    }
    return 0;
}

static int lua_SET_GENERATE_SPEED(lua_State *L)
{
    GameTurnDelta interval   = luaL_checkinteger(L,1);

    game.generate_speed = saturate_set_unsigned(interval, 16);
    update_dungeon_generation_speeds();
    return 0;
}

static int lua_START_MONEY(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    GoldAmount gold_val = luaL_checkinteger(L, 2);
        
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        if (gold_val > SENSIBLE_GOLD)
        {
            gold_val = SENSIBLE_GOLD;
            SCRPTWRNLOG("Gold added to player reduced to %d", SENSIBLE_GOLD);
        }
        player_add_offmap_gold(i, gold_val);
    }
    return 0; 

}

static int lua_ROOM_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long rkind                      = luaL_checkNamedCommand(L,2,room_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    TbBool is_available             = lua_toboolean(L, 4);
        
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        set_room_available(i,rkind,can_be_available,is_available);
    }
    return 0; 
}
static int lua_CREATURE_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long cr_kind                    = luaL_checkNamedCommand(L,2,creature_desc);
    TbBool can_be_attracted         = lua_toboolean(L, 3);
    long amount_forced              = luaL_checkinteger(L, 4);
        
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        if (!set_creature_available(i,cr_kind,can_be_attracted,amount_forced))
            WARNLOG("Setting creature %s availability for player %d failed.",creature_code_name(cr_kind),(int)i);
    }
    return 0; 
}
static int lua_MAGIC_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long spell                      = luaL_checkNamedCommand(L,2,spell_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    TbBool is_available             = lua_toboolean(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        set_power_available(i,spell,can_be_available,is_available);
    }
    return 0;
}
static int lua_TRAP_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long trap_type                  = luaL_checkNamedCommand(L,2,trap_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    long number_available           = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        set_trap_buildable_and_add_to_amount(i, trap_type, can_be_available, number_available);
    }
    return 0;
}
static int lua_RESEARCH(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long research_type              = luaL_checkNamedCommand(L,2,research_desc);
    int room_or_spell;
    switch (research_type)
    {
        case 1:
            room_or_spell = luaL_checkNamedCommand(L,3,power_desc); 
            break;
        case 2:
            room_or_spell = luaL_checkNamedCommand(L,3,room_desc); 
            break;
        default:
            luaL_error (L,"invalid research_type %d",research_type);
            return 0;
    }
    long research_value         = luaL_checkint(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        update_or_add_players_research_amount(i, research_type, room_or_spell, research_value);
    }
    return 0;
}
static int lua_RESEARCH_ORDER(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long research_type              = luaL_checkNamedCommand(L,2,research_desc);
    int room_or_spell;
    switch (research_type)
    {
        case 1:
            room_or_spell = luaL_checkNamedCommand(L,3,power_desc); 
            break;
        case 2:
            room_or_spell = luaL_checkNamedCommand(L,3,room_desc); 
            break;
        default:
            luaL_error (L,"invalid research_type %d",research_type);
            return 0;
    }
    long research_value         = luaL_checkint(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        if (!research_overriden_for_player(i))
            remove_all_research_from_player(i);
        add_research_to_player(i, research_type, room_or_spell, research_value);
    }
    return 0;
}

static int lua_COMPUTER_PLAYER(lua_State *L)
{
    PlayerNumber player = luaL_checkPlayerSingle(L,1);
    long attitude       = luaL_checkint(L,2);

    script_support_setup_player_as_computer_keeper(player, attitude);
    return 0;
}

static int lua_SET_TIMER(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long timr_id              = luaL_checkNamedCommand(L,2,timer_desc);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        restart_script_timer(i, timr_id);
    }
    return 0;
}

static int lua_ADD_TUNNELLER_TO_LEVEL(lua_State *L)
{
    PlayerNumber plr_id          = luaL_checkPlayerSingle(L,1);
    TbMapLocation spawn_location = luaL_checkLocation(L,2);
    TbMapLocation head_for       = luaL_checkHeadingLocation(L,3);
    long level                   = luaL_checkinteger(L,5);
    long gold_held               = luaL_checkinteger(L,6);

    struct Thing* thing = script_process_new_tunneler(plr_id, spawn_location, head_for, level-1, gold_held);

    lua_pushThing(L, thing);
    return 1;
}

static int lua_WIN_GAME(lua_State *L)
{
    struct PlayerRange player_range;
    if(lua_isnone(L,1))
    {
        player_range.start_idx = 0;
        player_range.end_idx = PLAYERS_COUNT;
    }        
    else 
        player_range = luaL_checkPlayerRange(L, 1);

        
    for (size_t i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct PlayerInfo *player = get_player(i);
        set_player_as_won_level(player);
    }
    return 0; 
}
static int lua_LOSE_GAME(lua_State *L)
{
    struct PlayerRange player_range;
    if(lua_isnone(L,1))
    {
        player_range.start_idx = 0;
        player_range.end_idx = PLAYERS_COUNT;
    }        
    else 
        player_range = luaL_checkPlayerRange(L, 1);

    for (size_t i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct PlayerInfo *player = get_player(i);
        set_player_as_lost_level(player);
    }
    return 0;
}

static int lua_MAX_CREATURES(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long max_amount                 = luaL_checkinteger(L, 2);

    for (int i=player_range.start_idx; i < player_range.end_idx; i++)
    {
        SYNCDBG(4,"Setting player %d max attracted creatures to %d.",(int)i,(int)max_amount);
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        dungeon->max_creatures_attracted = max_amount;
    }
    return 0;
}

static int lua_DOOR_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long door_type                  = luaL_checkNamedCommand(L,2,door_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    long number_available           = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        set_door_buildable_and_add_to_amount(i, door_type, can_be_available, number_available);
    }
    return 0;
}

static int lua_DISPLAY_OBJECTIVE(lua_State *L)
{
    long msg_id    = luaL_checkinteger(L, 1);
    TbMapLocation zoom_location = luaL_optLocation(L,2);

    set_general_objective(msg_id,zoom_location,0,0);
    return 0;
}

static int lua_DISPLAY_OBJECTIVE_WITH_POS(lua_State *L)
{
    long msg_id   = luaL_checkinteger(L, 1);
    long stl_x    = luaL_checkstl_x(L, 2);
    long stl_y    = luaL_checkstl_y(L, 3);
    
    set_general_objective(msg_id,0,stl_x,stl_y);
    return 0;
}

static int lua_DISPLAY_INFORMATION(lua_State *L)
{
    long msg_id    = luaL_checkinteger(L, 1);
    TbMapLocation zoom_location = luaL_optLocation(L,2);

    set_general_information(msg_id,zoom_location,0,0);
    return 0;
}

static int lua_DISPLAY_INFORMATION_WITH_POS(lua_State *L)
{
    long msg_id    = luaL_checkinteger(L, 1);
    long stl_x    = luaL_checkstl_x(L, 2);
    long stl_y    = luaL_checkstl_y(L, 3);

    set_general_objective(msg_id,0,stl_x,stl_y);
    return 0;
}

static int lua_ADD_TUNNELLER_PARTY_TO_LEVEL(lua_State *L)
{
    PlayerNumber owner           = luaL_checkPlayerSingle(L, 1);
    const char *party_name       = luaL_checkstring(L,  2);
    TbMapLocation spawn_location = luaL_checkLocation(L,  3);
    TbMapLocation head_for       = luaL_checkHeadingLocation(L,4);
    long target                  = luaL_checkinteger(L, 5);
    long crtr_level              = luaL_checkinteger(L, 6);
    GoldAmount carried_gold      = luaL_checkinteger(L, 7);

    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return 0;
    }
    // Recognize party name
    long prty_id = get_party_index_of_name(party_name);
    if (prty_id < 0)
    {
        SCRPTERRLOG("Party of requested name, '%s', is not defined", party_name);
        return 0;
    }
    struct Party* party = &gameadd.script.creature_partys[prty_id];
    if (party->members_num >= GROUP_MEMBERS_COUNT-1)
    {
        SCRPTERRLOG("Party too big for ADD_TUNNELLER (Max %d members)", GROUP_MEMBERS_COUNT-1);
        return 0;
    }

    script_process_new_tunneller_party(owner, prty_id, spawn_location, head_for, crtr_level-1, carried_gold);
    return 0;
}

static int lua_ADD_CREATURE_TO_POOL(lua_State *L)
{
    long crtr_model = luaL_checkNamedCommand(L,1,creature_desc);
    long amount     = luaL_checkinteger(L, 2);

    add_creature_to_pool(crtr_model, amount, false);
    return 0;
}

static int lua_RESET_ACTION_POINT(lua_State *L)
{
    ActionPointId apt_idx     = luaL_checkActionPoint(L, 1);

    action_point_reset_idx(apt_idx);
    return 0;
}

static int lua_SET_CREATURE_MAX_LEVEL(lua_State *L)
{

    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long crtr_model                 = luaL_checkNamedCommand(L,2,creature_desc);
    long max_level                  = luaL_checkinteger(L, 3);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        if (max_level == -1)
            max_level = CREATURE_MAX_LEVEL + 1;
        dungeon->creature_max_level[crtr_model%gameadd.crtr_conf.model_count] = max_level;
    }
    return 0;
}

static int lua_SET_MUSIC(lua_State *L)
{
    long track_number                  = luaL_checkinteger(L, 1);

    if (track_number >= FIRST_TRACK && track_number <= max_track)
    {
        game.audiotrack = track_number;
    }
    
}

static int lua_TUTORIAL_FLASH_BUTTON(lua_State *L)
{

    long          button    = luaL_checkinteger(L, 1);
    GameTurnDelta gameturns = luaL_checkinteger(L, 2);

    gui_set_button_flashing(button,gameturns);
}
/*
static int lua_SET_CREATURE_STRENGTH(lua_State *L)
static int lua_SET_CREATURE_HEALTH(lua_State *L)
static int lua_SET_CREATURE_ARMOUR(lua_State *L)
static int lua_SET_CREATURE_FEAR_WOUNDED(lua_State *L)
static int lua_SET_CREATURE_FEAR_STRONGER(lua_State *L)
static int lua_SET_CREATURE_FEARSOME_FACTOR(lua_State *L)
static int lua_SET_CREATURE_PROPERTY(lua_State *L)
static int lua_SET_COMPUTER_GLOBALS(lua_State *L)
static int lua_SET_COMPUTER_CHECKS(lua_State *L)
static int lua_SET_COMPUTER_EVENT(lua_State *L)
static int lua_SET_COMPUTER_PROCESS(lua_State *L)
static int lua_ALLY_PLAYERS(lua_State *L)
static int lua_DEAD_CREATURES_RETURN_TO_POOL(lua_State *L)
static int lua_BONUS_LEVEL_TIME(lua_State *L)
static int lua_QUICK_OBJECTIVE(lua_State *L)
static int lua_QUICK_INFORMATION(lua_State *L)
static int lua_QUICK_OBJECTIVE_WITH_POS(lua_State *L)
static int lua_QUICK_INFORMATION_WITH_POS(lua_State *L)
static int lua_SWAP_CREATURE(lua_State *L)
static int lua_PRINT(lua_State *L)
static int lua_MESSAGE(lua_State *L)
static int lua_PLAY_MESSAGE(lua_State *L)
static int lua_ADD_GOLD_TO_PLAYER(lua_State *L)
static int lua_SET_CREATURE_TENDENCIES(lua_State *L)
static int lua_REVEAL_MAP_RECT(lua_State *L)
static int lua_CONCEAL_MAP_RECT(lua_State *L)
static int lua_REVEAL_MAP_LOCATION(lua_State *L)
static int lua_LEVEL_VERSION(lua_State *L)
static int lua_KILL_CREATURE(lua_State *L)
static int lua_COMPUTER_DIG_TO_LOCATION(lua_State *L)
static int lua_USE_POWER_ON_CREATURE(lua_State *L)
static int lua_USE_POWER_AT_POS(lua_State *L)
static int lua_USE_POWER_AT_SUBTILE(lua_State *L)
static int lua_USE_POWER_AT_LOCATION(lua_State *L)
static int lua_USE_POWER(lua_State *L)
static int lua_USE_SPECIAL_INCREASE_LEVEL(lua_State *L)
static int lua_USE_SPECIAL_MULTIPLY_CREATURES",
static int lua_USE_SPECIAL_MAKE_SAFE(lua_State *L)
static int lua_USE_SPECIAL_LOCATE_HIDDEN_WORLD"
static int lua_USE_SPECIAL_TRANSFER_CREATURE(lua_State *L)
static int lua_TRANSFER_CREATURE(lua_State *L)
static int lua_CHANGE_CREATURES_ANNOYANCE(lua_State *L)
static int lua_RUN_AFTER_VICTORY(lua_State *L)
static int lua_LEVEL_UP_CREATURE(lua_State *L)
static int lua_CHANGE_CREATURE_OWNER(lua_State *L)
static int lua_SET_GAME_RULE(lua_State *L)
static int lua_SET_TRAP_CONFIGURATION(lua_State *L)
static int lua_SET_DOOR_CONFIGURATION(lua_State *L)
static int lua_SET_OBJECT_CONFIGURATION(lua_State *L)
static int lua_SET_CREATURE_CONFIGURATION(lua_State *L)
static int lua_SET_SACRIFICE_RECIPE(lua_State *L)
static int lua_REMOVE_SACRIFICE_RECIPE(lua_State *L)
static int lua_SET_BOX_TOOLTIP(lua_State *L)
static int lua_SET_BOX_TOOLTIP_ID(lua_State *L)
static int lua_CHANGE_SLAB_OWNER(lua_State *L)
static int lua_CHANGE_SLAB_TYPE(lua_State *L)
static int lua_CREATE_EFFECTS_LINE(lua_State *L)
static int lua_QUICK_MESSAGE(lua_State *L)
static int lua_DISPLAY_MESSAGE(lua_State *L)
static int lua_USE_SPELL_ON_CREATURE(lua_State *L)
static int lua_SET_HEART_HEALTH(lua_State *L)
static int lua_ADD_HEART_HEALTH(lua_State *L)
static int lua_CREATURE_ENTRANCE_LEVEL(lua_State *L)
static int lua_RANDOMISE_FLAG(lua_State *L)
static int lua_COMPUTE_FLAG(lua_State *L)
static int lua_DISPLAY_TIMER(lua_State *L)
static int lua_ADD_TO_TIMER(lua_State *L)
static int lua_ADD_BONUS_TIME(lua_State *L)
static int lua_DISPLAY_VARIABLE(lua_State *L)
static int lua_DISPLAY_COUNTDOWN(lua_State *L)
static int lua_HIDE_TIMER(lua_State *L)
static int lua_HIDE_VARIABLE(lua_State *L)
static int lua_CREATE_EFFECT(lua_State *L)
static int lua_CREATE_EFFECT_AT_POS(lua_State *L)
static int lua_HEART_LOST_QUICK_OBJECTIVE(lua_State *L)
static int lua_HEART_LOST_OBJECTIVE(lua_State *L)
static int lua_SET_DOOR(lua_State *L)
static int lua_SET_CREATURE_INSTANCE(lua_State *L)
static int lua_SET_HAND_RULE(lua_State *L)
static int lua_MOVE_CREATURE(lua_State *L)
static int lua_COUNT_CREATURES_AT_ACTION_POINT(lua_State *L)
static int lua_SET_TEXTURE(lua_State *L)
static int lua_HIDE_HERO_GATE(lua_State *L)




*/














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



static const luaL_reg game_methods[] = {
    {"CREATE_PARTY",                        lua_CREATE_PARTY},
    {"ADD_TO_PARTY",                        lua_ADD_TO_PARTY},
    {"DELETE_FROM_PARTY",                   lua_DELETE_FROM_PARTY},
    {"ADD_PARTY_TO_LEVEL",                  lua_ADD_PARTY_TO_LEVEL},
    {"ADD_CREATURE_TO_LEVEL",               lua_ADD_CREATURE_TO_LEVEL},
    {"ADD_OBJECT_TO_LEVEL",                 lua_ADD_OBJECT_TO_LEVEL},
    {"SET_HATE",                            lua_SET_HATE},
    {"SET_GENERATE_SPEED",                  lua_SET_GENERATE_SPEED},
    {"START_MONEY",                         lua_START_MONEY},
    {"ROOM_AVAILABLE",                      lua_ROOM_AVAILABLE},
    {"CREATURE_AVAILABLE",                  lua_CREATURE_AVAILABLE},
    {"MAGIC_AVAILABLE",                     lua_MAGIC_AVAILABLE},
    {"TRAP_AVAILABLE",                      lua_TRAP_AVAILABLE},
    {"RESEARCH",                            lua_RESEARCH},
    {"RESEARCH_ORDER",                      lua_RESEARCH_ORDER},
    {"COMPUTER_PLAYER",                     lua_COMPUTER_PLAYER},
    {"SET_TIMER",                           lua_SET_TIMER},
    {"ADD_TUNNELLER_TO_LEVEL",              lua_ADD_TUNNELLER_TO_LEVEL},
    {"WIN_GAME",                            lua_WIN_GAME},
    {"LOSE_GAME",                           lua_LOSE_GAME},
    {"MAX_CREATURES",                       lua_MAX_CREATURES},
    {"DOOR_AVAILABLE",                      lua_DOOR_AVAILABLE},
    {"DISPLAY_OBJECTIVE",                   lua_DISPLAY_OBJECTIVE},
    {"DISPLAY_OBJECTIVE_WITH_POS",          lua_DISPLAY_OBJECTIVE_WITH_POS},
    {"DISPLAY_INFORMATION",                 lua_DISPLAY_INFORMATION},
    {"DISPLAY_INFORMATION_WITH_POS",        lua_DISPLAY_INFORMATION_WITH_POS},
    {"ADD_TUNNELLER_PARTY_TO_LEVEL",        lua_ADD_TUNNELLER_PARTY_TO_LEVEL},
    {"ADD_CREATURE_TO_POOL",                lua_ADD_CREATURE_TO_POOL},
    {"RESET_ACTION_POINT",                  lua_RESET_ACTION_POINT},
    {"SET_CREATURE_MAX_LEVEL",              lua_SET_CREATURE_MAX_LEVEL},
    {"SET_MUSIC",                           lua_SET_MUSIC},
    {"TUTORIAL_FLASH_BUTTON",               lua_TUTORIAL_FLASH_BUTTON},
    /*
    {"SET_CREATURE_STRENGTH",               lua_SET_CREATURE_STRENGTH},
    {"SET_CREATURE_HEALTH",                 lua_SET_CREATURE_HEALTH},
    {"SET_CREATURE_ARMOUR",                 lua_SET_CREATURE_ARMOUR},
    {"SET_CREATURE_FEAR_WOUNDED",           lua_SET_CREATURE_FEAR_WOUNDED},
    {"SET_CREATURE_FEAR_STRONGER",          lua_SET_CREATURE_FEAR_STRONGER},
    {"SET_CREATURE_FEARSOME_FACTOR",        lua_SET_CREATURE_FEARSOME_FACTOR},
    {"SET_CREATURE_PROPERTY",               lua_SET_CREATURE_PROPERTY},
    {"SET_COMPUTER_GLOBALS",                lua_SET_COMPUTER_GLOBALS},
    {"SET_COMPUTER_CHECKS",                 lua_SET_COMPUTER_CHECKS},
    {"SET_COMPUTER_EVENT",                  lua_SET_COMPUTER_EVENT},
    {"SET_COMPUTER_PROCESS",                lua_SET_COMPUTER_PROCESS},
    {"ALLY_PLAYERS",                        lua_ALLY_PLAYERS},
    {"DEAD_CREATURES_RETURN_TO_POOL",       lua_DEAD_CREATURES_RETURN_TO_POOL},
    {"BONUS_LEVEL_TIME",                    lua_BONUS_LEVEL_TIME},
    {"QUICK_OBJECTIVE",                     lua_QUICK_OBJECTIVE},
    {"QUICK_INFORMATION",                   lua_QUICK_INFORMATION},
    {"QUICK_OBJECTIVE_WITH_POS",            lua_QUICK_OBJECTIVE_WITH_POS},
    {"QUICK_INFORMATION_WITH_POS",          lua_QUICK_INFORMATION_WITH_POS},
    {"SWAP_CREATURE",                       lua_SWAP_CREATURE},
    {"PRINT",                               lua_PRINT},
    {"MESSAGE",                             lua_MESSAGE},
    {"PLAY_MESSAGE",                        lua_PLAY_MESSAGE},
    {"ADD_GOLD_TO_PLAYER",                  lua_ADD_GOLD_TO_PLAYER},
    {"SET_CREATURE_TENDENCIES",             lua_SET_CREATURE_TENDENCIES},
    {"REVEAL_MAP_RECT",                     lua_REVEAL_MAP_RECT},
    {"CONCEAL_MAP_RECT",                    lua_CONCEAL_MAP_RECT},
    {"REVEAL_MAP_LOCATION",                 lua_REVEAL_MAP_LOCATION},
    {"LEVEL_VERSION",                       lua_LEVEL_VERSION},
    {"KILL_CREATURE",                       lua_KILL_CREATURE},
    {"COMPUTER_DIG_TO_LOCATION",            lua_COMPUTER_DIG_TO_LOCATION},
    {"USE_POWER_ON_CREATURE",               lua_USE_POWER_ON_CREATURE},
    {"USE_POWER_AT_POS",                    lua_USE_POWER_AT_POS},
    {"USE_POWER_AT_SUBTILE",                lua_USE_POWER_AT_SUBTILE},
    {"USE_POWER_AT_LOCATION",               lua_USE_POWER_AT_LOCATION},
    {"USE_POWER",                           lua_USE_POWER},
    {"USE_SPECIAL_INCREASE_LEVEL",          lua_USE_SPECIAL_INCREASE_LEVEL},
    {"USE_SPECIAL_MULTIPLY_CREATURES",      lua_USE_SPECIAL_MULTIPLY_CREATURES},
    {"USE_SPECIAL_MAKE_SAFE",               lua_USE_SPECIAL_MAKE_SAFE},
    {"USE_SPECIAL_LOCATE_HIDDEN_WORLD",     lua_USE_SPECIAL_LOCATE_HIDDEN_WORLD},
    {"USE_SPECIAL_TRANSFER_CREATURE",       lua_USE_SPECIAL_TRANSFER_CREATURE},
    {"TRANSFER_CREATURE",                   lua_TRANSFER_CREATURE},
    {"CHANGE_CREATURES_ANNOYANCE",          lua_CHANGE_CREATURES_ANNOYANCE},
    {"ADD_TO_FLAG",                         lua_ADD_TO_FLAG},
    {"SET_CAMPAIGN_FLAG",                   lua_SET_CAMPAIGN_FLAG},
    {"ADD_TO_CAMPAIGN_FLAG",                lua_ADD_TO_CAMPAIGN_FLAG},
    {"EXPORT_VARIABLE",                     lua_EXPORT_VARIABLE},
    {"RUN_AFTER_VICTORY",                   lua_RUN_AFTER_VICTORY},
    {"LEVEL_UP_CREATURE",                   lua_LEVEL_UP_CREATURE},
    {"CHANGE_CREATURE_OWNER",               lua_CHANGE_CREATURE_OWNER},
    {"SET_GAME_RULE",                       lua_SET_GAME_RULE},
    {"SET_TRAP_CONFIGURATION",              lua_SET_TRAP_CONFIGURATION},
    {"SET_DOOR_CONFIGURATION",              lua_SET_DOOR_CONFIGURATION},
    {"SET_OBJECT_CONFIGURATION",            lua_SET_OBJECT_CONFIGURATION},
    {"SET_CREATURE_CONFIGURATION",          lua_SET_CREATURE_CONFIGURATION},
    {"SET_SACRIFICE_RECIPE",                lua_SET_SACRIFICE_RECIPE},
    {"REMOVE_SACRIFICE_RECIPE",             lua_REMOVE_SACRIFICE_RECIPE},
    {"SET_BOX_TOOLTIP",                     lua_SET_BOX_TOOLTIP},
    {"SET_BOX_TOOLTIP_ID",                  lua_SET_BOX_TOOLTIP_ID},
    {"CHANGE_SLAB_OWNER",                   lua_CHANGE_SLAB_OWNER},
    {"CHANGE_SLAB_TYPE",                    lua_CHANGE_SLAB_TYPE},
    {"CREATE_EFFECTS_LINE",                 lua_CREATE_EFFECTS_LINE},
    {"QUICK_MESSAGE",                       lua_QUICK_MESSAGE},
    {"DISPLAY_MESSAGE",                     lua_DISPLAY_MESSAGE},
    {"USE_SPELL_ON_CREATURE",               lua_USE_SPELL_ON_CREATURE},
    {"SET_HEART_HEALTH",                    lua_SET_HEART_HEALTH},
    {"ADD_HEART_HEALTH",                    lua_ADD_HEART_HEALTH},
    {"CREATURE_ENTRANCE_LEVEL",             lua_CREATURE_ENTRANCE_LEVEL},
    {"RANDOMISE_FLAG",                      lua_RANDOMISE_FLAG},
    {"COMPUTE_FLAG",                        lua_COMPUTE_FLAG},
    {"DISPLAY_TIMER",                       lua_DISPLAY_TIMER},
    {"ADD_TO_TIMER",                        lua_ADD_TO_TIMER},
    {"ADD_BONUS_TIME",                      lua_ADD_BONUS_TIME},
    {"DISPLAY_VARIABLE",                    lua_DISPLAY_VARIABLE},
    {"DISPLAY_COUNTDOWN",                   lua_DISPLAY_COUNTDOWN},
    {"HIDE_TIMER",                          lua_HIDE_TIMER},
    {"HIDE_VARIABLE",                       lua_HIDE_VARIABLE},
    {"CREATE_EFFECT",                       lua_CREATE_EFFECT},
    {"CREATE_EFFECT_AT_POS",                lua_CREATE_EFFECT_AT_POS},
    {"HEART_LOST_QUICK_OBJECTIVE",          lua_HEART_LOST_QUICK_OBJECTIVE},
    {"HEART_LOST_OBJECTIVE",                lua_HEART_LOST_OBJECTIVE},
    {"SET_DOOR",                            lua_SET_DOOR},
    {"SET_CREATURE_INSTANCE",               lua_SET_CREATURE_INSTANCE},
    {"SET_HAND_RULE",                       lua_SET_HAND_RULE},
    {"MOVE_CREATURE",                       lua_MOVE_CREATURE},
    {"COUNT_CREATURES_AT_ACTION_POINT",     lua_COUNT_CREATURES_AT_ACTION_POINT},
    {"SET_TEXTURE",                         lua_SET_TEXTURE},
    {"HIDE_HERO_GATE",                      lua_HIDE_HERO_GATE},
    */
    {"GetCreatureNear", lua_get_creature_near},
    {"SendChatMessage", send_chat_message},
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

static int make_thing_zombie (lua_State *L)
{
    JUSTLOG("make_thing_zombie");
    struct Thing *thing = luaL_checkThing(L, 1);

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

static int thing_tostring(lua_State *L)
{
    char buff[32];
    struct Thing* thing = luaL_checkThing(L, 1);
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