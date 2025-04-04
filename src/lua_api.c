#include "pre_inc.h"

#include "../deps/luajit/src/lua.h"
#include "../deps/luajit/src/lauxlib.h"
#include "../deps/luajit/src/lualib.h"

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

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

static int lua_MAX_CREATURES(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    long max_amount                 = luaL_checkinteger(L, 2);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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
    TbBool can_be_available         = lua_tointeger(L, 3);
    TbBool is_available             = lua_toboolean(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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


    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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
    long crtr_model = luaL_checkCreature_or_creature_wildcard(L,3);

    long sum = 0;
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

    gameadd.script_timer_player = player_id;
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
    long crtr_level        = luaL_checkinteger(L, 4);
    long carried_gold      = luaL_checkinteger(L, 5);
    long spawn_type        = SpwnT_Default;

    if ((crtr_level < 1) || (crtr_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return 0;
    }

    // Recognize place where party is created
    if (location == 0)
        return 0;

    lua_pushThing(L,script_create_new_creature(plr_idx, crtr_id, location, carried_gold, crtr_level-1,spawn_type));
    
    return 1;
}

static int lua_ADD_TUNNELLER_TO_LEVEL(lua_State *L)
{
    PlayerNumber plr_id          = luaL_checkPlayerSingle(L,1);
    TbMapLocation spawn_location = luaL_checkLocation(L,2);
    TbMapLocation head_for       = luaL_checkHeadingLocation(L,3); // checks 2 params
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
    long crtr_level              = luaL_checkCrtLevel(L, 6);
    GoldAmount carried_gold      = luaL_checkinteger(L, 7);


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

    // Recognize place where party is created
    if (location == 0)
        return 0;
    struct Party* party = &gameadd.script.creature_partys[prty_id];
    script_process_new_party(party, owner, location, 1);
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

static int lua_QUICK_OBJECTIVE(lua_State *L)
{
    const char *msg_text = lua_tostring(L, 1);
    TbMapLocation target = luaL_checkLocation(L, 2);

    process_objective(msg_text, target, 0, 0);
    return 0;
}

static int lua_QUICK_INFORMATION(lua_State *L)
{
    long slot = luaL_checkIntMinMax(L, 1, 0,QUICK_MESSAGES_COUNT-1);
    const char *msg_text = lua_tostring(L, 2);
    TbMapLocation target = luaL_checkLocation(L, 3);
    snprintf(gameadd.quick_messages[slot], MESSAGE_TEXT_LEN, "%s", msg_text);

    set_quick_information(slot, target, 0, 0);
    return 0;
}

static int lua_QUICK_OBJECTIVE_WITH_POS(lua_State *L)
{
    const char *msg_text = lua_tostring(L, 1);
    MapSubtlCoord stl_x = luaL_checkstl_x(L, 3);
    MapSubtlCoord stl_y = luaL_checkstl_y(L, 4);

    process_objective(msg_text, 0, stl_x, stl_y);
    return 0;
}

static int lua_QUICK_INFORMATION_WITH_POS(lua_State *L)
{
    long slot = luaL_checkIntMinMax(L, 1, 0,QUICK_MESSAGES_COUNT-1);
    const char *msg_text = lua_tostring(L, 2);
    MapSubtlCoord stl_x = luaL_checkstl_x(L, 3);
    MapSubtlCoord stl_y = luaL_checkstl_y(L, 4);
    snprintf(gameadd.quick_messages[slot], MESSAGE_TEXT_LEN, "%s", msg_text);

    set_quick_information(slot, 0, stl_x, stl_y);
    return 0;
}

static int lua_DISPLAY_MESSAGE(lua_State *L)
{
    int msg_id = luaL_checkinteger(L, 1);
    const char *msg =  get_string(msg_id);
    char id;
    char type;
    luaL_checkMessageIcon(L, 1, &type, &id);

    message_add(type,id, msg);

    return 0;
}

static int lua_QUICK_MESSAGE(lua_State *L)
{
    const char *msg = lua_tostring(L, 1);
    char id;
    char type;
    luaL_checkMessageIcon(L, 2, &type, &id);

    message_add(type,id, msg);

    return 0;
}

static int lua_HEART_LOST_OBJECTIVE(lua_State *L)
{
    long message_id = luaL_checkinteger(L, 1);
    TbMapLocation target = luaL_checkLocation(L, 2);

    gameadd.heart_lost_display_message = true;
    gameadd.heart_lost_quick_message = false;
    gameadd.heart_lost_message_id = message_id;
    gameadd.heart_lost_message_target = target; 
    return 0;
}
static int lua_HEART_LOST_QUICK_OBJECTIVE(lua_State *L)
{
    long slot = luaL_checkIntMinMax(L, 1, 0,QUICK_MESSAGES_COUNT-1);
    const char *msg = lua_tostring(L, 2);
    TbMapLocation target = luaL_checkLocation(L, 3);

    snprintf(gameadd.quick_messages[slot], MESSAGE_TEXT_LEN, "%s", msg);

    gameadd.heart_lost_display_message = true;
    gameadd.heart_lost_quick_message = true;
    gameadd.heart_lost_message_id = slot;
    gameadd.heart_lost_message_target = target; 
    return 0;
}
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

    gameadd.script_timer_player = player;
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

    gameadd.script_variable_player = player;
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
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

static int lua_ADD_HEART_HEALTH(lua_State *L)
{
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L,1);
    HitPoints healthdelta = lua_tointeger(L,2);
    TbBool warn_on_damage = lua_toboolean(L,3);
    
    add_heart_health(plyr_idx,healthdelta,warn_on_damage);
    return 0;
}


static int lua_ADD_OBJECT_TO_LEVEL(lua_State *L)
{
    long obj_id            = luaL_checkNamedCommand(L,1,object_desc);
    TbMapLocation location = luaL_checkLocation(L,  2);
    long arg               = lua_tointeger(L,3);
    PlayerNumber plr_idx   = luaL_checkPlayerSingle(L, 4);
    short angle            = lua_tointeger(L, 5);

    struct Coord3d pos;
    if (!get_coords_at_location(&pos, location,true))
    {
        return 0;
    }
    lua_pushThing(L,script_process_new_object(obj_id, pos.x.stl.num, pos.y.stl.num, arg, plr_idx,angle));
    return 1;
}

static int lua_ADD_OBJECT_TO_LEVEL_AT_POS(lua_State *L)
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
    return 0;
}

static int lua_PLACE_TRAP(lua_State *L)
{
    PlayerNumber plyridx = luaL_checkPlayerSingle(L,1);
    long trapkind = luaL_checkNamedCommand(L,2,trap_desc);
    MapSubtlCoord stl_x = luaL_checkstl_x(L,3);
    MapSubtlCoord stl_y = luaL_checkstl_y(L,4);
    TbBool free = lua_toboolean(L,5);

    script_place_trap(plyridx, trapkind, stl_x, stl_y, free);
    return 0;
}



//Manipulating Configs

static void set_configuration(lua_State *L, const struct NamedFieldSet* named_fields_set)
{

    const char* id_str      = lua_tostring(L, 1);
    const char* property    = lua_tostring(L, 2);
    
    short id = get_id(named_fields_set->names, id_str);
    if (id == -1)
    {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Unknown %s, '%s'", named_fields_set->block_basename, id_str);
        luaL_argerror(L, 1, error_msg);
        return;
    }
    if (id > named_fields_set->max_count)
    {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "'%s%d' is out of range",named_fields_set->block_basename, id);
        luaL_argerror(L, 1, error_msg);
        return;
    }
    
    long property_id = get_named_field_id(named_fields_set->named_fields, property);
    if (property_id == -1)
    {    
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Expected a valid property name, got '%s'",property);
        luaL_argerror(L, 1, error_msg);
        return;
    }
    
    const struct NamedField* field = &named_fields_set->named_fields[property_id];
    
    char concatenated_values[MAX_TEXT_LENGTH];
    if (field->argnum == -1)
    {
        concatenated_values[0] = '\0';
        for (int i = 3; lua_tostring(L, i) != NULL; i++) {
            strncat(concatenated_values, lua_tostring(L, i), sizeof(concatenated_values) - strlen(concatenated_values) - 1);
            if (lua_tostring(L, i + 1) != NULL) {
            strncat(concatenated_values, " ", sizeof(concatenated_values) - strlen(concatenated_values) - 1);
            }
        }
        int64_t value = parse_named_field_value(field, concatenated_values,named_fields_set,id,ccs_Lua);
        assign_named_field_value(&named_fields_set->named_fields[property_id],value,named_fields_set,id, ccs_Lua);
        
    }
    else
    {
        int i = 0;
        while (lua_tostring(L, i + 3) != NULL)
        {    
            if( named_fields_set->named_fields[property_id + i].name == NULL || 
                (strcmp(named_fields_set->named_fields[property_id + i].name, named_fields_set->named_fields[property_id].name) != 0))
            {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "more values then expected for property: '%s' '%s'", property, lua_tostring(L, i + 3));
                luaL_argerror(L, 1, error_msg);
                return;
            }
            int64_t value = parse_named_field_value(&named_fields_set->named_fields[property_id + i], lua_tostring(L, i + 3),named_fields_set,id,ccs_Lua);
            assign_named_field_value(&named_fields_set->named_fields[property_id + i],value,named_fields_set,id, ccs_Lua);
            i++;
        }
    }
}

