#include "pre_inc.h"

#include "../deps/luajit/src/lua.h"
#include "../deps/luajit/src/lauxlib.h"
#include "../deps/luajit/src/lualib.h"

#include "bflib_basics.h"
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
#include "keeperfx.hpp"
#include "music_player.h"
#include "lua_base.h"
#include "lua_params.h"


#include "post_inc.h"

/**********************************************/
static int thing_set_field(lua_State *L);
static int thing_get_field(lua_State *L);

static const struct luaL_Reg thing_methods[];


/***************************************************************************************************/
/************    Api Functions    ******************************************************************/
/***************************************************************************************************/

//Setup Commands

static int lua_SET_GENERATE_SPEED(lua_State *L)
{
    GameTurnDelta interval   = luaL_checkinteger(L,1);

    game.generate_speed = saturate_set_unsigned(interval, 16);
    update_dungeon_generation_speeds();
    return 0;
}

static int lua_COMPUTER_PLAYER(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L,1);
    if (lua_isnumber(L, 2))
    {
        long attitude       = luaL_checkint(L,2);
        script_support_setup_player_as_computer_keeper(player_idx, attitude);
        return 0;
    }

    const char* comp_model = luaL_checkstring(L,2);

    if(strcasecmp(comp_model,"ROAMING") == 0)
    {

        struct PlayerInfo* player = get_player(player_idx);
        player->player_type = PT_Roaming;
        player->allocflags |= PlaF_Allocated;
        player->allocflags |= PlaF_CompCtrl;
        player->id_number = player_idx;
        return 0;
        
    }
    else
    {
        luaL_error(L,"invalid COMPUTER_PLAYER param '%s'",comp_model);
        return 0;
    }
}

static int lua_ALLY_PLAYERS(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    PlayerNumber player_idx = luaL_checkPlayerSingle(L,2);
    uchar state = luaL_checkinteger(L, 3);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        set_ally_with_player(i, player_idx,   (state & 1) ? true : false);
        set_ally_with_player(player_idx, i,   (state & 1) ? true : false);
        set_player_ally_locked(i, player_idx, (state & 2) ? true : false);
        set_player_ally_locked(player_idx, i, (state & 2) ? true : false);
    }
    return 0;
}

static int lua_START_MONEY(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    GoldAmount gold_val = luaL_checkinteger(L, 2);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
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

static int lua_MAX_CREATURES(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long max_amount                 = luaL_checkinteger(L, 2);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        SYNCDBG(4,"Setting player %d max attracted creatures to %d.",(int)i,(int)max_amount);
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        dungeon->max_creatures_attracted = max_amount;
    }
    return 0;
}

static int lua_ADD_CREATURE_TO_POOL(lua_State *L)
{
    long crtr_model = luaL_checkNamedCommand(L,1,creature_desc);
    long amount     = luaL_checkinteger(L, 2);

    add_creature_to_pool(crtr_model, amount);
    return 0;
}

static int lua_CREATURE_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long cr_kind                    = luaL_checkNamedCommand(L,2,creature_desc);
    TbBool can_be_attracted         = lua_toboolean(L, 3);
    long amount_forced              = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        if (!set_creature_available(i,cr_kind,can_be_attracted,amount_forced))
            WARNLOG("Setting creature %s availability for player %d failed.",creature_code_name(cr_kind),(int)i);
    }
    return 0;
}

static int lua_DEAD_CREATURES_RETURN_TO_POOL(lua_State *L)
{
    TbBool return_to_pool         = lua_toboolean(L, 3);
    set_flag_value(game.flags_cd, MFlg_DeadBackToPool, return_to_pool);
    return 0;
}

static int lua_ROOM_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long rkind                      = luaL_checkNamedCommand(L,2,room_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    TbBool is_available             = lua_toboolean(L, 4);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        set_room_available(i,rkind,can_be_available,is_available);
    }
    return 0;
}

static int lua_MAGIC_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long power                      = luaL_checkNamedCommand(L,2,power_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    TbBool is_available             = lua_toboolean(L, 4);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        set_power_available(i,power,can_be_available,is_available);
    }
    return 0;
}

static int lua_DOOR_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long door_type                  = luaL_checkNamedCommand(L,2,door_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    long number_available           = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        set_door_buildable_and_add_to_amount(i, door_type, can_be_available, number_available);
    }
    return 0;
}

static int lua_TRAP_AVAILABLE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long trap_type                  = luaL_checkNamedCommand(L,2,trap_desc);
    TbBool can_be_available         = lua_toboolean(L, 3);
    long number_available           = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        set_trap_buildable_and_add_to_amount(i, trap_type, can_be_available, number_available);
    }
    return 0;
}




//Script flow control

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


    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
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

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        struct PlayerInfo *player = get_player(i);
        set_player_as_lost_level(player);
    }
    return 0;
}

static int lua_COUNT_CREATURES_AT_ACTION_POINT(lua_State *L)
{
    ActionPointId ap_idx = luaL_checkActionPoint(L, 1);
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 2);
    long crtr_model = luaL_checkNamedCommand(L,3,creature_desc);

    long sum = 0;
    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        sum += count_player_creatures_of_model_in_action_point(i, crtr_model, ap_idx);
    }
    lua_pushinteger(L, sum);
    return 1;
}

static int lua_SET_TIMER(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long timr_id              = luaL_checkNamedCommand(L,2,timer_desc);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        restart_script_timer(i, timr_id);
    }
    return 0;
}

static int lua_ADD_TO_TIMER(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long timr_id              = luaL_checkNamedCommand(L,2,timer_desc);
    long amount               = luaL_checkinteger(L, 3);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        add_to_script_timer(i, timr_id, amount);
    }
    return 0;
}
static int lua_DISPLAY_TIMER(lua_State *L)
{
    PlayerNumber player_id = luaL_checkPlayerSingle(L, 1);
    long timr_id              = luaL_checkNamedCommand(L,2,timer_desc);
    long display              = luaL_checkinteger(L, 3);

    gameadd.script_player = player_id;
    gameadd.script_timer_id = timr_id;
    gameadd.script_timer_limit = 0;
    gameadd.timer_real = display;
    game.flags_gui |= GGUI_ScriptTimer;
    return 0;
}