static int lua_SET_DOOR_CONFIGURATION(lua_State *L)
{
    set_configuration(L, &trapdoor_door_named_fields_set);
    return 0;
}

static int lua_SET_OBJECT_CONFIGURATION(lua_State *L)
{
    set_configuration(L, &objects_named_fields_set);
    return 0;
}

static int lua_SET_TRAP_CONFIGURATION(lua_State *L)
{
    set_configuration(L, &trapdoor_trap_named_fields_set);
    return 0;
}

//static int lua_SET_CREATURE_CONFIGURATION(lua_State *L)
//static int lua_SET_EFFECT_GENERATOR_CONFIGURATION(lua_State *L)
//static int lua_SET_POWER_CONFIGURATION(lua_State *L)

static int lua_SET_ROOM_CONFIGURATION(lua_State *L)
{
    set_configuration(L, &terrain_room_named_fields_set);
    return 0;
}

static int lua_SET_GAME_RULE(lua_State *L)
{

    //TODO implement
    short rulegroup;
    short ruledesc;

    luaL_checkGameRule(L,1,&rulegroup,&ruledesc);
    //long rulevalue = luaL_checkinteger(L, 2);
    //update_game_rule(rulegroup, ruledesc, rulevalue);
    return 0;
}

static int lua_SET_HAND_RULE(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    long crtr_id = luaL_checkNamedCommand(L,2,creature_desc);
    long rule_slot = luaL_checkinteger(L, 3);
    long rule_action = luaL_checkNamedCommand(L,4,rule_action_desc);
    long rule = luaL_checkNamedCommand(L,4,rule_action_desc);
    long param = luaL_checkinteger(L, 5);
    
    script_set_hand_rule(player_idx, crtr_id, rule_action, rule_slot, rule, param);
    return 0;
}

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

    script_set_sacrifice_recipe(command, reward, victims);
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

    script_set_sacrifice_recipe(command, reward, victims);
    return 0;

}