static int lua_HIDE_TIMER(lua_State *L)
{
    game.flags_gui &= ~GGUI_ScriptTimer;
    return 0;
}
static int lua_BONUS_LEVEL_TIME(lua_State *L)
{
    GameTurn turns = luaL_checkinteger(L, 1);
    TbBool clocktime = lua_toboolean(L, 2);
    if (turns > 0)
    {
        game.bonus_time = game.play_gameturn + turns;
        set_flag(game.flags_gui, GGUI_CountdownTimer);
    }
    else
    {
        game.bonus_time = 0;
        clear_flag(game.flags_gui, GGUI_CountdownTimer);
    }
    gameadd.timer_real = clocktime;
    return 0;
}
static int lua_ADD_BONUS_TIME(lua_State *L)
{
    GameTurnDelta turns = luaL_checkinteger(L, 1);
    game.bonus_time += turns;
    return 0;
}


//Adding New Creatures and Parties to the Level

static int lua_ADD_CREATURE_TO_LEVEL(lua_State *L)
{
    PlayerNumber plr_idx   = luaL_checkPlayerSingle(L, 1);
    long crtr_id           = luaL_checkNamedCommand(L,2,creature_desc);
    TbMapLocation location = luaL_checkLocation(L,  3);
    long ncopies           = luaL_checkinteger(L, 4);
    long crtr_level        = luaL_checkinteger(L, 5);
    long carried_gold      = luaL_checkinteger(L, 6);
    long spawn_type        = SpwnT_Default;

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
        lua_pushThing(L,script_create_new_creature(plr_idx, crtr_id, location, carried_gold, crtr_level-1,spawn_type));
    }
    return ncopies;
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

static int lua_CREATE_PARTY(lua_State *L)
{
    const char* party_name = luaL_checklstring(L, 1,NULL);
    create_party(party_name);
    return 0;
}
static int lua_ADD_TO_PARTY(lua_State *L)
{
    long party_id          = luaL_checkParty(L,  1);
    long crtr_id           = luaL_checkNamedCommand(L,2,creature_desc);
    long experience        = luaL_checklong(L, 3);
    long gold              = luaL_checklong(L, 4);
    long objective_id      = luaL_checkNamedCommand(L, 5,hero_objective_desc);
    long countdown         = luaL_checklong (L, 6);


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
    long party_id          = luaL_checkParty(L,  1);
    const char* creature   = lua_tostring(L,  2);
    long experience  = lua_tointeger(L, 3);

    long creature_id = get_rid(creature_desc, creature);
    if (creature_id == -1)
    {
      SCRPTERRLOG("Unknown creature, '%s'", creature);
      return 0;
    }

    delete_member_from_party(party_id, creature_id, experience);
    return 0;
}

static int lua_ADD_TUNNELLER_PARTY_TO_LEVEL(lua_State *L)
{
    PlayerNumber owner           = luaL_checkPlayerSingle(L, 1);
    long prty_id                 = luaL_checkParty(L,  2);
    TbMapLocation spawn_location = luaL_checkLocation(L,  3);
    TbMapLocation head_for       = luaL_checkHeadingLocation(L,4); // checks 2 params
    //long target                  = luaL_checkinteger(L, 6);//todo check why this is unused
    long crtr_level              = luaL_checkCrtLevel(L, 7);
    GoldAmount carried_gold      = luaL_checkinteger(L, 8);


    struct Party* party = &gameadd.script.creature_partys[prty_id];
    if (party->members_num >= GROUP_MEMBERS_COUNT-1)
    {
        SCRPTERRLOG("Party too big for ADD_TUNNELLER (Max %d members)", GROUP_MEMBERS_COUNT-1);
        return 0;
    }

    script_process_new_tunneller_party(owner, prty_id, spawn_location, head_for, crtr_level, carried_gold);
    return 0;
}

static int lua_ADD_PARTY_TO_LEVEL(lua_State *L)
{
    PlayerNumber owner     = luaL_checkPlayerSingle(L, 1);
    long prty_id           = luaL_checkParty(L,  2);
    TbMapLocation location = luaL_checkLocation(L,  3);
    long ncopies           = luaL_checkinteger(L, 4);

    if (ncopies < 1)
    {
        SCRPTERRLOG("Invalid NUMBER parameter");
        return 0;
    }

    // Recognize place where party is created
    if (location == 0)
        return 0;
    struct Party* party = &gameadd.script.creature_partys[prty_id];
    script_process_new_party(party, owner, location, ncopies);
        return 0;
}

//Displaying information and affecting interface

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

//static int lua_QUICK_OBJECTIVE(lua_State *L)
//static int lua_QUICK_INFORMATION(lua_State *L)
//static int lua_QUICK_OBJECTIVE_WITH_POS(lua_State *L)
//static int lua_QUICK_INFORMATION_WITH_POS(lua_State *L)
//static int lua_DISPLAY_MESSAGE(lua_State *L)
//static int lua_QUICK_MESSAGE(lua_State *L)
//static int lua_HEART_LOST_OBJECTIVE(lua_State *L)
//static int lua_HEART_LOST_QUICK_OBJECTIVE(lua_State *L)
//static int lua_PLAY_MESSAGE(lua_State *L)
static int lua_TUTORIAL_FLASH_BUTTON(lua_State *L)
{
    long          button    = luaL_checkinteger(L, 1);
    GameTurnDelta gameturns = luaL_checkinteger(L, 2);

    gui_set_button_flashing(button,gameturns);
    return 0;
}

static int lua_DISPLAY_COUNTDOWN(lua_State *L)
{
    PlayerNumber player   = luaL_checkPlayerSingle(L, 1);
    int timer = luaL_checkNamedCommand(L,2,timer_desc);
    int target = luaL_checkinteger(L,3);
    int clocktime = lua_toboolean(L,4);

    gameadd.script_player = player;
    gameadd.script_timer_id = timer;
    gameadd.script_timer_limit = target;
    gameadd.timer_real = clocktime;
    game.flags_gui |= GGUI_ScriptTimer;
    return 0;    
}

static int lua_DISPLAY_VARIABLE(lua_State *L)
{
    PlayerNumber player   = luaL_checkPlayerSingle(L, 1);
    int variable = luaL_checkinteger(L,2);
    int target = luaL_checkinteger(L,3);

    gameadd.script_player = player;
    gameadd.script_value_type = variable;
    gameadd.script_value_id = target;
    game.flags_gui |= GGUI_Variable;
    return 0;
}

static int lua_HIDE_VARIABLE(lua_State *L)
{
    game.flags_gui &= ~GGUI_Variable;
    return 0;
}

//Manipulating Map

static int lua_REVEAL_MAP_LOCATION(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    TbMapLocation target = luaL_checkLocation(L,  2);
    MapSubtlDelta range = luaL_checkinteger(L, 3);


    SYNCDBG(0, "Revealing location type %lu", target);
    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        long x = 0;
        long y = 0;
        find_map_location_coords(target, &x, &y, i, __func__);
        if ((x == 0) && (y == 0))
        {
            WARNLOG("Can't decode location %lu", target);
            return 0;
        }
        if (range == -1)
        {
            struct CompoundCoordFilterParam iter_param;
            iter_param.plyr_idx = i;
            slabs_fill_iterate_from_slab(subtile_slab(x), subtile_slab(y), slabs_reveal_slab_and_corners, &iter_param);
        } else
            player_reveal_map_area(i, x, y,range,range);
    }
    return 0;

}
static int lua_REVEAL_MAP_RECT(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    MapSubtlCoord stl_x    = luaL_checkstl_x(L, 2);
    MapSubtlCoord stl_y    = luaL_checkstl_y(L, 3);
    MapSubtlDelta width    = lua_tointeger(L,4);
    MapSubtlDelta height   = lua_tointeger(L,5);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        player_reveal_map_area(i,stl_x,stl_y,width,height);
    }
    return 0;
}
static int lua_CONCEAL_MAP_RECT(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    MapSubtlCoord stl_x    = luaL_checkstl_x(L, 2);
    MapSubtlCoord stl_y    = luaL_checkstl_y(L, 3);
    MapSubtlDelta width    = lua_tointeger(L,4);
    MapSubtlDelta height   = lua_tointeger(L,5);
    TbBool conceal_all     = lua_toboolean(L,6);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        player_conceal_map_area(i,stl_x,stl_y,width,height,conceal_all);
    }
    return 0;
}