//Manipulating Creature stats


static int lua_SET_CREATURE_INSTANCE(lua_State *L)
{
    ThingModel crmodel = luaL_checkNamedCommand(L,1,creature_desc);
    int slot = luaL_checkinteger(L, 2);
    int instance = luaL_checkNamedCommand(L, 3,instance_desc);
    unsigned char level = luaL_checkinteger(L, 4);

    script_set_creature_instance(crmodel, slot, instance, level);
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
        dungeon->creature_max_level[crtr_model%game.conf.crtr_conf.model_count] = max_level;
    }
    return 0;
}

//static int lua_SET_CREATURE_PROPERTY(lua_State *L)

static int lua_SET_CREATURE_TENDENCIES(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    int tendancy = luaL_checkNamedCommand(L, 2, tendency_desc);
    int val = luaL_checkinteger(L, 3);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct PlayerInfo *player = get_player(i);
        set_creature_tendencies(player, tendancy, val);
        if (is_my_player(player))
        {
            struct Dungeon* dungeon = get_players_dungeon(player);
            game.creatures_tend_imprison = ((dungeon->creature_tendencies & 0x01) != 0);
            game.creatures_tend_flee = ((dungeon->creature_tendencies & 0x02) != 0);
        }
    }
    return 0;
}

static int lua_CREATURE_ENTRANCE_LEVEL(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    unsigned char level = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        dungeon->creature_entrance_level = level - 1;
    }
    return 0;
}



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


//Tweaking computer players

static int lua_COMPUTER_DIG_TO_LOCATION(lua_State *L)
{
    PlayerNumber plr_idx      = luaL_checkPlayerSingle(L, 1);
    TbMapLocation origin      = luaL_checkLocation(L,  2);
    TbMapLocation destination = luaL_checkLocation(L,  3);
    script_computer_dig_to_location(plr_idx, origin, destination);
    return 0;
}

static int lua_SET_COMPUTER_PROCESS(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    const char* procname = luaL_checkstring(L, 2);
    long val1 = luaL_checkinteger(L,3);
    long val2 = luaL_checkinteger(L,4);
    long val3 = luaL_checkinteger(L,5);  
    long val4 = luaL_checkinteger(L,6);
    long val5 = luaL_checkinteger(L,7);
    long n = 0;
    for (long i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct Computer2* comp = get_computer_player(i);
        if (computer_player_invalid(comp)) {
            continue;
        }
        for (long k = 0; k < COMPUTER_PROCESSES_COUNT; k++)
        {
            struct ComputerProcess* cproc = &comp->processes[k];
            if (flag_is_set(cproc->flags, ComProc_Unkn0002))
                break;
            if (strcasecmp(procname, cproc->name) == 0)
            {
                cproc->priority = val1;
                cproc->confval_2 = val2;
                cproc->confval_3 = val3;
                cproc->confval_4 = val4;
                cproc->confval_5 = val5;
                n++;
            }
        }
    }
    return 0;
}