static int lua_SET_DOOR(lua_State *L)
{
    long doorAction = luaL_checkNamedCommand(L,1,locked_desc);
    MapSlabCoord slb_x = luaL_checkslb_x(L,2);
    MapSlabCoord slb_y = luaL_checkslb_y(L,3);

    struct Thing* doortng = get_door_for_position(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    if (!thing_is_invalid(doortng))
    {
        switch (doorAction)
        {
        case 0:
            unlock_door(doortng);
            break;
        case 1:
            lock_door(doortng);
            break;
        }
    }
    return 0;
}

static int lua_ADD_OBJECT_TO_LEVEL(lua_State *L)
{
    long obj_id            = luaL_checkNamedCommand(L,1,object_desc);
    MapSubtlCoord stl_x    = luaL_checkstl_x(L, 2);
    MapSubtlCoord stl_y    = luaL_checkstl_y(L, 3);
    long arg               = lua_tointeger(L,4);
    PlayerNumber plr_idx   = luaL_checkPlayerSingle(L, 5);
    short angle            = lua_tointeger(L, 6);

    lua_pushThing(L,script_process_new_object(obj_id, stl_x, stl_y, arg, plr_idx,angle));
    return 1;
}
static int lua_ADD_EFFECT_GENERATOR_TO_LEVEL(lua_State *L)
{
    ThingModel gen_id      = luaL_checkNamedCommand(L,1,effectgen_desc);
    TbMapLocation location = luaL_checkLocation(L,  2);
    long range             = luaL_checkinteger(L, 3);

    lua_pushThing(L,script_process_new_effectgen(gen_id, location, range));
    return 1;
}

static int lua_PLACE_DOOR(lua_State *L)
{
    PlayerNumber plyridx = luaL_checkPlayerSingle(L,1);
    long doorkind = luaL_checkNamedCommand(L,2,door_desc);
    MapSlabCoord slb_x = luaL_checkslb_x(L,3);
    MapSlabCoord slb_y = luaL_checkslb_y(L,4);
    TbBool locked = lua_toboolean(L,5);
    TbBool free = lua_toboolean(L,6);

    script_place_door(plyridx, doorkind, slb_x, slb_y, locked, free);
}

static int lua_PLACE_TRAP(lua_State *L)
{
    PlayerNumber plyridx = luaL_checkPlayerSingle(L,1);
    long trapkind = luaL_checkNamedCommand(L,2,trap_desc);
    MapSubtlCoord stl_x = luaL_checkstl_x(L,3);
    MapSubtlCoord stl_y = luaL_checkstl_y(L,4);
    TbBool free = lua_toboolean(L,5);

    script_place_trap(plyridx, trapkind, stl_x, stl_y, free);
}



//Manipulating Configs
/*
static int lua_SET_GAME_RULE(lua_State *L)
{
    const char *ruledesc = lua_tostring(L, 1);
    long rulevalue = luaL_checkinteger(L, 2);

    long rulegroup = 0;

    long ruledesc_id = get_id(special_game_rules_desc, ruledesc);
    if (ruledesc_id != -1)
    {
        rulegroup = -1;
        switch (ruledesc)
        {
        case 1: // PreserveClassicBugs
            // this one is a special case because in the cfg it's not done trough number
            if ((ruleval < 0) || (ruleval >= ClscBug_ListEnd))
            {
                SCRPTERRLOG("Game Rule '%s' value %ld out of range", scline->tp[0], ruleval);
                return;
            }
            game.conf.rules.game.classic_bugs_flags = rulevalue;
            break;
        case 2: // AlliesShareVision
            // this one is a special case because it updates minimap
            SCRIPTDBG(7, "Changing Game Rule '%s' from %d to %ld", rulename, game.conf.rules.game.allies_share_vision, rulevalue);
            game.conf.rules.game.allies_share_vision = (TbBool)rulevalue;
            panel_map_update(0, 0, gameadd.map_subtiles_x + 1, gameadd.map_subtiles_y + 1);
            break;
        case 3: // MapCreatureLimit
            // this one is a special case because it needs to kill of additional creatures
            SCRIPTDBG(7, "Changing Game Rule '%s' from %u to %ld", rulename, game.conf.rules.game.creatures_count, rulevalue);
            game.conf.rules.game.creatures_count = rulevalue;
            short count = setup_excess_creatures_to_leave_or_die(game.conf.rules.game.creatures_count);
            if (count > 0)
            {
                SCRPTLOG("Map creature limit reduced, causing %d creatures to leave or die", count);
            }
            break;
        default:
            WARNMSG("Unsupported Game Rule, command %d.", ruledesc);
            break;
        }
        switch (ruledesc_id)
        {
        case 1: // PreserveClassicBugs
                // this one is a special case because in the cfg it's not done trough number
        }
    }
    else
    {
        for (size_t i = 0; i < sizeof(ruleblocks) / sizeof(ruleblocks[0]); i++)
        {
            ruledesc = get_named_field_id(ruleblocks[i], scline->tp[0]);
            if (ruledesc != -1)
            {
                rulegroup = i;
                if (ruleval < (ruleblocks[i] + ruledesc)->min)
                {
                    ruleval = (ruleblocks[i] + ruledesc)->min;
                    SCRPTERRLOG("Game Rule '%s' value %ld is smaller then minimum of %I64d", scline->tp[0], ruleval, (ruleblocks[i] + ruledesc)->min);
                }
                else if (ruleval > (ruleblocks[i] + ruledesc)->max)
                {
                    ruleval = (ruleblocks[i] + ruledesc)->max;
                    SCRPTERRLOG("Game Rule '%s' value %ld is bigger then maximum of %I64d", scline->tp[0], ruleval, (ruleblocks[i] + ruledesc)->max);
                }
                break;
                assign_named_field_value((ruleblocks[i] + ruledesc), rulevalue);
            }
        }
    }

    if (ruledesc == -1)
    {
        luaL_argerror(L, 2, "Unknown Game Rule");
        return 0;
    }
}

*/
/*
// SET_HAND_RULE([player],[creature],[rule_slot],[rule_action],[rule]*,[param]*)
static int lua_SET_HAND_RULE(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    long crtr_id = luaL_checkNamedCommand(L,2,creature_desc);
    long rule_slot = luaL_checkinteger(L, 3);
    long rule_action = luaL_checkNamedCommand(L,4,rule_action_desc);
    long rule = luaL_checkNamedCommand(L,4,rule_action_desc);
    long param = luaL_integer(L, 5);
    
    long crtr_id_start = ((crtr_id == CREATURE_ANY) || (crtr_id == CREATURE_NOT_A_DIGGER)) ? 0 : crtr_id;
    long crtr_id_end = ((crtr_id == CREATURE_ANY) || (crtr_id == CREATURE_NOT_A_DIGGER)) ? CREATURE_TYPES_MAX : crtr_id + 1;

    struct Dungeon* dungeon;
    for (int plyr_idx = context->plr_start; plyr_idx < context->plr_end; plyr_idx++)
    {
        for (int ci = crtr_id_start; ci < crtr_id_end; ci++)
        {

            //todo maybe should use creature_model_matches_model somewhere?
            if (crtr_id == CREATURE_NOT_A_DIGGER)
            {
                if (creature_kind_is_for_dungeon_diggers_list(plyr_idx,ci))
                {
                    continue;
                }
            }
            dungeon = get_dungeon(plyr_idx);
            if (rule_action == HandRuleAction_Allow || rule_action == HandRuleAction_Deny)
            {
                dungeon->hand_rules[ci][rule_slot].enabled = 1;
                dungeon->hand_rules[ci][rule_slot].type = rule_action;
                dungeon->hand_rules[ci][rule_slot].allow = rule_action;
                dungeon->hand_rules[ci][rule_slot].param = param;
            } else
            {
                dungeon->hand_rules[ci][rule_slot].enabled = rule_action == HandRuleAction_Enable;
            }
        }
    }

}
*/

//static int lua_SET_DOOR_CONFIGURATION(lua_State *L)
//static int lua_SET_OBJECT_CONFIGURATION(lua_State *L)
//static int lua_SET_TRAP_CONFIGURATION(lua_State *L)
//static int lua_SET_CREATURE_CONFIGURATION(lua_State *L)
//static int lua_SET_EFFECT_GENERATOR_CONFIGURATION(lua_State *L)
//static int lua_SET_POWER_CONFIGURATION(lua_State *L)


static int lua_SWAP_CREATURE(lua_State *L)
{
    long crtr_id1 = luaL_checkNamedCommand(L,1,creature_desc);
    long crtr_id2 = luaL_checkNamedCommand(L,2,creature_desc);

    swap_creature(crtr_id1, crtr_id2);
    return 0;
}

static int lua_SET_SACRIFICE_RECIPE(lua_State *L)
{

    int command = luaL_checkNamedCommand(L,1,rules_sacrifices_commands);
    const char * reward_str = luaL_checkstring(L,1);
    int reward =  0;
    ThingModel victims[MAX_SACRIFICE_VICTIMS];
    for (int i = 0; i < MAX_SACRIFICE_VICTIMS; i++)
    {
        long crtr_model  = luaL_optNamedCommand(L,i + 1,creature_desc);
        victims[i] = crtr_model;
    }


    if ((command == SacA_CustomPunish) || (command == SacA_CustomReward))
    {
        reward = get_id(flag_desc, reward_str) + 1;
    }
    else
    {
        reward = get_id(creature_desc, reward_str);
        if (reward == -1)
        {
            reward = get_id(sacrifice_unique_desc, reward_str);
        }
        if (reward == -1)
        {
            reward = get_id(spell_desc, reward_str);
        }
    }
    if (reward == -1 && (strcmp(reward_str, "NONE") == 0))
    {
        reward = 0;
    }

    if (reward < 0)
    {
        reward = 0;
        command = SacA_None;
        luaL_error(L,"Unexpected parameter:%s", reward_str);
    }

    script_set_sacrifice_recipe(command, reward, victims, 0);
    return 0;

}

static int lua_REMOVE_SACRIFICE_RECIPE(lua_State *L)
{
    int command = SacA_None;
    int reward =  0;
    ThingModel victims[MAX_SACRIFICE_VICTIMS];

    for (int i = 0; i < MAX_SACRIFICE_VICTIMS; i++)
    {
        long crtr_model  = luaL_optNamedCommand(L,i + 1,creature_desc);
        victims[i] = crtr_model;
    }

    script_set_sacrifice_recipe(command, reward, victims, 0);
    return 0;

}

//Manipulating Creature stats


static int lua_SET_CREATURE_INSTANCE(lua_State *L)
{
    ThingModel crmodel = luaL_checkNamedCommand(L,1,creature_desc);
    int slot = luaL_checkinteger(L, 2);
    int instance = luaL_checkinteger(L, 3);
    unsigned char level = luaL_checkinteger(L, 4);

    script_set_creature_instance(crmodel, slot, instance, level);
    return 0;
}

static int lua_SET_CREATURE_MAX_LEVEL(lua_State *L)
{

    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long crtr_model                 = luaL_checkNamedCommand(L,2,creature_desc);
    long max_level                  = luaL_checkinteger(L, 3);

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        if (max_level == -1)
            max_level = CREATURE_MAX_LEVEL + 1;
        dungeon->creature_max_level[crtr_model%game.conf.crtr_conf.model_count] = max_level;
    }
    return 0;
}
//static int lua_SET_CREATURE_PROPERTY(lua_State *L)
//static int lua_SET_CREATURE_TENDENCIES(lua_State *L)
//static int lua_CREATURE_ENTRANCE_LEVEL(lua_State *L)





//Manipulating Research

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

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
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

    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        if (!research_overriden_for_player(i))
            remove_all_research_from_player(i);
        add_research_to_player(i, research_type, room_or_spell, research_value);
    }
    return 0;
}


//Tweaking computer players

static int lua_COMPUTER_DIG_TO_LOCATION(lua_State *L)
{
    PlayerNumber plr_idx      = luaL_checkPlayerSingle(L, 1);
    TbMapLocation origin      = luaL_checkLocation(L,  2);
    TbMapLocation destination = luaL_checkLocation(L,  3);
    script_computer_dig_to_location(plr_idx, origin, destination);
    return 0;
}

//static int lua_SET_COMPUTER_PROCESS(lua_State *L)
//static int lua_SET_COMPUTER_CHECKS(lua_State *L)
//static int lua_SET_COMPUTER_GLOBALS(lua_State *L)
//static int lua_SET_COMPUTER_EVENT(lua_State *L)


//Specials
//static int lua_USE_SPECIAL_INCREASE_LEVEL(lua_State *L)
//static int lua_USE_SPECIAL_MULTIPLY_CREATURES",
//static int lua_USE_SPECIAL_MAKE_SAFE(lua_State *L)
//static int lua_SET_BOX_TOOLTIP(lua_State *L)
//static int lua_SET_BOX_TOOLTIP_ID(lua_State *L)



//Effects
//static int lua_CREATE_EFFECT(lua_State *L)
//static int lua_CREATE_EFFECT_AT_POS(lua_State *L)
//static int lua_CREATE_EFFECTS_LINE(lua_State *L)








static int lua_SET_MUSIC(lua_State *L)
{
    long track_number                  = luaL_checkinteger(L, 1);

    if (track_number >= FIRST_TRACK && track_number <= max_track)
    {
        game.audiotrack = track_number;
    }
    return 0;
}


static int lua_ZOOM_TO_LOCATION(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    TbMapLocation location = luaL_checkLocation(L,  2);
    struct Coord3d pos;
    
    find_location_pos(location, player_idx, &pos, __func__);
    set_player_zoom_to_position(get_player(player_idx),&pos);

    return 0;
}

static int lua_LOCK_POSSESSION(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    TbBool locked = lua_toboolean(L, 2);

    struct PlayerInfo *player;
    for (PlayerNumber i = player_range.start_idx; i <= player_range.end_idx; i++)
    {
        player = get_player(i);
        if (player_exists(player))
        {
            player->possession_lock = locked;
        }
    }

    return 0;
}

static int lua_SET_DIGGER(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long new_dig_model = luaL_checkNamedCommand(L,2,creature_desc);

    for (PlayerNumber plyr_idx = player_range.start_idx; plyr_idx <= player_range.end_idx; plyr_idx++)
    {
        update_players_special_digger_model(plyr_idx, new_dig_model);
    }
    return 0;
}

/*
static int lua_MESSAGE(lua_State *L)
static int lua_ADD_GOLD_TO_PLAYER(lua_State *L)
static int lua_USE_POWER_ON_CREATURE(lua_State *L)
static int lua_USE_POWER_AT_POS(lua_State *L)
static int lua_USE_POWER_AT_SUBTILE(lua_State *L)
static int lua_USE_POWER_AT_LOCATION(lua_State *L)
static int lua_USE_POWER(lua_State *L)

static int lua_USE_SPECIAL_LOCATE_HIDDEN_WORLD"
static int lua_USE_SPECIAL_TRANSFER_CREATURE(lua_State *L)


static int lua_CHANGE_SLAB_OWNER(lua_State *L)
static int lua_CHANGE_SLAB_TYPE(lua_State *L)
static int lua_USE_SPELL_ON_CREATURE(lua_State *L)
static int lua_HIDE_HERO_GATE(lua_State *L)

*/


/**********************************************/
// game
/**********************************************/


static int lua_get_creature_near(lua_State *L)
{
    MapSubtlCoord stl_x = luaL_checkstl_x(L, 1);
    MapSubtlCoord stl_y = luaL_checkstl_y(L, 2);

    struct Thing *thing = get_creature_near(stl_x * COORD_PER_STL, stl_y * COORD_PER_STL);

    lua_pushThing(L, thing);
    return 1;
}

static int lua_get_thing_by_idx(lua_State *L)
{
    // the arguments lua passes to the C code
    ThingIndex tng_idx = lua_tointeger(L, 1);

    struct Thing *thing = thing_get(tng_idx);

    // arguments you push back to lua
    lua_pushThing(L, thing);
    return 1;
}

static int send_chat_message(lua_State *L)
{
    int plyr_idx = luaL_checkPlayerSingle(L, 1);
    const char *msg = lua_tostring(L, 2);

    char type = MsgType_Player;

    message_add(type,plyr_idx, msg);

    return 0;
}

static int lua_print(lua_State *L)
{
    const char* msg = lua_tostring(L, 1);

    JUSTLOG("%s",msg);
    return 0;
}

static int lua_get_things_of_class(lua_State *L)
{
    ThingClass class_id = luaL_checkNamedCommand(L,1,class_commands);

    const struct StructureList* slist = get_list_for_thing_class(class_id);
    ThingIndex i = slist->index;
    long k = 0;

    lua_newtable(L);

    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code

        lua_pushThing(L, thing);  // Push the value onto the stack
        lua_rawseti(L, -2, k + 1);      // Set table[-2][i + 1] = value
        
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return 1; // return value is the amount of args you push back
}