static int lua_SET_COMPUTER_CHECKS(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    const char* chkname = luaL_checkstring(L, 2);
    long val1 = luaL_checkinteger(L,3);
    long val2 = luaL_checkinteger(L,4);
    long val3 = luaL_checkinteger(L,5);  
    long val4 = luaL_checkinteger(L,6);
    long val5 = luaL_checkinteger(L,7);

    long n = 0;
    for (long i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct Computer2* comp = get_computer_player(i);
        if (computer_player_invalid(comp)) {
            continue;
        }
        for (long k = 0; k < COMPUTER_CHECKS_COUNT; k++)
        {
            struct ComputerCheck* ccheck = &comp->checks[k];
            if ((ccheck->flags & ComChk_Unkn0002) != 0)
                break;
            if (strcasecmp(chkname, ccheck->name) == 0)
            {
                ccheck->turns_interval = val1;
                ccheck->param1 = val2;
                ccheck->param2 = val3;
                ccheck->param3 = val4;
                ccheck->last_run_turn = val5;
                n++;
            }
        }
    }
    return 0;
}

static int lua_SET_COMPUTER_GLOBALS(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    int dig_stack_size       = luaL_checkinteger(L,2);
    int processes_time       = luaL_checkinteger(L,3);
    int click_rate           = luaL_checkinteger(L,4);
    int max_room_build_tasks = luaL_checkinteger(L,5);
    int turn_begin           = luaL_checkinteger(L,6);
    int sim_before_dig       = luaL_checkinteger(L,7);
    int min_drop_delay       = luaL_checkinteger(L,8);

    for (long i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct Computer2* comp = get_computer_player(i);
        if (computer_player_invalid(comp))
        {
            continue;
        }
        comp->dig_stack_size = dig_stack_size;
        comp->processes_time = processes_time;
        comp->click_rate = click_rate;
        comp->max_room_build_tasks = max_room_build_tasks;
        comp->turn_begin = turn_begin;
        comp->sim_before_dig = sim_before_dig;
        if (min_drop_delay != -1)
        {
            comp->task_delay = min_drop_delay;
        }
    }
    return 0;
}

static int lua_SET_COMPUTER_EVENT(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    const char* evntname = luaL_checkstring(L, 2);
    long val1 = luaL_checkinteger(L,3);
    long val2 = luaL_checkinteger(L,4);
    long val3 = luaL_checkinteger(L,5);  
    long val4 = luaL_checkinteger(L,6);
    long val5 = luaL_checkinteger(L,7);

    long n = 0;
    for (long i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        struct Computer2* comp = get_computer_player(i);
        if (computer_player_invalid(comp)) {
            continue;
        }
        for (long k = 0; k < COMPUTER_EVENTS_COUNT; k++)
        {
            struct ComputerEvent* event = &comp->events[k];
            if (strcasecmp(evntname, event->name) == 0)
            {
                event->test_interval = val1;
                event->param1 = val2;
                event->param2 = val3;
                event->param3 = val4;
                event->last_test_gameturn = val5;
                n++;
            }
        }
    }
    return 0;
}


//Specials
static int lua_USE_SPECIAL_INCREASE_LEVEL(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    char count;
    if(lua_isnone(L,2))
    {
        count = 1;
    }
    else
    {
        count = luaL_checkinteger(L, 2);
    }

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        script_use_special_increase_level(i,count);
    }
    return 0;
}

static int lua_USE_SPECIAL_MULTIPLY_CREATURES(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        script_use_special_multiply_creatures(i);
    }
    return 0;
}

static int lua_MAKE_SAFE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        script_make_safe(i);
    }
    return 0;
}

static int lua_MAKE_UNSAFE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        script_make_unsafe(i);
    }
    return 0;
}

static int lua_SET_BOX_TOOLTIP(lua_State *L)
{
    long box_id = luaL_checkinteger(L, 1);
    const char* tooltip = luaL_checkstring(L, 2);

    snprintf(gameadd.box_tooltip[box_id], MESSAGE_TEXT_LEN, "%s", tooltip);
    return 0;
}
static int lua_SET_BOX_TOOLTIP_ID(lua_State *L)
{
    long box_id = luaL_checkinteger(L, 1);
    long tooltip_id = luaL_checkinteger(L, 2);

    snprintf(gameadd.box_tooltip[box_id], MESSAGE_TEXT_LEN, "%s", get_string(tooltip_id));
    return 0;
}

static int lua_LOCATE_HIDDEN_WORLD(lua_State *L)
{
    script_locate_hidden_world();
    return 0;
}