static const luaL_Reg global_methods[] = {
//Setup Commands
   {"SET_GENERATE_SPEED"            ,lua_SET_GENERATE_SPEED           },
   {"COMPUTER_PLAYER"               ,lua_COMPUTER_PLAYER              },
   {"ALLY_PLAYERS"                  ,lua_ALLY_PLAYERS                 },
   {"START_MONEY"                   ,lua_START_MONEY                  },
   {"MAX_CREATURES"                 ,lua_MAX_CREATURES                },
   {"ADD_CREATURE_TO_POOL"          ,lua_ADD_CREATURE_TO_POOL         },
   {"CREATURE_AVAILABLE"            ,lua_CREATURE_AVAILABLE           },
   {"DEAD_CREATURES_RETURN_TO_POOL" ,lua_DEAD_CREATURES_RETURN_TO_POOL},
   {"ROOM_AVAILABLE"                ,lua_ROOM_AVAILABLE               },
   {"MAGIC_AVAILABLE"               ,lua_MAGIC_AVAILABLE              },
   {"DOOR_AVAILABLE"                ,lua_DOOR_AVAILABLE               },
   {"TRAP_AVAILABLE"                ,lua_TRAP_AVAILABLE               },

//Script flow control
   {"WIN_GAME"                             ,lua_WIN_GAME                        },
   {"LOSE_GAME"                            ,lua_LOSE_GAME                       },
   {"COUNT_CREATURES_AT_ACTION_POINT"      ,lua_COUNT_CREATURES_AT_ACTION_POINT },
   {"SET_TIMER"                            ,lua_SET_TIMER                       },
   {"ADD_TO_TIMER"                         ,lua_ADD_TO_TIMER                    },
   {"DISPLAY_TIMER"                        ,lua_DISPLAY_TIMER                   },
   {"HIDE_TIMER"                           ,lua_HIDE_TIMER                      },
   {"BONUS_LEVEL_TIME"                     ,lua_BONUS_LEVEL_TIME                },
   {"ADD_BONUS_TIME"                       ,lua_ADD_BONUS_TIME                  },

//Adding New Creatures and Parties to the Level
   {"ADD_CREATURE_TO_LEVEL"                ,lua_ADD_CREATURE_TO_LEVEL           },
   {"ADD_TUNNELLER_TO_LEVEL"               ,lua_ADD_TUNNELLER_TO_LEVEL          },
   {"CREATE_PARTY"                         ,lua_CREATE_PARTY                    },
   {"ADD_TO_PARTY"                         ,lua_ADD_TO_PARTY                    },
   {"DELETE_FROM_PARTY"                    ,lua_DELETE_FROM_PARTY               },
   {"ADD_TUNNELLER_PARTY_TO_LEVEL"         ,lua_ADD_TUNNELLER_PARTY_TO_LEVEL    },
   {"ADD_PARTY_TO_LEVEL"                   ,lua_ADD_PARTY_TO_LEVEL              },

//Displaying information and affecting interface
   {"DISPLAY_OBJECTIVE"                    ,lua_DISPLAY_OBJECTIVE               },
   {"DISPLAY_OBJECTIVE_WITH_POS"           ,lua_DISPLAY_OBJECTIVE_WITH_POS      },
   {"DISPLAY_INFORMATION"                  ,lua_DISPLAY_INFORMATION             },
   {"DISPLAY_INFORMATION_WITH_POS"         ,lua_DISPLAY_INFORMATION_WITH_POS    },

   //{"QUICK_OBJECTIVE"                      ,lua_QUICK_OBJECTIVE                 },
   //{"QUICK_OBJECTIVE_WITH_POS"             ,lua_QUICK_OBJECTIVE_WITH_POS        },
   //{"QUICK_INFORMATION"                    ,lua_QUICK_INFORMATION               },
   //{"QUICK_INFORMATION_WITH_POS"           ,lua_QUICK_INFORMATION_WITH_POS      },
   //{"DISPLAY_MESSAGE"                      ,lua_DISPLAY_MESSAGE                 },
   //{"QUICK_MESSAGE"                        ,lua_QUICK_MESSAGE                   },
   //{"HEART_LOST_OBJECTIVE"                 ,lua_HEART_LOST_OBJECTIVE            },
   //{"HEART_LOST_QUICK_OBJECTIVE"           ,lua_HEART_LOST_QUICK_OBJECTIVE      },
   //{"PLAY_MESSAGE"                         ,lua_PLAY_MESSAGE                    },
   {"TUTORIAL_FLASH_BUTTON"                ,lua_TUTORIAL_FLASH_BUTTON           },
   {"DISPLAY_COUNTDOWN"                    ,lua_DISPLAY_COUNTDOWN               },
   {"DISPLAY_VARIABLE"                     ,lua_DISPLAY_VARIABLE                },
   {"HIDE_VARIABLE"                        ,lua_HIDE_VARIABLE                   },

//Manipulating Map
   {"REVEAL_MAP_LOCATION"                  ,lua_REVEAL_MAP_LOCATION             },
   {"REVEAL_MAP_RECT"                      ,lua_REVEAL_MAP_RECT                 },
   {"CONCEAL_MAP_RECT"                     ,lua_CONCEAL_MAP_RECT                },
   {"SET_DOOR"                             ,lua_SET_DOOR                        },
   {"ADD_OBJECT_TO_LEVEL"                  ,lua_ADD_OBJECT_TO_LEVEL             },
   {"ADD_EFFECT_GENERATOR_TO_LEVEL"        ,lua_ADD_EFFECT_GENERATOR_TO_LEVEL   },
   {"PLACE_DOOR"                           ,lua_PLACE_DOOR                      },
   {"PLACE_TRAP"                           ,lua_PLACE_TRAP                      },
   
   

//Manipulating Configs
   //{"SET_GAME_RULE"                        ,lua_SET_GAME_RULE                   },
   //{"SET_HAND_RULE"                        ,lua_SET_HAND_RULE                   },
   //{"SET_DOOR_CONFIGURATION"               ,lua_SET_DOOR_CONFIGURATION          },
   //{"NEW_OBJECT_TYPE"                      ,lua_NEW_OBJECT_TYPE                 },
   //{"SET_OBJECT_CONFIGURATION"             ,lua_SET_OBJECT_CONFIGURATION        },
   //{"NEW_TRAP_TYPE"                        ,lua_NEW_TRAP_TYPE                   },
   //{"SET_TRAP_CONFIGURATION"               ,lua_SET_TRAP_CONFIGURATION          },
   //{"NEW_CREATURE_TYPE"                    ,lua_NEW_CREATURE_TYPE               },
   //{"SET_CREATURE_CONFIGURATION"           ,lua_SET_CREATURE_CONFIGURATION      },
   //{"SET_EFFECT_GENERATOR_CONFIGURATION"   ,lua_SET_EFFECT_GENERATOR_CONFIGURATI},
   //{"SET_POWER_CONFIGURATION"              ,lua_SET_POWER_CONFIGURATION         },
   {"SWAP_CREATURE"                        ,lua_SWAP_CREATURE                   },
   //{"NEW_ROOM_TYPE"                        ,lua_NEW_ROOM_TYPE                   },
   //{"SET_ROOM_CONFIGURATION"               ,lua_SET_ROOM_CONFIGURATION          },
   {"SET_SACRIFICE_RECIPE"                 ,lua_SET_SACRIFICE_RECIPE            },
   {"REMOVE_SACRIFICE_RECIPE"              ,lua_REMOVE_SACRIFICE_RECIPE         },


//Manipulating Creature stats
   {"SET_CREATURE_INSTANCE"                ,lua_SET_CREATURE_INSTANCE           },
   {"SET_CREATURE_MAX_LEVEL"               ,lua_SET_CREATURE_MAX_LEVEL          },
   //{"SET_CREATURE_PROPERTY"                ,lua_SET_CREATURE_PROPERTY           },
   //{"SET_CREATURE_TENDENCIES"              ,lua_SET_CREATURE_TENDENCIES         },
   //{"CREATURE_ENTRANCE_LEVEL"              ,lua_CREATURE_ENTRANCE_LEVEL         },


//Manipulating Research
   {"RESEARCH"                             ,lua_RESEARCH                        },
   {"RESEARCH_ORDER"                       ,lua_RESEARCH_ORDER                  },


//Tweaking computer players
   {"COMPUTER_DIG_TO_LOCATION"             ,lua_COMPUTER_DIG_TO_LOCATION        },
   //{"SET_COMPUTER_PROCESS"                 ,lua_SET_COMPUTER_PROCESS            },
   //{"SET_COMPUTER_CHECKS"                  ,lua_SET_COMPUTER_CHECKS             },
   //{"SET_COMPUTER_GLOBALS"                 ,lua_SET_COMPUTER_GLOBALS            },
   //{"SET_COMPUTER_EVENT"                   ,lua_SET_COMPUTER_EVENT              },
/*
//Specials
   {"USE_SPECIAL_INCREASE_LEVEL"           ,lua_USE_SPECIAL_INCREASE_LEVEL      },
   {"USE_SPECIAL_MULTIPLY_CREATURES"       ,lua_USE_SPECIAL_MULTIPLY_CREATURES  },
   {"USE_SPECIAL_TRANSFER_CREATURE"        ,lua_USE_SPECIAL_TRANSFER_CREATURE   },
   {"SET_BOX_TOOLTIP"                      ,lua_SET_BOX_TOOLTIP                 },
   {"SET_BOX_TOOLTIP_ID"                   ,lua_SET_BOX_TOOLTIP_ID              },

//Effects
   {"CREATE_EFFECT"                        ,lua_CREATE_EFFECT                   },
   {"CREATE_EFFECT_AT_POS"                 ,lua_CREATE_EFFECT_AT_POS            },
   {"CREATE_EFFECTS_LINE"                  ,lua_CREATE_EFFECTS_LINE             },

//Other
   {"USE_POWER"                            ,lua_USE_POWER                       },
   {"USE_POWER_AT_LOCATION"                ,lua_USE_POWER_AT_LOCATION           },
   {"USE_POWER_AT_POS"                     ,lua_USE_POWER_AT_POS                },
   {"USE_POWER_ON_CREATURE"                ,lua_USE_POWER_ON_CREATURE           },
   {"USE_SPELL_ON_CREATURE"                ,lua_USE_SPELL_ON_CREATURE           },
   {"USE_SPELL_ON_PLAYERS_CREATURES"       ,lua_USE_SPELL_ON_PLAYERS_CREATURES  },
   {"USE_POWER_ON_PLAYERS_CREATURES"       ,lua_USE_POWER_ON_PLAYERS_CREATURES  },
   {"LOCATE_HIDDEN_WORLD"                  ,lua_LOCATE_HIDDEN_WORLD             },
   {"MAKE_SAFE"                            ,lua_MAKE_SAFE                       },
   {"MAKE_UNSAFE"                          ,lua_MAKE_UNSAFE                     },
   {"SET_HAND_GRAPHIC"                     ,lua_SET_HAND_GRAPHIC                },
   {"SET_INCREASE_ON_EXPERIENCE"           ,lua_SET_INCREASE_ON_EXPERIENCE      },
*/
   {"SET_MUSIC"                            ,lua_SET_MUSIC                       },
   {"ZOOM_TO_LOCATION"                     ,lua_ZOOM_TO_LOCATION                },
   {"LOCK_POSSESSION"                      ,lua_LOCK_POSSESSION                 },
   {"SET_DIGGER"                           ,lua_SET_DIGGER                      },

//debug stuff
   {"print"           ,lua_print      },

    {"GetCreatureNear", lua_get_creature_near},
    {"SendChatMessage", send_chat_message},
    {"getThingByIdx", lua_get_thing_by_idx},
    {"get_things_of_class", lua_get_things_of_class},
};
/*
static const luaL_Reg game_meta[] = {
    {NULL, NULL}
};
*/
static void global_register(lua_State *L)
{
    //luaL_newlib(L, global_methods);
    for (size_t i = 0; i < (sizeof(global_methods)/sizeof(global_methods[0])); i++)
    {
        lua_register(L, global_methods[i].name, global_methods[i].func);
    }
    lua_setglobal(L, "thing");
}


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
    int value = luaL_checkinteger(L, 3);

    //char* read_only_arr[] =  ["index","creation_turn"];

    if (strcmp(key, "orientation") == 0) {
        thing->move_angle_xy = value;
    } else {
        //luaL_error(L, "Unknown field: %s", key);
    }

    return 0;
}

//static int lua_TRANSFER_CREATURE(lua_State *L)
//static int lua_LEVEL_UP_CREATURE(lua_State *L)
//static int lua_MOVE_CREATURE(lua_State *L)
//static int lua_CHANGE_CREATURE_OWNER(lua_State *L)
//static int lua_CHANGE_CREATURES_ANNOYANCE(lua_State *L)

// Function to get field values
static int thing_get_field(lua_State *L) {

    const char* key = luaL_checkstring(L, 2);

    // Check if the key exists in thing_methods
    for (int i = 0; thing_methods[i].name != NULL; i++) {
        if (strcmp(key, thing_methods[i].name) == 0) {
            // If the key exists in thing_methods, call the corresponding function
            lua_pushcfunction(L, thing_methods[i].func);
            return 1;
        }
    }
    
    struct Thing* thing = luaL_checkThing(L, 1);

    if (strcmp(key, "ThingIndex") == 0) {
        lua_pushinteger(L, thing->index);
    } else if (strcmp(key, "creation_turn") == 0) {
        lua_pushinteger(L, thing->creation_turn);
    } else if (strcmp(key, "model") == 0) {
        lua_pushstring(L,thing_model_only_name(thing->class_id,thing->model));
    } else if (strcmp(key, "owner") == 0) {
        lua_pushPlayer(L, thing->owner);
    } else if (strcmp(key, "pos") == 0) {
        lua_pushPos(L, &thing->mappos);
    } else {
        lua_pushnil(L);
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
    {"MakeThingZombie", make_thing_zombie},
    {"CreatureWalkTo",  lua_creature_walk_to},
    {"KillCreature",    lua_kill_creature},
    {"DeleteThing",     lua_delete_thing},
    
   //{"TRANSFER_CREATURE"                    ,lua_TRANSFER_CREATURE               },
   //{"KILL_CREATURE"                        ,lua_KILL_CREATURE                   },
   //{"LEVEL_UP_CREATURE"                    ,lua_LEVEL_UP_CREATURE               },
   //{"MOVE_CREATURE"                        ,lua_MOVE_CREATURE                   },
   //{"CHANGE_CREATURE_OWNER"                ,lua_CHANGE_CREATURE_OWNER           },
   //{"CHANGE_CREATURES_ANNOYANCE"           ,lua_CHANGE_CREATURES_ANNOYANCE      },
    {NULL, NULL}
};

static const struct luaL_Reg thing_meta[] = {
    {"__tostring", thing_tostring},
    {"__index",    thing_get_field},
    {"__newindex", thing_set_field},
    {"__eq",       thing_eq},
    {NULL, NULL}
};


static int Thing_register(lua_State *L)
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




/**********************************************/
//Player
/**********************************************/

static int player_tostring(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    
    lua_pushstring(L,get_conf_parameter_text(player_desc,player_idx));
    return 1;

}

// Function to set field values
static int player_set_field(lua_State *L) {

    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    const char* key = luaL_checkstring(L, 2);
    int value = luaL_checkinteger(L, 3);

    long variable_type;
    long variable_id;

    if (parse_get_varib(key, &variable_id, &variable_type))
    {
        set_variable(player_idx,variable_type,variable_id,value);
        return 0;
    }

    /*
    JUSTLOG("set key %s",key);

    //char* read_only_arr[] =  ["index","creation_turn"];

    if (strcmp(key, "Orientation") == 0) {
        thing->move_angle_xy = value;
    } else {
        //luaL_error(L, "Unknown field: %s", key);
    }
*/
    return 0;
}

// Function to get field values
static int player_get_field(lua_State *L) {

    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);
    const char* key = luaL_checkstring(L, 2);

    //heart
    if (strcmp(key, "heart") == 0) {
        struct Thing* heartng = get_player_soul_container(plyr_idx);
        lua_pushThing(L, heartng);
        return 1;
    }

    

    long variable_type;
    long variable_id;

    if (parse_get_varib(key, &variable_id, &variable_type))
    {
        lua_pushinteger(L, get_condition_value(plyr_idx, variable_type, variable_id));
        return 1;
    }

/*
    if (strcmp(key, "index") == 0) {
        lua_pushinteger(L, thing->index);
    } else if (strcmp(key, "creation_turn") == 0) {
        lua_pushinteger(L, thing->creation_turn);
    } else if (strcmp(key, "Owner") == 0) {
        lua_pushPlayer(L, thing->owner);


    } else {
        lua_pushnil(L);
    }
*/
    return 1;

}