//Effects
static int lua_CREATE_EFFECT(lua_State *L)
{
    EffectOrEffElModel effect_id = luaL_checkEffectOrEffElModel(L,1);
    TbMapLocation location = luaL_checkLocation(L,  2);
    long height = luaL_checkinteger(L, 3);

    struct Coord3d pos;
    if (!get_coords_at_location(&pos, location,true))
    {
        return 0;
    }

    lua_pushThing(L,script_create_effect(&pos, effect_id, height));
    return 1;
}

static int lua_CREATE_EFFECT_AT_POS(lua_State *L)
{
    EffectOrEffElModel effect_id = luaL_checkEffectOrEffElModel(L,1);
    MapSubtlCoord stl_x = luaL_checkstl_x(L, 2);
    MapSubtlCoord stl_y = luaL_checkstl_y(L, 3);
    long height = luaL_checkinteger(L, 4);

    struct Coord3d pos;
    set_coords_to_subtile_center(&pos, stl_x, stl_y, 0);
    pos.z.val += get_floor_height(pos.x.stl.num, pos.y.stl.num);
    lua_pushThing(L,script_create_effect(&pos, effect_id, height));
    return 1;
}

static int lua_CREATE_EFFECTS_LINE(lua_State *L)
{
    TbMapLocation from = luaL_checkLocation(L,  1);
    TbMapLocation to   = luaL_checkLocation(L,  2);
    char curvature = luaL_checkinteger(L, 3);
    unsigned char spatial_stepping = luaL_checkinteger(L, 4);
    unsigned char temporal_stepping = luaL_checkinteger(L, 5);
    EffectOrEffElModel effct_id = luaL_checkEffectOrEffElModel(L,6);

    create_effects_line(from, to, curvature, spatial_stepping, temporal_stepping, effct_id);
    return 0;
}



static int lua_SET_MUSIC(lua_State *L)
{
    if (lua_isnumber(L, 1)) {
        
        long track_number = luaL_checkinteger(L, 1);
        if (track_number == 0) {
            stop_music();
        }
        else {
            play_music_track(track_number);
        }
    } else {
        const char *track_name = luaL_checkstring(L, 1);
        play_music(prepare_file_fmtpath(FGrp_CmpgMedia, "%s", track_name));
    }
    return 0;
}

static int lua_SET_HAND_GRAPHIC(lua_State *L)
{
    PlayerNumber player_idx = luaL_checkPlayerSingle(L, 1);
    long hand_idx = luaL_checkNamedCommand(L,1,powerhand_desc);

    struct PlayerInfo * player = get_player(player_idx);
    player->hand_idx = hand_idx;
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
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
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

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        update_players_special_digger_model(i, new_dig_model);
    }
    return 0;
}

static int lua_USE_POWER_ON_CREATURE(lua_State *L)
{
    struct Thing *thing = luaL_checkThing(L, 1);
    long pwkind = luaL_checkNamedCommand(L,2,power_desc);
    long power_level = luaL_checkinteger(L, 3);
    PlayerNumber caster = luaL_checkPlayerSingle(L, 4);
    TbBool is_free = lua_toboolean(L, 5);

    script_use_power_on_creature(thing, pwkind, power_level, caster, is_free);
    return 0;
}

static int lua_USE_POWER_AT_POS(lua_State *L)
{
    PlayerNumber caster = luaL_checkPlayerSingle(L, 1);
    MapSubtlCoord stl_x = luaL_checkstl_x(L, 2);
    MapSubtlCoord stl_y = luaL_checkstl_y(L, 3);
    PowerKind pwkind = luaL_checkNamedCommand(L,4,power_desc);
    KeepPwrLevel power_level = luaL_checkinteger(L, 5);
    TbBool is_free = lua_toboolean(L, 6);

    unsigned long allow_flags = PwCast_AllGround | PwCast_Unrevealed;
    unsigned long mod_flags = 0;
    if (is_free)
        set_flag(mod_flags,PwMod_CastForFree);

    magic_use_power_on_subtile(caster, pwkind, power_level, stl_x, stl_y, allow_flags, mod_flags);
    return 0;
}

static int lua_USE_POWER_AT_LOCATION(lua_State *L)
{
    PlayerNumber caster = luaL_checkPlayerSingle(L, 1);
    TbMapLocation location = luaL_checkLocation(L, 2);
    PowerKind pwkind = luaL_checkNamedCommand(L,3,power_desc);
    KeepPwrLevel power_level = luaL_checkinteger(L, 4);
    TbBool is_free = lua_toboolean(L, 5);

    MapSubtlCoord stl_x = 0;
    MapSubtlCoord stl_y = 0;
    find_map_location_coords(location, &stl_x, &stl_y, caster, __func__);

    unsigned long allow_flags = PwCast_AllGround | PwCast_Unrevealed;
    unsigned long mod_flags = 0;
    if (is_free)
        set_flag(mod_flags,PwMod_CastForFree);

    magic_use_power_on_subtile(caster, pwkind, power_level, stl_x, stl_y, allow_flags, mod_flags);
    return 0;
}