static int player_eq(lua_State *L) {

    if (!lua_istable(L, 1) || !lua_istable(L, 2)) {
        luaL_error(L, "Expected a table");
        return 1;
    }

    // Get idx field
    lua_getfield(L, 1, "playerId");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'playerId' to be an integer");
        return 1;
    }
    int idx1 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack

    // Get idx field
    lua_getfield(L, 2, "playerId");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Expected 'playerId' to be an integer");
        return 1;
    }
    int idx2 = lua_tointeger(L, -1);
    lua_pop(L, 1);  // Pop the idx value off the stack


    lua_pushboolean(L, idx1 == idx2);
    return 1;
}

static const struct luaL_Reg player_methods[] = {
   //{"SET_TEXTURE"                          ,lua_SET_TEXTURE                     },   
   //{"ADD_GOLD_TO_PLAYER"                   ,lua_ADD_GOLD_TO_PLAYER              },
   //{"SET_PLAYER_COLOR"                     ,lua_SET_PLAYER_COLOR                },
   //{"SET_PLAYER_MODIFIER"                  ,lua_SET_PLAYER_MODIFIER             },
   //{"ADD_TO_PLAYER_MODIFIER"               ,lua_ADD_TO_PLAYER_MODIFIER          },
//static int lua_SET_TEXTURE(lua_State *L)
    {NULL, NULL}
};

static const struct luaL_Reg player_meta[] = {
    {"__tostring", player_tostring},
    {"__index",    player_get_field},
    {"__newindex", player_set_field},
    {"__eq",       player_eq},
    {NULL, NULL}
};

static void Player_register(lua_State *L) {
    // Create a metatable for thing and add it to the registry
    luaL_newmetatable(L, "Player");

    // Set the __index and __newindex metamethods
    luaL_setfuncs(L, player_meta, 0);

    // Create a methods table
    luaL_newlib(L, player_methods);

    // Hide the metatable by setting the __metatable field to nil
    lua_pushliteral(L, "__metatable");
    lua_pushnil(L);
    lua_rawset(L, -3);

    // Pop the metatable from the stack
    lua_pop(L, 1);


    for (PlayerNumber i = 0; i < PLAYERS_COUNT; i++)
    {
        lua_pushPlayer(L,i);
        lua_setglobal(L, get_conf_parameter_text(player_desc,i));
    }
    

}


/**********************************************/
//...
/**********************************************/


void reg_host_functions(lua_State *L)
{
    Player_register(L);
    global_register(L);
    Thing_register(L);
}