static int lua_USE_POWER(lua_State *L)
{
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);
    PowerKind power_kind = luaL_checkNamedCommand(L,2,power_desc);
    TbBool free = lua_toboolean(L, 3);

    script_use_power(plyr_idx,power_kind,free);
    return 0;
}

static int lua_USE_SPECIAL_TRANSFER_CREATURE(lua_State *L)
{
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);
    if (plyr_idx == my_player_number)
    {
        struct Thing *heartng = get_player_soul_container(plyr_idx);
        struct PlayerInfo* player = get_my_player();
        start_transfer_creature(player, heartng);
    }
    return 0;
}

static int lua_CHANGE_SLAB_OWNER(lua_State *L)
{
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);
    MapSlabCoord slb_x = luaL_checkslb_x(L, 2);
    MapSlabCoord slb_y = luaL_checkslb_y(L, 3);

    change_slab_owner_from_script(plyr_idx, slb_x, slb_y);
    return 0;
}

static int lua_CHANGE_SLAB_TYPE(lua_State *L)
{
    MapSlabCoord slb_x = luaL_checkslb_x(L, 1);
    MapSlabCoord slb_y = luaL_checkslb_y(L, 2);
    SlabKind slb_kind = luaL_checkNamedCommand(L, 3,slab_desc);
    int fill_type = luaL_optNamedCommand(L, 4,fill_desc);

    if (fill_type > 0)
    {
        struct CompoundCoordFilterParam iter_param;
        iter_param.num1 = slb_kind;
        iter_param.num2 = fill_type;
        iter_param.num3 = get_slabmap_block(slb_x, slb_y)->kind;
        slabs_fill_iterate_from_slab(slb_x, slb_y, slabs_change_type, &iter_param);
    }
    else
    {
        replace_slab_from_script(slb_x, slb_y,slb_kind);
    }

    return 0;
}

static int lua_USE_SPELL_ON_CREATURE(lua_State *L)
{
    struct Thing *thing = luaL_checkThing(L, 1);
    long spell_id = luaL_checkNamedCommand(L,2,spell_desc);
    int spell_level = luaL_checkinteger(L, 3);

    unsigned long fmcl_bytes = (spell_id << 8) | spell_level;

    script_use_spell_on_creature(thing->owner, thing, fmcl_bytes);
    return 0;
}

static int lua_HIDE_HERO_GATE(lua_State *L)
{
    long gate_number = luaL_checkinteger(L, 1);
    TbBool hide = lua_toboolean(L, 2);

    struct Thing* thing = find_hero_gate_of_number(gate_number);
    if (hide)
    {
        light_turn_light_off(thing->light_id);
        create_effect(&thing->mappos, TngEff_BallPuffWhite, thing->owner);
        place_thing_in_creature_controlled_limbo(thing);
    }
    else
    {
        create_effect(&thing->mappos, TngEff_BallPuffWhite, thing->owner);
        remove_thing_from_creature_controlled_limbo(thing);
        light_turn_light_on(thing->light_id);
    }
    return 0;
}

static int lua_CHANGE_CREATURES_ANNOYANCE(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    ThingModel crmodel = luaL_checkNamedCommand(L,2,creature_desc);
    long operation = luaL_checkNamedCommand(L,3,script_operator_desc);
    long anger = luaL_checkinteger(L, 4);

    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
        script_change_creatures_annoyance(i, crmodel, operation, anger);
    }
    return 0;
}

/*
static int lua_SET_PLAYER_MODIFIER(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
    }
    return 0;
}

static int lua_ADD_TO_PLAYER_MODIFIER(lua_State *L)
{
    struct PlayerRange player_range = luaL_checkPlayerRange(L, 1);
    for (PlayerNumber i = player_range.start_idx; i < player_range.end_idx; i++)
    {
    }
    return 0;
}
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

static int lua_get_creature_by_criterion(lua_State *L)
{
    PlayerNumber plyr_idx = luaL_checkPlayerRangeId(L, 1);
    ThingModel crmodel = luaL_checkCreature_or_creature_wildcard(L,2);
    long criteria = luaL_checkNamedCommand(L,3,creature_select_criteria_desc);

    struct Thing* thing = script_get_creature_by_criteria(plyr_idx, crmodel, criteria);
    lua_pushThing(L, thing);
    return 1;
}


static int lua_print(lua_State *L)
{
    //const char* msg = lua_tostring(L, 1);
    //JUSTLOG("%s",msg);

    int nargs = lua_gettop(L);
    char buffer[16384]; // Adjust size as needed
    int offset = 0;

    for (int i = 1; i <= nargs; i++) {
        size_t len;
        const char *str;

        if (lua_isstring(L, i) || lua_isnumber(L, i)) {
            str = lua_tolstring(L, i, &len);
        } else if (lua_isboolean(L, i)) {
            str = lua_toboolean(L, i) ? "true" : "false";
            len = strlen(str);
        } else if (lua_isnil(L, i)) {
            str = "nil";
            len = 3;
        } else {
            str = luaL_typename(L, i); // Prints type name (e.g., "table", "function")
            len = strlen(str);
        }

        // Ensure we don't overflow the buffer
        if (offset + len + 2 >= sizeof(buffer)) // +2 for "\t" + null terminator
            break;

        memcpy(buffer + offset, str, len);
        offset += len;

        if (i < nargs) {
            buffer[offset++] = '\t'; // Separate arguments with tabs
        }
    }

    buffer[offset] = '\0'; // Null-terminate

    JUSTLOG("%s", buffer); // Print using your logging function

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


static int lua_is_action_point_activated_by_player(lua_State *L)
{
    PlayerNumber plyr_idx = luaL_checkPlayerSingle(L, 1);
    ActionPointId apt_idx = luaL_checkActionPoint(L, 2);

    lua_pushboolean(L, action_point_activated_by_player(apt_idx, plyr_idx));
    return 1;
}

static int lua_run_dkscript_command(lua_State *L)
{
    const char* map_command_const = lua_tostring(L, 1);
    //lua strings are constant, script_scan_line doesn't like const, so copy it to a temp buffer
    char map_command[256];
    strncpy(map_command, map_command_const, sizeof(map_command) - 1);
    map_command[sizeof(map_command) - 1] = '\0'; // Ensure null termination

    script_scan_line(map_command, false, 99);
    return 0;
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
   {"QUICK_OBJECTIVE"                      ,lua_QUICK_OBJECTIVE                 },
   {"QUICK_OBJECTIVE_WITH_POS"             ,lua_QUICK_OBJECTIVE_WITH_POS        },
   {"QUICK_INFORMATION"                    ,lua_QUICK_INFORMATION               },
   {"QUICK_INFORMATION_WITH_POS"           ,lua_QUICK_INFORMATION_WITH_POS      },
   {"DISPLAY_MESSAGE"                      ,lua_DISPLAY_MESSAGE                 },
   {"QUICK_MESSAGE"                        ,lua_QUICK_MESSAGE                   },
   {"HEART_LOST_OBJECTIVE"                 ,lua_HEART_LOST_OBJECTIVE            },
   {"HEART_LOST_QUICK_OBJECTIVE"           ,lua_HEART_LOST_QUICK_OBJECTIVE      },
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
   {"ADD_HEART_HEALTH"                     ,lua_ADD_HEART_HEALTH                },
   {"ADD_OBJECT_TO_LEVEL"                  ,lua_ADD_OBJECT_TO_LEVEL             },
   {"ADD_OBJECT_TO_LEVEL_AT_POS"           ,lua_ADD_OBJECT_TO_LEVEL_AT_POS      },
   {"ADD_EFFECT_GENERATOR_TO_LEVEL"        ,lua_ADD_EFFECT_GENERATOR_TO_LEVEL   },
   {"PLACE_DOOR"                           ,lua_PLACE_DOOR                      },
   {"PLACE_TRAP"                           ,lua_PLACE_TRAP                      },
   {"CHANGE_SLAB_OWNER"                    ,lua_CHANGE_SLAB_OWNER               },
   {"CHANGE_SLAB_TYPE"                     ,lua_CHANGE_SLAB_TYPE                },
   {"HIDE_HERO_GATE"                       ,lua_HIDE_HERO_GATE                  },
   
//Manipulating Configs
    //{"NEW_CREATURE_TYPE"                    ,lua_NEW_CREATURE_TYPE               },
    //{"NEW_OBJECT_TYPE"                      ,lua_NEW_OBJECT_TYPE                 },
    //{"NEW_TRAP_TYPE"                        ,lua_NEW_TRAP_TYPE                   },
    //{"NEW_ROOM_TYPE"                        ,lua_NEW_ROOM_TYPE                   },
    {"SET_DOOR_CONFIGURATION"              ,lua_SET_DOOR_CONFIGURATION          },
    {"SET_OBJECT_CONFIGURATION"            ,lua_SET_OBJECT_CONFIGURATION        },
    {"SET_TRAP_CONFIGURATION"              ,lua_SET_TRAP_CONFIGURATION          },
    //{"SET_CREATURE_CONFIGURATION"           ,lua_SET_CREATURE_CONFIGURATION      },
    //{"SET_EFFECT_GENERATOR_CONFIGURATION"   ,lua_SET_EFFECT_GENERATOR_CONFIGURATI},
    //{"SET_POWER_CONFIGURATION"              ,lua_SET_POWER_CONFIGURATION         },
    {"SET_ROOM_CONFIGURATION"              ,lua_SET_ROOM_CONFIGURATION          },
    {"SET_GAME_RULE"                       ,lua_SET_GAME_RULE                   },
    {"SET_HAND_RULE"                       ,lua_SET_HAND_RULE                   },
    {"SWAP_CREATURE"                       ,lua_SWAP_CREATURE                   },
    {"SET_SACRIFICE_RECIPE"                ,lua_SET_SACRIFICE_RECIPE            },
    {"REMOVE_SACRIFICE_RECIPE"             ,lua_REMOVE_SACRIFICE_RECIPE         },

//Manipulating Creature stats
   {"SET_CREATURE_INSTANCE"                ,lua_SET_CREATURE_INSTANCE           },
   {"SET_CREATURE_MAX_LEVEL"               ,lua_SET_CREATURE_MAX_LEVEL          },
   //{"SET_CREATURE_PROPERTY"                ,lua_SET_CREATURE_PROPERTY           },
   {"SET_CREATURE_TENDENCIES"              ,lua_SET_CREATURE_TENDENCIES         },
   {"CREATURE_ENTRANCE_LEVEL"              ,lua_CREATURE_ENTRANCE_LEVEL         },
   {"CHANGE_CREATURES_ANNOYANCE"           ,lua_CHANGE_CREATURES_ANNOYANCE      },

//Manipulating Research
   {"RESEARCH"                             ,lua_RESEARCH                        },
   {"RESEARCH_ORDER"                       ,lua_RESEARCH_ORDER                  },

//Tweaking computer players
   {"COMPUTER_DIG_TO_LOCATION"             ,lua_COMPUTER_DIG_TO_LOCATION        },
   {"SET_COMPUTER_PROCESS"                 ,lua_SET_COMPUTER_PROCESS            },
   {"SET_COMPUTER_CHECKS"                  ,lua_SET_COMPUTER_CHECKS             },
   {"SET_COMPUTER_GLOBALS"                 ,lua_SET_COMPUTER_GLOBALS            },
   {"SET_COMPUTER_EVENT"                   ,lua_SET_COMPUTER_EVENT              },

//Specials
   {"USE_SPECIAL_INCREASE_LEVEL"           ,lua_USE_SPECIAL_INCREASE_LEVEL      },
   {"USE_SPECIAL_MULTIPLY_CREATURES"       ,lua_USE_SPECIAL_MULTIPLY_CREATURES  },
   {"USE_SPECIAL_TRANSFER_CREATURE"        ,lua_USE_SPECIAL_TRANSFER_CREATURE   },
   {"SET_BOX_TOOLTIP"                      ,lua_SET_BOX_TOOLTIP                 },
   {"SET_BOX_TOOLTIP_ID"                   ,lua_SET_BOX_TOOLTIP_ID              },
   {"MAKE_SAFE"                            ,lua_MAKE_SAFE                       },
   {"MAKE_UNSAFE"                          ,lua_MAKE_UNSAFE                     },
   {"LOCATE_HIDDEN_WORLD"                  ,lua_LOCATE_HIDDEN_WORLD             },

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
    //{"USE_SPELL_ON_PLAYERS_CREATURES"       ,lua_USE_SPELL_ON_PLAYERS_CREATURES  },
    //{"USE_POWER_ON_PLAYERS_CREATURES"       ,lua_USE_POWER_ON_PLAYERS_CREATURES  },
    {"SET_HAND_GRAPHIC"                     ,lua_SET_HAND_GRAPHIC                },
    //{"SET_INCREASE_ON_EXPERIENCE"           ,lua_SET_INCREASE_ON_EXPERIENCE      },
    {"SET_MUSIC"                            ,lua_SET_MUSIC                       },
    {"ZOOM_TO_LOCATION"                     ,lua_ZOOM_TO_LOCATION                },
    {"LOCK_POSSESSION"                      ,lua_LOCK_POSSESSION                 },
    {"SET_DIGGER"                           ,lua_SET_DIGGER                      },

//debug stuff
    {"print"                        ,lua_print                     },
    {"RunDKScriptCommand"           ,lua_run_dkscript_command      },
    
//retrieving lua vars
    {"GetCreatureNear",               lua_get_creature_near},
    {"getCreatureByCriterion",        lua_get_creature_by_criterion},
    {"getThingByIdx",                 lua_get_thing_by_idx},
    {"getThingsOfClass",              lua_get_things_of_class},
    {"isActionPointActivatedByPlayer",lua_is_action_point_activated_by_player},
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


void Player_register(lua_State *L);
void Thing_register(lua_State *L);


void reg_host_functions(lua_State *L)
{
    Player_register(L);
    global_register(L);
    Thing_register(L);
}
